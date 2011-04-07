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

#ifdef _MAC_QUICKTIME_COREAUDIO

#include <vector>
//#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph

namespace CoreAudioNotePlayer
{

    struct Destination
    {
        MIDIEndpointRef m_ref;
        std::string m_name;
    };
    const std::vector<Destination>& getDestinations();
    
    void init();
    void free();

    void playNote(int pitchID, int volume, int duration, int channel, int instrument);
    void stopNote();
    
    void au_seq_note_on(const int note, const int volume, const int channel);
    void au_seq_note_off(const int note, const int channel);
    void au_seq_prog_change(const int instrument, const int channel);
    void au_seq_controlchange(const int controller, const int value, const int channel);
    void au_seq_pitch_bend(const int value, const int channel);
    
    void au_reset_all_controllers();
}

#endif