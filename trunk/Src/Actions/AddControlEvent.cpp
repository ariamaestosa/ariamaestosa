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

#include "Actions/AddControlEvent.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/ControllerEvent.h"
#include "Midi/Sequence.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/ControllerEditor.h"

using namespace AriaMaestosa::Action;

AddControlEvent::AddControlEvent(const int x, const int value, const int controller) :
    SingleTrackAction( _("add control event") )
{
    AddControlEvent::x = x;
    AddControlEvent::value = value;
    AddControlEvent::controller = controller;
    removedEventValue = -1;
}

AddControlEvent::~AddControlEvent()
{
}

void AddControlEvent::undo()
{
    
    const int type = track->graphics->controllerEditor->getCurrentControllerType();
    
    if (type != 201 /*tempo*/)
    {
        const int controlEventsAmount = track->m_control_events.size();
        for(int n=0; n<controlEventsAmount; n++)
        {
            
            if (track->m_control_events[n].getTick() == x and
                track->m_control_events[n].getController() == controller)
            {
                // if event was added where there was no event before
                if (removedEventValue == -1)
                {
                    track->m_control_events.erase(n);
                    break;
                }
                else
                    // if event replaced an event that existed before
                {
                    track->m_control_events[n].setValue( removedEventValue );
                }
            }//endif
        }//next
    }
    else //tempo
    {
        const int tempoEventsAmount = track->sequence->tempoEvents.size();
        for(int n=0; n<tempoEventsAmount; n++)
        {
            
            if (track->sequence->tempoEvents[n].getTick() == x and
                track->sequence->tempoEvents[n].getController() == controller)
            {
                // if event was added where there was no event before
                if (removedEventValue == -1)
                {
                    track->sequence->tempoEvents.erase(n);
                    break;
                }
                else
                    // if event replaced an event that existed before
                {
                    track->sequence->tempoEvents[n].setValue( removedEventValue );
                }
            }//endif
        }//next
    }
    
}

void AddControlEvent::perform()
{
    ControllerEvent* event = new ControllerEvent(track->sequence, controller, x, value);
    track->addControlEvent( event, &removedEventValue );
}


