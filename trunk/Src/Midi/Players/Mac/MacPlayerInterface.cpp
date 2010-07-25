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

#include "wx/string.h"

#include "Utils.h"
#include "AriaCore.h"
#include "Dialogs/WaitWindow.h"
#include "GUI/MainFrame.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/Mac/QTKitPlayer.h"
#include "Midi/Players/Mac/CoreAudioNotePlayer.h"
#include "Midi/Players/Mac/AudioToolboxPlayer.h"
#include "Midi/Players/PlatformMidiManager.h"
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
    
    // Silly way to pass parameters to the thread
    wxString export_audio_filepath;
    Sequence* g_sequence;

    void* add_events_func( void* ptr )
    {
        // when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
        const int firstMeasureValue=getMeasureData()->getFirstMeasure();
        getMeasureData()->setFirstMeasure(0);
        
        char* data;
        int length = -1;
        
        int startTick = -1, songLength = -1;
        allocAsMidiBytes(g_sequence, false, &songLength, &startTick, &data, &length, true);
        
        //exportToAudio( data, length, filepath );
        qtkit_setData(data, length);
        bool success = qtkit_exportToAiff( export_audio_filepath.mb_str() );
        
        if (!success)
        {
            // FIXME - give visual message. warning this is a thread.
            std::cerr << "EXPORTING FAILED" << std::endl;
        }
        
        // send hide progress window event
        MAKE_HIDE_PROGRESSBAR_EVENT(event);
        getMainFrame()->GetEventHandler()->AddPendingEvent(event);
        
        free(data);
        getMeasureData()->setFirstMeasure(firstMeasureValue);
        
        return (void*)NULL;
    }

    void playMidiBytes(char* bytes, int length);

    class MacMidiManager : public PlatformMidiManager
    {        
        AudioToolboxMidiPlayer* audioToolboxMidiPlayer;
        
        bool playing;
        bool use_qtkit;
        
        int stored_songLength;
        
    public:
        
        MacMidiManager()
        {
            playing   = false;
            use_qtkit = true;
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
            if (playing) return false; //already playing
            
            g_sequence = sequence;
            
            char* data;
            int datalength = -1;
            
            int songLengthInTicks = -1;
            allocAsMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);
            
            stored_songLength = songLengthInTicks + sequence->ticksPerBeat()*3;
            playMidiBytes(data, datalength);
            
            free(data);
            
            return true;
        }
        
        virtual bool playSelected(Sequence* sequence, /*out*/ int* startTick)
        {
            
            if (playing) return false; //already playing
            
            g_sequence = sequence;
            
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
            
            return true;
        }
        
        virtual bool exportMidiFile(Sequence* sequence, wxString filepath)
        {
            AriaMaestosa::exportMidiFile(sequence, filepath);
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
        }
        
        virtual int trackPlaybackProgression()
        {
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
            
        }
        
        virtual void playMidiBytes(char* bytes, int length)
        {
            CoreAudioNotePlayer::stopNote();
            
            // if length==8, this is just the empty song to load QT. (when the app opens, Quicktime is triggered
            // with an empty song to make it load) - FIXME: this check is ugly
            if (length == 8 or g_sequence->tempoEvents.size() == 0) use_qtkit = true;
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
        
        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument)
        {
            if (playing) return;
            CoreAudioNotePlayer::playNote( noteNum, volume, duration, channel, instrument );
        }
        
        virtual bool isPlaying()
        {
            return playing;
        }
        
        virtual void stopNote()
        {
            CoreAudioNotePlayer::stopNote();
        }
        
        virtual void stop()
        {
            if ( use_qtkit ) qtkit_stop();
            else audioToolboxMidiPlayer->stop();
            
            playing = false;
        }
        
        virtual void initMidiPlayer()
        {
            qtkit_init();
            CoreAudioNotePlayer::init();
            audioToolboxMidiPlayer=new AudioToolboxMidiPlayer();
            
            // trigger quicktime with empty song, just to make it load
            // FIXME - doesn't work anymore with newer QT versions
            char bytes[8];
            bytes[0] = 'M';
            bytes[1] = 'T';
            bytes[2] = 'h';
            bytes[3] = 'd';
            bytes[4] = 0;
            bytes[5] = 0;
            bytes[6] = 0;
            bytes[7] = 0;
            
            playMidiBytes(bytes, 8);
            stop();
        }
        
        virtual void freeMidiPlayer()
        {
            qtkit_free();
            CoreAudioNotePlayer::free();
            delete audioToolboxMidiPlayer;
            audioToolboxMidiPlayer=NULL;
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
