#ifndef _midi_data_provider_
#define _midi_data_provider_

#include "Config.h"
#include "stdio.h"

namespace AriaMaestosa
{

class MidiDataProvider
{
    //for file mode
    //FILE* fileReader;

    //for byte mode
    char* data;
    int length, pos;

    int mode;

    DECLARE_LEAK_CHECK();
  public:

    MidiDataProvider(char* filename);
    MidiDataProvider(char* data, int length);
    ~MidiDataProvider();

    int read_byte();
    void put_back_byte(int c);

    bool read_bytes(unsigned char* data, int length, size_t count);

};

}

#endif
