
/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


// refer to MacPlayerInterface for docs

#ifdef _MAC_QUICKTIME_COREAUDIO
#include "Config.h"

#include "Midi/Players/Mac/CoreAudioNotePlayer.h"

#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph


#include "wx/wx.h"

namespace CoreAudioNotePlayer {

// ==========================================================================================
// ==========================================================================================
class StopNoteTimer : public wxTimer {

public:

StopNoteTimer() : wxTimer(){ }

void Notify()
{
	CoreAudioNotePlayer::stopNote();
	wxTimer::Stop();
}

void start(int duration)
{
	Start(duration);
}
};
// ==========================================================================================
// ==========================================================================================


// This call creates the Graph and the Synth unit...
OSStatus	CreateAUGraph (AUGraph &outGraph, AudioUnit &outSynth)
{
	OSStatus result;
	//create the nodes of the graph
	AUNode synthNode, limiterNode, outNode;

	ComponentDescription cd;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;

	require_noerr (result = NewAUGraph (&outGraph), home);

	cd.componentType = kAudioUnitType_MusicDevice;
	cd.componentSubType = kAudioUnitSubType_DLSSynth;

	require_noerr (result = AUGraphNewNode (outGraph, &cd, 0, NULL, &synthNode), home);

	cd.componentType = kAudioUnitType_Effect;
	cd.componentSubType = kAudioUnitSubType_PeakLimiter;

	require_noerr (result = AUGraphNewNode (outGraph, &cd, 0, NULL, &limiterNode), home);

	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_DefaultOutput;
	require_noerr (result = AUGraphNewNode (outGraph, &cd, 0, NULL, &outNode), home);

	require_noerr (result = AUGraphOpen (outGraph), home);

	require_noerr (result = AUGraphConnectNodeInput (outGraph, synthNode, 0, limiterNode, 0), home);
	require_noerr (result = AUGraphConnectNodeInput (outGraph, limiterNode, 0, outNode, 0), home);

	// ok we're good to go - get the Synth Unit...
	require_noerr (result = AUGraphGetNodeInfo(outGraph, synthNode, 0, 0, 0, &outSynth), home);

home:
		return result;
}

OSStatus PathToFSSpec(const char *filename, FSSpec &outSpec)
{
	FSRef fsRef;
	OSStatus result;
	require_noerr (result = FSPathMakeRef ((const UInt8*)filename, &fsRef, 0), home);
	require_noerr (result = FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, &outSpec, NULL), home);

home:
		return result;
}


// some MIDI constants:
enum {
	kMidiMessage_ControlChange 		= 0xB,
	kMidiMessage_ProgramChange 		= 0xC,
	kMidiMessage_BankMSBControl 	= 0,
	kMidiMessage_BankLSBControl		= 32,
	kMidiMessage_NoteOn 			= 0x9
};

AUGraph graph = 0;
AudioUnit synthUnit;
char* bankPath = 0;
UInt8 midiChannelInUse = 0; //we're using midi channel 1...
bool playing = false;

StopNoteTimer* stopNoteTimer = NULL;

/*
 // set bank
 {
	 jdkmidi::MIDITimedBigMessage m;

	 m.SetTime( 0 );
	 m.SetControlChange( channel, 0, 0 );

	 if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; exit(1); }

	 m.SetTime( 0 );
	 m.SetControlChange( channel, 32, 0 );

	 if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; exit(1); }
 }

 // set instrument
...

 // set track name
...

 // set maximum volume
 {
	 jdkmidi::MIDITimedBigMessage m;

	 m.SetTime( 0 );
	 m.SetControlChange( channel, 7, 127 );

	 if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; exit(1); }
 }

 */

void init()
{

	stopNoteTimer = new StopNoteTimer();

	OSStatus result;

	require_noerr (result = CreateAUGraph (graph, synthUnit), home);

	// initialize and start the graph
	require_noerr (result = AUGraphInitialize (graph), home);

	//set our bank
	require_noerr (result = MusicDeviceMIDIEvent(synthUnit,
												 kMidiMessage_ControlChange << 4 | midiChannelInUse,
												 kMidiMessage_BankMSBControl, 0,
												 0/*sample offset*/), home);

	require_noerr (result = MusicDeviceMIDIEvent(synthUnit,
												 kMidiMessage_ProgramChange << 4 | midiChannelInUse,
												 0/*prog change num*/, 0,
												 0/*sample offset*/), home);

	//CAShow (graph); // prints out the graph so we can see what it looks like...

	require_noerr (result = AUGraphStart (graph), home);


home:

		return;

}

void free()
{
	if(stopNoteTimer != NULL)
	{
		delete stopNoteTimer;
		stopNoteTimer = NULL;
	}

	if (graph)
	{
		AUGraphStop (graph); // stop playback - AUGraphDispose will do that for us but just showing you what to do
		DisposeAUGraph (graph);
	}
}

int lastNote;
int lastChannel;


void playNote(int pitchID, int volume, int duration, int channel, int instrument)
{

	if(playing) stopNote();

	OSStatus result;
	UInt32 noteOnCommand = 	kMidiMessage_NoteOn << 4 | channel;
	UInt32 progamChange = kMidiMessage_ProgramChange << 4 | channel;

	lastNote = pitchID;
	lastChannel = channel;

	// set instrument
	require_noerr (result = MusicDeviceMIDIEvent(synthUnit, progamChange, instrument, 0, 0), home);

	// note on
	require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, pitchID, volume, 0), home);

	stopNoteTimer->start(duration);
	playing = true;
home:

		return;
}

void stopNote()
{
	if(!playing) return;

	OSStatus result;
	UInt32 noteOnCommand = 	kMidiMessage_NoteOn << 4 | lastChannel;

	// note off
	require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, lastNote, 0, 0), home);
	playing = false;
home:

		return;
}

}

#endif
