#ifdef _ALSA

#include <glib.h>
#include "AriaCore.h"
#include "Midi/Players/Alsa/AlsaPort.h"
#include <iostream>
#include <wx/wx.h>
#include "IO/IOUtils.h"
#include "PreferencesData.h"

namespace AriaMaestosa
{

MidiDevice::MidiDevice(MidiContext* context, int client_arg, int port_arg, const char* name_arg) :
	name(name_arg, wxConvUTF8)
{
    client = client_arg;
    port = port_arg;
    MidiDevice::midiContext = context;
}

bool MidiDevice::open()
{
    address.client = client;
    address.port = port;

    snd_seq_port_subscribe_alloca(&midiContext->subs);

    snd_seq_port_subscribe_set_sender(midiContext->subs, &midiContext->address);
    snd_seq_port_subscribe_set_dest(midiContext->subs, &address);

    const int success = snd_seq_subscribe_port(midiContext->sequencer, midiContext->subs);
    if(success != 0)
    {
        wxMessageBox( _("Failed to open ALSA port.") );
        return false;
    }
    return true;
}


void MidiDevice::close()
{
    snd_seq_unsubscribe_port(midiContext->sequencer, midiContext->subs);
    snd_seq_drop_output(midiContext->sequencer);
    snd_seq_free_queue(midiContext->sequencer, midiContext->queue);
    snd_seq_close(midiContext->sequencer);
}


#if 0
#pragma mark -
#endif

MidiContext::MidiContext()
{
    queue = -1;
}

MidiContext::~MidiContext()
{
    g_array_free(destlist, TRUE);
}

void runTimidity()
{
    // start by checking if TiMidity is already running
    FILE * command_output;
    char output[128];
    int amount_read = 1;
    std::string full_output;


    command_output = popen("ps -A", "r");
    if (command_output != NULL)
    {
        while (amount_read > 0)
        {
            amount_read = fread(output, 1, 127, command_output);
            if (amount_read <= 0)
            {
                break;
            }
            else
            {
                output[amount_read] = '\0';
                full_output += output;
                //std::cout << output << std::endl;
            }
        } // end while
    } // end if
    
    pclose(command_output);
    
    const bool timidityIsRunning = full_output.find("timidity") != std::string::npos;
    if (timidityIsRunning)
    {
        std::cout << "TiMidity appears to be already running.\n";
        return;
    }
    
    //  -c '/home/mmg/Desktop/timidity-synth/default/timidity.cfg'
    // -EFreverb=0
    std::cout << "Launching TiMidity ALSA deamon\n";
    wxString cmd("timidity -iA -Os", wxConvUTF8);
    wxExecute(cmd);
    wxMilliSleep(500); // let the timidity deamon some time to start
}

void MidiContext::closeDevice()
{
    device->close();
}

bool MidiContext::openDevice(bool launchTimidity)
{
    if (launchTimidity)
    {
        runTimidity();
    }
    
    findDevices();
    
    // -------------- ask user to choose a midi port -----------------
    const int deviceAmount = getDeviceAmount();

    if (deviceAmount < 1)
    {
        wxMessageBox( _("No midi port found, no sound will be available.") );
        return false;
    }

    bool success = false;
    
        
    MidiDevice* d = NULL;
    
    wxString port = PreferencesData::getInstance()->getValue(SETTING_ID_MIDI_OUTPUT);
    if (port == wxString(wxT("default")))
    {
        if (devices.size() > 0)
        {
            d = getDevice(0); // default is to take the first device
            PreferencesData::getInstance()->setValue(SETTING_ID_MIDI_OUTPUT, d->name);
        }
    }
    else
    {
        wxString a_str = port.BeforeFirst(wxT(':'));
        wxString b_str = port.BeforeFirst(wxT(' ')).AfterLast(wxT(':'));
        
        long a, b;
        if (!a_str.ToLong(&a) || !b_str.ToLong(&b))
        {
            wxMessageBox( wxT("Invalid midi output selected") );
            fprintf(stderr, "Invalid midi output selected  <%s:%s>\n", (const char*)a_str.utf8_str(),
                    (const char*)b_str.utf8_str());
            return false;
        }
        
        d = getDevice(a,b);
    }
    
    if (d == NULL)
    {
        success = false;
    }
    else
    {
        success = openDevice(d);
    }
    
    if (!success)
    {
        wxMessageBox( _("The selected ALSA device could not be opened.") );
    }
    
    return success;
}

MidiDevice* MidiContext::getDevice(int i)
{
    if(i<0 or i>(int)devices.size()) return NULL;
    return &devices[i];
}

MidiDevice* MidiContext::getDevice(int client, int port)
{
    for (int n=0; n<(int)devices.size(); n++)
    {
        if(devices[n].client == client and devices[n].port == port) return &devices[n];
    }
    return NULL;
}

void MidiContext::findDevices()
{
    devices.clearAndDeleteAll();
    snd_seq_client_info_t* clientInfo;

    if (snd_seq_open(&sequencer, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0)
    {
        return;
    }

    snd_seq_client_info_alloca(&clientInfo);
    snd_seq_client_info_set_client(clientInfo, -1);

    // iterate through clients
    while (snd_seq_query_next_client(sequencer, clientInfo) >= 0)
    {
        snd_seq_port_info_t* pinfo;

        snd_seq_port_info_alloca(&pinfo);
        snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(clientInfo));
        snd_seq_port_info_set_port(pinfo, -1);

        // and now through ports
        while (snd_seq_query_next_port(sequencer, pinfo) >= 0)
        {
            unsigned int capability = snd_seq_port_info_get_capability(pinfo);

            if ((capability & SND_SEQ_PORT_CAP_SUBS_WRITE) == 0)
            {
                continue;
            }

            int client  = (snd_seq_port_info_get_addr(pinfo))->client;
            int port    = (snd_seq_port_info_get_addr(pinfo))->port;

            std::cout << client << ":" << port << " --> " << snd_seq_port_info_get_name(pinfo) << std::endl;
            devices.push_back( new MidiDevice(this, client, port, snd_seq_port_info_get_name(pinfo)) );

        }

    }

    if (snd_seq_set_client_name(sequencer, "Aria") < 0)
    {
        return;
    }

    //midiContext.address.port = snd_seq_create_simple_port(midiContext.sequencer, "Aria Port 0", SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
    address.port =  snd_seq_create_simple_port(sequencer, "Aria Port 0",
                     SND_SEQ_PORT_CAP_WRITE |
                     SND_SEQ_PORT_CAP_SUBS_WRITE |
                     SND_SEQ_PORT_CAP_READ,
                     SND_SEQ_PORT_TYPE_APPLICATION);
                     //SND_SEQ_PORT_TYPE_MIDI_GENERIC);

    address.client = snd_seq_client_id (sequencer);
    snd_seq_set_client_pool_output (sequencer, 1024);

    destlist = g_array_new(0, 0, sizeof(snd_seq_addr_t));
}

int MidiContext::getDeviceAmount()
{
    return devices.size();
}

// 'timerStarted' is not used by this class
// it's just a conveniant place to keep this info
bool MidiContext::isPlaying()
{
    return timerStarted;
}
void MidiContext::setPlaying(bool playing)
{
    timerStarted = playing;
}

bool MidiContext::openDevice(MidiDevice* device)
{
    MidiContext::device = device;
    if (device == NULL) return false;
    return device->open();
}


}

#endif
