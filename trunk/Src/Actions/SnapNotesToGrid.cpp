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

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

SnapNotesToGrid::SnapNotesToGrid() :
    //I18N: (undoable) action name
    SingleTrackAction( _("snap notes to grid") )
{
}

SnapNotesToGrid::~SnapNotesToGrid()
{
}

void SnapNotesToGrid::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    int n = 0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setTick( note_start[n] );
        current_note->setEndTick( note_end[n] );
        n++;
    }
    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}

void SnapNotesToGrid::perform()
{
    //undo_obj.saveState(track);
    
    ASSERT(m_track != NULL);
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    
    const int n_amount = notes.size();
    for (int n=0; n<n_amount; n++)
    {
        Note* note = notes.get(n);
        if (not note->isSelected()) continue;
        
        note_start.push_back( note->getTick() );
        note_end.push_back( note->getEndTick() );
        
        int len = note->getEndTick() - note->getTick();
        note->setTick( m_track->snapMidiTickToGrid( note->getTick(), true ) );
        
        int end_tick = note->getTick() + m_track->snapMidiTickToGrid( len, false );
        if ( note->getTick() == end_tick )
        {
            // note was collapsed, not good.
            // use the 'ceil' variant of snapTickToGrid instead
            end_tick = m_track->snapMidiTickToGrid_ceil( note->getEndTick() );
        }
        
        note->setEndTick( end_tick );
        relocator.rememberNote(notes[n]);
    }
    
    
    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}



