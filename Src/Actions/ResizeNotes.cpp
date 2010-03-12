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

#include "Actions/ResizeNotes.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"


using namespace AriaMaestosa::Action;

ResizeNotes::ResizeNotes(const int relativeWidth, const int noteID) : SingleTrackAction( _("resize note(s)") )
{
    ResizeNotes::relativeWidth = relativeWidth;
    ResizeNotes::noteID = noteID;
}

ResizeNotes::~ResizeNotes()
{
}

void ResizeNotes::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    while( (current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->resize( -relativeWidth );
    }
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void ResizeNotes::perform()
{
    assert(track != NULL);
    
    if (track->graphics->editorMode == DRUM) return;
    
    assert(noteID != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    
    if (noteID==SELECTED_NOTES)
    {
        
        bool played=false;
        for(int n=0; n<track->m_notes.size(); n++)
        {
            if (!track->m_notes[n].isSelected()) continue;
            
            track->m_notes[n].resize(relativeWidth);
            relocator.rememberNote(track->m_notes[n]);
            
            if (!played)
            {
                track->m_notes[n].play(false);
                played = true;
            }
        }//next
        
    }
    else
    {
        
        assertExpr(noteID,<,track->m_notes.size());
        assertExpr(noteID,>=,0);
        
        track->m_notes[noteID].resize(relativeWidth);
        track->m_notes[noteID].play(false);
        relocator.rememberNote(track->m_notes[noteID]);
    }
    
    track->reorderNoteOffVector();
}




