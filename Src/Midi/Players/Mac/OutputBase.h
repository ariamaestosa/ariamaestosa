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

#ifndef __OUTPUT_BASE_H__
#define __OUTPUT_BASE_H__

#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph

class OutputBase
{
protected:
    MIDIClientRef m_client;
    
public:
    OutputBase();
    virtual ~OutputBase();
    
    void playNote(int pitchID, int volume, int duration, int channel, int instrument);
    void stopNote();
    
    virtual void note_on(const int note, const int volume, const int channel) = 0;
    virtual void note_off(const int note, const int channel) = 0;
    virtual void prog_change(const int instrument, const int channel) = 0;
    virtual void controlchange(const int controller, const int value, const int channel) = 0;
    virtual void pitch_bend(const int value, const int channel) = 0;
    
    void reset_all_controllers();
};

#endif
