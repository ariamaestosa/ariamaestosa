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

#include "Actions/ScaleTrack.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

namespace AriaMaestosa
{
	namespace Action
{
	void ScaleTrack::undo()
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
	void ScaleTrack::perform()
	{
		assert(track != NULL);

		const int noteAmount=track->notes.size();
		
		for(int n=0; n<noteAmount; n++)
		{
			
			if(selectionOnly and !track->notes[n].isSelected()) continue; // skip unselected notes if we only want to affect selection
			
			note_start.push_back( track->notes[n].startTick );
			note_end.push_back( track->notes[n].endTick );
			
			track->notes[n].startTick = (int)(
									   (track->notes[n].startTick-relative_to)*factor + relative_to
									   );
			track->notes[n].endTick = (int)(
									 (track->notes[n].endTick-relative_to)*factor + relative_to
									 );
			relocator.rememberNote(track->notes[n]);
			
		}//next
		
		track->reorderNoteVector();
		track->reorderNoteOffVector();
}
ScaleTrack::ScaleTrack(float factor, int relative_to, bool selectionOnly)
{
	ScaleTrack::factor = factor;
	ScaleTrack::relative_to = relative_to;
	ScaleTrack::selectionOnly = selectionOnly;
}
ScaleTrack::~ScaleTrack() {}
}
}
