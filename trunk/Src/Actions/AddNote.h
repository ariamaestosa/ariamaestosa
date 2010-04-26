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


#ifndef _addnote_
#define _addnote_

#include "Actions/EditAction.h"

namespace AriaMaestosa
{
    class Track;
    
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class AddNote : public SingleTrackAction
        {
            friend class AriaMaestosa::Track;
            int pitchID, startTick, endTick, volume, string;
            
            NoteRelocator relocator;
        public:
            AddNote(const int pitchID, const int startTick, const int endTick, const int volume, const int string=-1);
            void perform();
            void undo();
            virtual ~AddNote();
        };
    }
}
#endif
