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

SetNoteVolume::SetNoteVolume(const int volume, const int noteID) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change note(s) volume") )
{
    m_volume = volume;
    m_note_ID = noteID;
}

SetNoteVolume::~SetNoteVolume()
{
}

void SetNoteVolume::undo()
{
    std::cout << "undoing set volume" << std::endl;
    //undo_obj.restoreState(track);
    Note* current_note;
    relocator.setParent(track);
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
    ASSERT(track != NULL);
    
    if (m_note_ID == SELECTED_NOTES)
    {
        
        bool played = false;
        const int noteAmount = track->m_notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (track->m_notes[n].isSelected())
            {
                m_volumes.push_back( track->m_notes[n].volume  );
                track->m_notes[n].setVolume( m_volume );
                relocator.rememberNote(track->m_notes[n]);
                if (!played)
                {
                    track->m_notes[n].play(true);
                    played = true;
                }
            }
        }//next note
        
    }
    else
    {
        // single note
        
        ASSERT_E(m_note_ID,>=,0);
        ASSERT_E(m_note_ID,<,track->m_notes.size());
        
        // if user changed the volume of a note that is not selected, change the volume of this note only
        m_volumes.push_back( track->m_notes[m_note_ID].volume  );
        track->m_notes[m_note_ID].setVolume( m_volume );
        relocator.rememberNote(track->m_notes[m_note_ID]);
        track->m_notes[m_note_ID].play(true);
    }
    
}

