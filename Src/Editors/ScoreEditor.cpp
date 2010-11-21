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

#include "Actions/EditAction.h"
#include "Actions/AddNote.h"
#include "Dialogs/Preferences.h"
#include "Editors/ScoreEditor.h"
#include "Analysers/ScoreAnalyser.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/ImageProvider.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/KeyPicker.h"
#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "Renderers/RenderAPI.h"

#include "AriaCore.h"

#include <string>

/**
 * Explainations:
 *
 *        'level'        describes Y position of note, ranges from 0 (highest pitch) to 73 (lowest pitch)
 *                    Each level represents and Y postion a note can take.
 *                    A black line on the score is a level. A white space between 2 black lines is a level.
 *                    White spaces above and below the score are made of levels too.
 *                    The height of a level in pixels is defined by 'int y_step'.
 *
 *        'note_12'   means A=0, A#=1, B=2, C=3, C#=4, etc.
 *
 *        'note_7'    means A=0, B=1, C=2, D=3, etc.
 *
 */

using namespace AriaMaestosa;

namespace AriaMaestosa
{
    /** will contain half the height of a note 'main circle', this will be used for centerin
      *  the note on the line
      */
    int head_radius = -1;
    
    /** height in pixels of each level */
    const int Y_STEP_HEIGHT = 5;
}

#if 0
#pragma mark -
#pragma mark ScoreMidiConverter
#endif

// ----------------------------------------------------------------------------------------------------------
// ---------------------------------------      ScoreMidiConverter     --------------------------------------
// ----------------------------------------------------------------------------------------------------------

ScoreMidiConverter::ScoreMidiConverter()
{

    for (int n=0; n<7; n++) scoreNotesSharpness[n] = NATURAL;

    accidentals =  false;
    for (int n=0; n<7; n++) accidentalScoreNotesSharpness[n] = -1; // FIXME - why -1 and not PITCH_SIGN_NONE?
    accidentalsMeasure = -1;

    going_in_sharps = false;
    going_in_flats = false;
    octave_shift = 0;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::setNoteSharpness(Note7 note, PitchSign sharpness)
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

// ----------------------------------------------------------------------------------------------------------

bool ScoreMidiConverter::goingInSharps()
{
    return going_in_sharps;
}

// ----------------------------------------------------------------------------------------------------------

bool ScoreMidiConverter::goingInFlats()
{
    return going_in_flats;
}

// ----------------------------------------------------------------------------------------------------------

PitchSign ScoreMidiConverter::getKeySigSharpnessSignForLevel(const unsigned int level)
{
    ASSERT_E(level,<,73);
    
    PitchSign value = scoreNotesSharpness[ levelToNote7(level) ];
    
    if (value == NATURAL) return PITCH_SIGN_NONE;
    else return value;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getMiddleCLevel()
{
    return middleCLevel;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::setOctaveShift(int octaves)
{
    octave_shift = octaves;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getOctaveShift()
{
    return octave_shift;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getScoreCenterCLevel()
{
    if (octave_shift == 1) return ottavaAltaCLevel;
    else if (octave_shift == -1) return ottavaBassaCLevel;

    return middleCLevel;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::levelToNote(const int level)
{
    if (level<0 or level>=73) return -1;
    return levelToMidiNote[level];
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::resetAccidentalsForNewRender()
{
    accidentals =  false;
    for(int n=0; n<7; n++) accidentalScoreNotesSharpness[n] = -1;
    accidentalsMeasure = -1;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::noteToLevel(const Note* noteObj, PitchSign* sign)
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
            if      (noteObj->preferred_accidental_sign == SHARP) useFlats = false;
            else if (noteObj->preferred_accidental_sign == FLAT)  useFlats = true;
        }
        else
        {
            useFlats = goingInFlats();
        }

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
        else
        {
            answer_level = -1; // nothing found
        }
    }
    else if (current_type == NATURAL_ON_LEVEL)
    {
        if (sign != NULL) answer_sign = NATURAL;
        answer_level = level;
    }
    else if (current_type == DIRECT_ON_LEVEL)
    {
        if (sign != NULL) answer_sign = PITCH_SIGN_NONE;
        answer_level = level;
    }
    else
    {
        answer_level = -1; // nothing found
    }

    // accidentals
    if (sign != NULL)
    {
        if (accidentals)
        {
            const int measure = getMeasureData()->measureAtTick(noteObj->startTick);

            // when going to another measure, reset accidentals
            if (measure != accidentalsMeasure)
            {
                accidentals =  false;
                for (int n=0; n<7; n++)
                {
                    accidentalScoreNotesSharpness[n] = -1;
                }
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

// ----------------------------------------------------------------------------------------------------------
//TODO: actually return a Note7 variable
int ScoreMidiConverter::levelToNote7(const unsigned int level)
{
    int r = 7-level%7;
    if (r < 0) r+=7;
    if (r > 6) r-=7;
    return r;
}

// ----------------------------------------------------------------------------------------------------------

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
    if      ( scoreNotesSharpness[NOTE_7_C] == SHARP ) middleCNote128 -= 1;
    else if ( scoreNotesSharpness[NOTE_7_C] == FLAT )  middleCNote128 += 1;

    const int ottavaAltaCNote128 = middleCNote128 - 12;
    const int ottavaBassaCNote128 = middleCNote128 + 12;

    // do levelToMidiNote first
    Note7 note_7 = NOTE_7_A;
    int octave = 0;
    
    for (int n=0; n<73; n++)
    {
        const PitchSign sharpness = scoreNotesSharpness[note_7];

        levelToMidiNote[n] = Editor::findNotePitch( note_7, sharpness, 9 - octave); //FIXME: octave numbers are wrong in Score Editor

        ASSERT_E(levelToMidiNote[n],<,128);
        ASSERT_E(levelToMidiNote[n],>,-1);

        midiNoteToLevel[ levelToMidiNote[n] ] = n;
        midiNoteToLevel_type[ levelToMidiNote[n] ] = DIRECT_ON_LEVEL;

        // find middle C
        if ( levelToMidiNote[n] == middleCNote128 ) middleCLevel = n; // set on which line is middle C
        if ( levelToMidiNote[n] == ottavaAltaCNote128 ) ottavaAltaCLevel = n;
        if ( levelToMidiNote[n] == ottavaBassaCNote128 ) ottavaBassaCLevel = n;

        // if note is flat or sharp, also find what this line would be with a natural sign
        if (sharpness != NATURAL)
        {
            const int natural_note_on_this_line = Editor::findNotePitch( note_7, NATURAL, 9 - octave); //FIXME: octave numbers are wrong in Score Editor

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

        note_7 = Note7( int(note_7) - 1 );
        if (note_7 == NOTE_7_B) octave++;           // we went below C
        if (note_7 < NOTE_7_A)  note_7 = NOTE_7_G;  // handle warp-around
    }

}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getMidiNoteForLevelAndSign(const unsigned int level, int sharpness)
{
    if (level < 0 or level > 73) return -1;

    if      (sharpness == PITCH_SIGN_NONE) return levelToNote(level);
    else if (sharpness == NATURAL)         return levelToNaturalNote[level];
    else if (sharpness == SHARP)           return levelToNaturalNote[level]-1;
    else if (sharpness == FLAT)            return levelToNaturalNote[level]+1;
    else return -1; // shouldn't happen
}

// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------  CTOR/DTOR  -------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Ctor/dtor
#endif


ScoreEditor::ScoreEditor(Track* track) : Editor(track)
{
    g_clef = true;
    f_clef = true;

    const long scoreView = Core::getPrefsLongValue("scoreview");
    musicalNotationEnabled = scoreView == 0 or scoreView == 1;
    linearNotationEnabled  = scoreView == 0 or scoreView == 2;

    converter = new ScoreMidiConverter();

    converter->updateConversionData();

    g_clef_analyser = new ScoreAnalyser(this, converter->getScoreCenterCLevel()-5);
    f_clef_analyser = new ScoreAnalyser(this, converter->getScoreCenterCLevel()+6);

    setYStep( Y_STEP_HEIGHT );

    m_sb_position = converter->getMiddleCLevel() / 73.0;
}

// ----------------------------------------------------------------------------------------------------------

ScoreEditor::~ScoreEditor()
{
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

/*
 * These will be called by the popup menu from KeyPicker when the user changes settings
 */
void ScoreEditor::enableFClef(bool enabled)     { f_clef = enabled;       }
void ScoreEditor::enableGClef(bool enabled)     { g_clef = enabled;       }
void ScoreEditor::enableMusicalNotation(const bool enabled) { musicalNotationEnabled = enabled; }
void ScoreEditor::enableLinearNotation(const bool enabled)  { linearNotationEnabled = enabled; }

bool ScoreEditor::isGClefEnabled() const { return g_clef; }
bool ScoreEditor::isFClefEnabled() const { return f_clef; }

/** order in wich signs of the key signature appear */
const Note7 sharp_order[] = { NOTE_7_F, NOTE_7_C, NOTE_7_G, NOTE_7_D, NOTE_7_A, NOTE_7_E, NOTE_7_B };
const Note7 flat_order[]  = { NOTE_7_B, NOTE_7_E, NOTE_7_A, NOTE_7_D, NOTE_7_G, NOTE_7_C, NOTE_7_F };

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::onKeyChange(const int symbol_amount, const KeyType type)
{
    // reset key signature before beginning
    converter->setNoteSharpness(NOTE_7_A, NATURAL);
    converter->setNoteSharpness(NOTE_7_B, NATURAL);
    converter->setNoteSharpness(NOTE_7_C, NATURAL);
    converter->setNoteSharpness(NOTE_7_D, NATURAL);
    converter->setNoteSharpness(NOTE_7_E, NATURAL);
    converter->setNoteSharpness(NOTE_7_F, NATURAL);
    converter->setNoteSharpness(NOTE_7_G, NATURAL);

    if (type == KEY_TYPE_SHARPS)
    {
        for (int n=NOTE_7_A; n<symbol_amount; n++)
        {
            converter->setNoteSharpness(sharp_order[n], SHARP);
        }
    }
    else if (type == KEY_TYPE_FLATS)
    {
        for (int n=NOTE_7_A; n<symbol_amount; n++)
        {
            converter->setNoteSharpness(flat_order[n], FLAT);
        }
    }

    converter->updateConversionData();
    g_clef_analyser->setStemPivot(converter->getScoreCenterCLevel()-5);
    f_clef_analyser->setStemPivot(converter->getScoreCenterCLevel()+6);
}

// ----------------------------------------------------------------------------------------------------------

ScoreMidiConverter* ScoreEditor::getScoreMidiConverter()
{
    return converter;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::setNoteSign(const int sign, const int noteID)
{

    const int noteLevel = converter->noteToLevel(m_track->getNote(noteID));

    const int new_pitch = converter->getMidiNoteForLevelAndSign(noteLevel, sign);
    if (new_pitch == -1)
    {
        wxBell();
        return;
    }

    Note* note = m_track->getNote(noteID);
    note->pitchID = new_pitch;

    if (sign != NATURAL) note->preferred_accidental_sign = sign;

}

// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

#define LEVEL_TO_Y( lvl ) (Y_STEP_HEIGHT * (lvl) + getEditorYStart() - getYScrollInPixels()-2)

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
        return getStemX(info.getTick(), info.m_stem_type);
    }
}
using namespace EditorStemParams;
    
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::renderNote_pass1(NoteRenderInfo& renderInfo)
{
    AriaRender::lineWidth(2);

    renderInfo.setY( LEVEL_TO_Y(renderInfo.m_level) - head_radius + 4 );

    // note head
    /*
    if (renderInfo.unknown_duration)
    {
        AriaRender::primitives();
        AriaRender::character('?', renderInfo.x, renderInfo.y + 3);
    }
    else
     */
    
    RelativeXCoord relX(renderInfo.getTick(), MIDI);
    const int noteX = relX.getRelativeTo(WINDOW);
    
    if (renderInfo.m_instant_hit)
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
    else if (renderInfo.m_hollow_head)
    {
        AriaRender::images();
        if (renderInfo.m_selected) AriaRender::setImageState(AriaRender::STATE_SELECTED_NOTE);
        else                       AriaRender::setImageState(AriaRender::STATE_NOTE);
        
        noteOpen->move(noteX, renderInfo.getY());
        noteOpen->render();
    }
    else
    {
        AriaRender::images();
        if (renderInfo.m_selected) AriaRender::setImageState(AriaRender::STATE_SELECTED_NOTE);
        else                       AriaRender::setImageState(AriaRender::STATE_NOTE);
        
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
    if (renderInfo.m_level < score_from_level)
    {
        for (int lvl=score_from_level+1; lvl>renderInfo.m_level + renderInfo.m_level%2; lvl -= 2)
        {
            const int lvly = getEditorYStart() + Y_STEP_HEIGHT*lvl - head_radius - getYScrollInPixels() + 2;
            AriaRender::line(noteX-5, lvly, noteX+15, lvly);
        }
    }

    // draw small lines below score if needed
    if (renderInfo.m_level > score_to_level)
    {
        for (int lvl=score_to_level; lvl<=renderInfo.m_level - renderInfo.m_level%2 + 2; lvl += 2)
        {
            const int lvly = getEditorYStart() + Y_STEP_HEIGHT*lvl - head_radius - getYScrollInPixels() + 2;
            AriaRender::line(noteX-5, lvly, noteX+15, lvly);
        }
    }

    // draw small lines between both scores if needed
    if (g_clef and f_clef and renderInfo.m_level == middle_c_level)
    {
        const int lvly = getEditorYStart() + Y_STEP_HEIGHT*(middle_c_level+1) - head_radius -
                         getYScrollInPixels() + 2;
        AriaRender::line(noteX-5, lvly, noteX+15, lvly);
    }

    AriaRender::lineWidth(2);

    // dotted
    if (renderInfo.m_dotted)
    {
        AriaRender::color(1,1,1);
        AriaRender::pointSize(5);
        AriaRender::point(noteX + 14, renderInfo.getY() + 5);

        if (renderInfo.m_selected) AriaRender::color(1,0,0);
        else                       AriaRender::color(0,0,0);
        
        AriaRender::pointSize(3);

        AriaRender::point(noteX + 14, renderInfo.getY() + 5);
    }

    // sharpness sign
    if (renderInfo.m_sign == SHARP)
    {
        AriaRender::images();
        sharpSign->move(noteX - 5, renderInfo.getY() + head_radius);
        sharpSign->render();
        AriaRender::primitives();
    }
    else if (renderInfo.m_sign == FLAT)
    {
        AriaRender::images();
        flatSign->move(noteX - 5, renderInfo.getY() + head_radius);
        flatSign->render();
        AriaRender::primitives();
    }
    else if (renderInfo.m_sign == NATURAL)
    {
        AriaRender::images();
        naturalSign->move(noteX - 5, renderInfo.getY() + head_radius);
        naturalSign->render();
        AriaRender::primitives();
    }
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::renderNote_pass2(NoteRenderInfo& renderInfo, ScoreAnalyser* analyser)
{
    AriaRender::primitives();
    if (renderInfo.m_selected) AriaRender::color(1,0,0);
    else                       AriaRender::color(0,0,0);

    // stem
    if (renderInfo.m_draw_stem)
    {

        if (renderInfo.m_stem_type == STEM_UP or renderInfo.m_stem_type == STEM_DOWN)
        {
            int stem_from_y = LEVEL_TO_Y(renderInfo.getStemOriginLevel());
            if (renderInfo.m_stem_type == STEM_DOWN) stem_from_y += 6;
            
            AriaRender::line( getStemX(renderInfo), stem_from_y,
                              getStemX(renderInfo), LEVEL_TO_Y(analyser->getStemTo(renderInfo))   );
        }


        // flags
        if (renderInfo.m_flag_amount > 0 and not renderInfo.m_beam)
        {
            static const int stem_height = noteFlag->getImageHeight();
            const int stem_end = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
            const int flag_y_origin = (renderInfo.m_stem_type == STEM_UP ? stem_end : stem_end - stem_height );
            const int flag_x_origin = getStemX(renderInfo);
            const int flag_step = (renderInfo.m_stem_type == STEM_UP ? 7 : -7 );

            noteFlag->setFlip(false, renderInfo.m_stem_type != STEM_UP);

            AriaRender::images();
            for (int n=0; n<renderInfo.m_flag_amount; n++)
            {
                noteFlag->move(flag_x_origin , flag_y_origin + n*flag_step);
                noteFlag->render();
            }
            AriaRender::primitives();
        }
    }

    // triplet
    if (renderInfo.m_draw_triplet_sign and renderInfo.m_triplet_arc_tick_start != -1)
    {
        int triplet_arc_x_start = -1;
        int triplet_arc_x_end = -1;
        
        if (renderInfo.m_triplet_arc_tick_start != -1)
        {
             RelativeXCoord relXStart(renderInfo.m_triplet_arc_tick_start, MIDI);
             triplet_arc_x_start = relXStart.getRelativeTo(WINDOW) + 8;
        }
        if (renderInfo.m_triplet_arc_tick_end != -1)
        {
            RelativeXCoord relXEnd(renderInfo.m_triplet_arc_tick_end, MIDI);
            triplet_arc_x_end = relXEnd.getRelativeTo(WINDOW) + 8;
        }
        

        const int center_x = (triplet_arc_x_end == -1 ?
                              triplet_arc_x_start :
                              (triplet_arc_x_start + triplet_arc_x_end)/2);
        
        const int radius_x = (triplet_arc_x_end == -1 or triplet_arc_x_end == triplet_arc_x_start ?
                              10 : (triplet_arc_x_end - triplet_arc_x_start)/2);

        AriaRender::color(0,0,0);
        AriaRender::arc(center_x, LEVEL_TO_Y(renderInfo.m_triplet_arc_level) +
                        (renderInfo.m_triplet_show_above ? 0 : 10), radius_x, 10,
                        renderInfo.m_triplet_show_above);

        AriaRender::images();
        AriaRender::renderNumber(wxT("3"), center_x-2, LEVEL_TO_Y(renderInfo.m_triplet_arc_level) +
                                 (renderInfo.m_triplet_show_above ? 2 : 20));
        AriaRender::primitives();
    }

    
    // tie
    if (renderInfo.getTiedToTick() != -1)
    {
    
        const float center_tick = (renderInfo.getTiedToTick() + renderInfo.getTick())/2.0;
        
        RelativeXCoord relXFrom(renderInfo.getTiedToTick(), MIDI);
        const int x_from = relXFrom.getRelativeTo(WINDOW);
        
        RelativeXCoord relXTo(renderInfo.getTick(), MIDI);
        const int x_to = relXTo.getRelativeTo(WINDOW);
                
        const bool show_above = renderInfo.isTieUp();

        RelativeXCoord relCenter(center_tick, MIDI);
        const int center_x = relCenter.getRelativeTo(WINDOW) + 6;
        const int radius_x = (x_to - x_from)/2;
        
        const int base_y = LEVEL_TO_Y(renderInfo.getStemOriginLevel()) + head_radius;
        AriaRender::arc(center_x, base_y + (show_above ? -5 : 5), radius_x, 6, show_above);
    }

    // beam
    if (renderInfo.m_beam)
    {
        AriaRender::color(0,0,0);
        AriaRender::lineWidth(2);
        
        const int x1 = getStemX(renderInfo);
        int y1       = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
        int y2       = LEVEL_TO_Y(renderInfo.m_beam_to_level);

        const int y_diff = (renderInfo.m_stem_type == STEM_UP ? 5 : -5);

        AriaRender::lineSmooth(true);
        for (int n=0; n<renderInfo.m_flag_amount; n++)
        {
            AriaRender::line(x1, y1, getStemX(renderInfo.m_beam_to_tick, renderInfo.m_stem_type), y2);
            y1 += y_diff;
            y2 += y_diff;
        }
        AriaRender::lineSmooth(false);
    }
}

// ----------------------------------------------------------------------------------------------------------


void renderSilence(const int duration, const int tick, const int type, const int silences_y,
                   const bool triplet, const bool dotted,
                   const int dot_delta_x, const int dot_delta_y)
{
    ASSERT_E(tick,>,-1);
    RelativeXCoord relX(tick, MIDI);
    const int x = relX.getRelativeTo(WINDOW) + 5;
    
    if ( type == 1 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x,silences_y, x+15, silences_y+Y_STEP_HEIGHT);
    }
    else if ( type == 2 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x, silences_y+Y_STEP_HEIGHT, x+15, silences_y+Y_STEP_HEIGHT*2);
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

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::render(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    if (not ImageProvider::imagesLoaded()) return;

    if (head_radius == -1) head_radius = noteOpen->getImageHeight()/2;
    const int yscroll = getYScrollInPixels();

    AriaRender::beginScissors(10, getEditorYStart(), m_width - 15, 20 + m_height);

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);

    const int middle_c_level = converter->getScoreCenterCLevel();

    g_clef_analyser->clearAndPrepare();
    f_clef_analyser->clearAndPrepare();

    if (g_clef)
    {
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level-2)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll,
                                 getEditorYStart() + (middle_c_level-10)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll);
    }
    
    if (f_clef)
    {
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level+2)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll,
                                 getEditorYStart() + (middle_c_level+10)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll);
    }
    
    // ---------------------- draw notes ----------------------------
    AriaRender::color(0,0,0);
    AriaRender::pointSize(4);

    const int noteAmount = m_track->getNoteAmount();

    const int first_x_to_consider = getMeasureData()->firstPixelInMeasure(
                                        getMeasureData()->measureAtPixel(0)
                                                                          ) + 1;
    const int last_x_to_consider  = getMeasureData()->lastPixelInMeasure(
                                        getMeasureData()->measureAtPixel(m_width+15)
                                                                         );

    if (musicalNotationEnabled) converter->resetAccidentalsForNewRender();

    // render pass 1. draw linear notation if relevant, gather information and do initial rendering for
    // musical notation
    for (int n=0; n<noteAmount; n++)
    {
        PitchSign note_sign;
        const int noteLevel = converter->noteToLevel(m_track->getNote(n), &note_sign);

        if (noteLevel == -1) continue;
        const int noteLength = m_track->getNoteEndInMidiTicks(n) - m_track->getNoteStartInMidiTicks(n);
        const int tick = m_track->getNoteStartInMidiTicks(n);

        int       x1 = m_track->getNoteStartInPixels(n) - m_sequence->getXScrollInPixels() +
                       Editor::getEditorXStart();
        const int x2 = m_track->getNoteEndInPixels(n)   - m_sequence->getXScrollInPixels() +
                       Editor::getEditorXStart();

        // don't consider notes that won't be visible
        if (x2 < first_x_to_consider) continue;
        if (x1 > last_x_to_consider)  break;

        if (linearNotationEnabled)
        {
            if (musicalNotationEnabled) x1 += 8;

            if (x1 < x2) // when notes are too short to be visible, don't draw them
            {
                float volume = m_track->getNoteVolume(n)/127.0;

                // draw the quad with black border that is visible in linear notation mode
                AriaRender::primitives();
                if (m_track->isNoteSelected(n) and focus)
                {
                    AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
                }
                else
                {
                    AriaRender::color((1-volume*0.7), (1-volume*0.7), 1);
                } 

                const int y1 = noteLevel*Y_STEP_HEIGHT+1   + getEditorYStart() - getYScrollInPixels() - 1;
                const int y2 = (noteLevel+1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels() - 1;

                if (musicalNotationEnabled)
                {
                    AriaRender::bordered_rect_no_start(x1+1, y1, x2-1, y2);
                }
                else
                {
                    AriaRender::bordered_rect(x1+1, y1, x2-1, y2);
                }

            }

            // when musical notation is disabled, we need to render the sharpness sign here
            // (when it's activated, it's done by the note drawing code. When note drawing code
            // is disabled, we need to do it ourselves).
            if (not musicalNotationEnabled)
            {
                AriaRender::images();

                if (note_sign == SHARP)
                {
                    sharpSign->move(x1 - 5,
                                    noteLevel*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels());
                    sharpSign->render();
                }
                else if (note_sign == FLAT)
                {
                    flatSign->move(x1 - 5,
                                   noteLevel*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels());
                    flatSign->render();
                }
                else if (note_sign == NATURAL)
                {
                    naturalSign->move(x1 - 5,
                                      noteLevel*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels());
                    naturalSign->render();
                }

                AriaRender::primitives();
            }
        }// end if linear

        if (musicalNotationEnabled)
        {
            // build visible notes vector with initial info in it
            NoteRenderInfo currentNote(tick, noteLevel, noteLength, note_sign,
                                       m_track->isNoteSelected(n), m_track->getNotePitchID(n));

            // add note to either G clef score or F clef score
            if (g_clef and not f_clef)
            {
                g_clef_analyser->addToVector(currentNote);
            }
            else if (f_clef and not g_clef)
            {
                f_clef_analyser->addToVector(currentNote);
            }
            else if (f_clef and g_clef)
            {
                const int middleC = converter->getScoreCenterCLevel();
                if (noteLevel < middleC)
                {
                    g_clef_analyser->addToVector(currentNote);
                }
                else if (noteLevel > middleC)
                {
                    f_clef_analyser->addToVector(currentNote);
                }
                else
                {
                    // note is exactly on middle C... do our best to
                    // guess on which clef to put this note
                    // we'll check nearby notes in case it can help us'
                    int check_note = -1;
                    if      (n > 0)            check_note = n-1;
                    else if (n+1 < noteAmount) check_note = n+1;

                    if (check_note != -1)
                    {
                        const int checkNoteLevel = converter->noteToLevel( m_track->getNote(check_note), NULL );
                        
                        if (checkNoteLevel > middleC)  f_clef_analyser->addToVector(currentNote);
                        else                           g_clef_analyser->addToVector(currentNote);
                    }
                    else
                    {
                        g_clef_analyser->addToVector(currentNote);
                    }
                    
                } // end if note on middle C
            } // end if both G and F clefs
        } // end if musical notation enabled
    } // next note


    // render musical notation if enabled
    if (musicalNotationEnabled)
    {
        if (g_clef)
        {
            const int silences_y = getEditorYStart() + Y_STEP_HEIGHT*(converter->getScoreCenterCLevel()-8) - getYScrollInPixels() + 1;
            renderScore(g_clef_analyser, silences_y);
        }

        if (f_clef)
        {
            const int silences_y = getEditorYStart() + Y_STEP_HEIGHT*(converter->getScoreCenterCLevel()+4) - getYScrollInPixels() + 1;
            renderScore(f_clef_analyser, silences_y);
        }
    }


    AriaRender::lineWidth(1);
    // ------------------------- mouse drag (preview) ------------------------

    AriaRender::primitives();

    if (not m_clicked_on_note and m_mouse_is_in_editor)
    {
        // selection
        if (m_selecting)
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
                       m_sequence->getXScrollInMidiTicks()) * m_sequence->getZoom()
                      );
            int preview_x2=
                (int)(
                      (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) ) -
                       m_sequence->getXScrollInMidiTicks()) * m_sequence->getZoom()
                      );

            if (not (preview_x1 < 0 or preview_x2 < 0) and preview_x2 > preview_x1)
            {

                const int y_base = ((mousey_initial - getEditorYStart() +
                                     getYScrollInPixels())/Y_STEP_HEIGHT)*Y_STEP_HEIGHT;
                
                const int y_add = getEditorYStart() - getYScrollInPixels();

                AriaRender::rect(preview_x1+Editor::getEditorXStart(), y_base-2 + y_add,
                                 preview_x2+Editor::getEditorXStart(), y_base+Y_STEP_HEIGHT+1 + y_add);

            }

        }// end if selection or addition

    }


    // ------------------------- move note (preview) -----------------------
    AriaRender::primitives();
    if (m_clicked_on_note)
    {
        AriaRender::color(1, 0.85, 0, 0.5);

        int x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
        int y_difference = mousey_current - mousey_initial;

        const int x_pixel_move = (int)( snapMidiTickToGrid(x_difference) * m_sequence->getZoom() );
        const int y_step_move = (int)round( (float)y_difference/ (float)Y_STEP_HEIGHT );

        // move a single note
        if (m_last_clicked_note!=-1)
        {
            int x1       = m_track->getNoteStartInPixels(m_last_clicked_note) -
                           m_sequence->getXScrollInPixels() + Editor::getEditorXStart();
            const int x2 = m_track->getNoteEndInPixels(m_last_clicked_note) -
                           m_sequence->getXScrollInPixels() + Editor::getEditorXStart();
            
            const int noteLevel = converter->noteToLevel( m_track->getNote(m_last_clicked_note) );

            AriaRender::rect(x1 + 1 + x_pixel_move,
                             (noteLevel+y_step_move)*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels()-1,
                             x2 - 1 + x_pixel_move,
                             (noteLevel+1+y_step_move)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels()-1);

        }
        else
        {
            // move a bunch of notes

            for (int n=0; n<m_track->getNoteAmount(); n++)
            {
                if (not m_track->isNoteSelected(n)) continue;

                int x1 = m_track->getNoteStartInPixels(n) - m_sequence->getXScrollInPixels() +
                         Editor::getEditorXStart();
                
                const int x2 = m_track->getNoteEndInPixels(n) - m_sequence->getXScrollInPixels() +
                               Editor::getEditorXStart();

                const int noteLevel = converter->noteToLevel(m_track->getNote(n));

                AriaRender::rect(x1 + 1 + x_pixel_move,
                                 (noteLevel+y_step_move)*Y_STEP_HEIGHT+1 + getEditorYStart() -
                                    getYScrollInPixels()-1,
                                 x2 - 1 + x_pixel_move,
                                 (noteLevel+1+y_step_move)*Y_STEP_HEIGHT + getEditorYStart() -
                                    getYScrollInPixels()-1);

            }//next

        }

    }// end if clicked on note


    // --------------------------- grey left part -----------------------------
    if (not focus) AriaRender::color(0.4, 0.4, 0.4);
    else          AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0,                           getEditorYStart(),
                     Editor::getEditorXStart()-3, getYEnd());

    // ------------------------------- draw keys and horizontal lines -------------------------------

    AriaRender::color(0,0,0);

    if (g_clef)
    {
        AriaRender::primitives();

        // draw horizontal score lines
        for (int n = middle_c_level - 10 ; n < middle_c_level; n+=2)
        {
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
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
        for (int n = min_level_with_signs; n < max_level_with_signs; n++)
        {
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
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
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
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
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
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
        const int clef_y = getEditorYStart() + (middle_c_level-6)*Y_STEP_HEIGHT -  yscroll + 5;
        clefG_drawable->move(Editor::getEditorXStart() - 55, clef_y);
        clefG_drawable->render();
        /*
        AriaRender::color(0, 0, 0);
        AriaRender::primitives();
        if (converter->getOctaveShift() == -1) AriaRender::text("8", Editor::getEditorXStart() - 30, clef_y-10 );
        else if (converter->getOctaveShift() == 1) AriaRender::text("8", Editor::getEditorXStart() - 30, clef_y );
         */
    }
    if (f_clef)
    {
        const int clef_y = getEditorYStart() + (middle_c_level+4)*Y_STEP_HEIGHT -  yscroll + 5;
        clefF_drawable->move(Editor::getEditorXStart() - 65, clef_y);
        clefF_drawable->render();
        /*
        AriaRender::color(0, 0, 0);
        AriaRender::primitives();
        if (converter->getOctaveShift() == -1) AriaRender::text("8", Editor::getEditorXStart() - 30, clef_y-10 );
        else if (converter->getOctaveShift() == 1) AriaRender::text("8", Editor::getEditorXStart() - 30, clef_y+10 );
         */
    }

    // ---------------------------- scrollbar -----------------------
    if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::setImageState(AriaRender::STATE_NORMAL);
    AriaRender::endScissors();
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::renderScore(ScoreAnalyser* analyser, const int silences_y)
{
    int visibleNoteAmount = analyser->noteRenderInfo.size();

    // first note rendering pass
    for(int i=0; i<visibleNoteAmount; i++) renderNote_pass1( analyser->noteRenderInfo[i] );

    // render silences
    const unsigned int first_visible_measure = getMeasureData()->measureAtPixel( Editor::getEditorXStart() );
    const unsigned int last_visible_measure = getMeasureData()->measureAtPixel( getXEnd() );
    
    SilenceAnalyser::findSilences( &renderSilence, analyser, first_visible_measure, last_visible_measure, silences_y );

    // ------------------------- second note rendering pass -------------------

    // analyse notes to know how to build the score
    analyser->analyseNoteInfo();

    // triplet signs, tied notes, flags and beams
    visibleNoteAmount = analyser->noteRenderInfo.size();
    for(int i=0; i<visibleNoteAmount; i++)
    {
        ASSERT_E(i,<,(int)analyser->noteRenderInfo.size());
        renderNote_pass2(analyser->noteRenderInfo[i], analyser);
    }

}


// ----------------------------------------------------------------------------------------------------------
// -----------------------------------------    EVENT METHODS   ---------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Events
#endif

void ScoreEditor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseHeldDown(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
                              RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseExited(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
    if (x.getRelativeTo(EDITOR)<-30 and x.getRelativeTo(WINDOW)>15 and y>getEditorYStart())
    {
        KeyPicker* picker = Core::getKeyPicker();
        
        picker->setParent( m_track );
        picker->setChecks(musicalNotationEnabled, linearNotationEnabled, f_clef, g_clef,
                          converter->getOctaveShift());
        
        Display::popupMenu( picker, x.getRelativeTo(WINDOW),y);
        return;
    }
    // user clicked on a note on the staff
    else if (x.getRelativeTo(EDITOR)<0 and x.getRelativeTo(EDITOR)>-30 and y>getEditorYStart())
    {
        const int level = getLevelAtY(y-Y_STEP_HEIGHT/2);
        const int pitchID = converter->levelToNote(level);
        if (pitchID != -1)
        {
            PlatformMidiManager::get()->playNote(131 - pitchID, m_default_volume, 500 /* duration */, 0,
                                                 m_track->getInstrument() );
        }
        return;
    }
    
    Editor::mouseDown(x, y);
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseDrag(mousex_current, mousey_current, mousex_initial, mousey_initial);

}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
                          RelativeXCoord mousex_initial, int mousey_initial)
{
    Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);

}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::rightClick(RelativeXCoord x, int y)
{
    Editor::rightClick(x, y);

}

// ----------------------------------------------------------------------------------------------------------
// ---------------------------------------    EDITOR METHODS      -------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Editor methods
#endif


int ScoreEditor::getYScrollInPixels()
{
    // check if visible area is large enough to display everything
    if (73*Y_STEP_HEIGHT <= m_height)
    {
        useVerticalScrollbar(false);
        return 0;
    }
    else
    {
        useVerticalScrollbar(true);
    }

    return (int)( m_sb_position*(73*Y_STEP_HEIGHT - m_height) );
}

// ----------------------------------------------------------------------------------------------------------

NoteSearchResult ScoreEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int head_radius = noteOpen->getImageHeight()/2;
    const int noteAmount  = m_track->getNoteAmount();
    const int mx          = x.getRelativeTo(WINDOW);

    for (int n=0; n<noteAmount; n++)
    {

        //const int notePitch = track->getNotePitchID(n);
        const int noteLevel = converter->noteToLevel( m_track->getNote(n) );
        if (noteLevel == -1) continue;
        
        const int note_y  = getEditorYStart() + Y_STEP_HEIGHT*noteLevel - head_radius -
                            getYScrollInPixels() + 2;
        const int note_x  = Editor::getEditorXStart() + m_track->getNoteStartInPixels(n) -
                            m_sequence->getXScrollInPixels();
        const int note_x2 = Editor::getEditorXStart() + m_track->getNoteEndInPixels(n) -
                            m_sequence->getXScrollInPixels();

        if (linearNotationEnabled)
        {
            if (mx < note_x2 and mx > note_x and y < note_y+11 and y > note_y)
            {
                noteID = n;

                if (m_track->isNoteSelected(n) and not Display::isSelectLessPressed())
                {
                    // clicked on a selected note
                    return FOUND_SELECTED_NOTE;
                }
                else
                {
                    return FOUND_NOTE;
                }
            }
        }
        else if (musicalNotationEnabled)
        {

            if (mx<note_x+11 and mx>note_x and y < note_y+11 and y > note_y+2)
            {
                noteID = n;

                if (m_track->isNoteSelected(n) and not Display::isSelectLessPressed())
                {
                    // clicked on a selected note
                    return FOUND_SELECTED_NOTE;
                }
                else
                {
                    return FOUND_NOTE;
                }
            }

        }
        else
        {
            return FOUND_NOTHING;
        }

    }

    return FOUND_NOTHING;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::noteClicked(const int id)
{
    m_track->selectNote(ALL_NOTES, false);
    m_track->selectNote(id, true);
    m_track->playNote(id);
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    const int level = getLevelAtY(mouseY);
    if (level < 0 or level >= 73) return;
    
    const int note = converter->levelToNote(level);
    if (note == -1) return;

    m_track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, m_default_volume ) );
}

// ----------------------------------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                    RelativeXCoord& mousex_initial, int mousey_initial)
{
    const int head_radius = noteOpen->getImageHeight()/2;
    const int noteAmount  = m_track->getNoteAmount();
    
    const int mxc = mousex_current.getRelativeTo(WINDOW);
    const int mxi = mousex_initial.getRelativeTo(WINDOW);

    for (int n=0; n<noteAmount; n++)
    {
        const int noteLevel = converter->noteToLevel( m_track->getNote(n) );
        if (noteLevel == -1) continue;
        
        const int note_y = getEditorYStart() + Y_STEP_HEIGHT*noteLevel - head_radius - getYScrollInPixels() + 2;
        const int note_x = Editor::getEditorXStart() + m_track->getNoteStartInPixels(n) -
                           m_sequence->getXScrollInPixels();

        if (std::min(mxc, mxi) < note_x and std::max(mxc,mxi)>note_x and
            std::min(mousey_current, mousey_initial) < note_y and
            std::max(mousey_current, mousey_initial) > note_y)
        {
            m_track->selectNote(n, true);
        }
        else
        {
            m_track->selectNote(n, false);
        }

    }
}

