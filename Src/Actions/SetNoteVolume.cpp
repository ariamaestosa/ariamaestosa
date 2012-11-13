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

#include "Actions/SetNoteVolume.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

SetNoteVolume::SetNoteVolume(const int value, const int noteID, const bool increment) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change note(s) volume") )
{
    m_value = value;
    m_note_ID = noteID;
    m_increment = increment;
}

SetNoteVolume::~SetNoteVolume()
{
}

void SetNoteVolume::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    int n = 0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setVolume( m_volumes[n] );
        n++;
    }
}

void SetNoteVolume::perform()
{
    int volume;
    ASSERT(m_track != NULL);
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();

    if (m_note_ID == SELECTED_NOTES)
    {
        bool played = false;
        const int noteAmount = notes.size();
        
        for (int n=0; n<noteAmount; n++)
        {
            if (notes[n].isSelected())
            {
                volume = notes[n].getVolume();
                m_volumes.push_back(volume);
                adjustVolume(volume);
                notes[n].setVolume(volume);
                relocator.rememberNote(notes[n]);
                if (not played)
                {
                    notes[n].play(true);
                    played = true;
                }
            }
        }//next note
        
    }
    else
    {
        // single note
        
        ASSERT_E(m_note_ID,>=,0);
        ASSERT_E(m_note_ID,<,notes.size());
        
        // if user changed the volume of a note that is not selected, change the volume of this note only
        volume = notes[m_note_ID].getVolume();
        m_volumes.push_back(volume);
        adjustVolume(volume);
        notes[m_note_ID].setVolume(volume);
        relocator.rememberNote(notes[m_note_ID]);
        notes[m_note_ID].play(true);
    }
    
}

void SetNoteVolume::adjustVolume(int& volume)
{
    if (m_increment)
    {
        volume += m_value;
        if (volume<0) volume = 0;
        if (volume>SCHAR_MAX) volume = SCHAR_MAX;
    }
    else
    {
        volume = m_value;
    }
}

