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

#include <wx/intl.h>

#include "Actions/NumberPressed.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

NumberPressed::NumberPressed(const int number) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change note fret") )
{
    m_number = number;
}

// ----------------------------------------------------------------------------------------------------------

NumberPressed::~NumberPressed()
{
}

// ----------------------------------------------------------------------------------------------------------

void NumberPressed::undo()
{
    relocator.setParent(m_track);
    relocator.prepareToRelocate();
    Note* note = relocator.getNextNote();
    note->setFret(m_previous_number);
}

// ----------------------------------------------------------------------------------------------------------

void NumberPressed::perform()
{
    bool played = false;
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    const int noteAmount = notes.size();
    
    for (int n=0; n<noteAmount; n++)
    {
        if (not notes[n].isSelected()) continue;

        m_previous_number = notes[n].getFret();
        notes[n].setFret(m_number);
        relocator.rememberNote( notes[n] );
        if (not played)
        {
            notes[n].play(true);
            played = true;
        }
        return;
    }//next
}


