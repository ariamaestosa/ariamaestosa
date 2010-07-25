/*
 *  Copyright (C) 1999-2003 -------
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
#if 0

#include "AriaCore.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"

#include "wx/wx.h"
#include "wx/utils.h"
#include "wx/process.h"

namespace AriaMaestosa
{

    int songLengthInTicks = -1;
    bool playing = false;
    int current_tick = -1;


    void cleanup_after_playback()
    {
        // all sound off, reset all controllers, reset pitch bend, etc.
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

    
    class ExamplePlayer : public PlatformMidiManager
    {
        
    public:
        
        virtual bool playSequence(Sequence* sequence, /*out*/int* startTick)
        {
            /*
             * see 'playSelected' below, it's essentially the same, just
             * replace 'true' with 'false' where it says 'selection only'
             */
        }
        
        virtual bool playSelected(Sequence* sequence, /*out*/int* startTick)
        {
            if(playing) return false; // already playing
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
            makeMidiBytes(sequence, true /* selection only */, &songLengthInTicks, startTick, &data, &datalength, true);

            // add some breath time at the end so that last note is not cut too sharply
            // 'makeMidiBytes' adds some silence at the end so no need to worry about possible problems
            songLengthInTicks += sequence->ticksPerBeat();

            /*
             * PATH 2 : Use the generic Aria/jdkmidi sequencer, then native functions below
             *          are used and only need to do direct output when called, no native sequencer invovled
             */

            SequencerThread* seqthread = new SequencerThread(true /* selection only */, sequence);
            seqthread->go(startTick);

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
        virtual void exportAudioFile(Sequence* sequence, wxString filepath)
        {
        }
        
        virtual const wxString getAudioExtension()
        {
            return wxT(".wav");
        }
        
        virtual const wxString getAudioWildcard()
        {
            return  wxString( _("WAV file")) + wxT("|*.wav");
        }


        virtual bool exportMidiFile(Sequence* sequence, wxString filepath)
        {
            // okay to use generic function for this
            return AriaMaestosa::exportMidiFile(sequence, filepath);
        }

        // get current tick, either from native API or from a variable you keep around and update from the playback thread
        virtual int trackPlaybackProgression()
        {
            current_tick = getTickFromNativeAPI();
            if(current_tick > songLengthInTicks || current_tick == -1)
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

        // called when app opens
        virtual void initMidiPlayer()
        {
        }

        // called when app closes
        virtual void freeMidiPlayer()
        {
        }

        // play/stop a single preview note (no sequencing is to be done, only direct output)
        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument)
        {
        }
        
        virtual void stopNote()
        {
        }

        // ----------------------------------------------------------------------
        // functions below will only be called if you use the generic AriaSequenceTimer
        // if you use a native sequencer instead you can leave them empty

        // if you're going to use the generic sequencer/timer, implement those to send events to native API
        virtual void seq_note_on(const int note, const int volume, const int channel)
        {
        }

        virtual void seq_note_off(const int note, const int channel)
        {
        }

        virtual void seq_prog_change(const int instrument, const int channel)
        {
        }

        virtual void seq_controlchange(const int controller, const int value, const int channel)
        {
        }

        virtual void seq_pitch_bend(const int value, const int channel)
        {
        }

        // called repeatedly by the generic sequencer to tell
        // the midi player what is the current progression
        // the sequencer will call this with -1 as argument to indicate it exits.
        virtual void seq_notify_current_tick(const int tick_arg)
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
        virtual bool seq_must_continue()
        {
            return playing;
        }

    };
    
    class ExampleMidiManagerFactory : public PlatformMidiManagerFactory
    {
    public:
        ExampleMidiManagerFactory() : PlatformMidiManagerFactory(wxT("ExampleMidiDriver"))
        {
        }
        virtual ~ExampleMidiManagerFactory()
        {
        }
        
        virtual PlatformMidiManager* newInstance()
        {
            return new ExamplePlayer();
        }
    };
    ExampleMidiManagerFactory g_example_midi_manager_factory;
}

#endif
