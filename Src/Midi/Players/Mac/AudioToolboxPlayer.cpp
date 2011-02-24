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

// refer to MacPlayerInterface for docs

#ifdef _MAC_QUICKTIME_COREAUDIO

#include "Utils.h"

#include "Midi/Players/Mac/AudioToolboxPlayer.h"
#include <iostream>

#include <wx/timer.h>

namespace AriaMaestosa
{

AudioToolboxMidiPlayer::AudioToolboxMidiPlayer()
{

    playing = false;

}

void AudioToolboxMidiPlayer::loadSequence(char* midiData, int length)
{

    if (NewMusicPlayer(&musicPlayer) != noErr)
        std::cout << "NewMusicPlayer failed" << std::endl;

    if (NewMusicSequence(&musicSequence) != noErr)
        std::cout << "NewMusicSequence failed" << std::endl;

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

    if (MusicSequenceFileLoadData(musicSequence, CFDataCreate(NULL, (UInt8*)midiData, length), kMusicSequenceFile_MIDIType, 0) != noErr)
        std::cout << "MusicSequenceFileLoadData failed" << std::endl;
    
#else
#warning Using deprecated code for OSX 10.5
    if (MusicSequenceLoadSMFData(musicSequence, CFDataCreate(NULL, (UInt8*)midiData, length) ) != noErr)
        std::cout << "MusicSequenceLoadSMFData failed" << std::endl;

#endif
    
    if (MusicPlayerSetSequence(musicPlayer, musicSequence) != noErr)
        std::cout << "MusicPlayerSetSequence failed" << std::endl;

}

void AudioToolboxMidiPlayer::play()
{


    if (MusicPlayerSetTime(musicPlayer, (MusicTimeStamp)0.0) != noErr)
        std::cout << "MusicPlayerSetTime failed" << std::endl;

    if (MusicPlayerStart(musicPlayer) != noErr)
        std::cout << "MusicPlayerStart failed" << std::endl;

    playing = true;

}

void AudioToolboxMidiPlayer::stop()
{

    if ( !playing ) return;

    if (MusicPlayerStop(musicPlayer) != noErr)
        std::cout << "MusicPlayerStop failed" << std::endl;

    if (DisposeMusicPlayer(musicPlayer) != noErr)
        std::cout << "********* DisposeMusicPlayer failed" << std::endl;

    if (DisposeMusicSequence(musicSequence) != noErr)
        std::cout << "********* DisposeMusicSequence failed" << std::endl;

    if (NewMusicPlayer(&musicPlayer) != noErr)
        std::cout << "NewMusicPlayer failed" << std::endl;

    if (NewMusicSequence(&musicSequence) != noErr)
        std::cout << "NewMusicSequence failed" << std::endl;

    playing = false;
}

int AudioToolboxMidiPlayer::getPosition()
{
    MusicTimeStamp outTime;
    MusicPlayerGetTime( musicPlayer, &outTime );

    return (int)outTime;
}

AudioToolboxMidiPlayer::~AudioToolboxMidiPlayer()
{
}

}

#endif
