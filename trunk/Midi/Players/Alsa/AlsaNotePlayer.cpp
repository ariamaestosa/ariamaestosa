#include "wx/wx.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Players/Alsa/AlsaNotePlayer.h"
#include "Midi/Players/Alsa/AlsaPort.h"

namespace AriaMaestosa
{
namespace AlsaNotePlayer
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
    MidiContext* context;

}
}


#include <alsa/asoundlib.h>

namespace AriaMaestosa
{

namespace PlatformMidiManager
{

    // play/stop a single preview note
    void playNote(int noteNum, int volume, int duration, int channel, int instrument)
    {
        using namespace AlsaNotePlayer;
        if(context->isPlaying()) return;
        if(lastPlayedNote != -1) stopNote();
        programChange(channel, instrument);
        controlChange(channel, 7, 127); // max volume
        noteOn(channel, noteNum, volume, duration);
        lastPlayedNote = noteNum;
        lastChannel = channel;
    }

    void stopNote()
    {
        AlsaNotePlayer::noteOff(AlsaNotePlayer::lastChannel, AlsaNotePlayer::lastPlayedNote);
        AlsaNotePlayer::lastPlayedNote = -1;
    }

}

namespace AlsaNotePlayer
{


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

void setContext(MidiContext* context_arg)
{
   context = context_arg;
}

void stopNoteIfAny()
{
    if(lastPlayedNote != -1) PlatformMidiManager::stopNote();
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
