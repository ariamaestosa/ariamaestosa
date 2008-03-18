/*
 *  Copyright (C) 1999-2003 -------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

// everything here must be added/changed/checked
#ifdef __WIN32__

#include "AriaCore.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"

#include "wx/wx.h"
#include "wx/utils.h"
#include "wx/process.h"

namespace AriaMaestosa
{
    namespace PlatformMidiManager {
        
        int songLengthInTicks = -1;
        bool playing = false;
        
        bool playSequence(Sequence* sequence, /*out*/int* startTick)
        {
            // start by stopping any note currently playing
            stopNote();
            
            // then get midi data from sequence and pass it to native API
            // if you have a native API that reads midi bytes,
            // the easiest is to use something like the following :
            char* data;
            int datalength = -1;
            makeMidiBytes(sequence, false /* selection only */, &songLengthInTicks, startTick, &data, &datalength, true);
            
            // add some breath time at the end so that last note is not cut too sharply
            // 'makeMidiBytes' adds some silence at the end so no need to worry about possible problems
            songLengthInTicks += sequence->ticksPerBeat();
            
            playing = true;
            // feed midi bytes to native API if you can
        }
        bool playSelected(Sequence* sequence, /*out*/int* startTick)
        {
            // start by stopping any note currently playing
            stopNote();
            
            // then get midi data from sequence and pass it to native API
            // if you have a native API that reads midi bytes,
            // the easiest is to use something like the following :
            char* data;
            int datalength = -1;
            
            // add some breath time at the end so that last note is not cut too sharply
            // 'makeMidiBytes' adds some silence at the end so no need to worry about possible problems
            songLengthInTicks += sequence->ticksPerBeat();
            
            playing = true;
            // feed midi bytes to native API if you can
        }
        bool isPlaying()
        {
            return playing;
        }
        
        void stop()
        {
            playing = false;
        }
        
        // export to audio file, e.g. .wav
        void exportAudioFile(Sequence* sequence, wxString filepath)
        {
        }
        const wxString getAudioExtension()
        {
            return wxT(".wav");
        }
        const wxString getAudioWildcard()
        {
            return  wxString( _("WAV file")) + wxT("|*.wav");
        }
        
        
        bool exportMidiFile(Sequence* sequence, wxString filepath)
        {
            // okay to use generic function for this
            return AriaMaestosa::exportMidiFile(sequence, filepath);
        }
        
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
        int tick;
        int trackPlaybackProgression()
        {
            tick = getTickFromNativeAPI(...);
            if(tick > songLengthInTicks)
            {
                // song is over
                
                playing = false;
                // whatever else you want to do there
                
                // notify app that song is over
                Core::songHasFinishedPlaying();
                return -1;
            }
            return tick;
        }
        int getCurrentTick()
        {
            if(playing)
                return tick;
            else
                return -1;
        }

    	
        // called when app opens
        void initMidiPlayer()
        {
        }
        
        // called when app closes
        void freeMidiPlayer()
        {
        }
        
        // play/stop a single preview note (no sequencing is to be done, only direct output)
        void playNote(int noteNum, int volume, int duration, int channel, int instrument)
        {
        }
        void stopNote()
        {
        }
        

    }
}

#endif
