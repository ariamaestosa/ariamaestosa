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

#include "Actions/ShiftFrets.h"
#include "Actions/EditAction.h"

// FIXME(DESIGN) : don't rely on GUI classes
#include "GUI/GraphicalTrack.h"

#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

ShiftFrets::ShiftFrets(const int amount, const int noteID) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change note fret") )
{
    m_amount = amount;
    m_note_id = noteID;
}

void ShiftFrets::undo()
{
    //undo_obj.restoreState(track);
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    int n = 0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setFret( m_frets[n] );
        n++;
    }
}

void ShiftFrets::perform()
{
    ASSERT(track != NULL);
    ASSERT(m_note_id != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    
    // only accept to do this in guitar mode
    if (track->graphics->getEditorMode() != GUITAR)  return;
    
    // concerns all selected notes
    if (m_note_id == SELECTED_NOTES)
    {
        
        bool played = false;
        const int noteAmount = track->m_notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (not track->m_notes[n].isSelected()) continue;
            
            m_frets.push_back( track->m_notes[n].getFret() );
            track->m_notes[n].shiftFret(m_amount);
            relocator.rememberNote( track->m_notes[n] );
            
            if (not played)
            {
                track->m_notes[n].play(true);
                played = true;
            }
        }//next
        
    }
    // only one specific note
    else
    {
        // warning : not yet used so not tested
        ASSERT_E(m_note_id,>=,0);
        ASSERT_E(m_note_id,<,track->m_notes.size());
        
        m_frets.push_back( track->m_notes[m_note_id].getFret() );
        track->m_notes[m_note_id].shiftFret(m_amount);
        relocator.rememberNote( track->m_notes[m_note_id] );
        
        track->m_notes[m_note_id].play(true);
    }
    
}


