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

#ifndef _common_midi_utils_
#define _common_midi_utils_

// forward
namespace jdkmidi{ class MIDIMultiTrack; }

namespace AriaMaestosa {

class Sequence; // forward

// these functions are to be used by PlatformMidiManager to ease generating midi data
void makeMidiBytes(Sequence* sequence, bool selectionOnly, /*out*/int* songlength, /*out*/int* startTick, /*out*/char** midiSongData, /*out*/int* datalength, bool playing);

// write a midi file
bool exportMidiFile(Sequence* sequence, wxString filepath);

bool makeJDKMidiSequence(Sequence* sequence, jdkmidi::MIDIMultiTrack& tracks, bool selectionOnly,
						 /*out*/int* songLengthInTicks, /*out*/int* startTick, /*out*/ int* numTracks, bool playing);

int convertTempoBendToBPM(int val);


class AriaSequenceTimer
{
    Sequence* seq;
    public:

    AriaSequenceTimer(Sequence* seq);
    void run();
};

}

#endif
