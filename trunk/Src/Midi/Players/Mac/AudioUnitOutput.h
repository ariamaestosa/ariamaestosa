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

#include "Midi/Players/Mac/OutputBase.h"

/**
  * @ingroup midi.players
  *
  * An OS X player that uses AudioUnit to output to the built-in software synthesizer.
  */
class AudioUnitOutput : public OutputBase
{
    AUGraph m_graph;
    AudioUnit m_synth_unit;
    const char* m_custom_sound_font;
    
    void setBank(uint8_t midiChannelInUse);
    void programChange(uint8_t progChangeNum, uint8_t midiChannelInUse);
    
public:
    
    AudioUnitOutput(const char* custom_sound_font);
    ~AudioUnitOutput();
    
    bool outputToDisk(const char* outputFilePath,
                      const char* data,
                      const int data_size);

    void note_on(const int note, const int volume, const int channel);
    void note_off(const int note, const int channel);
    void prog_change(const int instrument, const int channel);
    void controlchange(const int controller, const int value, const int channel);
    void pitch_bend(const int value, const int channel);

};

#endif
