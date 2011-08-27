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

#include <vector>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <map>

class RtMidiIn;

namespace AriaMaestosa
{
    
    class Sequence;
    class Track;
    class PlatformMidiManagerFactory;
    
    /**
     * When writing Aria, I tried to make everything as cross-platform as possible. So, I've used only
     * cross-platform libs. However, i could not find any satisfying lib to do midi playback.
     *
     * So, i had to implement platform-specific code. To port Aria to another platform, all you need to do is
     * implement this interface, and provide a PlatformMidiManagerFactory for your manager (declare the
     * factory as a global so that it registers itself automagically when Aria opens)
     *
     * @ingroup midi.players
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
     *     playback), will call the various PlatformMidiManager::get()->seq_* functions (which must be implemented for
     *     anything to happen)
     */
    class PlatformMidiManager
    {
    protected:
        bool m_recording;
        RtMidiIn* m_midi_input;
        
        struct NoteInfo
        {
            int m_note_on_tick;
            int m_velocity;
        };
        
        /** Used when recording. Key is the midi note ID. */
        std::map<int, NoteInfo> m_open_notes;
        
        PlatformMidiManager();

        int m_start_tick;
        
        /** Track where notes go when recording */
        Track* m_record_target;
        
        /** rtmidi callback function */
        static void recordCallback( double deltatime, std::vector< unsigned char > *message, void *userData );

    public:
        
        virtual ~PlatformMidiManager() { }
        
        static std::vector<wxString> getChoices();
        static PlatformMidiManager* get();
        static void registerManager(PlatformMidiManagerFactory* newManager);
        
        /**
         * @brief                  starts playing the entire sequence, from the measure being marked as
         *                         the first one
         * @param[out] startTick   gives the tick where the song starts playing
         * @post                   the implementation of this method must set variable @c m_start_tick
         */
        virtual bool playSequence(Sequence* sequence, /*out*/int* startTick) = 0;
        
        /**
         * @brief                  starts playing the selected notes from the current track
         * @param[out] startTick   gives the tick where the song starts playing
         * @post                   the implementation of this method must set variable @c m_start_tick
         */
        virtual bool playSelected(Sequence* sequence, /*out*/int* startTick) = 0;
        
        /**
         * @return  whether a song is playing as a result of PlatformMidiManager::get()->playSequence or
         *          PlatformMidiManager::get()->playSelected
         * @note    "preview" notes, as issued by PlatformMidiManager::get()->playNote, do NOT count here;
         *          this function really only tracks the playback of entire sequences/tracks portions,
         *          not of single notes alone
         */
        virtual bool isPlaying() = 0;
        
        /**
         * @brief        stops a playing sequence previously started by PlatformMidiManager::get()->playSequence
         *               or PlatformMidiManager::get()->playSelected
         * @pre this function is not to be called when there is no songn playing
         *               (when PlatformMidiManager::get()->isPlaying returns false)
         */
        virtual void stop() = 0;
        
        virtual void exportAudioFile(Sequence* sequence, wxString filepath) = 0;
        
        /**
         * @brief  called repeatedly during playback to know progression
         *
         * @return midi tick currently being played, -1 if none
         * @note   must return the number of ticks elapsed since startTick
         *         (as returned by PlatformMidiManager::get()->playSequence / PlatformMidiManager::get()->playSelected)
         */
        virtual int trackPlaybackProgression() = 0;
        
        /** @brief called when app opens */
        virtual void initMidiPlayer() = 0;
        
        /** @brief called when app closes */
        virtual void freeMidiPlayer() = 0;
        
        /**
         * @brief play/stop a single "preview" note
         *
         * By "preview" note I mean that this function is used to play notes during editing,
         * not during actuakl song playback.
         * @note calls to this function should be ignored while PlatformMidiManager::get()->isPlaying returns true
         */
        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument) = 0;
        
        /**
         * @brief stop any note started with PlatformMidiManager::get()->playNote
         * @note  it is harmless to invoke this function even if no note is currently playing
         */
        virtual void stopNote() = 0;
        
        /**
         * @return the extension of sampled audio files that this platform implementation supports
         *         e.g. ".aiff", ".wav"
         * @note   this interface currently supports a single sampled audio format per platform
         * @note   returning wxEmptyString will be interpreted as "audio export not supported"
         */
        virtual const wxString getAudioExtension() = 0;
        
        /**
         * @return the wildcard string used for the file dialog when exporting to sampled audio
         *         e.g. "AIFF file|*.aiff"
         */
        virtual const wxString getAudioWildcard() = 0;
        
        /**
          * @return a list of possible MIDI output destinations (usually software synthesizers
          *         or physical devices). The first of the list will be used as default. All
          *         names must be unique and should remain persistent across runs.
          */
        virtual wxArrayString getOutputChoices() = 0;
        
        virtual wxArrayString getInputChoices();
        
        virtual bool startRecording(wxString outputPort, Track* target);

        virtual void stopRecording();
        
        bool isRecording() const { return m_recording; }
        
        // ---------- non-native sequencer interface ---------
        virtual void seq_note_on      (const int note, const int volume, const int channel)      { }
        virtual void seq_note_off     (const int note, const int channel)                        { }
        virtual void seq_prog_change  (const int instrument, const int channel)                  { }
        virtual void seq_controlchange(const int controller, const int value, const int channel) { }
        virtual void seq_pitch_bend   (const int value, const int channel)                       { }
        
        /**
          * @brief called repeatedly by the generic sequencer to tell the midi player what is the current
          *        progression. the sequencer will call this with -1 as argument to indicate it exits.
          */
        virtual void seq_notify_current_tick(const int tick) { }
        
        /**
          * @brief will be called by the generic sequencer to determine whether it should continue
          * @return false to stop it, true to continue
          */
        virtual bool seq_must_continue() { return false; }
    };
    
    /**
      * @brief Each midi manager implementation should also create a subclass of the factory and declare
      *        and instanciate one instance globally.
      * The factory will then automagically register itself and appear in the list of available MIDi drivers.
      */
    class PlatformMidiManagerFactory
    {
        wxString m_name;
    public:
        PlatformMidiManagerFactory(wxString name)
        {
            m_name = name;
            PlatformMidiManager::registerManager(this);
        }
        virtual ~PlatformMidiManagerFactory() {}
        
        wxString getName() const
        {
            return m_name;
        }
        virtual PlatformMidiManager* newInstance() = 0;
    };
    
}

#endif
