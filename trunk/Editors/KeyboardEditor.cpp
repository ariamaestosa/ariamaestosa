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
#include "Config.h"

#include "wx/wx.h"

#include "Actions/EditAction.h"
#include "Actions/AddNote.h"

#include "Renderers/Drawable.h"
#include "GUI/ImageProvider.h"
#include "Editors/KeyboardEditor.h"
#include "Editors/RelativeXCoord.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/MagneticGrid.h"
#include "Pickers/KeyPicker.h"
#include "GUI/GraphicalTrack.h"
#include "Renderers/RenderAPI.h"

#include "AriaCore.h"



namespace AriaMaestosa {

const int y_step = 10;

// ***********************************************************************************************************************************************************
// **********************************************************    CONSTRUCTOR      ****************************************************************************
// ***********************************************************************************************************************************************************

KeyboardEditor::KeyboardEditor(Track* track) : Editor(track)
{
    note_greyed_out[0] = false;
    note_greyed_out[1] = true;
    note_greyed_out[2] = false;
    note_greyed_out[3] = false;
    note_greyed_out[4] = true;
    note_greyed_out[5] = false;
    note_greyed_out[6] = true;
    note_greyed_out[7] = false;
    note_greyed_out[8] = false;
    note_greyed_out[9] = true;
    note_greyed_out[10] = false;
    note_greyed_out[11] = true;
    sb_position=0.5;
}

KeyboardEditor::~KeyboardEditor()
{

}

// ****************************************************************************************************************************************************
// *********************************************************    EVENTS      ***************************************************************************
// ****************************************************************************************************************************************************

void KeyboardEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
    if (x.getRelativeTo(EDITOR)<-20 and x.getRelativeTo(WINDOW)>15 and y>getEditorYStart())
    {
        KeyPicker* picker = Core::getKeyPicker();
        picker->setParent(track);
        Display::popupMenu(picker, x.getRelativeTo(WINDOW), y);
        return;
    }

    Editor::mouseDown(x, y);
}

// ****************************************************************************************************************************************************
// ****************************************************    EDITOR METHODS      ************************************************************************
// ****************************************************************************************************************************************************


void KeyboardEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
    const int note = (mouseY - getEditorYStart() + getYScrollInPixels())/y_step;
    track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, default_volume ) );
}

void KeyboardEditor::noteClicked(const int id)
{
    track->selectNote(ALL_NOTES, false);
    track->selectNote(id, true);
    track->playNote(id);
}

NoteSearchResult KeyboardEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int x_edit = x.getRelativeTo(EDITOR);

    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
        const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
        const int y1=track->getNotePitchID(n)*y_step + getEditorYStart() - getYScrollInPixels();

        if (x_edit > x1 and x_edit < x2 and y > y1 and y < y1+12)
        {
            noteID = n;

            if (track->isNoteSelected(n) and !Display:: isSelectLessPressed())
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

void KeyboardEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial)
{
    for(int n=0; n<track->getNoteAmount(); n++)
    {

        int x1=track->getNoteStartInPixels(n);
        int x2=track->getNoteEndInPixels(n);
        int from_note=track->getNotePitchID(n);

        if ( x1>std::min( mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) ) + sequence->getXScrollInPixels() and
            x2<std::max( mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR) ) + sequence->getXScrollInPixels() and
            from_note*y_step+getEditorYStart()-getYScrollInPixels() > std::min(mousey_current, mousey_initial) and
            from_note*y_step+getEditorYStart()-getYScrollInPixels() < std::max(mousey_current, mousey_initial) )
        {

            track->selectNote(n, true);

        }else{
            track->selectNote(n, false);
        }
    }//next

}

int KeyboardEditor::getYScrollInPixels()
{
    return (int)(   sb_position*(120*11-height-20)   );
}

void KeyboardEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if (note.startTick+relativeX < 0) return; // refuse to move before song start
    if (note.pitchID+relativeY < 0) return; // reject moves that would make illegal notes
    if (note.pitchID+relativeY >= 128) return;

    note.startTick += relativeX;
    note.endTick   += relativeX;
    note.pitchID   += relativeY;
}

// ***********************************************************************************************************************************************************
// ************************************************************    RENDER      *******************************************************************************
// ***********************************************************************************************************************************************************

void KeyboardEditor::render()
{
    render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

void KeyboardEditor::render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{

    if (!ImageProvider::imagesLoaded()) return;

    AriaRender::beginScissors(10, getEditorYStart(), width-15, 20+height);

    // ------------------ draw lined background ----------------

    int levelid = getYScrollInPixels()/y_step;
    const int yscroll = getYScrollInPixels();
    const int y1 = getEditorYStart();
    const int last_note = ( yscroll + getYEnd() - getEditorYStart() )/y_step;
    const int x1 = getEditorsXStart();
    const int x2 = getXEnd();

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);
    AriaRender::rect(x1, getEditorYStart(), x2, getYEnd());

    // horizontal lines
    AriaRender::color(0.94, 0.94, 0.94, 1);
    while(levelid < last_note)
    {
        const int note12 = 11 - ((levelid - 3) % 12);
        if (note_greyed_out[note12] or
           levelid>131 or levelid<4 /* out of midi range notes... there's a few at the top and bottom -  FIXME - don't show them at all */)
        {
            AriaRender::rect(x1, y1 + levelid*y_step - yscroll+1,
                             x2, y1 + (levelid+1)*y_step - yscroll+1);
        }
        else
        {
            AriaRender::line(x1, y1 + (levelid+1)*y_step - yscroll+1,
                             x2, y1 + (levelid+1)*y_step - yscroll+1);
        }

        levelid++;
    }

    drawVerticalMeasureLines(getEditorYStart(), getYEnd());

    // ---------------------- draw background notes ------------------

    if (backgroundTracks.size() > 0)
    {
        const int amount = backgroundTracks.size();
        int color = 0;
        // iterate through all tracks that need to be rendered as background
        for(int bgtrack=0; bgtrack<amount; bgtrack++)
        {
            Track* track = backgroundTracks.get(bgtrack);
            const int noteAmount = track->getNoteAmount();

            // pick a color
            switch(color)
            {
                case 0: AriaRender::color(1, 0.85, 0, 0.5); break;
                case 1: AriaRender::color(0, 1, 0, 0.5); break;
                case 2: AriaRender::color(1, 0, 0.85, 0.5); break;
                case 3: AriaRender::color(1, 0, 0, 0.5); break;
                case 4: AriaRender::color(0, 0.85, 1, 0.5); break;
            }
            color++; if (color>4) color = 0;

            // render the notes
            for(int n=0; n<noteAmount; n++)
            {

                int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
                int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();

                // don't draw notes that won't be visible
                if (x2<0) continue;
                if (x1>width) break;

                const int y=track->getNotePitchID(n);


                AriaRender::rect(x1+getEditorsXStart(), y*y_step+1 + getEditorYStart() - getYScrollInPixels(),
                                          x2+getEditorsXStart()-1, (y+1)*y_step + getEditorYStart() - getYScrollInPixels());
            }

        }
    }

    // ---------------------- draw notes ----------------------------
    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {

        int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
        int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();

        // don't draw notes that won't be visible
        if (x2<0) continue;
        if (x1>width) break;

        int y=track->getNotePitchID(n);
        float volume=track->getNoteVolume(n)/127.0;

        if (track->isNoteSelected(n) and focus) AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        else AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);

        AriaRender::bordered_rect(x1+getEditorsXStart()+1, y*y_step+1 + getEditorYStart() - getYScrollInPixels(),
                                  x2+getEditorsXStart()-1, (y+1)*y_step + getEditorYStart() - getYScrollInPixels());
    }


    // ------------------ draw keyboard ----------------

    // grey background
    if (!focus) AriaRender::color(0.4, 0.4, 0.4);
    else AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0, getEditorYStart(), getEditorsXStart()-25,  getYEnd());

    for(int g_octaveID=0; g_octaveID<11; g_octaveID++)
    {
        int g_octave_y=g_octaveID*120-getYScrollInPixels();
        if (g_octave_y>-120 and g_octave_y<height+20)
        {
            AriaRender::images();

            if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
            else AriaRender::setImageState(AriaRender::STATE_NORMAL);

            noteTrackDrawable->move(getEditorsXStart()-noteTrackDrawable->getImageWidth(), from_y+barHeight+20 + g_octave_y);
            noteTrackDrawable->render();

            AriaRender::primitives();
            AriaRender::color(0,0,0);

            // FIXME - all those from_y+barHeight+20 stuff needs to be cleaned up for good
            AriaRender::line(0, from_y+barHeight+20 + g_octave_y+1,
                             getEditorsXStart()-25, from_y+barHeight+20 + g_octave_y+1);

            // octave number
            AriaRender::images();
            AriaRender::color(0,0,0);
            AriaRender::renderNumber(10-g_octaveID, 30, from_y+barHeight+21 + g_octave_y +120/2);
            
        }//end if
    }//next


    // ------------------------- mouse drag (preview) ------------------------

    AriaRender::primitives();

    if (!clickedOnNote and mouse_is_in_editor)
    {

        if (selecting)
        {
            // selection
            AriaRender::color(0,0,0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);

        }
        else
        {
            // add note (preview)
            AriaRender::color(1, 0.85, 0);


            int preview_x1=
                (int)(
                      (snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI)) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            int preview_x2=
                (int)(
                      (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI)) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );

            if (!(preview_x1<0 or preview_x2<0) and preview_x2>preview_x1)
            {
                AriaRender::rect(preview_x1+getEditorsXStart(),
                                 ((mousey_initial - getEditorYStart() + getYScrollInPixels())/y_step)*y_step + getEditorYStart() - getYScrollInPixels(),
                                 preview_x2+getEditorsXStart(),
                                 ((mousey_initial - getEditorYStart() + getYScrollInPixels())/y_step)*y_step+y_step + getEditorYStart() - getYScrollInPixels());
            }

        }// end if selection or addition
    }

    // ------------------------- move note (preview) -----------------------
    if (clickedOnNote)
    {
        AriaRender::color(1, 0.85, 0, 0.5);

        int x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
        int y_difference = mousey_current - mousey_initial;

        const int x_step_move = (int)( snapMidiTickToGrid(x_difference) * sequence->getZoom() );
        const int y_step_move = (int)round( (float)y_difference/ (float)y_step );

        // move a single note
        if (lastClickedNote!=-1)
        {
            int x1=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            int x2=track->getNoteEndInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            int y=track->getNotePitchID(lastClickedNote);

            AriaRender::rect(x1+x_step_move+getEditorsXStart(),
                             (y+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels(),
                             x2-1+x_step_move+getEditorsXStart(),
                             (y+y_step_move+1)*y_step + getEditorYStart() - getYScrollInPixels());
        }
        else
        {
            // move a bunch of notes

            for(int n=0; n<track->getNoteAmount(); n++)
            {
                if (!track->isNoteSelected(n)) continue;

                int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
                int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
                int y=track->getNotePitchID(n);

                AriaRender::rect(x1+x_step_move+getEditorsXStart(),
                                 (y+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels(),
                                 x2-1+x_step_move+getEditorsXStart(),
                                 (y+y_step_move+1)*y_step + getEditorYStart() - getYScrollInPixels());
            }//next

        }

    }

    // ---------------------------- scrollbar -----------------------
    // FIXME - instead implement ernderScrollbar(focus)...
    if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::color(1,1,1);
    AriaRender::endScissors();

}



void KeyboardEditor::loadKey(const PitchSign sharpness_symbol, const int symbol_amount)
{
    static const int note7_to_note12[] = {
        /* A */ 0,
        /* B */ 2,
        /* C */ 3,
        /* D */ 5,
        /* E */ 7,
        /* F */ 8,
        /* G */ 10};

    // if key is e.g. G Major, "major_note" will be set to note12 equivalent of G.
    // to load a minor key, it's just set to the major one that has same sharps and flats
    // to ease the process
    int major_note12 = 0;

    if (symbol_amount == 0 or sharpness_symbol == NATURAL)
    {
        major_note12 = note7_to_note12[C];
    }
    else if (sharpness_symbol == SHARP)
    {
        switch(symbol_amount)
        {
            case 1: major_note12 = note7_to_note12[G]; break;
            case 2: major_note12 = note7_to_note12[D]; break;
            case 3: major_note12 = note7_to_note12[A]; break;
            case 4: major_note12 = note7_to_note12[E]; break;
            case 5: major_note12 = note7_to_note12[B]; break;
            case 6: major_note12 = note7_to_note12[F]+1; /* F# */break;
            case 7: major_note12 = note7_to_note12[C]+1; /* C# */ break;
        }
    }
    else if (sharpness_symbol == FLAT)
    {
        switch(symbol_amount)
        {
            case 1: major_note12 = note7_to_note12[F]; break;
            case 2: major_note12 = note7_to_note12[B]-1; /* Bb */ break;
            case 3: major_note12 = note7_to_note12[E]-1; /* Eb */ break;
            case 4: major_note12 = note7_to_note12[A]-1 + 12; /* Ab */ break;
            case 5: major_note12 = note7_to_note12[D]-1; /* Db */ break;
            case 6: major_note12 = note7_to_note12[G]-1; /* Gb */ break;
            case 7: major_note12 = note7_to_note12[C]-1; /* Cb */break;
        }
    }

#define NEXT n--; if (n<0) n+=12
    int n = major_note12 + 7;
    if (n > 11) n -= 12;

    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = true; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = true; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = true; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = true; NEXT;
    note_greyed_out[n] = false; NEXT;
    note_greyed_out[n] = true;
#undef NEXT

}

}
