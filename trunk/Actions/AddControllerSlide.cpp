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

#include "Actions/AddControllerSlide.h"
#include "Editors/ControllerEditor.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/ControllerEvent.h"
#include "Midi/Sequence.h"

#include "Midi/MeasureData.h"


namespace AriaMaestosa
{
    namespace Action
{
    void AddControllerSlide::undo()
    {
        ControllerEvent* current_event;
        relocator.setParent(track);
        relocator.prepareToRelocate();

        //std::cout << "will undo control slide" << std::endl;
       // std::cout << "track->controlEvents.size() = " << track->controlEvents.size() << std::endl;

        const int type = track->graphics->controllerEditor->getCurrentControllerType();

        if(type != 201 /*tempo*/)
        {
            while( (current_event = relocator.getNextControlEvent()) and current_event != NULL)
            {
                const int controlEventsAmount = track->controlEvents.size();
                for(int n=0; n<controlEventsAmount; n++)
                {
                    if(track->controlEvents.get(n) == current_event)
                    {
                        track->controlEvents.erase(n);
                        break;
                    }//endif
                }//next
            }//wend
        }
        else
        { // tempo events
            while( (current_event = relocator.getNextControlEvent()) and current_event != NULL)
            {
                // remove tempo events
                const int tempoEventsAmount = track->sequence->tempoEvents.size();
                for(int n=0; n<tempoEventsAmount; n++)
                {
                    if(track->sequence->tempoEvents.get(n) == current_event)
                    {
                        track->sequence->tempoEvents.erase(n);
                        break;
                    }//endif
                }//next
            }//wend
        }

        // add back any control event that was removed
        const int controlAmount = removedControlEvents.size();
        if(controlAmount > 0)
        {

            for(int n=0; n<controlAmount; n++)
            {
                // FIXME - will iterate through all events everytime... could have better performances
                track->addControlEvent( removedControlEvents.get(n) );
            }
            // we will be using these events again, make sure it doesn't delete them
            removedControlEvents.clearWithoutDeleting();

        }
    }

    void AddControllerSlide::addOneEvent(ControllerEvent* ptr, ptr_vector<ControllerEvent>* vector, int id)
    {
        vector->add( ptr, id );
        relocator.rememberControlEvent(*ptr);
    }
    void AddControllerSlide::pushBackOneEvent(ControllerEvent* ptr, ptr_vector<ControllerEvent>* vector)
    {
        vector->push_back( ptr );
        relocator.rememberControlEvent(*ptr);
    }

    void AddControllerSlide::perform()
    {

        assertExpr(controller,>=,0);
        assertExpr(controller,<,205);
        assertExpr(x1,>=,0);
        assertExpr(x2,>=,0);
        assertExpr(value1,>=,0);
        assertExpr(value1,<,128);
        assertExpr(value2,>=,0);
        assertExpr(value2,<,128);

        int addedAmount=0;

        // the vector events will be added to - allows to use the same code for multiple uses
        ptr_vector<ControllerEvent>* vector;

        // tempo events
        if(controller==201) vector = &track->sequence->tempoEvents;
        // controller and pitch bend events
        else vector = &track->controlEvents;

        /*
        std::cout << "************************************************" << std::endl;
        std::cout << "before starting" << std::endl;
           for(int ixc=0; ixc<track->controlEvents.size(); ixc++)
               std::cout << ((float)track->controlEvents[ixc].getTick()/(float)getMeasureData()->beatLengthInTicks()) << std::endl;
        */

        // track is empty, events can be added without any further checking
        if(vector->size()==0)
        {

            int previous_value = value1;

            addOneEvent( new ControllerEvent(track->sequence, controller, x1, previous_value), vector, 0 );
            //vector->add( new ControllerEvent(track->sequence, controller, x1, previous_value), 0 );
            addedAmount++;

            for(int tick=0; tick<x2-x1; tick++)
            {
                const int newvalue =
                (int)(
                      value1 + (value2-value1)*((float)tick / (float)(x2-x1))
                      );

                if(newvalue == previous_value )continue;

                addOneEvent( new ControllerEvent(track->sequence, controller, x1+tick, newvalue), vector, addedAmount );
                addedAmount++;
                previous_value = newvalue;
            }//next
            return;
        }//end if

        // remove events located where we will be adding new ones
        for(int n=0; n<vector->size(); n++)
        {
            // we reached end of track, we may now stop
            if( n>=vector->size() ) break;

            // we've not yet reached the area where stuff must be erase. Go forward.
            if( (*vector)[n].getTick()<x1 ) continue;

            // events there are after the area we will add events to, so they are not to be erased and we may stop.
            if( (*vector)[n].getTick()>x2 ) break;

            // all types of controllers go in the same vector.
            // we only want to remove those of the current type
            if( (*vector)[n].getController()==controller)
            {
                removedControlEvents.push_back( vector->get(n) );
                vector->remove(n);
                n-=2; if(n<0) n=-1;
            }

        }

        // find where to add events
        int previous_value = 0;
        int event_i = 0;
        bool addAfterAll=false;

        {
            const int eventAmount=vector->size();

            if(eventAmount == 0) addAfterAll = true;
            else if((*vector)[0].getTick() > x2){ event_i = 0; }
            else if((*vector)[eventAmount-1].getTick() < x1){ addAfterAll=true; }
            else
            {
                while( (*vector)[event_i].getTick() <= x1 and event_i+1 < eventAmount ) { event_i++; }
            }
        }

        // iterate from first tick to last tick of the slide, adding events all along, in time order
        for(int tick=0; tick<x2-x1; tick++)
        {
            // calculate next value in "slide", at the current tick
            const int newvalue =
            (int)(
                  value1 + (value2-value1)*((float)tick / (float)(x2-x1))
                  );

            // don't add useless controller events (two consecutive events with same value)
            if(newvalue == previous_value ) continue;

            if(addAfterAll) pushBackOneEvent( new ControllerEvent(track->sequence, controller, x1+tick, newvalue), vector );
            else
            {
                bool  notAtEnd = true;
                while( (*vector)[event_i].getTick() <= (x1+tick) and notAtEnd)
                {
                    event_i++;
                    if(!(event_i < vector->size()))
                    {
                        addAfterAll=true; // we reached end. append remaining events to end
                        tick--;
                        notAtEnd=false;
                        break;
                    }

                }
                if(notAtEnd)
                {
                    addOneEvent( new ControllerEvent(track->sequence, controller, x1+tick, newvalue), vector,  event_i );
                    event_i++;
                }
            }

            previous_value = newvalue;
        }//next

        /*
        std::cout << "-------------" << std::endl;
        std::cout << "we added new events" << std::endl;
        for(int ixc=0; ixc<track->controlEvents.size(); ixc++)
            std::cout << ((float)track->controlEvents[ixc].getTick()/(float)getMeasureData()->beatLengthInTicks()) << std::endl;
        */

#ifdef _MORE_DEBUG_CHECKS
        std::cout << "checking order" << std::endl;
        track->checkControlEventsOrder();
#endif

        // FIXME - until i'm sure this action is bug-free i'll leave this check after. Should be fixed and removed though.
        track->reorderControlVector();

}
AddControllerSlide::AddControllerSlide(const int x1, const int value1, const int x2, const int value2, const int controller)
{
    AddControllerSlide::x1 = x1;
    AddControllerSlide::value1 = value1;
    AddControllerSlide::x2 = x2;
    AddControllerSlide::value2 = value2;
    AddControllerSlide::controller = controller;
}
AddControllerSlide::~AddControllerSlide()
{
}

}
}
