/*
 * Copyright (C) 1999-2003 Steve Ratcliffe
 * with major modifications from Aria Meastosa author - 2007.
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

/*
 * A quick and dirty way to make Aria run on Linux taken and modified from the pmidi app.
 * There is still a lot of work to do to get a proper implementation.
 * I still have a long way to go to understand Alsa.
 * I will continue studying the code and adapting it as i understand how it works.
 *
 * Meanwhile, if someone is knowledgable in Alsa, don't hesistate to help!! ^^
 */

#ifdef _PMIDI_ALSA


#include "AriaCore.h"
#include "Midi/Players/Alsa/AlsaNotePlayer.h"
#include "Midi/Players/Alsa/AlsaPort.h"
#include "Midi/Players/Sequencer.h"
#include "IO/IOUtils.h"

#include <alsa/asoundlib.h>

#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "GUI/MainFrame.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"

#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "wx/wx.h"
#include "wx/utils.h"
#include "wx/process.h"

namespace AriaMaestosa
{
namespace PlatformMidiManager
{

MidiContext* context;

bool must_stop=false;
Sequence* sequence;

void cleanup_after_playback()
{
    context->setPlaying(false);
    resetAllControllers();
}

int currentTick;
void seq_notify_current_tick(const int tick)
{
    currentTick = tick;
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


class SequencerThread : public wxThread
{
    jdkmidi::MIDIMultiTrack* jdkmidiseq;
    jdkmidi::MIDISequencer* jdksequencer;
    int songLengthInTicks;
    bool selectionOnly;
    int m_start_tick;

    public:

    SequencerThread(const bool selectionOnly)
    {
        jdkmidiseq = NULL;
        jdksequencer = NULL;
        SequencerThread::selectionOnly = selectionOnly;
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
        songLengthInTicks += sequence->ticksPerBeat();

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

        must_stop = true;
        cleanup_after_playback();

        return 0;
    }
};

#pragma mark -

// called when app opens
void initMidiPlayer()
{
    alsa_output_module_init();

    context = new MidiContext();

    if(! context->askOpenDevice() )
    {
        std::cerr << "failed to open ALSA device" << std::endl;
        exit(1);
    }
    alsa_output_module_setContext(context);

    context->setPlaying(false);
}

// called when app closes
void freeMidiPlayer()
{
    context->closeDevice();
    delete context;
    alsa_output_module_free();
}

bool playSequence(Sequence* sequence, /*out*/int* startTick)
{
    // std::cout << "  * playSequencer" << std::endl;
    if(context->isPlaying())
    {
        std::cout << "cannot play, it's already playing" << std::endl;
        return false; //already playing
    }
    stopNoteIfAny();
    must_stop = false;
    context->setPlaying(true);

    PlatformMidiManager::sequence = sequence;
    PlatformMidiManager::currentTick = 0;

    // std::cout << "  * playSequencer - creating new thread" << std::endl;

    SequencerThread* seqthread = new SequencerThread(false /* selection only */);
    seqthread->go(startTick);

    return true;
}

bool playSelected(Sequence* sequence, /*out*/int* startTick)
{
    if(context->isPlaying()) return false; //already playing
    stopNoteIfAny();
    must_stop = false;
    context->setPlaying(true);

    PlatformMidiManager::sequence = sequence;
    PlatformMidiManager::currentTick = 0;

    SequencerThread* seqthread = new SequencerThread(true /* selection only */);
    seqthread->go(startTick);

    return true;


/*
        if(context->isPlaying()) return false; //already playing
        stopNoteIfAny();
        must_stop = false;

        PlatformMidiManager::sequence = sequence;
        PlatformMidiManager::currentTick = 0;

        songLengthInTicks = -1;

        makeMidiBytes(sequence, true, &songLengthInTicks, startTick, &data, &datalength, true);

		stored_songLength = songLengthInTicks + sequence->ticksPerBeat();

		// start in a new thread as to not block the UI during playback;
        threads::add_events.runFunction(&add_events_func);

        context->setPlaying(true);
		return true;
		*/
}

// returns current midi tick, or -1 if over
int trackPlaybackProgression()
{
//std::cout << "trackPlaybackProgression ";
    if(context->isPlaying() and currentTick != -1)
    {
        //std::cout << "returning " << currentTick << std::endl;
        return currentTick;
    }
    else
    {
        std::cout << "SONG DONE" << std::endl;
        must_stop = true;
        Core::songHasFinishedPlaying();
        return -1;
    }
}

bool seq_must_continue()
{
    return !must_stop;
}

bool isPlaying()
{
    return context->isPlaying();
}

void stop()
{
    must_stop = true;
    //context->setPlaying(false);

    //cleanup_after_playback();
}

/*
// midi tick currently being played, -1 if none
int getCurrentTick()
{
    std::cout << "get current tick..." << std::endl;
    if(context->isPlaying())
        return currentTick;
    else
        return -1;
}
*/

bool exportMidiFile(Sequence* sequence, wxString filepath)
{
	return AriaMaestosa::exportMidiFile(sequence, filepath);
}

const wxString getAudioExtension()
{
    return wxT(".wav");
}

const wxString getAudioWildcard()
{
    return  wxString( _("WAV file")) + wxT("|*.wav");
}

/*
void trackPlayback_thread_loop()
{
    while(!PlatformMidiManager::must_stop)
    {

        PlatformMidiManager::currentTick = seqContext->getCurrentTick();

        if(PlatformMidiManager::currentTick >=
        PlatformMidiManager::songLengthInTicks or
        PlatformMidiManager::currentTick==-1) PlatformMidiManager::must_stop=true;
    }

    // clean up any events remaining on the queue and stop it
    snd_seq_drop_output(seqContext->getAlsaHandle());
    seqContext->stopTimer();
    PlatformMidiManager::allSoundOff();

    PlatformMidiManager::currentTick = -1;

    if(root != NULL)
    {
        md_free(MD_ELEMENT(root));
        root = NULL;
    }

    delete seqContext;

    // Restore signal handler
    //signal(SIGINT, SIG_DFL);// FIXME - are these removed or not?
    context->setPlaying(false);

    PlatformMidiManager::resetAllControllers();

}
*/




#pragma mark -

// --- export to audio thread --
wxString export_audio_filepath;
void* export_audio_func( void *ptr )
{

    // the file is exported to midi, and then we tell timidity to make it into wav
	wxString tempMidiFile = export_audio_filepath.BeforeLast('/') + wxT("/aria_temp_file.mid");

	AriaMaestosa::PlatformMidiManager::exportMidiFile(sequence, tempMidiFile);
	wxString cmd = wxT("timidity -Ow -o \"") + export_audio_filepath + wxT("\" \"") + tempMidiFile + wxT("\" -idt");
	std::cout << "executing " << cmd.mb_str() << std::endl;

    FILE * command_output;
    char output[128];
    int amount_read = 1;

    std::cout << "-----------------\ntimidity output\n-----------------\n";
    try
    {
        command_output = popen(cmd.mb_str(), "r");
        if(command_output == NULL) throw;

        while(amount_read > 0)
        {
            amount_read = fread(output, 1, 127, command_output);
            if(amount_read <= 0) break;
            else
            {
                output[amount_read] = '\0';
                std::cout << output << std::endl;
            }
        }
    }
    catch(...)
    {
        std::cout << "An error occured while exporting audio file." << std::endl;
        return (void*)NULL;
    }

    std::cout << "\n-----------------" << std::endl;
    pclose(command_output);

    // send hide progress window event
    MAKE_HIDE_PROGRESSBAR_EVENT(event);
    getMainFrame()->GetEventHandler()->AddPendingEvent(event);

	wxRemoveFile(tempMidiFile);

    return (void*)NULL;
}
void exportAudioFile(Sequence* sequence, wxString filepath)
{
    PlatformMidiManager::sequence = sequence;
    PlatformMidiManager::export_audio_filepath = filepath;
    threads::export_audio.runFunction( &export_audio_func );
}



}
}

#endif
