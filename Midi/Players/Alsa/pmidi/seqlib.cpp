/*
 * File: seqlib.m - Interface to the alsa sequencer library.
 *
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
 */
#ifdef _PMIDI_ALSA

#include <alsa/asoundlib.h>

#include "glib.h"

#include "Midi/Players/Alsa/pmidi/elements.h"

#include <iostream>

#include <stdio.h>
#include <string.h>
#include "Midi/Players/Alsa/pmidi/seqlib.h"

namespace AriaMaestosa
{

static void set_channel(snd_seq_event_t *ep, int chan);

/*
 * Create a main client for an application. A queue is
 * allocated and a client is created.
 */
SeqContext::SeqContext()
{
	int  q;

	this->main = this; /* This is the main context */

	if (snd_seq_open(&alsa_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0)
		std::cout << "ERROR " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(ioError, "Could not open sequencer: %s", snd_strerror(errno));

	q = snd_seq_alloc_queue(alsa_handle);
	this->client = snd_seq_client_id(alsa_handle);
	this->queue = q;

	this->destlist = g_array_new(0, 0, sizeof(snd_seq_addr_t));

	this->queue = q;
	this->source.client = this->client;
	this->source.port = 0;

	this->newPort();
}


/*
 * Free a context and any associated data and resources. If the
 * main context is freed then the sequencer is closed. TODO:
 * link all together so can all be closed at once.
 */
SeqContext::~SeqContext()
{

	if (this->main == this) {
		// This is the main context
		snd_seq_event_t ev;
		unsigned long t;

		snd_seq_drop_output(alsa_handle);

		t = 0;
		seq_midi_event_init(this, &ev, t, 0);
		seq_midi_control(this, &ev, 0, MIDI_CTL_ALL_SOUNDS_OFF, 0);
		// wont build when uncommented, but doesn't seem important so i'll just leave it out
		//seq_send_to_all(this, &ev);
		snd_seq_drain_output(alsa_handle);

		snd_seq_free_queue(alsa_handle, this->queue);
		snd_seq_close(alsa_handle);
	}
}

/*
 * Creates a new port on the specified context and returns the
 * port number.
 *  Arguments:
 *    this      - Context to create the port for
 */
int SeqContext::newPort()
{
	int  ret;

	ret = snd_seq_create_simple_port(alsa_handle, NULL,
					 SND_SEQ_PORT_CAP_WRITE |
					 SND_SEQ_PORT_CAP_SUBS_WRITE |
					 SND_SEQ_PORT_CAP_READ,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (ret < 0) {
		printf("Error creating port %s\n", snd_strerror(errno));
		return -1;
	}
	this->port_count++;
	this->source.port = ret;

	return ret;/*XXX*/
}

void SeqContext::destroyPort(int port)
{
}

/*
 * Set up the context so that all subsequent events will be sent
 * to the specified client and port combination. A
 * subscription is made to the client/port combination.
 *  Arguments:
 *    this      - Context to modify
 *    client    - Client to send subsequent events to
 *    port      - Port on the client to send events to
 */
int SeqContext::connectToPort(int client, int port)
{
	snd_seq_addr_t addr;
	int  ret;

	memset(&addr, 0, sizeof(addr));
	addr.client = client;
	addr.port = port;

	g_array_append_val(this->destlist, addr);

	ret = snd_seq_connect_to(alsa_handle, this->source.port, client, port);

	return ret;
}

/*
 * Set the initial time base and tempo. This should only be used
 * for initialisation when there is nothing playing. To
 * change the tempo during a song tempo change events are used.
 * If realtime is false the resolution is in ticks per quarter
 * note. If true, the the resolution is microseconds. There is
 * a macro XXX to convert from SMPTE codes.
 *
 *  Arguments:
 *    this      - Application context
 *    resolution - Ticks per quarter note or realtime resolution
 *    tempo     - Beats per minute
 *    realtime  - True if absolute time base
 */
int SeqContext::initTempo(int resolution, int tempo, int realtime)
{
	snd_seq_queue_tempo_t *qtempo;
	int  ret;

	snd_seq_queue_tempo_alloca(&qtempo);
	memset(qtempo, 0, snd_seq_queue_tempo_sizeof());
	snd_seq_queue_tempo_set_ppq(qtempo, resolution);
	snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/tempo);

	ret = snd_seq_set_queue_tempo(alsa_handle, this->queue, qtempo);

	return ret;
}

int SeqContext::getCurrentTick()
{
    snd_seq_queue_status_t* qstatus;
    snd_seq_queue_status_malloc(&qstatus);

    snd_seq_get_queue_status(alsa_handle,this->queue,qstatus);

    snd_seq_tick_time_t tick = snd_seq_queue_status_get_tick_time(qstatus);
    snd_seq_queue_status_free(qstatus);
    return tick;

    /*
    snd_seq_queue_status_malloc (snd_seq_queue_status_t **ptr)

    snd_seq_tick_time_t 	snd_seq_queue_status_get_tick_time (const snd_seq_queue_status_t *info)
 	Get the tick time of a queue_status container.

int 	snd_seq_get_queue_status (snd_seq_t *handle, int q, snd_seq_queue_status_t *status)
 	obtain the running state of the queue
 	*/
}

/*
 * Set the context to use the specified queue. All events sent
 * on this context will now use this queue.
 *
 *  Arguments:
 *    this      - Context to modify
 *    q         - The queue number
 */
 /*
void SeqContext::SetQueue(int q)
{
}
*/

/*
 * Send the event to the specified client and port.
 *
 *  Arguments:
 *    this      - Client context
 *    ev        - Event to send
 *    client    - Client to send the event to
 *    port      - Port to send the event to
 */
int SeqContext::sendTo( snd_seq_event_t *ev, int client, int port)
{
	ev->source = this->source;
	ev->queue = this->queue;
	ev->dest.client = client;
	ev->dest.port = port;
	this->write(ev);

	return 0;
}

/*
 * seq_send_to_all:
 * Send the event to all the connected devices across all
 * possible channels. The messages are sent to the blocking
 * control port and so should not have timestamps in the
 * future. This function is intended for all-sounds-off type
 * controls etc.
 *
 *  Arguments:
 * 	this: Client context
 * 	ep: Event prototype to be sent
 */
int SeqContext::sendToAll(snd_seq_event_t *ep)
{
	int  dev;
	int  chan;
	snd_seq_addr_t *addr;

	for (dev = 0; ; dev++)
	{
		addr = this->getDeviceAddress(dev);
		if (addr == NULL)
			break; /* No more */

		ep->queue = this->queue;
		ep->dest = *addr;
		for (chan = 0; chan < 16; chan++)
		{
			set_channel(ep, chan);
			(void) this->write(ep);
		}
	}

	return 0;
}

/*
 * seq_dev_addr:
 * Return the address corresponding to the specified logical
 * device.
 *
 * 	Arguments:
 * 	dev: Device to get the address of.
 * 	inout: Input or output device
 */
snd_seq_addr_t * SeqContext::getDeviceAddress(int dev)
{
	GArray *list;
	snd_seq_addr_t *addr;

	list = this->destlist;

	if (dev >= (int)list->len)
		return NULL;

	addr = &g_array_index(list, snd_seq_addr_t, dev);

	return addr;
}

/*
 * Start the timer. (What about timers other than the system
 * one?)
 */
void SeqContext::startTimer()
{
	this->controlTimer(SND_SEQ_EVENT_START);
}

/*
 * Stop the timer. (What about timers other than the system
 * one?)
 */
void SeqContext::stopTimer()
{
	this->controlTimer(SND_SEQ_EVENT_STOP);
}

void SeqContext::controlTimer(int onoff)
{

	if (onoff == SND_SEQ_EVENT_START)
		snd_seq_start_queue(alsa_handle, this->queue, 0);
	else
	{
		snd_seq_stop_queue(alsa_handle, this->queue, 0);
	}

	snd_seq_drain_output(alsa_handle);

}

/*
 * Write out the event. This routine blocks until
 * successfully written.
 *
 *  Arguments:
 *    this      - Application context
 *    ep        - Event
 */
int SeqContext::write(snd_seq_event_t *ep)
{
	int  err = 0;

	err = snd_seq_event_output(alsa_handle, ep);
	if (err < 0)
		return err;

	return err;
}

/*
 * Return the handle to the underlying snd_seq stream.
 */
snd_seq_t* SeqContext::getAlsaHandle()
{
	return alsa_handle;
}

static void
set_channel(snd_seq_event_t *ep, int chan)
{

	switch (ep->type) {
	case SND_SEQ_EVENT_NOTE:
	case SND_SEQ_EVENT_NOTEON:
	case SND_SEQ_EVENT_NOTEOFF:
		ep->data.note.channel = chan;
		break;
	case SND_SEQ_EVENT_KEYPRESS:
	case SND_SEQ_EVENT_PGMCHANGE:
	case SND_SEQ_EVENT_CHANPRESS:
	case SND_SEQ_EVENT_PITCHBEND:
	case SND_SEQ_EVENT_CONTROL14:
	case SND_SEQ_EVENT_NONREGPARAM:
	case SND_SEQ_EVENT_REGPARAM:
	case SND_SEQ_EVENT_CONTROLLER:
		ep->data.control.channel = chan;
		break;
	default:
		if (snd_seq_ev_is_channel_type(ep))
			g_warning("Missed a case in set_channel");
		break;
	}
}

}
#endif
