#include "Midi/Players/Alsa/AlsaPort.h"
#include <iostream>
#include "wx/wx.h"
#include "IO/IOUtils.h"

namespace AriaMaestosa
{



MidiDevice::MidiDevice(MidiContext* context, int client_arg, int port_arg, const char* name_arg)
{
    INIT_LEAK_CHECK();
    client = client_arg;
    port = port_arg;
    name = fromCString(name_arg);
    MidiDevice::midiContext = context;
}

MidiDevice* MidiContext::getDevice(int i)
{
    if(i<0 or i>devices.size()) return NULL;
    return &devices[i];
}

MidiDevice* MidiContext::getDevice(int client, int port)
{
    for(int n=0; n<devices.size(); n++)
    {
        if(devices[n].client == client and devices[n].port == port) return &devices[n];
    }
    return NULL;
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
	//snd_seq_port_subscribe_alloca(&midiContext->subs);
	//snd_seq_port_subscribe_set_sender(midiContext->subs, &midiContext->address);
	//snd_seq_port_subscribe_set_dest(midiContext->subs, &address);

	snd_seq_unsubscribe_port(midiContext->sequencer, midiContext->subs);
	snd_seq_drop_output(midiContext->sequencer);
    snd_seq_free_queue(midiContext->sequencer, midiContext->queue);
    snd_seq_close(midiContext->sequencer);

	//snd_seq_port_subscribe_free(midiContext.subs);
}


#pragma mark -

MidiContext::MidiContext()
{
    INIT_LEAK_CHECK();
    queue = -1;
}

MidiContext::~MidiContext()
{
    /*
    if(queue != -1)
    {
        if(snd_seq_free_queue(sequencer, queue) != 0)
            printf("deleting queue failed\n");
        queue = -1;
    }
*/
    g_array_free(destlist, TRUE);
}

void MidiContext::closeDevice()
{
    device->close();
}

bool MidiContext::askOpenDevice()
{
    findDevices();

    // -------------- ask user to choose a midi port -----------------
    const int deviceAmount = getDeviceAmount();

    if(deviceAmount < 1)
    {
        wxMessageBox( _("No midi port available, aborting.") );
        exit(1);
    }

    wxString choices[deviceAmount];
    MidiDevice* currentDevice;
    for(int n=0; n<deviceAmount; n++)
    {
        currentDevice = getDevice(n);
        choices[n] =    to_wxString(currentDevice->client) +
                        wxString( wxT(":") ) +
                        to_wxString(currentDevice->port) +
                        wxString( wxT(" (") ) +
                        currentDevice->name +
                        wxString( wxT(")") );
    }

    int userChoice = wxGetSingleChoiceIndex(_("Select an ALSA midi port to use for output."),
                                            _("ALSA midi port choice"),
                                            deviceAmount, choices);
    if(userChoice == -1) return false;

    bool success = openDevice(getDevice(userChoice));
    if(success) return true;
    else
    {
        wxMessageBox( _("The selected ALSA device could not be opened.") );
        return askOpenDevice();
    }
    return false;
}

void MidiContext::findDevices()
{
    devices.clear();
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
			devices.push_back( MidiDevice(this, client, port, snd_seq_port_info_get_name(pinfo)) );

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

	// FIXME - what's the use of creating a queue? seqlib does
	//queue = snd_seq_alloc_queue (sequencer);
    //if(queue < 0) printf("Failed to allocate queue\n");

	//snd_seq_set_client_pool_output (midiContext.sequencer, 1024);
	snd_seq_set_client_pool_output (sequencer, 1024);

	destlist = g_array_new(0, 0, sizeof(snd_seq_addr_t));
	// snd_seq_set_client_pool_output(seq_handle, (seq_len<<1) + 4);
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
    if(device == NULL) return false;
    return device->open();
}


}
