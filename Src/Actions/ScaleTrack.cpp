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

using namespace AriaMaestosa::Action;

ScaleTrack::ScaleTrack(float factor, int relative_to, bool selectionOnly) :
    //I18N: (undoable) action name
    SingleTrackAction( _("scale note(s)") )
{
    ScaleTrack::factor = factor;
    ScaleTrack::relative_to = relative_to;
    ScaleTrack::selectionOnly = selectionOnly;
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
    while( (current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->startTick = note_start[n];
        current_note->endTick = note_end[n];
        n++;
    }
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void ScaleTrack::perform()
{
    ASSERT(track != NULL);
    
    const int noteAmount=track->m_notes.size();
    
    for(int n=0; n<noteAmount; n++)
    {
        
        if (selectionOnly and !track->m_notes[n].isSelected()) continue; // skip unselected notes if we only want to affect selection
        
        note_start.push_back( track->m_notes[n].startTick );
        note_end.push_back( track->m_notes[n].endTick );
        
        track->m_notes[n].startTick = (int)(
                                          (track->m_notes[n].startTick-relative_to)*factor + relative_to
                                          );
        track->m_notes[n].endTick = (int)(
                                        (track->m_notes[n].endTick-relative_to)*factor + relative_to
                                        );
        relocator.rememberNote(track->m_notes[n]);
        
    }//next
    
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

