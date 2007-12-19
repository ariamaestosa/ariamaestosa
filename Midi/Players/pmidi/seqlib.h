/*
 * File: seqlib.h
 *
 * Copyright (C) 1999-2003 Steve Ratcliffe
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
#include <alsa/asoundlib.h>

//#ifdef __cplusplus
//extern "C"
//{
//#endif

namespace AriaMaestosa
{

typedef struct seq_context seq_context_t;

seq_context_t *seq_create_context();
seq_context_t *seq_new_client(seq_context_t *ctxp);
void seq_free_context(seq_context_t *cxtp);
int seq_new_port(seq_context_t *ctxp);
void seq_destroy_port(seq_context_t *cxtp, int port);
int seq_connect_add(seq_context_t *ctxp, int client, int port);
int seq_init_tempo(seq_context_t *ctxp, int resolution, int tempo,
        int realtime);
void seq_set_queue(seq_context_t *ctxp, int q);
int seq_sendto(seq_context_t *ctxp, snd_seq_event_t *ev, int client, int port);
snd_seq_addr_t * seq_dev_addr(seq_context_t *ctxp, int dev);
void seq_start_timer(seq_context_t *ctxp);
void seq_stop_timer(seq_context_t *ctxp);
void seq_control_timer(seq_context_t *ctxp, int onoff);
int seq_write(seq_context_t *ctxp, snd_seq_event_t *ep);
void *seq_handle(seq_context_t *ctxp);

int get_current_tick(seq_context_t* ctxp);

void seq_midi_event_init(seq_context_t *ctxp, snd_seq_event_t *ep,
        unsigned long time, int devchan);
void seq_midi_note(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_note_on(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_note_off(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int vel, int length);
void seq_midi_keypress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int note,
        int value);
void seq_midi_control(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int control,
        int value);
void seq_midi_program(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int program);
void seq_midi_chanpress(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan,
        int pressure);
void seq_midi_pitchbend(seq_context_t *ctxp, snd_seq_event_t *ep, int devchan, int bend);
void seq_midi_tempo(seq_context_t *ctxp, snd_seq_event_t *ep, int tempo);
void seq_midi_sysex(seq_context_t *ctxp, snd_seq_event_t *ep, int status,
        unsigned char *data, int length);
void seq_midi_echo(seq_context_t *ctxp, unsigned long time);

}

//#ifdef __cplusplus
//}
//#endif
