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

#include "Actions/ShiftString.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

ShiftString::ShiftString(const int amount, const int noteid) :
    //I18N: (undoable) action name
    SingleTrackAction( _("string change") )
{
    m_amount = amount;
    m_note_id = noteid;
}

// ----------------------------------------------------------------------------------------------------------

ShiftString::~ShiftString()
{
}

// ----------------------------------------------------------------------------------------------------------

void ShiftString::undo()
{
    Note* current_note;
    m_relocator.setParent(track);
    m_relocator.prepareToRelocate();
    
    int n = 0;
    while ((current_note = m_relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setStringAndFret( m_strings[n], m_frets[n] );
        n++;
    }
}

// ----------------------------------------------------------------------------------------------------------

void ShiftString::perform()
{
    ASSERT(track != NULL);
    
    // only accept to do this in guitar mode
    if (track->getNotationType() != GUITAR) return;
    
    // not supported in this function (mostly because not needed, but could logically be implmented)
    ASSERT(m_note_id != ALL_NOTES);
    
    if (m_note_id == SELECTED_NOTES)
    {
        
        bool played=false;
        const int amount_n = track->m_notes.size();
        
        for (int n=0; n<amount_n; n++)
        {
            if (not track->m_notes[n].isSelected()) continue;
            
            m_frets.push_back( track->m_notes[n].getFret() );
            m_strings.push_back( track->m_notes[n].getString() );
            
            track->m_notes[n].shiftString(m_amount);
            
            m_relocator.rememberNote( track->m_notes[n] );
            
            if (not played)
            {
                track->m_notes[n].play(false);
                played = true;
            }
        }//next
        
    }
    else
    {
        ASSERT_E(m_note_id,>=,0);
        ASSERT_E(m_note_id,<,track->m_notes.size());
        
        m_frets.push_back( track->m_notes[m_note_id].getFret() );
        m_strings.push_back( track->m_notes[m_note_id].getString() );
        track->m_notes[m_note_id].shiftString(m_amount);
        m_relocator.rememberNote( track->m_notes[m_note_id] );
        track->m_notes[m_note_id].play(false);
    }
    
}

// ----------------------------------------------------------------------------------------------------------
