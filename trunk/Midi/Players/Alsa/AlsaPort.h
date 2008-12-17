#ifndef _alsaport_
#define _alsaport_

#include "Config.h"
#include <alsa/asoundlib.h>
#include <vector>
#include "glib.h"
#include "wx/wx.h"

namespace AriaMaestosa
{
    class MidiDevice;

    class MidiContext
    {
        std::vector<MidiDevice> devices;
        bool timerStarted;

        public:
        LEAK_CHECK(MidiContext);

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
        void closeDevice();
        bool askOpenDevice();

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
        LEAK_CHECK(MidiDevice);
        snd_seq_addr_t address;
        int client, port;
        wxString name;

        MidiDevice(MidiContext* context, int client_arg, int port_arg, const char* name_arg);
        bool open();
        void close();

    };

}

#endif
