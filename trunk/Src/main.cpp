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

#include "wx/wx.h"
#include "wx/stdpaths.h"
#include "wx/config.h"

#include "Utils.h"
#include "Dialogs/Preferences.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "GUI/ImageProvider.h"
#include "IO/IOUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Midi/KeyPresets.h"
#include "languages.h"
#include "unit_test.h"

#include <iostream>

#include "main.h"

IMPLEMENT_APP(AriaMaestosa::wxWidgetApp)

//FIXME: remove global, make it member
namespace AriaMaestosa
{
    bool render_loop_on = false;
}
using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(wxWidgetApp,wxApp)
EVT_ACTIVATE_APP(wxWidgetApp::onActivate)
EVT_IDLE(wxWidgetApp::onIdle)
END_EVENT_TABLE()

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::activateRenderLoop(bool on)
{
    if (on and !render_loop_on)
    {
        Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        render_loop_on = true;
    }

    else if (!on and render_loop_on)
    {
        Disconnect( wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        render_loop_on = false;
    }
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::onIdle(wxIdleEvent& evt)
{
    if (render_loop_on)
    {
        frame->mainPane->playbackRenderLoop();
        evt.RequestMore();
    }
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::onActivate(wxActivateEvent& evt)
{
#ifdef __WXMAC__
    if (evt.GetActive())
    {
        frame->Raise();
    }
#endif
}

// ------------------------------------------------------------------------------------------------------

bool wxWidgetApp::OnInit()
{
    for (int n=0; n<argc; n++)
    {
        if (wxString(argv[n]) == wxT("--utest"))
        {
            okToLog = false;
            Core::setPlayDuringEdit(PLAY_NEVER);
            prefs = PreferencesData::getInstance();
            prefs->init();

            UnitTestCase::showMenu();
            exit(0);
        }
    }
    
    frame = NULL;

#if USE_WX_LOGGING
    int x, y;

    m_log_window = new wxLogWindow(NULL, wxT("Log") );
    m_log_frame  = m_log_window->GetFrame();
    m_log_frame->SetWindowStyle(wxDEFAULT_FRAME_STYLE | wxSTAY_ON_TOP);

    m_log_frame->SetSize( wxRect(0,wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)*3/4, 400,250) );
    wxLog::SetActiveTarget(m_log_window);
#endif
    
    prefs = PreferencesData::getInstance();
    prefs->init();
    Core::setPlayDuringEdit((PlayDuringEditMode)Core::getPrefsValue("playDuringEdit"));
    
    //read presets
    KeyPresetGroup::getInstance();
    
    PlatformMidiManager::initMidiPlayer();

    frame = new MainFrame();
    AriaMaestosa::setCurrentSequenceProvider(frame);
    frame->init();

    frame->updateHorizontalScrollbar(0);

    Display::render();

    SetTopWindow(frame);

    // check if filenames to open were given on the command-line
    for (int n=0; n<argc; n++)
    {
        wxString fileName = wxString(argv[n]);
        if (fileName.EndsWith(wxT("aria")))
        {
            frame->loadAriaFile( fileName );
        }
        else if (fileName.EndsWith(wxT("mid")) or fileName.EndsWith(wxT("midi")))
        {
            frame->loadMidiFile( fileName );
        }
    }

    return true;
}

// ------------------------------------------------------------------------------------------------------

#if USE_WX_LOGGING

void wxWidgetApp::closeLogWindow()
{
    wxLog::SetActiveTarget(NULL);
    m_log_frame->Close();
    wxDELETE(m_log_window);
}

#endif
    
// ------------------------------------------------------------------------------------------------------

int wxWidgetApp::OnExit()
{
#ifdef _MORE_DEBUG_CHECKS
    MemoryLeaks::checkForLeaks();
#endif
    return 0;
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::MacOpenFile(const wxString &fileName)
{

    if (fileName.EndsWith(wxT("aria")))
    {
        frame->loadAriaFile( fileName );
    }
    else if (fileName.EndsWith(wxT("mid")) or fileName.EndsWith(wxT("midi")))
    {
        frame->loadMidiFile( fileName );
    }
    else
    {
        wxMessageBox(_("Unknown file type: ") + fileName);
    }

}

// ------------------------------------------------------------------------------------------------------
