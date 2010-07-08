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

// everything here must be added/changed/checked, that's just a placeholder to help anyone willing to help
#ifdef __WIN32__

#include "AriaCore.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"
#include "Midi/Players/Sequencer.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/sequencer.h"

#include "wx/wx.h"
#include "wx/utils.h"
#include "wx/process.h"

// macro to pack a MIDI short message
#define MAKEMIDISHORTMSG(cStatus, cChannel, cData1, cData2)            \
    cStatus | cChannel | (((UINT)cData1) << 8) | (((DWORD)cData2) << 16)


#define ROUND(x) (int) ((x)+0.5)


// MIDI Status Bytes for Channel Voice Messages
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_POLY_PRESSURE      0xA0
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_PROGRAM_CHANGE     0xC0
#define MIDI_CHANNEL_PRESSURE   0xD0
#define MIDI_AFTERTOUCH         0xD0  // synonym for channel pressure
#define MIDI_PITCH_WHEEL        0xE0


namespace AriaMaestosa
{
    namespace PlatformMidiManager
    {

        class StopNoteTimer : public wxTimer
        {
            public:

                StopNoteTimer() : wxTimer(){ }

                void Notify()
                {
                    PlatformMidiManager::stopNote();
                    wxTimer::Stop();
                }

                void start(int duration)
                {
                    wxTimer::Start(duration);
                }
        };


        int songLengthInTicks = -1;
        bool playing = false;
        int current_tick = -1;
		HMIDIOUT m_hOutMidiDevice = 0;
		bool m_bOutOpen = false;
		int lastPlayedNote = -1, lastChannel;
        StopNoteTimer* stopNoteTimer = NULL;

		bool play(Sequence* sequence, /*out*/int* startTick, bool selectionOnly);



		// all sound off, reset all controllers, reset pitch bend, etc.
        void cleanup_after_playback()
        {
        	midiOutReset(m_hOutMidiDevice);
        }

        class SequencerThread : public wxThread
        {
            jdkmidi::MIDIMultiTrack* jdkmidiseq;
            jdkmidi::MIDISequencer* jdksequencer;
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
                std::cout << "cleaning up sequencer" << std::endl;
                if(jdksequencer != NULL) delete jdksequencer;
                if(jdkmidiseq != NULL) delete jdkmidiseq;
            }

            void prepareSequencer()
            {
                jdkmidiseq = new jdkmidi::MIDIMultiTrack();
                songLengthInTicks = -1;
                int trackAmount = -1;
                m_start_tick = 0;
                makeJDKMidiSequence(sequence, *jdkmidiseq, selectionOnly, &songLengthInTicks,
                                    &m_start_tick, &trackAmount, true /* for playback */);

                //std::cout << "trackAmount=" << trackAmount << " start_tick=" << m_start_tick<<
                //        " songLengthInTicks=" << songLengthInTicks << std::endl;

                jdksequencer = new jdkmidi::MIDISequencer(jdkmidiseq);
            }

            void go(int* startTick /* out */)
            {
                if(Create() != wxTHREAD_NO_ERROR)
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


        bool playSequence(Sequence* sequence, /*out*/int* startTick)
        {
        	return play(sequence, startTick, false);
            /*
             * see 'playSelected' below, it's essentially the same, just
             * replace 'true' with 'false' where it says 'selection only'
             */
        }
        bool playSelected(Sequence* sequence, /*out*/int* startTick)
        {
        	return play(sequence, startTick, true);
        }
        bool isPlaying()
        {
            return playing;
        }

        void stop()
        {
            playing = false;
        }

        // export to audio file, e.g. .wav
        void exportAudioFile(Sequence* /*sequence*/, wxString /*filepath*/)
        {
        }
        const wxString getAudioExtension()
        {
            return wxEmptyString;
            //return wxT(".wav");
        }
        const wxString getAudioWildcard()
        {
            return wxEmptyString;
            //return  wxString( _("WAV file")) + wxT("|*.wav");
        }


        bool exportMidiFile(Sequence* sequence, wxString filepath)
        {
            // okay to use generic function for this
            return AriaMaestosa::exportMidiFile(sequence, filepath);
        }

        // get current tick, either from native API or from a variable (current_tick) you keep around and update from the playback thread
        // current_tick is updated by seq_notify_current_tick
        int trackPlaybackProgression()
        {
            if (current_tick > songLengthInTicks || current_tick == -1)
            {
                // song is over
                playing = false;
                // this function is probably called too many times in this example...
                cleanup_after_playback();

                // notify app that song is over
                Core::songHasFinishedPlaying();
                return -1;
            }

            return current_tick;
        }


        // Not used currently, but could be added to enumerate the MIDI devices (i. e. MIDI Yoke virtual devices)
		void enumerateDevices()
		{
			int i;
			DWORD wRtn;
			int midi_num_outputs;
			LPMIDIOUTCAPS midi_out_caps;

			midi_num_outputs = midiOutGetNumDevs();
			midi_out_caps = (LPMIDIOUTCAPS)malloc(sizeof(MIDIOUTCAPS)*midi_num_outputs);

			if (midi_out_caps != NULL)
			{
				std::cout << "Manufacturer ID | Product ID | Driver Version | Name";

				for (i = 0; i < midi_num_outputs; i++)
				{
					wRtn = midiOutGetDevCaps(i, (LPMIDIOUTCAPS) & midi_out_caps[i],
											 sizeof(MIDIOUTCAPS));
					if (wRtn == MMSYSERR_NOERROR)
                    {
						// see http://msdn.microsoft.com/en-us/library/dd798467%28VS.85%29.aspx
						std::cout << midi_out_caps[i].wMid << " " << midi_out_caps[i].wPid << " "
										<< midi_out_caps[i].vDriverVersion << " " << midi_out_caps[i].szPname;
					}
				}
				free (midi_out_caps);
			}


		}


        // called when app opens
        void initMidiPlayer()
        {

			if( !m_bOutOpen )
			{
				//enumerateDevices();

				// To access Midi Yoke Output, simply put its number instead of MIDI_MAPPER
			  int e = ::midiOutOpen(
				&m_hOutMidiDevice,
				MIDI_MAPPER,
				0,
				0,
				CALLBACK_NULL
				);

			  if( e!=0 )
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
				return;
			  }
			  m_bOutOpen=true;
			}

			stopNoteTimer = new StopNoteTimer();

			return;
        }

        // called when app closes
        void freeMidiPlayer()
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
        void playNote(int noteNum, int volume, int duration, int channel, int instrument)
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

        void stopNote()
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
        void seq_note_on(const int note, const int volume, const int channel)
        {
			DWORD dwMsg;

        	dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, channel, note, volume);
			::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }


        void seq_note_off(const int note, const int channel)
        {
        	 DWORD dwMsg;

        	// 0 velocity turns note off
        	dwMsg = MAKEMIDISHORTMSG(MIDI_NOTE_ON, channel, note, 0);
			::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }

        void seq_prog_change(const int instrument, const int channel)
        {
			DWORD dwMsg;

        	dwMsg = MAKEMIDISHORTMSG(MIDI_PROGRAM_CHANGE, channel, instrument, 0);
			::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }

        void seq_controlchange(const int controller, const int value, const int channel)
        {
        	DWORD dwMsg;

        	dwMsg = MAKEMIDISHORTMSG(MIDI_CONTROL_CHANGE, channel, controller, value);
			::midiOutShortMsg(m_hOutMidiDevice, dwMsg);
        }

        void seq_pitch_bend(const int value, const int channel)
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
        void seq_notify_current_tick(const int tick_arg)
        {
            current_tick = tick_arg;

            if(tick_arg == -1)
            {
                // song done
                playing = false;
                cleanup_after_playback();
            }
        }

        // will be called by the generic sequencer to determine whether it should continue
        // return false to stop it.
        bool seq_must_continue()
        {
            return playing;
        }

        bool play(Sequence* sequence, /*out*/int* startTick, bool selectionOnly)
        {
            if (not m_bOutOpen) return false;
            if (playing) return false; // already playing
            playing = true;
            current_tick = 0;

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

            return true;
        }

    }
}

#endif
