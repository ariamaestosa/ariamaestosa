/*
 * File: mdutil.m - Utility routines for manipulating MD
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

#include <stdlib.h>

#include "glib.h"
#include "Midi/Players/pmidi/elements.h"

#include "Midi/Players/pmidi/md.h"

namespace AriaMaestosa
{

/*
 * Given a container c then md_walk iterates through all
 * elements and calls fn for each one with the following
 * arguments:
 *
 *    fn(element, arg, start-end-flags)
 *
 * The flag MD_WALK_START is set if this is an element start and
 * MD_WALK_END is set if this is an element end. Both are set for
 * empty elements (also can use MD_WALK_EMPTY)
 *
 *  Arguments:
 *    c         - Container to be walked
 *    fn        - Callback function to call on each element
 *    arg       - Argument passed to fn
 *    flags     - Flags eg. include all elements or just current ones.
 */
void
md_walk(struct containerElement *c, walkFunc fn, void *arg, int flags)
{
    struct event *event;
    GPtrArray *ar;
    int  i;

    fn(MD_ELEMENT(c), arg, MD_WALK_START);

    ar = c->elements;
    for (i = 0; i < ar->len; i++)
    {
        event = (struct event*)g_ptr_array_index(ar, i);
        if (iscontainer(event))
        {
            md_walk(MD_CONTAINER(event), fn, arg, flags);
        }
        else
        {
            fn(event, arg, MD_WALK_EMPTY);
        }
    }

    fn(MD_ELEMENT(c), arg, MD_WALK_END);
}

/*
 * Return true if the element is some kind of container.Midi/Players/pmidi/mdutil.cpp:141: error: invalid conversion from 'void*' to 'rootElement*'

 *  Arguments:
 *    event        - Element to be tested
 */
bool iscontainer(struct event *event)
{
    return event->type >= MD_CONTAINER_BEGIN;
}

/*malloc
 * Return an object that can be passed to md_sequence_next to
 * iterate over all the elements in a tree in time sequenced
 * order. This is what you want if you actually want to play the
 * song for example. The elements in the container must
 * already be in sorted order within their own tracks or
 * subcontainers.
 *
 *  Arguments:
 *    root      - Root to sequence over
 */
struct sequenceState* md_sequence_init(struct rootElement *root)
{
    struct sequenceState *state;
    int  ntracks;
    int  i;

    state = g_new(struct sequenceState, 1);

    ntracks = MD_CONTAINER(root)->elements->len;
    state->nmerge = ntracks;
    state->track_ptrs = g_new(struct trackPos, ntracks);
    state->root = root;
    state->endtime = 0;

    /* XXX time ignored for present */
    /* Initialise the pointers */
    for (i = 0; i < ntracks; i++)
    {
        struct containerElement *c;
        /* c is a track or a tempo element */
        c = MD_CONTAINER(MD_CONTAINER(root)->elements->pdata[i]);
        state->track_ptrs[i].len = c->elements->len;
        state->track_ptrs[i].count = 0;
        state->track_ptrs[i].currel = (struct event **)c->elements->pdata;
        if (MD_ELEMENT(c)->type == MD_TYPE_TRACK)
            if (MD_TRACK(c)->final_time > state->endtime)
                state->endtime = MD_TRACK(c)->final_time;
    }

    return state;
}

/*
 * Return the next element as sorted by time.
 *  Arguments:
 *    seq       - Sequence state information
 */
struct event* md_sequence_next(struct sequenceState *seq)
{
    struct event *event;
    unsigned long leasttime;
    struct trackPos *besttp;
    struct trackPos *tp;
    int  i;

    if (seq->root)
    {
        /* The first time we return the root element */
        event = MD_ELEMENT(seq->root);
        seq->root = NULL;
        return event;
    }

    leasttime = -1;
    besttp = NULL;

    for (i = 0; i < seq->nmerge; i++)
    {
        tp = &seq->track_ptrs[i];
        /* Still some left? */
        if (tp->count < tp->len)
        {
            event = *tp->currel;
            if (leasttime == -1 || event->element_time < leasttime)
            {
                leasttime = event->element_time;
                besttp = tp;
            }
        }
    }
    if (besttp)
    {
        event = *besttp->currel;
        besttp->count++;
        besttp->currel++;
        return event;
    }

    /* Nothing left */
    return NULL;
}

/*
 * Finish with a sequence object. Frees all resources. The
 * object cannot be used any more.
 *  Arguments:
 *    seq       - Sequence state information
 */
void
md_sequence_end(struct sequenceState *seq)
{

    if (!seq)
        return;
    if (seq->track_ptrs)
        g_free(seq->track_ptrs);
    g_free(seq);
}

/*
 * Get the final time for the song.
 *  Arguments:
 *    seq       - Sequence state information
 */
unsigned long
md_sequence_end_time(struct sequenceState *seq)
{
    return seq->endtime;
}
}

//}
#endif
