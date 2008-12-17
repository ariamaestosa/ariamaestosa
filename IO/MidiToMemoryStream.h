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

#ifndef _miditomemorystream_
#define _miditomemorystream_

#include "Config.h"
#include <sstream>
#include <vector>
#include "jdkmidi/filewrite.h"

namespace AriaMaestosa {

class MidiToMemoryStream : public jdkmidi::MIDIFileWriteStream {

    //std::sstream os;

    std::vector<char> data;
    int pos, length;

public:
    LEAK_CHECK(MidiToMemoryStream);

    MidiToMemoryStream();
    ~MidiToMemoryStream();

    long Seek( const long pos, const int whence );
    int WriteChar( const int c );
    //int getMidiData(char* midiData);

    int getDataLength();
    void storeMidiData(char* midiData);
};

}
#endif
