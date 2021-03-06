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

#ifndef __SET_TRACK_VOLUME
#define __SET_TRACK_VOLUME

#include "Actions/EditAction.h"

namespace AriaMaestosa
{
    class Track;
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class SetTrackVolume : public SingleTrackAction
        {
            int m_volume;
            friend class AriaMaestosa::Track;
            
            /** for undo */
            int m_old_volume;
            
        public:
            
            SetTrackVolume(const int volume);
            void perform();
            void undo();
            virtual ~SetTrackVolume();
        };
        
    }
}
#endif
