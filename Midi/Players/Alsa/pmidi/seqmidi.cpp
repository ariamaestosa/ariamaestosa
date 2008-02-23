/*
 * File: seqmidi.m - convert to the sequencer events
 *
 * Copyright (C) 1999 Steve Ratcliffe
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
 */
#ifdef _PMIDI_ALSA

#include <iostream>

#include "glib.h"
#include "Midi/Players/Alsa/pmidi/elements.h"

#include <string.h>
#include "Midi/Players/Alsa/pmidi/seqlib.h"
#include "Midi/Players/Alsa/pmidi/seqpriv.h"

namespace AriaMaestosa
{

/*
 * Initialize a midi event from the context. The source and
 * destination addresses will be set from the information
 * stored in the context, which depends on the previous
 * connection calls. In addition the time and channel are set.
 * This should be called first before any of the following
 * functions.
 *
 *  Arguments:
 *    ctxp      - Client application
 *    ep        - Event to init
 *    time      - Midi time
 *    devchan   - Midi channel
 */
void
seq_midi_event_init(seq_context_t *ctxp, snd_seq_event_t *ep,
        unsigned long time, int devchan)
{
	int  dev;

	dev = devchan >> 4;

	/*
	 * If insufficient output devices have been registered, then we
	 * just scale the device back to fit in the correct range.  This
	 * is not necessarily what you want.
	 */
	if (dev >= ctxp->ctxndest)
		dev = dev % ctxp->ctxndest;

	snd_seq_ev_clear(ep);
	snd_seq_ev_schedule_tick(ep, ctxp->queue, 0, time);
	ep->source = ctxp->source;
	if (ctxp->ctxndest > 0)
		ep->dest = g_array_index(ctxp->destlist, snd_seq_addr_t, dev);
}

/*
 * Send a note event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of noten declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:116: error: '::freopen' has not been declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:117: error: '::fscanf' has not been d
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void
seq_midi_note(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel,
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTE;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;
	seq_write(ctxp, ep);
}

/*
 * Send a note on event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void
seq_midi_note_on(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel,
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEON;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	seq_write(ctxp, ep);
}

/*
 * Send a note off event.
 *  Arguments:
 *    ctxp      - Client context
 *    ep        - Event template
 *    note      - Pitch of note
 *    vel       - Velocity of note
 *    length    - Length of note
 */
void
seq_midi_note_off(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note, int vel,
        int length)
{
	ep->type = SND_SEQ_EVENT_NOTEOFF;

	ep->data.note.channel = devchan & 0xf;
	ep->data.note.note = note;
	ep->data.note.velocity = vel;
	ep->data.note.duration = length;

	seq_write(ctxp, ep);
}

/*
 * Send a key pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    note      - Note to be altered
 *    value     - Pressure value
 */
void
seq_midi_keypress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int value)
{
	ep->type = SND_SEQ_EVENT_KEYPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = note;
	ep->data.control.value = value;
	seq_write(ctxp, ep);
}

/*
 * Send a control event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    control   - Controller to change
 *    value     - Value to set it to
 */
void
seq_midi_control(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int control,
        int value)
{
	ep->type = SND_SEQ_EVENT_CONTROLLER;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.param = control;
	ep->data.control.value = value;
	seq_write(ctxp, ep);
}

/*
 * Send a program change event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    program   - Program to set
 */
void
seq_midi_program(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int program)
{
	ep->type = SND_SEQ_EVENT_PGMCHANGE;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = program;
	seq_write(ctxp, ep);
}

/*
 * Send a channel pressure event.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    pressure  - Pressure value
 */
void
seq_midi_chanpress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int pressure)
{
	ep->type = SND_SEQ_EVENT_CHANPRESS;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = pressure;
	seq_write(ctxp, ep);
}

/*
 * Send a pitchbend message. The bend parameter is centered on
 * zero, negative values mean a lower pitch.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    bend      - Bend value, centered on zero.n declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:116: error: '::freopen' has not been declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:117: error: '::fscanf' has not been d
 */
void
seq_midi_pitchbend(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int bend)
{
	ep->type = SND_SEQ_EVENT_PITCHBEND;

	ep->data.control.channel = devchan & 0xf;
	ep->data.control.value = bend;
	seq_write(ctxp, ep);
}

/*
 * Send a tempo event. The tempo parameter is in microseconds
 * per beat.
 *  Arguments:n declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:116: error: '::freopen' has not been declared
/usr/lib/gcc/powerpc-linux-gnu/4.1.2/../../../../include/c++/4.1.2/cstdio:117: error: '::fscanf' has not been d
 *    ctxp      - Application context
 *    ep        - Event template
 *    tempo     - New tempo in usec per beat
 */
void
seq_midi_tempo(seq_context_t *ctxp, snd_seq_event_t *ep, int tempo)
{
	ep->type = SND_SEQ_EVENT_TEMPO;

	ep->data.queue.queue = ctxp->queue;
	ep->data.queue.param.value = tempo;
	ep->dest.client = SND_SEQ_CLIENT_SYSTEM;
	ep->dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
	seq_write(ctxp, ep);
}

/*
 * Send a sysex event. The status byte is to distiguish
 * continuation sysex messages.
 *  Arguments:
 *    ctxp      - Application context
 *    ep        - Event template
 *    status    - Status byte for sysex
 *    data      - Data to send
 *    length    - Length of data
 */
void
seq_midi_sysex(seq_context_t *ctxp, snd_seq_event_t *ep, int status,
        unsigned char *data, int length)
{
	unsigned char* ndata;
	int  nlen;

	ep->type = SND_SEQ_EVENT_SYSEX;

	ndata = (unsigned char*)g_malloc(length + 1);
	nlen = length +1;

	ndata[0] = status;
	memcpy(ndata+1, data, length);

	snd_seq_ev_set_variable(ep, nlen, ndata);

	seq_write(ctxp, ep);

	g_free(ndata);
}

/*
 * Send an echo event back to the source client at the specified
 * time.
 *
 *  Arguments:
 *    ctxp      - Application context
 *    time      - Time of event
 */
void
seq_midi_echo(seq_context_t *ctxp, unsigned long time)
{
	snd_seq_event_t ev;

	seq_midi_event_init(ctxp, &ev, time, 0);
	/* Loop back */
	ev.dest = ctxp->source;
	seq_write(ctxp, &ev);
}
//}
}
#endif
