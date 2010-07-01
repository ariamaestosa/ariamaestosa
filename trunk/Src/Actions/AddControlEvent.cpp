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
#include "unit_test.h"

using namespace AriaMaestosa::Action;

// ---------------------------------------------------------------------------------------------------------

AddControlEvent::AddControlEvent(const int x, const int value, const int controller) :
    //I18N: (undoable) action name
    SingleTrackAction( _("add control event") )
{
    m_x = x;
    m_value = value;
    m_controller = controller;
    m_removed_event_value = -1;
}

// ---------------------------------------------------------------------------------------------------------

AddControlEvent::~AddControlEvent()
{
}

// ---------------------------------------------------------------------------------------------------------

void AddControlEvent::undo()
{
    
    const int type = track->graphics->controllerEditor->getCurrentControllerType();
    
    if (type != 201 /*tempo*/)
    {
        const int controlEventsAmount = track->m_control_events.size();
        for(int n=0; n<controlEventsAmount; n++)
        {
            
            if (track->m_control_events[n].getTick() == m_x and
                track->m_control_events[n].getController() == m_controller)
            {
                // if event was added where there was no event before
                if (m_removed_event_value == -1)
                {
                    track->m_control_events.erase(n);
                    break;
                }
                else
                    // if event replaced an event that existed before
                {
                    track->m_control_events[n].setValue( m_removed_event_value );
                }
            }//endif
        }//next
    }
    else //tempo
    {
        const int tempoEventsAmount = track->sequence->tempoEvents.size();
        for(int n=0; n<tempoEventsAmount; n++)
        {
            
            if (track->sequence->tempoEvents[n].getTick() == m_x and
                track->sequence->tempoEvents[n].getController() == m_controller)
            {
                // if event was added where there was no event before
                if (m_removed_event_value == -1)
                {
                    track->sequence->tempoEvents.erase(n);
                    break;
                }
                else
                    // if event replaced an event that existed before
                {
                    track->sequence->tempoEvents[n].setValue( m_removed_event_value );
                }
            }//endif
        }//next
    }
    
}

// ---------------------------------------------------------------------------------------------------------

void AddControlEvent::perform()
{
    ControllerEvent* event = new ControllerEvent(track->sequence, m_controller, m_x, m_value);
    track->addControlEvent( event, &m_removed_event_value );
}

// ---------------------------------------------------------------------------------------------------------

using namespace AriaMaestosa;
UNIT_TEST(TestInsert)
{    
    Sequence* seq = new Sequence(NULL, NULL, NULL, false);
    
    Track* t = new Track(seq);
    
    // make a factory sequence to work from
    seq->importing = true;
    t->addControlEvent_import(0,   64,  0);
    t->addControlEvent_import(100, 127, 0);
    t->addControlEvent_import(200, 64,  0);
    t->addControlEvent_import(300, 0,   0);
    seq->importing = false;
    require(t->getControllerEventAmount(0) == 4, "sanity check"); // sanity check on the way...

    seq->addTrack(t);
    
    // test the action
    seq->getTrack(0)->action(new AddControlEvent(150, 100, 0));
    
    require(t->getControllerEventAmount(0) == 5, "the number of events was increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0, "events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 64, "events were properly ordered");

    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");

    require(t->getControllerEvent(2, 0)->getTick()  == 150, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 100, "events were properly ordered");

    require(t->getControllerEvent(3, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 64, "events were properly ordered");

    require(t->getControllerEvent(4, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(4, 0)->getValue() == 0, "events were properly ordered");

    delete seq;
}

// ---------------------------------------------------------------------------------------------------------

UNIT_TEST(TestOverwriteFirst)
{    
    Sequence* seq = new Sequence(NULL, NULL, NULL, false);
    
    Track* t = new Track(seq);
    
    // make a factory sequence to work from
    seq->importing = true;
    t->addControlEvent_import(0,   64,  0);
    t->addControlEvent_import(100, 127, 0);
    t->addControlEvent_import(200, 64,  0);
    t->addControlEvent_import(300, 0,   0);
    seq->importing = false;
    require(t->getControllerEventAmount(0) == 4, "sanity check"); // sanity check on the way...
    
    seq->addTrack(t);
    
    // test the action
    seq->getTrack(0)->action(new AddControlEvent(0, 100, 0));
    
    require(t->getControllerEventAmount(0) == 4, "the number of events was not increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0, "events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 100, "value of first event was changed");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 64, "events were properly ordered");
    
    require(t->getControllerEvent(3, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 0, "events were properly ordered");
    
    seq->getTrack(0)->action(new AddControlEvent(200, 100, 0));
    
    require(t->getControllerEventAmount(0) == 4, "the number of events was not increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0, " events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 100, "events were properly ordered");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 100, "value of third event was changed");
    
    require(t->getControllerEvent(3, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 0,   "events were properly ordered");
    
    
    delete seq;
}

// ---------------------------------------------------------------------------------------------------------

UNIT_TEST(TestOverwriteLast)
{    
    Sequence* seq = new Sequence(NULL, NULL, NULL, false);
    
    Track* t = new Track(seq);
    
    // make a factory sequence to work from
    seq->importing = true;
    t->addControlEvent_import(0,   64,  0);
    t->addControlEvent_import(100, 127, 0);
    t->addControlEvent_import(200, 64,  0);
    t->addControlEvent_import(300, 0,   0);
    seq->importing = false;
    require(t->getControllerEventAmount(0) == 4, "sanity check"); // sanity check on the way...
    
    seq->addTrack(t);
    
    // test the action
    seq->getTrack(0)->action(new AddControlEvent(300, 100, 0));
    
    require(t->getControllerEventAmount(0) == 4, "the number of events was not increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0,   "events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 64,  "events were properly ordered");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 64,  "events were properly ordered");
    
    require(t->getControllerEvent(3, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 100, "events were properly ordered");
    
    seq->getTrack(0)->action(new AddControlEvent(200, 111, 0));
    
    require(t->getControllerEventAmount(0) == 4, "the number of events was not increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0, " events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 64, "events were properly ordered");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 111, "value of third event was changed");
    
    require(t->getControllerEvent(3, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 100, "events were properly ordered");
    
    
    delete seq;
}

// ---------------------------------------------------------------------------------------------------------

UNIT_TEST(TestAppend)
{    
    Sequence* seq = new Sequence(NULL, NULL, NULL, false);
    
    Track* t = new Track(seq);
    
    // make a factory sequence to work from
    seq->importing = true;
    t->addControlEvent_import(0,   64,  0);
    t->addControlEvent_import(100, 127, 0);
    t->addControlEvent_import(200, 64,  0);
    t->addControlEvent_import(300, 0,   0);
    seq->importing = false;
    require(t->getControllerEventAmount(0) == 4, "sanity check"); // sanity check on the way...
    
    seq->addTrack(t);
    
    // test the action
    seq->getTrack(0)->action(new AddControlEvent(400, 123, 0));
    
    require(t->getControllerEventAmount(0) == 5, "the number of events was increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0,  "events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 64, "events were properly ordered");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 127, "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 64,  "events were properly ordered");
    
    require(t->getControllerEvent(3, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 0,   "events were properly ordered");
    
    require(t->getControllerEvent(4, 0)->getTick()  == 400, "new event was added at the end");
    require(t->getControllerEvent(4, 0)->getValue() == 123, "new event was added at the end");
    
    delete seq;
}

// ---------------------------------------------------------------------------------------------------------

UNIT_TEST(TestPrepend)
{    
    Sequence* seq = new Sequence(NULL, NULL, NULL, false);
    
    Track* t = new Track(seq);
    
    // make a factory sequence to work from
    seq->importing = true;
    t->addControlEvent_import(50,   64,  0);
    t->addControlEvent_import(100, 127, 0);
    t->addControlEvent_import(200, 64,  0);
    t->addControlEvent_import(300, 0,   0);
    seq->importing = false;
    require(t->getControllerEventAmount(0) == 4, "sanity check"); // sanity check on the way...
    
    seq->addTrack(t);
    
    // test the action
    seq->getTrack(0)->action(new AddControlEvent(0, 53, 0));
    
    require(t->getControllerEventAmount(0) == 5, "the number of events was increased");
    require(t->getControllerEvent(0, 0)->getTick()  == 0,   "events were properly ordered");
    require(t->getControllerEvent(0, 0)->getValue() == 53,  "events were properly ordered");
    
    require(t->getControllerEvent(1, 0)->getTick()  == 50,  "events were properly ordered");
    require(t->getControllerEvent(1, 0)->getValue() == 64,  "events were properly ordered");
    
    require(t->getControllerEvent(2, 0)->getTick()  == 100, "events were properly ordered");
    require(t->getControllerEvent(2, 0)->getValue() == 127, "events were properly ordered");

    require(t->getControllerEvent(3, 0)->getTick()  == 200, "events were properly ordered");
    require(t->getControllerEvent(3, 0)->getValue() == 64,  "events were properly ordered");
    
    require(t->getControllerEvent(4, 0)->getTick()  == 300, "events were properly ordered");
    require(t->getControllerEvent(4, 0)->getValue() == 0,   "events were properly ordered");
    
    delete seq;
}

