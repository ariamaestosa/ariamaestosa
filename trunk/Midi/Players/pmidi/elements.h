/*
 *
 * File: elements.h
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
 *
 *
 */

#include <stdlib.h>
#include "glib.h"

namespace AriaMaestosa
{


#define MD_CONTAINER_BEGIN  50  /* Begining of container types */

/*
 * The basic element type.  All elements have a time associated with
 * them and most use the device and channel field.
 */
struct event {
	short	type;			/* Element type */
	guint32 element_time;	/* Time for this element */
	short device_channel;	/* Device/channel for this element */
};
#define MD_ELEMENT(e) \
	((struct event *)md_check_cast((struct event *)(e), MD_TYPE_ELEMENT))


struct containerElement {
	struct event parent;
	GPtrArray *elements;	/* List of elements */
};
#define MD_CONTAINER(e) \
	((struct containerElement *)md_check_cast((struct event *)(e), MD_TYPE_CONTAINER))


struct rootElement {
	struct containerElement parent;
	short  format;	/* Midi format */
	short  tracks;	/* Number of tracks */
	short  time_base;	/* Time base value */
};
#define MD_ROOT(e) \
	((struct rootElement *)md_check_cast((struct event *)(e), MD_TYPE_ROOT))


struct trackElement {
	struct containerElement parent;
	guint32 final_time;
};
#define MD_TRACK(e) \
	((struct trackElement *)md_check_cast((struct event *)(e), MD_TYPE_TRACK))


struct tempomapElement {
	struct containerElement parent;
};
#define MD_TEMPOMAP(e) \
	((struct tempomapElement *)md_check_cast((struct event *)(e), MD_TYPE_TEMPOMAP))


struct noteElement {
	struct event parent;
	short  note;
	short  vel;
	int    length;
	short  offvel;	/* Note Off velocity */
};
#define MD_NOTE(e) \
	((struct noteElement *)md_check_cast((struct event *)(e), MD_TYPE_NOTE))


struct partElement {
	struct containerElement parent;
	guint32 final_time;
};
#define MD_PART(e) \
	((struct partElement *)md_check_cast((struct event *)(e), MD_TYPE_PART))


struct controlElement {
	struct event parent;
	short  control;	/* Controller number */
	short  value;	/* Controller value */
};
#define MD_CONTROL(e) \
	((struct controlElement *)md_check_cast((struct event *)(e), MD_TYPE_CONTROL))


struct programElement {
	struct event parent;
	int  program;	/* Program number */
};
#define MD_PROGRAM(e) \
	((struct programElement *)md_check_cast((struct event *)(e), MD_TYPE_PROGRAM))


struct keytouchElement {
	struct event parent;
	int  note;
	int  velocity;
};
#define MD_KEYTOUCH(e) \
	((struct keytouchElement *)md_check_cast((struct event *)(e), MD_TYPE_KEYTOUCH))


struct pressureElement {
	struct event parent;
	int  velocity;
};
#define MD_PRESSURE(e) \
	((struct pressureElement *)md_check_cast((struct event *)(e), MD_TYPE_PRESSURE))


struct pitchElement {
	struct event parent;
	int  pitch;
};
#define MD_PITCH(e) \
	((struct pitchElement *)md_check_cast((struct event *)(e), MD_TYPE_PITCH))


struct sysexElement {
	struct event parent;
	int  status;
	unsigned char *data;
	int  length;
};
#define MD_SYSEX(e) \
	((struct sysexElement *)md_check_cast((struct event *)(e), MD_TYPE_SYSEX))


struct metaElement {
	struct event parent;
};
#define MD_META(e) \
	((struct metaElement *)md_check_cast((struct event *)(e), MD_TYPE_META))


struct mapElement {
	struct metaElement parent;
};
#define MD_MAP(e) \
	((struct mapElement *)md_check_cast((struct event *)(e), MD_TYPE_MAP))


struct keysigElement {
	struct mapElement parent;
	char key;		/* Key signature */
	char minor;		/* Is this a minor key or not */
};
#define MD_KEYSIG(e) \
	((struct keysigElement *)md_check_cast((struct event *)(e), MD_TYPE_KEYSIG))


struct timesigElement {
	struct mapElement parent;
	short top;		/* 'top' of timesignature */
	short bottom;	/* 'bottom' of timesignature */
	short clocks;	/* Can't remember what this is */
	short n32pq;	/* Thirtysecond notes per quarter */
};
#define MD_TIMESIG(e) \
	((struct timesigElement *)md_check_cast((struRct event *)(e), MD_TYPE_TIMESIG))


struct tempoElement {
	struct mapElement parent;
	int  micro_tempo;	/* The tempo in microsec per quarter note */
};
#define MD_TEMPO(e) \
	((struct tempoElement *)md_check_cast((struct event *)(e), MD_TYPE_TEMPO))


struct textElement {
	struct event parent;
	int  type;	/* Type of text (lyric, copyright etc) */
	char *name;	/* Type as text */
	char *text; /* actual text */
	int  length; /* length of the text (including a null?) */
};
#define MD_TEXT(e) \
	((struct textElement *)md_check_cast((struct event *)(e), MD_TYPE_TEXT))


struct smpteoffsetElement {
	struct event parent;
	short  hours;
	short  minutes;
	short  seconds;
	short  frames;
	short  subframes;
};
#define MD_SMPTEOFFSET(e) \
	((struct smpteoffsetElement *)md_check_cast((struct event *)(e), MD_TYPE_SMPTEOFFSET))


struct event *md_element_new();
struct containerElement *md_container_new();
struct rootElement *md_root_new(void);
struct trackElement *md_track_new(void);
struct tempomapElement *md_tempomap_new();
struct noteElement *md_note_new(short note, short vel, int length);
struct partElement *md_part_new(void);
struct controlElement *md_control_new(short control, short value);
struct programElement *md_program_new(int program);
struct keytouchElement *md_keytouch_new(int note, int vel);
struct pressureElement *md_pressure_new(int vel);
struct pitchElement *md_pitch_new(int val);
struct sysexElement *md_sysex_new(int status, unsigned char *data, int len);
struct metaElement *md_meta_new();
struct mapElement *md_map_new();
struct keysigElement *md_keysig_new(short key, short minor);
struct timesigElement *md_timesig_new(short top, short bottom, short clocks,
        short n32pq);
struct tempoElement *md_tempo_new(int m);
struct textElement *md_text_new(int type, char *text);
struct smpteoffsetElement *md_smpteoffset_new(short hours, short minutes,
        short seconds, short frames, short subframes);
void md_add(struct containerElement *c, struct event *e);
void md_free(struct event *el);
struct event *md_check_cast(struct event *el, int type);


/* Defines for types */
#define MD_TYPE_PART (0 + MD_CONTAINER_BEGIN)
#define MD_TYPE_ROOT (1 + MD_CONTAINER_BEGIN)
#define MD_TYPE_KEYTOUCH 2
#define MD_TYPE_TEXT 3
#define MD_TYPE_PITCH 4
#define MD_TYPE_PROGRAM 5
#define MD_TYPE_META 6
#define MD_TYPE_PRESSURE 7
#define MD_TYPE_NOTE 8
#define MD_TYPE_ELEMENT 9
#define MD_TYPE_SMPTEOFFSET 10
#define MD_TYPE_TEMPO 11
#define MD_TYPE_TEMPOMAP (12 + MD_CONTAINER_BEGIN)
#define MD_TYPE_SYSEX 13
#define MD_TYPE_TRACK (14 + MD_CONTAINER_BEGIN)
#define MD_TYPE_KEYSIG 15
#define MD_TYPE_TIMESIG 16
#define MD_TYPE_CONTAINER 17
#define MD_TYPE_MAP 18
#define MD_TYPE_CONTROL 19

}
