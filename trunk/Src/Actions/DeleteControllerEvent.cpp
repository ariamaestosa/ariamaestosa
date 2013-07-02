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

#include "AriaCore.h"
#include "Actions/DeleteControllerEvent.h"
#include "Actions/EditAction.h"
#include "Midi/Note.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"

// FIXME(DESIGN): this action should not rely on GUI classes
#include "GUI/GraphicalTrack.h"
#include "Editors/ControllerEditor.h"

#include "UnitTest.h"
#include <cmath>

#include <wx/intl.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

DeleteControllerEvent::DeleteControllerEvent(int tick) :
    //I18N: (undoable) action name
    SingleTrackAction( _("delete controller event") )
{
    m_id_controller = -1;
    m_tick = tick;
}

// ----------------------------------------------------------------------------------------------------------

DeleteControllerEvent::~DeleteControllerEvent()
{
}

// ----------------------------------------------------------------------------------------------------------

void DeleteControllerEvent::undo()
{
    const int controlAmount = removedControlEvents.size();
    
    if (m_id_controller == PSEUDO_CONTROLLER_LYRICS)
    {
        for (int n=0; n<controlAmount; n++)
        {
            m_track->getSequence()->addTextEvent( (TextEvent*)removedControlEvents.get(n) );
        }
        // we will be using the notes again, make sure it doesn't delete them
        removedControlEvents.clearWithoutDeleting();
    }
    else if (controlAmount > 0)
    {
        
        for (int n=0; n<controlAmount; n++)
        {
            m_track->addControlEvent( removedControlEvents.get(n) );
        }
        // we will be using the notes again, make sure it doesn't delete them
        removedControlEvents.clearWithoutDeleting();
        
    }
}

// ----------------------------------------------------------------------------------------------------------

void DeleteControllerEvent::perform()
{
    ASSERT(m_track != NULL);
    

    ptr_vector<ControllerEvent>& ctrls = m_visitor->getControlEventVector();
    
    ControllerEditor* editor = m_track->getGraphics()->getControllerEditor();
    const int type = editor->getCurrentControllerType();

    m_id_controller = type;
    
    if (type == PSEUDO_CONTROLLER_TEMPO)
    {
        // remove tempo events
        Sequence* sequence = m_track->getSequence();
        const int tempoEventsAmount = sequence->getTempoEventAmount();
        for (int n=0; n<tempoEventsAmount; n++)
        {
            const int tick = sequence->getTempoEvent(n)->getTick();
            
            if (tick != m_tick) continue; // this event is not concerned by selection
            
            removedControlEvents.push_back( sequence->extractTempoEvent(n) );
        }//next
        sequence->removeMarkedTempoEvents();
    }
    else if (type == PSEUDO_CONTROLLER_LYRICS)
    {
        // remove tempo events
        Sequence* sequence = m_track->getSequence();
        const int textEventsAmount = sequence->getTextEventAmount();
        for (int n=0; n<textEventsAmount; n++)
        {
            const int tick = sequence->getTextEvent(n)->getTick();
            
            if (tick != m_tick) continue; // this event is not concerned by selection
            
            removedControlEvents.push_back( sequence->extractTextEvent(n) );
        }//next
        sequence->removeMarkedTextEvents();
    }
    else
    {
        // remove controller events
        for (int n=0; n<ctrls.size(); n++)
        {
            
            if (ctrls[n].getController() != type) continue; // in another controller
            
            const int tick = ctrls[n].getTick();
            
            if (tick != m_tick) continue; // this event is not concerned by selection
            
            removedControlEvents.push_back( ctrls.get(n) );
            ctrls.remove(n);
            n--;
        }//next
        
    }
    
}


