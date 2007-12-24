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

#include "Actions/MoveNotes.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/Editor.h"

namespace AriaMaestosa
{
namespace Action
{

void MoveNotes::undo()
{
		Note* current_note;
		relocator.setParent(track);
		relocator.prepareToRelocate();
		while( (current_note = relocator.getNextNote()) and current_note != NULL)
		{
			//current_note->move( -relativeX, -relativeY, mode );
            track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, -relativeY);
		}
		track->reorderNoteVector();
		track->reorderNoteOffVector();
}
void MoveNotes::perform()
{
	assert(track != NULL);
	// for undo
	mode = track->graphics->editorMode;
	
	// perform action
	//track->moveNotes(relativeX, relativeY, noteID, &relocator);
	assert(noteID != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
	
    //saveUndoMemory();
	
    if(noteID==SELECTED_NOTES)
	{
		
		bool played = false;
		
		const int noteAmount=track->notes.size();
        for(int n=0; n<noteAmount; n++)
		{
            if(!track->notes[n].isSelected()) continue;
			
            //track->notes[n].move(relativeX, relativeY, track->graphics->editorMode);
            track->graphics->getCurrentEditor()->moveNote(track->notes[n], relativeX, relativeY);
			relocator.rememberNote( track->notes[n] );
			
			if(!played)
			{
				if(relativeY != 0) track->notes[n].play(true);
				else track->notes[n].play(false);
				played = true;
			}
        }//next
    }
	else
	{
		// move a single note
        assertExpr(noteID,>=,0);
        assertExpr(noteID,<,track->notes.size());
		
        //track->notes[noteID].move(relativeX, relativeY, track->graphics->editorMode);
        track->graphics->getCurrentEditor()->moveNote(track->notes[noteID], relativeX, relativeY);
		relocator.rememberNote( track->notes[noteID] );
		
		if(relativeX != 0)
		{
			if(relativeY != 0) track->notes[noteID].play(true);
			else track->notes[noteID].play(false);
		}
    }
	
    track->reorderNoteVector();
	track->reorderNoteOffVector();
}
MoveNotes::MoveNotes(const int relativeX, const int relativeY, const int noteID)
{
	MoveNotes::relativeX = relativeX;
	MoveNotes::relativeY = relativeY;
	MoveNotes::noteID = noteID;
}
MoveNotes::~MoveNotes() {}

}
}
