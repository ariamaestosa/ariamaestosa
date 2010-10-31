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

#ifndef __COMMON_MIDI_UTILS_H__
#define __COMMON_MIDI_UTILS_H__

/** @defgroup midi */

#include <wx/string.h>

// forward
namespace jdkmidi{ class MIDIMultiTrack; }

namespace AriaMaestosa
{
    
    class Sequence; // forward
    
    /**
      * @brief used to ease generating midi data
      * @ingroup midi
      */
    void allocAsMidiBytes(Sequence* sequence, bool selectionOnly, /*out*/int* songlength, /*out*/int* startTick, /*out*/char** midiSongData, /*out*/int* datalength, bool playing);
    
    /**
      * @brief write a midi file
      * @ingroup midi
      */
    bool exportMidiFile(Sequence* sequence, wxString filepath);
    
    /**
      * @brief converts an Aria sequence into a libjdkmidi sequence
      * @ingroup midi
      */
    bool makeJDKMidiSequence(Sequence* sequence, jdkmidi::MIDIMultiTrack& tracks, bool selectionOnly,
                             /*out*/int* songLengthInTicks, /*out*/int* startTick, /*out*/ int* numTracks, bool playing);
    
    /** @ingroup midi */
    int convertTempoBendToBPM(int val);
    
    
}

#endif
