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

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

ResizeNotes::ResizeNotes(const int relativeWidth, const int noteID) :
    //I18N: (undoable) action name
    SingleTrackAction( _("resize note(s)") )
{
    m_relative_width = relativeWidth;
    m_note_ID = noteID;
}

// ----------------------------------------------------------------------------------------------------------

ResizeNotes::~ResizeNotes()
{
}

// ----------------------------------------------------------------------------------------------------------

void ResizeNotes::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->resize( -m_relative_width );
    }
    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}

// ----------------------------------------------------------------------------------------------------------

void ResizeNotes::perform()
{
    ASSERT(m_track != NULL);
        
    // not supported in this function (mostly because not needed)
    ASSERT(m_note_ID != ALL_NOTES); 
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();

    if (m_note_ID == SELECTED_NOTES)
    {        
        bool played = false;
        for (int n=0; n<notes.size(); n++)
        {
            if (not notes[n].isSelected()) continue;
            
            notes[n].resize(m_relative_width);
            relocator.rememberNote(notes[n]);
            
            if (not played)
            {
                notes[n].play(false);
                played = true;
            }
        }//next
        
    }
    else
    {
        
        ASSERT_E(m_note_ID,<,notes.size());
        ASSERT_E(m_note_ID,>=,0);
        
        notes[m_note_ID].resize(m_relative_width);
        notes[m_note_ID].play(false);
        relocator.rememberNote(notes[m_note_ID]);
    }
    
    m_track->reorderNoteOffVector();
}

// ----------------------------------------------------------------------------------------------------------
