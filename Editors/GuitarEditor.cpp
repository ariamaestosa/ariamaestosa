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
#include "Actions/UpdateGuitarTuning.h"
#include "Actions/AddNote.h"
#include "Actions/ShiftString.h"

#include "Editors/GuitarEditor.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/TuningPicker.h"
#include "Images/ImageProvider.h"
#include "GUI/RenderUtils.h"

#include "AriaCore.h"

#include <string>

#include "Editors/RelativeXCoord.h"

namespace AriaMaestosa {

const int first_string_position = 17;
const int y_step = 10;
	
GuitarEditor::GuitarEditor(Track* track) : Editor(track)
{

    mouse_is_in_editor=false;

    clickedOnNote=false;

    lastClickedNote=-1;

	// let the tuning picker set-up the tuning of this guitar editor
    TuningPicker* picker = Core::getTuningPicker();
    picker->setParent(this); // standard
	picker->loadTuning(1, false); // standard
	
	Editor::useVerticalScrollbar(false);
}

/*
 * This is called when user changes the tuning. This tells the editor to change notes so that they match the new settings.
 */

void GuitarEditor::tuningUpdated(const bool user_triggered)
{
	if(user_triggered)
	{
		track->action( new Action::UpdateGuitarTuning() );
	}
	else
	{
		Action::UpdateGuitarTuning action;
		action.setParentTrack(track);
		action.perform();
	}
}

void GuitarEditor::render()
{
	render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

void GuitarEditor::render(RelativeXCoord mousex_current, int mousey_current, RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{

    if(!ImageProvider::imagesLoaded()) return;

    const int string_amount = tuning.size();

    AriaRender::beginScissors(10, getEditorYStart(), width-15, 20+height);

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);
    AriaRender::rect( 0, getEditorYStart(), getXEnd(), getYEnd());

    drawVerticalMeasureLines(getEditorYStart() + first_string_position,
							 getEditorYStart() + first_string_position + (string_amount-1)*y_step);

    // ------------------------------- draw strings -------------------------------
    AriaRender::color(0,0,0);

    for(unsigned int n=0; n<tuning.size(); n++)
	{
        AriaRender::line( getEditorXStart(), getEditorYStart() + first_string_position + n*y_step,
                          getXEnd(), getEditorYStart() + first_string_position + n*y_step);
    }



    // ---------------------- draw notes ----------------------------
    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
	{
        int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
        int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();

        // don't draw notes that won't visible
        if(x2<0) continue;
        if(x1>width) break;

        int string=track->getNoteString(n);
        int fret=track->getNoteFret(n);

        float volume=track->getNoteVolume(n)/127.0;

        if(track->isNoteSelected(n) and focus) AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        else AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);

        AriaRender::bordered_rect_no_start(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2,
                                           x2-1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+2);
        
        // fret number
        if(track->isNoteSelected(n)  and focus) AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        else AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);
        
        AriaRender::quad(x1+getEditorXStart()-1, getEditorYStart()+first_string_position+string*y_step-8,
                         x1+getEditorXStart()-1, getEditorYStart()+first_string_position+string*y_step,
                         x1+14+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step,
                         x1+9+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-8);
        
        if((!track->isNoteSelected(n) or !focus) && volume>0.5) AriaRender::color(1, 1, 1); // if note color is too dark, draw the fret number in white
        else AriaRender::color(0, 0, 0);
        
        // convert the fret nubmer to chars, and draw it
        char buffer[2];
        sprintf (buffer, "%d", fret);

        AriaRender::small_text(buffer, x1+getEditorXStart(), getEditorYStart() + first_string_position + string*y_step + 1);
        
    }//next
    
    
    // mouse drag
    if(!clickedOnNote and mouse_is_in_editor)
	{
        
        // -------------------- selection ----------------------
        if(selecting)
        {
            AriaRender::color(0,0,0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);
        }
        // ----------------------- add note (preview) --------------------
        else if(mousey_current<getYEnd()-15)
        {
            
            AriaRender::color(1, 0.85, 0);
            
            const int preview_x1=
                (int)(
                      (snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI)) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            const int preview_x2=
                (int)(
                      (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI)) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            
            int string = (int)round( (float)(mousey_initial - getEditorYStart() - first_string_position) / (float)y_step);
            
            if(!(preview_x1<0 || preview_x2<0 || string<0 || (unsigned int)string>tuning.size()-1) and preview_x2>preview_x1)
            {
                AriaRender::rect(preview_x1+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5,
                                 preview_x2+getEditorXStart(), (string+1)*y_step + getEditorYStart() + first_string_position - 5);
            }
            
        }// end if selection or addition
        
    }
    
    // ------------------------- move note (preview) -----------------------
    if(clickedOnNote)
	{
        
        AriaRender::color(1, 0.85, 0, 0.5);
        
        const int x_difference = mousex_current.getRelativeTo(MIDI)-mousex_initial.getRelativeTo(MIDI);
        const int y_difference = mousey_current-mousey_initial;

        const int x_steps_to_move = (int)( snapMidiTickToGrid(x_difference)*sequence->getZoom() );
		const int y_steps_to_move = (int)round( (float)y_difference / (float)y_step );

        // move a single note
        if(lastClickedNote!=-1)
		{
            const int x1=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            const int x2=track->getNoteEndInPixels(lastClickedNote) - sequence->getXScrollInPixels();
            const int string=track->getNoteString(lastClickedNote);

            AriaRender::rect(x1+x_steps_to_move+getEditorXStart(),
                             string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step,
                             x2-1+x_steps_to_move+getEditorXStart(),
                             string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
        }
		else
		{
            // move a bunch of notes

            for(int n=0; n<track->getNoteAmount(); n++)
            {
                if(!track->isNoteSelected(n)) continue;

                const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
                const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
                const int string=track->getNoteString(n);

                AriaRender::rect(x1+x_steps_to_move+getEditorXStart(),
                                 string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step,
                                 x2-1+x_steps_to_move+getEditorXStart(),
                                 string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
            }//next

        }//end if lastClickedNote!=-1

    }//end if clickedOnNote

    // -----------------------------------------------------------------
    // left part with string names
    // -----------------------------------------------------------------

    // grey background
    if(!focus) AriaRender::color(0.4, 0.4, 0.4);
    else AriaRender::color(0.8, 0.8, 0.8);
    
    AriaRender::rect( 0, getEditorYStart(),
                      getEditorXStart()-3, getYEnd());
    
    // string names
    AriaRender::color(0,0,0);

    const int text_x = getEditorXStart()-25;
    for(int n=0; n<string_amount; n++)
	{
        const int text_y = getEditorYStart() + 20 + n*y_step;
        
        const int octave=tuning[n]/12;
        const int note=tuning[n]%12;

        // FIXME - what about flats?
        
        wxString label;
        
        switch(note){
            case 0:
                label = wxT("B");
                break;
            case 1:
                label = wxT("A#");
                break;
            case 2:
                label = wxT("A");
                break;
            case 3:
                label = wxT("G#");
                break;
            case 4:
                label = wxT("G");
                break;
            case 5:
                label = wxT("F#");
                break;
            case 6:
                label = wxT("F");
                break;
            case 7:
                label = wxT("E");
                break;
            case 8:
                label = wxT("D#");
                break;
            case 9:
                label = wxT("D");
                break;
            case 10:
                label = wxT("C#");
                break;
            case 11:
                label = wxT("C");
                break;
        } // end switch

        label += to_wxString(10-octave);
        
        AriaRender::small_text( label.mb_str(), text_x, text_y );
    }//next


    AriaRender::endScissors();
    AriaRender::images();

}

void GuitarEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
	if(x.getRelativeTo(EDITOR)<0 and x.getRelativeTo(EDITOR)>-75 and y>getEditorYStart())
	{
        Core::getTuningPicker()->setParent(this);
        Display::popupMenu(Core::getTuningPicker(),x.getRelativeTo(WINDOW),y);
        return;
	}
    
	Editor::mouseDown(x, y);
}

void GuitarEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial)
{
	for(int n=0; n<track->getNoteAmount(); n++)
	{
		const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels(); // on-screen pixel where note starts
		const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels(); // on-screen pixel where note ends

		const int string=track->getNoteString(n);

		// check if note is within selection boundaries
		if(x1 > std::min(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
		   x2 < std::max(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
		   std::min(mousey_current, mousey_initial) < getEditorYStart() + first_string_position + string*y_step and
		   std::max(mousey_current, mousey_initial) > getEditorYStart() + first_string_position + string*y_step)
		{
			track->selectNote(n, true);
		}
		else
		{
			track->selectNote(n, false);
		}
	}//next
}

void GuitarEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if(note.startTick+relativeX < 0) return; // refuse to move before song start
    
    note.startTick += relativeX;
    note.endTick   += relativeX;
    
    if(note.string+relativeY<0 or note.string+relativeY > (int)tuning.size()-1)
        return; // note will end on a string that doesn't exist if we move it like that
    
    note.string += relativeY;
    note.findNoteFromStringAndFret();
}


GuitarEditor::~GuitarEditor()
{
}

NoteSearchResult GuitarEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
	const int x_edit = x.getRelativeTo(EDITOR);
	
	const int noteAmount = track->getNoteAmount();
	// iterate through note n reverse order (last drawn note appears on top and must be first selected)
	for(int n=noteAmount-1; n>-1; n--)
	{
		const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
		const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
		
		const int string=track->getNoteString(n);
		
		if(x_edit > x1 and
		   x_edit < x2 and
		   y < getEditorYStart() + first_string_position + string*y_step + 4 and
		   y > getEditorYStart() + first_string_position + string*y_step - 5)
		{
			
			noteID = n;
			
			if(track->isNoteSelected(n) and !Display:: isSelectLessPressed())
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
void GuitarEditor::noteClicked(const int id)
{
	track->selectNote(ALL_NOTES, false);
	track->selectNote(id, true);
	track->playNote(id);
}
void GuitarEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
	int string = (int)round( (float)(mouseY - getEditorYStart() - first_string_position) / (float)y_step );
	
	if(string<0) return; //invalid note, don't add it
	if(string>=(int)tuning.size()) return; //invalid note, don't add it
				
	track->action( new Action::AddNote(tuning[string], snapped_start_tick, snapped_end_tick, default_volume, string ) );
}


}
