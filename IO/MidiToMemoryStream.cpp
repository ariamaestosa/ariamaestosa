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

#include "IO/MidiToMemoryStream.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

/*
 * libjdkmidi by default can only save midi bytes to a file.
 * So i wrote this "fake stream" that captures the bytes and stores them in memory rather than to a file.
 * This is a quick'n'dirty hack that should probably be reconsidered, even though it doesn't seem to cause major performance issues. (FIXME)
 */

namespace AriaMaestosa {

MidiToMemoryStream::MidiToMemoryStream() : MIDIFileWriteStream()
{
    pos=0;
    length = 0;
}


long MidiToMemoryStream::Seek( const long pos_add, const int whence )
{

    if(whence == SEEK_SET) pos = pos_add; // i think this one is the only once used
    else if(whence == SEEK_CUR) pos += pos_add;
    else if(whence == SEEK_END) pos = length-2;

    return 0;
}

int MidiToMemoryStream::WriteChar( const int c )
{

    if(pos==length)
    {
        data.push_back(c);
        length++;
    }
    else
    {
        data[pos] = c;
    }
    pos++;
    return 1;
}

int MidiToMemoryStream::getDataLength()
{
    return length;
}

void MidiToMemoryStream::storeMidiData(char* midiData)
{

    for(int n=0; n<length; n++)
    {
        midiData[n]=data[n];
    }

}

MidiToMemoryStream::~MidiToMemoryStream()
{
}

}
