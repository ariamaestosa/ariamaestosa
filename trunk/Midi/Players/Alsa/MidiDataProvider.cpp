
#ifdef _PMIDI_ALSA

#include "Midi/Players/Alsa/MidiDataProvider.h"
#include <iostream>
#include "Config.h"

namespace AriaMaestosa
{
    enum mode
{
    MODE_FILE,
    MODE_BYTES
};

MidiDataProvider::MidiDataProvider(char* filename)
{
    mode = MODE_FILE;
    fileReader = fopen(filename, "rb");
	if (fileReader == NULL)
	{
        //except(ioError, _("Could not open file %s"), name);
        printf("Could not open file %s", filename);
        assert(false);
	}
}

MidiDataProvider::MidiDataProvider(char* data, int length)
{
    mode = MODE_BYTES;
    MidiDataProvider:: data = data;
    MidiDataProvider::length = length;
    pos = 0;
}

MidiDataProvider::~MidiDataProvider()
{
   // if(mode == MODE_FILE)
   //     fclose(fileReader);

   // if(mode == MODE_BYTES)
        free(data);
}

int MidiDataProvider::read_byte()
{
   // if(mode == MODE_FILE)
   //     return getc(fileReader);

    //if(mode == MODE_BYTES)
    //{
        return (pos<length ? data[pos++] : -1);
   // }
}

void MidiDataProvider::put_back_byte(int c)
{
   // if(mode == MODE_FILE)
   //     ungetc(c, fileReader);

   // if(mode == MODE_BYTES)
        pos--;
}

bool MidiDataProvider::read_bytes(unsigned char* data_arg, int length_arg, size_t count)
{
   // if(mode == MODE_FILE)
   //         return fread(data_arg, length_arg, count, fileReader) == 1;

    //if(mode == MODE_BYTES)
    //{
        if(pos+length_arg > length) return false;

        for(int n=0; n<length_arg; n++)
        {
            data_arg[n] = data[pos+n];
        }
        pos += length_arg;
   // }
    return true;
}

}

#endif
