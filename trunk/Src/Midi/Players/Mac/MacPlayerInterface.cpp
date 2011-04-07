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

/*
 
 The Mac implementation of Platform midi player uses QTKit, AudioToolbox and CoreAudio.
 
 CoreAudio is used to play single notes (the previews while editing)
 
 For actual playback of midi data, both QTKit and AudioToolbox implementations exist. The rationale behind this
 is that:
 
 - QTKit offers more precise info for getting current tick, however it returns time and not ticks. It is easy to
   calculate current midi tick from time with tempo, but not when there are tempo bends. Furthermore, it allows to
   export to audio formats like AIFF.
 - AudioToolkit returns current playback location in midi beats. This is much less precise than tempo. However,
   this does follow tempo bends.
 
 So, when there are tempo bends, use AudioToolkit. Otherwise, use QTKit.
 
 */

#ifdef _MAC_QUICKTIME_COREAUDIO


#include <iostream>

#include <wx/string.h>

#include "Utils.h"
#include "AriaCore.h"
#include "Dialogs/WaitWindow.h"
#include "GUI/MainFrame.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/Mac/QTKitPlayer.h"
#include "Midi/Players/Mac/AUNotePlayer.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Players/Sequencer.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"


#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/sysex.h"
#include "jdkmidi/sequencer.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/process.h"

namespace AriaMaestosa
{
    
    class MyPThread
    {
        int id;
        pthread_t thread;
    public:
        MyPThread(){}
        
        void runFunction(void* (*func)(void*) )
        {
            id = pthread_create( &thread, NULL, func, (void*)NULL);
        }
    };
    
    
    namespace threads
    {
        MyPThread export_audio;
    }
    
    bool g_playing;
    
    int g_current_tick;
    
    bool g_thread_should_continue = true;
    
    
    // Silly way to pass parameters to the thread
    wxString export_audio_filepath;
    Sequence* g_sequence = NULL;
    
    void* add_events_func( void* ptr )
    {
        ASSERT(g_sequence != NULL);
        MeasureData* md = g_sequence->getMeasureData();
        
        // when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
        const int firstMeasureValue = md->getFirstMeasure();
        md->setFirstMeasure(0);
        
        char* data;
        int length = -1;
        
        int startTick = -1, songLength = -1;
        allocAsMidiBytes(g_sequence, false, &songLength, &startTick, &data, &length, true);
        
        //exportToAudio( data, length, filepath );
        qtkit_setData(data, length);
        bool success = qtkit_exportToAiff( export_audio_filepath.mb_str() );
        
        if (not success)
        {
            // FIXME - give visual message. warning this is a thread.
            std::cerr << "EXPORTING FAILED" << std::endl;
        }
        
        // send hide progress window event
        MAKE_HIDE_PROGRESSBAR_EVENT(event);
        getMainFrame()->GetEventHandler()->AddPendingEvent(event);
        
        free(data);
        md->setFirstMeasure(firstMeasureValue);
        
        return (void*)NULL;
    }

    void cleanup_after_playback()
    {
        g_playing = false;
        CoreAudioNotePlayer::au_reset_all_controllers();
    }
    
    class SequencerThread : public wxThread
    {
        jdkmidi::MIDIMultiTrack* jdkmidiseq;
        jdkmidi::MIDISequencer* jdksequencer;
        int songLengthInTicks;
        bool m_selection_only;
        int m_start_tick;
        
    public:
        
        SequencerThread(const bool selectionOnly)
        {
            jdkmidiseq = NULL;
            jdksequencer = NULL;
            m_selection_only = selectionOnly;
        }
        ~SequencerThread()
        {
            if (jdksequencer != NULL) delete jdksequencer;
            if (jdkmidiseq != NULL) delete jdkmidiseq;
        }
        
        void prepareSequencer()
        {
            jdkmidiseq = new jdkmidi::MIDIMultiTrack();
            songLengthInTicks = -1;
            int trackAmount = -1;
            m_start_tick = 0;
            makeJDKMidiSequence(g_sequence, *jdkmidiseq, m_selection_only, &songLengthInTicks,
                                &m_start_tick, &trackAmount, true /* for playback */);
            songLengthInTicks += g_sequence->ticksPerBeat();
            
            //std::cout << "trackAmount=" << trackAmount << " start_tick=" << m_start_tick<<
            //        " songLengthInTicks=" << songLengthInTicks << std::endl;
            
            jdksequencer = new jdkmidi::MIDISequencer(jdkmidiseq);
            
            g_current_tick = m_start_tick;
        }
        
        void go(int* startTick /* out */)
        {
            g_playing = true;
            g_thread_should_continue = true;

            if (Create() != wxTHREAD_NO_ERROR)
            {
                std::cerr << "[OSX Player] ERROR: failed to create thread" << std::endl;
                return;
            }
            SetPriority(85 /* 0 = min, 100 = max */);
            
            prepareSequencer();
            *startTick = m_start_tick;
            
            Run();
        }
        
        ExitCode Entry()
        {
            AriaSequenceTimer timer(g_sequence);
            timer.run(jdksequencer, songLengthInTicks);
            
            //must_stop = true;
            cleanup_after_playback();
            
            return 0;
        }
    };
    
    void playMidiBytes(char* bytes, int length);

    class MacMidiManager : public PlatformMidiManager
    {        
        //AudioToolboxMidiPlayer* audioToolboxMidiPlayer;
        
        //bool use_qtkit;
        
        int stored_songLength;
        
    public:
        
        MacMidiManager()
        {
            g_playing   = false;
            //use_qtkit = true;
            stored_songLength = 0;
        }
        
        virtual ~MacMidiManager() { }
        
        virtual const wxString getAudioExtension()
        {
            return wxT(".aiff");
        }
        
        virtual const wxString getAudioWildcard()
        {
            return  wxString( _("AIFF file")) + wxT("|*.aiff");
        }
        
        virtual bool playSequence(Sequence* sequence, /*out*/ int* startTick)
        {
            if (g_playing) return false; //already playing
            
            g_sequence = sequence;
            
            /*
            char* data;
            int datalength = -1;
            
            int songLengthInTicks = -1;
            allocAsMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);
            
            stored_songLength = songLengthInTicks + sequence->ticksPerBeat()*3;
            playMidiBytes(data, datalength);
            
            free(data);
             */
            
            SequencerThread* seqthread = new SequencerThread(false /* selection only */);
            seqthread->go(startTick);
            
            return true;
        }
        
        virtual bool playSelected(Sequence* sequence, /*out*/ int* startTick)
        {
            
            if (g_playing) return false; //already playing
            
            g_sequence = sequence;
            
            /*
            char* data;
            int datalength = -1;
            int songLengthInTicks = -1;
            
            allocAsMidiBytes(sequence, true, &songLengthInTicks, startTick, &data, &datalength, true);
            
            if (songLengthInTicks < 1)
            {
                std::cout << "song is empty" << std::endl;
                free(data);
                return false;
            }
            
            stored_songLength = songLengthInTicks + sequence->ticksPerBeat();
            playMidiBytes(data, datalength);
            
            free(data);
            */
            
            SequencerThread* seqthread = new SequencerThread(true /* selection only */);
            seqthread->go(startTick);
            
            return true;
        }
        
        virtual void exportAudioFile(Sequence* sequence, wxString filepath)
        {
            g_sequence = sequence;
            export_audio_filepath = filepath;
            threads::export_audio.runFunction( &add_events_func );
        }
        
        virtual int getCurrentTick()
        {
            return g_current_tick;
            /*
            if (use_qtkit)
            {
                const float time = qtkit_getCurrentTime();
                
                if (time == -1) return -1; // not playing
                
                return (int)(time * g_sequence->getTempo()/60.0 * (float)g_sequence->ticksPerBeat());
            }
            else
            {
                return audioToolboxMidiPlayer->getPosition() * g_sequence->ticksPerBeat();
            }
             */
        }
        
        virtual wxArrayString getOutputChoices()
        {
            const std::vector<CoreAudioNotePlayer::Destination>& destinations = CoreAudioNotePlayer::getDestinations();
            wxArrayString out;
            out.Add(_("OSX Software Synthesizer"));
            for (unsigned int n=0; n<destinations.size(); n++)
            {
                out.Add(wxString(destinations[n].m_name.c_str(), wxConvUTF8));
            }
            return out;
        }
        
        virtual int trackPlaybackProgression()
        {
            return g_current_tick;
            /*
            int currentTick = getCurrentTick();
            
            // song ends
            if (currentTick >= stored_songLength-1 or currentTick == -1)
            {
                Core::songHasFinishedPlaying();
                currentTick=-1;
                stop();
                return -1;
            }
            
            
            return currentTick;
            */
        }
        
        /*
        virtual void playMidiBytes(char* bytes, int length)
        {
            CoreAudioNotePlayer::stopNote();
            
            // if length==8, this is just the empty song to load QT. (when the app opens, Quicktime is triggered
            // with an empty song to make it load) - FIXME: this check is ugly
            if (length == 8 or g_sequence->getTempoEventAmount() == 0) use_qtkit = true;
            else use_qtkit=false;
            
            if (use_qtkit)
            {
                qtkit_setData(bytes, length);
                qtkit_play();
            }
            else
            {
                audioToolboxMidiPlayer->loadSequence(bytes, length);
                audioToolboxMidiPlayer->play();
            }
            
            playing = true;
        }
        */
        
        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument)
        {
            if (g_playing) return;
            CoreAudioNotePlayer::playNote( noteNum, volume, duration, channel, instrument );
        }
        
        virtual bool isPlaying()
        {
            return g_playing;
        }
        
        virtual void stopNote()
        {
            CoreAudioNotePlayer::stopNote();
        }
        
        virtual void stop()
        {
            //if ( use_qtkit ) qtkit_stop();
            //else audioToolboxMidiPlayer->stop();
            
            g_thread_should_continue = false;
        }
        
        virtual void initMidiPlayer()
        {
            //qtkit_init();
            CoreAudioNotePlayer::init();
            //audioToolboxMidiPlayer = new AudioToolboxMidiPlayer();
        }
        
        virtual void freeMidiPlayer()
        {
            //qtkit_free();
            CoreAudioNotePlayer::free();
            //delete audioToolboxMidiPlayer;
            //audioToolboxMidiPlayer = NULL;
        }

        void seq_note_on(const int note, const int volume, const int channel)
        {
            CoreAudioNotePlayer::au_seq_note_on(note, volume, channel);
        }
        
        void seq_note_off(const int note, const int channel)
        {
            CoreAudioNotePlayer::au_seq_note_off(note, channel);
        }
        
        void seq_prog_change(const int instrument, const int channel)
        {
            CoreAudioNotePlayer::au_seq_prog_change(instrument, channel);
        }
        
        void seq_controlchange(const int controller, const int value, const int channel)
        {
            CoreAudioNotePlayer::au_seq_controlchange(controller, value, channel);
        }
        
        void seq_pitch_bend(const int value, const int channel)
        {
            CoreAudioNotePlayer::au_seq_pitch_bend(value, channel);
        }
        
        /**
         * @brief called repeatedly by the generic sequencer to tell the midi player what is the current
         *        progression. the sequencer will call this with -1 as argument to indicate it exits.
         */
        virtual void seq_notify_current_tick(const int tick)
        {
            g_current_tick = tick;
            
            if (tick == -1) g_playing = false;
        }
        
        /**
         * @brief will be called by the generic sequencer to determine whether it should continue
         * @return false to stop it, true to continue
         */
        bool seq_must_continue()
        {
            return g_thread_should_continue;
        }
        
    }; // end class
    
    class MacMidiManagerFactory : public PlatformMidiManagerFactory
    {
    public:
        MacMidiManagerFactory() : PlatformMidiManagerFactory(wxT("QuickTime/AudioToolkit"))
        {
        }
        virtual ~MacMidiManagerFactory()
        {
        }
        
        virtual PlatformMidiManager* newInstance()
        {
            return new MacMidiManager();
        }
    };
    MacMidiManagerFactory g_mac_midi_manager_factory;
    
} // end namespace

#endif
