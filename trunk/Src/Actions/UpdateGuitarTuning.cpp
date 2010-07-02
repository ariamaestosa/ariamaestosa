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

#include "Actions/UpdateGuitarTuning.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Editors/GuitarEditor.h"
#include "GUI/GraphicalTrack.h"

using namespace AriaMaestosa::Action;

UpdateGuitarTuning::UpdateGuitarTuning() :
    //I18N: (undoable) action name
    SingleTrackAction( _("change guitar tuning") )
{
}

UpdateGuitarTuning::~UpdateGuitarTuning()
{
}

void UpdateGuitarTuning::undo()
{
    GuitarTuning* tuning = track->getGuitarTuning();
    tuning->tuning = previous_tuning;
    
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();
    int n=0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setStringAndFret( strings[n], frets[n] );
        n++;
    }
    
    const int amount_n = track->m_notes.size();
    for (int n=0; n<amount_n; n++)
    {
        track->m_notes[n].checkIfStringAndFretMatchNote(true);
    }//next
}

void UpdateGuitarTuning::perform()
{
    ASSERT(track != NULL);
    
    previous_tuning = track->getGuitarTuning()->previous_tuning;
    
    const int amount_n = track->m_notes.size();
    for(int n=0; n<amount_n; n++)
    {
        frets.push_back( track->m_notes[n].getFret() );
        strings.push_back( track->m_notes[n].getString() );
        
        track->m_notes[n].checkIfStringAndFretMatchNote(true);
        
        relocator.rememberNote( track->m_notes[n] );
    }//next
    
}


