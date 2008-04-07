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

#ifndef _platform_midi_player_
#define _platform_midi_player_

/*
 * When writing this app, i tried to make everything as cross-platform as possible. So, i've used only cross-platform libs.
 * However, i could not find any satisfying lib to do midi playback.
 *
 * So, i had to implement platform-specific code. To port Aria to another platform, all you need to do is implement these functions, and surround them
 * with an #ifdef (like in MacPlayerInterface) that will be defined only on the right platform.
 */

namespace AriaMaestosa {

	class Sequence;

namespace PlatformMidiManager {


	bool playSequence(Sequence* sequence, /*out*/int* startTick);
	bool playSelected(Sequence* sequence, /*out*/int* startTick);
    bool isPlaying();
    void stop();

	void exportAudioFile(Sequence* sequence, wxString filepath);
	bool exportMidiFile(Sequence* sequence, wxString filepath);

    /*
     * returns midi tick currently being played, -1 if none
     *
     * called repeatedly during playback
     *
     * difference between 'trackPlaybackProgression' and 'getCurrentTick' is that the first
     * gets the data from the native API, and acts upon it (for instance checking if song is over)
     * 'getCurrentTick' just returns the last retrieved tick value. Those 2 can probably be
     * merged in a future code cleanup
     */
	int trackPlaybackProgression();
	//int getCurrentTick();

	// called when app opens
    void initMidiPlayer();

	// called when app closes
    void freeMidiPlayer();

	// play/stop a single preview note
	void playNote(int noteNum, int volume, int duration, int channel, int instrument);
	void stopNote();

	const wxString getAudioExtension();
	const wxString getAudioWildcard();

	// ---------- non-native sequencer interface ---------
	// fill these only if you use the generic AriaSequenceTimer midi sequencer/timer
	// if you use a native sequencer in the above functions, these will not be called
	void seq_note_on(const int note, const int volume, const int channel);
	void seq_note_off(const int note, const int channel);
	void seq_prog_change(const int instrument, const int channel);
	void seq_controlchange(const int controller, const int value, const int channel);
	void seq_pitch_bend(const int value, const int channel);
	void seq_reset();
	// called repeatedly by the generic sequencer to tell
	// the midi player what is the current progression
	// the sequencer will call this with -1 as argument to indicate it exits.
	void seq_notify_current_tick(const int tick);

	bool seq_must_continue();
}

}

#endif
