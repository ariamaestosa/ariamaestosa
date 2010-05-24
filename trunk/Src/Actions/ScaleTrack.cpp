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
#include "Midi/Track.h"

#include "wx/intl.h"

using namespace AriaMaestosa::Action;

ScaleTrack::ScaleTrack(float factor, int relative_to, bool selectionOnly) :
    //I18N: (undoable) action name
    SingleTrackAction( _("scale note(s)") )
{
    m_factor = factor;
    m_relative_to = relative_to;
    m_selection_only = selectionOnly;
}

ScaleTrack::~ScaleTrack()
{
}

void ScaleTrack::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    int n=0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->startTick = m_note_start[n];
        current_note->endTick = m_note_end[n];
        n++;
    }
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void ScaleTrack::perform()
{
    ASSERT(track != NULL);
    
    const int noteAmount = track->m_notes.size();
    
    for (int n=0; n<noteAmount; n++)
    {
        // skip unselected notes if we only want to affect selection
        if (m_selection_only and not track->m_notes[n].isSelected()) continue; 
        
        m_note_start.push_back( track->m_notes[n].startTick );
        m_note_end.push_back( track->m_notes[n].endTick );
        
        track->m_notes[n].startTick = (int)(
                                          (track->m_notes[n].startTick-m_relative_to)*m_factor + m_relative_to
                                          );
        track->m_notes[n].endTick = (int)(
                                        (track->m_notes[n].endTick-m_relative_to)*m_factor + m_relative_to
                                        );
        relocator.rememberNote(track->m_notes[n]);
        
    }//next
    
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

