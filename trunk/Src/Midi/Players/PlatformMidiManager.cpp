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

#include "Midi/Players/PlatformMidiManager.h"
#include "PreferencesData.h"
#include "ptr_vector.h"
#include <wx/intl.h>
#include <wx/msgdlg.h>

#include "RtMidi.h"

using namespace AriaMaestosa;

ptr_vector<PlatformMidiManagerFactory, REF>* g_all_midi_managers = NULL;
PlatformMidiManager* g_manager = NULL;

// ----------------------------------------------------------------------------------------------------------

std::vector<wxString> PlatformMidiManager::getChoices()
{
    std::vector<wxString> out;
    const int amount = g_all_midi_managers->size();
    for (int n=0; n<amount; n++)
    {
        out.push_back(g_all_midi_managers->get(n)->getName());
    }
    return out;
}

// ----------------------------------------------------------------------------------------------------------

PlatformMidiManager* PlatformMidiManager::get()
{
    if (g_all_midi_managers == NULL)
    {
        // FIXME: instead of aborting, fire a no-op driver
        wxMessageBox(_("Bad binary, no MIDI driver was compiled in!"));
        fprintf(stderr, "Bad binary, no MIDI driver was compiled in!");
        exit(1);
    }
    
    if (g_manager == NULL)
    {
        wxString preferredMidiDriver = PreferencesData::getInstance()->getValue(SETTING_ID_MIDI_DRIVER);

        const int count = g_all_midi_managers->size();
        for (int n=0; n<count; n++)
        {
            if (g_all_midi_managers->get(n)->getName() == preferredMidiDriver)
            {
                g_manager = g_all_midi_managers->get(n)->newInstance();
                break;
            }
        }
        
        if (g_manager == NULL)
        {
            // TODO: fix invalid preferences value?
            g_manager = g_all_midi_managers->get(0)->newInstance();
        }
    }
    return g_manager;
}

// ----------------------------------------------------------------------------------------------------------

void PlatformMidiManager::registerManager(PlatformMidiManagerFactory* newManager)
{
    // hack to make sure the vector is always created before using it
    static bool firstTime = true;
    if (firstTime)
    {
        g_all_midi_managers = new ptr_vector<PlatformMidiManagerFactory, REF>();
        firstTime = false;
    }
    
    g_all_midi_managers->push_back(newManager);
}

// ----------------------------------------------------------------------------------------------------------

wxArrayString PlatformMidiManager::getInputChoices()
{
    wxArrayString out;
    RtMidiIn* midiin = new RtMidiIn();
    
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 0)
    {
        std::cout << "No ports available!\n";
        return out;
    }
    
    std::string portName;
    for (unsigned int i=0; i<nPorts; i++)
    {
        try
        {
            portName = midiin->getPortName(i);
        }
        catch (RtError &error)
        {
            error.printMessage();
            continue;
        }
        std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
        out.Add( wxString(portName.c_str(), wxConvUTF8) );
    }
    delete midiin;
    return out;
}

// ----------------------------------------------------------------------------------------------------------

void recordCallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
    unsigned int nBytes = message->size();
    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << deltatime << std::endl;
}

// ----------------------------------------------------------------------------------------------------------

bool PlatformMidiManager::startRecording(wxString outputPort)
{
    RtMidiIn *midiin = new RtMidiIn();
    
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 0)
    {
        wxMessageBox(_("Sorry, no MIDI input port is available"));
        return false;
    }
    
    std::string portName;
    int portId = -1;
    for (unsigned int i=0; i<nPorts; i++)
    {
        try
        {
            portName = midiin->getPortName(i);
        }
        catch (RtError &error)
        {
            error.printMessage();
            continue;
        }
        
        if (outputPort == wxString(portName.c_str(), wxConvUTF8))
        {
            portId = i;
            break;
        }
    }
    
    
    if (portId == -1)
    {
        wxMessageBox( _("Sorry, failed to open the selected MIDI input port") );
        return false;
    }
    
    midiin->openPort( portId );
    
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue.
    midiin->setCallback( &recordCallback );
    
    // Don't ignore sysex, timing, or active sensing messages.
    midiin->ignoreTypes( false, false, false );
    
    /*
    std::cout << "\nReading MIDI input ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);
    delete midiin;
     */
    return true;
}
