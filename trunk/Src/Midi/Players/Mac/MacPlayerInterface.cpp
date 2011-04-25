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
#include "Midi/Players/Mac/OutputBase.h"
#include "Midi/Players/Mac/AUNotePlayer.h"
#include "Midi/Players/Mac/QTKitPlayer.h"
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

#include <wx/msgdlg.h>

namespace AriaMaestosa
{
    bool g_playing;
    
    int g_current_tick;
    
    bool g_thread_should_continue = true;
    
    OutputBase* output = NULL;
    
    class AudioExport : public wxThread
    {
        wxString m_filepath;
        Sequence* m_sequence;
        char* m_data;
        int m_length;
        int m_firstMeasureValue;
        
    public:
        
        AudioExport(Sequence* sequence, wxString filepath)
        {
            m_filepath = filepath;
            m_sequence = sequence;
            
            if (Create() != wxTHREAD_NO_ERROR)
            {
                wxMessageBox("Failed to create Audio Export thread!\n");
                return;
            }
            
            MeasureData* md = m_sequence->getMeasureData();

            // when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
            m_firstMeasureValue = md->getFirstMeasure();
            md->setFirstMeasure(0);
            
            int startTick = -1, songLength = -1;
            allocAsMidiBytes(m_sequence, false, &songLength, &startTick, &m_data, &m_length, true);
            
            qtkit_setData(m_data, m_length);
            
            Run();
        }
        
        virtual ExitCode Entry()
        {
            MeasureData* md = m_sequence->getMeasureData();
            
            //exportToAudio( data, length, filepath );
            //qtkit_setData(m_data, m_length);
            bool success = qtkit_exportToAiff( m_filepath.mb_str() );
            
            if (not success)
            {
                // FIXME - give visual message. warning this is a thread.
                std::cerr << "EXPORTING FAILED" << std::endl;
            }
            
            // send hide progress window event
            MAKE_HIDE_PROGRESSBAR_EVENT(event);
            getMainFrame()->GetEventHandler()->AddPendingEvent(event);
            
            
            free(m_data);
            md->setFirstMeasure(m_firstMeasureValue);
            
            return 0;
        }
    };
    
    void cleanup_after_playback()
    {
        g_playing = false;
        output->reset_all_controllers();
    }
    
    class SequencerThread : public wxThread
    {
        jdkmidi::MIDIMultiTrack* jdkmidiseq;
        jdkmidi::MIDISequencer* jdksequencer;
        int songLengthInTicks;
        bool m_selection_only;
        int m_start_tick;
        Sequence* m_sequence;
        
    public:
        
        SequencerThread(Sequence* seq, const bool selectionOnly)
        {
            m_sequence = seq;
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
            makeJDKMidiSequence(m_sequence, *jdkmidiseq, m_selection_only, &songLengthInTicks,
                                &m_start_tick, &trackAmount, true /* for playback */);
            songLengthInTicks += m_sequence->ticksPerBeat();
            
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
            AriaSequenceTimer timer(m_sequence);
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
        
        Sequence* m_sequence;
        
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
            
            m_sequence = sequence;
            
            /*
            char* data;
            int datalength = -1;
            
            int songLengthInTicks = -1;
            allocAsMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);
            
            stored_songLength = songLengthInTicks + sequence->ticksPerBeat()*3;
            playMidiBytes(data, datalength);
            
            free(data);
             */
            
            SequencerThread* seqthread = new SequencerThread(sequence, false /* selection only */);
            seqthread->go(startTick);
            
            return true;
        }
        
        virtual bool playSelected(Sequence* sequence, /*out*/ int* startTick)
        {
            
            if (g_playing) return false; //already playing
            
            m_sequence = sequence;
            
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
            
            SequencerThread* seqthread = new SequencerThread(sequence, true /* selection only */);
            seqthread->go(startTick);
            
            return true;
        }
        
        virtual void exportAudioFile(Sequence* sequence, wxString filepath)
        {
            new AudioExport(sequence, filepath);
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
            // FIXME: bring th
            //const std::vector<CoreMidiOutput::Destination>& destinations = CoreAudioNotePlayer::getDestinations();
            wxArrayString out;
            out.Add(_("OSX Software Synthesizer"));
            /*
            for (unsigned int n=0; n<destinations.size(); n++)
            {
                out.Add(wxString(destinations[n].m_name.c_str(), wxConvUTF8));
            }
             */
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
            output->playNote( noteNum, volume, duration, channel, instrument );
        }
        
        virtual bool isPlaying()
        {
            return g_playing;
        }
        
        virtual void stopNote()
        {
            output->stopNote();
        }
        
        virtual void stop()
        {
            //if ( use_qtkit ) qtkit_stop();
            //else audioToolboxMidiPlayer->stop();
            
            g_thread_should_continue = false;
        }
        
        virtual void initMidiPlayer()
        {
            output = new AudioUnitOutput();
        }
        
        virtual void freeMidiPlayer()
        {
            delete output;
            output = NULL;
        }

        void seq_note_on(const int note, const int volume, const int channel)
        {
            output->note_on(note, volume, channel);
        }
        
        void seq_note_off(const int note, const int channel)
        {
            output->note_off(note, channel);
        }
        
        void seq_prog_change(const int instrument, const int channel)
        {
            output->prog_change(instrument, channel);
        }
        
        void seq_controlchange(const int controller, const int value, const int channel)
        {
            output->controlchange(controller, value, channel);
        }
        
        void seq_pitch_bend(const int value, const int channel)
        {
            output->pitch_bend(value + 8192, channel);
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
