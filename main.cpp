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


#include "GUI/MainFrame.h"
#include "GUI/GLPane.h"
#include "GUI/MainPane.h"
#include "GUI/MeasureBar.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Images/ImageProvider.h"
#include "IO/IOUtils.h"

#include <iostream>

#include "Config.h"	

#ifdef _MORE_DEBUG_CHECKS
#include "LeakCheck.h"
#endif

#include "languages.h"
#include "main.h"

IMPLEMENT_APP(AriaMaestosa::wxWidgetApp)

namespace AriaMaestosa {

bool render_loop_on = false;

BEGIN_EVENT_TABLE(wxWidgetApp,wxApp)
EVT_ACTIVATE_APP(wxWidgetApp::onActivate)
EVT_IDLE(wxWidgetApp::onIdle)
END_EVENT_TABLE()
	
wxConfig* prefs;

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
    if(render_loop_on)
    {
        frame->mainPane->playbackRenderLoop();
        evt.RequestMore();
    }
}

void wxWidgetApp::onActivate(wxActivateEvent& evt)
{
	if(evt.GetActive())
	{
		frame->Raise();
	}
}

bool wxWidgetApp::OnInit()
{
    frame = NULL;
	prefs = (wxConfig*) wxConfig::Get();
	
	initLanguageSupport(prefs);
	
    PlatformMidiManager::initMidiPlayer();

    frame=new MainFrame();
	frame->init();
    Display::render();

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
