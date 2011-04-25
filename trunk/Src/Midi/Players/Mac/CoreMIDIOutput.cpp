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

#include "CoreMIDIOutput.h"
#include <wx/utils.h>

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


std::vector<CoreMidiOutput::Destination> g_destinations;

const std::vector<CoreMidiOutput::Destination>& CoreMidiOutput::getDestinations()
{
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
    
    return g_destinations;
}

CoreMidiOutput::CoreMidiOutput()
{    
    OSStatus result = MIDIOutputPortCreate(m_client, 
                                           CFSTR("myport"), 
                                           &m_port);
    if (result != 0) fprintf(stderr, "MIDIInputPortCreate failed!!\n");
    
    /*
    MIDITimeStamp timestamp = 0;   // 0 will mean play now. 
    Byte buffer[1024];             // storage space for MIDI Packets (max 65536)
    MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
    MIDIPacket *currentpacket = MIDIPacketListInit(packetlist);
    
    const int MESSAGESIZE = 3;
    
    printf("==== Will send notes ====\n");
    
    for (int note=30; note<80; note++)
    {
        Byte noteon[MESSAGESIZE] = {0x90, note, 90};
        currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), 
                                          currentpacket, timestamp, MESSAGESIZE, noteon);
        
        result = MIDISend(port, MIDIGetDestination(0), packetlist);
        if (result != 0) fprintf(stderr, "MIDISend failed!!\n");
        
        wxMilliSleep(250);
        
        currentpacket = MIDIPacketListInit(packetlist);
        Byte noteoff[MESSAGESIZE] = {0x90, note, 0};
        currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), 
                                          currentpacket, timestamp, MESSAGESIZE, noteoff);
        
        
        MIDISend(port, MIDIGetDestination(0), packetlist);
        if (result != 0) fprintf(stderr, "MIDISend failed!!\n");
    }
    
    printf("Done");
     */
}

CoreMidiOutput::~CoreMidiOutput()
{
    // TODO: clean up
}

void CoreMidiOutput::note_on(const int note, const int volume, const int channel)
{
    MIDITimeStamp timestamp = 0;   // 0 will mean play now. 
    Byte buffer[1024];             // storage space for MIDI Packets (max 65536)
    MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
    MIDIPacket *currentpacket = MIDIPacketListInit(packetlist);
    
    const int MESSAGESIZE = 3;

    Byte noteon[MESSAGESIZE] = {0x90 | channel, note, 90};
    currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), 
                                      currentpacket, timestamp, MESSAGESIZE, noteon);
    
    OSStatus result = MIDISend(m_port, MIDIGetDestination(0), packetlist);
    if (result != 0) fprintf(stderr, "MIDISend failed!!\n");
    
     
}

void CoreMidiOutput::note_off(const int note, const int channel)
{
    MIDITimeStamp timestamp = 0;   // 0 will mean play now. 
    Byte buffer[1024];             // storage space for MIDI Packets (max 65536)
    MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
    MIDIPacket *currentpacket = MIDIPacketListInit(packetlist);
    
    const int MESSAGESIZE = 3;

    currentpacket = MIDIPacketListInit(packetlist);
    Byte noteoff[MESSAGESIZE] = {0x90 | channel, note, 0};
    currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), 
                                      currentpacket, timestamp, MESSAGESIZE, noteoff);
    
    
    OSStatus result = MIDISend(m_port, MIDIGetDestination(0), packetlist);
    if (result != 0) fprintf(stderr, "MIDISend failed!!\n");

}

void CoreMidiOutput::prog_change(const int instrument, const int channel)
{
}

void CoreMidiOutput::controlchange(const int controller, const int value, const int channel)
{
}

void CoreMidiOutput::pitch_bend(const int value, const int channel)
{
}

void CoreMidiOutput::reset_all_controllers()
{
}
