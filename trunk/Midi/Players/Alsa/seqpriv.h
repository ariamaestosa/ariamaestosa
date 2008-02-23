/*
 * File: seqpriv.h
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

//#ifdef __cplusplus
//extern "C"
//{
//#endif

namespace AriaMaestosa
{

struct seq_context {
	snd_seq_t *handle; /* The snd_seq handle to /dev/snd/seq */
	int  client;/* The client associated with this context */
	int  queue; /* The queue to use for all operations */
	snd_seq_addr_t  source;	/* Source for events */
	GArray  *destlist;	/* Destination list */
#define ctxndest destlist->len
#define ctxdest  destlist->data

	char  timer_started;	/* True if timer is running */
	int   port_count;		/* Ports allocated */

	struct seq_context *main; /* Pointer to the main context */
	GSList *ctlist;		/* Context list if a main context */
};

//#ifdef __cplusplus
//}
//#endif

}
