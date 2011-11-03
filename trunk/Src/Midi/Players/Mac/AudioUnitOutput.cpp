
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


#ifdef _MAC_QUICKTIME_COREAUDIO
#include "Utils.h"

#include "Midi/Players/Mac/AudioUnitOutput.h"
#include "PreferencesData.h"

#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph

#include <wx/timer.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>

using namespace AriaMaestosa;


// This call creates the Graph and the Synth unit...
OSStatus CreateAUGraph (AUGraph &outGraph, AudioUnit &outSynth)
{
    OSStatus result;
    //create the nodes of the graph
    AUNode synthNode, limiterNode, outNode;
    
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    AudioComponentDescription cd;
#else
    ComponentDescription cd;
#endif
    
    cd.componentManufacturer = kAudioUnitManufacturer_Apple;
    cd.componentFlags = 0;
    cd.componentFlagsMask = 0;
    
    result = NewAUGraph (&outGraph);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: NewAUGraph failed\n");
        goto CreateAUGraph_home;
    }
    
    cd.componentType = kAudioUnitType_MusicDevice;
    cd.componentSubType = kAudioUnitSubType_DLSSynth;
    
    result = AUGraphAddNode (outGraph, &cd, &synthNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode failed\n");
        goto CreateAUGraph_home;
    }
    
    cd.componentType = kAudioUnitType_Effect;
    cd.componentSubType = kAudioUnitSubType_PeakLimiter;  
    
    result = AUGraphAddNode (outGraph, &cd, &limiterNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode (limiterNode) failed\n");
        goto CreateAUGraph_home;
    }
    
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;  
    result = AUGraphAddNode (outGraph, &cd, &outNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode (outNode) failed\n");
        goto CreateAUGraph_home;
    }
    
    result = AUGraphOpen (outGraph);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphOpen failed with error code %i (%s - %s)\n",
                (int)result, GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
        goto CreateAUGraph_home;
    }
    
    result = AUGraphConnectNodeInput (outGraph, synthNode, 0, limiterNode, 0);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphConnectNodeInput (synthNode) failed\n");
        goto CreateAUGraph_home;
    }
    
    result = AUGraphConnectNodeInput (outGraph, limiterNode, 0, outNode, 0);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphConnectNodeInput (limiterNode) failed\n");
        goto CreateAUGraph_home;
    }
    
    result = AUGraphNodeInfo(outGraph, synthNode, 0, &outSynth);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphNodeInfo failed\n");
        goto CreateAUGraph_home;
    }
    
    
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
enum
{
    kMidiMessage_ControlChange      = 0xB,
    kMidiMessage_ProgramChange      = 0xC,
    kMidiMessage_BankMSBControl     = 0,
    kMidiMessage_BankLSBControl     = 32,
    kMidiMessage_NoteOn             = 0x9,
    kMidiMessage_PitchBend          = 0xE
};

enum Controllers
{
    kController_BankMSBControl 	= 0,
    kController_BankLSBControl	= 32,
};


// ------------------------------------------------------------------------------------------------------

AUGraph graph = 0;
AudioUnit synthUnit;
char* bankPath = 0;

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::programChange(uint8_t progChangeNum, uint8_t midiChannelInUse)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_ProgramChange << 4 | midiChannelInUse, 
                                                 progChangeNum, 0,
                                                 0 /*sample offset*/), home_programChange);
    return;
    
home_programChange:
    
    fprintf(stderr, "Error in MidiPlayer::programChange : %i (%s - %s)\n",
            (int)result, GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::setBank(uint8_t midiChannelInUse)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_ControlChange << 4 | midiChannelInUse, 
                                                 kController_BankMSBControl, 0,
                                                 0 /*sample offset*/), home_setBank);
    return;
    
home_setBank:
    
    fprintf(stderr, "Error in MidiPlayer::setBank : %i (%s - %s)\n",
            (int)result, GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
}

// ------------------------------------------------------------------------------------------------------

AudioUnitOutput::AudioUnitOutput()
{
    uint8_t midiChannelInUse = 0;
    
    OSStatus result;
    
    require_noerr (result = CreateAUGraph (graph, synthUnit), err1);
    
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
    require_noerr (result = AUGraphInitialize (graph), err2);
    
    for (int n=0; n<15; n++)
    {
        programChange(0, n);
        setBank(n);
    }
    
    //set our bank
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit,
                                                 kMidiMessage_ControlChange << 4 | midiChannelInUse,
                                                 kMidiMessage_BankMSBControl, 0,
                                                 0/*sample offset*/), err3);
    
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit,
                                                 kMidiMessage_ProgramChange << 4 | midiChannelInUse,
                                                 0/*prog change num*/, 0,
                                                 0/*sample offset*/), err4);
    
    //CAShow (graph); // prints out the graph so we can see what it looks like...
    
    require_noerr (result = AUGraphStart (graph), err5);
    return;
    
err1:
    fprintf(stderr, "[PlatformMIDIManager] ERROR: An error has occured when initing the AUDIO UNIT graph (err 1)\n");
    wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
    return;
    
err2:
    fprintf(stderr, "[PlatformMIDIManager] ERROR: An error has occured when initing the AUDIO UNIT graph (err 2)\n");
    wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
    return;
    
err3:
    fprintf(stderr, "[PlatformMIDIManager] ERROR: An error has occured when initing the AUDIO UNIT graph (err 3)\n");
    wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
    return;
    
err4:
    fprintf(stderr, "[PlatformMIDIManager] ERROR: An error has occured when initing the AUDIO UNIT graph (err 4)\n");
    wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
    return;
    
err5:
    fprintf(stderr, "[PlatformMIDIManager] ERROR: An error has occured when initing the AUDIO UNIT graph (err 5)\n");
    wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
    return;
}

// ------------------------------------------------------------------------------------------------------

AudioUnitOutput::~AudioUnitOutput()
{
    if (graph)
    {
        AUGraphStop (graph); // stop playback - AUGraphDispose will do that for us but just showing you what to do
        DisposeAUGraph (graph);
    }
}


// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Sequencer Functions
#endif

void AudioUnitOutput::note_on(const int note, const int volume, const int channel)
{
    OSStatus result;
    UInt32 noteOnCommand = kMidiMessage_NoteOn << 4 | channel;
    
    // note on
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, note, volume, 0), home);
    
home:
    
    return;
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::note_off(const int note, const int channel)
{
    OSStatus result;
    UInt32 noteOnCommand = kMidiMessage_NoteOn << 4 | channel;
    
    // note off
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, note, 0, 0), home);
    
home:
    
    return;
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::prog_change(const int instrument, const int channel)
{
    OSStatus result;
    UInt32 progamChange = kMidiMessage_ProgramChange << 4 | channel;
    
    // set instrument
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, progamChange, instrument, 0, 0), home);
    
home:
    
    return;        
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::controlchange(const int controller, const int value, const int channel)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_ControlChange << 4 | channel, 
                                                 controller, value,
                                                 0 /*sample offset*/), home_setBank);
    return;
    
home_setBank:
    
    fprintf(stderr, "Error in MidiPlayer::au_seq_controlchange\n");
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::pitch_bend(const int value, const int channel)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                 kMidiMessage_PitchBend << 4 | channel, 
                                                 (value & 0x7F), ((value >> 7) & 0x7F),
                                                 0 /*sample offset*/), home_setBank);
    return;
    
home_setBank:
    return;
}


#endif
