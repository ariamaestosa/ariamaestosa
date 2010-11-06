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

#ifndef _mvnotes_
#define _mvnotes_

#include "Actions/EditAction.h"

namespace AriaMaestosa
{
    class Track;
    namespace Action
    {
        
        /**
         * @ingroup actions
         */
        class MoveNotes : public SingleTrackAction
        {
            enum MoveMode
            {
                DELTA,           //!< move that only requires to know the amount of steps to perform a correct undo
                SCORE_VERTICAL,  //!< this modes requires more info than just amount of steps to undo correctly
                GUITAR_VERTICAL, //!< this modes requires more info than just amount of steps to undo correctly
                DRUMS_VERTICAL   //!< this modes requires more info than just amount of steps to undo correctly
            };
            
            int m_relativeX, m_relativeY, m_note_ID;
            friend class AriaMaestosa::Track;
            
            MoveMode m_move_mode;
            
            // for undo
            NoteRelocator relocator;
            int m_mode;
            
            // vertical movements in score require a little more than others because of possible accidentals
            std::vector<short> undo_pitch; // for SCORE_VERTICAL mode
            std::vector<short> undo_fret; // for GUITAR_VERTICAL mode
            std::vector<short> undo_string;
            
        public:
            
            /**
              * @param noteID  ID of the note to move in range [0 .. noteAmount - 1], or SELECTED_NOTES
              */
            MoveNotes(const int relativeX, const int relativeY, const int noteID);
            
            void perform();
            void undo();
            
            void doMoveOneNote(const int noteid);
            
            virtual ~MoveNotes();
        };
    }
}
#endif
