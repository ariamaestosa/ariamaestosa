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

#include "Actions/RearrangeNotes.h"
#include "Actions/ShiftString.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "AriaCore.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

RearrangeNotes::RearrangeNotes() :
    //I18N: (undoable) action name
    SingleTrackAction( _("rearrange notes") )
{
}

RearrangeNotes::~RearrangeNotes()
{
}

void RearrangeNotes::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();

    int i=0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setStringAndFret( string[i], fret[i] );
        i++;
    }//wend
}

void RearrangeNotes::perform()
{
    ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
    // FIXME: fix this abuse of the importing feature
    OwnerPtr<Sequence::Import> import(m_track->getSequence()->startImport()); // just to make sure notes are not played while reordering

    ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
    
    std::vector<int> candidates;

    //ptr_vector<Note>& notes = m_visitor->getNotesVector();
    
    const int noteAmount = m_track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {

        // find notes that have the same tick
        candidates.clear();
        int x1 = m_track->getNoteStartInMidiTicks(n);
        candidates.push_back(n);

        ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
        
        // found 2 consecutive notes with same tick - check if there are more
        if (n < noteAmount-1 and m_track->getNoteStartInMidiTicks(n + 1) == x1)
        {
            n++;
            while (n < noteAmount and m_track->getNoteStartInMidiTicks(n) == x1)
            {
                candidates.push_back(n);
                n++;
            }
            n--;
        }

        ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
        
        if (candidates.size()>1)
        { // we found many notes with the same tick

            const int size = candidates.size();

            // check if any of them are selected
            bool selected = false;
            for (int i=0; i<size; i++)
            {
                if (m_track->isNoteSelected( candidates[i] )) selected = true;
            }
            if (not selected) continue;

            // check if more than one are on the same string (if they overlap)
            bool need_to_reorder = false;

            for (int a=0; a<size; a++)
            {
                for (int b=0; b<size; b++)
                {

                    if (a != b) continue;

                    if (m_track->getNoteString(candidates[a]) == m_track->getNoteString(candidates[b]) /*overlapping notes*/
                        or  /*split powerchord*/
                        (abs(m_track->getNotePitchID(candidates[b]) - m_track->getNotePitchID(candidates[b]))==7
                         and abs(m_track->getNoteString(candidates[a]) - m_track->getNoteString(candidates[b]))>1)
                        )
                    {

                        need_to_reorder = true;

                    }//endif

                }//next
            }//next

            if (need_to_reorder)
            {
                ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );

                // order in pitch order (lowest to highest)
                for (int i=0; i<size-1; i++)
                {

                    if (m_track->getNotePitchID(candidates[i]) < m_track->getNotePitchID(candidates[i+1]))
                    {

                        int tmp = candidates[i+1];

                        candidates[i+1] = candidates[i];
                        candidates[i] = tmp;

                        i=-1;
                    }

                }// next


                // shift first note down if possible (if necessary)
                const int fstr = m_track->getNoteString(candidates[0]) - (size - 1); // calculate where highest note would end up this way

                if (fstr < 0)
                { // if highest note wouldn't fit in available space, shift down all notes as many times as needed
                    for (int m=0; m<abs(fstr); m++)
                    {
                        //FIXME: why do I do this to note 0 fstr times???
                        Note* current = m_track->getNote(candidates[0]);
                        relocator.rememberNote( current );

                        fret.push_back( m_track->getNoteFret(candidates[0]) );
                        string.push_back( m_track->getNoteString(candidates[0]) );

                        Action::ShiftString action( 1, candidates[0] );
                        action.setParentTrack(m_track, m_visitor);
                        action.perform();
                    }
                }

                const int base_string = m_track->getNoteString(candidates[0]);

                ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
                
                // lay them out
                for (int i=1; i<size; i++)
                {
                    Note* current = m_track->getNote(candidates[i]);
                    relocator.rememberNote( current );

                    fret.push_back( m_track->getNoteFret(candidates[i]) );
                    string.push_back( m_track->getNoteString(candidates[i]) );

                    // place lowest note on lowest needed string, then second lowest note on second lowest string, etc
                    Action::ShiftString action( base_string - i - m_track->getNoteString(candidates[i]) , candidates[i]);
                    action.setParentTrack(m_track, new Track::TrackVisitor(*m_visitor));
                    action.perform();
                }
                ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
            }

        }//endif
        
        ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
    }//next

}



