#ifndef _alsa_midi_helper_
#define _alsa_midi_helper_

#include <alsa/asoundlib.h>

namespace AriaMaestosa
{
    class MidiContext;

namespace PlatformMidiManager
{
void alsa_output_module_setContext(MidiContext* context_arg);

void alsa_output_module_init();
void alsa_output_module_free();


 void allSoundOff();
 void resetAllControllers();
 void stopNoteIfAny();

}
}

#endif
