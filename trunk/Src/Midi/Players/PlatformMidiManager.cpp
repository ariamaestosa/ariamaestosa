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

#include "AriaCore.h"

#include "Actions/AddNote.h"
#include "Actions/AddControlEvent.h"
#include "Actions/Record.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "PreferencesData.h"
#include "ptr_vector.h"
#include "Utils.h"
#include <wx/intl.h>
#include <wx/msgdlg.h>

#include "RtMidi.h"

using namespace AriaMaestosa;

ptr_vector<PlatformMidiManagerFactory, REF>* g_all_midi_managers = NULL;
PlatformMidiManager* g_manager = NULL;

// ----------------------------------------------------------------------------------------------------------

PlatformMidiManager::PlatformMidiManager()
{
    m_recording = false;
    m_record_action = NULL;
    m_playthrough = PreferencesData::getInstance()->getIntValue(SETTING_ID_PLAYTHROUGH);
}

// ----------------------------------------------------------------------------------------------------------

std::vector<wxString> PlatformMidiManager::getChoices()
{
    std::vector<wxString> out;
    const int amount = g_all_midi_managers->size();
    for (int n=0; n<amount; n++)
    {
        out.push_back(g_all_midi_managers->get(n)->getName());
    }
    return out;
}

// ----------------------------------------------------------------------------------------------------------

PlatformMidiManager* PlatformMidiManager::get()
{
    if (g_all_midi_managers == NULL)
    {
        // FIXME: instead of aborting, fire a no-op driver
        wxMessageBox(_("Bad binary, no MIDI driver was compiled in!"));
        fprintf(stderr, "Bad binary, no MIDI driver was compiled in!");
        exit(1);
    }
    
    if (g_manager == NULL)
    {
        wxString preferredMidiDriver = PreferencesData::getInstance()->getValue(SETTING_ID_MIDI_DRIVER);

        const int count = g_all_midi_managers->size();
        for (int n=0; n<count; n++)
        {
            if (g_all_midi_managers->get(n)->getName() == preferredMidiDriver)
            {
                g_manager = g_all_midi_managers->get(n)->newInstance();
                break;
            }
        }
        
        if (g_manager == NULL)
        {
            // TODO: fix invalid preferences value?
            g_manager = g_all_midi_managers->get(0)->newInstance();
        }
    }
    return g_manager;
}

// ----------------------------------------------------------------------------------------------------------

void PlatformMidiManager::registerManager(PlatformMidiManagerFactory* newManager)
{
    // hack to make sure the vector is always created before using it
    static bool firstTime = true;
    if (firstTime)
    {
        g_all_midi_managers = new ptr_vector<PlatformMidiManagerFactory, REF>();
        firstTime = false;
    }
    
    g_all_midi_managers->push_back(newManager);
}

// ----------------------------------------------------------------------------------------------------------

wxArrayString PlatformMidiManager::getInputChoices()
{
    wxArrayString out;
    RtMidiIn* midiin = new RtMidiIn();
    
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 0)
    {
        // No input port is available
        return out;
    }
    
    std::string portName;
    for (unsigned int i=0; i<nPorts; i++)
    {
        try
        {
            portName = midiin->getPortName(i);
        }
        catch (RtError &error)
        {
            error.printMessage();
            continue;
        }
        std::cout << "  Input Port #" << i+1 << ": " << portName << '\n';
        out.Add( wxString(portName.c_str(), wxConvUTF8) );
    }
    delete midiin;
    return out;
}

// ----------------------------------------------------------------------------------------------------------

void PlatformMidiManager::recordCallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
    PlatformMidiManager* self = (PlatformMidiManager*)userData;
    
    ASSERT( MAGIC_NUMBER_OK_FOR(*self) );
    
    // self->m_open_notes
    unsigned int nBytes = message->size();
    
    if (nBytes >= 3)
    {
        int messageType = message->at(0) & 0xF0;
        int channel = message->at(0) & 0x0F;
        int value = message->at(1);
        int value2 = message->at(2);
        
        //printf("message %x on channel %i = %i %i\n", messageType, channel, value, value2);
        
        int now_tick = self->m_start_tick + self->getAccurateTick();
        
        switch (messageType)
        {
            case 0x90:
            case 0x80:
                if (messageType == 0x90 and value2 > 0)
                {
                    //printf("NOTE ON on channel %i; note : %i velocity : %i\n", channel, value, value2);
                    
                    // FIXME: we are in a thread, not all players may be thread-safe!!
                    if (self->m_playthrough) self->seq_note_on(value, value2, self->m_record_target->getChannel());
                    
                    NoteInfo n = {now_tick, value2};
                    self->m_open_notes[value] = n;
                }
                else
                {
                    //printf("NOTE OFF on channel %i; note : %i velocity : %i\n", channel, value, value2);
                    
                    // FIXME: we are in a thread, not all players may be thread-safe!!
                    if (self->m_playthrough) self->seq_note_off(value, self->m_record_target->getChannel());
                    
                    if (self->m_open_notes.find(value) != self->m_open_notes.end())
                    {
                        NoteInfo n = self->m_open_notes[value];
                        self->m_open_notes.erase(value);
                        
                        if (self->m_record_action != NULL)
                        {
                            self->m_record_action->action(new Action::AddNote(131 - value,
                                                                              n.m_note_on_tick,
                                                                              now_tick,
                                                                              n.m_velocity,
                                                                              false));
                        }
                    }
                }
                break;
                
            case 0xC0:
                //printf("PROGRAM CHANGE on channel %i; instrument : %i\n", channel, value);
                break;
                
            case 0xE0:
            {
                float val = ControllerEvent::fromPitchBendValue((value | (value2 << 7)) - 8192);
                
                if (self->m_record_action != NULL)
                {
                    self->m_record_action->action(new Action::AddControlEvent(now_tick, val, PSEUDO_CONTROLLER_PITCH_BEND));
                }
                
                // FIXME: we are in a thread, not all players may be thread-safe!!
                if (self->m_playthrough) self->seq_pitch_bend((value | (value2 << 7)) - 8192,
                                                              self->m_record_target->getChannel());

                break;
            }
            case 0xB0:
                if (self->m_record_action != NULL)
                {
                    self->m_record_action->action(new Action::AddControlEvent(now_tick, 127 - value2, value));
                }
                
                // FIXME: we are in a thread, not all players may be thread-safe!!
                if (self->m_playthrough) self->seq_controlchange(value, value2,
                                                                 self->m_record_target->getChannel());

                
                break;
                
            default:
                printf("UNKNOWN EVENT %x on channel %i; value : %i %i\n", messageType, channel, value, value2);

        }
    }
    
    /*
    for ( unsigned int i=0; i<nBytes; i++ )
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        std::cout << "stamp = " << deltatime << std::endl;
     */
}

// ----------------------------------------------------------------------------------------------------------

bool PlatformMidiManager::startRecording(wxString outputPort, Track* target)
{
    m_record_target = target;
    m_midi_input = new RtMidiIn();
    
    // Check available ports.
    unsigned int nPorts = m_midi_input->getPortCount();
    if (nPorts == 0)
    {
        wxMessageBox(_("Sorry, no MIDI input port is available"));
        return false;
    }
    
    std::string portName;
    int portId = -1;
    for (unsigned int i=0; i<nPorts; i++)
    {
        try
        {
            portName = m_midi_input->getPortName(i);
        }
        catch (RtError &error)
        {
            error.printMessage();
            continue;
        }
        
        if (outputPort == wxString(portName.c_str(), wxConvUTF8))
        {
            portId = i;
            break;
        }
    }
    
    
    if (portId == -1)
    {
        wxMessageBox( _("Sorry, failed to open the selected MIDI input port") );
        return false;
    }
    
    m_recording = true;
    m_record_action = new Action::Record();
    
    // add the action to the action stack so it can be undone
    m_record_target->action(m_record_action);
    
    try
    {
        m_midi_input->openPort( portId );
    }
    catch (std::exception& e)
    {
        m_recording = false;
        delete m_midi_input;
        m_midi_input = NULL;
        wxMessageBox( wxString(_("Sorry, failed to open the selected MIDI input port")) + wxT("\n") +
                      wxString(e.what(), wxConvUTF8) );
        return false;
    }
    // Set our callback function.  This should be done immediately after
    // opening the port to avoid having incoming messages written to the
    // queue.
    m_midi_input->setCallback( &recordCallback, this );
    
    // Don't ignore sysex, timing, or active sensing messages.
    m_midi_input->ignoreTypes( false, false, false );

    return true;
}

// ----------------------------------------------------------------------------------------------------------

void PlatformMidiManager::stopRecording()
{
    m_recording = false;
    
    try
    {
        m_midi_input->closePort();
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "[rtmidi] %s\n", e.what());
    }
    delete m_midi_input;
    m_midi_input = NULL;
    m_record_action = NULL;
}

// ----------------------------------------------------------------------------------------------------------

