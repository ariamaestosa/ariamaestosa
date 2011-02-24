
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

// maybe worth looking at http://developer.apple.com/documentation/QuickTime/RM/MusicAndAudio/qtma/C-Chapter/chapter_1000_section_4.html
// since my code is deprecated

#ifdef _MAC_QUICKTIME_COREAUDIO
#include "Utils.h"

#include "Midi/Players/Mac/CoreAudioNotePlayer.h"

#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph


#include <wx/timer.h>

namespace CoreAudioNotePlayer
{

// ==========================================================================================
// ==========================================================================================
class StopNoteTimer : public wxTimer
{

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
OSStatus    CreateAUGraph (AUGraph &outGraph, AudioUnit &outSynth)
{
    OSStatus result;
    //create the nodes of the graph
    AUNode synthNode, limiterNode, outNode;
    
    ComponentDescription cd;
    cd.componentManufacturer = kAudioUnitManufacturer_Apple;
    cd.componentFlags = 0;
    cd.componentFlagsMask = 0;
    
    require_noerr (result = NewAUGraph (&outGraph), CreateAUGraph_home);
    
    cd.componentType = kAudioUnitType_MusicDevice;
    cd.componentSubType = kAudioUnitSubType_DLSSynth;
    
    require_noerr (result = AUGraphAddNode (outGraph, &cd, &synthNode), CreateAUGraph_home);
    
    cd.componentType = kAudioUnitType_Effect;
    cd.componentSubType = kAudioUnitSubType_PeakLimiter;  
    
    require_noerr (result = AUGraphAddNode (outGraph, &cd, &limiterNode), CreateAUGraph_home);
    
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;  
    require_noerr (result = AUGraphAddNode (outGraph, &cd, &outNode), CreateAUGraph_home);
    
    require_noerr (result = AUGraphOpen (outGraph), CreateAUGraph_home);
    
    require_noerr (result = AUGraphConnectNodeInput (outGraph, synthNode, 0, limiterNode, 0), CreateAUGraph_home);
    require_noerr (result = AUGraphConnectNodeInput (outGraph, limiterNode, 0, outNode, 0), CreateAUGraph_home);
    
    // ok we're good to go - get the Synth Unit...
    require_noerr (result = AUGraphNodeInfo(outGraph, synthNode, 0, &outSynth), CreateAUGraph_home);
    
CreateAUGraph_home:
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
    kMidiMessage_ControlChange      = 0xB,
    kMidiMessage_ProgramChange      = 0xC,
    kMidiMessage_BankMSBControl     = 0,
    kMidiMessage_BankLSBControl     = 32,
    kMidiMessage_NoteOn             = 0x9
};
    
enum Controllers
{
    kController_BankMSBControl 	= 0,
    kController_BankLSBControl	= 32,
};
    

AUGraph graph = 0;
AudioUnit synthUnit;
char* bankPath = 0;
//uint8_t midiChannelInUse = 0; //we're using midi channel 1...
bool playing = false;

StopNoteTimer* stopNoteTimer = NULL;

void programChange(uint8_t progChangeNum, uint8_t midiChannelInUse)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_ProgramChange << 4 | midiChannelInUse, 
                                                 progChangeNum, 0,
                                                 0 /*sample offset*/), home_programChange);
    return;
home_programChange:
    fprintf(stderr, "Error in MidiPlayer::programChange\n");
}

void setBank(uint8_t midiChannelInUse)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_ControlChange << 4 | midiChannelInUse, 
                                                 kController_BankMSBControl, 0,
                                                 0 /*sample offset*/), home_setBank);
    return;
home_setBank:
    fprintf(stderr, "Error in MidiPlayer::setBank\n");
}
    
void init()
{
    uint8_t midiChannelInUse = 0;
    stopNoteTimer = new StopNoteTimer();

    OSStatus result;

    require_noerr (result = CreateAUGraph (graph, synthUnit), home);

    /*
    // if the user supplies a sound bank, we'll set that before we initialize and start playing
    if (bankPath) 
    {
        FSRef fsRef;
        require_noerr (result = FSPathMakeRef ((const UInt8*)bankPath, &fsRef, 0), ctor_home);
        
        printf ("Setting Sound Bank:%s\n", bankPath);
        
        require_noerr (result = AudioUnitSetProperty (synthUnit,
                                                      kMusicDeviceProperty_SoundBankFSRef,
                                                      kAudioUnitScope_Global, 0,
                                                      &fsRef, sizeof(fsRef)), ctor_home);
        
    }
    */
    
    // initialize and start the graph
    require_noerr (result = AUGraphInitialize (graph), home);

    for (int n=0; n<15; n++)
    {
        programChange(0, n);
        setBank(n);
    }
    
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
    return;

home:
    fprintf(stderr, "An error seems to have occured when initing the AUDIO UNIT graph\n");
    return;

}

void free()
{
    if (stopNoteTimer != NULL)
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

    if (playing) stopNote();

    OSStatus result;
    UInt32 noteOnCommand =        kMidiMessage_NoteOn << 4 | channel;
    UInt32 progamChange  = kMidiMessage_ProgramChange << 4 | channel;

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
    if (not playing) return;

    OSStatus result;
    UInt32 noteOnCommand =     kMidiMessage_NoteOn << 4 | lastChannel;

    // note off
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, lastNote, 0, 0), home);
    playing = false;
home:

        return;
}
    
}

#endif
