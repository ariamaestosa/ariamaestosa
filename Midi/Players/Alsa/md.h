/*
 * File: md.h
 *
 * Copyright (C) 1999 Steve Ratcliffe
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
 *
 */

//extern "C"
//{
namespace AriaMaestosa
{

/* Defines for md_walk() */
#define MD_WALK_ALL	1	/* Include all elements including deleted, hidden*/

/* Defines for walk callback flags */
#define MD_WALK_START 0x001	/* This is a start element */
#define MD_WALK_END   0x002	/* This is the end of an element */
#define MD_WALK_EMPTY (MD_WALK_START|MD_WALK_END) /* Empty element */

/* Typedef for md_walk callback function */
typedef void (*walkFunc)(struct event *, void *, int);


/*
 * Structure to keep track of the position on each track that
 * is being merged.
 */
struct sequenceState {
	int  nmerge;	/* Number of tracks in trackPos to merge */
	struct trackPos *track_ptrs; /* Position pointers */
	struct rootElement *root; /* Root to be returned first */
	unsigned long endtime;	/* End time */
};
struct trackPos {
	int  len;	/* Total length of this container element */
	int  count;	/* Current position count */
	struct event **currel; /* Pointer to current position */
};

void md_walk(struct containerElement *c, walkFunc fn, void *arg, int flags);
bool iscontainer(struct event *el);
struct sequenceState *md_sequence_init(struct rootElement *root);
struct event *md_sequence_next(struct sequenceState *seq);
void md_sequence_end(struct sequenceState *seq);
unsigned long md_sequence_end_time(struct sequenceState *seq);

}

//}
