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

#include "Utils.h"
#include <sstream>
#include <vector>
#include "jdkmidi/filewrite.h"

namespace AriaMaestosa
{
    
    /**
     * libjdkmidi by default can only save midi bytes to a file.
     * So i wrote this "fake stream" that captures the bytes and stores them in memory rather than to a file.
     * This is a quick'n'dirty hack that should probably be reconsidered, even though it doesn't seem to
     * cause major performance issues. (FIXME)
     */
    class MidiToMemoryStream : public jdkmidi::MIDIFileWriteStream
    {
        std::vector<char> data;
        int pos, length;
        
    public:
        LEAK_CHECK();
        
        MidiToMemoryStream();
        ~MidiToMemoryStream();
        
        long Seek( const long pos, const int whence );
        int  WriteChar( const int c );
        int  getDataLength();
        void storeMidiData(char* midiData);
    };
    
}
#endif
