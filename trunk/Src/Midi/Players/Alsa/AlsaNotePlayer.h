#ifndef _alsa_midi_helper_
#define _alsa_midi_helper_

#include <alsa/asoundlib.h>

namespace AriaMaestosa
{
class MidiContext;

    /** global flag that is set when ALSA midi could not be initialized */ // FIXME: globals are ugly
    extern bool sound_available;
    
    /**
      * @ingroup midi.players
      *
      * Contains ALSA sequencer functions
      */
    namespace AlsaPlayerStuff
    {
        void alsa_output_module_setContext(MidiContext* context_arg);

        void alsa_output_module_init();
        void alsa_output_module_free();


        void allSoundOff();
        void resetAllControllers();
        void stopNoteIfAny();

        void playNote(int noteNum, int volume, int duration, int channel, int instrument);
        void stopNote();

        void seq_note_on(const int note, const int volume, const int channel);
        void seq_note_off(const int note, const int channel);
        void seq_prog_change(const int instrumentID, const int channel);
        void seq_controlchange(const int controller, const int value, const int channel);
        void seq_pitch_bend(const int value, const int channel);
    }
}

#endif
