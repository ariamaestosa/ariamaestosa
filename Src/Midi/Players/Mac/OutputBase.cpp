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

#include <wx/timer.h>
#include "Midi/Players/Mac/OutputBase.h"


const int PITCH_BEND_LOWEST = 0;
const int PITCH_BEND_CENTER = 8192;
const int PITCH_BEND_HIGHEST = 16383;

class StopNoteTimer : public wxTimer
{
    
    OutputBase* m_o;
public:
    
    StopNoteTimer(OutputBase* o) : wxTimer()
    {
        m_o = o;
    }
    
    void Notify()
    {
        m_o->stopNote();
        wxTimer::Stop();
    }
    
    void start(int duration)
    {
        Start(duration);
    }
};


StopNoteTimer* stopNoteTimer = NULL;


// ------------------------------------------------------------------------------------------------------

OutputBase::OutputBase()
{
    MIDIClientCreate(CFSTR("MidiOutput"), NULL, NULL, &m_client);
    
    stopNoteTimer = new StopNoteTimer(this);
    m_playing = false;
}

// ------------------------------------------------------------------------------------------------------

OutputBase::~OutputBase()
{
    MIDIClientDispose(m_client);
    
    if (stopNoteTimer != NULL)
    {
        delete stopNoteTimer;
        stopNoteTimer = NULL;
    }
}

// ------------------------------------------------------------------------------------------------------

void OutputBase::playNote(int pitchID, int volume, int duration, int channel, int instrument)
{
    if (m_playing) stopNote();
    
    m_last_note = pitchID;
    m_last_channel = channel;
    
    prog_change(instrument, channel);
    note_on(pitchID, volume, channel);
    stopNoteTimer->start(duration);
    m_playing = true;

    return;
}

// ------------------------------------------------------------------------------------------------------

void OutputBase::stopNote()
{
    if (not m_playing) return;

    note_off(m_last_note, m_last_channel);
    m_playing = false;

    return;
}

// ------------------------------------------------------------------------------------------------------

void OutputBase::reset_all_controllers()
{
    for (int channel=0; channel<16; channel++)
    {
        controlchange(0x78 /*120*/ /* all sound off */, 0, channel);
        controlchange(0x79 /*121*/ /* reset controllers */, 0, channel);
        controlchange(7 /* reset volume */, 127, channel);
        controlchange(10 /* reset pan */, 64, channel);
        pitch_bend(PITCH_BEND_CENTER, channel);
    }
}

#endif
