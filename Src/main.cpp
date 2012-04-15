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

#include <wx/stdpaths.h>
#include <wx/config.h>
#include <wx/msgdlg.h>
#include <wx/log.h>

#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/KeyPresets.h"
#include "PreferencesData.h"
#include "languages.h"
#include "UnitTest.h"
#include "Utils.h"

#include <iostream>

#include "main.h"

IMPLEMENT_APP(AriaMaestosa::wxWidgetApp)

using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(wxWidgetApp,wxApp)
EVT_ACTIVATE_APP(wxWidgetApp::onActivate)
EVT_IDLE(wxWidgetApp::onIdle)
END_EVENT_TABLE()

// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::activateRenderLoop(bool on)
{
    if (on and not m_render_loop_on)
    {
        Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        m_render_loop_on = true;
    }

    else if (not on and m_render_loop_on)
    {
        Disconnect( wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        m_render_loop_on = false;
    }
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::onIdle(wxIdleEvent& evt)
{
    if (m_render_loop_on)
    {
        frame->getMainPane()->playbackRenderLoop();
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
    wxLogVerbose( wxT("wxWidgetsApp::OnInit (enter)") );
    m_render_loop_on = false;
    
    frame = NULL;
    
    wxLog::SetActiveTarget( new wxLogStderr() );

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
        else if (wxString(argv[n]) == wxT("--verbose"))
        {
            wxLog::SetLogLevel(wxLOG_Info);
            wxLog::SetVerbose(true);
        }
    }
    
    wxLogVerbose( wxT("[main] init preferences") );
    prefs = PreferencesData::getInstance();
    prefs->init();
    
    if (prefs->getBoolValue(SETTING_ID_CHECK_NEW_VERSION))
    {
        checkVersionOnline();
    }
    
    Core::setPlayDuringEdit((PlayDuringEditMode)PreferencesData::getInstance()->getIntValue("playDuringEdit"));
    
    //read presets
    KeyPresetGroup::getInstance();
    
    wxLogVerbose( wxT("[main] init midi player") );
    PlatformMidiManager::get()->initMidiPlayer();

    wxLogVerbose( wxT("[main] init main frame") );
    frame = new MainFrame();
    AriaMaestosa::setCurrentSequenceProvider(frame);
    frame->init();

    wxLogVerbose( wxT("[main] init main frame 2") );

    frame->updateHorizontalScrollbar(0);

    Display::render();

    SetTopWindow(frame);

    wxLogVerbose( wxT("[main] init main frame 3") );
    
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

    wxLogVerbose( wxT("wxWidgetsApp::OnInit (leave)") );
    return true;
}
    
// ------------------------------------------------------------------------------------------------------

int wxWidgetApp::OnExit()
{
    wxLogVerbose( wxT("wxWidgetsApp::OnExit") );
    
#ifdef _MORE_DEBUG_CHECKS
    MemoryLeaks::checkForLeaks();
#endif
    return 0;
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::MacOpenFile(const wxString &fileName)
{
    wxLogVerbose( wxT("wxWidgetsApp::MacOpenFile") );
    
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

bool wxWidgetApp::OnExceptionInMainLoop()
{
    wxString what = wxT("Unknown error");
    
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        what = wxString(e.what(), wxConvUTF8);
    }
    
    std::cerr << "/!\\ An internal error occurred : an exception was caught unhandled\n" << what.mb_str()
               << std::endl;
    wxMessageBox(_("Sorry an internal error occurred : an exception was caught unhandled : ") + what);
    return true;
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::OnUnhandledException()
{
    wxString what = wxT("Unknown error");
    
    try
    {
        throw;
    }
    catch (std::exception& e)
    {
        what = wxString(e.what(), wxConvUTF8);
    }
    
    std::cerr << "/!\\ An internal error occurred : an exception was caught unhandled\n" << what.mb_str()
              << std::endl;
    wxMessageBox(_("Sorry an internal error occurred : an exception was caught unhandled : ") + what);
}