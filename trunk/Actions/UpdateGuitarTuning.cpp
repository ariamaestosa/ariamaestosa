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

namespace AriaMaestosa
{
    namespace Action
{
    void UpdateGuitarTuning::undo()
    {
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();
        int n=0;
        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            current_note->setStringAndFret( strings[n], frets[n] );
            n++;
        }
    }
    void UpdateGuitarTuning::perform()
    {
        //undo_obj.saveState(track);
        assert(track != NULL);


        const int amount_n = track->notes.size();
        for(int n=0; n<amount_n; n++)
        {
            frets.push_back( track->notes[n].getFret() );
            strings.push_back( track->notes[n].getString() );

            track->notes[n].checkIfStringAndFretMatchNote(true);

            relocator.rememberNote( track->notes[n] );
        }//next

    }

UpdateGuitarTuning::UpdateGuitarTuning() {}
UpdateGuitarTuning::~UpdateGuitarTuning() {}
}
}
