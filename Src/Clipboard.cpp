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


/*
 The clipboard will always delete notes you give it.
 When you get notes from the Clipboard, or add notes to it,
 always make a copy of the Note object using the copy constructor.
*/

#include "Clipboard.h"
#include "ptr_vector.h"
#include "Midi/Note.h"

namespace AriaMaestosa {

    namespace Clipboard {

        // a vector to store copied notes
        ptr_vector<Note> clipboard;
        int beat_length = 960; // store beat length of copied notes, in case you want to copy from a song to another with different beat lengths

        void clear()
        {
            clipboard.clearAndDeleteAll();
        }
        void setBeatLength(const int beat_length_arg)
        {
            beat_length = beat_length_arg;
        }
        int getBeatLength()
        {
            return beat_length;
        }

        void add(Note* n)
        {
            clipboard.push_back(n);
        }

        int getSize()
        {
            return clipboard.size();
        }

        Note* getNote( int index )
        {
            ASSERT_E(index, >=, 0);
            ASSERT_E(index, <, clipboard.size());

            return &clipboard[index];
        }

    }

}
