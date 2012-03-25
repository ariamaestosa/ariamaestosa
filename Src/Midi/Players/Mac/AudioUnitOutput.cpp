
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

#include "Midi/Players/Mac/CAStreamBasicDescription.h"
#include "Midi/Players/Mac/AudioUnitOutput.h"
#include "Midi/Players/Mac/AUOutputBL.h"

#include "PreferencesData.h"

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreServices/CoreServices.h> //for file stuff
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h> //for AUGraph
#include <CoreMIDI/CoreMIDI.h>

#include <wx/timer.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>

using namespace AriaMaestosa;

#if 0
#pragma mark -
#pragma mark Render to file
#endif

// Code by Apple, from http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html
static void str2OSType (const char * inString, OSType &outType)
{
	if (inString == NULL) {
		outType = 0;
		return;
	}
	
	size_t len = strlen(inString);
	if (len <= 4) {
		char workingString[5];
		
		workingString[4] = 0;
		workingString[0] = workingString[1] = workingString[2] = workingString[3] = ' ';
		memcpy (workingString, inString, strlen(inString));
		outType = 	*(workingString + 0) <<	24	|
        *(workingString + 1) <<	16	|
        *(workingString + 2) <<	8	|
        *(workingString + 3);
		return;
	}
    
	if (len <= 8) {
		if (sscanf (inString, "%lx", &outType) == 0) {
			printf ("* * Bad conversion for OSType\n"); 
			exit(1);
		}
		return;
	}
	printf ("* * Bad conversion for OSType\n"); 
	exit(1);
}


// Code by Apple, from http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html
bool WriteOutputFile (const char*       outputFilePath, 
                      OSType			dataFormat, 
                      Float64			srate, 
                      MusicTimeStamp	sequenceLength, 
                      bool              shouldPrint,
                      AUGraph			inGraph,
                      UInt32			numFrames,
                      MusicPlayer		player)
{
    // delete existing output  file
    
    if (wxFileExists(wxString(outputFilePath, wxConvUTF8)))
    {
        wxRemoveFile(wxString(outputFilePath, wxConvUTF8));
    }
    
	OSStatus result = 0;
	UInt32 size;
    
	CAStreamBasicDescription outputFormat;
	outputFormat.mChannelsPerFrame = 2;
	outputFormat.mSampleRate = srate;
	outputFormat.mFormatID = dataFormat;
	
	AudioFileTypeID destFileType = kAudioFileAIFFType;
	
	if (dataFormat == kAudioFormatLinearPCM) {
		outputFormat.mBytesPerPacket = outputFormat.mChannelsPerFrame * 2;
		outputFormat.mFramesPerPacket = 1;
		outputFormat.mBytesPerFrame = outputFormat.mBytesPerPacket;
		outputFormat.mBitsPerChannel = 16;
		
		if (destFileType == kAudioFileWAVEType)
			outputFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger
            | kLinearPCMFormatFlagIsPacked;
		else
			outputFormat.mFormatFlags = kLinearPCMFormatFlagIsBigEndian
            | kLinearPCMFormatFlagIsSignedInteger
            | kLinearPCMFormatFlagIsPacked;
	} else {
		// use AudioFormat API to fill out the rest.
		size = sizeof(outputFormat);
		result = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outputFormat);
        if (result != 0)
        {
            fprintf(stderr, "AudioFormatGetProperty Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        }
	}
    
	//if (shouldPrint) {
	//	printf ("Writing to file: %s with format:\n* ", outputFilePath);
		outputFormat.Print();
	//}
	
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*)outputFilePath,
                                                           strlen(outputFilePath), false);
    
	ExtAudioFileRef outfile;
	result = ExtAudioFileCreateWithURL(url, destFileType, &outputFormat, NULL, 0, &outfile);
	CFRelease (url);
	
    if (result != 0)
    {
        fprintf(stderr, "ExtAudioFileCreateWithURL Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        return false;
    }
    
	AudioUnit outputUnit;
	UInt32 nodeCount;
    result = AUGraphGetNodeCount (inGraph, &nodeCount);
    if (result != 0)
    {
        fprintf(stderr, "AUGraphGetNodeCount Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        return false;
    }
    
    if (nodeCount == 0) printf("[CoreAudioOutput] WARNING : 0 nodes??\n");
    
	for (UInt32 i = 0; i < nodeCount; ++i) 
	{
		AUNode node;
		result = AUGraphGetIndNode(inGraph, i, &node);
        if (result != 0)
        {
            fprintf(stderr, "AUGraphGetIndNode Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
            return false;
        }
        
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        AudioComponentDescription desc;
#else
        ComponentDescription desc;
#endif
		result = AUGraphNodeInfo(inGraph, node, &desc, NULL);
        if (result != 0)
        {
            fprintf(stderr, "AUGraphNodeInfo Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
            return false;
        }
        
		if (desc.componentType == kAudioUnitType_Output) 
		{
			result = AUGraphNodeInfo(inGraph, node, 0, &outputUnit);
            if (result != 0)
            {
                fprintf(stderr, "AUGraphNodeInfo Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
                return false;
            }
            
			break;
		}
	}
        
	{
		CAStreamBasicDescription clientFormat = CAStreamBasicDescription();
        result = AudioUnitGetProperty(outputUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Output, 0,
                                      &clientFormat, &size);
        if (result != 0)
        {
            fprintf(stderr, "AudioUnitGetProperty Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
            return false;
        }
    
        
		size = sizeof(clientFormat);
		result = ExtAudioFileSetProperty(outfile, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat);
        if (result != 0)
        {
            fprintf(stderr, "ExtAudioFileSetProperty Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
            return false;
        }
        
        printf("Everything is set up for export, will start the loop. Saving to <%s>\n",
               outputFilePath);
        
		int percentage = 0;
		{
			MusicTimeStamp currentTime;
			AUOutputBL outputBuffer (clientFormat, numFrames);
			AudioTimeStamp tStamp;
			memset (&tStamp, 0, sizeof(AudioTimeStamp));
			tStamp.mFlags = kAudioTimeStampSampleTimeValid;
			do {
				outputBuffer.Prepare();
				AudioUnitRenderActionFlags actionFlags = 0;
				result = AudioUnitRender (outputUnit, &actionFlags, &tStamp, 0, numFrames, outputBuffer.ABL());
                if (result != 0)
                {
                    fprintf(stderr, "AudioUnitRender Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
                    return false;
                }
    
				tStamp.mSampleTime += numFrames;
				
				result = ExtAudioFileWrite(outfile, numFrames, outputBuffer.ABL());
                if (result != 0)
                {
                    fprintf(stderr, "ExtAudioFileWrite Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
                    return false;
                }
                
                
				result = MusicPlayerGetTime (player, &currentTime);
                if (result != 0)
                {
                    fprintf(stderr, "MusicPlayerGetTime Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
                    return false;
                }
                
                if ((int)currentTime % 20 == 0)
                {
                    int new_percentage = (int)round((float)currentTime/(float)sequenceLength*100.0f);
                    if (new_percentage > percentage)
                    {
                        printf("  --> %i%% done\n", new_percentage);
                        percentage = new_percentage;
                    }
                }
                
			} while (currentTime < sequenceLength);
		}
	}
	
    // close
	result = ExtAudioFileDispose(outfile);
    if (result != 0)
    {
        fprintf(stderr, "ExtAudioFileDispose Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        return false;
    }
    
    return true;
}

// Code by Apple, from http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html
OSStatus SetUpGraph (AUGraph &inGraph, UInt32 numFrames, Float64 &sampleRate, bool isOffline)
{
	OSStatus result = noErr;
	AudioUnit outputUnit = 0;
	AUNode outputNode;
	
	// the frame size is the I/O size to the device
	// the device is going to run at a sample rate it is set at
	// so, when we set this, we also have to set the max frames for the graph nodes
	UInt32 nodeCount;
	result = AUGraphGetNodeCount (inGraph, &nodeCount);
    if (result != 0)
    {
        fprintf(stderr, "AUGraphGetNodeCount Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
    }
    
    
	for (int i = 0; i < (int)nodeCount; ++i) 
	{
		AUNode node;
		result = AUGraphGetIndNode(inGraph, i, &node);
        if (result != 0)
        {
            fprintf(stderr, "AUGraphGetIndNode Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        }
        
        
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        AudioComponentDescription desc;
#else
        ComponentDescription desc;
#endif
		AudioUnit unit;
		result = AUGraphNodeInfo(inGraph, node, &desc, &unit);
        if (result != 0)
        {
            fprintf(stderr, "Error %i at %s:%i\n", (int)result, __FILE__, __LINE__);
        }
        
		
		if (desc.componentType == kAudioUnitType_Output) 
		{
			if (outputUnit == 0) {
				outputUnit = unit;
				require_noerr (result = AUGraphNodeInfo(inGraph, node, 0, &outputUnit), home);
				
				if (!isOffline) {
					// these two properties are only applicable if its a device we're playing too
					require_noerr (result = AudioUnitSetProperty (outputUnit, 
                                                                     kAudioDevicePropertyBufferFrameSize, 
                                                                     kAudioUnitScope_Output, 0,
                                                                     &numFrames, sizeof(numFrames)), home);
                    
                    /*
					require_noerr (result = AudioUnitAddPropertyListener (outputUnit, 
                                                                             kAudioDeviceProcessorOverload, 
                                                                             OverlaodListenerProc, 0), home);
                    */
                    
					// if we're rendering to the device, then we render at its sample rate
					UInt32 theSize;
					theSize = sizeof(sampleRate);
					
					require_noerr (result = AudioUnitGetProperty (outputUnit,
                                                                     kAudioUnitProperty_SampleRate,
                                                                     kAudioUnitScope_Output, 0,
                                                                     &sampleRate, &theSize), home);
				} else {
                    // remove device output node and add generic output
					require_noerr (result = AUGraphRemoveNode (inGraph, node), home);
					desc.componentSubType = kAudioUnitSubType_GenericOutput;
					require_noerr (result = AUGraphAddNode (inGraph, &desc, &node), home);
					require_noerr (result = AUGraphNodeInfo(inGraph, node, NULL, &unit), home);
					outputUnit = unit;
					outputNode = node;
					
					// we render the output offline at the desired sample rate
					require_noerr (result = AudioUnitSetProperty (outputUnit,
                                                                     kAudioUnitProperty_SampleRate,
                                                                     kAudioUnitScope_Output, 0,
                                                                     &sampleRate, sizeof(sampleRate)), home);
				}
				// ok, lets start the loop again now and do it all...
				i = -1;
			}
		}
		else
		{
            // we only have to do this on the output side
            // as the graph's connection mgmt will propogate this down.
			if (outputUnit) {	
                // reconnect up to the output unit if we're offline
				if (isOffline && desc.componentType != kAudioUnitType_MusicDevice) {
					require_noerr (result = AUGraphConnectNodeInput (inGraph, node, 0, outputNode, 0), home);
				}
				
				require_noerr (result = AudioUnitSetProperty (unit,
                                                                 kAudioUnitProperty_SampleRate,
                                                                 kAudioUnitScope_Output, 0,
                                                                 &sampleRate, sizeof(sampleRate)), home);
                
                
			}
		}
		require_noerr (result = AudioUnitSetProperty (unit, kAudioUnitProperty_MaximumFramesPerSlice,
                                                         kAudioUnitScope_Global, 0,
                                                         &numFrames, sizeof(numFrames)), home);
	}
	
home:
	return result;
}


// Code by Apple, from http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html

OSStatus LoadSMF(const char* data, int dataSize, MusicSequence& sequence, MusicSequenceLoadFlags loadFlags)
{
	OSStatus result = noErr;
	
	result = NewMusicSequence(&sequence);
	if (result != 0)
    {
        fprintf(stderr, "Error %i at NewMusicSequence\n", (int)result);
        return result;
    }
    	
    CFDataRef cfdata = CFDataCreate(kCFAllocatorDefault, (const UInt8*)data, dataSize);
	result = MusicSequenceFileLoadData (sequence, cfdata, 0, loadFlags);
    if (result != 0)
    {
        fprintf(stderr, "Error %i at MusicSequenceFileLoad\n", (int)result);
        return result;
    }
    
    return 0;
}

// Code by Apple, from http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html
OSStatus GetSynthFromGraph (AUGraph& inGraph, AudioUnit& outSynth)
{	
	UInt32 nodeCount;
	OSStatus result = noErr;
	require_noerr (result = AUGraphGetNodeCount (inGraph, &nodeCount), fail);
	
	for (UInt32 i = 0; i < nodeCount; ++i) 
	{
		AUNode node;
		require_noerr (result = AUGraphGetIndNode(inGraph, i, &node), fail);
        
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        AudioComponentDescription desc;
#else
        ComponentDescription desc;
#endif
		require_noerr (result = AUGraphNodeInfo(inGraph, node, &desc, 0), fail);
		
		if (desc.componentType == kAudioUnitType_MusicDevice) 
		{
			require_noerr (result = AUGraphNodeInfo(inGraph, node, 0, &outSynth), fail);
			return noErr;
		}
	}
	
fail:		// didn't find the synth AU
    fprintf(stderr, "Error in GetSynthFromGraph\n");
	return -1;
}



bool AudioUnitOutput::outputToDisk(const char* outputFilePath,
                                   const char* data,
                                   const int data_size,
                                   wxArrayString& errorMessages)
{
    OSStatus result;
    
    AUGraph graph = 0;
    AudioUnit theSynth = 0;
    MusicSequence sequence;
    Float32 maxCPULoad = .8;

// const char* GetMacOSStatusErrorString(OSStatus err);
// const char* GetMacOSStatusCommentString(OSStatus err);
#define CHECK_RETURN_CODE(fn) if (result != 0) { \
    wxString s = wxString::Format(fn " failed with error code %i", (int)result);\
    fprintf(stderr, "%s\n", (const char*)s.utf8_str()); \
    errorMessages.Add(s); \
    return false; \
    }

    result = LoadSMF(data, data_size, sequence, 0 /* flags */);
    CHECK_RETURN_CODE("LoadSMF");
    
    result = MusicSequenceGetAUGraph (sequence, &graph);
    CHECK_RETURN_CODE("MusicSequenceGetAUGraph");
    
    result = AUGraphOpen (graph);     
    CHECK_RETURN_CODE("AUGraphOpen");
    
    result = GetSynthFromGraph (graph, theSynth);
    CHECK_RETURN_CODE("GetSynthFromGraph");
    
    result = AudioUnitSetProperty (theSynth,
                                   kAudioUnitProperty_CPULoad,
                                   kAudioUnitScope_Global, 0,
                                   &maxCPULoad, sizeof(maxCPULoad));
    CHECK_RETURN_CODE("AudioUnitSetProperty");
    
    // if the user supplies a sound bank, we'll set that before we initialize and start playing
    if (m_custom_sound_font != NULL) 
    {
        printf("setting soundfont <%s>\n", m_custom_sound_font);
        FSRef fsRef;
        result = FSPathMakeRef ((const UInt8*)m_custom_sound_font, &fsRef, 0);
        CHECK_RETURN_CODE("FSPathMakeRef");

        result = AudioUnitSetProperty (theSynth,
                                       kMusicDeviceProperty_SoundBankFSRef,
                                       kAudioUnitScope_Global, 0,
                                       &fsRef, sizeof(fsRef));
        CHECK_RETURN_CODE("AudioUnitSetProperty[kMusicDeviceProperty_SoundBankFSRef]");
    }
    
    // need to tell synth that is going to render a file.
    UInt32 value = 1;
    result = AudioUnitSetProperty(theSynth,
                                  kAudioUnitProperty_OfflineRender,
                                  kAudioUnitScope_Global, 0,
                                  &value, sizeof(value));
    CHECK_RETURN_CODE("AudioUnitSetProperty[kAudioUnitProperty_OfflineRender]");
    
    UInt32 numFrames = 512;
    Float64 sample_rate = 44000;

    result = SetUpGraph (graph, numFrames, sample_rate, (outputFilePath != NULL));
    CHECK_RETURN_CODE("SetUpGraph");

    result = AUGraphInitialize (graph);
    CHECK_RETURN_CODE("AUGraphInitialize");
    
    MusicPlayer player;
    result = NewMusicPlayer (&player);
    CHECK_RETURN_CODE("NewMusicPlayer");
    
    result = MusicPlayerSetSequence (player, sequence);
    CHECK_RETURN_CODE("MusicPlayerSetSequence");
    
    // figure out sequence length
    UInt32 ntracks;
    result = MusicSequenceGetTrackCount (sequence, &ntracks);
    CHECK_RETURN_CODE("MusicSequenceGetTrackCount");
    
    MusicTimeStamp sequenceLength = 0;
    
    for (UInt32 i = 0; i < ntracks; ++i)
    {
        MusicTrack track;
        MusicTimeStamp trackLength;
        UInt32 propsize = sizeof(MusicTimeStamp);
        result = MusicSequenceGetIndTrack(sequence, i, &track);
        CHECK_RETURN_CODE("MusicSequenceGetIndTrack");

        result = MusicTrackGetProperty(track, kSequenceTrackProperty_TrackLength,
                                                         &trackLength, &propsize);
        CHECK_RETURN_CODE("MusicTrackGetProperty[kSequenceTrackProperty_TrackLength]");
       
        if (trackLength > sequenceLength)
            sequenceLength = trackLength;
        
        /*
        if (!trackSet.empty() && (trackSet.find(i) == trackSet.end()))
        {
            Boolean mute = true;
            require_noerr (result = MusicTrackSetProperty(track, kSequenceTrackProperty_MuteStatus, &mute, sizeof(mute)), fail);
        }
        */
    }
	// now I'm going to add 8 beats on the end for the reverb/long releases to tail off...
    sequenceLength += 8;
    
    //result = MusicPlayerSetTime (player, startTime);
    
    //startRunningTime = CAHostTimeBase::GetCurrentTime ();
     
    result = MusicPlayerStart(player);
    CHECK_RETURN_CODE("MusicPlayerStart");

    OSType dataFormat = 0;
    str2OSType("lpcm", dataFormat); //0; //kAudioFormatLinearPCM;

    bool success = WriteOutputFile(outputFilePath, dataFormat, sample_rate, sequenceLength,
                                   false /* print */, graph, numFrames, player);
    
    if (not success)
    {
        fprintf(stderr, "[AudioUnitOutput] ERROR: WriteOutputFile returned false\n");
        return false;
    }
    
    result = MusicPlayerStop(player);
    if (result != 0)
    {
        fprintf(stderr, "MusicPlayerStop failed with error code %i\n", (int)result);
        // ignore errors in cleanup
        return true;
    }
    
    result = DisposeMusicPlayer(player);
    if (result != 0)
    {
        fprintf(stderr, "DisposeMusicPlayer failed with error code %i\n", (int)result);
        // ignore errors in cleanup
        return true;
    }
    
    result = DisposeMusicSequence(sequence);
    if (result != 0)
    {
        fprintf(stderr, "DisposeMusicSequence failed with error code %i\n", (int)result);
        // ignore errors in cleanup
        return true;
    }
    
    result = DisposeMusicSequence(sequence);
    if (result != 0)
    {
        fprintf(stderr, "DisposeMusicSequence failed with error code %i\n", (int)result);
        // ignore errors in cleanup
        return true;
    }
    
    return true;
}

#if 0
#pragma mark -
#endif

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
        return result;
    }
    
    cd.componentType = kAudioUnitType_MusicDevice;
    cd.componentSubType = kAudioUnitSubType_DLSSynth;
    
    result = AUGraphAddNode (outGraph, &cd, &synthNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode failed\n");
        return result;
    }
    
    cd.componentType = kAudioUnitType_Effect;
    cd.componentSubType = kAudioUnitSubType_PeakLimiter;  
    
    result = AUGraphAddNode(outGraph, &cd, &limiterNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode (limiterNode) failed\n");
        return result;
    }
    
    cd.componentType = kAudioUnitType_Output;
    cd.componentSubType = kAudioUnitSubType_DefaultOutput;  
    result = AUGraphAddNode (outGraph, &cd, &outNode);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphAddNode (outNode) failed\n");
        return result;
    }
    
    result = AUGraphOpen(outGraph);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphOpen failed with error code %i (%s - %s)\n",
                (int)result, GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
        return result;
    }
    
    result = AUGraphConnectNodeInput(outGraph, synthNode, 0, limiterNode, 0);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphConnectNodeInput (synthNode) failed\n");
        return result;
    }
    
    result = AUGraphConnectNodeInput (outGraph, limiterNode, 0, outNode, 0);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphConnectNodeInput (limiterNode) failed\n");
        return result;
    }
    
    result = AUGraphNodeInfo(outGraph, synthNode, 0, &outSynth);
    if (result != 0)
    {
        fprintf(stderr, "[PlatformMIDIManager] ERROR: AUGraphNodeInfo failed\n");
        return result;
    }
    
    return result;
}

OSStatus PathToFSSpec(const char *filename, FSSpec &outSpec)
{
    FSRef fsRef;
    OSStatus result;
    result = FSPathMakeRef ((const UInt8*)filename, &fsRef, 0);
    if (result != 0)
    {
        fprintf(stderr, "FSPathMakeRef failed\n");
        return result;
    }
    
    result = FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL, NULL, &outSpec, NULL);
    if (result != 0)
    {
        fprintf(stderr, "FSGetCatalogInfo failed\n");
        return result;
    }
    
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

void AudioUnitOutput::programChange(uint8_t progChangeNum, uint8_t midiChannelInUse)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, 
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
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, 
                                                 kMidiMessage_ControlChange << 4 | midiChannelInUse, 
                                                 kController_BankMSBControl, 0,
                                                 0 /*sample offset*/), home_setBank);
    return;
    
home_setBank:
    
    fprintf(stderr, "Error in MidiPlayer::setBank : %i (%s - %s)\n",
            (int)result, GetMacOSStatusErrorString(result), GetMacOSStatusCommentString(result));
}

// ------------------------------------------------------------------------------------------------------

AudioUnitOutput::AudioUnitOutput(const char* custom_sound_font)
{
    m_graph = 0;
    
    m_custom_sound_font = custom_sound_font;
    
    uint8_t midiChannelInUse = 0;

    OSStatus result;
    
    result = CreateAUGraph(m_graph, m_synth_unit);
    if (result != 0)
    {
        fprintf(stderr, "CreateAUGraph ERROR %i\n", (int)result);
        wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
        return;
    }

    // if the user supplies a sound bank, we'll set that before we initialize and start playing
    if (custom_sound_font != NULL) 
    {
        FSRef fsRef;
        result = FSPathMakeRef ((const UInt8*)custom_sound_font, &fsRef, 0);
        if (result != 0)
        {
            wxMessageBox( _("Sorry, failed to set custom soundfont.") );
            fprintf(stderr, "Failed to set custom sound font, location 1. Error code is %i\n", (int)result);
        }
        else
        {
            result = AudioUnitSetProperty (m_synth_unit,
                                           kMusicDeviceProperty_SoundBankFSRef,
                                           kAudioUnitScope_Global, 0,
                                           &fsRef, sizeof(fsRef));
            if (result != 0)
            {
                wxMessageBox( _("Sorry, failed to set custom soundfont.") );
                fprintf(stderr, "Failed to set custom sound font, location 2. Error code is %i\n", (int)result);
            }
        }
    }
        
    // initialize and start the graph
    result = AUGraphInitialize(m_graph);
    if (result != 0)
    {
        fprintf(stderr, "AUGraphInitialize ERROR %i\n", (int)result);
        wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
        return;
    }
    
    for (int n=0; n<15; n++)
    {
        programChange(0, n);
        setBank(n);
    }
    
    //set our bank
    result = MusicDeviceMIDIEvent(m_synth_unit,
                                  kMidiMessage_ControlChange << 4 | midiChannelInUse,
                                  kMidiMessage_BankMSBControl, 0,
                                  0/*sample offset*/);
    if (result != 0)
    {
        fprintf(stderr, "MusicDeviceMIDIEvent ERROR %i\n", (int)result);
        wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
        return;
    }
    
    result = MusicDeviceMIDIEvent(m_synth_unit,
                                  kMidiMessage_ProgramChange << 4 | midiChannelInUse,
                                  0/*prog change num*/, 0,
                                  0/*sample offset*/);
    if (result != 0)
    {
        fprintf(stderr, "MusicDeviceMIDIEvent ERROR %i\n", (int)result);
        wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
        return;
    }
    
    //CAShow (graph); // prints out the graph so we can see what it looks like...
    
    result = AUGraphStart(m_graph);
    if (result != 0)
    {
        fprintf(stderr, "AUGraphStart ERROR %i\n", (int)result);
        wxMessageBox( _("An error has occurred when connecting to the MIDI output device with AudioUnit; audio playback may be unavailable") );
        return;
    }
}

// ------------------------------------------------------------------------------------------------------

AudioUnitOutput::~AudioUnitOutput()
{
    if (m_graph)
    {
        AUGraphStop(m_graph);
        DisposeAUGraph(m_graph);
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
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, noteOnCommand, note, volume, 0), home);
    
home:
    
    return;
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::note_off(const int note, const int channel)
{
    OSStatus result;
    UInt32 noteOnCommand = kMidiMessage_NoteOn << 4 | channel;
    
    // note off
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, noteOnCommand, note, 0, 0), home);
    
home:
    
    return;
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::prog_change(const int instrument, const int channel)
{
    OSStatus result;
    UInt32 progamChange = kMidiMessage_ProgramChange << 4 | channel;
    
    // set instrument
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, progamChange, instrument, 0, 0), home);
    
home:
    
    return;        
}

// ------------------------------------------------------------------------------------------------------

void AudioUnitOutput::controlchange(const int controller, const int value, const int channel)
{
    OSStatus result;
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, 
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
    require_noerr (result = MusicDeviceMIDIEvent(m_synth_unit, 
                                                 kMidiMessage_PitchBend << 4 | channel, 
                                                 (value & 0x7F), ((value >> 7) & 0x7F),
                                                 0 /*sample offset*/), home_setBank);
    return;
    
home_setBank:
    return;
}


#endif
