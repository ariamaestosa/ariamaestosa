/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */



#include "Editors/ScoreEditor.h"
#include "Editors/ScoreAnalyser.h"
#include "Editors/RelativeXCoord.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/KeyPicker.h"
#include "GUI/ImageProvider.h"
#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "Midi/MeasureData.h"
#include "Renderers/RenderAPI.h"
#include "Dialogs/Preferences.h"
#include "Actions/EditAction.h"
#include "Actions/AddNote.h"

#include "AriaCore.h"

#include <string>



/*
 * Explainations:
 *
 *        'level'        describes Y position of note, ranges from 0 (highest pitch) to 73 (lowest pitch)
 *                    Each level represents and Y postion a note can take.
 *                    A black line on the score is a level. A white space between 2 black lines is a level.
 *                    White spaces above and below the score are made of levels too.
 *                    The height of a level in pixels is defined by 'int y_step'.
 *
 *        'note_12'    means A=0, A#=1, B=2, C=3, C#=4, etc.
 *
 *        'note_7'    means A=0, B=1, C=2, D=3, etc.
 *
 */

namespace AriaMaestosa {

// will contain half the height of a note 'main circle', this will be used for centering the note on the string
int head_radius = -1;

// height in pixels of each level
const int y_step = 5;

int findNotePitch(int note_7, PitchSign sharpness)
{
    int note=0;

    if (note_7==0) note+=2; // A
    else if (note_7==1) note+=0; // B
    else if (note_7==2) note+=11; // C
    else if (note_7==3) note+=9; // D
    else if (note_7==4) note+=7; // E
    else if (note_7==5) note+=6; // F
    else if (note_7==6) note+=4; // G
    else
    {
        std::cerr << "Invalid note: " << note_7 << std::endl;
        return 0;
    }

    if (sharpness==SHARP) note-=1;
    else if (sharpness==FLAT) note+=1;

    return note;
}

#if 0
#pragma mark -
#endif

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                            ScoreMidiConverter
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ScoreMidiConverter::ScoreMidiConverter()
{

    for(int n=0; n<7; n++) scoreNotesSharpness[n] = NATURAL;

    accidentals =  false;
    for(int n=0; n<7; n++) accidentalScoreNotesSharpness[n] = -1; // FIXME - why -1 and not PITCH_SIGN_NONE?
    accidentalsMeasure = -1;

    going_in_sharps = false;
    going_in_flats = false;
    octave_shift = 0;
}

void ScoreMidiConverter::setNoteSharpness(NOTES note, PitchSign sharpness)
{

    scoreNotesSharpness[note] = sharpness;

    if (sharpness == SHARP)
    {
        going_in_sharps = true;
        going_in_flats = false;
    }
    else if (sharpness == FLAT)
    {
        going_in_sharps = false;
        going_in_flats = true;
    }
}

// are we using a key that will make flat signs appear next to the key?
bool ScoreMidiConverter::goingInSharps()
{
    return going_in_sharps;
}
// are we using a key that will make sharp signs appear next to the key?
bool ScoreMidiConverter::goingInFlats()
{
    return going_in_flats;
}
// what sign should appear next to the key for this note? (FLAT, SHARP or PITCH_SIGN_NONE)
PitchSign ScoreMidiConverter::getKeySigSharpnessSignForLevel(const unsigned int level)
{
    assertExpr(level,<,73);
    
    PitchSign value = scoreNotesSharpness[ levelToNote7(level) ];
    
    if (value == NATURAL) return PITCH_SIGN_NONE;
    else return value;
}

int ScoreMidiConverter::getMiddleCLevel() { return middleCLevel; }

void ScoreMidiConverter::setOctaveShift(int octaves)   { octave_shift = octaves; }
int ScoreMidiConverter::getOctaveShift(){ return octave_shift; }
int ScoreMidiConverter::getScoreCenterCLevel()
{
    if (octave_shift == 1) return ottavaAltaCLevel;
    else if (octave_shift == -1) return ottavaBassaCLevel;

    return middleCLevel;
}

/*
// with the current key, what sign must be shown next to note if we want it to have given pitch? (FLAT, SHARP, NATURAL or PITCH_SIGN_NONE)
int ScoreMidiConverter::getSharpnessSignForMidiNote(const unsigned int note)
{
    return showSignNextToNote[ note ];
}
*/

// returns what note is at given level
int ScoreMidiConverter::levelToNote(const int level)
{
    if (level<0 or level>=73) return -1;
    return levelToMidiNote[level];
}

void ScoreMidiConverter::resetAccidentalsForNewRender()
{
    accidentals =  false;
    for(int n=0; n<7; n++) accidentalScoreNotesSharpness[n] = -1;
    accidentalsMeasure = -1;
}

// returns on what level the given note will appear, and with what sign
int ScoreMidiConverter::noteToLevel(Note* noteObj, PitchSign* sign)
{
    const int note = noteObj->pitchID;
    if (note>=128 or note<0) return -1;

    const int level = midiNoteToLevel[note];
    const NoteToLevelType current_type = midiNoteToLevel_type[note];

    PitchSign answer_sign = PITCH_SIGN_NONE;
    int answer_level = -1;

    if (current_type == SHARP_OR_FLAT)
    {
        // decide whether to use a flat or a sharp to display this note
        // first check if there's a user-specified sign. otherwise pick default (quite arbitrarly)
        bool useFlats = false;
        if (noteObj->preferred_accidental_sign != -1)
        {
            if (noteObj->preferred_accidental_sign == SHARP) useFlats = false;
            else if (noteObj->preferred_accidental_sign == FLAT) useFlats = true;
        }
        else
            useFlats = goingInFlats();

        if (useFlats and (note-1)>0)
        {
            if (sign!=NULL) answer_sign = FLAT;
            answer_level = midiNoteToLevel[note-1];
        }
        else if (note < 127)
        {
            if (sign!=NULL) answer_sign = SHARP;
            answer_level = midiNoteToLevel[note+1];
        }
        else answer_level = -1; // nothing found
    }
    else if (current_type == NATURAL_ON_LEVEL)
    {
        if (sign!=NULL) answer_sign = NATURAL;
        answer_level = level;
    }
    else if (current_type == DIRECT_ON_LEVEL)
    {
        if (sign!=NULL) answer_sign = PITCH_SIGN_NONE;
        answer_level = level;
    }
    else answer_level = -1; // nothing found

    // accidentals
    if (sign!=NULL)
    {
        if (accidentals)
        {
            const int measure = getMeasureData()->measureAtTick(noteObj->startTick);

            // when going to another measure, reset accidentals
            if (measure != accidentalsMeasure)
            {
                accidentals =  false;
                for(int n=0; n<7; n++) accidentalScoreNotesSharpness[n] = -1;
                accidentalsMeasure = measure;
            }
            else
            {
                // we are not going in another measure, apply accidental to current note
                const int current_accidental = accidentalScoreNotesSharpness[ levelToNote7(answer_level) ];
                if (current_accidental != -1)
                {
                    // the current note continues using the same accidental, no change
                    if (current_accidental == answer_sign) answer_sign = PITCH_SIGN_NONE;
                    // the current note does NOT continue using the same accidental, display another sign
                    else if (current_accidental != answer_sign and
                            (answer_sign == NATURAL or answer_sign == PITCH_SIGN_NONE))
                    {
                        const int accidentalType = accidentalScoreNotesSharpness[ levelToNote7(answer_level) ];
                        // we left the original key by adding a sharp of flat,
                        // and now we come back to the original key. show a
                        // natural sign
                        if (accidentalType == FLAT or accidentalType == SHARP)
                        {
                            answer_sign = NATURAL;
                        }
                        // we left the key by using a natural accidental on a key
                        // flat/sharp, and now we're coming back on the original key
                        // so show again the original sign
                        else if (accidentalType == NATURAL /*and answer_sign == PITCH_SIGN_NONE*/)
                        {
                            answer_sign = getKeySigSharpnessSignForLevel( answer_level );
                        }
                        else if (accidentalType == PITCH_SIGN_NONE)
                        {
                            // FIXME - not sure this is appropriate
                            std::cout << "WARNING: accidentalType == PITCH_SIGN_NONE :(" << std::endl;
                            answer_sign = NATURAL;
                        }
                    }

                }
            }
        }

        // set accidentals
        if (answer_sign != PITCH_SIGN_NONE)
        {
            accidentals = true;
            const int measure = getMeasureData()->measureAtTick(noteObj->startTick);
            accidentalsMeasure = measure;

            accidentalScoreNotesSharpness[ levelToNote7(answer_level) ] = answer_sign;

            // remove accidental if no more needed
            if ((scoreNotesSharpness[ levelToNote7(answer_level) ] == NATURAL and answer_sign == NATURAL) or
               scoreNotesSharpness[ levelToNote7(answer_level) ] == answer_sign)
            {
                accidentalScoreNotesSharpness[ levelToNote7(answer_level) ] = -1;
            }
        }
    }

    if (sign!=NULL) *sign = answer_sign;
    return answer_level;
}

// what is the name of the note played on this level?
int ScoreMidiConverter::levelToNote7(const unsigned int level)
{
    int r = 7-level%7;
    if (r < 0) r+=7;
    if (r > 6) r-=7;
    return r;
}

// called when key has changed, rebuilds conversion tables and other
// data needed for all conversions and information requests this class provides
void ScoreMidiConverter::updateConversionData()
{

    // FIXME - mess

    // reset all
    for(int n=0; n<128; n++)
    {
        midiNoteToLevel[n] = -1;
        midiNoteToLevel_type[n] = SHARP_OR_FLAT;
    }

    middleCLevel = -1;
    ottavaAltaCLevel = -1;
    ottavaBassaCLevel = -1;

    int middleCNote128 = 71;
    if ( scoreNotesSharpness[C] == SHARP ) middleCNote128 -= 1;
    else if ( scoreNotesSharpness[C] == FLAT ) middleCNote128 += 1;

    const int ottavaAltaCNote128 = middleCNote128 - 12;
    const int ottavaBassaCNote128 = middleCNote128 + 12;

    // do levelToMidiNote first
    int note_7 = 0, octave = 0;
    for(int n=0; n<73; n++)
    {
        const PitchSign sharpness = scoreNotesSharpness[note_7];

        levelToMidiNote[n] = findNotePitch( note_7, sharpness ) + octave*12;

        assertExpr(levelToMidiNote[n],<,128);
        assertExpr(levelToMidiNote[n],>,-1);

        midiNoteToLevel[ levelToMidiNote[n] ] = n;
        midiNoteToLevel_type[ levelToMidiNote[n] ] = DIRECT_ON_LEVEL;

        // find middle C
        if ( levelToMidiNote[n] == middleCNote128 ) middleCLevel = n; // set on which line is middle C
        if ( levelToMidiNote[n] == ottavaAltaCNote128 ) ottavaAltaCLevel = n;
        if ( levelToMidiNote[n] == ottavaBassaCNote128 ) ottavaBassaCLevel = n;

        // if note is flat or sharp, also find what this line would be with a natural sign
        if (sharpness != NATURAL)
        {
            const int natural_note_on_this_line = findNotePitch( note_7, NATURAL ) + octave*12;

            // FIXME - it may not be necessary to fill it again every time
            levelToNaturalNote[n] = natural_note_on_this_line;

            // only use natural signs if this note cannot be displayed without natural sign on another line
            // in wich case 'midiNoteToLevel' will have been set (or will be overwritten later)
            if (midiNoteToLevel[ natural_note_on_this_line ] == -1)
            {
                midiNoteToLevel[ natural_note_on_this_line ] = n;
                midiNoteToLevel_type[ natural_note_on_this_line ] = NATURAL_ON_LEVEL;
            }
        }
        else
        {
            // FIXME - it may not be necessary to fill it again every time
            levelToNaturalNote[n] = levelToMidiNote[n];
        }

        note_7 --;
        if (note_7 == 1) octave++;

        if (note_7 < 0)
            note_7 = 6;
    }

}

int ScoreMidiConverter::getMidiNoteForLevelAndSign(const unsigned int level, int sharpness)
{
    if (level < 0 or level > 73) return -1;

    if (sharpness == PITCH_SIGN_NONE) return levelToNote(level);
    else if (sharpness == NATURAL) return levelToNaturalNote[level];
    else if (sharpness == SHARP) return levelToNaturalNote[level]-1;
    else if (sharpness == FLAT) return levelToNaturalNote[level]+1;
    else return -1; // shouldn't happen
}

#if 0
#pragma mark -
#endif

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ScoreEditor::ScoreEditor(Track* track) : Editor(track)
{
    g_clef = true;
    f_clef = true;

    const long scoreView = Core::getPrefsValue("scoreview");
    musicalNotationEnabled = scoreView == 0 or scoreView == 1;
    linearNotationEnabled  = scoreView == 0 or scoreView == 2;

    converter = new ScoreMidiConverter();

    converter->updateConversionData();

    g_clef_analyser = new ScoreAnalyser(this, converter->getScoreCenterCLevel()-5);
    f_clef_analyser = new ScoreAnalyser(this, converter->getScoreCenterCLevel()+6);

    setYStep( y_step );

    sb_position = converter->getMiddleCLevel() / 73.0;
}
ScoreEditor::~ScoreEditor()
{
}

/*
 * These will be called by the popup menu from KeyPicker when the user changes settings
 */
void ScoreEditor::enableFClef(bool enabled)     { f_clef = enabled;       }
void ScoreEditor::enableGClef(bool enabled)     { g_clef = enabled;       }
void ScoreEditor::enableMusicalNotation(const bool enabled) { musicalNotationEnabled = enabled; }
void ScoreEditor::enableLinearNotation(const bool enabled)  { linearNotationEnabled = enabled; }

/** get info about clefs */
bool ScoreEditor::isGClefEnabled() const { return g_clef; }
bool ScoreEditor::isFClefEnabled() const { return f_clef; }

// order in wich signs of the key signature appear
const NOTES sharp_order[] = { F, C, G, D, A, E, B };
const NOTES flat_order[]  = { B, E, A, D, G, C, F };

// where parameters are e.g. 5 sharps, 3 flats, etc.
void ScoreEditor::loadKey(const PitchSign sharpness_symbol, const int symbol_amount)
{
    // reset key signature before beginning
    converter->setNoteSharpness(A, NATURAL);
    converter->setNoteSharpness(B, NATURAL);
    converter->setNoteSharpness(C, NATURAL);
    converter->setNoteSharpness(D, NATURAL);
    converter->setNoteSharpness(E, NATURAL);
    converter->setNoteSharpness(F, NATURAL);
    converter->setNoteSharpness(G, NATURAL);

    key_flats_amnt = 0;
    key_sharps_amnt = 0;

    if (sharpness_symbol == SHARP)
    {
        key_sharps_amnt = symbol_amount;

        for(int n=A; n<symbol_amount; n++)
        {
            converter->setNoteSharpness( sharp_order[n], sharpness_symbol);
        }
    }
    else if (sharpness_symbol == FLAT)
    {
        key_flats_amnt = symbol_amount;

        for(int n=A; n<symbol_amount; n++)
        {
            converter->setNoteSharpness( flat_order[n], sharpness_symbol);
        }
    }

    converter->updateConversionData();
    g_clef_analyser->setStemPivot(converter->getScoreCenterCLevel()-5);
    f_clef_analyser->setStemPivot(converter->getScoreCenterCLevel()+6);
}

ScoreMidiConverter* ScoreEditor::getScoreMidiConverter()
{
    return converter;
}

// user clicked on a sign in the track's header
void ScoreEditor::setNoteSign(const int sign, const int noteID)
{

    const int noteLevel = converter->noteToLevel(track->getNote(noteID));

    const int new_pitch = converter->getMidiNoteForLevelAndSign(noteLevel, sign);
    if (new_pitch == -1)
    {
        wxBell();
        return;
    }

    Note* note = track->getNote(noteID);
    note->pitchID = new_pitch;

    if (sign != NATURAL) note->preferred_accidental_sign = sign;

}

#if 0
#pragma mark -
#endif

#define LEVEL_TO_Y( lvl ) (y_step * lvl + getEditorYStart() - getYScrollInPixels()-2)

namespace EditorStemParams
{
    int stem_up_x_offset = 9;
    float stem_up_y_offset = 0.4;
    int stem_down_x_offset = 1;
    float stem_down_y_offset = 0.8;

    int getStemX(const int tick, const STEM stem_type)
    {
        RelativeXCoord relX(tick, MIDI);
        const int noteX = relX.getRelativeTo(WINDOW);
        
        if     (stem_type == STEM_UP)   return (noteX + stem_up_x_offset);
        else if (stem_type == STEM_DOWN) return (noteX + stem_down_x_offset);
        else return -1;
    }
    int getStemX(const NoteRenderInfo& info)
    {
        return getStemX(info.tick, info.stem_type);
    }
}
using namespace EditorStemParams;
    
// where 'renderInfo' is a 'NoteRenderInfo' object of current note.
// 'vector' is where all visible notes will are added, to be analysed after
void ScoreEditor::renderNote_pass1(NoteRenderInfo& renderInfo)
{


    AriaRender::lineWidth(2);

    renderInfo.setY( LEVEL_TO_Y(renderInfo.level) - head_radius + 4 );

    // note head
    /*
    if (renderInfo.unknown_duration)
    {
        AriaRender::primitives();
        AriaRender::character('?', renderInfo.x, renderInfo.y + 3);
    }
    else
     */
    
    RelativeXCoord relX(renderInfo.tick, MIDI);
    const int noteX = relX.getRelativeTo(WINDOW);
    
    if (renderInfo.instant_hit)
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::lineWidth(2);
        // draw a 'X'
        AriaRender::line(noteX-5, renderInfo.getY() - 7,
                         noteX+5, renderInfo.getY() + 7);
        AriaRender::line(noteX-5, renderInfo.getY() + 7,
                         noteX+5, renderInfo.getY() - 7);
    }
    else if (renderInfo.hollow_head)
    {
        AriaRender::images();
        if (renderInfo.selected)
            AriaRender::setImageState(AriaRender::STATE_SELECTED_NOTE);
        else
            AriaRender::setImageState(AriaRender::STATE_NOTE);
        noteOpen->move(noteX, renderInfo.getY());
        noteOpen->render();
    }
    else
    {
        AriaRender::images();
        if (renderInfo.selected)
            AriaRender::setImageState(AriaRender::STATE_SELECTED_NOTE);
        else
            AriaRender::setImageState(AriaRender::STATE_NOTE);
        noteClosed->move(noteX, renderInfo.getY());
        noteClosed->render();
    }

    // if note is above or below keys, we need to display small lines from the score
    // to the note so that the amount of levels is visible
    AriaRender::primitives();
    AriaRender::color(0,0,0);
    AriaRender::lineWidth(1);
    const int middle_c_level = converter->getScoreCenterCLevel();

    // set min and max level where small line is not needed
    int score_from_level = 0;
    if (g_clef) score_from_level = middle_c_level - 10;
    else if (f_clef) score_from_level = middle_c_level + 2;

    int score_to_level = 999;
    if (f_clef) score_to_level = middle_c_level + 11;
    else if (g_clef) score_to_level = middle_c_level - 1;

    // draw small lines above score if needed
    if (renderInfo.level < score_from_level)
    {
        for(int lvl=score_from_level+1; lvl>renderInfo.level+renderInfo.level%2; lvl -= 2)
        {
            const int lvly = getEditorYStart() + y_step*lvl - head_radius - getYScrollInPixels() + 2;
            AriaRender::line(noteX-5, lvly, noteX+15, lvly);
        }
    }

    // draw small lines below score if needed
    if (renderInfo.level > score_to_level)
    {
        for(int lvl=score_to_level; lvl<=renderInfo.level-renderInfo.level%2+2; lvl += 2)
        {
            const int lvly = getEditorYStart() + y_step*lvl - head_radius - getYScrollInPixels() + 2;
            AriaRender::line(noteX-5, lvly, noteX+15, lvly);
        }
    }

    // draw small lines between both scores if needed
    if (g_clef and f_clef and renderInfo.level == middle_c_level)
    {
        const int lvly = getEditorYStart() + y_step*(middle_c_level+1) - head_radius - getYScrollInPixels() + 2;
        AriaRender::line(noteX-5, lvly, noteX+15, lvly);
    }

    AriaRender::lineWidth(2);

    // dotted
    if (renderInfo.dotted)
    {
        AriaRender::color(1,1,1);
        AriaRender::pointSize(5);
        AriaRender::point(noteX + 14, renderInfo.getY() + 5);

        if (renderInfo.selected) AriaRender::color(1,0,0);
        else AriaRender::color(0,0,0);
        AriaRender::pointSize(3);

        AriaRender::point(noteX + 14, renderInfo.getY() + 5);
    }

    // sharpness sign
    if (renderInfo.sign == SHARP)
    {
        AriaRender::images();
        sharpSign->move(noteX - 5, renderInfo.getY() + head_radius);
        sharpSign->render();
        AriaRender::primitives();
    }
    else if (renderInfo.sign == FLAT)
    {
        AriaRender::images();
        flatSign->move(noteX - 5, renderInfo.getY() + head_radius);
        flatSign->render();
        AriaRender::primitives();
    }
    else if (renderInfo.sign == NATURAL)
    {
        AriaRender::images();
        naturalSign->move(noteX - 5, renderInfo.getY() + head_radius);
        naturalSign->render();
        AriaRender::primitives();
    }
}

void ScoreEditor::renderNote_pass2(NoteRenderInfo& renderInfo, ScoreAnalyser* analyser)
{
    AriaRender::primitives();
    if (renderInfo.selected)
        AriaRender::color(1,0,0);
    else
        AriaRender::color(0,0,0);

    // stem
    if (renderInfo.draw_stem)
    {

        if (renderInfo.stem_type == STEM_UP or renderInfo.stem_type == STEM_DOWN)
        {
            int stem_from_y = LEVEL_TO_Y(renderInfo.getStemOriginLevel());
            if (renderInfo.stem_type == STEM_DOWN) stem_from_y += 6;
            
            AriaRender::line( getStemX(renderInfo), stem_from_y,
                              getStemX(renderInfo), LEVEL_TO_Y(analyser->getStemTo(renderInfo))   );
        }


        // flags
        if (renderInfo.flag_amount>0 and not renderInfo.beam)
        {
            static const int stem_height = noteFlag->image->height;
            const int stem_end = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
            const int flag_y_origin = (renderInfo.stem_type==STEM_UP ? stem_end : stem_end - stem_height );
            const int flag_x_origin = getStemX(renderInfo);
            const int flag_step = (renderInfo.stem_type==STEM_UP ? 7 : -7 );

            noteFlag->setFlip( false, renderInfo.stem_type!=STEM_UP );

            AriaRender::images();
            for(int n=0; n<renderInfo.flag_amount; n++)
            {
                noteFlag->move( flag_x_origin , flag_y_origin + n*flag_step);
                noteFlag->render();
            }
            AriaRender::primitives();
        }
    }

    // triplet
    if (renderInfo.drag_triplet_sign and renderInfo.triplet_arc_tick_start != -1)
    {
        int triplet_arc_x_start = -1;
        int triplet_arc_x_end = -1;
        
        if (renderInfo.triplet_arc_tick_start != -1)
        {
             RelativeXCoord relXStart(renderInfo.triplet_arc_tick_start, MIDI);
             triplet_arc_x_start = relXStart.getRelativeTo(WINDOW) + 8;
        }
        if (renderInfo.triplet_arc_tick_end != -1)
        {
            RelativeXCoord relXEnd(renderInfo.triplet_arc_tick_end, MIDI);
            triplet_arc_x_end = relXEnd.getRelativeTo(WINDOW) + 8;
        }
        

        const int center_x = (triplet_arc_x_end == -1 ? triplet_arc_x_start : (triplet_arc_x_start + triplet_arc_x_end)/2);
        const int radius_x = (triplet_arc_x_end == -1 or  triplet_arc_x_end == triplet_arc_x_start ?
                              10 : (triplet_arc_x_end - triplet_arc_x_start)/2);

        AriaRender::color(0,0,0);
        AriaRender::arc(center_x, LEVEL_TO_Y(renderInfo.triplet_arc_level) + (renderInfo.triplet_show_above ? 0 : 10), radius_x, 10, renderInfo.triplet_show_above);

        AriaRender::images();
        AriaRender::renderNumber(wxT("3"), center_x-2, LEVEL_TO_Y(renderInfo.triplet_arc_level) + ( renderInfo.triplet_show_above? 2 : 20));
        AriaRender::primitives();
    }

    
    // tie
    if (renderInfo.getTiedToTick() != -1)
    {
    
        const float center_tick = (renderInfo.getTiedToTick() + renderInfo.tick)/2.0;
        
        RelativeXCoord relXFrom(renderInfo.getTiedToTick(), MIDI);
        const int x_from = relXFrom.getRelativeTo(WINDOW);
        
        RelativeXCoord relXTo(renderInfo.tick, MIDI);
        const int x_to = relXTo.getRelativeTo(WINDOW);
                
        const bool show_above = renderInfo.isTieUp();

        RelativeXCoord relCenter(center_tick, MIDI);
        const int center_x = relCenter.getRelativeTo(WINDOW) + 6;
        const int radius_x = (x_to - x_from)/2;
        
        const int base_y = LEVEL_TO_Y(renderInfo.getStemOriginLevel()) + head_radius;
        AriaRender::arc(center_x, base_y + (show_above ? -5 : 5), radius_x, 6, show_above);
    }

    // beam
    if (renderInfo.beam)
    {
        AriaRender::color(0,0,0);
        AriaRender::lineWidth(2);
        
        const int x1 = getStemX(renderInfo);
        int y1       = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
        int y2       = LEVEL_TO_Y(renderInfo.beam_to_level);

        const int y_diff = (renderInfo.stem_type == STEM_UP ? 5 : -5);

        AriaRender::lineSmooth(true);
        for(int n=0; n<renderInfo.flag_amount; n++)
        {
            AriaRender::line(x1, y1, getStemX(renderInfo.beam_to_tick, renderInfo.stem_type), y2);
            y1 += y_diff;
            y2 += y_diff;
        }
        AriaRender::lineSmooth(false);
    }
}


void renderSilence(const int duration, const int tick, const int type, const int silences_y, const bool triplet, const bool dotted,
                   const int dot_delta_x, const int dot_delta_y)
{
    assertExpr(tick,>,-1);
    RelativeXCoord relX(tick, MIDI);
    const int x = relX.getRelativeTo(WINDOW) + 5;
    
    if ( type == 1 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x,silences_y, x+15, silences_y+y_step);
    }
    else if ( type == 2 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x, silences_y+y_step, x+15, silences_y+y_step*2);
    }
    else if ( type == 4 )
    {
        AriaRender::images();
        silence4->move(x, silences_y);
        silence4->render();
    }
    else if ( type == 8 )
    {
        AriaRender::images();
        silence8->move(x-3, silences_y);
        silence8->render();
    }
    else if ( type == 16 )
    {
        AriaRender::images();
        silence8->move(x-7, silences_y+3);
        silence8->render();
        silence8->move(x-5, silences_y-3);
        silence8->render();
    }

    AriaRender::primitives();

    // dotted
    if (dotted)
    {
        AriaRender::color(1,1,1);
        AriaRender::pointSize(5);
        AriaRender::point(x + 14 + dot_delta_x, silences_y + 5 + dot_delta_y);

        AriaRender::color(0,0,0);
        AriaRender::pointSize(3);
        AriaRender::point(x + 14 + dot_delta_x, silences_y + 5 + dot_delta_y);
    }

    // triplet
    if (triplet)
    {
        AriaRender::arc(x+5, silences_y + 25, 10, 10, false);

        AriaRender::color(0,0,0);

        AriaRender::images();
        AriaRender::renderNumber(wxT("3"), x+3, silences_y+31);
        AriaRender::primitives();
    }

}


void ScoreEditor::render(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    if (head_radius == -1) head_radius = noteOpen->getImageHeight()/2;

    if (!ImageProvider::imagesLoaded()) return;
    const int yscroll = getYScrollInPixels();

    AriaRender::beginScissors(10, getEditorYStart(), width-15, 20+height);

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);

    const int middle_c_level = converter->getScoreCenterCLevel();

    g_clef_analyser->clearAndPrepare();
    f_clef_analyser->clearAndPrepare();

    if (g_clef)
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level-2)*y_step + y_step/2 - yscroll,
                                 getEditorYStart() + (middle_c_level-10)*y_step + y_step/2 - yscroll);

    if (f_clef)
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level+2)*y_step + y_step/2 - yscroll,
                                 getEditorYStart() + (middle_c_level+10)*y_step + y_step/2 - yscroll);

    // ---------------------- draw notes ----------------------------
    AriaRender::color(0,0,0);
    AriaRender::pointSize(4);

    const int noteAmount = track->getNoteAmount();

    const int first_x_to_consider = getMeasureData()->firstPixelInMeasure( getMeasureData()->measureAtPixel(0) ) + 1;
    const int last_x_to_consider = getMeasureData()->lastPixelInMeasure( getMeasureData()->measureAtPixel(width+15) );

    if (musicalNotationEnabled) converter->resetAccidentalsForNewRender();

    // render pass 1. draw linear notation if relevant, gather information and do initial rendering for musical notation
    for(int n=0; n<noteAmount; n++)
    {
        PitchSign note_sign;
        const int noteLevel = converter->noteToLevel(track->getNote(n), &note_sign);

        if (noteLevel == -1) continue;
        const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
        const int tick = track->getNoteStartInMidiTicks(n);

        int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();
        const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();

        // don't consider notes that won't be visible
        if (x2 < first_x_to_consider) continue;
        if (x1 > last_x_to_consider) break;

        if (linearNotationEnabled)
        {
            if (musicalNotationEnabled) x1 += 8;

            if (x1 < x2) // when notes are too short to be visible, don't draw them
            {
                float volume=track->getNoteVolume(n)/127.0;

                // draw the quad with black border that is visible in linear notation mode
                AriaRender::primitives();
                if (track->isNoteSelected(n) and focus)
                    AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
                else
                    AriaRender::color((1-volume*0.7), (1-volume*0.7), 1);

                const int y1 = noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels()-1;
                const int y2 = (noteLevel+1)*y_step + getEditorYStart() - getYScrollInPixels()-1;

                if (musicalNotationEnabled)
                    AriaRender::bordered_rect_no_start(x1+1, y1, x2-1, y2);
                else
                    AriaRender::bordered_rect(x1+1, y1, x2-1, y2);

            }

            // when musical notation is disabled, we need to render the sharpness sign here
            // (when it's activated, it's done by the note drawing code. When note drawing code
            // is disabled, we need to do it ourselves).
            if (not musicalNotationEnabled)
            {
                AriaRender::images();

                if (note_sign == SHARP)
                {
                    sharpSign->move(x1 - 5, noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
                    sharpSign->render();
                }
                else if (note_sign == FLAT)
                {
                    flatSign->move(x1 - 5,  noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
                    flatSign->render();
                }
                else if (note_sign == NATURAL)
                {
                    naturalSign->move(x1 - 5,  noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
                    naturalSign->render();
                }

                AriaRender::primitives();
            }
        }// end if linear

        if (musicalNotationEnabled)
        {
            // build visible notes vector with initial info in it
            NoteRenderInfo currentNote(tick, noteLevel, noteLength, note_sign,
                                     track->isNoteSelected(n), track->getNotePitchID(n));

            // add note to either G clef score or F clef score
            if (g_clef and not f_clef)
                g_clef_analyser->addToVector(currentNote, false);
            else if (f_clef and not g_clef)
                f_clef_analyser->addToVector(currentNote, false);
            else if (f_clef and g_clef)
            {
                const int middleC = converter->getScoreCenterCLevel();
                if (noteLevel < middleC)
                    g_clef_analyser->addToVector(currentNote, false);
                else if (noteLevel > middleC)
                    f_clef_analyser->addToVector(currentNote, false);
                else
                {
                    // note is exactly on middle C... do our best to
                    // guess on which clef to put this note
                    // we'll check nearby notes in case it can help us'
                    int check_note = -1;
                    if (n>0) check_note=n-1;
                    else if (n+1<noteAmount) check_note = n+1;

                    if (check_note != -1)
                    {
                        const int checkNoteLevel = converter->noteToLevel(track->getNote(check_note), NULL);
                        if (checkNoteLevel > middleC)
                            f_clef_analyser->addToVector(currentNote, false);
                        else
                            g_clef_analyser->addToVector(currentNote, false);
                    }
                    else
                        g_clef_analyser->addToVector(currentNote, false);
                } // end if note on middle C
            } // end if both G and F clefs
        } // end if musical notation enabled
    } // next note


    // render musical notation if enabled
    if (musicalNotationEnabled)
    {
        if (g_clef)
        {
            const int silences_y = getEditorYStart() + y_step*(converter->getScoreCenterCLevel()-8) - getYScrollInPixels() + 1;
            renderScore(g_clef_analyser, silences_y);
        }

        if (f_clef)
        {
            const int silences_y = getEditorYStart() + y_step*(converter->getScoreCenterCLevel()+4) - getYScrollInPixels() + 1;
            renderScore(f_clef_analyser, silences_y);
        }
    }


    AriaRender::lineWidth(1);
    // ------------------------- mouse drag (preview) ------------------------

    AriaRender::primitives();

    if (!clickedOnNote and mouse_is_in_editor)
    {
        // selection
        if (selecting)
        {
            AriaRender::color(0, 0, 0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);

        }
        else
        {
            // ----------------------- add note (preview) --------------------

            AriaRender::color(1, 0.85, 0);

            int preview_x1=
                (int)(
                      (snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI) ) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            int preview_x2=
                (int)(
                      (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) ) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );

            if (!(preview_x1<0 || preview_x2<0) and preview_x2>preview_x1)
            {

                const int y_base = ((mousey_initial - getEditorYStart() + getYScrollInPixels())/y_step)*y_step;
                const int y_add = getEditorYStart() - getYScrollInPixels();

                AriaRender::rect(preview_x1+getEditorsXStart(), y_base-2 + y_add,
                                 preview_x2+getEditorsXStart(), y_base+y_step+1 + y_add);

            }

        }// end if selection or addition

    }


    // ------------------------- move note (preview) -----------------------
    AriaRender::primitives();
    if (clickedOnNote)
    {
        AriaRender::color(1, 0.85, 0, 0.5);

        int x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
        int y_difference = mousey_current - mousey_initial;

        const int x_pixel_move = (int)( snapMidiTickToGrid(x_difference) * sequence->getZoom() );
        const int y_step_move = (int)round( (float)y_difference/ (float)y_step );

        // move a single note
        if (lastClickedNote!=-1)
        {
            int x1=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorsXStart();
            const int x2=track->getNoteEndInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorsXStart();
            //const int notePitch = track->getNotePitchID(lastClickedNote);
            const int noteLevel = converter->noteToLevel(track->getNote(lastClickedNote));

            AriaRender::rect(x1+1+x_pixel_move, (noteLevel+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels()-1,
                             x2-1+x_pixel_move, (noteLevel+1+y_step_move)*y_step + getEditorYStart() - getYScrollInPixels()-1);

        }
        else
        {
            // move a bunch of notes

            for(int n=0; n<track->getNoteAmount(); n++)
            {
                if (!track->isNoteSelected(n)) continue;

                int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();
                const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();
                //const int notePitch = track->getNotePitchID(n);
                const int noteLevel = converter->noteToLevel(track->getNote(n));

                AriaRender::rect(x1+1+x_pixel_move, (noteLevel+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels()-1,
                                 x2-1+x_pixel_move, (noteLevel+1+y_step_move)*y_step + getEditorYStart() - getYScrollInPixels()-1);

            }//next

        }

    }// end if clicked on note


    // --------------------------- grey left part -----------------------------
    if (!focus) AriaRender::color(0.4, 0.4, 0.4);
    else AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0, getEditorYStart(),
                     getEditorsXStart()-3, getYEnd());

    // ------------------------------- draw keys and horizontal lines -------------------------------

    AriaRender::color(0,0,0);

    if (g_clef)
    {
        AriaRender::primitives();

        // draw horizontal score lines
        for(int n = middle_c_level - 10 ; n < middle_c_level; n+=2)
        {
            const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            AriaRender::line(0, liney, getXEnd(), liney);
        }

        // draw sharp/flat signs next to key
        AriaRender::images();

        int max_level_with_signs = -1;
        int min_level_with_signs = -1;

        if (converter->goingInSharps())
        {
            min_level_with_signs = middle_c_level - 11;
            max_level_with_signs = middle_c_level - 4;
        }
        else if (converter->goingInFlats())
        {
            min_level_with_signs = middle_c_level - 9;
            max_level_with_signs = middle_c_level - 2;
        }


        AriaRender::images();
        for(int n = min_level_with_signs; n < max_level_with_signs; n++)
        {
            const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            const PitchSign sharpness = converter->getKeySigSharpnessSignForLevel(n);

            if (sharpness == SHARP)
            {
                sharpSign->move( 48 + sharp_sign_x[ converter->levelToNote7(n) ], liney-1 );
                sharpSign->render();
            }
            else if (sharpness == FLAT)
            {
                flatSign->move( 48 + flat_sign_x[ converter->levelToNote7(n) ], liney );
                flatSign->render();
            }
        }
    }

    if (f_clef)
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);

        // draw horizontal score lines
        for(int n = middle_c_level+2 ; n < middle_c_level + 11; n+=2)
        {
            const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            AriaRender::line(0, liney, getXEnd(), liney);
        }

        // draw sharp/flat signs next to key
        AriaRender::images();

        int max_level_with_signs = -1;
        int min_level_with_signs = -1;

        if (converter->goingInSharps())
        {
            min_level_with_signs = middle_c_level + 3;
            max_level_with_signs = middle_c_level + 10;
        }
        else if (converter->goingInFlats())
        {
            min_level_with_signs = middle_c_level + 5;
            max_level_with_signs = middle_c_level + 12;
        }

        AriaRender::images();

        for(int n = min_level_with_signs; n < max_level_with_signs; n++)
        {
            const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            const int sharpness = converter->getKeySigSharpnessSignForLevel(n);
            if (sharpness == SHARP)
            {
                sharpSign->move( 48 + sharp_sign_x[ converter->levelToNote7(n) ], liney-1 ); sharpSign->render();
            }
            else if (sharpness == FLAT)
            {
                flatSign->move( 48 + flat_sign_x[ converter->levelToNote7(n) ], liney ); flatSign->render();
            }
        }
    }

    // --------------------------- clefs -------------------------
    AriaRender::images();
    if (g_clef)
    {
        const int clef_y = getEditorYStart() + (middle_c_level-6)*y_step -  yscroll + 5;
        clefG_drawable->move(getEditorsXStart() - 55, clef_y);
        clefG_drawable->render();
        /*
        AriaRender::color(0, 0, 0);
        AriaRender::primitives();
        if (converter->getOctaveShift() == -1) AriaRender::text("8", getEditorsXStart() - 30, clef_y-10 );
        else if (converter->getOctaveShift() == 1) AriaRender::text("8", getEditorsXStart() - 30, clef_y );
         */
    }
    if (f_clef)
    {
        const int clef_y = getEditorYStart() + (middle_c_level+4)*y_step -  yscroll + 5;
        clefF_drawable->move(getEditorsXStart() - 65, clef_y);
        clefF_drawable->render();
        /*
        AriaRender::color(0, 0, 0);
        AriaRender::primitives();
        if (converter->getOctaveShift() == -1) AriaRender::text("8", getEditorsXStart() - 30, clef_y-10 );
        else if (converter->getOctaveShift() == 1) AriaRender::text("8", getEditorsXStart() - 30, clef_y+10 );
         */
    }

    // ---------------------------- scrollbar -----------------------
    if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::setImageState(AriaRender::STATE_NORMAL);
    AriaRender::endScissors();
}


void ScoreEditor::renderScore(ScoreAnalyser* analyser, const int silences_y)
{
    int visibleNoteAmount = analyser->noteRenderInfo.size();

    // first note rendering pass
    for(int i=0; i<visibleNoteAmount; i++) renderNote_pass1( analyser->noteRenderInfo[i] );

    // render silences
    const unsigned int first_visible_measure = getMeasureData()->measureAtPixel( getEditorsXStart() );
    const unsigned int last_visible_measure = getMeasureData()->measureAtPixel( getXEnd() );
    analyser->renderSilences( &renderSilence, first_visible_measure, last_visible_measure, silences_y );

    // ------------------------- second note rendering pass -------------------

    // analyse notes to know how to build the score
    analyser->analyseNoteInfo();

    // triplet signs, tied notes, flags and beams
    visibleNoteAmount = analyser->noteRenderInfo.size();
    for(int i=0; i<visibleNoteAmount; i++)
    {
        assertExpr(i,<,(int)analyser->noteRenderInfo.size());
        renderNote_pass2(analyser->noteRenderInfo[i], analyser);
    }

}


// ***************************************************************************************************************************************************
// ****************************************************    EVENT METHODS      ************************************************************************
// ***************************************************************************************************************************************************
#if 0
#pragma mark -
#endif

void ScoreEditor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseHeldDown(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void ScoreEditor::TrackPropertiesDialog(RelativeXCoord mousex_current, int mousey_current,
                              RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::TrackPropertiesDialog(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void ScoreEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
    if (x.getRelativeTo(EDITOR)<-20 and x.getRelativeTo(WINDOW)>15 and y>getEditorYStart())
    {
        KeyPicker* picker = Core::getKeyPicker();
        picker->setParent(track);
        picker->setChecks( musicalNotationEnabled, linearNotationEnabled, f_clef, g_clef, converter->getOctaveShift() );
        Display::popupMenu( picker,x.getRelativeTo(WINDOW),y);
        return;
    }

    Editor::mouseDown(x, y);

}
void ScoreEditor::mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseDrag(mousex_current, mousey_current, mousex_initial, mousey_initial);

}
void ScoreEditor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
                          RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);

}

void ScoreEditor::rightClick(RelativeXCoord x, int y)
{
    Editor::rightClick(x, y);

}

// ****************************************************************************************************************************************************
// ****************************************************    EDITOR METHODS      ************************************************************************
// ****************************************************************************************************************************************************
#if 0
#pragma mark -
#endif


int ScoreEditor::getYScrollInPixels()
{
    // check if visible area is large enough to display everything
    if (73*y_step <= height)
    {
        useVerticalScrollbar(false);
        return 0;
    }
    else
    {
        useVerticalScrollbar(true);
    }

    return (int)(  sb_position*(73*y_step-height) );
}

NoteSearchResult ScoreEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int head_radius = noteOpen->getImageHeight()/2;
    const int noteAmount = track->getNoteAmount();
    const int mx = x.getRelativeTo(WINDOW);

    for(int n=0; n<noteAmount; n++)
    {

        //const int notePitch = track->getNotePitchID(n);
        const int noteLevel = converter->noteToLevel(track->getNote(n));
        if (noteLevel == -1) continue;
        const int note_y = getEditorYStart() + y_step*noteLevel - head_radius - getYScrollInPixels() + 2;
        const int note_x = getEditorsXStart() + track->getNoteStartInPixels(n)  - sequence->getXScrollInPixels();
        const int note_x2 = getEditorsXStart() + track->getNoteEndInPixels(n)  - sequence->getXScrollInPixels();

        if (linearNotationEnabled)
        {
            if ( mx<note_x2 and mx>note_x and y < note_y+11 and y > note_y)
            {
                noteID = n;

                if (track->isNoteSelected(n) and !Display:: isSelectLessPressed())
                    // clicked on a selected note
                    return FOUND_SELECTED_NOTE;
                else
                    return FOUND_NOTE;
            }
        }
        else if (musicalNotationEnabled)
        {

            if ( mx<note_x+11 and mx>note_x and y < note_y+11 and y > note_y+2)
            {
                noteID = n;

                if (track->isNoteSelected(n) and !Display:: isSelectLessPressed())
                    // clicked on a selected note
                    return FOUND_SELECTED_NOTE;
                else
                    return FOUND_NOTE;
            }

        }
        else { return FOUND_NOTHING; }

    }

    return FOUND_NOTHING;
}

void ScoreEditor::noteClicked(const int id)
{
    track->selectNote(ALL_NOTES, false);
    track->selectNote(id, true);
    track->playNote(id);
}

void ScoreEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    const int level = (mouseY - getEditorYStart() + getYScrollInPixels())/y_step;
    if (level<0 or level>=73) return;
    const int note = converter->levelToNote(level);
    if (note == -1) return;

    track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, default_volume ) );
}

void ScoreEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
        if (note.startTick+relativeX < 0) return; // refuse to move before song start

        note.startTick += relativeX;
        note.endTick   += relativeX;

        if (relativeY==0) return;
        /*
        if (Display::isSelectMorePressed() or Display::isCtrlDown())
        {
            note.pitchID += relativeY;
        }
        else
        {
            */
            int noteLevel = converter->noteToLevel(&note);
            noteLevel += relativeY;
            if (noteLevel > 0 and noteLevel < 73) // reject illegal notes
                note.pitchID = converter->levelToNote(noteLevel);
            /*
        }
        */
}

void ScoreEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial)
{
    const int head_radius = noteOpen->getImageHeight()/2;
    const int noteAmount = track->getNoteAmount();
    const int mxc = mousex_current.getRelativeTo(WINDOW);
    const int mxi = mousex_initial.getRelativeTo(WINDOW);

    for(int n=0; n<noteAmount; n++)
    {
        //const int notePitch = track->getNotePitchID(n);
        const int noteLevel = converter->noteToLevel(track->getNote(n));
        if (noteLevel == -1) continue;
        const int note_y = getEditorYStart() + y_step*noteLevel - head_radius - getYScrollInPixels() + 2;
        const int note_x = getEditorsXStart() + track->getNoteStartInPixels(n)  - sequence->getXScrollInPixels();

        if ( std::min(mxc, mxi)<note_x and std::max(mxc,mxi)>note_x and
            std::min(mousey_current, mousey_initial) < note_y and
            std::max(mousey_current, mousey_initial) > note_y)
        {
            track->selectNote(n, true);
        }
        else
        {
            track->selectNote(n, false);
        }

    }
}


int ScoreEditor::getYStep(){ return y_step; }
int ScoreEditor::getHalfNoteHeight(){ return head_radius; }

}
