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
#include "Actions/ShiftBySemiTone.h"
#include "Analysers/ScoreAnalyser.h"
#include "Editors/ScoreEditor.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/ImageProvider.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/KeyPicker.h"
#include "PreferencesData.h"
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

ScoreMidiConverter::ScoreMidiConverter(GraphicalSequence* parent)
{
    m_sequence = parent;

    for (int n=0; n<7; n++) m_score_notes_sharpness[n] = NATURAL;

    m_accidentals = false;
    for (int n=0; n<128; n++) m_accidental_score_notes_sharpness[n] = -1; // FIXME - why -1 and not PITCH_SIGN_NONE?
    m_accidentals_measure = -1;

    m_going_in_sharps = false;
    m_going_in_flats = false;
    m_octave_shift = 0;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::setNoteSharpness(Note7 note, PitchSign sharpness)
{

    m_score_notes_sharpness[note] = sharpness;

    if (sharpness == SHARP)
    {
        m_going_in_sharps = true;
        m_going_in_flats = false;
    }
    else if (sharpness == FLAT)
    {
        m_going_in_sharps = false;
        m_going_in_flats = true;
    }
}

// ----------------------------------------------------------------------------------------------------------

PitchSign ScoreMidiConverter::getKeySigSharpnessSignForLevel(const unsigned int level) const
{
    ASSERT_E(level,<,73);
    
    PitchSign value = m_score_notes_sharpness[ levelToNote7(level) ];
    
    if (value == NATURAL) return PITCH_SIGN_NONE;
    else                  return value;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::setOctaveShift(int octaves)
{
    m_octave_shift = octaves;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getScoreCenterCLevel() const
{
    if      (m_octave_shift == 1)  return m_ottava_alta_C_level;
    else if (m_octave_shift == -1) return m_ottava_bassa_C_level;

    return m_middle_C_level;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::levelToNote(const int level)
{
    if (level < 0 or level >= 73) return -1;
    return m_level_to_midi_note[level];
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::resetAccidentalsForNewRender()
{
    m_accidentals =  false;
    
    for (int n=0; n<128; n++) m_accidental_score_notes_sharpness[n] = -1;
    m_accidentals_measure = -1;
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::noteToLevel(const Note* noteObj, PitchSign* sign)
{
    const int note = noteObj->getPitchID();
    return noteToLevel(noteObj, note, sign);
}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::noteToLevel(const Note* noteObj, const int note, PitchSign* sign)
{
    const int level = m_midi_note_to_level[note];
    const NoteToLevelType current_type = m_midi_note_to_level_type[note];

    PitchSign answer_sign = PITCH_SIGN_NONE;
    int answer_level = -1;

    if (current_type == SHARP_OR_FLAT)
    {
        // decide whether to use a flat or a sharp to display this note
        // first check if there's a user-specified sign. otherwise pick default (quite arbitrarly)
        bool useFlats = false;
        if (noteObj != NULL and noteObj->getPreferredAccidentalSign() != -1)
        {
            if      (noteObj->getPreferredAccidentalSign() == SHARP) useFlats = false;
            else if (noteObj->getPreferredAccidentalSign() == FLAT)  useFlats = true;
        }
        else
        {
            useFlats = goingInFlats();
        }

        if (useFlats and (note-1)>0)
        {
            if (sign != NULL) answer_sign = FLAT;
            answer_level = m_midi_note_to_level[note-1];
        }
        else if (note < 127)
        {
            if (sign != NULL) answer_sign = SHARP;
            answer_level = m_midi_note_to_level[note+1];
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
        if (m_accidentals)
        {
            const int measure = m_sequence->getModel()->getMeasureData()->measureAtTick(noteObj->getTick());

            // when going to another measure, reset accidentals
            if (measure != m_accidentals_measure)
            {
                m_accidentals = false;
                for (int n=0; n<128; n++)
                {
                    m_accidental_score_notes_sharpness[n] = -1;
                }
                m_accidentals_measure = measure;
            }
            else
            {
                // we are not going in another measure, apply accidental to current note
                const int current_accidental = m_accidental_score_notes_sharpness[ levelToNote(answer_level) ];
                if (current_accidental != -1)
                {
                    // the current note continues using the same accidental, no change
                    if (current_accidental == answer_sign) answer_sign = PITCH_SIGN_NONE;
                    // the current note does NOT continue using the same accidental, display another sign
                    else if (current_accidental != answer_sign and
                            (answer_sign == NATURAL or answer_sign == PITCH_SIGN_NONE))
                    {
                        const int accidentalType = m_accidental_score_notes_sharpness[ levelToNote(answer_level) ];
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
        if (answer_sign != PITCH_SIGN_NONE and noteObj != NULL)
        {
            m_accidentals = true;
            const int measure = m_sequence->getModel()->getMeasureData()->measureAtTick(noteObj->getTick());
            m_accidentals_measure = measure;

            m_accidental_score_notes_sharpness[ levelToNote(answer_level) ] = answer_sign;

            // remove accidental if no more needed
            if ((m_score_notes_sharpness[ levelToNote7(answer_level) ] == NATURAL and answer_sign == NATURAL) or
               m_score_notes_sharpness[ levelToNote7(answer_level) ] == answer_sign)
            {
                m_accidental_score_notes_sharpness[ levelToNote(answer_level) ] = -1;
            }
        }
    }

    if (sign != NULL) *sign = answer_sign;
    return answer_level;
}

// ----------------------------------------------------------------------------------------------------------
//TODO: actually return a Note7 variable
int ScoreMidiConverter::levelToNote7(const unsigned int level) const
{
    int r = 7 - level%7;
    
    if (r < 0) r += 7;
    if (r > 6) r -= 7;
    return r;
}

// ----------------------------------------------------------------------------------------------------------

void ScoreMidiConverter::updateConversionData()
{

    // FIXME - mess

    // reset all
    for (int n=0; n<128; n++)
    {
        m_midi_note_to_level[n] = -1;
        m_midi_note_to_level_type[n] = SHARP_OR_FLAT;
    }

    m_middle_C_level = -1;
    m_ottava_alta_C_level = -1;
    m_ottava_bassa_C_level = -1;

    int middleCNote128 = 71;
    if      ( m_score_notes_sharpness[NOTE_7_C] == SHARP ) middleCNote128 -= 1;
    else if ( m_score_notes_sharpness[NOTE_7_C] == FLAT )  middleCNote128 += 1;

    const int ottavaAltaCNote128 = middleCNote128 - 12;
    const int ottavaBassaCNote128 = middleCNote128 + 12;

    // do m_level_to_midi_note first
    Note7 note_7 = NOTE_7_A;
    int octave = 0;
    
    for (int n=0; n<73; n++)
    {
        const PitchSign sharpness = m_score_notes_sharpness[note_7];

        m_level_to_midi_note[n] = Note::findNotePitch( note_7, sharpness, 9 - octave); //FIXME: octave numbers are wrong in Score Editor

        ASSERT_E(m_level_to_midi_note[n],<,128);
        ASSERT_E(m_level_to_midi_note[n],>,-1);

        m_midi_note_to_level[ m_level_to_midi_note[n] ] = n;
        m_midi_note_to_level_type[ m_level_to_midi_note[n] ] = DIRECT_ON_LEVEL;

        // find middle C
        if (m_level_to_midi_note[n] == middleCNote128)      m_middle_C_level = n; // set on which line is middle C
        if (m_level_to_midi_note[n] == ottavaAltaCNote128)  m_ottava_alta_C_level = n;
        if (m_level_to_midi_note[n] == ottavaBassaCNote128) m_ottava_bassa_C_level = n;

        // if note is flat or sharp, also find what this line would be with a natural sign
        if (sharpness != NATURAL)
        {
            //FIXME: octave numbers are wrong in Score Editor
            const int natural_note_on_this_line = Note::findNotePitch( note_7, NATURAL, 9 - octave);

            m_level_to_natural_note[n] = natural_note_on_this_line;

            // only use natural signs if this note cannot be displayed without natural sign on another line
            // in wich case 'm_midi_note_to_level' will have been set (or will be overwritten later)
            if (m_midi_note_to_level[ natural_note_on_this_line ] == -1)
            {
                m_midi_note_to_level[ natural_note_on_this_line ] = n;
                m_midi_note_to_level_type[ natural_note_on_this_line ] = NATURAL_ON_LEVEL;
            }
        }
        else
        {
            m_level_to_natural_note[n] = m_level_to_midi_note[n];
        }

        note_7 = Note7( int(note_7) - 1 );
        if (note_7 == NOTE_7_B) octave++;           // we went below C
        if (note_7 < NOTE_7_A)  note_7 = NOTE_7_G;  // handle warp-around
    }

}

// ----------------------------------------------------------------------------------------------------------

int ScoreMidiConverter::getMidiNoteForLevelAndSign(const unsigned int level, int sharpness)
{
    if (level > 73) return -1;

    if      (sharpness == PITCH_SIGN_NONE) return levelToNote(level);
    else if (sharpness == NATURAL)         return m_level_to_natural_note[level];
    else if (sharpness == SHARP)           return m_level_to_natural_note[level]-1;
    else if (sharpness == FLAT)            return m_level_to_natural_note[level]+1;
    else return -1; // shouldn't happen
}

// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------  CTOR/DTOR  -------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Ctor/dtor
#endif


ScoreEditor::ScoreEditor(GraphicalTrack* track) : Editor(track)
{
    m_g_clef = true;
    m_f_clef = true;
    m_clicked_note = -1;

    const int scoreView = PreferencesData::getInstance()->getIntValue("scoreview");
    m_musical_notation_enabled = (scoreView == 0 or scoreView == 1);
    m_linear_notation_enabled  = (scoreView == 0 or scoreView == 2);

    m_converter = new ScoreMidiConverter(m_gsequence);

    m_converter->updateConversionData();

    m_g_clef_analyser = new ScoreAnalyser(this, m_converter->getScoreCenterCLevel()-5);
    m_f_clef_analyser = new ScoreAnalyser(this, m_converter->getScoreCenterCLevel()+6);

    setYStep( Y_STEP_HEIGHT );

    m_sb_position = m_converter->getMiddleCLevel() / 73.0;
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
void ScoreEditor::enableFClef(bool enabled)     { m_f_clef = enabled;       }
void ScoreEditor::enableGClef(bool enabled)     { m_g_clef = enabled;       }
void ScoreEditor::enableMusicalNotation(const bool enabled) { m_musical_notation_enabled = enabled; }
void ScoreEditor::enableLinearNotation(const bool enabled)  { m_linear_notation_enabled = enabled; }

/** order in wich signs of the key signature appear */
const Note7 sharp_order[] = { NOTE_7_F, NOTE_7_C, NOTE_7_G, NOTE_7_D, NOTE_7_A, NOTE_7_E, NOTE_7_B };
const Note7 flat_order[]  = { NOTE_7_B, NOTE_7_E, NOTE_7_A, NOTE_7_D, NOTE_7_G, NOTE_7_C, NOTE_7_F };

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::onKeyChange(const int symbol_amount, const KeyType type)
{
    // reset key signature before beginning
    m_converter->setNoteSharpness(NOTE_7_A, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_B, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_C, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_D, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_E, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_F, NATURAL);
    m_converter->setNoteSharpness(NOTE_7_G, NATURAL);

    if (type == KEY_TYPE_SHARPS)
    {
        for (int n=NOTE_7_A; n<symbol_amount; n++)
        {
            m_converter->setNoteSharpness(sharp_order[n], SHARP);
        }
    }
    else if (type == KEY_TYPE_FLATS)
    {
        for (int n=NOTE_7_A; n<symbol_amount; n++)
        {
            m_converter->setNoteSharpness(flat_order[n], FLAT);
        }
    }

    m_converter->updateConversionData();
    m_g_clef_analyser->setStemPivot(m_converter->getScoreCenterCLevel() - 5);
    m_f_clef_analyser->setStemPivot(m_converter->getScoreCenterCLevel() + 6);
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::setNoteSign(const int sign, const int noteID)
{
    const int noteLevel = m_converter->noteToLevel(m_track->getNote(noteID));

    const int new_pitch = m_converter->getMidiNoteForLevelAndSign(noteLevel, sign);
    if (new_pitch == -1)
    {
        wxBell();
        return;
    }

    Note* note = m_track->getNote(noteID);
    note->setPitchID( new_pitch );

    if (sign != NATURAL) note->setPreferredAccidentalSign( sign );

}

// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Rendering
#endif

#define LEVEL_TO_Y( lvl ) (Y_STEP_HEIGHT * (lvl) + getEditorYStart() - getYScrollInPixels()-2)

namespace EditorStemParams
{
    int   g_stem_up_x_offset   = 9;
    int   g_stem_down_x_offset = 1;

    int getStemX(const int tick, const STEM stem_type, GraphicalSequence* gseq)
    {
        RelativeXCoord relX(tick, MIDI, gseq);
        const int noteX = relX.getRelativeTo(WINDOW);
        
        if      (stem_type == STEM_UP)   return (noteX + g_stem_up_x_offset);
        else if (stem_type == STEM_DOWN) return (noteX + g_stem_down_x_offset);
        else return -1;
    }
    
    int getStemX(const NoteRenderInfo& info, GraphicalSequence* gseq)
    {
        return getStemX(info.getTick(), info.m_stem_type, gseq);
    }
}
using namespace EditorStemParams;
    
// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::renderNote_pass1(NoteRenderInfo& renderInfo, const AriaColor& baseColor)
{
    AriaRender::lineWidth(2);

    renderInfo.setY( LEVEL_TO_Y(renderInfo.getLevel()) - head_radius + 4 );

    // note head
    /*
    if (renderInfo.unknown_duration)
    {
        AriaRender::primitives();
        AriaRender::character('?', renderInfo.x, renderInfo.y + 3);
    }
    else
     */
    
    RelativeXCoord relX(renderInfo.getTick(), MIDI, m_gsequence);
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
    // to the note so that the amount of levels is visible (ledger lines)
    AriaRender::primitives();
    AriaRender::color(0,0,0);
    AriaRender::lineWidth(1);
    const int middle_c_level = m_converter->getScoreCenterCLevel();

    // set min and max level where small line is not needed
    int score_from_level = 0;
    if      (m_g_clef) score_from_level = middle_c_level - 10;
    else if (m_f_clef) score_from_level = middle_c_level + 2;

    int score_to_level = 999;
    if      (m_f_clef) score_to_level = middle_c_level + 11;
    else if (m_g_clef) score_to_level = middle_c_level - 1;
    
    // draw small lines above score if needed
    if (renderInfo.getLevel() < score_from_level)
    {
        for (int lvl=score_from_level-1; lvl>renderInfo.getLevel(); lvl --)
        {
            if ((score_from_level - lvl) % 2 == 1)
            {
                const int lvly = getEditorYStart() + Y_STEP_HEIGHT*lvl - head_radius - getYScrollInPixels() + 2;
                AriaRender::line(noteX-5, lvly, noteX+15, lvly);
            }
        }
    }

    // draw small lines below score if needed
    if (renderInfo.getLevel() > score_to_level)
    {
        for (int lvl=score_to_level; lvl<=renderInfo.getLevel()+1; lvl++)
        {
            if ((lvl - score_to_level) % 2 == 0)
            {
                const int lvly = getEditorYStart() + Y_STEP_HEIGHT*lvl - head_radius - getYScrollInPixels() + 2;
                AriaRender::line(noteX-5, lvly, noteX+15, lvly);
            }
        }
    }

    // draw small lines between both scores if needed
    if (m_g_clef and m_f_clef and renderInfo.getLevel() == middle_c_level)
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
        else                       AriaRender::color(baseColor.r, baseColor.g, baseColor.b);
        
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

void ScoreEditor::renderNote_pass2(NoteRenderInfo& renderInfo, ScoreAnalyser* analyser, const AriaColor& baseColor)
{
    AriaRender::primitives();
    if (renderInfo.m_selected) AriaRender::color(1,0,0);
    else                       AriaRender::color(baseColor.r, baseColor.g, baseColor.b);

    // stem
    if (renderInfo.m_draw_stem)
    {

        if (renderInfo.m_stem_type == STEM_UP or renderInfo.m_stem_type == STEM_DOWN)
        {
            int stem_from_y = LEVEL_TO_Y(renderInfo.getStemOriginLevel());
            if (renderInfo.m_stem_type == STEM_DOWN) stem_from_y += 6;
            
            const int stemx = getStemX(renderInfo, m_gsequence);
                        
            AriaRender::line(stemx, stem_from_y,
                             stemx, LEVEL_TO_Y(analyser->getStemTo(renderInfo))   );
        }


        // flags
        AriaRender::images();
        if (renderInfo.m_selected) AriaRender::setImageState(AriaRender::STATE_SELECTED_NOTE);
        else                       AriaRender::setImageState(AriaRender::STATE_NOTE);
        
        if (renderInfo.m_flag_amount > 0 and not renderInfo.m_beam)
        {
            static const int stem_height = noteFlag->getImageHeight();
            const int stem_end = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
            const int flag_y_origin = (renderInfo.m_stem_type == STEM_UP ? stem_end : stem_end - stem_height );
            const int flag_x_origin = getStemX(renderInfo, m_gsequence);
            const int flag_step = (renderInfo.m_stem_type == STEM_UP ? 7 : -7 );

            noteFlag->setFlip(false, renderInfo.m_stem_type != STEM_UP);

            
            for (int n=0; n<renderInfo.m_flag_amount; n++)
            {
                noteFlag->move(flag_x_origin , flag_y_origin + n*flag_step);
                noteFlag->render();
            }
        }
        AriaRender::primitives();
    }

    // triplet
    if (renderInfo.m_draw_triplet_sign and renderInfo.m_triplet_arc_tick_start != -1)
    {
        int triplet_arc_x_start = -1;
        int triplet_arc_x_end = -1;
        
        if (renderInfo.m_triplet_arc_tick_start != -1)
        {
             RelativeXCoord relXStart(renderInfo.m_triplet_arc_tick_start, MIDI, m_gsequence);
             triplet_arc_x_start = relXStart.getRelativeTo(WINDOW) + 8;
        }
        if (renderInfo.m_triplet_arc_tick_end != -1)
        {
            RelativeXCoord relXEnd(renderInfo.m_triplet_arc_tick_end, MIDI, m_gsequence);
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
        AriaRender::renderNumber("3", center_x-2, LEVEL_TO_Y(renderInfo.m_triplet_arc_level) +
                                 (renderInfo.m_triplet_show_above ? 2 : 20));
        AriaRender::primitives();
    }

    
    // tie
    if (renderInfo.getTiedToTick() != -1)
    {
    
        const float center_tick = (renderInfo.getTiedToTick() + renderInfo.getTick())/2.0;
        
        RelativeXCoord relXFrom(renderInfo.getTiedToTick(), MIDI, m_gsequence);
        const int x_from = relXFrom.getRelativeTo(WINDOW);
        
        RelativeXCoord relXTo(renderInfo.getTick(), MIDI, m_gsequence);
        const int x_to = relXTo.getRelativeTo(WINDOW);
                
        const bool show_above = renderInfo.isTieUp();

        RelativeXCoord relCenter(center_tick, MIDI, m_gsequence);
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
        
        const int x1 = getStemX(renderInfo, m_gsequence);
        int y1       = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
        int y2       = LEVEL_TO_Y(renderInfo.m_beam_to_level);

        const int y_diff = (renderInfo.m_stem_type == STEM_UP ? 5 : -5);

        AriaRender::lineSmooth(true);
        const int count = renderInfo.m_flag_amount;
        for (int n=0; n<count; n++)
        {
            const int xto = getStemX(renderInfo.m_beam_to_tick, renderInfo.m_stem_type, m_gsequence);
            AriaRender::line(x1, y1, xto, y2);
            y1 += y_diff;
            y2 += y_diff;
        }
        AriaRender::lineSmooth(false);
    }
}

// ----------------------------------------------------------------------------------------------------------


void renderSilence(const Sequence* seq, const int duration, const int tick, const int type,
                   const int silences_y, const bool triplet, const bool dotted,
                   const int dot_delta_x, const int dot_delta_y, void* userData)
{
    ASSERT_E(tick,>,-1);
    
    GraphicalSequence* gseq = (GraphicalSequence*)userData;
    
    RelativeXCoord relX(tick, MIDI, gseq);
    const int x = relX.getRelativeTo(WINDOW) + 5;
    
    if ( type == 1 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x, silences_y + 1, x + 15, silences_y + Y_STEP_HEIGHT + 2);
    }
    else if ( type == 2 )
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x, silences_y + Y_STEP_HEIGHT + 1, x + 15, silences_y + Y_STEP_HEIGHT*2 + 1);
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
        AriaRender::renderNumber("3", x+3, silences_y+31);
        AriaRender::primitives();
    }

}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::render(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    TrackRenderContext ctx;
    AriaColor ariaColor;
    bool renderSilences;
    
    if (not ImageProvider::imagesLoaded()) return;

    if (head_radius == -1) head_radius = noteOpen->getImageHeight()/2;
    const int yscroll = getYScrollInPixels();

    AriaRender::beginScissors(LEFT_EDGE_X, getEditorYStart(), m_width - RIGHT_SCISSOR, m_height);

    // white background
    AriaRender::primitives();
    
    // TODO : fixme 
    if (m_track->isPlayed())
    {
        AriaRender::color(1,1,1);
    }
    else
    {
        AriaRender::color(0.8,0.8,0.8);
    }
    

    const int middle_c_level = m_converter->getScoreCenterCLevel();


    if (m_g_clef)
    {
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level-2)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll,
                                 getEditorYStart() + (middle_c_level-10)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll);
    }
    
    if (m_f_clef)
    {
        drawVerticalMeasureLines(getEditorYStart() + (middle_c_level+2)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll,
                                 getEditorYStart() + (middle_c_level+10)*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll);
    }
    
    // ---------------------- draw notes ----------------------------
    AriaRender::color(0,0,0);
    AriaRender::pointSize(4);

    
    if (m_musical_notation_enabled) m_converter->resetAccidentalsForNewRender();


    MeasureBar* mb = m_gsequence->getMeasureBar();
    const bool mouseValid = (mousex_current.isValid() and mousex_initial.isValid());
    const int mxc = (mouseValid ? mousex_current.getRelativeTo(WINDOW) : -1);
    const int mxi = (mouseValid ? mousex_initial.getRelativeTo(WINDOW) : -1);
    
    ctx.first_x_to_consider = mb->firstPixelInMeasure( mb->measureAtPixel(0) ) + 1;
    ctx.last_x_to_consider  = mb->lastPixelInMeasure( mb->measureAtPixel(m_width + 15) );
    ctx.mouse_x1 = std::min(mxc, mxi) ;
    ctx.mouse_x2 = std::max(mxc, mxi) ;
    ctx.mouse_y1 = std::min(mousey_current, mousey_initial);
    ctx.mouse_y2 = std::max(mousey_current, mousey_initial);
    
    renderSilences = true;
    if (m_background_tracks.size() > 0)
    {
        const int amount = m_background_tracks.size();
        renderSilences = false;
        int color = 0;
        
        // iterate through all tracks that need to be rendered as background
        for (int bgtrack=0; bgtrack<amount; bgtrack++)
        {
            Track* otherTrack = m_background_tracks.get(bgtrack);

            // pick a color 
            // @todo : factor with same code in KeyboardEditor
            switch (color)
            {
                case 0: ariaColor.set(1, 0.85, 0,    0.5); break;
                case 1: ariaColor.set(0, 1,    0,    0.5); break;
                case 2: ariaColor.set(1, 0,    0.85, 0.5); break;
                case 3: ariaColor.set(1, 0,    0,    0.5); break;
                case 4: ariaColor.set(0, 0.85, 1,    0.5); break;
            }
            color++; if (color > 4) color = 0;

            renderTrack(otherTrack, ctx, false, false, renderSilences, ariaColor);
        }
    }

    ariaColor.set(0.0f, 0.0f, 0.0f, 1.0f);
    renderTrack(m_track, ctx, focus, true, renderSilences, ariaColor);
  

    AriaRender::lineWidth(1);
    // ------------------------- mouse drag (preview) ------------------------

    AriaRender::primitives();

    if (not m_clicked_on_note and m_mouse_is_in_editor)
    {
        // selection
        if (m_selecting)
        {
            AriaRender::select_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);

        }
        else
        {
            // ----------------------- add note (preview) --------------------

            AriaRender::color(1, 0.85, 0);

            const int tscroll = m_gsequence->getXScrollInMidiTicks();
            const float zoom = m_gsequence->getZoom();
            
            
            const int tick1 = m_track->snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI), true);
            const int len = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
            const int tick2 = tick1 + m_track->snapMidiTickToGrid(len, false);
            
            int preview_x1 = (int)((tick1 - tscroll) * zoom);
            int preview_x2 = (int)((tick2 - tscroll) * zoom);
            
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

        const int x_pixel_move = (int)( m_track->snapMidiTickToGrid(x_difference, false) * m_gsequence->getZoom() );
        const int y_step_move  = (int)round( (float)y_difference/ (float)Y_STEP_HEIGHT );

        // move a single note
        if (m_last_clicked_note!=-1)
        {
            int x1       = m_graphical_track->getNoteStartInPixels(m_last_clicked_note) -
                           m_gsequence->getXScrollInPixels() + Editor::getEditorXStart();
            const int x2 = m_graphical_track->getNoteEndInPixels(m_last_clicked_note) -
                           m_gsequence->getXScrollInPixels() + Editor::getEditorXStart();
            
            const int noteLevel = m_converter->noteToLevel( m_track->getNote(m_last_clicked_note) );

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

                const int x1 = m_graphical_track->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels() +
                               Editor::getEditorXStart();
                
                const int x2 = m_graphical_track->getNoteEndInPixels(n) - m_gsequence->getXScrollInPixels() +
                               Editor::getEditorXStart();

                const int noteLevel = m_converter->noteToLevel(m_track->getNote(n));

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
    else           AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0,                           getEditorYStart(),
                     Editor::getEditorXStart()-3, getYEnd());

    if (m_clicked_note != -1)
    {
        const int lvl = m_converter->noteToLevel( NULL, m_clicked_note );
        
        AriaRender::color(0.4, 0.4, 0.4);
        AriaRender::rect(Editor::getEditorXStart() - 30,
                         lvl*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels()-1,
                         Editor::getEditorXStart() - 3,
                         (lvl + 1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels()-1);
        
    }
    
    // ------------------------------- draw keys and horizontal lines -------------------------------

    AriaRender::color(0,0,0);

    if (m_g_clef)
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

        if (m_converter->goingInSharps())
        {
            min_level_with_signs = middle_c_level - 11;
            max_level_with_signs = middle_c_level - 4;
        }
        else if (m_converter->goingInFlats())
        {
            min_level_with_signs = middle_c_level - 9;
            max_level_with_signs = middle_c_level - 2;
        }


        AriaRender::images();
        for (int n = min_level_with_signs; n < max_level_with_signs; n++)
        {
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
            const PitchSign sharpness = m_converter->getKeySigSharpnessSignForLevel(n);

            if (sharpness == SHARP)
            {
                sharpSign->move( 48 + sharp_sign_x[ m_converter->levelToNote7(n) ], liney-1 );
                sharpSign->render();
            }
            else if (sharpness == FLAT)
            {
                flatSign->move( 48 + flat_sign_x[ m_converter->levelToNote7(n) ], liney );
                flatSign->render();
            }
        }
    }

    if (m_f_clef)
    {
        AriaRender::primitives();
        AriaRender::color(0,0,0);

        // draw horizontal score lines
        for (int n = middle_c_level+2 ; n < middle_c_level + 11; n+=2)
        {
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
            AriaRender::line(0, liney, getXEnd(), liney);
        }

        // draw sharp/flat signs next to key
        AriaRender::images();

        int max_level_with_signs = -1;
        int min_level_with_signs = -1;

        if (m_converter->goingInSharps())
        {
            min_level_with_signs = middle_c_level + 3;
            max_level_with_signs = middle_c_level + 10;
        }
        else if (m_converter->goingInFlats())
        {
            min_level_with_signs = middle_c_level + 5;
            max_level_with_signs = middle_c_level + 12;
        }

        AriaRender::images();

        for (int n = min_level_with_signs; n < max_level_with_signs; n++)
        {
            const int liney = getEditorYStart() + n*Y_STEP_HEIGHT + Y_STEP_HEIGHT/2 - yscroll;
            const int sharpness = m_converter->getKeySigSharpnessSignForLevel(n);
            if (sharpness == SHARP)
            {
                sharpSign->move( 48 + sharp_sign_x[ m_converter->levelToNote7(n) ], liney-1 ); sharpSign->render();
            }
            else if (sharpness == FLAT)
            {
                flatSign->move( 48 + flat_sign_x[ m_converter->levelToNote7(n) ], liney ); flatSign->render();
            }
        }
    }

    // --------------------------- clefs -------------------------
    AriaRender::images();
    if (m_g_clef)
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
    if (m_f_clef)
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

void ScoreEditor::renderTrack(Track* track, const TrackRenderContext& ctx, bool focus, 
                bool enableSelection, bool renderSilences, const AriaColor& baseColor)
{
    const int noteAmount = track->getNoteAmount();
    int previous_tick = -1;
    
    GraphicalTrack* otherGTrack = m_gsequence->getGraphicsFor(track);
    ASSERT(otherGTrack != NULL);
    
    m_g_clef_analyser->clearAndPrepare();
    m_f_clef_analyser->clearAndPrepare();
    
    // render pass 1. draw linear notation if relevant, gather information and do initial rendering for
    // musical notation
    for (int n=0; n<noteAmount; n++)
    {
        PitchSign note_sign;
        const int noteLevel = m_converter->noteToLevel(track->getNote(n), &note_sign);

        if (noteLevel == -1) continue;
        
        const int tick = track->getNoteStartInMidiTicks(n);
        ASSERT_E(tick, >=, previous_tick);
        previous_tick = tick;
        const int noteLength = track->getNoteEndInMidiTicks(n) - tick;

        const int original_x1 = otherGTrack->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels() +
                                Editor::getEditorXStart();
        int       x1 = original_x1;
        const int x2 = otherGTrack->getNoteEndInPixels(n)   - m_gsequence->getXScrollInPixels() +
                       Editor::getEditorXStart();

        // don't consider notes that won't be visible
        if (x2 < ctx.first_x_to_consider) continue;
        if (x1 > ctx.last_x_to_consider)  break;

        if (m_linear_notation_enabled)
        {
            if (m_musical_notation_enabled) x1 += 8;

            if (x1 < x2) // when notes are too short to be visible, don't draw them
            {
                const int y1 = noteLevel*Y_STEP_HEIGHT     + getEditorYStart() - getYScrollInPixels();
                const int y2 = (noteLevel+1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels() - 1;
                float volume = track->getNoteVolume(n)/127.0;

                // draw the quad with black border that is visible in linear notation mode
                AriaRender::primitives();
                if (m_selecting and enableSelection and ctx.mouse_x1 < original_x1 + head_radius and
                    ctx.mouse_x2 > original_x1 + head_radius and
                    ctx.mouse_y1 < y1+2 and ctx.mouse_y2 > y1+2)
                {
                    AriaRender::color(0.94f, 1.0f, 0.0f);
                }
                else if (enableSelection and track->isNoteSelected(n) and focus)
                {
                    AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
                }
                else
                {
                    AriaRender::color((1-volume*0.7), (1-volume*0.7), 1);
                }

                if (m_musical_notation_enabled)
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
            if (not m_musical_notation_enabled)
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

        if (m_musical_notation_enabled)
        {
            MeasureData* md = m_sequence->getMeasureData();
            
            // build visible notes vector with initial info in it
            NoteRenderInfo currentNote = NoteRenderInfo::factory(tick, noteLevel, noteLength, note_sign,
                                                                 enableSelection and track->isNoteSelected(n),
                                                                 track->getNotePitchID(n), md);

            // add note to either G clef score or F clef score
            if (m_g_clef and not m_f_clef)
            {
                m_g_clef_analyser->addToVector(currentNote);
            }
            else if (m_f_clef and not m_g_clef)
            {
                m_f_clef_analyser->addToVector(currentNote);
            }
            else if (m_f_clef and m_g_clef)
            {
                const int middleC = m_converter->getScoreCenterCLevel();
                if (noteLevel < middleC)
                {
                    m_g_clef_analyser->addToVector(currentNote);
                }
                else if (noteLevel > middleC)
                {
                    m_f_clef_analyser->addToVector(currentNote);
                }
                else
                {
                    // note is exactly on middle C... do our best to
                    // guess on which clef to put this note
                    // we'll check nearby notes in case it can help us
                    int check_note = -1;
                    if      (n > 0)            check_note = n-1;
                    else if (n+1 < noteAmount) check_note = n+1;

                    if (check_note != -1)
                    {
                        const int checkNoteLevel = m_converter->noteToLevel( track->getNote(check_note), (PitchSign*)NULL );
                        
                        if (checkNoteLevel > middleC)  m_f_clef_analyser->addToVector(currentNote);
                        else                           m_g_clef_analyser->addToVector(currentNote);
                    }
                    else
                    {
                        m_g_clef_analyser->addToVector(currentNote);
                    }
                    
                } // end if note on middle C
            } // end if both G and F clefs
        } // end if musical notation enabled
    } // next note
    
    
    if (m_g_clef)
    {
        m_g_clef_analyser->doneAdding();
    }
    if (m_f_clef)
    {
        m_f_clef_analyser->doneAdding();
    }
    
    // render musical notation if enabled
    if (m_musical_notation_enabled)
    {
        if (m_g_clef)
        {
            const int silences_y = getEditorYStart() +
                                   Y_STEP_HEIGHT*(m_converter->getScoreCenterCLevel()-8) -
                                   getYScrollInPixels() + 1;
            renderScore(m_g_clef_analyser, silences_y, renderSilences, baseColor);
        }

        if (m_f_clef)
        {
            const int silences_y = getEditorYStart() +
                                   Y_STEP_HEIGHT*(m_converter->getScoreCenterCLevel()+4) -
                                  getYScrollInPixels() + 1;
            renderScore(m_f_clef_analyser, silences_y, renderSilences, baseColor);
        }
    }
}


// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::renderScore(ScoreAnalyser* analyser, const int silences_y, 
            bool renderSilences, const AriaColor& baseColor)
{
    int visibleNoteAmount = analyser->getNoteCount();
    
    // first note rendering pass
    for (int i=0; i<visibleNoteAmount; i++) renderNote_pass1(analyser->m_note_render_info[i], baseColor);

    AriaRender::setImageState(AriaRender::STATE_NOTE);
    
    // render silences
    if (renderSilences)
    {
        MeasureBar* mb = m_gsequence->getMeasureBar();

        const unsigned int first_visible_measure = mb->measureAtPixel( Editor::getEditorXStart() );
        const unsigned int last_visible_measure  = mb->measureAtPixel( getXEnd() );
        
        SilenceAnalyser::findSilences(m_sequence, &renderSilence, analyser, first_visible_measure,
                                      last_visible_measure, silences_y, m_gsequence);
                                  
    }

    // ------------------------- second note rendering pass -------------------

    // analyse notes to know how to build the score
    analyser->analyseNoteInfo();

    // triplet signs, tied notes, flags and beams
    visibleNoteAmount = analyser->m_note_render_info.size();
    for (int i=0; i<visibleNoteAmount; i++)
    {
        ASSERT_E(i,<,(int)analyser->m_note_render_info.size());
        renderNote_pass2(analyser->m_note_render_info[i], analyser, baseColor);
    }
    AriaRender::setImageState(AriaRender::STATE_NOTE);
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
        
        picker->setParent( m_graphical_track );
        picker->setChecks(m_musical_notation_enabled, m_linear_notation_enabled, m_f_clef, m_g_clef,
                          m_converter->getOctaveShift());
        
        Display::popupMenu( picker, x.getRelativeTo(WINDOW),y);
        return;
    }
    // user clicked on a note on the staff
    else if (x.getRelativeTo(EDITOR)<0 and x.getRelativeTo(EDITOR)>-30 and y>getEditorYStart())
    {
        const int level = getLevelAtY(y-Y_STEP_HEIGHT/2);
        const int pitchID = m_converter->levelToNote(level);
        if (pitchID != -1)
        {
            if (not PlatformMidiManager::get()->isRecording())
            {
                PlatformMidiManager::get()->playNote(131 - pitchID, m_track->getDefaultVolume(),
                                                     500 /* duration */, 0, m_track->getInstrument());
            }
            m_clicked_note = pitchID;
            Display::render();
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
    if (m_clicked_note)
    {
        m_clicked_note = -1;
        Display::render();
    }
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::rightClick(RelativeXCoord x, int y)
{
    Editor::rightClick(x, y);

}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::processMouseMove(RelativeXCoord x, int y)
{
    const int level = getLevelAtY(y);
    if (level < 0 or level >= 73) return;
    int note = m_converter->levelToNote(level);
    
    Note12 note12;
    int octave;
    
    if (Note::findNoteName(note, &note12 /* out */, &octave /* out */))
    {
        wxString status;
        
        switch (m_track->getKeyType())
        {
            case KEY_TYPE_SHARPS:
            case KEY_TYPE_C:
                status = NOTE_12_NAME[note12];
                break;
                
            case KEY_TYPE_FLATS:
            default:
                status = NOTE_12_NAME_FLATS[note12];
                break;
        }
        
        status = wxGetTranslation(status);
        status << octave;
        if (not PlatformMidiManager::get()->isPlaying())
        {
            getMainFrame()->setStatusText(status);
        }
    }
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
        const int noteLevel = m_converter->noteToLevel( m_track->getNote(n) );
        if (noteLevel == -1) continue;
        
        const int note_y  = getEditorYStart() + Y_STEP_HEIGHT*noteLevel - head_radius -
                            getYScrollInPixels() + 2;
        const int note_x  = Editor::getEditorXStart() + m_graphical_track->getNoteStartInPixels(n) -
                            m_gsequence->getXScrollInPixels();
        const int note_x2 = Editor::getEditorXStart() + m_graphical_track->getNoteEndInPixels(n) -
                            m_gsequence->getXScrollInPixels();

        if (m_linear_notation_enabled)
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
        else if (m_musical_notation_enabled)
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

void ScoreEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    const int level = getLevelAtY(mouseY);
    if (level < 0 or level >= 73) return;
    
    const int note = m_converter->levelToNote(level);
    if (note == -1) return;

    m_track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick,
                                         m_track->getDefaultVolume()));
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
        if (note.getTick() + relativeX < 0) return; // refuse to move before song start

        note.setTick( note.getTick() + relativeX );
        note.setEndTick( note.getEndTick() + relativeX );

        if (relativeY == 0) return;
        /*
        if (Display::isSelectMorePressed() or Display::isCtrlDown())
        {
            note.pitchID += relativeY;
        }
        else
        {
            */
            int noteLevel = m_converter->noteToLevel(&note);
            noteLevel += relativeY;
            if (noteLevel > 0 and noteLevel < 73)
            {
                // reject illegal notes
                note.setPitchID( m_converter->levelToNote(noteLevel) );
            }
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
        const int noteLevel = m_converter->noteToLevel( m_track->getNote(n) );
        if (noteLevel == -1) continue;
        
        const int note_y = getEditorYStart() + Y_STEP_HEIGHT*noteLevel - getYScrollInPixels() + 2;
        const int note_x = Editor::getEditorXStart() + m_graphical_track->getNoteStartInPixels(n) -
                           m_gsequence->getXScrollInPixels() + head_radius;

        if (std::min(mxc, mxi) < note_x and std::max(mxc,mxi)>note_x and
            std::min(mousey_current, mousey_initial) < note_y and
            std::max(mousey_current, mousey_initial) > note_y)
        {
            m_graphical_track->selectNote(n, true);
        }
        else
        {
            m_graphical_track->selectNote(n, false);
        }

    }
}

// ----------------------------------------------------------------------------------------------------------

void ScoreEditor::processKeyPress(int keycode, bool commandDown, bool shiftDown)
{
    if (commandDown or shiftDown)
    {
        if (keycode == WXK_UP)
        {
            m_track->action( new Action::ShiftBySemiTone(-1, SELECTED_NOTES) );
            Display::render();
            return;
        }
        
        if (keycode == WXK_DOWN)
        {
            m_track->action( new Action::ShiftBySemiTone(1, SELECTED_NOTES) );
            Display::render();
            return;
        }
        
    }
    
    Editor::processKeyPress(keycode, commandDown, shiftDown);
}

