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
#include "Midi/MeasureData.h"
#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

ScaleTrack::ScaleTrack(float factor, int relative_to, bool selectionOnly) :
    //I18N: (undoable) action name
    SingleTrackAction( _("scale note(s)") )
{
    m_factor = factor;
    m_relative_to = relative_to;
    m_selection_only = selectionOnly;
}

// ----------------------------------------------------------------------------------------------------------

ScaleTrack::~ScaleTrack()
{
}

// ----------------------------------------------------------------------------------------------------------

void ScaleTrack::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    int n=0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setTick( m_note_start[n] );
        current_note->setEndTick( m_note_end[n] );
        n++;
    }
    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}

// ----------------------------------------------------------------------------------------------------------

void ScaleTrack::perform()
{
    ASSERT(m_track != NULL);
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    const int noteAmount = notes.size();
    
    int last_tick = -1;
    
    for (int n=0; n<noteAmount; n++)
    {
        // skip unselected notes if we only want to affect selection
        if (m_selection_only and not notes[n].isSelected()) continue; 
        
        const int startTick = notes[n].getTick();
        const int endTick   = notes[n].getEndTick();
        
        m_note_start.push_back( startTick );
        m_note_end.push_back( endTick );
        
        notes[n].setTick   ( (int)( (startTick - m_relative_to)*m_factor + m_relative_to ) );
        notes[n].setEndTick( (int)( (endTick   - m_relative_to)*m_factor + m_relative_to ) );
        relocator.rememberNote(notes[n]);
        
        if (notes[n].getEndTick() > last_tick) last_tick = notes[n].getEndTick();
        
    }//next
    
    MeasureData* md = m_track->getSequence()->getMeasureData();
    if (last_tick > md->getTotalTickAmount())
    {        
        md->extendToTick(last_tick);
    }
    
    
    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}

// ----------------------------------------------------------------------------------------------------------

