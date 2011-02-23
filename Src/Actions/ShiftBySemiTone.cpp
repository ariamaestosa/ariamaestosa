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

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

ShiftBySemiTone::ShiftBySemiTone(const int deltaY, const int noteid) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change pitch") )
{
    m_delta_y = deltaY;
    m_note_id = noteid;
}

// ----------------------------------------------------------------------------------------------------------

ShiftBySemiTone::~ShiftBySemiTone()
{
}

// ----------------------------------------------------------------------------------------------------------

void ShiftBySemiTone::undo()
{
    Note* current_note;
    m_relocator.setParent(m_track);
    m_relocator.prepareToRelocate();
    
    int n = 0;
    while ((current_note = m_relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setPitchID( current_note->getPitchID() - m_delta_y );
        n++;
    }
}

// ----------------------------------------------------------------------------------------------------------

void ShiftBySemiTone::perform()
{
    ASSERT(m_track != NULL);
    
    if (not m_track->isNotationTypeEnabled(SCORE)) return;
    
    // not supported in this function (mostly bacause not needed, but could logically be implemented)
    ASSERT(m_note_id != ALL_NOTES);
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    
    // concerns all selected notes
    if (m_note_id == SELECTED_NOTES)
    {
        bool played = false;
        const int amount_n = notes.size();
        for (int n=0; n<amount_n; n++)
        {
            Note* note = notes.get(n);
            
            if (not note->isSelected()) continue;
            
            note->setPitchID( note->getPitchID() + m_delta_y );
            m_relocator.rememberNote( notes[n] );
            
            if (not played)
            {
                note->play(true);
                played = true;
            }
        }//next
        
    }
    // only concernes one specific note
    else
    {
        // warning : not yet used so not tested
        ASSERT_E(m_note_id,>=,0);
        ASSERT_E(m_note_id,<,notes.size());
        
        Note* note = notes.get(m_note_id);

        note->setPitchID( note->getPitchID() + m_delta_y );
        m_relocator.rememberNote( notes[m_note_id] );
        
        note->play(true);
    }
    
}

// ----------------------------------------------------------------------------------------------------------
