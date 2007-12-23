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

#include "OpenGL.h"

#include "Actions/EditAction.h"
#include "Actions/AddNote.h"
#include "Actions/MoveNotes.h"

#include "GUI/GLPane.h"
#include "GUI/GraphicalTrack.h"
#include "Images/Drawable.h"
#include "Images/Image.h"
#include "Images/ImageProvider.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Editors/DrumEditor.h"
#include "Editors/RelativeXCoord.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/MagneticGrid.h"
#include "main.h"

namespace AriaMaestosa {
	
const int y_step = 10;
	
DrumInfo::DrumInfo(int midiKey, char* name)
{
    DrumInfo::midiKey = midiKey;
    DrumInfo::name = name;
    if(midiKey == -1 ) section=true;
    else section = false;
    
    sectionExpanded=true;
}
// ***********************************************************************************************************************************************************
// **********************************************************    CONSTRUCTOR      ****************************************************************************
// ***********************************************************************************************************************************************************

DrumEditor::DrumEditor(Track* track) : Editor(track)
{
	
    sb_position=0;
    mouse_is_in_editor=false;
    clickedOnNote=false;
    lastClickedNote = -1;
    showUsedDrumsOnly=false;
	
    useDefaultDrumSet();
	Editor::useInstantNotes();
}

DrumEditor::~DrumEditor()
{
	// FIXME- may need to delete menu!
}

void DrumEditor::useCustomDrumSet()
{
	
	bool inuse[128];
	for(int n=0; n<128; n++)
	{
		inuse[n]=false;
	}
	
	const int drum_amount = track->getNoteAmount();
	for(int drumID=0; drumID<drum_amount; drumID++)
	{
		inuse[ track->getNotePitchID(drumID) ] = true;
	}
		
	drums.clear();
	
	if(inuse[36] or inuse[35] or inuse[38] or inuse[40] or inuse[37])
	{
		drums.push_back( DrumInfo(-1, "Drumkit")); 
		if(inuse[36]) drums.push_back( DrumInfo(36, "Bass drum 1"));
		if(inuse[35]) drums.push_back( DrumInfo(35, "Bass drum 2")); 
		if(inuse[38]) drums.push_back( DrumInfo(38, "Snare"));
		if(inuse[40]) drums.push_back( DrumInfo(40, "Snare 2"));
		if(inuse[37]) drums.push_back( DrumInfo(37, "Stick")); 
	}
	
	if(inuse[42] or inuse[46] or inuse[44])
	{
		drums.push_back( DrumInfo(-1, "Hi-hat")); 
		if(inuse[42]) drums.push_back( DrumInfo(42, "Closed hi-hat"));
		if(inuse[46]) drums.push_back( DrumInfo(46, "Open hi-hat"));
		if(inuse[44]) drums.push_back( DrumInfo(44, "Pedal hi-hat"));
	}
	
	if(inuse[49] or inuse[57] or inuse[55] or inuse[52] or inuse[51] or inuse[59] or inuse[53])
	{
		drums.push_back( DrumInfo(-1, "Cymbal")); 
		if(inuse[49]) drums.push_back( DrumInfo(49, "Crash"));
		if(inuse[57]) drums.push_back( DrumInfo(57, "Crash 2"));
		if(inuse[55]) drums.push_back( DrumInfo(55, "Splash"));
		if(inuse[52]) drums.push_back( DrumInfo(52, "Chinese"));
		if(inuse[51]) drums.push_back( DrumInfo(51, "Ride"));
		if(inuse[59]) drums.push_back( DrumInfo(59, "Ride 2"));
		if(inuse[53]) drums.push_back( DrumInfo(53, "Ride bell"));
	}
	
	if(inuse[41] or inuse[43] or inuse[45] or inuse[47] or inuse[48] or inuse[50])
	{
		drums.push_back( DrumInfo(-1, "Toms"));
		if(inuse[41]) drums.push_back( DrumInfo(41, "Tom 1"));
		if(inuse[43]) drums.push_back( DrumInfo(43, "Tom 2"));
		if(inuse[45]) drums.push_back( DrumInfo(45, "Tom 3"));
		if(inuse[47]) drums.push_back( DrumInfo(47, "Tom 4"));
		if(inuse[48]) drums.push_back( DrumInfo(48, "Tom 5"));
		if(inuse[50]) drums.push_back( DrumInfo(50, "Tom 6"));
	}
	
	if(inuse[76] or inuse[77] or inuse[69] or inuse[67] or inuse[68] or inuse[58] or inuse[62] or inuse[63] or inuse[64])
	{
		drums.push_back( DrumInfo(-1, "African")); 
		if(inuse[76]) drums.push_back( DrumInfo(76, "Hi wood block"));
		if(inuse[77]) drums.push_back( DrumInfo(77, "Lo wood block"));
		if(inuse[69]) drums.push_back( DrumInfo(69, "Cabasa"));
		if(inuse[67]) drums.push_back( DrumInfo(67, "High agogo"));
		if(inuse[68]) drums.push_back( DrumInfo(68, "Low agogo"));
		if(inuse[58]) drums.push_back( DrumInfo(58, "Vibraslap"));
		if(inuse[62]) drums.push_back( DrumInfo(62, "Mute hi conga"));
		if(inuse[63]) drums.push_back( DrumInfo(63, "Open hi conga"));
		if(inuse[64]) drums.push_back( DrumInfo(64, "Low conga"));
	}
	
	if(inuse[73] or inuse[74] or inuse[75] or inuse[78] or inuse[79] or inuse[70] or inuse[56] or inuse[60] or inuse[61] or inuse[85] or inuse[86] or inuse[87])
	{
		drums.push_back( DrumInfo(-1, "Latin")); 
		if(inuse[73]) drums.push_back( DrumInfo(73, "Short guiro"));
		if(inuse[74]) drums.push_back( DrumInfo(74, "Long guiro"));
		if(inuse[75]) drums.push_back( DrumInfo(75, "Claves"));
		if(inuse[78]) drums.push_back( DrumInfo(78, "Mute cuica"));
		if(inuse[79]) drums.push_back( DrumInfo(79, "Open cuica"));
		if(inuse[70]) drums.push_back( DrumInfo(70, "Maracas"));
		if(inuse[56]) drums.push_back( DrumInfo(56, "Cowbell")); 
		if(inuse[60]) drums.push_back( DrumInfo(60, "Hi bongo"));
		if(inuse[61]) drums.push_back( DrumInfo(61, "Low bongo"));
		if(inuse[85]) drums.push_back( DrumInfo(85, "Castanets"));
		if(inuse[86]) drums.push_back( DrumInfo(86, "Mute surdo"));
		if(inuse[87]) drums.push_back( DrumInfo(87, "Open surdo"));
	}
	
	if(inuse[54] or inuse[65] or inuse[66] or inuse[71] or inuse[72] or inuse[80] or inuse[81] or inuse[82] or inuse[83] or inuse[84] or inuse[31])
	{
		drums.push_back( DrumInfo(-1, "Others")); 
		if(inuse[54]) drums.push_back( DrumInfo(54, "Tambourine"));
		if(inuse[65]) drums.push_back( DrumInfo(65, "High timbale"));
		if(inuse[66]) drums.push_back( DrumInfo(66, "Low timbale"));
		if(inuse[71]) drums.push_back( DrumInfo(71, "Short whistle"));
		if(inuse[72]) drums.push_back( DrumInfo(72, "Long whistle"));
		if(inuse[80]) drums.push_back( DrumInfo(80, "Mute triangle"));
		if(inuse[81]) drums.push_back( DrumInfo(81, "Open triangle"));
		if(inuse[82]) drums.push_back( DrumInfo(82, "Shaker"));
		if(inuse[83]) drums.push_back( DrumInfo(83, "Jingle Bell"));
		if(inuse[84]) drums.push_back( DrumInfo(84, "Bell Tree"));
		if(inuse[31]) drums.push_back( DrumInfo(31, "Stick"));
	}
	
	if(inuse[34] or inuse[33] or inuse[32] or inuse[30] or inuse[29] or inuse[28] or inuse[27] or inuse[39])
	{
		drums.push_back( DrumInfo(-1, "Sound effects")); 
		if(inuse[34]) drums.push_back( DrumInfo(34, "Metro bell"));
		if(inuse[33]) drums.push_back( DrumInfo(33, "Metro"));
		if(inuse[32]) drums.push_back( DrumInfo(32, "Square"));
		if(inuse[30]) drums.push_back( DrumInfo(30, "Pull"));
		if(inuse[29]) drums.push_back( DrumInfo(29, "Push"));
		if(inuse[28]) drums.push_back( DrumInfo(28, "Slap"));
		if(inuse[27]) drums.push_back( DrumInfo(27, "High Q"));
		if(inuse[39]) drums.push_back( DrumInfo(39, "Clap"));
	}
	
    // prepare midiKeyToVectorID
    for(int n=0; n<128; n++)
        midiKeyToVectorID[n]=-1;
    
    for(unsigned int n=0; n<drums.size(); n++)
        midiKeyToVectorID[ drums[n].midiKey ]=n;
}

void DrumEditor::useDefaultDrumSet()
{
	drums.clear();
	
    drums.push_back( DrumInfo(-1, "Drumkit")); 
    drums.push_back( DrumInfo(36, "Bass drum 1"));
    drums.push_back( DrumInfo(35, "Bass drum 2")); 
    drums.push_back( DrumInfo(38, "Snare"));
    drums.push_back( DrumInfo(40, "Snare 2"));
    drums.push_back( DrumInfo(37, "Stick")); 
    
    drums.push_back( DrumInfo(-1, "Hi-hat")); 
    drums.push_back( DrumInfo(42, "Closed hi-hat"));
    drums.push_back( DrumInfo(46, "Open hi-hat"));
	drums.push_back( DrumInfo(44, "Pedal hi-hat"));
	
	drums.push_back( DrumInfo(-1, "Cymbal")); 
    drums.push_back( DrumInfo(49, "Crash"));
	drums.push_back( DrumInfo(57, "Crash 2"));
	drums.push_back( DrumInfo(55, "Splash"));
    drums.push_back( DrumInfo(52, "Chinese"));
    drums.push_back( DrumInfo(51, "Ride"));
	drums.push_back( DrumInfo(59, "Ride 2"));
	drums.push_back( DrumInfo(53, "Ride bell"));
	
    drums.push_back( DrumInfo(-1, "Toms"));
    drums.push_back( DrumInfo(41, "Tom 1"));
    drums.push_back( DrumInfo(43, "Tom 2"));
    drums.push_back( DrumInfo(45, "Tom 3"));
    drums.push_back( DrumInfo(47, "Tom 4"));
    drums.push_back( DrumInfo(48, "Tom 5"));
    drums.push_back( DrumInfo(50, "Tom 6"));
    
    drums.push_back( DrumInfo(-1, "African")); 
    drums.push_back( DrumInfo(76, "Hi wood block"));
    drums.push_back( DrumInfo(77, "Lo wood block"));
    drums.push_back( DrumInfo(69, "Cabasa"));
    drums.push_back( DrumInfo(67, "High agogo"));
    drums.push_back( DrumInfo(68, "Low agogo"));
    drums.push_back( DrumInfo(58, "Vibraslap"));
    drums.push_back( DrumInfo(62, "Mute hi conga"));
    drums.push_back( DrumInfo(63, "Open hi conga"));
    drums.push_back( DrumInfo(64, "Low conga"));
    
    drums.push_back( DrumInfo(-1, "Latin")); 
    drums.push_back( DrumInfo(73, "Short guiro"));
    drums.push_back( DrumInfo(74, "Long guiro"));
    drums.push_back( DrumInfo(75, "Claves"));
    drums.push_back( DrumInfo(78, "Mute cuica"));
    drums.push_back( DrumInfo(79, "Open cuica"));
    drums.push_back( DrumInfo(70, "Maracas"));
    drums.push_back( DrumInfo(56, "Cowbell")); 
    drums.push_back( DrumInfo(60, "Hi bongo"));
    drums.push_back( DrumInfo(61, "Low bongo"));
    drums.push_back( DrumInfo(85, "Castanets"));
    drums.push_back( DrumInfo(86, "Mute surdo"));
    drums.push_back( DrumInfo(87, "Open surdo"));
    
    drums.push_back( DrumInfo(-1, "Others")); 
    drums.push_back( DrumInfo(54, "Tambourine"));
    drums.push_back( DrumInfo(65, "High timbale"));
    drums.push_back( DrumInfo(66, "Low timbale"));
    drums.push_back( DrumInfo(71, "Short whistle"));
    drums.push_back( DrumInfo(72, "Long whistle"));
    drums.push_back( DrumInfo(80, "Mute triangle"));
    drums.push_back( DrumInfo(81, "Open triangle"));
    drums.push_back( DrumInfo(82, "Shaker"));
    drums.push_back( DrumInfo(83, "Jingle Bell"));
    drums.push_back( DrumInfo(84, "Bell Tree"));
    drums.push_back( DrumInfo(31, "Stick"));
    
    drums.push_back( DrumInfo(-1, "Sound effects")); 
    drums.push_back( DrumInfo(34, "Metro bell"));
    drums.push_back( DrumInfo(33, "Metro"));
    drums.push_back( DrumInfo(32, "Square"));
    drums.push_back( DrumInfo(30, "Pull"));
    drums.push_back( DrumInfo(29, "Push"));
    drums.push_back( DrumInfo(28, "Slap"));
    drums.push_back( DrumInfo(27, "High Q"));
    drums.push_back( DrumInfo(39, "Clap"));
	
	
    // prepare midiKeyToVectorID
    for(int n=0; n<128; n++)
        midiKeyToVectorID[n]=-1;
    
    for(unsigned int n=0; n<drums.size(); n++)
        midiKeyToVectorID[ drums[n].midiKey ]=n;
}



// *******************************************************************************************************************************************************
// ***********************************************************    EDITOR      ****************************************************************************
// *******************************************************************************************************************************************************

NoteSearchResult DrumEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
	const int noteAmount = track->getNoteAmount();
	for(int n=0; n<noteAmount; n++)
	{
		const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
		
		assert(track->getNotePitchID(n)>0);
		assert(track->getNotePitchID(n)<128);
		
		const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
		if(drumIDInVector == -1)
		{
			continue;
		}
		
		const int drumy=getYForDrum(drumIDInVector);
		
		if(x.getRelativeTo(EDITOR)>drumx-1 and x.getRelativeTo(EDITOR)<drumx+5 and y>drumy and y<drumy+y_step)
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
		
	}//next note
	
	return FOUND_NOTHING;
}

void DrumEditor::noteClicked(const int id)
{
	track->selectNote(ALL_NOTES, false);
	track->selectNote(id, true);
	track->playNote(id);
}
void DrumEditor::addNote(const int snappedX, const int mouseY)
{
	const int drumID = getDrumAtY(mouseY); // FIXME - double checks? something very similar is also checked in MouseDown
    
	if(drumID < 0) return;
	if(drumID > (int)drums.size()-1) return;
    
	const int note = drums[ drumID ].midiKey;
    if(note == -1) return;

	track->action( new Action::AddNote(note, snappedX, snappedX+sequence->ticksPerBeat()/32+1, 80 ) );
}

void DrumEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if(note.startTick+relativeX < 0) return; // refuse to move before song start
    
    note.startTick += relativeX;
    note.endTick   += relativeX;
    
    if(relativeY==0) return;
    
    /*
     *  Since the drums are not in midi order on screen, the procedure to move drums up or down is to:
     *      1. Find the screen location of this midi key
     *      2. Move the drum in screen location
     *      3. Find the new midi key at the new screen location
     */
    
    assert(note.pitchID>=0);
    assert(note.pitchID<128);
    
    // find where on screen this particular drum is drawn (their screen position is not in the same order as the midi order)
    int newVectorLoc = midiKeyToVectorID[note.pitchID];
    if(newVectorLoc == -1) return;
    
    // move to the new location (in screen order)
    if(newVectorLoc + relativeY < 0 or
       newVectorLoc + relativeY > (int)drums.size()-1 ) return; // discard moves that would result in an out-of-bound note
    
    newVectorLoc += relativeY;
    
    // skip sections
    while(drums[newVectorLoc].midiKey==-1)
    {
        
        newVectorLoc+=(relativeY/abs(relativeY)); // keep the same sign, but only move 1 step at a time from now on
        
        // discard moves that would result in an out-of-bound note
        if(newVectorLoc < 0 or newVectorLoc > (int)drums.size()-1 )
            return;
    }
    
    // find the midi key at the new location
    const int new_pitchID = drums[newVectorLoc].midiKey;
    if(new_pitchID < 0 or new_pitchID > 127)
        return; // invalid location - discard
    
    note.pitchID = new_pitchID;
}

void DrumEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial)
{
	for(int n=0; n<track->getNoteAmount(); n++)
	{
		const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();
		
		assert(track->getNotePitchID(n)>0);
		assert(track->getNotePitchID(n)<128);
		
		const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
		if(drumIDInVector == -1) continue;
		
		const int drumy=getYForDrum(drumIDInVector) + 5;
		
		if(drumx>std::min(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
		   drumx<std::max(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
		   drumy > std::min(mousey_current, mousey_initial) and
		   drumy < std::max(mousey_current, mousey_initial))
		{
			track->selectNote(n, true);
		}
		else
		{
			track->selectNote(n, false);
		}
		
	}//next note
}

int DrumEditor::getYScrollInPixels()
{
    // check if visible area is large enough to display everything
	if((int)(drums.size()*y_step) < height)
	{
		useVerticalScrollbar(false);
		return 0;
	}
	else
	{
		useVerticalScrollbar(true);
	}
    
    return (int)(   sb_position*(drums.size()*y_step-height )   );
}

// *******************************************************************************************************************************************************
// ***********************************************************    EVENTS      ****************************************************************************
// *******************************************************************************************************************************************************
void DrumEditor::mouseDown(RelativeXCoord x, const int y)
{
	
	const int drumID = getDrumAtY(y);
	
	if(drumID < 0) return;
	if(drumID > (int)drums.size()-1) return;
	
	const int note = drums[ drumID ].midiKey;
	
	if(note==-1)
	{
		// user clicked on a section. if click is on the triangle, expand/collapse it. otherwise, it just selects nothing.
		if(x.getRelativeTo(EDITOR) < 25 and x.getRelativeTo(EDITOR) > 0)
        {
			drums[ drumID ].sectionExpanded = !drums[ drumID ].sectionExpanded;
            return;
        }
		else
			track->selectNote(ALL_NOTES, false, true);
	}
	
	Editor::mouseDown(x, y);
    
}

void DrumEditor::mouseUp(RelativeXCoord mousex_current, const int mousey_current,
						 RelativeXCoord mousex_initial, const int mousey_initial)
{
	Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);
    
	// ------------------- toggle "only show used drums" widget -----------------
	if(mousex_current.getRelativeTo(WINDOW) > getEditorXStart()-77 and
	   mousex_current.getRelativeTo(WINDOW) < getEditorXStart()-67 and
	   mousey_current - getYScrollInPixels() > getEditorYStart()+1 and
	   mousey_current - getYScrollInPixels() < getEditorYStart()+10)
	{
		if(track->getNoteAmount()<1) return;
		
		showUsedDrumsOnly = !showUsedDrumsOnly;
		
		if(showUsedDrumsOnly) useCustomDrumSet();
		else useDefaultDrumSet();
		render();
	}
	
}

// ***********************************************************************************************************************************************************
// ************************************************************    RENDER      *******************************************************************************
// ***********************************************************************************************************************************************************

void DrumEditor::render()
{
	render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

void DrumEditor::render(RelativeXCoord mousex_current, int mousey_current,
						RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    
    assert(sbArrowDrawable->image!=NULL);
    assert(sbBackgDrawable->image!=NULL);
    assert(sbBarDrawable->image!=NULL);
    
    glEnable(GL_SCISSOR_TEST);
    // glScissor doesn't seem to follow the coordinate system so this ends up in all kinds of weird code to map to my coord system (from_y going down)
    glScissor(10, getGLPane()-> getHeight() - (20+height + from_y+barHeight+20), width - 15, 20+height);
        
    drawVerticalMeasureLines(getEditorYStart(), getYEnd());
    
    // ----------------------- draw horizontal lines ---------------------
    glColor3f(0.5, 0.5, 0.5);
    const int drumAmount = drums.size();
    for(int drumID=0; drumID<drumAmount+1; drumID++)
	{
        const int y = getEditorYStart() + drumID*y_step - getYScrollInPixels();
        if(y<getEditorYStart() or y>getYEnd()) continue;
        
        glBegin(GL_LINES);
        
        glVertex2f(getEditorXStart(), y);
        glVertex2f(getXEnd(), y);
        
        glEnd();
        
    }
	

    // ---------------------- draw notes ----------------------------
    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
	{
        const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();

        // don't draw notes that won't visible
        if(drumx<0) continue;
        if(drumx>width) break;
        
        assert(track->getNotePitchID(n)>=0);
        assert(track->getNotePitchID(n)<128);
        
        const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
        if(drumIDInVector == -1)
		{
            //std::cout << "WARNING: a -1 drum event was found.\n";
            continue;
        }
        const float volume=track->getNoteVolume(n)/127.0;
        
        if(track->isNoteSelected(n) and focus) glColor3f((1-volume)*1, (1-(volume/2))*1, 0);
        else glColor3f((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);
        
        const int drumy=getYForDrum(drumIDInVector);
        
        glBegin(GL_TRIANGLES);
        
        glVertex2f(drumx, drumy);
        glVertex2f(drumx, drumy+y_step);
        glVertex2f(drumx+5, drumy+5);
        
        glEnd();
        
    }
    
    
    glEnable(GL_TEXTURE_2D);
    
    // ------------------------- mouse drag (preview) ------------------------
    if(!clickedOnNote)
	{
        if(mouse_is_in_editor)
		{
            
            // selection
            if(selecting)
			{
                glDisable(GL_TEXTURE_2D);
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
                glEnable(GL_TEXTURE_2D);
            }// end if selection or addition
        }// end if dragging on track
        
    } // end if !clickedOnNote

    // ------------------------- move note (preview) -----------------------
    if(clickedOnNote)
	{
        
        glDisable(GL_TEXTURE_2D);
        
        glColor4f(1, 0.85, 0, 0.5);
        
        const int x_difference = mousex_current.getRelativeTo(MIDI)-mousex_initial.getRelativeTo(MIDI);
        const int y_difference = mousey_current-mousey_initial;
        
        const int x_steps_to_move = (int)( snapMidiTickToGrid(x_difference)*sequence->getZoom() );
        const int y_steps_to_move = (int)round(y_difference/ (float)y_step );
            
        // move a single note
        if(lastClickedNote != -1)
		{
			const int drumx=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorXStart();
			
			const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(lastClickedNote) ];
			if(drumIDInVector != -1)
			{
				
				const int drumy=getYForDrum(drumIDInVector);
				
				glBegin(GL_TRIANGLES);
				glVertex2f(drumx + x_steps_to_move, drumy + y_steps_to_move*y_step);
				glVertex2f(drumx + x_steps_to_move, drumy + (y_steps_to_move+1)*y_step);
				glVertex2f(drumx + 5 + x_steps_to_move, drumy + y_step/2 + y_steps_to_move*y_step);
				glEnd();
			}
            
        }
		else
		{
            // move a bunch of notes

            for(int n=0; n<noteAmount; n++)
			{
                if(!track->isNoteSelected(n)) continue;
                
                const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();
                
                assert(track->getNotePitchID(n)>0);
                assert(track->getNotePitchID(n)<128);
                
                const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
                if(drumIDInVector == -1)
				{
                    //std::cout << "WARNING: a -1 drum event was found.\n";
                    continue;
                }
                //const float volume=track->getNoteVolume(n)/127.0;
                const int drumy=getYForDrum(drumIDInVector);
                
                glBegin(GL_TRIANGLES);
                glVertex2f(drumx+x_steps_to_move, drumy + y_steps_to_move*y_step);
                glVertex2f(drumx+x_steps_to_move, drumy + (y_steps_to_move+1)*y_step);
                glVertex2f(drumx + 5 + x_steps_to_move, drumy + y_step/2 + y_steps_to_move*y_step);
                glEnd();
                
            }
            
            
        }
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();
        
    }

    
    // -----------------------------------------------------------------
    // left part with drum names
    // -----------------------------------------------------------------
    glLoadIdentity();
    
    // grey background
    glDisable(GL_TEXTURE_2D);
    if(!focus) glColor3f(0.4, 0.4, 0.4);
    else glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_QUADS);
    
    glVertex2f( 0, getEditorYStart());
    glVertex2f( 0, getYEnd());
    glVertex2f( getEditorXStart()-3, getYEnd());
    glVertex2f( getEditorXStart()-3, getEditorYStart());
    
    glEnd();

    
    // drum names
    glColor3f(0,0,0);
    int drumY=-1;
    for(int drumID=0; drumID<drumAmount; drumID++)
	{
        drumY++;
        
		const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();
        if(y<getEditorYStart()-10 or y>getYEnd()) continue;
		
		// only show used drums widget
		if(drumY==0)
		{
			if(showUsedDrumsOnly)
			{
			    glColor3f(0,0,0);
                glBegin(GL_TRIANGLES);
                glVertex2f( getEditorXStart()-73, y+2);
                glVertex2f( getEditorXStart()-73, y+8);
                glVertex2f( getEditorXStart()-63, y+5);
                glEnd();
            }
			else
			{
                glBegin(GL_TRIANGLES);
                glVertex2f( getEditorXStart()-73, y+1);
                glVertex2f( getEditorXStart()-67, y+1);
                glVertex2f( getEditorXStart()-70, y+9);
                glEnd();
           }
			
		}
	
        
        glRasterPos2f(getEditorXStart()-74, y+9);
        
        if(drums[drumID].midiKey == -1) // section header
		{
            glColor3f(0,0,0);
            glBegin(GL_QUADS);
            glVertex2f(getEditorXStart(), y);
            glVertex2f(getEditorXStart(), y+y_step);
            glVertex2f(getXEnd(), y+y_step);
            glVertex2f(getXEnd(), y);
            glEnd();
            glColor3f(1,1,1);
            
            if(!drums[drumID].sectionExpanded) // expand/collapse widget of section header
			{
                glBegin(GL_TRIANGLES);
                glVertex2f( getEditorXStart()+7, y+2);
                glVertex2f( getEditorXStart()+7, y+8);
                glVertex2f( getEditorXStart()+17, y+5);
                glEnd();
            }
			else
			{
                glBegin(GL_TRIANGLES);
                glVertex2f( getEditorXStart()+7, y+1);
                glVertex2f( getEditorXStart()+13, y+1);
                glVertex2f( getEditorXStart()+10, y+9);
                glEnd();
            }
            
            glRasterPos2f(getEditorXStart()+20, y+9);
        }
        
        for(int i=0; drums[drumID].name[i]; i++) // draw name
		{
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, drums[drumID].name[i]);
        }
        glColor3f(0,0,0);
        
        // if section is collapsed, skip all its elements
        if(!drums[drumID].sectionExpanded)
		{
            // FIXME - check for array bound?
            drumID++;
            while(!drums[drumID++].section);
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum
    
    // -----------------------------------------------------------------
    // Scrollbar
    // -----------------------------------------------------------------
    glEnable(GL_TEXTURE_2D);
    
    if(!focus) glColor3f(0.5, 0.5, 0.5);
    else glColor3f(1,1,1);
	
	renderScrollbar();
	
    glColor3f(1,1,1);
    
    glDisable(GL_SCISSOR_TEST);
    
}


int DrumEditor::getDrumAtY(const int given_y)
{
    
    int drumY=-1;
    for(unsigned int drumID=0; drumID<drums.size(); drumID++)
	{
        drumY++;
        
        const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();

		// check if given y is in the area drum drumID
        if( given_y > y and given_y < y+y_step)  return drumID;
        
        // if section is collapsed, skip all its elements
        if(!drums[drumID].sectionExpanded)
		{
            drumID++;
            // FIXME - check for array bounds?
            while(!drums[drumID++].section);
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum

    return -1;
    
}

int DrumEditor::getYForDrum(const int given_drumID)
{
    
    int drumY=-1;
    for(unsigned int drumID=0; drumID<drums.size(); drumID++)
	{
        drumY++;
        
        const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();
        
        if( (int)given_drumID == (int)drumID)
		{
            return y;
        }
        
        // if section is collapsed, skip all its elements
        if(!drums[drumID].sectionExpanded)
		{
            drumID++;
            // FIXME - check for array bounds?
            while(!drums[drumID++].section);
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum
    
    return -1;
    
}


}
