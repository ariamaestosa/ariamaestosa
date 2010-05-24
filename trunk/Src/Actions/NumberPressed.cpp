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

#include "wx/intl.h"

#include "Actions/NumberPressed.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"

using namespace AriaMaestosa::Action;

NumberPressed::NumberPressed(const int number) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change note fret") )
{
    m_number = number;
}

NumberPressed::~NumberPressed()
{
}

void NumberPressed::undo()
{
    relocator.setParent(track);
    relocator.prepareToRelocate();
    Note* note = relocator.getNextNote();
    note->setFret(m_previous_number);
}

void NumberPressed::perform()
{
    ASSERT(track != NULL);

    if (track->graphics->editorMode != GUITAR) return;

    bool played = false;
    const int noteAmount = track->m_notes.size();
    
    for (int n=0; n<noteAmount; n++)
    {
        if (not track->m_notes[n].isSelected()) continue;

        m_previous_number = track->m_notes[n].getFret();
        track->m_notes[n].setFret(m_number);
        relocator.rememberNote( track->m_notes[n] );
        if (not played)
        {
            track->m_notes[n].play(true);
            played = true;
        }
        return;
    }//next
}


