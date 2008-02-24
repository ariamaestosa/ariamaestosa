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
#include "Midi/Players/Alsa/pmidi/elements.h"
#include "Midi/Players/Alsa/AlsaNotePlayer.h"
#include "Midi/Players/Alsa/AlsaPort.h"
#include "IO/IOUtils.h"

#include <alsa/asoundlib.h>

#include "Midi/Players/Alsa/pmidi/seqpriv.h"
#include "Midi/Players/Alsa/pmidi/seqlib.h"
#include "Midi/Players/Alsa/pmidi/md.h"
#include "Midi/Players/Alsa/pmidi/midi.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "GUI/MainFrame.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"

//#include <signal.h>
#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "wx/wx.h"
#include "wx/utils.h"
#include "wx/process.h"

namespace AriaMaestosa
{
MidiContext* context;

void prepareAlsaThenPlay(char* bytes, int length);
void trackPlayback_thread_loop();

struct rootElement *root;
//SeqContext* seqContext;

namespace PlatformMidiManager {

int currentTick;
int stored_songLength;
bool must_stop=false;
Sequence* sequence;

int songLengthInTicks;

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
MyPThread add_events;
MyPThread track_playback;
MyPThread export_audio;
}

// -- add events then play thread ---
char* data;
int datalength = -1;

void* add_events_func( void *ptr )
{
     AriaMaestosa::prepareAlsaThenPlay(data, datalength);
     return (void*)NULL;
}

// --- track playback thread ---
void* track_playback_func( void *ptr )
{
     AriaMaestosa::trackPlayback_thread_loop();
     return (void*)NULL;
}

#pragma mark -

// --- export to audio thread --
wxString export_audio_filepath;
void* export_audio_func( void *ptr )
{

    // the file is exported to midi, and then we tell timidity to make it into wav
	wxString tempMidiFile = export_audio_filepath.BeforeLast('/') + wxT("/aria_temp_file.mid");

	AriaMaestosa::PlatformMidiManager::exportMidiFile(sequence, tempMidiFile);
	wxString cmd = wxT("timidity -Ow -o \"") + export_audio_filepath + wxT("\" \"") + tempMidiFile + wxT("\" -idt");
	std::cout << "executing " << toCString( cmd ) << std::endl;

    FILE * command_output;
    char output[128];
    int amount_read = 1;

    std::cout << "-----------------\ntimidity output\n-----------------\n";
    try
    {
        command_output = popen(toCString(cmd), "r");
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

#pragma mark -

// called when app opens
void initMidiPlayer()
{
    AlsaNotePlayer::init();

    context = new MidiContext();

    if(! context->askOpenDevice() )
    {
        exit(1);
    }
    AlsaNotePlayer::setContext(context);

    context->setPlaying(false);
    stored_songLength = -1;
}

// called when app closes
void freeMidiPlayer()
{
    context->closeDevice();
    delete context;
    AlsaNotePlayer::free();
}

bool playSequence(Sequence* sequence, /*out*/int* startTick)
{
  		if(context->isPlaying()) return false; //already playing
  		AlsaNotePlayer::stopNoteIfAny();
        must_stop = false;

		PlatformMidiManager::sequence = sequence;
        PlatformMidiManager::currentTick = 0;

		songLengthInTicks = -1;
		makeMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);

		stored_songLength = songLengthInTicks + sequence->ticksPerBeat();

		// start in a new thread as to not block the UI during playback
        threads::add_events.runFunction(&add_events_func);

        context->setPlaying(true);
		return true;
}

bool playSelected(Sequence* sequence, /*out*/int* startTick)
{
        if(context->isPlaying()) return false; //already playing
        AlsaNotePlayer::stopNoteIfAny();
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
}

bool exportMidiFile(Sequence* sequence, wxString filepath)
{
	return AriaMaestosa::exportMidiFile(sequence, filepath);
}

// returns current midi tick, or -1 if over
int trackPlaybackProgression()
{

    if(context->isPlaying() and
        !(currentTick >= stored_songLength-1 or currentTick == -1) )
    {
        return currentTick;
    }
    else
    {
        Core::songHasFinishedPlaying();
        return -1;
    }
}

bool isPlaying()
{
    return context->isPlaying();
}

void stop()
{
    must_stop = true;
    context->setPlaying(false);
}

// midi tick currently being played, -1 if none
int getCurrentTick()
{
    if(context->isPlaying())
        return currentTick;
    else
        return -1;
}


const wxString getAudioExtension()
{
    return wxT(".wav");
}

const wxString getAudioWildcard()
{
    return  wxString( _("WAV file")) + wxT("|*.wav");
}

}

}

#pragma mark -

#include "glib.h"

namespace AriaMaestosa
{

void playMidiData(SeqContext *seqContext, char *data, int length);

SeqContext *seqContext; // FIXME - why 2? There's already one near the top

/* Number of elements in an array */
#define NELEM(a) ( sizeof(a)/sizeof((a)[0]) )

#define ADDR_PARTS 4 /* Number of part in a port description addr 1:2:3:4 */
#define SEP ", \t"	/* Separators for port description */

 //seq_context_t *pmidi_openport(char *portdesc);
 SeqContext *pmidi_openport(int client, int port);
 void playfile(SeqContext* seqContext, char* filename);
 void play(SeqContext* seqContext, struct event *el);
 //void set_signal_handler(seq_context_t *ctxp);
 //void signal_handler(int sig);

void prepareAlsaThenPlay(char* data, int length)
{
    seqContext = pmidi_openport(context->device->client, context->device->port);
    if (seqContext == NULL)
    {
        std::cout << "Could not open midi port" << std::endl;
        return;
    }


    /* Set signal handler */
    //set_signal_handler(ctxp);
    playMidiData(seqContext, data, length);
/*
    seq_free_context(ctxp);

    // Restore signal handler
    signal(SIGINT, SIG_DFL);
*/
    return;
}

SeqContext* pmidi_openport(int client, int port)
{
    SeqContext* seqContext = new SeqContext();

    int  err = seqContext->connectToPort(client, port);

    if (err < 0)
    {
        fprintf(stderr, "Could not connect to port %d:%d\n",
                client, port);
        return NULL;
    }
    return seqContext;

}

void playMidiData(SeqContext *seqContext, char *data, int length)
{
    struct sequenceState *seq;
    struct event* event;

    MidiDataProvider midiData(data, length);

    root = midi_read_file(midiData);
    if (!root)
    {
        std::cout << "ERROR: no root" << std::endl;
        return;
    }

    // Loop through all the elements in the song and play them
    seq = md_sequence_init(root);

    PlatformMidiManager::currentTick=0;

    // launch a new thread that takes care of tracking playback position
    PlatformMidiManager::threads::track_playback.runFunction(&PlatformMidiManager::track_playback_func);

    // the current thread continues pushing events to the queue
    // until all events have been processed or playback is interrupted
    while ((event = md_sequence_next(seq)) != NULL)
    {
        if(PlatformMidiManager::must_stop) return;
        play(seqContext, event);
    }

    // finish playing
    if(!PlatformMidiManager::must_stop)
        snd_seq_drain_output(seqContext->getAlsaHandle());

}

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
    AlsaNotePlayer::allSoundOff();

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

    AlsaNotePlayer::resetAllControllers();

}

/*
	unsigned long end;
	snd_seq_event_t *ep;

	if (strcmp(filename, "-") == 0)
		root = midi_read(stdin);
	else
		root = midi_read_file(filename);
	if (!root)
		return;

	//Get the end time for the tracks and echo an event to
	// wake us up at that time

	end = md_sequence_end_time(seq);
	seq_midi_echo(ctxp, end);

*/
void play(SeqContext* seqContext, struct event *el)
{
    snd_seq_event_t ev;

    seq_midi_event_init(seqContext, &ev, el->element_time, el->device_channel);

    switch (el->type)
    {
    case MD_TYPE_ROOT:
        seqContext->initTempo(MD_ROOT(el)->time_base, 120, 1);
        seqContext->startTimer();
        break;
    case MD_TYPE_NOTE:
        seq_midi_note(seqContext, &ev, el->device_channel, MD_NOTE(el)->note, MD_NOTE(el)->vel,
                      MD_NOTE(el)->length);
        break;
    case MD_TYPE_CONTROL:
        seq_midi_control(seqContext, &ev, el->device_channel, MD_CONTROL(el)->control,
                         MD_CONTROL(el)->value);
        break;
    case MD_TYPE_PROGRAM:
        seq_midi_program(seqContext, &ev, el->device_channel, MD_PROGRAM(el)->program);
        break;
    case MD_TYPE_TEMPO:
        seq_midi_tempo(seqContext, &ev, MD_TEMPO(el)->micro_tempo);
        break;
    case MD_TYPE_PITCH:
        seq_midi_pitchbend(seqContext, &ev, el->device_channel, MD_PITCH(el)->pitch);
        break;
    case MD_TYPE_PRESSURE:
        seq_midi_chanpress(seqContext, &ev, el->device_channel, MD_PRESSURE(el)->velocity);
        break;
    case MD_TYPE_KEYTOUCH:
        seq_midi_keypress(seqContext, &ev, el->device_channel, MD_KEYTOUCH(el)->note,
                          MD_KEYTOUCH(el)->velocity);
        break;
    case MD_TYPE_SYSEX:
        seq_midi_sysex(seqContext, &ev, MD_SYSEX(el)->status, MD_SYSEX(el)->data,
                       MD_SYSEX(el)->length);
        break;
    case MD_TYPE_TEXT:
    case MD_TYPE_KEYSIG:
    case MD_TYPE_TIMESIG:
    case MD_TYPE_SMPTEOFFSET:

        // Ones that have no sequencer action
        break;
    default:
        printf("WARNING: play: not implemented yet %d\n", el->type);
        break;
    }

}

/*
typedef struct sigaction s_sigaction;

 void
set_signal_handler(seq_context_t *ctxp)
{
    s_sigaction* sap = (s_sigaction*) calloc(1, sizeof(struct sigaction));

    g_ctxp = ctxp;

    sap->sa_handler = signal_handler;
    sigaction(SIGINT, sap, NULL);
}
*/
/* signal handler */
/*
 void
signal_handler(int sig)
{
    // Close device
    if (g_ctxp)
    {
        seq_free_context(g_ctxp);
    }

    exit(1);
}
*/
}

#endif
