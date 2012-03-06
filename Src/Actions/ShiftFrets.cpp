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
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    int n = 0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setStringAndFret( m_strings[n], m_frets[n] );
        n++;
    }
}

void ShiftFrets::perform()
{
    ASSERT(m_track != NULL);
    ASSERT(m_note_id != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    
    // only accept to do this in guitar mode
    if (not m_track->isNotationTypeEnabled(GUITAR))  return;
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    
    // concerns all selected notes
    if (m_note_id == SELECTED_NOTES)
    {
        
        bool played = false;
        const int noteAmount = notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (not notes[n].isSelected()) continue;
            
            m_strings.push_back( notes[n].getString() );
            m_frets.push_back( notes[n].getFret() );
            notes[n].shiftFret(m_amount);
            relocator.rememberNote( notes[n] );
            
            if (not played)
            {
                notes[n].play(true);
                played = true;
            }
        }//next
        
    }
    // only one specific note
    else
    {
        // warning : not yet used so not tested
        ASSERT_E(m_note_id,>=,0);
        ASSERT_E(m_note_id,<,notes.size());
        
        m_frets.push_back( notes[m_note_id].getFret() );
        m_strings.push_back( notes[m_note_id].getString() );
        notes[m_note_id].shiftFret(m_amount);
        relocator.rememberNote( notes[m_note_id] );
        
        notes[m_note_id].play(true);
    }
    
}


