#ifndef _alsa_midi_helper_
#define _alsa_midi_helper_

#include <alsa/asoundlib.h>

namespace AriaMaestosa
{
    class MidiContext;

namespace AlsaNotePlayer
{
void setContext(MidiContext* context_arg);

void init();
void free();
 void allSoundOff();
 void  noteOn (int channel, int note, int velocity, int duration);
 void  noteOff (int channel, int note);
 void  controlChange (int channel, int control, int value);
 void  programChange (int channel, int program);
 void  pitchBend (int channel, int value);
 void  resetAllControllers();
 void stopNoteIfAny();
}
}

#endif
