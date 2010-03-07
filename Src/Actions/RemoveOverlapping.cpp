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

#include "Actions/RemoveOverlapping.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Note.h"
#include "wx/wx.h"
#include "AriaCore.h"

using namespace AriaMaestosa::Action;


RemoveOverlapping::RemoveOverlapping() : SingleTrackAction( _("remove overlapping notes") )
{
}

RemoveOverlapping::~RemoveOverlapping()
{
}

void RemoveOverlapping::undo()
{
    const int noteAmount = removedNotes.size();
    for(int n=0; n<noteAmount; n++)
    {
        track->addNote( removedNotes.get(n), false );
    }
    // we will be using the notes again, make sure it doesn't delete them
    removedNotes.clearWithoutDeleting();
}

void RemoveOverlapping::perform()
{
    assert(track != NULL);
    wxBeginBusyCursor();
    
    //const int noteOffAmount = track->noteOff.size();
    const int noteAmount = track->notes.size();
    
    // compare all notes to see if they match
    for(int n1=0; n1<noteAmount; n1++)
    {
        
        for(int n2=0; n2<noteAmount; n2++)
        {
            
            if (n1 == n2) continue; // don't compare a note with itself
            if (track->notes.isMarked(n1)) continue; // skip notes already removed
            if (track->notes.isMarked(n2)) continue; // skip notes already removed
            
            // both notes have the same pitch, they are candidates for overlapping
            if (track->notes[n1].pitchID == track->notes[n2].pitchID)
            {
                const int from = std::min(track->notes[n1].startTick, track->notes[n2].startTick);
                const int to = std::max(track->notes[n1].endTick, track->notes[n2].endTick);
                // total length of both notes
                const int length =  (track->notes[n1].endTick - track->notes[n1].startTick) +
                (track->notes[n2].endTick - track->notes[n2].startTick);
                
                // if difference between beginning of first note and ending of second note is smaller than their lengths, then the notes overlap
                // if they both begin at the same tick they also overlap
                if ( (to - from < length) or (to - from == 0) )
                {
                    removedNotes.push_back(&track->notes[n1]);
                    track->markNoteToBeRemoved(n1);
                }
                
                
            }// end if pitches match
            
        }//next n2
        
    }//next n1
    
    track->removeMarkedNotes();
    wxEndBusyCursor();
    
    Display::render();
    
}


