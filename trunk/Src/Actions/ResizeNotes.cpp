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
#include "GUI/GraphicalTrack.h"
#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

ResizeNotes::ResizeNotes(const int relativeWidth, const int noteID) :
    //I18N: (undoable) action name
    SingleTrackAction( _("resize note(s)") )
{
    m_relative_width = relativeWidth;
    m_note_ID = noteID;
}

ResizeNotes::~ResizeNotes()
{
}

void ResizeNotes::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->resize( -m_relative_width );
    }
    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void ResizeNotes::perform()
{
    ASSERT(track != NULL);
    
    if (track->graphics->getEditorMode() == DRUM) return;
    
    // not supported in this function (mostly bacause not needed, but could logically be implmented)
    ASSERT(m_note_ID != ALL_NOTES); 
    
    if (m_note_ID == SELECTED_NOTES)
    {
        
        bool played=false;
        for(int n=0; n<track->m_notes.size(); n++)
        {
            if (!track->m_notes[n].isSelected()) continue;
            
            track->m_notes[n].resize(m_relative_width);
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
        
        ASSERT_E(m_note_ID,<,track->m_notes.size());
        ASSERT_E(m_note_ID,>=,0);
        
        track->m_notes[m_note_ID].resize(m_relative_width);
        track->m_notes[m_note_ID].play(false);
        relocator.rememberNote(track->m_notes[m_note_ID]);
    }
    
    track->reorderNoteOffVector();
}




