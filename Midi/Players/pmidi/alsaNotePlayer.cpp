#include "wx/wx.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Players/pmidi/alsaNotePlayer.h"

namespace AriaMaestosa
{
namespace AlsaNotePlayer
{
void playQueue();

class StopNoteTimer : public wxTimer {

public:

StopNoteTimer() : wxTimer(){ }

void Notify()
{
	PlatformMidiManager::stopNote();
	wxTimer::Stop();
}

void start(int duration)
{
	Start(duration);
}
};

}
}

#include <stdio.h>
#include <alsa/asoundlib.h>
#include "main.h"
#include <vector>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "Midi/Players/pmidi/AlsaPort.h"

namespace AriaMaestosa
{
namespace AlsaNotePlayer
{

StopNoteTimer* stopNoteTimer = NULL;
MidiContext* context;

void setContext(MidiContext* context_arg)
{
   context = context_arg;
}

void init()
{
 stopNoteTimer = new StopNoteTimer();
}
void free()
{
 	if(stopNoteTimer != NULL)
	{
		delete stopNoteTimer;
		stopNoteTimer = NULL;
	}
}

void noteOn(int channel, int note, int velocity, int duration)
{

	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_noteon(&event, channel, note, velocity);

	snd_seq_event_output_direct(context->sequencer, &event);
	snd_seq_drain_output(context->sequencer);

	stopNoteTimer->start(duration);
}

void noteOff(int channel, int note)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_noteoff(&event, channel, note, 0 /*velocity*/);

	snd_seq_event_output_direct(context->sequencer, &event);
	snd_seq_drain_output(context->sequencer);
}

void programChange(int channel, int program)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_pgmchange(&event, channel, program);

	if (snd_seq_event_output_direct(context->sequencer, &event) < 0)
	{
		return;
	}
	snd_seq_drain_output(context->sequencer);
}

void allSoundOff()
{
    for(int channel=0; channel<16; channel++)
    {
        controlChange(channel, 0x78 /*120*/ /* all sound off */, 0);
    }
}

void resetAllControllers()
{
    for(int channel=0; channel<16; channel++)
    {
        controlChange(channel, 0x78 /*120*/ /* all sound off */, 0);
        controlChange(channel, 0x79 /*121*/ /* reset controllers */, 0);
        controlChange(channel, 7 /* reset volume */, 127);
        controlChange(channel, 10 /* reset pan */, 64);
    }
}

void controlChange(int channel, int control, int value)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_controller(&event, channel, control, value);

	if(snd_seq_event_output_direct(context->sequencer, &event) < 0)
	{
		return;
	}
	snd_seq_drain_output(context->sequencer);
}

void pitchBend(int channel, int value)
{
	snd_seq_event_t event;

	snd_seq_ev_clear(&event);

	event.queue  = SND_SEQ_QUEUE_DIRECT;
	event.source = context->address;

	snd_seq_ev_set_subs(&event);
	snd_seq_ev_set_direct(&event);
	snd_seq_ev_set_pitchbend(&event, channel, value);

	snd_seq_event_output_direct(context->sequencer, &event);
	snd_seq_drain_output(context->sequencer);
}

}
}
