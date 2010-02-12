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

 For actual playback of midi data, both QTKit and AudioToolbox implementations exist. The rationale behind this is that:

 - QTKit offers more precise info for getting current tick, however it returns time and not ticks. It is easy to calculate current
   midi tick from time with tempo, but not when there are tempo bends. Furthermore, it allows to export to audio formats like AIFF.
 - AudioToolkit returns current playback location in midi beats. This is much less precise than tempo. However, this does follow tempo bends.

 So, when there are tempo bends, use AudioToolkit. Otherwise, use QTKit.

 */

#ifdef _MAC_QUICKTIME_COREAUDIO
#include "Config.h"

#include <iostream>

#include "wx/wx.h"

#include "AriaCore.h"

#include "Midi/MeasureData.h"

#include "GUI/MainFrame.h"

#include "Dialogs/WaitWindow.h"

#include "Midi/Sequence.h"
#include "Midi/Players/Mac/QTKitPlayer.h"
#include "Midi/Players/Mac/CoreAudioNotePlayer.h"
#include "Midi/Players/Mac/AudioToolboxPlayer.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"

#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"


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

namespace AriaMaestosa {

namespace PlatformMidiManager {

    void playMidiBytes(char* bytes, int length);

    AudioToolboxMidiPlayer* audioToolboxMidiPlayer;

    Sequence* sequence;
    bool playing=false;
    bool use_qtkit = true;

    int stored_songLength = 0;

    const wxString getAudioExtension()
    {
        return wxT(".aiff");
    }
    const wxString getAudioWildcard()
    {
        return  wxString( _("AIFF file")) + wxT("|*.aiff");
    }

    bool playSequence(Sequence* sequence, /*out*/ int* startTick)
    {
        if (playing) return false; //already playing

        PlatformMidiManager::sequence = sequence;

        char* data;
        int datalength = -1;

        int songLengthInTicks = -1;
        allocAsMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);

        stored_songLength = songLengthInTicks + sequence->ticksPerBeat()*3;
        playMidiBytes(data, datalength);

        free(data);

        return true;
    }

    bool playSelected(Sequence* sequence, /*out*/ int* startTick)
    {

        if (playing) return false; //already playing

        PlatformMidiManager::sequence = sequence;

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

    bool exportMidiFile(Sequence* sequence, wxString filepath)
    {
        AriaMaestosa::exportMidiFile(sequence, filepath);
        return true;
    }

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

    wxString export_audio_filepath;
    void* add_events_func( void *ptr )
    {
        // when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
        const int firstMeasureValue=getMeasureData()->getFirstMeasure();
        getMeasureData()->setFirstMeasure(0);

        char* data;
        int length = -1;

        int startTick = -1, songLength = -1;
        allocAsMidiBytes(sequence, false, &songLength, &startTick, &data, &length, true);

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

    void exportAudioFile(Sequence* sequence, wxString filepath)
    {
        PlatformMidiManager::sequence = sequence;
        export_audio_filepath = filepath;
        threads::export_audio.runFunction( &add_events_func );
    }

    int getCurrentTick()
    {
        if ( use_qtkit )
        {
            const float time = qtkit_getCurrentTime();

            if (time == -1) return -1; // not playing

            return (int)(
                         time * sequence->getTempo()/60.0 * (float)sequence->ticksPerBeat()
                         );
        }
        else
        {
            return audioToolboxMidiPlayer->getPosition() * sequence->ticksPerBeat();
        }
    }
    int trackPlaybackProgression()
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

    void playMidiBytes(char* bytes, int length)
    {
        CoreAudioNotePlayer::stopNote();

        // if length==8, this is just the empty song to load QT. (when the app opens, Quicktime is triggered with an empty song to make it load)
        if ( length==8 or sequence->tempoEvents.size()==0 ) use_qtkit=true;
        else use_qtkit=false;

        if ( use_qtkit )
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

    void playNote(int noteNum, int volume, int duration, int channel, int instrument)
    {
        if (playing) return;
        CoreAudioNotePlayer::playNote( noteNum, volume, duration, channel, instrument );
    }

    bool isPlaying()
    {
        return playing;
    }

    void stopNote()
    {
        CoreAudioNotePlayer::stopNote();
    }

    void stop()
    {
        if ( use_qtkit ) qtkit_stop();
        else audioToolboxMidiPlayer->stop();

        playing = false;
    }

    void initMidiPlayer()
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

    void freeMidiPlayer()
    {
        qtkit_free();
        CoreAudioNotePlayer::free();
        delete audioToolboxMidiPlayer;
        audioToolboxMidiPlayer=NULL;
    }


    // ---------- unused since we use native sequencer/timer on OS X ---------
    void seq_note_on(const int note, const int volume, const int channel){}
    void seq_note_off(const int note, const int channel){}
    void seq_prog_change(const int instrument, const int channel){}
    void seq_controlchange(const int controller, const int value, const int channel){}
    void seq_pitch_bend(const int value, const int channel){}

    // called repeatedly by the generic sequencer to tell
    // the midi player what is the current progression
    // the sequencer will call this with -1 as argument to indicate it exits.
    void seq_notify_current_tick(const int tick){}

    bool seq_must_continue(){return false;}

}
}

#endif
