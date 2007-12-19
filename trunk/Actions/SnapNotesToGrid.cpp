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

#include "Actions/SnapNotesToGrid.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"

namespace AriaMaestosa
{
	namespace Action
{
	void SnapNotesToGrid::undo()
	{
		Note* current_note;
		relocator.setParent(track);
		relocator.prepareToRelocate();
		int n=0;
		while( (current_note = relocator.getNextNote()) and current_note != NULL)
		{
			current_note->startTick = note_start[n];
			current_note->endTick = note_end[n];
			n++;
		}
		track->reorderNoteVector();
		track->reorderNoteOffVector();
	}
	void SnapNotesToGrid::perform()
	{
		//undo_obj.saveState(track);
		
		assert(track != NULL);
		
		const int n_amount = track->notes.size();
		for(int n=0; n<n_amount; n++)
		{
			if(!track->notes[n].isSelected()) continue;
			
			note_start.push_back( track->notes[n].startTick );
			note_end.push_back( track->notes[n].endTick );
			
			track->notes[n].startTick = track->graphics->getCurrentEditor()->snapMidiTickToGrid( track->notes[n].startTick );
			track->notes[n].endTick = track->graphics->getCurrentEditor()->snapMidiTickToGrid( track->notes[n].endTick );
			relocator.rememberNote(track->notes[n]);
		}
		
		
		track->reorderNoteVector();
		track->reorderNoteOffVector();
}
SnapNotesToGrid::SnapNotesToGrid()
{
}
SnapNotesToGrid::~SnapNotesToGrid() {}
}
}
