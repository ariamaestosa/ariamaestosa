/*
 *
 * File: elements.cpp - Operation on all elements
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
#include <string.h>

#include "glib.h"

#include "Midi/Players/Alsa/pmidi/elements.h"
#include "Midi/Players/Alsa/pmidi/md.h"

#include <iostream>

namespace AriaMaestosa
{

static void md_container_init(struct containerElement *e);

/*
 * Create and initialise a element element.
 */
struct event*
md_element_new()
{
	struct event*  new_evt;

	new_evt = (event*) g_malloc0(sizeof(*new_evt));
	MD_ELEMENT(new_evt)->type = MD_TYPE_ELEMENT;
	return new_evt;
}

/*
 * Create and initialise a container element.
 */
struct containerElement *
md_container_new()
{
	struct containerElement*  newElement;

	newElement = (containerElement*)g_malloc0(sizeof(*newElement));
	MD_ELEMENT(newElement)->type = MD_TYPE_CONTAINER;
	return newElement;
}

/*
 * Initialize data structures within the element. This is
 * only really required when pointers need allocating.
 *  Arguments:
 *    e         - Element to init
 */
static void
md_container_init(struct containerElement *e)
{

	e->elements = g_ptr_array_new();
}

/*
 * Create and initialise a root element.
 *  Arguments:
 *              -
 */
struct rootElement *
md_root_new(void)
{
	struct rootElement*  newRoot;

	newRoot = (rootElement*)g_malloc0(sizeof(*newRoot));
	MD_ELEMENT(newRoot)->type = MD_TYPE_ROOT;
	md_container_init(MD_CONTAINER(newRoot));
	return newRoot;
}

/*
 * Create and initialise a track element.
 *  Arguments:
 *              -
 */
struct trackElement*
md_track_new(void)
{
	struct trackElement* newTrackElmt;

	newTrackElmt = (trackElement*)g_malloc0(sizeof(*newTrackElmt));
	MD_ELEMENT(newTrackElmt)->type = MD_TYPE_TRACK;
	md_container_init(MD_CONTAINER(newTrackElmt));
	return newTrackElmt;
}

/*
 * Create and initialise a tempomap element.
 */
struct tempomapElement *
md_tempomap_new()
{
	struct tempomapElement *  newTempoMap;

	newTempoMap = (tempomapElement*)g_malloc0(sizeof(*newTempoMap));
	MD_ELEMENT(newTempoMap)->type = MD_TYPE_TEMPOMAP;
	md_container_init(MD_CONTAINER(newTempoMap));
	return newTempoMap;
}

/*
 * Create and initialise a note element.
 *  Arguments:
 *    note      -
 *    vel       -
 *    length    -
 */
struct noteElement *
md_note_new(short note, short vel, int length)
{
	struct noteElement *  newNoteElmt;

	newNoteElmt = (noteElement*)g_malloc0(sizeof(*newNoteElmt));
	MD_ELEMENT(newNoteElmt)->type = MD_TYPE_NOTE;
	newNoteElmt->note = note;
	newNoteElmt->vel  = vel;
	newNoteElmt->length = length;
	newNoteElmt->offvel = 0;
	return newNoteElmt;
}

/*
 * Create and initialise a part element.
 *  Arguments:
 *              -
 */
struct partElement *
md_part_new(void)
{
	struct partElement*  newPartElmt;

	newPartElmt = (partElement*)g_malloc0(sizeof(*newPartElmt));
	MD_ELEMENT(newPartElmt)->type = MD_TYPE_PART;
	md_container_init(MD_CONTAINER(newPartElmt));
	return newPartElmt;
}

/*
 * Create and initialise a control element.
 *  Arguments:
 *    control   -
 *    value     -
 */
struct controlElement *
md_control_new(short control, short value)
{
	struct controlElement *  newCtrl;

	newCtrl = (controlElement*) g_malloc0(sizeof(*newCtrl));
	MD_ELEMENT(newCtrl)->type = MD_TYPE_CONTROL;
	newCtrl->control = control;
	newCtrl->value = value;
	return newCtrl;
}

/*
 * Create and initialise a program element.
 *  Arguments:
 *    program   -
 */
struct programElement *
md_program_new(int program)
{
	struct programElement *  prgrm;

	prgrm = (programElement*) g_malloc0(sizeof(*prgrm));
	MD_ELEMENT(prgrm)->type = MD_TYPE_PROGRAM;
	prgrm->program = program;
	return prgrm;
}

/*
 * Create and initialise a keytouch element.
 *  Arguments:
 *    note      -
 *    vel       -
 */
struct keytouchElement *
md_keytouch_new(int note, int vel)
{
	struct keytouchElement *  keyElmt;

	keyElmt = (keytouchElement*) g_malloc0(sizeof(*keyElmt));
	MD_ELEMENT(keyElmt)->type = MD_TYPE_KEYTOUCH;
	keyElmt->note = note;
	keyElmt->velocity = vel;
	return keyElmt;
}

/*
 * Create and initialise a pressure element.
 *  Arguments:
 *    vel       -
 */
struct pressureElement *
md_pressure_new(int vel)
{
	struct pressureElement *  prsrElmnt;

	prsrElmnt = (pressureElement*)g_malloc0(sizeof(*prsrElmnt));
	MD_ELEMENT(prsrElmnt)->type = MD_TYPE_PRESSURE;
	prsrElmnt->velocity = vel;
	return prsrElmnt;
}

/*
 * Create and initialise a pitch element.
 */
struct pitchElement *
md_pitch_new(int val)
{
	struct pitchElement *  pitchElmnt;

	pitchElmnt = (pitchElement*)g_malloc0(sizeof(*pitchElmnt));
	MD_ELEMENT(pitchElmnt)->type = MD_TYPE_PITCH;
	pitchElmnt->pitch = val;
	return pitchElmnt;
}

/*
 * Create and initialise a sysex element.
 */
struct sysexElement *
md_sysex_new(int status, unsigned char *data, int len)
{
	struct sysexElement *  newSysex;

	newSysex = (sysexElement*)g_malloc0(sizeof(*newSysex));
	MD_ELEMENT(newSysex)->type = MD_TYPE_SYSEX;
	newSysex->status = status;
	newSysex->data = data;
	newSysex->length = len;
	return newSysex;
}

/*
 * Create and initialise a meta element.
 */
struct metaElement *
md_meta_new()
{
	struct metaElement *  newMetaElmnt;

	newMetaElmnt = (metaElement*)g_malloc0(sizeof(*newMetaElmnt));
	MD_ELEMENT(newMetaElmnt)->type = MD_TYPE_META;
	return newMetaElmnt;
}

/*
 * Create and initialise a map element.
 */
struct mapElement *
md_map_new()
{
	struct mapElement *  newMapElmnt;

	newMapElmnt = (mapElement*)g_malloc0(sizeof(*newMapElmnt));
	MD_ELEMENT(newMapElmnt)->type = MD_TYPE_MAP;
	return newMapElmnt;
}

/*
 * Create and initialise a keysig element.
 */
struct keysigElement *
md_keysig_new(short key, short minor)
{
	struct keysigElement *  keySigElmnt;

	keySigElmnt = (keysigElement*)g_malloc0(sizeof(*keySigElmnt));
	MD_ELEMENT(keySigElmnt)->type = MD_TYPE_KEYSIG;
	keySigElmnt->key = key;
	keySigElmnt->minor = minor != 0? 1: 0;
	return keySigElmnt;
}

/*
 * Create and initialise a timesig element.
 */
struct timesigElement *
md_timesig_new(short top, short bottom, short clocks, short n32pq)
{
	struct timesigElement *  timeSigElmnt;

	timeSigElmnt = (timesigElement*)g_malloc0(sizeof(*timeSigElmnt));
	MD_ELEMENT(timeSigElmnt)->type = MD_TYPE_TIMESIG;
	timeSigElmnt->top = top;
	timeSigElmnt->bottom = bottom;
	timeSigElmnt->clocks = clocks;
	timeSigElmnt->n32pq = n32pq;
	return timeSigElmnt;
}

/*
 * Create and initialise a tempo element.
 */
struct tempoElement *
md_tempo_new(int m)
{
	struct tempoElement *  tempoElmnt;

	tempoElmnt = (tempoElement*)g_malloc0(sizeof(*tempoElmnt));
	MD_ELEMENT(tempoElmnt)->type = MD_TYPE_TEMPO;
	tempoElmnt->micro_tempo = m;
	return tempoElmnt;
}

/*
 * Create and initialise a text element.
 */
struct textElement *
md_text_new(int type, char *text)
{
	struct textElement *  textElmnt;

	textElmnt = (textElement*)g_malloc0(sizeof(*textElmnt));
	MD_ELEMENT(textElmnt)->type = MD_TYPE_TEXT;
	{
	static char *typenames[] = {
		"",
		"text",
		"copyright",
		"trackname",
		"instrument",
		"lyric",
		"marker",
		"cuepoint",
	};
	textElmnt->type = type;
	textElmnt->name = typenames[type];
	textElmnt->text = text;
	if (text)
		textElmnt->length = strlen(text);
	else
		textElmnt->length = 0;
	}
	return textElmnt;
}

/*
 * Create and initialise a smpteoffset element.
 */
struct smpteoffsetElement *
md_smpteoffset_new(short hours, short minutes, short seconds, short frames,
        short subframes)
{
	struct smpteoffsetElement *  smpteElmnt;

	smpteElmnt = (smpteoffsetElement*)g_malloc0(sizeof(*smpteElmnt));
	MD_ELEMENT(smpteElmnt)->type = MD_TYPE_SMPTEOFFSET;
	smpteElmnt->hours = hours;
	smpteElmnt->minutes = minutes;
	smpteElmnt->seconds = seconds;
	smpteElmnt->frames = frames;
	smpteElmnt->subframes = subframes;
	return smpteElmnt;
}

/*
 * Add an element to a container element.
 */
void
md_add(struct containerElement *c, struct event *e)
{
	g_ptr_array_add(c->elements, e);
}

/*
 * Free a complete element tree.
 */
void
md_free(struct event *el)
{
	struct containerElement *c;
	int  i;

	if (el->type >= MD_CONTAINER_BEGIN)
	{
		c = MD_CONTAINER(el);
		for (i = 0; i < c->elements->len; i++)
		{
			struct event *p = (event*)g_ptr_array_index(c->elements, i);
			md_free(p);
		}
		g_ptr_array_free(c->elements, 1);
	}
	switch (el->type)
	{
	case MD_TYPE_TEXT:
		g_free(MD_TEXT(el)->text);
		break;
	case MD_TYPE_SYSEX:
		g_free(MD_SYSEX(el)->data);
		break;
	}
	g_free(el);

}

/*
 * Check that the given element can be casted to the given type.
 * This is mainly for debugging as mismatches will not happen
 * in proper use. In particular do not use this routine to check
 * types unless you want the program to exit if the check fails.
 *  Arguments:
 *    el        - Element to be cast
 *    type      - type to cast to
 */
struct event *
md_check_cast(struct event *el, int type)
{

	switch (type) {
	case MD_TYPE_CONTAINER:
		if (!iscontainer(el))
			std::cout << "ERROR " << __FILE__ << "@" << __LINE__ << std::endl;
			//except(debugError, "Cast to container from %d", el->type);
		return el;
	case MD_TYPE_ELEMENT:
		/* Anything can be cast to an element */
		if (el->type > 100 || el->type < 0)
			break;	/* Sanity check */
		return el;
	case MD_TYPE_META:
	case MD_TYPE_MAP:
		/* TEMP: this is a parent type */
		return el;
	case MD_TYPE_TRACK:
		if (el->type == MD_TYPE_TEMPOMAP)
			return el;
		break;
	}

	if (type == el->type)
		return el;

	std::cout << "ERROR " << __FILE__ << "@" << __LINE__ << std::endl;
	return NULL;

}

}

#endif
