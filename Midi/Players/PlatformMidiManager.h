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
 * So, i had to implement platform-specific code. To port Aria to another platform, all you need to do is implement these functions, and surrond them
 * with an #ifdef (like in MacPlayerInterface) that will be defined only on the right platform (can often be done in Config.h).
 */

namespace AriaMaestosa {
	
	class Sequence;
	
namespace PlatformMidiManager {
    
	
	bool playSequence(Sequence* sequence, /*out*/int* startTick);
	bool playSelected(Sequence* sequence, /*out*/int* startTick);
	void exportAudioFile(Sequence* sequence, wxString filepath);
	bool exportMidiFile(Sequence* sequence, wxString filepath);
	
	// returns current midi tick, or -1 if over
	int trackPlaybackProgression();
	
	bool isPlaying();
	
    void stop();
	
	// midi tick currently being played, -1 if none
    int getCurrentTick();
    	
	// called when app opens
    void initMidiPlayer();
	
	// called when app closes
    void freeMidiPlayer();
 
	// play/stop a single preview note
	void playNote(int noteNum, int volume, int duration, int channel, int instrument);
	void stopNote();
	
	const wxString getAudioExtension();
	const wxString getAudioWildcard();
	
}

}

#endif
