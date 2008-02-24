/*
 * File: seqlib.h
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
 *
 *
 * ---------
 * This file is a set of interfaces to provide a little
 * higher level view of the sequencer.  It is mainly experimental
 * to investigate new approaches.  If successful they will be proposed
 * as additions/replacements to the existing seq lib.
 *
 *
 *
 */
#include "Config.h"
#include <alsa/asoundlib.h>

namespace AriaMaestosa
{

class SeqContext
{
    DECLARE_LEAK_CHECK();
    void controlTimer(int onoff);

    public:
	snd_seq_t *alsa_handle; /* The snd_seq handle to /dev/snd/seq */
	int  client;/* The client associated with this context */
	int  queue; /* The queue to use for all operations */
	snd_seq_addr_t  source;	/* Source for events */
	GArray  *destlist;	/* Destination list */
#define ctxndest destlist->len
#define ctxdest  destlist->data

	char  timer_started;	/* True if timer is running */
	int   port_count;		/* Ports allocated */

	SeqContext* main; /* Pointer to the main context */
	GSList *ctlist;		/* Context list if a main context */

	// ---------------

	SeqContext();
	~SeqContext();

	int newPort();
	void destroyPort(int port);

	int connectToPort(int client, int port);

    int initTempo(int resolution, int tempo, int realtime);

    int getCurrentTick();

    //void setQueue(int q);
    int sendTo(snd_seq_event_t *ev, int client, int port);
    int sendToAll(snd_seq_event_t *ep);

    snd_seq_addr_t * getDeviceAddress(int device);

    void startTimer();
    void stopTimer();

    int write(snd_seq_event_t *ep);

    snd_seq_t* getAlsaHandle();
};

/*
seq_context_t *seq_create_context();
seq_context_t *seq_new_client(seq_context_t *ctxp);
void seq_free_context(seq_context_t *cxtp);
int seq_new_port(seq_context_t *ctxp);
void seq_destroy_port(seq_context_t *cxtp, int port);
int seq_connect_add(seq_context_t *ctxp, int client, int port);
int seq_init_tempo(seq_context_t *ctxp, int resolution, int tempo,
        int realtime);
int get_current_tick(seq_context_t* ctxp);
void seq_set_queue(seq_context_t *ctxp, int q);
int seq_sendto(seq_context_t *ctxp, snd_seq_event_t *ev, int client, int port);
snd_seq_addr_t * seq_dev_addr(seq_context_t *ctxp, int dev);
void seq_start_timer(seq_context_t *ctxp);
void seq_stop_timer(seq_context_t *ctxp);
void seq_control_timer(seq_context_t *ctxp, int onoff);
int seq_write(seq_context_t *ctxp, snd_seq_event_t *ep);
void *seq_handle(seq_context_t *ctxp);
int get_current_tick(seq_context_t* ctxp);
*/

class MidiEvent
{
    DECLARE_LEAK_CHECK();

    SeqContext* seqContext;
    snd_seq_event_t ep;
    unsigned long time;
    int devchan;

    public:

    MidiEvent(SeqContext* seqContext, unsigned long time, int devchan);

    void note(int note, int vel, int length);
    void noteOn(int note, int vel, int length);
    void noteOff(int note, int vel, int length);
    void keyPress(int note, int value);
    void control(int control, int value);
    void program(int program);
    void chanPress(int pressure);
    void pitchBend(int bend);
    void tempo(int tempo);
    void sysex(int status, unsigned char *data, int length);
    //void echo();
};

/*
void seq_midi_event_init(SeqContext *ctxp, snd_seq_event_t *ep,
        unsigned long time, int devchan);
void seq_midi_note(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_note_on(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_note_off(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_keypress(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int value);
void seq_midi_control(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int control,
        int value);
void seq_midi_program(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int program);
void seq_midi_chanpress(SeqContext *ctxp, snd_seq_event_t *ep, int devchan,
        int pressure);
void seq_midi_pitchbend(SeqContext *ctxp, snd_seq_event_t *ep, int devchan, int bend);
void seq_midi_tempo(SeqContext *ctxp, snd_seq_event_t *ep, int tempo);
void seq_midi_sysex(SeqContext *ctxp, snd_seq_event_t *ep, int status,
        unsigned char *data, int length);
void seq_midi_echo(SeqContext *ctxp, unsigned long time);
*/
}


