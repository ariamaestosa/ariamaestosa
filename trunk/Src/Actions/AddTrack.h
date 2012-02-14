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

#ifndef _addtrack_
#define _addtrack_

#include "Actions/EditAction.h"

namespace AriaMaestosa
{
    class Track;
    class GraphicalSequence;
    
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class AddTrack : public MultiTrackAction
        {
            /** A pointer to the track that was added, or NULL if action not performed yet */
            Track* m_added_track;
            
            /** if non-NULL, the new track will use the same settings as this */
            Track* m_model;
            
        public:
            /**
              * @param model if non-NULL, the new track will use the same settings as 'model'
              */
            AddTrack(Track* model=NULL);
            virtual ~AddTrack();

            void perform();
            void undo();
        };
        
    }
}
#endif
