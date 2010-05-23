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


#include "Actions/AddNote.h"
#include "Actions/EditAction.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/GuitarEditor.h"
#include "Midi/Track.h"
#include "Midi/Note.h"

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

AddNote::AddNote(const int pitchID, const int startTick, const int endTick,
                 const int volume, const int string) :
    //I18N: (undoable) action name
    SingleTrackAction( _("add note") )
{
    m_pitch_ID   = pitchID;
    m_start_tick = startTick;
    m_end_tick   = endTick;
    m_volume     = volume;
    m_string     = string;
}

// ----------------------------------------------------------------------------------------------------------

void AddNote::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        const int noteAmount = track->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            if (track->getNote(n) == current_note)
            {
                track->removeNote(n);
                break;
            }//endif
        }//next
    }//wend
}

// ----------------------------------------------------------------------------------------------------------

void AddNote::perform()
{
    ASSERT(track != NULL);
    
    
    Note* tmp_note;
    if (m_string == -1) tmp_note = new Note(track->graphics, m_pitch_ID, m_start_tick, m_end_tick, m_volume);
    else                tmp_note = new Note(track->graphics, m_pitch_ID, m_start_tick, m_end_tick, m_volume,
                                            m_string, 0);
    
    const bool success = track->addNote( tmp_note );
    
    if (success)
    {
        // select last added note
        track->selectNote(ALL_NOTES, false, true); // select last added note
        tmp_note->setSelected(true);
    }
    
    relocator.rememberNote( *tmp_note );
    
    tmp_note->play(true);
}

// ----------------------------------------------------------------------------------------------------------