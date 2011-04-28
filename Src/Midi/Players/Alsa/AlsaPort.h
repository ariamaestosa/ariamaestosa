#ifndef _alsaport_
#define _alsaport_

#include "Utils.h"
#include <alsa/asoundlib.h>
#include "glib.h"
#include <wx/string.h>
#include "ptr_vector.h"

#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/sysex.h"


namespace AriaMaestosa
{
    class MidiDevice;

    class MidiContext
    {
        ptr_vector<MidiDevice> devices;
        bool timerStarted;

        public:
        LEAK_CHECK();

        MidiDevice* device;

        snd_seq_t* sequencer;
        int queue;
        snd_seq_addr_t address;
        int port;
        snd_seq_port_subscribe_t *subs;
        GArray  *destlist;

        MidiContext();
        ~MidiContext();

        bool openDevice(MidiDevice* device);
        //bool openTimidityDevice();
        void closeDevice();
        bool openDevice(bool launchTimidity);

        bool isPlaying();
        void setPlaying(bool playing);

        void  findDevices ();
        int getDeviceAmount();
        MidiDevice* getDevice(int id);
        MidiDevice* getDevice(int client, int port);

    };

    class MidiDevice
    {
        MidiContext* midiContext;
    public:
        LEAK_CHECK();
        snd_seq_addr_t address;
        int client, port;
        wxString name;

        MidiDevice(MidiContext* context, int client_arg, int port_arg, const char* name_arg);
        bool open();
        void close();

    };

}

#endif
