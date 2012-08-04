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
#include <wx/snglinst.h>
#include <wx/ipc.h>      // IPC support
#include <wx/filename.h>
#include <wx/tokenzr.h>

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

static const wxString IPC_START = wxT("StartOther");
static const wxString IPC_APP_PORT = wxT("4242");
static const wxString CONNECTION_FILE_SEPARATOR = wxT("|");


using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(wxWidgetApp,wxApp)
EVT_ACTIVATE_APP(wxWidgetApp::onActivate)
EVT_IDLE(wxWidgetApp::onIdle)
END_EVENT_TABLE()



//----------------------------------------------------------------------------
//! IPC connection
class AppIPCConnection : public wxConnection
{

public:

#if wxCHECK_VERSION(2,9,1)
   bool OnExecute(const wxString& WXUNUSED(topic),
                  const void* data,
                  size_t size,
                  wxIPCFormat WXUNUSED(format))
#else
   bool OnExecute(const wxString& WXUNUSED(topic),
                  wxChar *data,
                  int size,
                  wxIPCFormat WXUNUSED(format))
#endif
    {
#if wxCHECK_VERSION(2,9,1)
        wxString concatenatedPaths((char*)data, wxConvUTF8);
#else
        wxString concatenatedPaths(data, wxConvUTF8);
#endif
        wxString path;
        
        std::cout << "[AppIPCConnection] OnExecute : " << size << " : " << concatenatedPaths.mb_str() << std::endl;
        
        wxStringTokenizer tokenizer(concatenatedPaths, CONNECTION_FILE_SEPARATOR);
        while ( tokenizer.HasMoreTokens() )
        {
            path = tokenizer.GetNextToken();
            wxGetApp().loadFile(path);
        }
    
        return true;
    }

};




//----------------------------------------------------------------------------
//! IPC server
class AppIPCServer : public wxServer
{
public:
        
    //! accept connection handler
    virtual wxConnectionBase *OnAcceptConnection (const wxString& topic)
    {
        if (topic != IPC_START)
        {
             return NULL;
        }
        else
        {
            return new AppIPCConnection;
        }
    }
};



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
    
    PlatformMidiManager* pmm = PlatformMidiManager::get();
    if (pmm->isRecording())
    {
        pmm->processRecordQueue();
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
    wxString appName;
    
    wxLog::SetActiveTarget(new wxLogStderr());
    wxLogVerbose( wxT("wxWidgetsApp::OnInit (enter)") );
    std::cout << "[main] wxWidgetsApp::OnInit (enter)" << std::endl;

    m_render_loop_on = false;
    frame = NULL;
    appName = GetAppName();
    
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
    
    m_single_instance_checker = NULL;
    
#ifndef __WXMAC__
    m_single_instance_checker = new wxSingleInstanceChecker(appName + wxGetUserId(), wxT("/tmp/"));
    
    if ( prefs->getBoolValue(SETTING_ID_SINGLE_INSTANCE_APPLICATION, true) &&
        m_single_instance_checker->IsAnotherRunning() )
    {
        std::cout << "[main] detected another Aria instance" << std::endl;
        if (!handleSingleInstance())
        {
            return false;
        }
    }
    
      // IPC server
    m_IPC_server = new AppIPCServer();
    if ( m_IPC_server->Create(IPC_APP_PORT) )
    {
        std::cout << "[main] AppIPCServer created" << std::endl;
    }
    else
    {
        wxDELETE(m_IPC_server);
    }

    if (prefs->getBoolValue(SETTING_ID_CHECK_NEW_VERSION, true))
    {
        checkVersionOnline();
    }
#endif

    Core::setPlayDuringEdit((PlayDuringEditMode)PreferencesData::getInstance()->getIntValue("playDuringEdit"));
    
    //read presets
    KeyPresetGroup::getInstance();
    
    wxLogVerbose( wxT("[main] init midi player") );
    PlatformMidiManager::get()->initMidiPlayer();

    wxLogVerbose( wxT("[main] init main frame") );
    frame = new MainFrame();
    AriaMaestosa::setCurrentSequenceProvider(frame);
    
    // check if filenames to open were given on the command-line
    wxArrayString filesToOpen;
    for (int n=1 ; n<argc ; n++)
    {
        wxString fileName = wxString(argv[n]);
        filesToOpen.Add(fileName);
        //loadFile(fileName);
    }
    
    frame->init(filesToOpen);

    wxLogVerbose( wxT("[main] init main frame 2") );

    frame->updateHorizontalScrollbar(0);

    Display::render();

    SetTopWindow(frame);

    wxLogVerbose( wxT("[main] init main frame 3") );

    wxLogVerbose( wxT("wxWidgetsApp::OnInit (leave)") );
    return true;
}
    
// ------------------------------------------------------------------------------------------------------

int wxWidgetApp::OnExit()
{
    wxLogVerbose( wxT("wxWidgetsApp::OnExit") );

    wxDELETE(m_single_instance_checker);
    
#ifdef _MORE_DEBUG_CHECKS
    MemoryLeaks::checkForLeaks();
#endif
    return 0;
}

// ------------------------------------------------------------------------------------------------------

void wxWidgetApp::MacOpenFile(const wxString &fileName)
{
    wxLogVerbose( wxT("wxWidgetsApp::MacOpenFile") );
    
    loadFile(fileName);
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


bool wxWidgetApp::handleSingleInstance()
{
    wxClient client;
    wxConnectionBase* conn = client.MakeConnection(wxEmptyString, IPC_APP_PORT, IPC_START);
    
    if (conn!=NULL)
    {
        wxString path;
    
        for (int i = 1; i < argc; ++i)
        {
            path += wxString(argv[i]);
            path += CONNECTION_FILE_SEPARATOR;
        }
        
        if (conn->Execute(path))
        {
            std::cout << "Another program instance is already running, aborting" << std::endl;
            std::cout << "Passing " << path.c_str() << " : " << path.Length() << std::endl;
            wxDELETE(m_single_instance_checker); // OnExit() won't be called if we return false
            return false;
        }
    }
    
    wxDELETE(conn);
    
    return true;
}


void wxWidgetApp::loadFile(const wxString& fileName)
{
    frame->loadFile(fileName);
}
