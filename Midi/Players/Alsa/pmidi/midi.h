/*
 *
 * File: midi.h
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
 *
 *
 */
/*
 * Midi status byte values.
 */
#include "Midi/Players/Alsa/MidiDataProvider.h"

//extern "C"
//{

namespace AriaMaestosa
{

const int MIDI_NOTE_OFF	= 0x80;
const int MIDI_NOTE_ON	= 0x90;
const int MIDI_KEY_AFTERTOUCH	= 0xa0;
const int MIDI_CONTROLER	= 0xb0;
const int MIDI_PATCH	= 0xc0;
const int MIDI_CHANNEL_AFTERTOUCH	= 0xd0;
const int MIDI_PITCH_WHEEL	= 0xe0;
const int MIDI_SYSEX =	0xf0;
const int MIDI_META	= 0xff;

/* Meta event defines */
const int MIDI_META_SEQUENCE  = 0;
/* The text type meta events */
const int MIDI_META_TEXT       = 1;
const int MIDI_META_COPYRIGHT  = 2;
const int MIDI_META_TRACKNAME  = 3;
const int MIDI_META_INSTRUMENT = 4;
const int MIDI_META_LYRIC      = 5;
const int MIDI_META_MARKER     = 6;
const int MIDI_META_CUE        = 7;
/* More meta events */
const int MIDI_META_CHANNEL      = 0x20;
const int MIDI_META_PORT         = 0x21;
const int MIDI_META_EOT          = 0x2f;
const int MIDI_META_TEMPO        = 0x51;
const int MIDI_META_SMPTE_OFFSET = 0x54;
const int MIDI_META_TIME         = 0x58;
const int MIDI_META_KEY          = 0x59;
const int MIDI_META_PROP         = 0x7f;

/** The maximum of the midi defined text types */
const int MIDI_MAX_TEXT_TYPE   = 7;

const long MIDI_HEAD_MAGIC = 0x4d546864;
const long MIDI_TRACK_MAGIC  = 0x4d54726b;

struct rootElement* midi_read(MidiDataProvider* midiData);
struct rootElement* midi_read_file(MidiDataProvider& midiData);

}

//}

