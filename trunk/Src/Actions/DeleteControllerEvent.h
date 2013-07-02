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

#ifndef __DELETE_CONTROLLER_EVENT_H__
#define __DELETE_CONTROLLER_EVENT_H__

#include "Actions/EditAction.h"
#include "ptr_vector.h"


namespace AriaMaestosa
{
    class Track;
    class Note;
    class Editor;
    
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class DeleteControllerEvent : public SingleTrackAction
        {
            friend class AriaMaestosa::Track;
            
            ptr_vector<ControllerEvent> removedControlEvents;
            int m_id_controller;
            int m_tick;
            
        public:
            
            DeleteControllerEvent(int tick);
            void perform();
            void undo();
            virtual ~DeleteControllerEvent();
        };
        
        
    }
}
#endif
