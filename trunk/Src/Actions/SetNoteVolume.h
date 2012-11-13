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

#ifndef _setnotevolume_
#define _setnotevolume_

#include "Actions/EditAction.h"
#include <vector>

namespace AriaMaestosa
{
    class Track;
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class SetNoteVolume : public SingleTrackAction
        {
            int m_value, m_note_ID;
            bool m_increment;
            friend class AriaMaestosa::Track;
            
            /** for undo */
            int m_old_volume;
            
            NoteRelocator relocator;
            std::vector<int> m_volumes;
            
            void adjustVolume(int& volume);
            
        public:
            
            /**
            * if increment is false : value is the volume to apply to the note(s)
            * if increment is true : value is an increment to apply to current note volume(s)
            */
            SetNoteVolume(const int value, const int noteID, const bool increment = false);
            void perform();
            void undo();
            virtual ~SetNoteVolume();
        };
        
    }
}
#endif
