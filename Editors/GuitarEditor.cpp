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
#include "GUI/GLPane.h"

#include "main.h"

#include "OpenGL.h"

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
    getMainFrame()->tuningPicker->setParent(this); // standard
	getMainFrame()->tuningPicker->loadTuning(1, false); // standard
	
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

    glEnable(GL_SCISSOR_TEST);
    // glScissor doesn't seem to follow the coordinate system so this ends up in all kinds of weird code to map to my coord system (from_y going down)
    glScissor(10, getGLPane()-> getHeight() - (20+height + from_y+barHeight+20), width-15, 20+height);

    // white background
    glDisable(GL_TEXTURE_2D);

    glColor3f(1,1,1);
    glBegin(GL_QUADS);

    glVertex2f( 0, getEditorYStart());
    glVertex2f( 0, getYEnd());
    glVertex2f( getXEnd(), getYEnd());
    glVertex2f( getXEnd(), getEditorYStart());

    glEnd();

    drawVerticalMeasureLines(getEditorYStart() + first_string_position,
							 getEditorYStart() + first_string_position + (string_amount-1)*y_step);

    // ------------------------------- draw strings -------------------------------
    glColor3f(0,0,0);
    glBegin(GL_LINES);

    for(unsigned int n=0; n<tuning.size(); n++)
	{
        glVertex2f( getEditorXStart(), getEditorYStart() + first_string_position + n*y_step );
        glVertex2f( getXEnd(), getEditorYStart() + first_string_position + n*y_step );
    }

    glEnd();


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

        if(track->isNoteSelected(n) and focus) glColor3f((1-volume)*1, (1-(volume/2))*1, 0);
        else glColor3f((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);

		// note body
        glBegin(GL_QUADS);
        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);
        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+2);
        glVertex2f(x2-1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+2);
        glVertex2f(x2-1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);
        glEnd();

		// lines around note
		glColor3f(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);
        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+2);

        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+3);
        glVertex2f(x2-1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+3);

        glVertex2f(x2+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step+2);
        glVertex2f(x2+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);

        glVertex2f(x1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);
        glVertex2f(x2-1+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-2);
        glEnd();

        // fret number
        if(track->isNoteSelected(n)  and focus) glColor3f((1-volume)*1, (1-(volume/2))*1, 0);
        else glColor3f((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);

        glBegin(GL_QUADS);
        glVertex2f(x1+getEditorXStart()-1, getEditorYStart()+first_string_position+string*y_step-8);
        glVertex2f(x1+getEditorXStart()-1, getEditorYStart()+first_string_position+string*y_step);
        glVertex2f(x1+14+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step);
        glVertex2f(x1+9+getEditorXStart(), getEditorYStart()+first_string_position+string*y_step-8);
        glEnd();

        if((!track->isNoteSelected(n) or !focus) && volume>0.5) glColor3f(1, 1, 1); // if note color is too dark, draw the fret number in whites
        else glColor3f(0, 0, 0);

        // convert the fret nubmer to chars, and draw it
        glRasterPos2f(x1+getEditorXStart(), getEditorYStart() + first_string_position + string*y_step + 1);
        char buffer[2];
        sprintf (buffer, "%d", fret);

        for(int i=0; buffer[i]; i++)
		{
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, buffer[i]);
        }

    }//next


    // mouse drag
    if(!clickedOnNote)
	{

        if(mouse_is_in_editor)
		{

            // -------------------- selection ----------------------
			if(selecting)
			{

                glColor3f(0,0,0);
                glBegin(GL_LINES);

                glVertex2f(mousex_initial.getRelativeTo(WINDOW), mousey_current);
                glVertex2f(mousex_initial.getRelativeTo(WINDOW), mousey_initial);

                glVertex2f(mousex_current.getRelativeTo(WINDOW), mousey_initial);
                glVertex2f(mousex_initial.getRelativeTo(WINDOW), mousey_initial);

                glVertex2f(mousex_current.getRelativeTo(WINDOW), mousey_initial);
                glVertex2f(mousex_current.getRelativeTo(WINDOW), mousey_current);

                glVertex2f(mousex_initial.getRelativeTo(WINDOW), mousey_current);
                glVertex2f(mousex_current.getRelativeTo(WINDOW), mousey_current);

                glEnd();

            }
			// ----------------------- add note (preview) --------------------
			else if(mousey_current<getYEnd()-15)
			{

                glColor3f(1, 0.85, 0);

                const int preview_x1=
                    (int)(
                          (snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI) /*+ sequence->getXScrollInMidiTicks()*/) -
                           sequence->getXScrollInMidiTicks())*sequence->getZoom()
                          );
                const int preview_x2=
                    (int)(
                          (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) /*+ sequence->getXScrollInMidiTicks()*/) -
                           sequence->getXScrollInMidiTicks())*sequence->getZoom()
                          );

                int string = (int)round( (float)(mousey_initial - getEditorYStart() - first_string_position) / (float)y_step);

                if(!(preview_x1<0 || preview_x2<0 || string<0 || (unsigned int)string>tuning.size()-1) and preview_x2>preview_x1)
				{
                    glBegin(GL_QUADS);

                    glVertex2f(preview_x1+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5);
                    glVertex2f(preview_x1+getEditorXStart(), (string+1)*y_step + getEditorYStart() + first_string_position - 5);
                    glVertex2f(preview_x2+getEditorXStart(), (string+1)*y_step + getEditorYStart() + first_string_position - 5);
                    glVertex2f(preview_x2+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5);

                    glEnd();
                }

            }// end if selection or addition

        }// end if dragging on track

    }// if !clickedOnNote
     // ------------------------- move note (preview) -----------------------
    if(clickedOnNote)
	{

        glDisable(GL_TEXTURE_2D);

        glColor4f(1, 0.85, 0, 0.5);

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

            glBegin(GL_QUADS);
            glVertex2f(x1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step);
            glVertex2f(x1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*10);
            glVertex2f(x2-1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
            glVertex2f(x2-1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step);
            glEnd();
        }
		else
		{
            // move a bunch of notes

            for(int n=0; n<track->getNoteAmount(); n++){
                if(!track->isNoteSelected(n)) continue;

                const int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
                const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels();
                const int string=track->getNoteString(n);

                glBegin(GL_QUADS);
                glVertex2f(x1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step);
                glVertex2f(x1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
                glVertex2f(x2-1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + (y_steps_to_move+1)*y_step);
                glVertex2f(x2-1+x_steps_to_move+getEditorXStart(), string*y_step + getEditorYStart() + first_string_position - 5 + y_steps_to_move*y_step);
                glEnd();
            }//next

        }//end if lastClickedNote!=-1

    }//end if clickedOnNote

    // -----------------------------------------------------------------
    // left part with string names
    // -----------------------------------------------------------------
    glLoadIdentity();

    // grey background
    if(!focus) glColor3f(0.4, 0.4, 0.4);
    else glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_QUADS);

    glVertex2f( 0, getEditorYStart());
    glVertex2f( 0, getYEnd());
    glVertex2f( getEditorXStart()-3, getYEnd());
    glVertex2f( getEditorXStart()-3, getEditorYStart());

    glEnd();

    // string names
    glColor3f(0,0,0);

    for(int n=0; n<string_amount; n++)
	{
        glRasterPos2f(getEditorXStart()-25, getEditorYStart() + 20 + n*y_step);

        const int octave=tuning[n]/12;
        const int note=tuning[n]%12;

        switch(note){
            case 0:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'B');
                break;
            case 1:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'A');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '#');
                break;
            case 2:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'A');
                break;
            case 3:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'G');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '#');
                break;
            case 4:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'G');
                break;
            case 5:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'F');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '#');
                break;
            case 6:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'F');
                break;
            case 7:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'E');
                break;
            case 8:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'D');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '#');
                break;
            case 9:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'D');
                break;
            case 10:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'C');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, '#');
                break;
            case 11:
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, 'C');
                break;
        } // end switch

        char buffer[2];
        sprintf (buffer, "%d", 10-octave);

        for(int i=0; buffer[i]; i++)
		{
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[i]);
        }

    }//next



    glEnable(GL_TEXTURE_2D);
    glDisable(GL_SCISSOR_TEST);

}

/*
void GuitarEditor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
								 RelativeXCoord mousex_initial, int mousey_initial)
{
	Editor::mouseHeldDown(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void GuitarEditor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
							   RelativeXCoord mousex_initial,int mousey_initial)
{
	Editor::mouseExited(mousex_current, mousey_current, mousex_initial, mousey_initial);
}
*/
void GuitarEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
	if(x.getRelativeTo(EDITOR)<0 and x.getRelativeTo(EDITOR)>-75 and y>getEditorYStart())
	{
		getMainFrame()->tuningPicker->setParent(this);
		getGLPane()->PopupMenu( getMainFrame()->tuningPicker,x.getRelativeTo(WINDOW),y);
        return;
	}
    
	Editor::mouseDown(x, y);
}
/*
void GuitarEditor::mouseDrag(RelativeXCoord mousex_current, const int mousey_current,
							 RelativeXCoord mousex_initial, const int mousey_initial)
{
	Editor::mouseDrag(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void GuitarEditor::mouseUp(RelativeXCoord mousex_current, const int mousey_current,
						   RelativeXCoord mousex_initial, const int mousey_initial)
{
	Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);
}
*/

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
			
			if(track->isNoteSelected(n) and !getGLPane()->isSelectLessPressed())
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
				
	track->action( new Action::AddNote(tuning[string], snapped_start_tick, snapped_end_tick, 80, string ) );
}


}
