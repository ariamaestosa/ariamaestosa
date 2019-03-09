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
#include "AriaCore.h"

#include <wx/intl.h>
#include <wx/utils.h>

#include <algorithm>

using namespace AriaMaestosa::Action;


RemoveOverlapping::RemoveOverlapping() :
    //I18N: (undoable) action name
    SingleTrackAction( _("remove overlapping notes") )
{
}

RemoveOverlapping::~RemoveOverlapping()
{
}

void RemoveOverlapping::undo()
{
    const int noteAmount = removedNotes.size();
    for (int n=0; n<noteAmount; n++)
    {
        m_track->addNote( removedNotes.get(n), false );
    }
    // we will be using the notes again, make sure it doesn't delete them
    removedNotes.clearWithoutDeleting();
}

void RemoveOverlapping::perform()
{
    ASSERT(m_track != NULL);
    wxBeginBusyCursor();
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    const int noteAmount = notes.size();
    
    // compare all notes to see if they match
    for (int n1=0; n1<noteAmount; n1++)
    {
        
        for (int n2=0; n2<noteAmount; n2++)
        {
            
            if (n1 == n2) continue; // don't compare a note with itself
            if (notes.isMarked(n1)) continue; // skip notes already removed
            if (notes.isMarked(n2)) continue; // skip notes already removed
            
            // both notes have the same pitch, they are candidates for overlapping
            if (notes[n1].getPitchID() == notes[n2].getPitchID())
            {
                const int from = std::min(notes[n1].getTick(), notes[n2].getTick());
                const int to = std::max(notes[n1].getEndTick(), notes[n2].getEndTick());
                // total length of both notes
                const int length =  (notes[n1].getEndTick() - notes[n1].getTick()) +
                (notes[n2].getEndTick() - notes[n2].getTick());
                
                // if difference between beginning of first note and ending of second note is smaller than their lengths, then the notes overlap
                // if they both begin at the same tick they also overlap
                if ( (to - from < length) or (to - from == 0) )
                {
                    removedNotes.push_back(&notes[n1]);
                    m_track->markNoteToBeRemoved(n1);
                }
                
                
            }// end if pitches match
            
        }//next n2
        
    }//next n1
    
    m_track->removeMarkedNotes();
    wxEndBusyCursor();
    
    Display::render();
    
}


