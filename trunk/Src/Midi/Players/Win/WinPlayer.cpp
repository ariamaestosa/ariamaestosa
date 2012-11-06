/*
 *  Copyright (C) Alexis Archambault
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifdef __WIN32__

#include "jdksmidi/multitrack.h"
#include "jdksmidi/sequencer.h"
#include "AriaCore.h"
#include "Dialogs/WaitWindow.h"
#include "IO/IOUtils.h"
#include "IO/MidiToMemoryStream.h"
#include "Midi/Players/Sequencer.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "PreferencesData.h"

#include <wx/string.h>
#include <wx/utils.h>
#include <wx/process.h>
#include <wx/strconv.h>
#include <wx/timer.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>

#include <sstream>

#include <Windows.h>

/** macro to pack a MIDI short message */
#define MAKEMIDISHORTMSG(cStatus, cChannel, cData1, cData2)            \
cStatus | cChannel | (((UINT)cData1) << 8) | (((DWORD)cData2) << 16)


#define ROUND(x) (int) ((x)+0.5)


// MIDI Status Bytes for Channel Voice Messages
const int MIDI_NOTE_OFF         = 0x80;
const int MIDI_NOTE_ON          = 0x90;
const int MIDI_POLY_PRESSURE    = 0xA0;
const int MIDI_CONTROL_CHANGE   = 0xB0;
const int MIDI_PROGRAM_CHANGE   = 0xC0;
const int MIDI_CHANNEL_PRESSURE = 0xD0;
const int MIDI_AFTERTOUCH       = 0xD0;  // synonym for channel pressure
const int MIDI_PITCH_WHEEL      = 0xE0;


namespace AriaMaestosa
{
    class StopNoteTimer : public wxTimer
    {
    public:
        
        StopNoteTimer() : wxTimer(){ }
        
        void Notify()
        {
            PlatformMidiManager::get()->stopNote();
            wxTimer::Stop();
        }
        
        void start(int duration)
        {
            wxTimer::Start(duration);
        }
    };
    
    // FIXME: cheap way to pass parameters to the thread
    int songLengthInTicks = -1;
    bool playing = false;
    int current_tick = -1;
    int g_current_accurate_tick;

    HMIDIOUT m_hOutMidiDevice = 0;
    bool m_bOutOpen = false;
    int lastPlayedNote = -1, lastChannel;
    StopNoteTimer* stopNoteTimer = NULL;
    
    /** all sound off, reset all controllers, reset pitch bend, etc. */
    void cleanup_after_playback()
    {
        midiOutReset(m_hOutMidiDevice);
    }
    
    /**
      * @ingroup midi.players
      *
      * Helper thread class for playback on Windows
      */
    class SequencerThread : public wxThread
    {
        jdksmidi::MIDIMultiTrack* jdkmidiseq;
        jdksmidi::MIDISequencer* jdksequencer;
        int songLengthInTicks;
        bool selectionOnly;
        int m_start_tick;
        Sequence* sequence;
        
    public:
        
        SequencerThread(const bool selectionOnly, Sequence* sequence)
        {
            jdkmidiseq = NULL;
            jdksequencer = NULL;
            SequencerThread::selectionOnly = selectionOnly;
            SequencerThread::sequence = sequence;
        }
        ~SequencerThread()
        {
            if (jdksequencer != NULL) delete jdksequencer;
            if (jdkmidiseq != NULL) delete jdkmidiseq;
        }
        
        void prepareSequencer()
        {
            jdkmidiseq = new jdksmidi::MIDIMultiTrack();
            songLengthInTicks = -1;
            int trackAmount = -1;
            m_start_tick = 0;
            makeJDKMidiSequence(sequence, *jdkmidiseq, selectionOnly, &songLengthInTicks,
                                &m_start_tick, &trackAmount, true /* for playback */);
            
            //std::cout << "trackAmount=" << trackAmount << " start_tick=" << m_start_tick<<
            //        " songLengthInTicks=" << songLengthInTicks << std::endl;
            
            jdksequencer = new jdksmidi::MIDISequencer(jdkmidiseq);
        }
        
        void go(int* startTick /* out */)
        {
            if (Create() != wxTHREAD_NO_ERROR)
            {
                std::cerr << "error creating thread" << std::endl;
                return;
            }
            SetPriority(85 /* 0 = min, 100 = max */);
            
            prepareSequencer();
            *startTick = m_start_tick;
            
            Run();
        }
        
        ExitCode Entry()
        {
            AriaSequenceTimer timer(sequence);
            timer.run(jdksequencer, songLengthInTicks);
            
            playing = false;
            cleanup_after_playback();
            
            return 0;
        }
    };
    
    
    /**
      * @ingroup midi.players
      *
      * Main interface for playback on Windows
      */
    class WinMidiManager : public PlatformMidiManager
    {
        bool play(Sequence* sequence, /*out*/int* startTick, bool selectionOnly);
    public:
        
        virtual bool playSequence(Sequence* sequence, /*out*/int* startTick)
        {
            return play(sequence, startTick, false);
            /*
             * see 'playSelected' below, it's essentially the same, just
             * replace 'true' with 'false' where it says 'selection only'
             */
        }
        
        virtual bool playSelected(Sequence* sequence, /*out*/int* startTick)
        {
            return play(sequence, startTick, true);
        }
        
        virtual bool isPlaying()
        {
            return playing;
        }
        
        virtual void stop()
        {
            playing = false;
        }
        
        // export to audio file, e.g. .wav
        virtual void exportAudioFile(Sequence* /*sequence*/, wxString /*filepath*/)
        {
        }
        
        virtual const wxString getAudioExtension()
        {
            return wxEmptyString;
            //return wxT(".wav");
        }
        
        virtual const wxString getAudioWildcard()
        {
            return wxEmptyString;
            //return  wxString( _("WAV file")) + wxT("|*.wav");
        }
        
        // get current tick, either from native API or from a variable (current_tick) you keep around and update from the playback thread
        // current_tick is updated by seq_notify_current_tick
        virtual int trackPlaybackProgression()
        {
            if (current_tick == -1)
            {
                cleanup_after_playback();
                return -1;
            }
            
            return current_tick;
        }
        
        virtual int getAccurateTick()
        {
            return g_current_accurate_tick;
        }
        
        wxArrayString m_devices;
        
        class DeviceEnumerator
        {
            int i;
            int midi_num_outputs;
            LPMIDIOUTCAPS midi_out_caps;

        public:
            DeviceEnumerator()
            {
                i = 0;
                midi_num_outputs = midiOutGetNumDevs();
                midi_out_caps = (LPMIDIOUTCAPS)malloc(sizeof(MIDIOUTCAPS)*midi_num_outputs);

            }
            ~DeviceEnumerator()
            {
                free(midi_out_caps);
            }
            
            bool hasNext()
            {
                return i < midi_num_outputs;
            }
            
            void next()
            {
                i++;
            }
            
            int getDeviceID() const
            {
                return i;
            }
            
            wxString getDeviceName()
            {
                DWORD wRtn;
                wRtn = midiOutGetDevCaps(i, (LPMIDIOUTCAPS) & midi_out_caps[i],
                                         sizeof(MIDIOUTCAPS));
                if (wRtn == MMSYSERR_NOERROR)
                {
                    // see http://msdn.microsoft.com/en-us/library/dd798467%28VS.85%29.aspx
                    wxString s;
                    s << wxString(midi_out_caps[i].szPname, wxMBConvUTF16()).mb_str()
                      << wxT(" v") << midi_out_caps[i].vDriverVersion;
                    return s;
                }
                else
                {
                    fprintf(stderr, "[WinPlayer] WARNING: Cannot retrieve name of output device %i\n", i);
                    return wxT("");
                }
            }
        };
        
        virtual wxArrayString getOutputChoices()
        {
            wxArrayString out;
            out.Add( _("Windows Software Synthesizer") );
            for (unsigned int n=0; n<m_devices.size(); n++)
            {
                out.Add(m_devices[n]);
            }
            return out;
        }
        
        // called when app opens
        virtual void initMidiPlayer()
        {
            
            if (not m_bOutOpen)
            {
                wxString driver = PreferencesData::getInstance()->getValue(SETTING_ID_MIDI_OUTPUT);
                
                int deviceID = -1;
                
                // enumerate devices
                for (DeviceEnumerator e; e.hasNext(); e.next())
                {
                    wxString name = e.getDeviceName();
                    m_devices.Add(name);
                    if (name == driver)
                    {
                        deviceID = e.getDeviceID();
                    }
                }
                
                int e;
                
                if (driver == "default" || driver == _("Windows Software Synthesizer"))
                {
                    if (driver == "default")
                    {
                        PreferencesData::getInstance()->setValue(SETTING_ID_MIDI_OUTPUT, _("Windows Software Synthesizer"));
                    }
                    
                    e = ::midiOutOpen(&m_hOutMidiDevice,
                                      MIDI_MAPPER,
                                      NULL,
                                      NULL,
                                      CALLBACK_NULL
                                      );
                }
                else if (deviceID != -1)
                {
                    e = ::midiOutOpen(&m_hOutMidiDevice,
                                      deviceID,
                                      NULL,
                                      NULL,
                                      CALLBACK_NULL
                                      );
                }
                else
                {
                    wxMessageBox( wxString(wxT("Cannot locate MIDI output ")) + driver );
                    return;
                }
                
                if (e != 0)
                {
                    wxMessageBox( _("Failed to initialize MIDI out device") );
                    switch (e)
                    {
                        case MIDIERR_NODEVICE:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : MIDIERR_NODEVICE"));
                            break;
                        case MMSYSERR_ALLOCATED:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : MMSYSERR_ALLOCATED"));
                            break;
                        case MMSYSERR_BADDEVICEID:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : MMSYSERR_BADDEVICEID"));
                            break;
                        case MMSYSERR_INVALPARAM:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : MMSYSERR_INVALPARAM"));
                            break;
                        case MMSYSERR_NOMEM:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : MMSYSERR_NOMEM"));
                            break;
                        default:
                            wxLogError(wxT("Failed to open windows MIDI output device, reason : <unknown>"));
                            break;
                    }
                    wxLogError(wxString::Format(wxT("(%i MIDI devices available)"), midiOutGetNumDevs()));
                    return;
                }
                m_bOutOpen = true;
            }
            
            stopNoteTimer = new StopNoteTimer();
            
            return;
        }
        
        // called when app closes
        virtual void freeMidiPlayer()
        {
            if (m_bOutOpen)
            {
                ::midiOutClose( m_hOutMidiDevice );
                m_bOutOpen=false;
            }
            
            if (stopNoteTimer != NULL)
            {
                delete stopNoteTimer;
                stopNoteTimer = NULL;
            }
            
        }
        
        // play/stop a single preview note (no sequencing is to be done, only direct output)
        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument)
        {
            if (not m_bOutOpen) return;
            
            DWORD dwMsg;
            
            if (lastPlayedNote != -1)
            {
                stopNote();
            }
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_PROGRAM_CHANGE, channel, instrument, 0);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, channel, noteNum, volume);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
            
            ASSERT(stopNoteTimer != NULL);
            stopNoteTimer->start(duration);
            lastPlayedNote = noteNum;
            lastChannel = channel;
        }
        
        virtual void stopNote()
        {
            if (not m_bOutOpen) return;
            
            DWORD dwMsg;
            
            // 0 velocity turns note off
            dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, lastChannel, lastPlayedNote, 0);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
            
            lastPlayedNote = -1;
        }
        
        // ----------------------------------------------------------------------
        // functions below will only be called if you use the generic AriaSequenceTimer
        // if you use a native sequencer instead you can leave them empty
        
        // if you're going to use the generic sequencer/timer, implement those to send events to native API
        virtual void seq_note_on(const int note, const int volume, const int channel)
        {
            DWORD dwMsg;
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, channel, note, volume);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }
        
        
        virtual void seq_note_off(const int note, const int channel)
        {
            DWORD dwMsg;
            
            // 0 velocity turns note off
            dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, channel, note, 0);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }
        
        virtual void seq_prog_change(const int instrument, const int channel)
        {
            DWORD dwMsg;
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_PROGRAM_CHANGE, channel, instrument, 0);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }
        
        virtual void seq_controlchange(const int controller, const int value, const int channel)
        {
            DWORD dwMsg;
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_CONTROL_CHANGE, channel, controller, value);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }
        
        virtual void seq_pitch_bend(const int value, const int channel)
        {
            DWORD dwMsg;
            
            int temp = ROUND(0x2000 * ((double)value + 1));
            if (temp > 0x3fff) temp = 0x3fff; // 14 bits maximum
            if (temp < 0) temp = 0;
            int c1 = temp & 0x7F; // low 7 bits
            int c2 = temp >> 7;   // high 7 bits
            
            dwMsg = MAKEMIDISHORTMSG(MIDI_PITCH_WHEEL, channel, c1, c2);
            ::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }
        
        // called repeatedly by the generic sequencer to tell
        // the midi player what is the current progression
        // the sequencer will call this with -1 as argument to indicate it exits.
        virtual void seq_notify_current_tick(const int tick_arg)
        {
            current_tick = tick_arg;
            
            if (tick_arg == -1)
            {
                // song done
                playing = false;
                cleanup_after_playback();
            }
        }
        
        virtual void seq_notify_accurate_current_tick(const int tick)
        {
            g_current_accurate_tick = tick;
        }
        
        // will be called by the generic sequencer to determine whether it should continue
        // return false to stop it.
        virtual bool seq_must_continue()
        {
            return playing;
        }
        
    };
    
    bool WinMidiManager::play(Sequence* sequence, /*out*/int* startTick, bool selectionOnly)
    {
        if (not m_bOutOpen) return false;
        if (playing) return false; // already playing
        playing = true;
        current_tick = 0;
        g_current_accurate_tick = 0;
        
        // stop any preview note currently playing
        stopNote();
        
        /*
         * Here there are a few ways to go.
         *
         * PATH 1 : Let helper functions generate midi bytes, and feed them
         *          to a native sequencer API that can read midi bytes
         */
        char* data;
        int datalength = -1;
        
        //AAR : replaced makeMidiBytes by allocAsMidiBytes
        //makeMidiBytes(sequence, true /* selection only */, &songLengthInTicks, startTick, &data, &datalength, true);
        allocAsMidiBytes(sequence, selectionOnly, &songLengthInTicks, startTick, &data, &datalength, true);
        
        // add some breath time at the end so that last note is not cut too sharply
        // 'makeMidiBytes' adds some silence at the end so no need to worry about possible problems
        songLengthInTicks += sequence->ticksPerBeat();
        
        /*
         * PATH 2 : Use the generic Aria/jdkmidi sequencer, then native functions below
         *          are used and only need to do direct output when called, no native sequencer invovled
         */
        
        SequencerThread* seqthread = new SequencerThread(selectionOnly, sequence);
        seqthread->go(startTick);
        
        m_start_tick = *startTick;
        
        current_tick = m_start_tick;
        g_current_accurate_tick = m_start_tick;
        
        return true;
    }
    
    class WinMidiManagerFactory : public PlatformMidiManagerFactory
    {
    public:
        WinMidiManagerFactory() : PlatformMidiManagerFactory(wxT("Windows"))
        {
        }
        virtual ~WinMidiManagerFactory()
        {
        }
        
        virtual PlatformMidiManager* newInstance()
        {
            return new WinMidiManager();
        }
    };
    WinMidiManagerFactory g_windows_midi_manager_factory;
}

#endif
