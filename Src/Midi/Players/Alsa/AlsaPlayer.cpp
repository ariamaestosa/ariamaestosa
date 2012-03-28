/*
 * Copyright (C) 2007-2010 Marianne Gagnon, based on code
 * Copyright (C) 1999-2003 Steve Ratcliffe
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

#ifdef _ALSA

#include <glib.h>

#include "AriaCore.h"
#include "Midi/Players/Alsa/AlsaNotePlayer.h"
#include "Midi/Players/Alsa/AlsaPort.h"
#include "Midi/Players/Sequencer.h"
#include "IO/IOUtils.h"

#include <alsa/asoundlib.h>

#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "PreferencesData.h"
#include "GUI/MainFrame.h"
#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"
#include "Dialogs/WaitWindow.h"

#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/process.h>

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

namespace AriaMaestosa
{

bool sound_available = false;

MidiContext* context = NULL;

bool must_stop=false;
Sequence* g_sequence;

void cleanup_after_playback()
{
    if (not sound_available) return;

    context->setPlaying(false);
    AlsaPlayerStuff::resetAllControllers();
}

int currentTick;
int g_current_accurate_tick;

class MyPThread
{
    int id;
    pthread_t thread;
  public:
    MyPThread(){}

    void runFunction(void* (*func)(void*) )
    {
        id = pthread_create( &thread, NULL, func, (void*)NULL);
    }
};


namespace threads
{
  MyPThread export_audio;
}

// --- export to audio thread --
wxString export_audio_filepath;
void* export_audio_func( void *ptr )
{
    // the file is exported to midi, and then we tell timidity to make it into wav
    wxString tempMidiFile = export_audio_filepath.BeforeLast('/') + wxT("/aria_temp_file.mid");
    
    AriaMaestosa::exportMidiFile(g_sequence, tempMidiFile);
    wxString cmd = wxT("timidity -Ow -o \"") + export_audio_filepath + wxT("\" \"") + tempMidiFile + wxT("\" -idt");
    std::cout << "executing " << cmd.mb_str() << std::endl;
    
    FILE * command_output;
    char output[128];
    int amount_read = 1;
    
    std::cout << "-----------------\ntimidity output\n-----------------\n";
    try
    {
        command_output = popen(cmd.mb_str(), "r");
        if (command_output == NULL) throw;
		
        while(amount_read > 0)
        {
            amount_read = fread(output, 1, 127, command_output);
            if (amount_read <= 0) break;
            else
            {
                output[amount_read] = '\0';
                std::cout << output << std::endl;
            }
        }
    }
    catch(...)
    {
        std::cout << "An error occured while exporting audio file." << std::endl;
        return (void*)NULL;
    }
    
    std::cout << "\n-----------------" << std::endl;
    pclose(command_output);
    
    // send hide progress window event
    MAKE_HIDE_PROGRESSBAR_EVENT(event);
    getMainFrame()->GetEventHandler()->AddPendingEvent(event);
    
    wxRemoveFile(tempMidiFile);
    
    return (void*)NULL;
}



class SequencerThread : public wxThread
{
    jdkmidi::MIDIMultiTrack* jdkmidiseq;
    jdkmidi::MIDISequencer* jdksequencer;
    int songLengthInTicks;
    bool selectionOnly;
    int m_start_tick;
    
public:
    
    SequencerThread(const bool selectionOnly)
    {
        jdkmidiseq = NULL;
        jdksequencer = NULL;
        SequencerThread::selectionOnly = selectionOnly;
    }
    ~SequencerThread()
    {
        if (jdksequencer != NULL) delete jdksequencer;
        if (jdkmidiseq != NULL) delete jdkmidiseq;
    }

    void prepareSequencer()
    {
        jdkmidiseq = new jdkmidi::MIDIMultiTrack();
        songLengthInTicks = -1;
        int trackAmount = -1;
        m_start_tick = 0;
        makeJDKMidiSequence(g_sequence, *jdkmidiseq, selectionOnly, &songLengthInTicks,
                            &m_start_tick, &trackAmount, true /* for playback */);
                        
        // add one beat at the end to leave some time for notes to ring off
        if (not g_sequence->isLoopEnabled())
        {
            songLengthInTicks += g_sequence->ticksPerBeat();
        }

        //std::cout << "trackAmount=" << trackAmount << " start_tick=" << m_start_tick<<
        //        " songLengthInTicks=" << songLengthInTicks << std::endl;

        jdksequencer = new jdkmidi::MIDISequencer(jdkmidiseq);
    }

    void go(int* startTick /* out */)
    {
        if (Create() != wxTHREAD_NO_ERROR)
        {
            std::cerr << "error creating thread" << std::endl;
            return;
        }
        SetPriority(85 /* 0 = min, 100 = max */);

        prepareSequencer();
        *startTick = m_start_tick;

        Run();
    }

    ExitCode Entry()
    {
        AriaSequenceTimer timer(g_sequence);
        timer.run(jdksequencer, songLengthInTicks);

        must_stop = true;
        cleanup_after_playback();

        return 0;
    }
};

#if 0
#pragma mark -
#endif

class AlsaMidiManager : public PlatformMidiManager
{
public:
    // called when app opens
    virtual void initMidiPlayer()
    {
        wxLogVerbose( wxT("AlsaMidiManager::initMidiPlayer (enter)") );
        AlsaPlayerStuff::alsa_output_module_init();
        
        context = new MidiContext();
        
        const bool launchTimidity = (PreferencesData::getInstance()->getBoolValue("launchTimidity"));
        
        if (not context->openDevice(launchTimidity))
        {
            wxMessageBox( _("Failed to open ALSA device, sound will be unavailable (you can try selecting another output device from the 'Output' menu)") );
            std::cerr << "failed to open ALSA device" << std::endl;
            sound_available = false;
            AlsaPlayerStuff::alsa_output_module_free();
            return;
        }
        sound_available = true;
        
        AlsaPlayerStuff::alsa_output_module_setContext(context);
        
        context->setPlaying(false);
        
        wxLogVerbose( wxT("AlsaMidiManager::initMidiPlayer (exit)") );
    }

    // called when app closes
    virtual void freeMidiPlayer()
    {
        wxLogVerbose( wxT("AlsaMidiManager::freeMidiPlayer") );
        
        if (context != NULL)
        {
            context->closeDevice();
            delete context;
        }
        AlsaPlayerStuff::alsa_output_module_free();
        sound_available = false;
    }

    virtual bool playSequence(Sequence* sequence, /*out*/int* startTick)
    {
        if (not sound_available) return true;

        // std::cout << "  * playSequencer" << std::endl;
        if (context->isPlaying())
        {
            std::cout << "cannot play, it's already playing" << std::endl;
            return false; //already playing
        }
        AlsaPlayerStuff::stopNoteIfAny();
        must_stop = false;
        context->setPlaying(true);

        g_sequence = sequence;
        currentTick = 0;
        g_current_accurate_tick = 0;

        // std::cout << "  * playSequencer - creating new thread" << std::endl;

        SequencerThread* seqthread = new SequencerThread(false /* selection only */);
        seqthread->go(startTick);

        m_start_tick = *startTick;
        
        currentTick = m_start_tick;
        g_current_accurate_tick = m_start_tick;
        
        return true;
    }

    virtual bool playSelected(Sequence* sequence, /*out*/int* startTick)
    {
        if (not sound_available) return true;

        if (context->isPlaying()) return false; //already playing
        AlsaPlayerStuff::stopNoteIfAny();
        must_stop = false;
        context->setPlaying(true);

        g_sequence = sequence;
        currentTick = 0;
        g_current_accurate_tick = 0;

        SequencerThread* seqthread = new SequencerThread(true /* selection only */);
        seqthread->go(startTick);

        m_start_tick = *startTick;
        
        currentTick = m_start_tick;
        g_current_accurate_tick = m_start_tick;
        
        return true;


    /*
            if (context->isPlaying()) return false; //already playing
            stopNoteIfAny();
            must_stop = false;

            PlatformMidiManager::get()->sequence = sequence;
            PlatformMidiManager::get()->currentTick = 0;

            songLengthInTicks = -1;

            makeMidiBytes(sequence, true, &songLengthInTicks, startTick, &data, &datalength, true);

            stored_songLength = songLengthInTicks + sequence->ticksPerBeat();

            // start in a new thread as to not block the UI during playback;
            threads::add_events.runFunction(&add_events_func);

            context->setPlaying(true);
            return true;
            */
    }

    // returns current midi tick, or -1 if over
    virtual int trackPlaybackProgression()
    {
        if (not sound_available) return 0;

    //std::cout << "trackPlaybackProgression ";
        if (context->isPlaying() and currentTick != -1)
        {
            //std::cout << "returning " << currentTick << std::endl;
            return currentTick;
        }
        else
        {
            std::cout << "SONG DONE" << std::endl;
            must_stop = true;
            Core::songHasFinishedPlaying();
            return -1;
        }
    }

    virtual int getAccurateTick()
    {
        return g_current_accurate_tick;
    }
    
    virtual bool seq_must_continue()
    {
        if (not sound_available) return false;

        return !must_stop;
    }

    virtual void seq_notify_current_tick(const int tick)
    {
        currentTick = tick;
    }

    virtual void seq_notify_accurate_current_tick(const int tick)
    {
        g_current_accurate_tick = tick;
    }
    
    virtual bool isPlaying()
    {
        if (not sound_available) return false;

        return context->isPlaying();
    }

    virtual void stop()
    {
        must_stop = true;
        //context->setPlaying(false);

        //cleanup_after_playback();
    }

    /*
    // midi tick currently being played, -1 if none
    int getCurrentTick()
    {
        std::cout << "get current tick..." << std::endl;
        if (context->isPlaying())
            return currentTick;
        else
            return -1;
    }
    */

    virtual const wxString getAudioExtension()
    {
        return wxT(".wav");
    }

    virtual const wxString getAudioWildcard()
    {
        return  wxString( _("WAV file")) + wxT("|*.wav");
    }

    /*
    void trackPlayback_thread_loop()
    {
        while(!PlatformMidiManager::get()->must_stop)
        {

            PlatformMidiManager::get()->currentTick = seqContext->getCurrentTick();

            if (PlatformMidiManager::get()->currentTick >=
            PlatformMidiManager::get()->songLengthInTicks or
            PlatformMidiManager::get()->currentTick==-1) PlatformMidiManager::get()->must_stop=true;
        }

        // clean up any events remaining on the queue and stop it
        snd_seq_drop_output(seqContext->getAlsaHandle());
        seqContext->stopTimer();
        PlatformMidiManager::get()->allSoundOff();

        PlatformMidiManager::get()->currentTick = -1;

        if (root != NULL)
        {
            md_free(MD_ELEMENT(root));
            root = NULL;
        }

        delete seqContext;

        // Restore signal handler
        //signal(SIGINT, SIG_DFL);// FIXME - are these removed or not?
        context->setPlaying(false);

        PlatformMidiManager::get()->resetAllControllers();

    }
    */

    virtual wxArrayString getOutputChoices()
    {
		wxArrayString out;
		
		const int deviceAmount = context->getDeviceAmount();
		for (int n=0; n<deviceAmount; n++)
		{
			MidiDevice* md = context->getDevice(n);
			std::cout << (const char*)md->name.mb_str() << std::endl;
			out.Add( wxString::Format(wxT("%i:%i "), md->client, md->port) + md->name );
		}
		
        return out;
    }
    
    virtual void exportAudioFile(Sequence* sequence, wxString filepath)
    {
        g_sequence = sequence;
        export_audio_filepath = filepath;
        threads::export_audio.runFunction( &export_audio_func );
    }

    virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument)
    {
        if (not sound_available) return;

        AlsaPlayerStuff::playNote(noteNum, volume, duration, channel, instrument);
    }

    virtual void stopNote()
    {
        if (not sound_available) return;

        AlsaPlayerStuff::stopNote();
    }

    virtual void seq_note_on(const int note, const int volume, const int channel)
    {
        AlsaPlayerStuff::seq_note_on(note, volume, channel);
    }


    virtual void seq_note_off(const int note, const int channel)
    {
        AlsaPlayerStuff::seq_note_off(note, channel);
    }

    virtual void seq_prog_change(const int instrumentID, const int channel)
    {
        AlsaPlayerStuff::seq_prog_change(instrumentID, channel);
    }

    virtual void seq_controlchange(const int controller, const int value, const int channel)
    {
        AlsaPlayerStuff::seq_controlchange(controller, value, channel);
    }

    virtual void seq_pitch_bend(const int value, const int channel)
    {
        AlsaPlayerStuff::seq_pitch_bend(value, channel);
    }

};

class AlsaMidiManagerFactory : public PlatformMidiManagerFactory
{
public:
    AlsaMidiManagerFactory() : PlatformMidiManagerFactory(wxT("Linux/ALSA"))
    {
    }
    virtual ~AlsaMidiManagerFactory()
    {
    }
    
    virtual PlatformMidiManager* newInstance()
    {
        return new AlsaMidiManager();
    }
};
AlsaMidiManagerFactory g_alsa_midi_manager_factory;


}

#endif
