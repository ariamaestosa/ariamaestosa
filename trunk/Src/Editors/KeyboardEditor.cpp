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
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/MagneticGrid.h"
#include "Pickers/KeyPicker.h"
#include "GUI/GraphicalTrack.h"
#include "Renderers/RenderAPI.h"

#include "AriaCore.h"



using namespace AriaMaestosa;


// ************************************************************************************************************
// *********************************************    CTOR/DTOR      ********************************************
// ************************************************************************************************************
#if 0
#pragma mark Ctor/dtor
#endif

KeyboardEditor::KeyboardEditor(Track* track) : Editor(track)
{
    loadKey(SHARP, 0);
    sb_position = 0.5;
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
    if (x.getRelativeTo(EDITOR)<-30 and x.getRelativeTo(WINDOW)>15 and y>getEditorYStart())
    {
        KeyPicker* picker = Core::getKeyPicker();
        picker->setParent(track);
        Display::popupMenu(picker, x.getRelativeTo(WINDOW), y);
        return;
    }
    // user clicked on a keyboard key
    else if (x.getRelativeTo(EDITOR)<0 and x.getRelativeTo(EDITOR)>-30 and y>getEditorYStart())
    {
        const int pitchID = getLevelAtY(y);
        PlatformMidiManager::playNote( 131-pitchID, default_volume, 500, 0, track->getInstrument() );
        return;
    }
    
    Editor::mouseDown(x, y);
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
    track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, default_volume ) );
}

// -----------------------------------------------------------------------------------------------------------
    
void KeyboardEditor::noteClicked(const int id)
{
    track->selectNote(ALL_NOTES, false);
    track->selectNote(id, true);
    track->playNote(id);
}

// -----------------------------------------------------------------------------------------------------------

NoteSearchResult KeyboardEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int x_edit = x.getRelativeTo(EDITOR);

    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
        const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
        const int y1=track->getNotePitchID(n)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels();

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

// -----------------------------------------------------------------------------------------------------------

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
            from_note*Y_STEP_HEIGHT+getEditorYStart()-getYScrollInPixels() > std::min(mousey_current, mousey_initial) and
            from_note*Y_STEP_HEIGHT+getEditorYStart()-getYScrollInPixels() < std::max(mousey_current, mousey_initial) )
        {

            track->selectNote(n, true);

        }else{
            track->selectNote(n, false);
        }
    }//next

}

// -----------------------------------------------------------------------------------------------------------
    
int KeyboardEditor::getYScrollInPixels()
{
    return (int)(   sb_position*(120*11-height-20)   );
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if (note.startTick+relativeX < 0)    return; // refuse to move before song start
    if (note.pitchID+relativeY   < 0)    return; // reject moves that would make illegal notes
    if (note.pitchID+relativeY   >= 128) return;

    note.startTick += relativeX;
    note.endTick   += relativeX;
    note.pitchID   += relativeY;
}

// ************************************************************************************************************
// ****************************************    RENDER      ****************************************************
// ************************************************************************************************************

#if 0
#pragma mark -
#pragma mark Render
#endif

void KeyboardEditor::render()
{
    render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

// -----------------------------------------------------------------------------------------------------------

void KeyboardEditor::render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{

    if (!ImageProvider::imagesLoaded()) return;

    AriaRender::beginScissors(10, getEditorYStart(), width-15, 20+height);

    // ------------------ draw lined background ----------------

    int levelid = getYScrollInPixels()/Y_STEP_HEIGHT;
    const int yscroll = getYScrollInPixels();
    const int y1 = getEditorYStart();
    const int last_note = ( yscroll + getYEnd() - getEditorYStart() )/Y_STEP_HEIGHT;
    const int x1 = getEditorXStart();
    const int x2 = getXEnd();

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);
    AriaRender::rect(x1, getEditorYStart(), x2, getYEnd());

    // horizontal lines
    AriaRender::color(0.94, 0.94, 0.94, 1);
    while (levelid < last_note)
    {
        //const int note12 = 11 - ((levelid - 3) % 12);
        const int pitchID = levelid; //FIXME: fix this conflation of level and pitch ID. it's handy in keyboard
                                     // editor, but a pain everywhere else...
        if (m_note_greyed_out[pitchID])
        {
            AriaRender::rect(x1, levelToY(levelid),
                             x2, levelToY(levelid+1));
        }
        else
        {
            AriaRender::line(x1, levelToY(levelid+1),
                             x2, levelToY(levelid+1));
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
        for (int bgtrack=0; bgtrack<amount; bgtrack++)
        {
            Track* track = backgroundTracks.get(bgtrack);
            const int noteAmount = track->getNoteAmount();

            // pick a color
            switch(color)
            {
                case 0: AriaRender::color(1, 0.85, 0, 0.5); break;
                case 1: AriaRender::color(0, 1, 0, 0.5);    break;
                case 2: AriaRender::color(1, 0, 0.85, 0.5); break;
                case 3: AriaRender::color(1, 0, 0, 0.5);    break;
                case 4: AriaRender::color(0, 0.85, 1, 0.5); break;
            }
            color++; if (color>4) color = 0;

            // render the notes
            for (int n=0; n<noteAmount; n++)
            {

                int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
                int x2=track->getNoteEndInPixels(n)   - sequence->getXScrollInPixels();

                // don't draw notes that won't be visible
                if (x2 < 0)     continue;
                if (x1 > width) break;

                const int pitch = track->getNotePitchID(n);

                AriaRender::rect(x1+getEditorXStart(),   levelToY(pitch),
                                 x2+getEditorXStart()-1, levelToY(pitch+1));
            }

        }
    }

    // ---------------------- draw notes ----------------------------
    const int noteAmount = track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {

        int x1 = track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
        int x2 = track->getNoteEndInPixels(n)   - sequence->getXScrollInPixels();

        // don't draw notes that won't be visible
        if (x2 < 0)     continue;
        if (x1 > width) break;

        int   y     = track->getNotePitchID(n);
        float volume = track->getNoteVolume(n)/127.0;

        if (track->isNoteSelected(n) and focus)
        {
            AriaRender::color((1-volume)*1,   (1-(volume/2))*1, 0);
        }
        else
        {
            AriaRender::color((1-volume)*0.9, (1-volume)*0.9,  (1-volume)*0.9);
        }

        AriaRender::bordered_rect(x1+getEditorXStart()+1, levelToY(y),
                                  x2+getEditorXStart()-1, levelToY(y+1));
    }


    // ------------------ draw keyboard ----------------

    // grey background
    if (not focus) AriaRender::color(0.4, 0.4, 0.4);
    else           AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0, getEditorYStart(), getEditorXStart()-25,  getYEnd());

    for (int g_octaveID=0; g_octaveID<11; g_octaveID++)
    {
        int g_octave_y=g_octaveID*120-getYScrollInPixels();
        if (g_octave_y>-120 and g_octave_y<height+20)
        {
            AriaRender::images();

            if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
            else AriaRender::setImageState(AriaRender::STATE_NORMAL);

            noteTrackDrawable->move(getEditorXStart()-noteTrackDrawable->getImageWidth(),
                                    from_y+barHeight+20 + g_octave_y);
            noteTrackDrawable->render();

            AriaRender::primitives();
            AriaRender::color(0,0,0);

            // FIXME - all those from_y+barHeight+20 stuff needs to be cleaned up for good
            AriaRender::line(0, from_y+barHeight+20 + g_octave_y+1,
                             getEditorXStart()-25, from_y+barHeight+20 + g_octave_y+1);

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
                AriaRender::rect(preview_x1+getEditorXStart(),
                                 ((mousey_initial - getEditorYStart() + getYScrollInPixels())/Y_STEP_HEIGHT)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels(),
                                 preview_x2+getEditorXStart(),
                                 ((mousey_initial - getEditorYStart() + getYScrollInPixels())/Y_STEP_HEIGHT)*Y_STEP_HEIGHT+Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels());
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
        const int y_step_move = (int)round( (float)y_difference/ (float)Y_STEP_HEIGHT );

        // move a single note
        if (lastClickedNote!=-1)
        {
            int x1=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            int x2=track->getNoteEndInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            int y=track->getNotePitchID(lastClickedNote);

            AriaRender::rect(x1+x_step_move+getEditorXStart(),
                             (y+y_step_move)*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels(),
                             x2-1+x_step_move+getEditorXStart(),
                             (y+y_step_move+1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels());
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

                AriaRender::rect(x1+x_step_move+getEditorXStart(),
                                 (y+y_step_move)*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels(),
                                 x2-1+x_step_move+getEditorXStart(),
                                 (y+y_step_move+1)*Y_STEP_HEIGHT + getEditorYStart() - getYScrollInPixels());
            }//next

        }

    }

    // ---------------------------- scrollbar -----------------------
    // FIXME - instead implement renderScrollbar(focus)...
    if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else        AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::color(1,1,1);
    AriaRender::endScissors();

}

// ************************************************************************************************************
// ****************************************    SPECIFIC      **************************************************
// ************************************************************************************************************

#if 0
#pragma mark -
#pragma mark Keyroll-specific
#endif

void KeyboardEditor::loadKey(const PitchSign sharpness_symbol, const int symbol_amount)
{

    /*
     static bool findNoteName(const int pitchID, Note12* note_12, int* octave);
     */
    
    // if key is e.g. G Major, "major_note" will be set to note12 equivalent of G.
    // to load a minor key, it's just set to the major one that has same sharps and flats
    // to ease the process
    Note12 major_note12 = NOTE_12_C;

    if (symbol_amount == 0 or sharpness_symbol == NATURAL)
    {
        major_note12 = NOTE_12_C;
    }
    else if (sharpness_symbol == SHARP)
    {
        switch (symbol_amount)
        {
            case 1: major_note12 = NOTE_12_G;       break;
            case 2: major_note12 = NOTE_12_D;       break;
            case 3: major_note12 = NOTE_12_A;       break;
            case 4: major_note12 = NOTE_12_E;       break;
            case 5: major_note12 = NOTE_12_B;       break;
            case 6: major_note12 = NOTE_12_F_SHARP; break;
            case 7: major_note12 = NOTE_12_C_SHARP; break;
        }
    }
    else if (sharpness_symbol == FLAT)
    {
        switch(symbol_amount)
        {
            case 1: major_note12 = NOTE_12_F;      break;
            case 2: major_note12 = NOTE_12_B_FLAT; break;
            case 3: major_note12 = NOTE_12_E_FLAT; break;
            case 4: major_note12 = NOTE_12_A_FLAT; break;
            case 5: major_note12 = NOTE_12_D_FLAT; break;
            case 6: major_note12 = NOTE_12_G_FLAT; break;
            case 7: major_note12 = NOTE_12_B;      break; // C flat
        }
    }

    bool note_12_greyed_out[12];
    
#define NEXT n--; if (n<0) n+=12
    int n = int(major_note12) + 7;
    if (n > 11) n -= 12;

    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;
#undef NEXT

    Note12 noteName;
    int octave;
    
    for (int n=0; n<131; n++)
    {
        if (findNoteName(n, &noteName, &octave))
        {
            m_note_greyed_out[n] = note_12_greyed_out[noteName];
        }
        else
        {
            m_note_greyed_out[n] = true;
        }
    }
    
}

