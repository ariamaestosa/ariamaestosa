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

#include "Actions/AddTextEvent.h"
#include "Actions/EditAction.h"
#include "Editors/ControllerEditor.h"
#include "Midi/Track.h"
#include "Midi/ControllerEvent.h"
#include "Midi/Sequence.h"
#include "UnitTest.h"

using namespace AriaMaestosa::Action;

// ---------------------------------------------------------------------------------------------------------

AddTextEvent::AddTextEvent(const int x, const wxString value, const int controller) :
//I18N: (undoable) action name
SingleTrackAction( _("text change") )
{
    m_x = x;
    m_value = value;
    m_controller = controller;
}

// ---------------------------------------------------------------------------------------------------------

AddTextEvent::~AddTextEvent()
{
}

// ---------------------------------------------------------------------------------------------------------

void AddTextEvent::undo()
{
    Sequence* sequence = m_track->getSequence();
    for (int n=0; n<sequence->getTextEventAmount(); n++)
    {
        if (sequence->getTextEvent(n)->getTick() == m_x)
        {
            ASSERT_E(sequence->getTextEvent(n)->getController(), ==, m_controller);
            
            // if event was added where there was no event before
            if (m_removed_event_value.Length() == 0)
            {
                sequence->eraseTextEvent(n);
                break;
            }
            else
                // if event replaced an event that existed before
            {
                sequence->setTextEventValue(n, m_removed_event_value);
            }
        }//endif
    }//next
}

// ---------------------------------------------------------------------------------------------------------

void AddTextEvent::perform()
{
    TextEvent* event = new TextEvent(m_controller, m_x, m_value);
    m_removed_event_value = m_track->getSequence()->addTextEvent( event );
}

// ---------------------------------------------------------------------------------------------------------
