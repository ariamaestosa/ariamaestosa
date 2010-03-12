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

#include "Actions/NumberPressed.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"

using namespace AriaMaestosa::Action;

NumberPressed::NumberPressed(const int number) : SingleTrackAction( _("change note fret") )
{
    NumberPressed::number = number;
}

NumberPressed::~NumberPressed()
{
}

void NumberPressed::undo()
{
    relocator.setParent(track);
    relocator.prepareToRelocate();
    Note* note = relocator.getNextNote();
    note->setFret(previousNumber);
}

void NumberPressed::perform()
{
    assert(track != NULL);

    if (track->graphics->editorMode != GUITAR) return;

    bool played = false;
    const int amount_n = track->m_notes.size();
    
    for (int n=0; n<amount_n; n++)
    {
        if (!track->m_notes[n].isSelected()) continue;

        previousNumber = track->m_notes[n].getFret();
        track->m_notes[n].setFret(number);
        relocator.rememberNote( track->m_notes[n] );
        if (!played)
        {
            track->m_notes[n].play(true);
            played = true;
        }
        return;
    }//next
}


