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

/*

 The Mac implementation of Platform midi player uses QTKit, AudioToolbox and CoreAudio.

 CoreAudio is used to play single notes (the previews while editing)

 For actual playback of midi data, both QTKit and AudioToolbox implementations exist. The rationale behind this is that:

 - QTKit offers more precise info for getting current tick, however it returns time and not ticks. It is easy to calculate current
   midi tick from time with tempo, but not when there are tempo bends. Furthermore, it allows to export to audio formats like AIFF.
 - AudioToolkit returns current playback location in midi beats. This is much less precise than tempo. However, this does follow tempo bends.

 So, when there are tempo bends, use AudioToolkit. Otherwise, use QTKit.

 */

#include "Config.h"
#ifdef _MAC_QUICKTIME_COREAUDIO

#include <iostream>

#include "wx/wx.h"

#include "main.h"

#include "GUI/GLPane.h"
#include "GUI/MainFrame.h"
#include "GUI/MeasureBar.h"

#include "Dialogs/WaitWindow.h"

#include "Midi/Sequence.h"
#include "Midi/Players/Mac/QTKitPlayer.h"
#include "Midi/Players/Mac/CoreAudioNotePlayer.h"
#include "Midi/Players/Mac/AudioToolboxPlayer.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"

#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"


#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/sysex.h"

#include "jdkmidi/sequencer.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/process.h"

namespace AriaMaestosa {

namespace PlatformMidiManager {

	void playMidiBytes(char* bytes, int length);
	void exportToAudio(char* bytes, int length, wxString filename);

    AudioToolboxMidiPlayer* audioToolboxMidiPlayer;

    Sequence* sequence;
    bool playing=false;
	bool use_qtkit = true;

	int stored_songLength = 0;

	const wxString getAudioExtension()
	{
		return wxT(".aiff");
	}
	const wxString getAudioWildcard()
	{
		return  wxString( _("AIFF file")) + wxT("|*.aiff");
	}

	bool playSequence(Sequence* sequence, /*out*/ int* startTick)
	{
		if(playing) return false; //already playing

		PlatformMidiManager::sequence = sequence;

		char* data;
		int datalength = -1;

		int songLengthInTicks = -1;
		makeMidiBytes(sequence, false, &songLengthInTicks, startTick, &data, &datalength, true);

		stored_songLength = songLengthInTicks + sequence->ticksPerBeat();
		playMidiBytes(data, datalength);

		free(data);

		return true;
	}

	bool playSelected(Sequence* sequence, /*out*/ int* startTick)
	{

		if(playing) return false; //already playing

		PlatformMidiManager::sequence = sequence;

		char* data;
		int datalength = -1;
		int songLengthInTicks = -1;

		makeMidiBytes(sequence, true, &songLengthInTicks, startTick, &data, &datalength, true);

		stored_songLength = songLengthInTicks + sequence->ticksPerBeat();
		playMidiBytes(data, datalength);

		free(data);

		return true;
	}

	bool exportMidiFile(Sequence* sequence, wxString filepath)
	{
		AriaMaestosa::exportMidiFile(sequence, filepath);
		return true;
	}

	void exportAudioFile(Sequence* sequence, wxString filepath)
	{

		// when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
		const int firstMeasureValue=getMeasureBar()->getFirstMeasure();
		getMeasureBar()->setFirstMeasure(0);

		char* data;
		int length = -1;

		int startTick = -1, songLength = -1;
		makeMidiBytes(sequence, false, &songLength, &startTick, &data, &length, true);

		exportToAudio( data, length, filepath );

		free(data);

		getMeasureBar()->setFirstMeasure(firstMeasureValue);
	}


	int trackPlaybackProgression()
	{

	    int currentTick = getCurrentTick();

		// song ends
		if(currentTick >= stored_songLength-1 or currentTick == -1){
			getMainFrame()->songHasFinishedPlaying();
			currentTick=-1;
			stop();
			return -1;
		}


		return currentTick;

	}

    void playMidiBytes(char* bytes, int length)
	{
		CoreAudioNotePlayer::stopNote();

		// if length==8, this is just the empty song to load QT. (when the app opens, Quicktime is triggered with an empty song to make it load)
		if( length==8 or sequence->tempoEvents.size()==0 ) use_qtkit=true;
		else use_qtkit=false;

		if( use_qtkit )
		{
			qtkit_setData(bytes, length);
			qtkit_play();
		}
		else
		{
			audioToolboxMidiPlayer->loadSequence(bytes, length);
			audioToolboxMidiPlayer->play();
		}

		playing = true;
    }

	void playNote(int noteNum, int volume, int duration, int channel, int instrument)
	{
		if(playing) return;
		CoreAudioNotePlayer::playNote( noteNum, volume, duration, channel, instrument );
	}

	bool isPlaying()
	{
		return playing;
	}

	void stopNote()
	{
		CoreAudioNotePlayer::stopNote();
	}

    void stop()
	{
		if( use_qtkit ) qtkit_stop();
		else audioToolboxMidiPlayer->stop();

		playing = false;
    }

	int getCurrentTick()
	{
		if( use_qtkit )
		{
			const float time = qtkit_getCurrentTime();

			if(time == -1) return -1; // not playing

			return (int)(
						 time * sequence->getTempo()/60.0 * (float)sequence->ticksPerBeat()
						 );
		}
		else
		{
			return audioToolboxMidiPlayer->getPosition() * sequence->ticksPerBeat();
		}
    }


    void exportToAudio(char* bytes, int length, wxString filename)
	{

        qtkit_setData(bytes, length);
		//bool success = qtkit_exportToAiff( "/Users/mathieu/Desktop/cauteriser.aiff" );
		//wxCSConv cs( wxFONTENCODING_UTF8 );
		//std::cout << "		Converted : " << ( const char * ) filename.mb_str( cs ) << std::endl;
		//const char* path_utf = ( const char * ) filename.mb_str( cs );
		//bool success = qtkit_exportToAiff( (char*)path_utf );

		//std::cout << (char*)to_UTF8_CString(filename) << std::endl;
		//bool success = qtkit_exportToAiff( (char*)to_UTF8_CString(filename) );


        wxCSConv cs( wxFONTENCODING_UTF8 );
        wxCharBuffer output = cs.cWC2MB(filename.wc_str());

		bool success = qtkit_exportToAiff( (char*)output.data() );

		//bool success = qtkit_exportToAiff( (char*)(filename.ToAscii().data()) );
       // bool success = qtkit_exportToAiff( (char*)toCString(filename) );
		if(!success)
		{
			std::cout << "EXPORTING FAILED" << std::endl;
		}
	WaitWindow::hide();
    }


    void initMidiPlayer()
	{
        qtkit_init();
		CoreAudioNotePlayer::init();
		audioToolboxMidiPlayer=new AudioToolboxMidiPlayer();

		// trigger quicktime with empty song, just to make it load
		char bytes[8];
		bytes[0] = 'M';
		bytes[1] = 'T';
		bytes[2] = 'h';
		bytes[3] = 'd';
		bytes[4] = 0;
		bytes[5] = 0;
		bytes[6] = 0;
		bytes[7] = 0;

		playMidiBytes(bytes, 8);
		stop();
    }

    void freeMidiPlayer()
	{
        qtkit_free();
		CoreAudioNotePlayer::free();
		delete audioToolboxMidiPlayer;
        audioToolboxMidiPlayer=NULL;
    }

}
}

#endif
