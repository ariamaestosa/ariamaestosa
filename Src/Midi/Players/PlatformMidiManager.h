/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __PLATFORM_MIDI_MANAGER_H__
#define __PLATFORM_MIDI_MANAGER_H__

namespace AriaMaestosa
{
    
    class Sequence;
    
    /**
     * When writing Aria, I tried to make everything as cross-platform as possible. So, I've used only
     * cross-platform libs. However, i could not find any satisfying lib to do midi playback.
     *
     * So, i had to implement platform-specific code. To port Aria to another platform, all you need to do is
     * implement these functions, and surround them with an #ifdef (like in MacPlayerInterface) that will be
     * defined only on the right platform.
     *
     * I may overhaul this in the future to make this a bit cleaner than ifdefs, but for now it does the job.
     *
     * There are, globally, two ways to implement song playback.
     * @li The first is to use a native MIDI sequencer (for instance, OS X provides functions that receive a bunch
     *     of MIDI data and play them, managing the timing/sequencing automagically). In this case, the seq_*
     *     functions need not be implemented. Note that there is a function to convert an Aria Sequence object
     *     into a buffer of MIDI bytes, this may come in handy.
     * @li The second is to use the MIDI sequencer provided with Aria (Midi/Players/Sequencer), which builds upon
     *     the simple sequencer provided by libjdkmidi. Simply create a libjdkmidi sequencer (makeJDKMidiSequence
     *     in Midi/CommonMidiUtils can be used to get a jdkmidi sequence, which can be fed to a jdkmidi sequencer),
     *     then create an object of AriaSequenceTimer type, giving it the jdkmidi sequence object.
     *     This object, when run it (and you will want to run it in a thread in order not the block the GUI during
     *     playback), will call the various PlatformMidiManager::seq_* functions (which must be implemented for
     *     anything to happen)
     */
    namespace PlatformMidiManager
    {
        
        /**
         * @brief                  starts playing the entire sequence, from the measure being marked as the first one
         * @param[out] startTick   gives the tick where the song starts playing
         */
        bool playSequence(Sequence* sequence, /*out*/int* startTick);
        
        /**
         * @brief                  starts playing the selected notes from the current track
         * @param[out] startTick   gives the tick where the song starts playing
         */
        bool playSelected(Sequence* sequence, /*out*/int* startTick);
        
        /**
         * @return  whether a song is playing as a result of PlatformMidiManager::playSequence or
         *          PlatformMidiManager::playSelected
         * @note    "preview" notes, as issued by PlatformMidiManager::playNote, do NOT count here;
         *          this function really only tracks the playback of entire sequences/tracks portions,
         *          not of single notes alone
         */
        bool isPlaying();
        
        /**
         * @brief        stops a playing sequence previously started by PlatformMidiManager::playSequence
         *               or PlatformMidiManager::playSelected
         * @pre this function is not to be called when there is no songn playing
         *               (when PlatformMidiManager::isPlaying returns false)
         */
        void stop();
        
        void exportAudioFile(Sequence* sequence, wxString filepath);
        
        //FIXME: not sure this belongs here at all.
        /**
         * @brief Invoked when user requests and export to MIDI file format
         * @note  Aria provides a factory implementation that can generally be used for this :
         *        AriaMaestosa::exportMidiFile(sequence, filepath);
         */
        bool exportMidiFile(Sequence* sequence, wxString filepath);
        
        /**
         * @brief  called repeatedly during playback to know progression
         *
         * @return midi tick currently being played, -1 if none
         * @note   must return the number of ticks elapsed since startTick
         *         (as returned by PlatformMidiManager::playSequence / PlatformMidiManager::playSelected)
         */
        int trackPlaybackProgression();
        
        /** @brief called when app opens */
        void initMidiPlayer();
        
        /** @brief called when app closes */
        void freeMidiPlayer();
        
        /**
         * @brief play/stop a single "preview" note
         *
         * By "preview" note I mean that this function is used to play notes during editing,
         * not during actuakl song playback.
         * @note calls to this function should be ignored while PlatformMidiManager::isPlaying returns true
         */
        void playNote(int noteNum, int volume, int duration, int channel, int instrument);
        
        /**
         * @brief stop any note started with PlatformMidiManager::playNote
         * @note  it is harmless to invoke this function even if no note is currently playing
         */
        void stopNote();
        
        /**
         * @return the extension of sampled audio files that this platform implementation supports
         *         e.g. ".aiff", ".wav"
         * @note   this interface currently supports a single sampled audio format per platform
         * @note   returning wxEmptyString will be interpreted as "audio export not supported"
         */
        const wxString getAudioExtension();
        
        /**
         * @return the wildcard string used for the file dialog when exporting to sampled audio
         *         e.g. "AIFF file|*.aiff"
         */
        const wxString getAudioWildcard();
        
        /* ---------- non-native sequencer interface ---------
         * fill these only if you use the generic AriaSequenceTimer midi sequencer/timer
         * if you use a native sequencer in the above functions, these will not be called
         */
        void seq_note_on(const int note, const int volume, const int channel);
        void seq_note_off(const int note, const int channel);
        void seq_prog_change(const int instrument, const int channel);
        void seq_controlchange(const int controller, const int value, const int channel);
        void seq_pitch_bend(const int value, const int channel);
        
        // called repeatedly by the generic sequencer to tell
        // the midi player what is the current progression
        // the sequencer will call this with -1 as argument to indicate it exits.
        void seq_notify_current_tick(const int tick);
        // will be called by the generic sequencer to determine whether it should continue
        // return false to stop it.
        bool seq_must_continue();
    }
    
}

#endif
