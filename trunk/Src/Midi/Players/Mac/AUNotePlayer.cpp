
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

#include "Midi/Players/Mac/AUNotePlayer.h"

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
    
    // The following is copied from: http://developer.apple.com/qa/qa2004/qa1374.html
    static CFStringRef EndpointName(MIDIEndpointRef endpoint, bool isExternal)
    {
        CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
        CFStringRef str;
        
        // begin with the endpoint's name
        str = NULL;
        MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
        if (str != NULL) {
            CFStringAppend(result, str);
            CFRelease(str);
        }
        
        MIDIEntityRef entity = NULL;
        MIDIEndpointGetEntity(endpoint, &entity);
        if (entity == NULL)
            // probably virtual
            return result;
        
        if (CFStringGetLength(result) == 0) {
            // endpoint name has zero length -- try the entity
            str = NULL;
            MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
            if (str != NULL) {
                CFStringAppend(result, str);
                CFRelease(str);
            }
        }
        // now consider the device's name
        MIDIDeviceRef device = NULL;
        MIDIEntityGetDevice(entity, &device);
        if (device == NULL)
            return result;
        
        str = NULL;
        MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
        if (str != NULL) {
            // if an external device has only one entity, throw away
            // the endpoint name and just use the device name
            if (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2) {
                CFRelease(result);
                return str;
            } else {
                // does the entity name already start with the device name?
                // (some drivers do this though they shouldn't)
                // if so, do not prepend
                if (CFStringCompareWithOptions(str /* device name */,
                                               result /* endpoint name */,
                                               CFRangeMake(0, CFStringGetLength(str)), 0) != kCFCompareEqualTo) {
                    // prepend the device name to the entity name
                    if (CFStringGetLength(result) > 0)
                        CFStringInsert(result, 0, CFSTR(" "));
                    CFStringInsert(result, 0, str);
                }
                CFRelease(str);
            }
        }
        return result;
    }
    static CFStringRef ConnectedEndpointName(MIDIEndpointRef endpoint)
    {
        CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
        CFStringRef str;
        OSStatus err;
        int i = 0;
        
        // Does the endpoint have connections?
        CFDataRef connections = NULL;
        int nConnected = 0;
        bool anyStrings = false;
        err = MIDIObjectGetDataProperty(endpoint, kMIDIPropertyConnectionUniqueID, &connections);
        if (connections != NULL) {
            // It has connections, follow them
            // Concatenate the names of all connected devices
            nConnected = CFDataGetLength(connections) / sizeof(MIDIUniqueID);
            if (nConnected) {
                const SInt32 *pid = (SInt32*) CFDataGetBytePtr(connections);
                for (i = 0; i < nConnected; ++i, ++pid) {
                    MIDIUniqueID id = EndianS32_BtoN(*pid);
                    MIDIObjectRef connObject;
                    MIDIObjectType connObjectType;
                    err = MIDIObjectFindByUniqueID(id, &connObject, &connObjectType);
                    if (err == noErr) {
                        if (connObjectType == kMIDIObjectType_ExternalSource  ||
                            connObjectType == kMIDIObjectType_ExternalDestination) {
                            // Connected to an external device's endpoint (10.3 and later).
                            str = EndpointName((OpaqueMIDIEndpoint*)(connObject), true);
                        } else {
                            // Connected to an external device (10.2) (or something else, catch-all)
                            str = NULL;
                            MIDIObjectGetStringProperty(connObject, kMIDIPropertyName, &str);
                        }
                        if (str != NULL) {
                            if (anyStrings)
                                CFStringAppend(result, CFSTR(", "));
                            else anyStrings = true;
                            CFStringAppend(result, str);
                            CFRelease(str);
                        }
                    }
                }
            }
            CFRelease(connections);
        }
        if (anyStrings)
            return result;
        
        // Here, either the endpoint had no connections, or we failed to obtain names for any of them.
        return EndpointName(endpoint, false);
    }
    
    
    std::vector<Destination> g_destinations;
    
    const std::vector<Destination>& getDestinations()
    {
        return g_destinations;
    }
    
    // This call creates the Graph and the Synth unit...
    OSStatus    CreateAUGraph (AUGraph &outGraph, AudioUnit &outSynth)
    {
        // -----------------------------------
        MIDIClientRef	theMidiClient;
        MIDIClientCreate(CFSTR("Play Sequence"), NULL, NULL, &theMidiClient);		
        ItemCount destCount = MIDIGetNumberOfDestinations();
        printf("[AUNotePlayer] %i MIDI destinations\n", (int)destCount);
        for (int n=0; n<(int)destCount; n++)
        {
            MIDIEndpointRef ref = MIDIGetDestination(n);
            CFStringRef nameCStr = ConnectedEndpointName(ref);
            char buffer[256];
            CFStringGetCString(nameCStr, buffer, 256, kCFStringEncodingISOLatin1);
            printf("[AUNotePlayer]   - '%s'\n", buffer);
            
            Destination d;
            d.m_ref = ref;
            d.m_name = buffer;
            g_destinations.push_back(d);
        }
        // require_noerr (result = MusicSequenceSetMIDIEndpoint (sequence, MIDIGetDestination(0)), fail);
        // -----------------------------------
        
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
    
    const int PITCH_BEND_LOWEST = 0;
    const int PITCH_BEND_CENTER = 8192;
    const int PITCH_BEND_HIGHEST = 16383;
    
    // ------------------------------------------------------------------------------------------------------
    
    AUGraph graph = 0;
    AudioUnit synthUnit;
    char* bankPath = 0;
    //uint8_t midiChannelInUse = 0; //we're using midi channel 1...
    bool playing = false;
    
    StopNoteTimer* stopNoteTimer = NULL;
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    
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
    
    // ------------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------------
    
#if 0
#pragma mark -
#pragma mark Sequencer Functions
#endif
    
    void au_seq_note_on(const int note, const int volume, const int channel)
    {
        OSStatus result;
        UInt32 noteOnCommand =        kMidiMessage_NoteOn << 4 | channel;
        
        // note on
        require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, note, volume, 0), home);

    home:
        
        return;
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    void au_seq_note_off(const int note, const int channel)
    {
        OSStatus result;
        UInt32 noteOnCommand = kMidiMessage_NoteOn << 4 | channel;
        
        // note off
        require_noerr (result = MusicDeviceMIDIEvent(synthUnit, noteOnCommand, note, 0, 0), home);

    home:
        
        return;
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    void au_seq_prog_change(const int instrument, const int channel)
    {
        OSStatus result;
        UInt32 progamChange  = kMidiMessage_ProgramChange << 4 | channel;

        // set instrument
        require_noerr (result = MusicDeviceMIDIEvent(synthUnit, progamChange, instrument, 0, 0), home);
        
    home:
        
        return;        
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    void au_seq_controlchange(const int controller, const int value, const int channel)
    {
        OSStatus result;
        require_noerr (result = MusicDeviceMIDIEvent(synthUnit, 
                                                     kMidiMessage_ControlChange << 4 | channel, 
                                                     controller, value,
                                                     0 /*sample offset*/), home_setBank);
        return;
        
    home_setBank:
        
        fprintf(stderr, "Error in MidiPlayer::setBank\n");
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    void au_seq_pitch_bend(const int value, const int channel)
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
    
    // ------------------------------------------------------------------------------------------------------
    
    void au_reset_all_controllers()
    {
        for (int channel=0; channel<16; channel++)
        {
            au_seq_controlchange(0x78 /*120*/ /* all sound off */, 0, channel);
            au_seq_controlchange(0x79 /*121*/ /* reset controllers */, 0, channel);
            au_seq_controlchange(7 /* reset volume */, 127, channel);
            au_seq_controlchange(10 /* reset pan */, 64, channel);
            au_seq_pitch_bend(127, channel);
        }
    }
}

#endif
