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


#include <cmath>

#include "Editors/KeyboardEditor.h"

#include "AriaCore.h"
#include "Actions/AddNote.h"
#include "Actions/EditAction.h"
#include "Actions/MoveNotes.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/ImageProvider.h"
#include "GUI/MainFrame.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/KeyPicker.h"
#include "Renderers/Drawable.h"
#include "Renderers/RenderAPI.h"
#include "Utils.h"

#define SHOW_MIDI_PITCH 0

using namespace AriaMaestosa;


// ************************************************************************************************************
// *********************************************    CTOR/DTOR      ********************************************
// ************************************************************************************************************
#if 0
#pragma mark Ctor/dtor
#endif

KeyboardEditor::KeyboardEditor(GraphicalTrack* track) : Editor(track)
{
    m_sb_position = 0.5;
}

// -----------------------------------------------------------------------------------------------------------

KeyboardEditor::~KeyboardEditor()
{

}

// **********************************************************************************************************
// ****************************************    EVENTS      **************************************************
// **********************************************************************************************************

#if 0
#pragma mark -
#pragma mark Events
#endif

void KeyboardEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
    if (x.getRelativeTo(EDITOR) < -30 and x.getRelativeTo(WINDOW) > 15 and y > getEditorYStart())
    {
        KeyPicker* picker = Core::getKeyPicker();
        picker->setParent( m_graphical_track );
        Display::popupMenu(picker, x.getRelativeTo(WINDOW), y);
        return;
    }
    // user clicked on a keyboard key
    else if (x.getRelativeTo(EDITOR) < 0 and x.getRelativeTo(EDITOR) > -30 and y > getEditorYStart())
    {
        const int pitchID = getLevelAtY(y);
        PlatformMidiManager::get()->playNote( 131-pitchID, m_default_volume, 500, 0, m_track->getInstrument() );
        return;
    }
    
    Editor::mouseDown(x, y);
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::processMouseMove(RelativeXCoord x, int y)
{
    const int note = getLevelAtY(y);
    
    Note12 note12;
    int octave;
    
    if (Note::findNoteName(note, &note12 /* out */, &octave /* out */))
    {
        wxString status;
        
        switch (m_track->getKeyType())
        {
            case KEY_TYPE_SHARPS:
            case KEY_TYPE_C:
                status << NOTE_12_NAME[note12];
                break;
                
            case KEY_TYPE_FLATS:
            default:
                status << NOTE_12_NAME_FLATS[note12];
                break;
        }
        
        status << octave;
        getMainFrame()->setStatusText(status);
    }
    else
    {
        getMainFrame()->setStatusText(wxT(""));
    }
}

// ***********************************************************************************************************
// ******************************************    EDITOR METHODS      *****************************************
// ***********************************************************************************************************
#if 0
#pragma mark -
#pragma mark Editor methods
#endif


void KeyboardEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    const int note = getLevelAtY(mouseY);
    m_track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, m_default_volume ) );
}

// -----------------------------------------------------------------------------------------------------------

NoteSearchResult KeyboardEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int x_edit = x.getRelativeTo(EDITOR);

    const int noteAmount = m_track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {
        const int x1 = m_graphical_track->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels();
        const int x2 = m_graphical_track->getNoteEndInPixels(n)   - m_gsequence->getXScrollInPixels();
        const int y1 = m_track->getNotePitchID(n)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels();

        if (x_edit > x1 and x_edit < x2 and y > y1 and y < y1+12)
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


        }//end if
    }//next

    return FOUND_NOTHING;
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial)
{
    const int mouse_x_min = std::min( mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) );
    const int mouse_x_max = std::max( mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) );
    const int mouse_y_min = std::min( mousey_current, mousey_initial );
    const int mouse_y_max = std::max( mousey_current, mousey_initial );
    const int xscroll = m_gsequence->getXScrollInPixels();
    
    const int count = m_track->getNoteAmount();
    for (int n=0; n<count; n++)
    {
        int x1        = m_graphical_track->getNoteStartInPixels(n);
        int x2        = m_graphical_track->getNoteEndInPixels(n);
        int from_note = m_track->getNotePitchID(n);
        const int y   = levelToY(from_note);

        if (x1 > mouse_x_min + xscroll and x2 < mouse_x_max + xscroll and
            y + Y_STEP_HEIGHT/2 > mouse_y_min and y + Y_STEP_HEIGHT/2 < mouse_y_max )
        {
            m_graphical_track->selectNote(n, true);
        }
        else
        {
            m_graphical_track->selectNote(n, false);
        }
    }//next

}

// -----------------------------------------------------------------------------------------------------------
    
int KeyboardEditor::getYScrollInPixels()
{
    return (int)( m_sb_position*(120*11 - m_height - 20) );
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if (note.getTick()    + relativeX < 0)    return; // refuse to move before song start
    if (note.getPitchID() + relativeY < 0)    return; // reject moves that would make illegal notes
    if (note.getPitchID() + relativeY >= 128) return;

    note.setTick( note.getTick() + relativeX );
    note.setEndTick( note.getEndTick() + relativeX );
    note.setPitchID( note.getPitchID() + relativeY );
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::processKeyPress(int keycode, bool commandDown, bool shiftDown)
{
    // shift by octave
    if (shiftDown and not commandDown)
    {
        if (keycode == WXK_UP)
        {
            m_track->action(new Action::MoveNotes(this, 0, -12, SELECTED_NOTES));
            Display::render();
            return;
        }
        
        if (keycode == WXK_DOWN)
        {
            m_track->action(new Action::MoveNotes(this, 0, 12, SELECTED_NOTES));
            Display::render();
            return;
        }
    }
    
    Editor::processKeyPress(keycode, commandDown, shiftDown);
}

// ************************************************************************************************************
// ****************************************    RENDER      ****************************************************
// ************************************************************************************************************

#if 0
#pragma mark -
#pragma mark Render
#endif


void KeyboardEditor::render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    if (not ImageProvider::imagesLoaded()) return;

    AriaRender::beginScissors(LEFT_EDGE_X, getEditorYStart(), m_width - RIGHT_SCISSOR, m_height);

    // ------------------ draw lined background ----------------

    int levelid = getYScrollInPixels()/Y_STEP_HEIGHT;
    const int yscroll = getYScrollInPixels();
    const int last_note = ( yscroll + getYEnd() - getEditorYStart() )/Y_STEP_HEIGHT;
    const int editor_x1 = getEditorXStart();
    const int editor_x2 = getXEnd();

    const KeyInclusionType* key_notes = m_track->getKeyNotes();
    
    // white background
    AriaRender::primitives();

    // horizontal lines
    AriaRender::color(0.94, 0.94, 0.94, 1);
    while (levelid < last_note)
    {
        //const int note12 = 11 - ((levelid - 3) % 12);
        const int pitchID = levelid; //FIXME: fix this conflation of level and pitch ID. it's handy in keyboard
                                     // editor, but a pain everywhere else...

        if (key_notes[pitchID] != KEY_INCLUSION_FULL)
        {
            AriaRender::rect(editor_x1, levelToY(levelid),
                             editor_x2, levelToY(levelid+1));
        }
        else
        {
            AriaRender::line(editor_x1, levelToY(levelid+1),
                             editor_x2, levelToY(levelid+1));
        }

        
#if SHOW_MIDI_PITCH
        const int midiID = (131 - pitchID);
        // octave number
        AriaRender::images();
        AriaRender::color(0,0,0);
        
        AriaRender::renderNumber(midiID, 100, levelToY(levelid+1));

        AriaRender::primitives();
        AriaRender::color(0.94, 0.94, 0.94, 1);
#endif
        
        
        levelid++;
    }

    drawVerticalMeasureLines(getEditorYStart(), getYEnd());

    // ---------------------- draw background notes ------------------

    if (m_background_tracks.size() > 0)
    {
        const int amount = m_background_tracks.size();
        int color = 0;
        
        // iterate through all tracks that need to be rendered as background
        for (int bgtrack=0; bgtrack<amount; bgtrack++)
        {
            Track* otherTrack = m_background_tracks.get(bgtrack);
            GraphicalTrack* otherGTrack = m_gsequence->getGraphicsFor(otherTrack);
            ASSERT(otherGTrack != NULL);
            const int noteAmount = otherTrack->getNoteAmount();

            // pick a color
            switch (color)
            {
                case 0: AriaRender::color(1, 0.85, 0,    0.5); break;
                case 1: AriaRender::color(0, 1,    0,    0.5); break;
                case 2: AriaRender::color(1, 0,    0.85, 0.5); break;
                case 3: AriaRender::color(1, 0,    0,    0.5); break;
                case 4: AriaRender::color(0, 0.85, 1,    0.5); break;
            }
            color++; if (color > 4) color = 0;

            // render the notes
            for (int n=0; n<noteAmount; n++)
            {

                int x1 = otherGTrack->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels();
                int x2 = otherGTrack->getNoteEndInPixels(n)   - m_gsequence->getXScrollInPixels();

                // don't draw notes that won't be visible
                if (x2 < 0)       continue;
                if (x1 > m_width) break;

                const int pitch = otherTrack->getNotePitchID(n);

                AriaRender::rect(x1 + getEditorXStart(),   levelToY(pitch),
                                 x2 + getEditorXStart()-1, levelToY(pitch+1));
            }

        }
    }

    AriaRender::primitives();
    // ---------------------- draw notes ----------------------------
    const float pscroll = m_gsequence->getXScrollInPixels();

    const int mouse_x_min = std::min(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) );
    const int mouse_x_max = std::max(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) );
    const int mouse_y_min = std::min(mousey_current, mousey_initial);
    const int mouse_y_max = std::max(mousey_current, mousey_initial);
    
    const int noteAmount = m_track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {
        const int x1 = m_graphical_track->getNoteStartInPixels(n) - pscroll;
        const int x2 = m_graphical_track->getNoteEndInPixels(n)   - pscroll;

        // don't draw notes that won't be visible
        if (x2 < 0)       continue;
        if (x1 > m_width) break;

        const int pitch = m_track->getNotePitchID(n);
        const int level = pitch;
        float volume    = m_track->getNoteVolume(n)/127.0;

        const int y1 = levelToY(level);
        const int y2 = levelToY(level+1);
        
        if (key_notes[pitch] == KEY_INCLUSION_NONE)
        {
            AriaRender::color(1.0f, 0.0f, 0.0f);
        }
        else if (m_selecting and x1 > mouse_x_min and x2 < mouse_x_max and
                 y1 + Y_STEP_HEIGHT/2 > mouse_y_min and y1 + Y_STEP_HEIGHT/2 < mouse_y_max)
        {
            AriaRender::color(0.94f, 1.0f, 0.0f);
        }
        else if (m_track->isNoteSelected(n) and focus)
        {
            AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        }
        else
        {
            AriaRender::color((1-volume)*0.9, (1-volume)*0.9,  (1-volume)*0.9);
        }

        
        AriaRender::bordered_rect(x1 + getEditorXStart() + 1, y1,
                                  x2 + getEditorXStart() - 1, y2);
    }


    AriaRender::primitives();

    if (not m_clicked_on_note and m_mouse_is_in_editor)
    {

        // -------- Selection
        if (m_selecting)
        {
            // selection
            AriaRender::select_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);
        }
        else
        {
            // -------- Add note (preview)
            AriaRender::color(1, 0.85, 0);

            const int tscroll = m_gsequence->getXScrollInMidiTicks();
            const float zoom = m_gsequence->getZoom();
            
            int preview_x1 =
                (int)(
                      (m_track->snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI)) - tscroll) * zoom
                      );
            int preview_x2 =
                (int)(
                      (m_track->snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI)) - tscroll) * zoom
                      );

            if (not (preview_x1 < 0 or preview_x2 < 0) and preview_x2 > preview_x1)
            {
                const int y1 = ((mousey_initial - getEditorYStart() + getYScrollInPixels())/Y_STEP_HEIGHT)*Y_STEP_HEIGHT +
                                getEditorYStart() - getYScrollInPixels();
                const int y2 = ((mousey_initial - getEditorYStart() + getYScrollInPixels())/Y_STEP_HEIGHT)*Y_STEP_HEIGHT +
                                Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels();
                
                AriaRender::rect(preview_x1 + getEditorXStart(), y1,
                                 preview_x2 + getEditorXStart(), y2);
            }

        }// end if selection or addition
    }

    // ------------------------- move note (preview) -----------------------
    if (m_clicked_on_note)
    {
        AriaRender::color(1, 0.85, 0, 0.5);

        int x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
        int y_difference = mousey_current - mousey_initial;

        const int x_step_move = (int)( m_track->snapMidiTickToGrid(x_difference) * m_gsequence->getZoom() );
        const int y_step_move = (int)round( (float)y_difference/ (float)Y_STEP_HEIGHT );

        // move a single note
        if (m_last_clicked_note != -1)
        {
            int x1 = m_graphical_track->getNoteStartInPixels(m_last_clicked_note) -
                     m_gsequence->getXScrollInPixels();
            int x2 = m_graphical_track->getNoteEndInPixels  (m_last_clicked_note) -
                     m_gsequence->getXScrollInPixels();
            int y  = m_track->getNotePitchID(m_last_clicked_note);

            AriaRender::rect(x1 + x_step_move + getEditorXStart(),
                             (y + y_step_move)*Y_STEP_HEIGHT + 1 + getEditorYStart() - getYScrollInPixels(),
                             x2 - 1 + x_step_move + getEditorXStart(),
                             (y + y_step_move + 1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels());
        }
        else
        {
            // move a bunch of notes

            for (int n=0; n<m_track->getNoteAmount(); n++)
            {
                if (not m_track->isNoteSelected(n)) continue;

                int x1 = m_graphical_track->getNoteStartInPixels(n) - m_gsequence->getXScrollInPixels();
                int x2 = m_graphical_track->getNoteEndInPixels  (n) - m_gsequence->getXScrollInPixels();
                int y  = m_track->getNotePitchID(n);

                AriaRender::rect(x1 + x_step_move + getEditorXStart(),
                                 (y + y_step_move)*Y_STEP_HEIGHT + 1 + getEditorYStart() - getYScrollInPixels(),
                                 x2 - 1 + x_step_move+getEditorXStart(),
                                 (y + y_step_move + 1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels());
            }//next

        }

    }

    // ------------------ draw keyboard ----------------
    
    // grey background
    if (not focus) AriaRender::color(0.4, 0.4, 0.4);
    else           AriaRender::color(0.8, 0.8, 0.8);
    
    AriaRender::rect(0, getEditorYStart(), getEditorXStart()-25,  getYEnd());
    
    for (int g_octaveID=0; g_octaveID<11; g_octaveID++)
    {
        int g_octave_y = g_octaveID*120 - getYScrollInPixels();
        
        if (g_octave_y > -120 and g_octave_y < m_height + 20)
        {
            AriaRender::images();
            
            if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
            else           AriaRender::setImageState(AriaRender::STATE_NORMAL);
            
            const int keyboard_image_x = getEditorXStart()-noteTrackDrawable->getImageWidth();
            noteTrackDrawable->move(keyboard_image_x, getEditorYStart() + g_octave_y);
            noteTrackDrawable->render();
            
            AriaRender::primitives();
            AriaRender::color(0,0,0);
            
            AriaRender::line(0,                getEditorYStart() + g_octave_y,
                             keyboard_image_x, getEditorYStart() + g_octave_y);
            
            // octave number
            AriaRender::images();
            AriaRender::color(0,0,0);
            AriaRender::renderNumber(9-g_octaveID, 30, getEditorYStart()+1 + g_octave_y + 120/2);
            
        }//end if
    }//next
    
    
    // ---------------------------- scrollbar -----------------------
    // FIXME - instead implement renderScrollbar(focus)...
    AriaRender::images();
    if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else           AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::color(1,1,1);
    AriaRender::endScissors();

}




