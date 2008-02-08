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

#include "Actions/ShiftBySemiTone.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"

namespace AriaMaestosa
{
	namespace Action
{
	void ShiftBySemiTone::undo()
	{
		Note* current_note;
		relocator.setParent(track);
		relocator.prepareToRelocate();
		int n=0;
		while( (current_note = relocator.getNextNote()) and current_note != NULL)
		{
            current_note->pitchID -= deltaY;
			n++;
		}
	}
	void ShiftBySemiTone::perform()
	{
		assert(track != NULL);
		
		if(track->graphics->editorMode != SCORE) return;
		
		assert(noteid != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implemented)
		
		// concerns all selected notes
		if(noteid==SELECTED_NOTES)
		{
			
			bool played = false;
			const int amount_n = track->notes.size();
			for(int n=0; n<amount_n; n++)
			{
				if(!track->notes[n].isSelected()) continue;
				
                track->notes[n].pitchID += deltaY;
				relocator.rememberNote( track->notes[n] );
				
				if(!played)
				{
					track->notes[n].play(true);
					played = true;
				}
			}//next
			
		}
		// only concernes one specific note
		else
		{
			// warning : not yet used so not tested
			assertExpr(noteid,>=,0);
			assertExpr(noteid,<,track->notes.size());
			
			track->notes[noteid].pitchID += deltaY;
			relocator.rememberNote( track->notes[noteid] );
			
			track->notes[noteid].play(true);
		}
		
}
ShiftBySemiTone::ShiftBySemiTone(const int deltaY, const int noteid)
{
	ShiftBySemiTone::deltaY = deltaY;
	ShiftBySemiTone::noteid = noteid;
}
ShiftBySemiTone::~ShiftBySemiTone() {}

}
}
