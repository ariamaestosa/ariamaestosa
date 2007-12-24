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
#include <wx/stdpaths.h>
#include <wx/config.h>

#include "main.h"

#include "GUI/MainFrame.h"
#include "GUI/GLPane.h"
#include "GUI/MeasureBar.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Images/ImageProvider.h"
#include "IO/IOUtils.h"

#include <iostream>

#include "Config.h"	

#ifdef __WXGTK__
#include "wx/stdpaths.h"
#endif

IMPLEMENT_APP(AriaMaestosa::wxWidgetApp)

#ifdef _MORE_DEBUG_CHECKS
#include "LeakCheck.h"
#endif

namespace AriaMaestosa {
	
bool render_loop_on = false;

BEGIN_EVENT_TABLE(wxWidgetApp,wxApp)
EVT_ACTIVATE_APP(wxWidgetApp::onActivate)
EVT_IDLE(wxWidgetApp::onIdle)
END_EVENT_TABLE()
	
MainFrame* frame = NULL;
wxLocale* locale;

MainFrame* getMainFrame(){ assert(frame != NULL); return frame; }
GLPane* getGLPane() { return frame->glPane; }
MeasureBar* getMeasureBar() { return frame->getCurrentSequence()->measureBar; }

wxConfig* prefs;

void activateRenderLoop(bool on)
{
    ((wxWidgetApp*)wxTheApp)->activateRenderLoop(on);
}
void wxWidgetApp::activateRenderLoop(bool on)
{
    if(on and !render_loop_on)
    {
        Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        render_loop_on = true;
    }
    
    else if(!on and render_loop_on)
    {
        Disconnect( wxEVT_IDLE, wxIdleEventHandler(wxWidgetApp::onIdle) );
        render_loop_on = false;
    }
}

void wxWidgetApp::onIdle(wxIdleEvent& evt)
{
//if(render_loop_on) std::cout << "rendering loop" << std::endl;

    if(render_loop_on)
    {
        frame->glPane->playbackRenderLoop();
        evt.RequestMore();
    }
}

void wxWidgetApp::onActivate(wxActivateEvent& evt)
{
	if(evt.GetActive())
	{
		frame->Raise();
	}
	//std::cout << "void wxWidgetApp::onActivate(wxActivateEvent& evt)" << std::endl;
}

bool wxWidgetApp::OnInit()
{
	//checkAppPath();
	
	prefs = (wxConfig*) wxConfig::Get();
	
	long language;
	
	if(! prefs->Read( wxT("lang"), &language) )
	{
		language = wxLANGUAGE_DEFAULT;
		std::cout << "failed to read prefs" << std::endl;
	}

	if(language == wxLANGUAGE_DEFAULT) std::cout << "language : default" << std::endl;
	else if(language == wxLANGUAGE_ENGLISH) std::cout << "language : english" << std::endl;
	else if(language == wxLANGUAGE_FRENCH) std::cout << "language : french" << std::endl;

	if(wxLocale::IsAvailable(language))
	{
	    locale = new wxLocale( language, wxLOCALE_CONV_ENCODING );

        #ifdef __WXGTK__
        // add locale search paths
        locale->AddCatalogLookupPathPrefix(wxT("/usr"));
        locale->AddCatalogLookupPathPrefix(wxT("/usr/local"));
        wxStandardPaths* paths = (wxStandardPaths*) &wxStandardPaths::Get();
        wxString prefix = paths->GetInstallPrefix();
        locale->AddCatalogLookupPathPrefix( prefix );
        #endif

	    locale = new wxLocale( language, wxLOCALE_CONV_ENCODING );
        locale->AddCatalog(wxT("aria_maestosa"));

	    if(! locale->IsOk() )
	    {
		    std::cout << "selected language is wrong" << std::endl;
		    delete locale;
		    locale = new wxLocale( wxLANGUAGE_ENGLISH );
	    }
    }
    else
    {
        std::cout << "The selected language is not supported by your system."
                  << "Try installing support for this language." << std::endl;
		locale = new wxLocale( wxLANGUAGE_ENGLISH );
    }

	// wxLANGUAGE_DEFAULT
	// wxLANGUAGE_ENGLISH
	// wxLANGUAGE_FRENCH
	
    PlatformMidiManager::initMidiPlayer();

    frame=new MainFrame();
	frame->init();
	frame->glPane->render();
    frame->glPane->swapBuffers();
	
	SetTopWindow(frame);
				 
    return true;
}


int wxWidgetApp::OnExit()
{
#ifdef _CHECK_FOR_LEAKS
	MemoryLeaks::checkForLeaks();
#endif
    return 0;
}


void wxWidgetApp::MacOpenFile(const wxString &fileName)
{

	if(fileName.EndsWith(wxT("aria")))
	{
		frame->loadAriaFile( fileName );	
	}
	else if(fileName.EndsWith(wxT("mid")) or fileName.EndsWith(wxT("midi")))
	{
		frame->loadMidiFile( fileName );	
	}
	else
	{
		wxMessageBox(_("Unknown file type: ") + fileName);	
	}
	
}

}
