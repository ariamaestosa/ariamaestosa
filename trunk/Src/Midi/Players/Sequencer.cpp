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

#include <wx/thread.h>

#include "GUI/MainFrame.h"
#include "Midi/Players/Sequencer.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"

#include "jdkmidi/multitrack.h"
#include "jdkmidi/sequencer.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/process.h"

// FIXME: the build system should check for them.
#ifdef __WXMSW__
#define HAVE_GETIMEOFDAY 0
#define HAVE_FTIME 1
#else
#define HAVE_GETIMEOFDAY 1
#define HAVE_FTIME 1
#endif

#if HAVE_GETIMEOFDAY
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif

namespace AriaMaestosa
{

// ------------------------------------------------------------
//     generic portable sequencer/timer implementation
// ------------------------------------------------------------

#if 0
#pragma mark -
#endif

#if HAVE_GETIMEOFDAY

class GetTimeOfDayTimer
{
    timeval _init_time;
public:

    void reset_and_start()
    {
        gettimeofday(&_init_time, 0);
    }

    int get_elapsed_millis()
    {
        timeval curr_time;
        timeval elap_time;
        gettimeofday(&curr_time, 0);
        timersub(&curr_time, &_init_time, &elap_time);
        return elap_time.tv_sec * 1000 + elap_time.tv_usec / 1000;
    }
};
typedef GetTimeOfDayTimer BasicTimer;

#elif HAVE_FTIME

class FtimeTimer
{
    long initial_sec, initial_millis;
    public:

    void reset_and_start()
    {
        // FIXME - ftime is apparently obsolete (http://linux.die.net/man/3/ftime)
        // use http://linux.die.net/man/2/gettimeofday instead

        timeb tb;
        ftime(&tb);
        initial_sec= tb.time;
        initial_millis = tb.millitm;
        //std::cout << "time = " << tb.time << " seconds " << tb.millitm << " millis" << std::endl;
    }

    int get_elapsed_millis()
    {
        timeb tb;
        ftime(&tb);

        const long current_sec = tb.time;
        const long current_millis = tb.millitm;

        const long delta_sec = current_sec - initial_sec;

        const long total_millis = delta_sec*1000 - initial_millis + current_millis;
        return total_millis;

    }
};
typedef FtimeTimer BasicTimer;

#else

class DummyTimer
{
    long time;
    public:
    void reset_and_start(){ time=0; }
    int get_elapsed_millis(){ time+=13; return time; }
};
typedef DummyTimer BasicTimer;

#endif

AriaSequenceTimer::AriaSequenceTimer(Sequence* seq)
{
    m_seq = seq;
}

BasicTimer* timer = NULL;

void cleanup_sequencer()
{
    if (timer != NULL) delete timer;
    timer = NULL;
}

void AriaSequenceTimer::run(jdkmidi::MIDISequencer* jdksequencer, const int songLengthInTicks)
{

    //std::cout << "  * AriaSequenceTimer::run" << std::endl;

    //std::cout << "trying to play " << seq->suggestFileName().mb_str() << std::endl;

    jdksequencer->GoToTimeMs( 0 );

    int bpm = m_seq->getTempo();
    const int beatlen = m_seq->ticksPerBeat();

    double ticks_per_millis = (double)bpm * (double)beatlen / (double)60000.0;

    //std::cout << "bpm = " << bpm << " beatlen=" << beatlen << " ticks_per_millis=" << ticks_per_millis << std::endl;

    float next_event_time = 0;

    jdkmidi::MIDITimedBigMessage ev;
    int ev_track;

    jdkmidi::MIDIClockTime tick;
    if (not jdksequencer->GetNextEventTime(&tick))
    {
        std::cerr << "[AriaSequenceTimer] failed to get first event time, returning" << std::endl;
        cleanup_sequencer();
        return;
    }
    
    //jdksequencer->ResetAllTracks();
    
    long previous_tick = tick;
    
    next_event_time = tick / ticks_per_millis;
    
    timer = new BasicTimer();
    timer->reset_and_start();
    
    long total_millis = 0;
    long last_millis = 0;
    
    int next_metronome_beat = -1;
    int played_metronome_tick = -1;
    
    int next_beat = 0;
    
    while (PlatformMidiManager::get()->seq_must_continue() or PlatformMidiManager::get()->isRecording())
    {
        // process all events that need to be done by the current tick
        while (next_event_time <= total_millis)
        {
            if (not jdksequencer->GetNextEvent( &ev_track, &ev ))
            {
                if (not PlatformMidiManager::get()->isRecording())
                {
                    std::cerr << "error, failed to retrieve next event, returning" << std::endl;
                    cleanup_sequencer();
                    return;
                }
                else
                {
                    Sequence* sequence = getMainFrame()->getCurrentSequence();
                    if (sequence->playWithMetronome())
                    {
                        // past the end of the song in record mode. Play metronome
                        const int beat = sequence->ticksPerBeat();
                        int metronome_beat = tick - (tick % beat);
                        if (next_metronome_beat == -1 or metronome_beat > next_metronome_beat)
                        {
                            next_metronome_beat = metronome_beat;
                        }
                        
                        const int metronomeInstrument = 37; // 31 (stick), 56 (cowbell), 37 (side stick)
                        const int metronomeVolume = 127;
                        
                        
                        if ((int)tick >= next_metronome_beat and next_metronome_beat != played_metronome_tick)
                        {
                            PlatformMidiManager::get()->seq_note_on(metronomeInstrument, metronomeVolume, 9);
                            played_metronome_tick = next_metronome_beat;
                        }
                    }
                }
            }
            const int channel = ev.GetChannel();

            if (ev.IsNoteOn())
            {
                const int note = ev.GetNote();
                const int volume = ev.GetVelocity();
                PlatformMidiManager::get()->seq_note_on(note, volume, channel);
            }
            else if (ev.IsNoteOff())
            {
                const int note = ev.GetNote();
                PlatformMidiManager::get()->seq_note_off(note, channel);
            }
            else if (ev.IsControlChange())
            {
                const int controllerID = ev.GetController();
                const int value = ev.GetControllerValue();
                PlatformMidiManager::get()->seq_controlchange(controllerID, value, channel);
            }
            else if (ev.IsPitchBend())
            {
                const int pitchBendVal = ev.GetBenderValue();
                PlatformMidiManager::get()->seq_pitch_bend(pitchBendVal, channel);
            }
            else if (ev.IsProgramChange())
            {
                const int instrument = ev.GetPGValue();
                PlatformMidiManager::get()->seq_prog_change(instrument, channel);
            }
            else if (ev.IsTempo())
            {
                //std::cout << "tempo event" << std::endl;
                const int event_bpm = ev.GetTempo32()/32;
                ticks_per_millis = (double)event_bpm * (double)beatlen / (double)60000.0;
            }
            /*
            else if ( ev.IsPolyPressure() )
                std::cout << "poly pressure" << std::endl;
            else if ( ev.IsChannelPressure() )
                std::cout << "channel pressure" << std::endl;
            else if ( ev.IsSystemMessage() )
                std::cout << "system message " << std::hex << ev.GetType() << std::endl;

            if ( ev.IsSysEx() )
                std::cout << "sys ex " << ev.GetSysExNum() << std::endl;
            else if ( ev.IsSongPosition() )
                std::cout << "song position" << std::endl;
            else if ( ev.IsNoOp() )
                std::cout << "no op" << std::endl;
            else if ( ev.IsBeatMarker() )
                std::cout << "beat marker" << std::endl;
            else
            {
                std::cout << "unknown event : " << ev.GetType() << std::endl;
            }*/

            previous_tick = tick;

            if (not jdksequencer->GetNextEventTime(&tick))
            {
                if (PlatformMidiManager::get()->isRecording())
                {
                    Sequence* seq = getMainFrame()->getCurrentSequence();
                    tick = previous_tick + seq->ticksPerBeat();
                }
                else
                {
                    cleanup_sequencer();
                    return;
                }
            }
            if ((long)tick <(long) previous_tick) continue; // something wrong about time order...

            if ((long)tick > (long)songLengthInTicks)
            {
                PlatformMidiManager::get()->seq_notify_current_tick(-1);
                if (not PlatformMidiManager::get()->isRecording())
                {
                    std::cout << "done, thread will exit" << std::endl;
                    cleanup_sequencer();
                    return;
                }
            }

            PlatformMidiManager::get()->seq_notify_current_tick(previous_tick);

            //std::cout << "next_event_time was " << next_event_time << " adding " <<
            //((tick - previous_tick) / ticks_per_millis) << " tick=" << tick <<
            //" previous_tick=" << previous_tick << "ticks_per_millis=" << ticks_per_millis << std::endl;
            next_event_time += (tick - previous_tick) / ticks_per_millis;

            /*
            static int i = 0;
            i++; if (i>10) i=0;
            if (i == 1)
            {
                std::cout << "next_event_time = " << next_event_time << "  -> tick = " << tick << std::endl;
            }
            */

        }
        
        wxThread::Sleep(10);
        
        
        last_millis = total_millis;
        const int delta = (timer->get_elapsed_millis() - last_millis);
        
        total_millis += delta;
        
        PlatformMidiManager::get()->seq_notify_accurate_current_tick(total_millis*ticks_per_millis);
        
        if (PlatformMidiManager::get()->isRecording())
        {
            const int tick = total_millis*ticks_per_millis;
            if (tick >= next_beat)
            {
                wxCommandEvent evt(wxEVT_EXTEND_TICK, wxID_ANY);
                evt.SetInt( tick );
                getMainFrame()->GetEventHandler()->AddPendingEvent( evt );
                
                Sequence* seq = getMainFrame()->getCurrentSequence();
                next_beat += seq->ticksPerBeat();
            }
        }
    }
    
    cleanup_sequencer();
}



}


