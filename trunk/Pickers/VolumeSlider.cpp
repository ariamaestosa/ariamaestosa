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

#include "Pickers/VolumeSlider.h"
#include "Midi/Note.h"
#include "Midi/Track.h"
#include "GUI/GLPane.h"
#include "GUI/MainFrame.h"
#include "Actions/EditAction.h"
#include "Actions/SetNoteVolume.h"

#include <iostream>
#include "Config.h"
#include "main.h"

namespace AriaMaestosa {
	
DEFINE_EVENT_TYPE(wxEVT_DESTROY_VOLUME_SLIDER)
		
BEGIN_EVENT_TABLE(VolumeSlider, wxDialog)

EVT_COMMAND_SCROLL_THUMBTRACK(1, VolumeSlider::volumeSlideChanging)
EVT_COMMAND_SCROLL_THUMBRELEASE(1, VolumeSlider::volumeSlideChanged)
EVT_COMMAND_SCROLL_LINEUP(1, VolumeSlider::volumeSlideChanging)
EVT_COMMAND_SCROLL_LINEDOWN(1, VolumeSlider::volumeSlideChanging)
EVT_COMMAND_SCROLL_PAGEUP(1, VolumeSlider::volumeSlideChanging)
EVT_COMMAND_SCROLL_PAGEDOWN(1, VolumeSlider::volumeSlideChanging)

EVT_TEXT(2, VolumeSlider::volumeTextChanged)
EVT_TEXT_ENTER(2, VolumeSlider::enterPressed)
EVT_KEY_DOWN(VolumeSlider::keyPress)

EVT_CLOSE(VolumeSlider::closed)
	
END_EVENT_TABLE()

VolumeSlider* slider = NULL;
	

class MyEvtHandler : public wxEvtHandler
{
public:
	
	virtual bool ProcessEvent(wxEvent& event)
	{
		if(event.GetEventType() == wxEVT_KEY_DOWN or event.GetEventType() == wxEVT_CHAR)
		{
			
			wxKeyEvent& evt = dynamic_cast<wxKeyEvent&>(event);
			
			if(evt.GetKeyCode()==WXK_ESCAPE || evt.GetKeyCode()==WXK_CANCEL || evt.GetKeyCode()==WXK_DELETE)
			{
				slider->closeWindow();
			}
			
			if(evt.GetKeyCode()==WXK_RETURN)
			{
				wxCommandEvent dummyEvt;
				slider->enterPressed(dummyEvt);
			}
			return true;
		}
		else
		{
			wxEvtHandler::ProcessEvent(event);
			return true;
		}
	}

};

void freeVolumeSlider()
	{
		if(slider != NULL)
		{
			slider->Destroy();
			slider = NULL;
		}

	}
	
void showVolumeSlider(int x, int y, int noteID, Track* track)
{
	if(slider == NULL) slider = new VolumeSlider();
	slider->show(x,y,noteID, track);
}
	
VolumeSlider::VolumeSlider() : wxDialog(NULL, 0,  _("volume"), wxDefaultPosition, wxSize(50,160), wxSTAY_ON_TOP )
{
	INIT_LEAK_CHECK();
	
    
    // FIXME- is it automatically destroyed?
	PushEventHandler( new MyEvtHandler() );
	
    /*
      http://wxforum.shadonet.com/viewtopic.php?t=16966
     You pointed out an important issue regarding the memory leaks possibilities. Like you, I didn't find in the documentation any explicit mention
     as to perform the PopEventHandler call. However, in the "event" sample provided with wxWidgets, they do call it in the frame's destructor,
     stating that "we must pop any remaining event handlers to avoid memory leaks and crashes!". Therefore, I decided to do so.

     void xxx::delete_key_event_handlers(wxWindow * current_window)
     {
         // Delete the key event handler from every control in the window (to avoid memory leaks)
         wxWindow * child;
         wxWindowList children = current_window->GetChildren();
         for ( wxWindowListNode * node = children.GetFirst(); node; node = node->GetNext() )
         {
             // Delete the handler
             child = node->GetData();
             child->PopEventHandler(true);
             
             // Recursion on the child's children
             delete_key_event_handlers(child);
         }
     }
     
        */
    
    slider=new wxSlider(this, 1, 60, 0, 127, wxDefaultPosition, wxSize(50,128), wxSL_VERTICAL | wxSL_INVERSE);

    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;

    valueText=new wxTextCtrl(this, 2, wxT("0"), wxPoint(0,130), smallsize, wxTE_PROCESS_ENTER);
    noteID=-1;
    currentTrack=NULL;
}

void VolumeSlider::closed(wxCloseEvent& evt)
{
	//std::cout << "***** close event *****" << std::endl;
	//closeWindow();
}

void VolumeSlider::show(int x, int y, int noteID, Track* track)
{
    // show the volume picking dialog
    
	if(track == NULL) return;

    VolumeSlider::currentTrack=track;
    VolumeSlider::noteID=noteID;
    
    SetPosition(wxPoint(x,y));
    slider->SetValue(currentTrack->getNoteVolume(noteID));
    
    char buffer[3];
    sprintf (buffer, "%d", currentTrack->getNoteVolume(noteID));
    valueText->SetValue( fromCString(buffer) );
        
    returnCode=ShowModal();
	//SetFocus();
}

void VolumeSlider::volumeSlideChanged(wxScrollEvent& evt)
{

	if(currentTrack == NULL) return;
	
    if(noteID!=-1)
	{
		
		if(currentTrack->isNoteSelected(noteID))
			currentTrack->action( new Action::SetNoteVolume(slider->GetValue(), SELECTED_NOTES) );
		else
			currentTrack->action( new Action::SetNoteVolume(slider->GetValue(), noteID) );
		
	}
	
    // close the volume picking dialog
    closeWindow();
	return;
}


void VolumeSlider::volumeSlideChanging(wxScrollEvent& evt)
{
    const int newValue = slider->GetValue();
    char buffer[3];
    sprintf (buffer, "%d", newValue);
    valueText->SetValue( fromCString(buffer) );
}

void VolumeSlider::volumeTextChanged(wxCommandEvent& evt)
{
}

void VolumeSlider::enterPressed(wxCommandEvent& evt)
{
        
        if(!valueText->GetValue().IsNumber())
	{
            wxBell();
            const int newValue = slider->GetValue();
            char buffer[3];
            sprintf (buffer, "%d", newValue);
            valueText->SetValue( fromCString(buffer) );
            return;
        }
        
        const int newValue = atoi_u(valueText->GetValue());
        if(newValue<0)
		{
            wxBell();
            valueText->SetValue(wxT("0"));
            return;
        }
		
        if(newValue>127)
		{
            wxBell();
            valueText->SetValue(wxT("127"));
            return;
        }
        
        assert(noteID!=-1);
        assert(currentTrack!=NULL);
        
        if(currentTrack->isNoteSelected(noteID))
            currentTrack->action( new Action::SetNoteVolume(newValue, SELECTED_NOTES) );
		else
            currentTrack->action( new Action::SetNoteVolume(newValue, noteID) );
        
        closeWindow();

}

void VolumeSlider::closeWindow()
{
	PopEventHandler(true);
	wxDialog::EndModal(returnCode);
	currentTrack=NULL;
	getGLPane()->SetFocus();
	
	wxCommandEvent event( wxEVT_DESTROY_VOLUME_SLIDER, 100000 );
	getMainFrame()->GetEventHandler()->AddPendingEvent( event );
	
}

void VolumeSlider::keyPress(wxKeyEvent& evt)
{
}

}
