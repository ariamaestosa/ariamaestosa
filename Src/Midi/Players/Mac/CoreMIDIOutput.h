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

#ifndef __CORE_MIDI_OUTPUT_H__
#define __CORE_MIDI_OUTPUT_H__

#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph
#include "Midi/Players/Mac/OutputBase.h"

#include <vector>

class CoreMidiOutput : public OutputBase
{
public:
    
    CoreMidiOutput();
    virtual ~CoreMidiOutput();
    
    struct Destination
    {
        MIDIEndpointRef m_ref;
        std::string m_name;
    };
    static const std::vector<Destination>& getDestinations();
    
    virtual void note_on(const int note, const int volume, const int channel);
    virtual void note_off(const int note, const int channel);
    virtual void prog_change(const int instrument, const int channel);
    virtual void controlchange(const int controller, const int value, const int channel);
    virtual void pitch_bend(const int value, const int channel);
    virtual void reset_all_controllers();
};

#endif
