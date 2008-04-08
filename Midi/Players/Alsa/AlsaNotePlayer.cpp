#include "wx/wx.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Players/Alsa/AlsaNotePlayer.h"
#include "Midi/Players/Alsa/AlsaPort.h"

#include <alsa/asoundlib.h>

namespace AriaMaestosa
{
namespace PlatformMidiManager
{

    void playQueue();

    // This is the timer that is started when a single note is played.
    // It is set to trigger a notification after an amount of time about
    // to note duration. When notification occurs, it stops the note.
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


int lastPlayedNote = -1, lastChannel;
StopNoteTimer* stopNoteTimer = NULL;
MidiContext* context_ref;

void allSoundOff()
{
    for(int channel=0; channel<16; channel++)
    {
        PlatformMidiManager::seq_controlchange(0x78 /*120*/ /* all sound off */, 0, channel);
    }
}

void resetAllControllers()
{
    for(int channel=0; channel<16; channel++)
    {
        seq_controlchange(0x78 /*120*/ /* all sound off */, 0, channel);
        seq_controlchange(0x79 /*121*/ /* reset controllers */, 0, channel);
        seq_controlchange(7 /* reset volume */, 127, channel);
        seq_controlchange( 10 /* reset pan */, 64, channel);
    }
    // FIXME - reset pitch bend!!
}

void stopNoteIfAny()
{
    if(lastPlayedNote != -1) stopNote();
}

void alsa_output_module_init()
{
    stopNoteTimer = new StopNoteTimer();
}
void alsa_output_module_free()
{
 	if(stopNoteTimer != NULL)
	{
		delete stopNoteTimer;
		stopNoteTimer = NULL;
	}
}

void alsa_output_module_setContext(MidiContext* context_arg)
{
   context_ref = context_arg;
}

// ----------------------------------------------------------
// base PlatformMidiManager functions

#pragma mark -

// play/stop a single preview note
void playNote(int noteNum, int volume, int duration, int channel, int instrument)
{
    if(context_ref->isPlaying()) return;
    if(lastPlayedNote != -1) stopNote();
    seq_prog_change(instrument, channel);
    seq_controlchange(7, 127, channel); // max volume
    seq_note_on(noteNum, volume, channel);
    stopNoteTimer->start(duration);

    lastPlayedNote = noteNum;
    lastChannel = channel;
}

void stopNote()
{
    seq_note_off(lastPlayedNote, lastChannel);
    lastPlayedNote = -1;
}


// ----------------------------------------------------------
// PlatformMidiManager generic timer functions

#pragma mark -

void seq_note_on(const int note, const int volume, const int channel)
{
    snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context_ref->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_noteon(&event, channel, note, volume);

	snd_seq_event_output_direct(context_ref->sequencer, &event);
	snd_seq_drain_output(context_ref->sequencer);
}


void seq_note_off(const int note, const int channel)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context_ref->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_noteoff(&event, channel, note, 0 /*velocity*/);

	snd_seq_event_output_direct(context_ref->sequencer, &event);
	snd_seq_drain_output(context_ref->sequencer);
}

void seq_prog_change(const int instrumentID, const int channel)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context_ref->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_pgmchange(&event, channel, instrumentID);

	if (snd_seq_event_output_direct(context_ref->sequencer, &event) < 0)
	{
		return;
	}
	snd_seq_drain_output(context_ref->sequencer);
}

void seq_controlchange(const int controller, const int value, const int channel)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context_ref->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_controller(&event, channel, controller, value);

	if(snd_seq_event_output_direct(context_ref->sequencer, &event) < 0)
	{
		return;
	}
	snd_seq_drain_output(context_ref->sequencer);
}

void seq_pitch_bend(const int value, const int channel)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context_ref->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_pitchbend(&event, channel, value);

	snd_seq_event_output_direct(context_ref->sequencer, &event);
	snd_seq_drain_output(context_ref->sequencer);
}


}
}
