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

#include "Editors/GuitarEditor.h"

#include "AriaCore.h"
#include "Actions/AddNote.h"
#include "Actions/EditAction.h"
#include "Actions/NumberPressed.h"
#include "Actions/ShiftFrets.h"
#include "Actions/ShiftString.h"
#include "Actions/UpdateGuitarTuning.h"
#include "GUI/GraphicalSequence.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/ImageProvider.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/TuningPicker.h"
#include "PreferencesData.h"
#include "Renderers/RenderAPI.h"
#include "Singleton.h"


namespace AriaMaestosa
{
    const int first_string_position = 17;
    const int y_step = 10;

    class GuitarNoteNamesSingleton : public AriaRenderArray, public Singleton<GuitarNoteNamesSingleton>
    {
        friend class Singleton<GuitarNoteNamesSingleton>;
        
        GuitarNoteNamesSingleton() : AriaRenderArray(NOTE_12_NAME, 12)
        {
        }
    public:

        
        virtual ~GuitarNoteNamesSingleton()
        {
        }
    };
    DEFINE_SINGLETON( GuitarNoteNamesSingleton );
}
using namespace AriaMaestosa;


// ----------------------------------------------------------------------------------------------------------

GuitarEditor::GuitarEditor(GraphicalTrack* track) : Editor(track)
{
    m_mouse_is_in_editor = false;
    m_clicked_on_note    = false;
    m_last_clicked_note  = -1;

    GuitarNoteNamesSingleton::getInstance()->setFont( getStringNameFont() );

    // set standard tuning by default (FIXME: don't duplicate the tuning from the tuning picker)
    std::vector<int> newTuning;
    newTuning.push_back( Note::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 4) );
    newTuning.push_back( Note::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 3) );
    newTuning.push_back( Note::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 3) );
    newTuning.push_back( Note::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 3) );
    newTuning.push_back( Note::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 2) );
    newTuning.push_back( Note::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 2) );
    m_track->getGuitarTuning()->setTuning(newTuning, false);
    
    Editor::useVerticalScrollbar(false);
}

// ----------------------------------------------------------------------------------------------------------

GuitarEditor::~GuitarEditor()
{
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::render(RelativeXCoord mousex_current, int mousey_current,
                          RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{

    if (not ImageProvider::imagesLoaded()) return;

    const GuitarTuning* tuning = m_track->getGuitarTuning();
    const int string_amount = tuning->tuning.size();

    AriaRender::beginScissors(LEFT_EDGE_X, getEditorYStart(), m_width - 15, m_height + BORDER_SIZE);

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);
    AriaRender::rect(0, getEditorYStart(), getXEnd(), getYEnd());

    drawVerticalMeasureLines(getEditorYStart() + first_string_position,
                             getEditorYStart() + first_string_position + (string_amount-1)*y_step);

    // ------------------------------- draw strings -------------------------------
    AriaRender::color(0,0,0);

    const int stringCount = tuning->tuning.size();
    for (int n=0; n<stringCount; n++)
    {
        AriaRender::line(Editor::getEditorXStart(), getEditorYStart() + first_string_position + n*y_step,
                        getXEnd(), getEditorYStart() + first_string_position + n*y_step);
    }

    int lastNote[stringCount];
    int lastNoteTick[stringCount];
    
    for (int n=0; n<stringCount; n++)
    {
        lastNote[n] = -1;
        lastNoteTick[n] = -1;
    }

    // ---------------------- draw notes ----------------------------
    const int noteAmount = m_track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {
        const int pscroll = m_gsequence->getXScrollInPixels();
        int x1 = m_graphical_track->getNoteStartInPixels(n) - pscroll;
        int x2 = m_graphical_track->getNoteEndInPixels(n)   - pscroll;

        
        // don't draw notes that won't visible
        if (x2 < 0    )   continue;
        if (x1 > m_width) break;

        AriaRender::primitives();
        
        const int tick   = m_track->getNoteStartInMidiTicks(n);
        const int string = m_track->getNoteString(n);
        const int fret   = m_track->getNoteFret(n);
        
        float volume = m_track->getNoteVolume(n)/127.0;

        if (m_track->isNoteSelected(n) and focus)
        {
            AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        }
        else
        {
            AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);
        }
        
        //lastNote[string] = fret;
        //lastNoteTick[string] = track->getNoteStartInMidiTick(n);

        const int minSize = 12; // TODO: don't hardcode
        const int maxTickDistance = 960; // TODO: don't hardcode
        
        x1 += Editor::getEditorXStart();
        x2 += Editor::getEditorXStart();
        const int y = getEditorYStart()+first_string_position+string*y_step;
        
        // Check if we draw the number or the body only
        if (x2 - x1 > minSize or lastNote[string] != fret or tick - lastNoteTick[string] > maxTickDistance)
        {
            AriaRender::bordered_rect_no_start(x1,  y - 2,  x2 - 1,  y + 2);

            // fret number
            if (m_track->isNoteSelected(n) and focus)
            {
                AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
            }
            else
            {
                AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);
            }

            if (fret < 10)
            {
                AriaRender::quad(x1 - 1,  y - 8,
                                 x1 - 1,  y,
                                 x1 + 14, y,
                                 x1 + 9,  y - 8);
            }
            else
            {
                AriaRender::quad(x1 - 1,  y - 8,
                                 x1 - 1,  y,
                                 x1 + 18, y,
                                 x1 + 13, y - 8);
            }
            
            AriaRender::images();

            // if note color is too dark, draw the fret number in white
            if ((not m_track->isNoteSelected(n) or not focus) and volume > 0.5)
            {
                AriaRender::color(1, 1, 1);
            }
            else
            {
                AriaRender::color(0, 0, 0);
            }

            // FIXME: draw twice to make it more visible...
            AriaRender::renderNumber(fret, x1, y + 3);
            AriaRender::renderNumber(fret, x1, y + 3);
        }
        else
        {
            // no fret number, display short note
            AriaRender::bordered_rect(x1,  y - 2,  x2 - 1,  y + 2);
        }
        
        lastNote[string] = fret;
        lastNoteTick[string] = tick;
        
    }//next

    AriaRender::primitives();

    // mouse drag
    if (not m_clicked_on_note and m_mouse_is_in_editor)
    {

        // -------------------- selection ----------------------
        if (m_selecting)
        {
            AriaRender::color(0,0,0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);
        }
        // ----------------------- add note (preview) --------------------
        else if (mousey_current<getYEnd()-15)
        {

            AriaRender::color(1, 0.85, 0);

            const int tscroll = m_gsequence->getXScrollInMidiTicks();
            const float zoom = m_gsequence->getZoom();
            
            const int preview_x1 =
                (int)(
                      (m_track->snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI)) - tscroll) * zoom
                      );
            const int preview_x2 =
                (int)(
                      (m_track->snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI)) - tscroll) * zoom
                      );

            int string = (int)round( (float)(mousey_initial - getEditorYStart() - first_string_position) / (float)y_step);

            if (not (preview_x1<0 or preview_x2<0 or string<0 or
                     (unsigned int)string > tuning->tuning.size()-1)
                and preview_x2 > preview_x1)
            {
                AriaRender::rect(preview_x1+Editor::getEditorXStart(),
                                 string*y_step + getEditorYStart() + first_string_position - 5,
                                 preview_x2+Editor::getEditorXStart(),
                                 (string+1)*y_step + getEditorYStart() + first_string_position - 5);
            }

        } // end if selection or addition

    }

    // ------------------------- move note (preview) -----------------------
    if (m_clicked_on_note)
    {

        AriaRender::color(1, 0.85, 0, 0.5);

        const int x_difference = mousex_current.getRelativeTo(MIDI)-mousex_initial.getRelativeTo(MIDI);
        const int y_difference = mousey_current-mousey_initial;

        const int x_steps_to_move = (int)( m_track->snapMidiTickToGrid(x_difference) * m_gsequence->getZoom() );
        const int y_steps_to_move = (int)round( (float)y_difference / (float)y_step );

        // move a single note
        if (m_last_clicked_note != -1)
        {
            const int x1     = m_graphical_track->getNoteStartInPixels(m_last_clicked_note) -
                               m_gsequence->getXScrollInPixels();
            const int x2     = m_graphical_track->getNoteEndInPixels(m_last_clicked_note) -
                               m_gsequence->getXScrollInPixels();
            const int string = m_track->getNoteString(m_last_clicked_note);

            AriaRender::rect(x1 + x_steps_to_move + Editor::getEditorXStart(),
                             string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step,
                             x2 - 1 + x_steps_to_move+Editor::getEditorXStart(),
                             string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
        }
        else
        {
            // move a bunch of notes

            for (int n=0; n<m_track->getNoteAmount(); n++)
            {
                if (not m_track->isNoteSelected(n)) continue;

                const int x1     = m_graphical_track->getNoteStartInPixels(n) -
                                   m_gsequence->getXScrollInPixels();
                const int x2     = m_graphical_track->getNoteEndInPixels(n)   -
                                   m_gsequence->getXScrollInPixels();
                const int string = m_track->getNoteString(n);

                AriaRender::rect(x1+x_steps_to_move+Editor::getEditorXStart(),
                                 string*y_step + getEditorYStart() + first_string_position - 5 +
                                 y_steps_to_move*y_step,
                                 x2-1+x_steps_to_move+Editor::getEditorXStart(),
                                 string*y_step + getEditorYStart() + first_string_position - 5 +
                                 (y_steps_to_move+1)*y_step);
            } // next

        } // end if m_last_clicked_note!=-1

    } // end if m_clicked_on_note

    // -----------------------------------------------------------------
    // left part with string names
    // -----------------------------------------------------------------

    // grey background
    if (not focus) AriaRender::color(0.4, 0.4, 0.4);
    else           AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0,                           getEditorYStart(),
                     Editor::getEditorXStart()-3, getYEnd());

    // string names
    AriaRender::images();
    AriaRender::color(0,0,0);

    const int text_x = Editor::getEditorXStart()-23;
    
    GuitarNoteNamesSingleton* instance = GuitarNoteNamesSingleton::getInstance();
    
    for (int n=0; n<string_amount; n++)
    {
        const int text_y = getEditorYStart() + 21 + n*y_step;

        Note12 noteName;
        int octave;
        bool success = Note::findNoteName(tuning->tuning[n], &noteName, &octave);

        if (success)
        {
            instance->bind();
            if (noteName == NOTE_12_A_SHARP or noteName == NOTE_12_C_SHARP or noteName == NOTE_12_D_SHARP or
                noteName == NOTE_12_F_SHARP or noteName == NOTE_12_G_SHARP)
            {
                instance->get((int)noteName).render(text_x-6, text_y );
            }
            else
            {
                instance->get((int)noteName).render(text_x, text_y );
            }
        }
        AriaRender::renderNumber( octave, text_x+8, text_y );
        
    }//next


    AriaRender::endScissors();
    AriaRender::images();

}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
    if (x.getRelativeTo(EDITOR) < 0 and x.getRelativeTo(EDITOR) > -75 and y > getEditorYStart())
    {
        TuningPicker* picker = Core::getTuningPicker();
        picker->setModel( m_track->getGuitarTuning(), getTrack() );
        Display::popupMenu( picker, x.getRelativeTo(WINDOW), y );
        return;
    }

    Editor::mouseDown(x, y);
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                     RelativeXCoord& mousex_initial, int mousey_initial)
{
    const int count = m_track->getNoteAmount();
    for (int n=0; n<count; n++)
    {
        // on-screen pixel where note starts
        const int x1 = m_graphical_track->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels();
        
        // on-screen pixel where note ends
        const int x2 = m_graphical_track->getNoteEndInPixels(n)   - m_gsequence->getXScrollInPixels();

        const int string = m_track->getNoteString(n);

        // check if note is within selection boundaries
        if (x1 > std::min(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
            x2 < std::max(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
            std::min(mousey_current, mousey_initial) < getEditorYStart() + first_string_position + string*y_step and
            std::max(mousey_current, mousey_initial) > getEditorYStart() + first_string_position + string*y_step)
        {
            m_graphical_track->selectNote(n, true);
        }
        else
        {
            m_graphical_track->selectNote(n, false);
        }
    }//next
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if (note.getTick() + relativeX < 0) return; // refuse to move before song start

    note.setTick( note.getTick() + relativeX );
    note.setEndTick( note.getEndTick() + relativeX );

    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    if (note.getStringConst() + relativeY < 0 or note.getStringConst() + relativeY > (int)tuning->tuning.size()-1)
    {
        // note will end on a string that doesn't exist if we move it like that
        return;
    }

    note.setStringAndFret(note.getString() + relativeY, note.getFret());
}

// ----------------------------------------------------------------------------------------------------------

NoteSearchResult GuitarEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int x_edit = x.getRelativeTo(EDITOR);

    const int noteAmount = m_track->getNoteAmount();
    
    // iterate through notes in reverse order (last drawn note appears on top and must be first selected)
    for (int n=noteAmount-1; n>-1; n--)
    {
        const int x1 = m_graphical_track->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels();
        const int x2 = m_graphical_track->getNoteEndInPixels(n)   - m_gsequence->getXScrollInPixels();

        const int string = m_track->getNoteString(n);

        if (x_edit > x1 and
            x_edit < x2 and
            y < getEditorYStart() + first_string_position + string*y_step + 4 and
            y > getEditorYStart() + first_string_position + string*y_step - 5)
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
    }//next

    return FOUND_NOTHING;
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::noteClicked(const int id)
{
    m_graphical_track->selectNote(ALL_NOTES, false);
    m_graphical_track->selectNote(id, true);
    m_track->playNote(id);
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    int string = (int)round( (float)(mouseY - getEditorYStart() - first_string_position) / (float)y_step );

    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    if (string < 0)                           return; //invalid note, don't add it
    if (string >= (int)tuning->tuning.size()) return; //invalid note, don't add it

    m_track->action( new Action::AddNote(tuning->tuning[string], snapped_start_tick, snapped_end_tick,
                                         m_default_volume, string ) );
}

// ----------------------------------------------------------------------------------------------------------

void GuitarEditor::processKeyPress(int keycode, bool commandDown, bool shiftDown)
{
    // ------------------- numbers -------------------
    // number at the top of the keyboard
    if (keycode >= 48 and keycode <= 57)
    {
        if (shiftDown)
        {
            m_track->action( new Action::NumberPressed(keycode - 48 + 10) );
        }
        else
        {
            m_track->action( new Action::NumberPressed(keycode - 48) );
        }
        Display::render();
    }
    
    // numpad
    if (keycode >= 324 and keycode <= 333)
    {
        if (shiftDown)
        {
            m_track->action( new Action::NumberPressed(keycode - 324 + 10) );
        }
        else
        {
            m_track->action( new Action::NumberPressed(keycode - 324) );
        }
        Display::render();
    }
    
    // ---------------- shift frets -----------------
    if (not commandDown and shiftDown)
    {
        
        if (keycode == WXK_LEFT)
        {
            m_track->action( new Action::ShiftFrets(-1, SELECTED_NOTES) );
            Display::render();
        }
        
        if (keycode == WXK_RIGHT)
        {
            m_track->action( new Action::ShiftFrets(1, SELECTED_NOTES) );
            Display::render();
        }
        
        if (keycode == WXK_UP)
        {
            m_track->action( new Action::ShiftString(-1, SELECTED_NOTES) );
            Display::render();
        }
        
        if (keycode == WXK_DOWN)
        {
            m_track->action( new Action::ShiftString(1, SELECTED_NOTES) );
            Display::render();
        }
        
    }
    
    Editor::processKeyPress(keycode, commandDown, shiftDown);
}
