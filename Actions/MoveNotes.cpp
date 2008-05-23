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
        
        int n = 0;
		while( (current_note = relocator.getNextNote()) and current_note != NULL)
		{
            if( move_mode == SCORE_VERTICAL )
            {
                current_note->pitchID = score_pitch[n];
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, 0);
                n++;
            }
            else if( move_mode == GUITAR_VERTICAL )
            {
                current_note->fret = fret[n];
                current_note->string = string[n];
                current_note->findNoteFromStringAndFret();
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, 0);
                n++;
            }
            else
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, -relativeY);
		}
		track->reorderNoteVector();
		track->reorderNoteOffVector();
}
void MoveNotes::perform()
{
	assert(track != NULL);
    
	mode = track->graphics->editorMode;
    if(mode == SCORE and relativeY != 0) move_mode = SCORE_VERTICAL;
    else if(mode == GUITAR and relativeY != 0) move_mode = GUITAR_VERTICAL;
    
	// perform action
	assert(noteID != ALL_NOTES); // not supported in this function (not needed)
	
    if(noteID==SELECTED_NOTES)
	{
		
		bool played = false;
		
		const int noteAmount=track->notes.size();
        for(int n=0; n<noteAmount; n++)
		{
            if(!track->notes[n].isSelected()) continue;
			
            doMoveOneNote(n);
			
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
		
        doMoveOneNote(noteID);
		
		if(relativeX != 0)
		{
			if(relativeY != 0) track->notes[noteID].play(true);
			else track->notes[noteID].play(false);
		}
    }
	
    track->reorderNoteVector();
	track->reorderNoteOffVector();
}

void MoveNotes::doMoveOneNote(const int noteid)
{
    if( move_mode == SCORE_VERTICAL ) score_pitch.push_back( track->notes[noteid].pitchID );
    else if( move_mode == GUITAR_VERTICAL )
    {
        fret.push_back( track->notes[noteid].fret );
        string.push_back( track->notes[noteid].string );
    }
    
    track->graphics->getCurrentEditor()->moveNote(track->notes[noteid], relativeX, relativeY);
    relocator.rememberNote( track->notes[noteid] );
}

MoveNotes::MoveNotes(const int relativeX, const int relativeY, const int noteID)
{
	MoveNotes::relativeX = relativeX;
	MoveNotes::relativeY = relativeY;
	MoveNotes::noteID = noteID;
    
    move_mode = DELTA;
}
MoveNotes::~MoveNotes() {}

}
}
