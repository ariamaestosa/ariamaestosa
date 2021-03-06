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
#include <wx/filename.h>

#include "jdksmidi/world.h"
#include "jdksmidi/track.h"
#include "jdksmidi/multitrack.h"
#include "jdksmidi/filereadmultitrack.h"
#include "jdksmidi/fileread.h"
#include "jdksmidi/fileshow.h"
#include "jdksmidi/filewritemultitrack.h"
#include "jdksmidi/msg.h"
#include "jdksmidi/sysex.h"
#include "jdksmidi/sequencer.h"

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
enum AudioExportEngine
{
    FLUIDSYNTH = 0,
    TIMIDITY = 1
};

wxString g_export_audio_filepath;
AudioExportEngine g_export_engine;
wxString g_fluisynth_soundfont;

void* export_audio_func( void *ptr )
{
    // the file is exported to midi, and then we tell timidity to make it into wav
    wxString tempMidiFile = g_export_audio_filepath.BeforeLast('/') + wxT("/aria_temp_file.mid");
    
    AriaMaestosa::exportMidiFile(g_sequence, tempMidiFile);
    wxString cmd;
    
    if (g_export_engine == FLUIDSYNTH)
    {
        // fluidsynth -O s32 -T wav -a file --fast-render=test.wav /usr/share/sounds/sf2/FluidR3_GM.sf2 '/home/mmg/Desktop/angelinblack/Angel in black piano.mid'
        cmd = wxT("fluidsynth -O s32 -T wav -a file --fast-render=\"") + g_export_audio_filepath +
              wxT("\" \"") + g_fluisynth_soundfont + wxT("\" \"") + tempMidiFile + wxT("\"");
    }
    else if (g_export_engine == TIMIDITY)
    {
        cmd = wxT("timidity -Ow -o \"") + g_export_audio_filepath + wxT("\" \"") + tempMidiFile + wxT("\" -idt");
    }
    else
    {
        ASSERT(false);
    }
    
    std::cout << "executing " << cmd.mb_str() << std::endl;
    
    FILE * command_output;
    char output[128];
    int amount_read = 1;
    
    std::cout << "-----------------\ntiMidity output\n-----------------\n";
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
    jdksmidi::MIDIMultiTrack* jdkmidiseq;
    jdksmidi::MIDISequencer* jdksequencer;
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
        jdkmidiseq = new jdksmidi::MIDIMultiTrack();
        songLengthInTicks = -1;
        int trackAmount = -1;
        m_start_tick = 0;
        makeJDKMidiSequence(g_sequence, *jdkmidiseq, selectionOnly, &songLengthInTicks,
                            &m_start_tick, &trackAmount, true /* for playback */);

        //std::cout << "trackAmount=" << trackAmount << " start_tick=" << m_start_tick<<
        //        " songLengthInTicks=" << songLengthInTicks << std::endl;

        jdksequencer = new jdksmidi::MIDISequencer(jdkmidiseq);
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
        
        const bool launchFluidSynth = (PreferencesData::getInstance()->getBoolValue("launchFluidSynth", false));
        
        if (not context->openDevice(launchFluidSynth))
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

            stored_songLength = songLengthInTicks + sequence->ticksPerQuarterNote();

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

        return currentTick;

/*
        //std::cout << "trackPlaybackProgression ";
        if (context->isPlaying() and currentTick != -1)
        {
            //std::cout << "returning " << currentTick << std::endl;
            return currentTick;
        }
        else
        {
            //std::cout << "SONG DONE" << std::endl;
            //must_stop = true;
            //Core::songHasFinishedPlaying();
            return -1;
        }
         * */
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
			out.Add(md->getFullName());
		}
		
        return out;
    }
    
    class AudioExportDialog : public wxDialog
    {
        wxRadioBox* m_radioBox;
        wxTextCtrl* m_soundfontTextCtrl;
        wxButton*   m_browseButton;
        
    public:
        AudioExportDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, _("Settings"), wxDefaultPosition, 
                                                       wxSize(460, 200), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX)
        {
            PreferencesData* prefs = PreferencesData::getInstance();
            g_export_engine = (AudioExportEngine)prefs->getIntValue(SETTING_ID_AUDIO_EXPORT_ENGINE);
            g_fluisynth_soundfont = prefs->getValue(SETTING_ID_FLUIDSYNTH_SOUNDFONT_PATH);
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            
            wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Choose the MIDI engine to use to render the file"));
            sizer->Add(label, 0, wxALL, 5);
            
            wxArrayString choices;
            choices.Add(wxT("FluidSynth"));
            choices.Add(wxT("TiMidity"));
            m_radioBox = new wxRadioBox(this, wxID_ANY, _("MIDI Engine"), wxDefaultPosition,
                                                  wxDefaultSize, choices, wxRA_SPECIFY_ROWS);
            m_radioBox->SetSelection(g_export_engine);
            sizer->AddStretchSpacer();
            sizer->Add(m_radioBox, 0, wxEXPAND | wxALL, 5);
            sizer->AddStretchSpacer();
            
            wxStaticText* soundfontLbl = new wxStaticText(this, wxID_ANY, _("Fluidsynth Soundfont"));
            sizer->Add(soundfontLbl, 0, wxALL, 5);
            
            wxBoxSizer* soundFontSizer = new wxBoxSizer(wxHORIZONTAL);
            
            m_soundfontTextCtrl = new wxTextCtrl(this, wxID_ANY, g_fluisynth_soundfont);
            soundFontSizer->Add(m_soundfontTextCtrl, 1, wxALL, 2);
            
            m_browseButton = new wxButton(this, wxID_ANY, wxT("..."), wxDefaultPosition,
                                          wxDLG_UNIT(this,wxSize(15,13)));
            soundFontSizer->Add(m_browseButton, 0, wxALL, 2);
            
            sizer->Add(soundFontSizer, 0, wxALL|wxEXPAND, 5);
        
            sizer->AddStretchSpacer();
            
            m_soundfontTextCtrl->Enable(g_export_engine == FLUIDSYNTH);
            
            wxButton* okBtn = new wxButton(this, wxID_OK, _("OK"));
            wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, _("Cancel"));

            wxStdDialogButtonSizer* stdDialogButtonSizer = new wxStdDialogButtonSizer();
            stdDialogButtonSizer->AddButton(okBtn);
            stdDialogButtonSizer->AddButton(cancelBtn);
            stdDialogButtonSizer->Realize();
            sizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);
            SetSizer(sizer);
            
            m_radioBox->Connect(m_radioBox->GetId(), wxEVT_COMMAND_RADIOBOX_SELECTED,
                                wxCommandEventHandler(AudioExportDialog::onChange), NULL, this);
            
            m_browseButton->Connect(m_browseButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(AudioExportDialog::onButtonClicked), NULL, this);
        }
        
        ~AudioExportDialog()
        {
            PreferencesData* prefs = PreferencesData::getInstance();
            
            g_fluisynth_soundfont = m_soundfontTextCtrl->GetValue();
 
            prefs->setValue(SETTING_ID_AUDIO_EXPORT_ENGINE, wxString::Format(wxT("%i"), g_export_engine));
            prefs->setValue(SETTING_ID_FLUIDSYNTH_SOUNDFONT_PATH, g_fluisynth_soundfont);
        }
        
        void onChange(wxCommandEvent& evt)
        {
            bool enable;
            
            //printf("-> %i\n", m_radioBox->GetSelection());
            g_export_engine = (AudioExportEngine)m_radioBox->GetSelection();
            enable = (g_export_engine == FLUIDSYNTH);
            m_soundfontTextCtrl->Enable(enable);
            m_browseButton->Enable(enable);
        }
        
        void onButtonClicked(wxCommandEvent& evt)
        {
            wxFileName soundFontFile(m_soundfontTextCtrl->GetValue());

            wxString soundFontFilePath = showFileDialog(this, _("Select file"), soundFontFile.GetPath(),
                                            soundFontFile.GetFullName(), wxString(_("Sound Font"))+wxT(" (*.sf2)|*.sf2"), false /* open */);
            
            if (!soundFontFilePath.IsEmpty())
            {
                m_soundfontTextCtrl->SetValue(soundFontFilePath);
            }
        }
    };
    
    virtual bool audioExportSetup()
    {
        AudioExportDialog dlg( getMainFrame() );
        return dlg.ShowModal() == wxID_OK;
    }
    
    virtual void exportAudioFile(Sequence* sequence, wxString filepath)
    {
        g_sequence = sequence;
        g_export_audio_filepath = filepath;
        
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
