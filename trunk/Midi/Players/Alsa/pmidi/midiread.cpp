/*
 *
 * File: midiread.m - Read in a midi file.
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

#include "glib.h"

#include <stdlib.h>
#include "Midi/Players/Alsa/pmidi/elements.h"

#include <iostream>

#include "Midi/Players/Alsa/MidiDataProvider.h"
#include "Midi/Players/Alsa/pmidi/midi.h"

namespace AriaMaestosa
{

/*
 * This structure is used to keep track of the state while
 * reading in a midi file.
 */
struct midistate {
    MidiDataProvider* midiData;
	//FILE *midiData;		/* File being read */
	int  current_time;	/* Current midi time */
	int  port;		/* Midi port number */
	int  device;	/* Midi device number */
	int  track_count;	/* Count of current track */
	int  chunk_size;	/* Size of current chunk */
	int  chunk_count;	/* Count within current chunk */
	GPtrArray *notes;	/* Currently on notes */

	struct tempomapElement *tempo_map;	/* The tempo map */
};

static struct rootElement *read_head(struct midistate *midi_state);
static struct trackElement *read_track(struct midistate *midi_state);
static void handle_status(struct midistate *midi_state, struct trackElement *track,
        int status);
static struct metaElement *handle_meta(struct midistate *midi_state, int type,
        unsigned char *data);
static int read_int(struct midistate *midi_state, int n);
static unsigned char *read_data(struct midistate *midi_state, int length);
static gint32 read_var(struct midistate *midi_state);
static void put_back(struct midistate *midi_state, char c);
static struct event *save_note(struct midistate *midi_state, int note, int vel);
static void finish_note(struct midistate *midi_state, int note, int vel);
static void skip_chunk(struct midistate *midi_state);

/*
 * Read in a midi file from the specified open file pointer, midiData
 * and return an mtree structure tree representing the file.
 *
 *  Arguments:
 *    midiData        - Input file pointer
 */
struct rootElement *
midi_read(MidiDataProvider* midiData)
{
	struct midistate mState;
	struct midistate *midi_state;
	struct rootElement *root;
	struct event *el;
	int  i;

	midi_state = &mState;
	midi_state->midiData = midiData;
	midi_state->tempo_map = md_tempomap_new();
	midi_state->notes = g_ptr_array_new();
	midi_state->port = 0;

	root = read_head(midi_state);
	md_add(MD_CONTAINER(root), NULL); /* Leave room for the tempo map */
	for (i = 0; i < root->tracks; i++)
	{
		el = MD_ELEMENT(read_track(midi_state));

		/* If format 1 then the first track is really the tempo map */
		if (root->format == 1
				&& i == 0
				&& MD_CONTAINER(el)->elements->len == 0)
        {
			/* It will be added after the loop */
			md_free(el);
			continue;
		}

		md_add(MD_CONTAINER(root), el);
	}

	g_ptr_array_index(MD_CONTAINER(root)->elements, 0) = midi_state->tempo_map;
	midi_state->tempo_map = NULL;

	g_ptr_array_free(midi_state->notes, 1);

	return root;
}

/*
 * Read in a midi file from the specified file name.
 *
 *  Arguments:
 *    name      - File name to read
 */
struct rootElement *
midi_read_file(MidiDataProvider& midiData)
{
	//FILE *midiData;
	//MidiDataProvider midiData(name);
	struct rootElement *root;

	//midiData = fopen(name, "rb");
	//if (midiData == NULL)
	//	except(ioError, "Could not open file %s", name);

	root = midi_read(&midiData);

	//fclose(midiData);

	return root;
}

/*
 * Read the header information from a midi file
 *
 *  Arguments:
 *    midi_state       - current midi state
 */
static struct rootElement *
read_head(struct midistate *midi_state)
{
	guint32  magic;
	int  length;
	struct rootElement *root;

	root = md_root_new();

	/* The first word just identifies the file as a midi file */
	magic = read_int(midi_state, 4);
	if (magic != MIDI_HEAD_MAGIC)
		std::cout << "ERROR : Midi data has bad header. " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(formatError, "Bad header (%x), probably not a real midi file", magic);

	/* The header chunk should be 6 bytes, (perhaps longer in the future) */
	length = read_int(midi_state, 4);
	if (length < 6)
		std::cout << "ERROR : Midi data has bad header length. " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(formatError, "Bad header length, probably not a real midi file");

	root->format = read_int(midi_state, 2);
	root->tracks = read_int(midi_state, 2);
	root->time_base = read_int(midi_state, 2);

	/* Should skip any extra bytes, (may not be seekable) */
	while (length > 6)
	{
		length--;
		midi_state->midiData->read_byte();
		//(void) getc(midi_state->midiData);
	}

	return root;
}

/*
 * Read in one track from the file, and return an element tree
 * describing it.
 *
 *  Arguments:
 *    midi_state       - Midi state
 */
static struct trackElement *
read_track(struct midistate *midi_state)
{
	int  status, laststatus;
	int  head;
	int  length;
	int  delta_time;
	struct trackElement *track;
	int  i;

	laststatus = 0;
	head = read_int(midi_state, 4);
	if (head != MIDI_TRACK_MAGIC)
		std::cout << "ERROR : Midi data has bad header length " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(formatError, "Bad track header (%x), probably not a midi file",head);

	length = read_int(midi_state, 4);
	midi_state->chunk_size = length;
	midi_state->chunk_count = 0;	/* nothing read yet */

	track = md_track_new();

	midi_state->current_time = 0;
	while (midi_state->chunk_count < midi_state->chunk_size)
	{

		delta_time = read_var(midi_state);
		midi_state->current_time += delta_time;

		status = read_int(midi_state, 1);
		if ((status & 0x80) == 0)
		{

			/*
			 * This is not a status byte and so running status is being
			 * used.  Re-use the previous status and push back this byte.
			 */
			put_back(midi_state, status);
			status = laststatus;
		}
		else
		{
			laststatus = status;
		}

		handle_status(midi_state, track, status);
	}

  restart:
	for (i = 0; i < midi_state->notes->len; i++)
	{
		struct noteElement *ns;
		ns = (noteElement*)g_ptr_array_index(midi_state->notes, i);
		midi_state->device = MD_ELEMENT(ns)->device_channel;
        printf("Left over note, finishing\n");
		finish_note(midi_state, ns->note, 0);
		goto restart;
	}

	midi_state->track_count++;

	return track;
}

/*
 * Complete the reading of the status byte. The parsed midi
 * command will be added to the specified track .
 *
 *  Arguments:
 *    midi_state       - Current midi file status
 *    track     - Current track
 *    status    - Status byte, ie. current command
 */
static void
handle_status(struct midistate *midi_state, struct trackElement *track, int status)
{
	int  ch;
	int  type;
	int  device;
	int  length;
	short note, vel, control;
	int  val;
	unsigned char *data;
	struct event *el;

	ch = status & 0x0f;
	type = status & 0xf0;

	/*
	 * Do not set the device if the type is 0xf0 as these commands are
	 * not channel specific
	 */
	device = 0;
	if (type != 0xf0)
		device = (midi_state->port<<4) + ch;
	midi_state->device = device;

	el = NULL;

	switch (type) {
	case MIDI_NOTE_OFF:
		note = read_int(midi_state, 1);
		vel = read_int(midi_state, 1);

		finish_note(midi_state, note, vel);
		break;

	case MIDI_NOTE_ON:
		note = read_int(midi_state, 1);
		vel = read_int(midi_state, 1);

		if (vel == 0) {
			/* This is really a note off */
			finish_note(midi_state, note, vel);
		} else {
			/* Save the start, so it can be matched with the note off */
			el = save_note(midi_state, note, vel);
		}
		break;

	case MIDI_KEY_AFTERTOUCH:
		note = read_int(midi_state, 1);
		vel = read_int(midi_state, 1);

		/* new aftertouchElement */
		el = MD_ELEMENT(md_keytouch_new(note, vel));
		break;

	case MIDI_CONTROLER:
		control = read_int(midi_state, 1);
		val = read_int(midi_state, 1);
		el = MD_ELEMENT(md_control_new(control, val));

		break;

	case MIDI_PATCH:
		val = read_int(midi_state, 1);
		el = MD_ELEMENT(md_program_new(val));
		break;

	case MIDI_CHANNEL_AFTERTOUCH:
		val = read_int(midi_state, 1);
		el = MD_ELEMENT(md_pressure_new(val));
		break;
	case MIDI_PITCH_WHEEL:
		val = read_int(midi_state, 1);
		val |= read_int(midi_state, 1) << 7;
		val -= 0x2000;	/* Center it around zero */
		el = MD_ELEMENT(md_pitch_new(val));
		break;

	/* Now for all the non-channel specific ones */
	case 0xf0:
		/* Deal with the end of track event first */
		if (ch == 0x0f) {
			type = read_int(midi_state, 1);
			if (type == 0x2f) {
				/* End of track - skip to end of real track */
				track->final_time = midi_state->current_time;
				skip_chunk(midi_state);
				return;
			}
		}

		/* Get the length of the following data */
		length = read_var(midi_state);
		data = read_data(midi_state, length);
		if (ch == 0x0f) {
			el = (struct event *)handle_meta(midi_state, type, data);
		} else {
			el = (struct event *)md_sysex_new(status, data, length);
		}
		break;
	default:
		std::cout << "ERROR : bad status type " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(formatError, "Bad status type 0x%x", type);
		/*NOTREACHED*/
	}

	if (el != NULL) {
		el->element_time = midi_state->current_time;
		el->device_channel = device;

		md_add(MD_CONTAINER(track), el);
	}
}

/*
 * Do extra handling of meta events. We want to save time
 * signature and key for use elsewhere, for example. This
 * routine create the correct type of class and returns it.
 *
 *  Arguments:
 *    midi_state       - The midi file state
 *    type      - The meta event type
 *    data      - The data for the event
 */
static struct metaElement *
handle_meta(struct midistate *midi_state, int type, unsigned char *data)
{
	struct metaElement *el = NULL;
	struct mapElement *map = NULL;
	int  micro_tempo;

	switch (type) {
	case MIDI_META_SEQUENCE:
		break;
	case MIDI_META_TEXT:
	case MIDI_META_COPYRIGHT:
	case MIDI_META_TRACKNAME:
	case MIDI_META_INSTRUMENT:
	case MIDI_META_LYRIC:
	case MIDI_META_MARKER:
	case MIDI_META_CUE:
		/* Text based events */
		el = MD_META(md_text_new(type, (char*)data));
		break;
	case MIDI_META_CHANNEL:
		break;
	case MIDI_META_PORT:
		midi_state->port = data[0];
		g_free(data);
		break;
	case MIDI_META_EOT:
		break;
	case MIDI_META_TEMPO:
		micro_tempo = ((data[0]<<16) & 0xff0000)
			+ ((data[1]<<8) & 0xff00) + (data[2] & 0xff);
		map = MD_MAP(md_tempo_new(micro_tempo));
		g_free(data);
		break;
	case MIDI_META_SMPTE_OFFSET:
		el = MD_META(md_smpteoffset_new(data[0], data[1], data[2], data[3],
			data[4]));
		break;
	case MIDI_META_TIME:
		map = MD_MAP(md_timesig_new(data[0], 1<<data[1],
			data[2], data[3]));
		g_free(data);
		break;
	case MIDI_META_KEY:
		map = MD_MAP(md_keysig_new(data[0], (data[1]==1)? 1: 0));
		g_free(data);
		break;
	case MIDI_META_PROP:
		/* Proprietry sequencer specific event */
		/* Just throw it out */
		break;
	default:
		g_warning("Meta event %d not implemented\n", type);
		break;
	}

	/* If this affected the tempo map then add it */
	if (map) {
		MD_ELEMENT(map)->element_time = midi_state->current_time;
		md_add(MD_CONTAINER(midi_state->tempo_map), MD_ELEMENT(map));
	}

	return el;
}

/*
 * Reads an interger from the midi file. The number of bytes to
 * be read is specified in n .
 *
 *  Arguments:
 *    midi_state       - Midi file state
 *    n         - Number of bytes to read
 */
static int
read_int(struct midistate *midi_state, int n)
{
	int  val;
	int  c;
	int  i;

	val = 0;

	for (i = 0; i < n; i++)
	{
		val <<= 8;
		c = midi_state->midiData->read_byte();
		//c = getc(midi_state->midiData);
		midi_state->chunk_count++;
		if (c == -1)
			std::cout << "ERROR : Unexpected end of midi data. " << __FILE__ << "@" << __LINE__ << std::endl;
			//except(formatError, "Unexpected end of file");

		val |= c;
	}

	return val;
}

/*
 * Read in a specified amount of data from the file. The return
 * is allocated data which must be freed by the caller. An extra
 * null byte is appended for the sake of text events.
 *  Arguments:
 *    midi_state       - Midifile state
 *    length    - Length of data to read
 */
static unsigned char *
read_data(struct midistate *midi_state, int length)
{

	unsigned char* data = (unsigned char*) g_malloc(length+1);

	if (length == 0) {
		data[0] = 0;
		return data;
	}

	//if (fread(data, length, 1, midi_state->midiData) == 1) {
	if (midi_state->midiData->read_bytes(data, length, 1))
	{
		midi_state->chunk_count += length;
		data[length] = '\0';
		return data;
	} else {
		std::cout << "ERROR : unexpected end of midi data " << __FILE__ << "@" << __LINE__ << std::endl;
		//except(formatError, "Unexpected end of file");
		/*NOTREACHED*/
	}
	return NULL;
}

/*
 * Read a variable length integer from the midifile and return
 * it. Returns an int32 so cannot really deal with more than
 * four bytes.
 *
 *  Arguments:
 *    midi_state       - Midi file state
 */
static gint32
read_var(struct midistate *midi_state)
{
	int  val;
	int  c;

	val = 0;
	do {
	    c = midi_state->midiData->read_byte();
		//c = getc(midi_state->midiData);
		midi_state->chunk_count++;
		if (c == -1)
			std::cout << "ERROR : Unexpected end of data " << __FILE__ << "@" << __LINE__ << std::endl;
			//except(formatError, "Unexpected end of file");
		val <<= 7;
		val |= (c & 0x7f);
	} while ((c & 0x80) == 0x80);

	return val;
}

/*
 * Push back a character. Have to also keep track of the
 * chunk_count.
 *  Arguments:
 *    midi_state       - Midi input state
 *    c         - Character to push back
 */
static void
put_back(struct midistate *midi_state, char c)
{
    midi_state->midiData->put_back_byte(c);
	//ungetc(c, midi_state->midiData);
	midi_state->chunk_count--;
}

/*
 * Save the initial note-on message. This will later be paired
 * with a note off message. We have to keep track of channel and
 * device that the note-on is for so that it can be correctly
 * matched.
 *
 *  Arguments:
 *    midi_state       - Midi file state
 *    note      - Note number
 *    vel       - Velocity
 */
static struct event *
save_note(struct midistate *midi_state, int note, int vel)
{
	struct noteElement *n;

	/* Create a new note and set its length to -1
	 * this will be filled in later, when the note-off arrives
	 */
	n = md_note_new(note, vel, -1);

	/* Save it so that we match up with the note off */
	g_ptr_array_add(midi_state->notes, n);

	return MD_ELEMENT(n);
}

/*
 * Called when a note off is seen. Finds the corresponding note
 * on and constructs a note element.
 *
 *  Arguments:
 *    midi_state       - Midi file state
 *    note      - Note number
 *    vel       - Note velocity
 */
static void
finish_note(struct midistate *midi_state, int note, int vel)
{
	int  i;
	GPtrArray *notes;
	struct noteElement *n;
	int  len;

	notes = midi_state->notes;
	n = NULL;
	for (i = notes->len-1; i >= 0; i--) {
		n = (noteElement*)g_ptr_array_index(notes, i);
		if (n->note == note && MD_ELEMENT(n)->device_channel == midi_state->device) {
			len = midi_state->current_time - MD_ELEMENT(n)->element_time;
			n->offvel = vel;
			n->length = len;
			if (n->length < 0) {
				printf("Len neg: midi_state->time%d, s->time%d, note=%d, s.vel%d\n",
					midi_state->current_time, MD_ELEMENT(n)->element_time,
					note, n->vel);
				n->length = 0;
			}
			g_ptr_array_remove_index_fast(notes, i);
			break;
		}
	}
}

/*
 * Skip to the end of the chunk. Note that the input may not be
 * seekable, so we just read bytes until at the end of the chunk.
 *
 *  Arguments:
 *    midi_state       - Midi file state
 */
static void
skip_chunk(struct midistate *midi_state)
{
	while (midi_state->chunk_count < midi_state->chunk_size)
		(void) read_int(midi_state, 1);
}

}

#endif
