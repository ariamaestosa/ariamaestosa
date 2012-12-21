#ifndef _alsaport_
#define _alsaport_

#include "Utils.h"
#include <alsa/asoundlib.h>
#include "glib.h"
#include <wx/string.h>
#include "ptr_vector.h"

#include "jdksmidi/world.h"
#include "jdksmidi/track.h"
#include "jdksmidi/multitrack.h"
#include "jdksmidi/filereadmultitrack.h"
#include "jdksmidi/fileread.h"
#include "jdksmidi/fileshow.h"
#include "jdksmidi/filewritemultitrack.h"
#include "jdksmidi/msg.h"
#include "jdksmidi/sysex.h"


namespace AriaMaestosa
{
    class MidiDevice;

    /**
      * @ingroup midi.players
      *
      * Represents an ALSA midi context
      */
    class MidiContext
    {
        ptr_vector<MidiDevice> devices;
        bool timerStarted;
        
        bool isExeRunning(const wxString& command);
        void runSoftSynth(const wxString& soundfontPath);
        void setDevice(MidiDevice** d, int index);

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
        //bool openTimidityDevice(); // obsoleted
        void closeDevice();
        bool openDevice(bool launchSoftSynth);

        bool isPlaying();
        void setPlaying(bool playing);

        void  findDevices ();
        int getDeviceAmount();
        MidiDevice* getDevice(int id);
        MidiDevice* getDevice(int client, int port);

    };

    /**
      * @ingroup midi.players
      *
      * Represents an ALSA midi device
      */
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
        wxString getFullName();
        
    };

}

#endif
