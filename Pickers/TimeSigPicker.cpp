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

#include "Pickers/TimeSigPicker.h"
#include "GUI/MainFrame.h"

#include <iostream>
#include "Config.h"
#include "AriaCore.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa {
	
    
DEFINE_EVENT_TYPE(wxEVT_DESTROY_TIMESIG_PICKER)
		
BEGIN_EVENT_TABLE(TimeSigPicker, wxDialog)

EVT_TEXT(1, TimeSigPicker::textNumChanged)
EVT_TEXT(2, TimeSigPicker::textDenomChanged)
EVT_TEXT_ENTER(1, TimeSigPicker::enterPressed)
EVT_TEXT_ENTER(2, TimeSigPicker::enterPressed)
    
EVT_CLOSE(TimeSigPicker::closed)
	
END_EVENT_TABLE()

TimeSigPicker* timesigpicker_frame = NULL;


namespace TimeSigPickerNames
{
    
    class QuickBoxLayout
        {
            wxBoxSizer* bsizer;
        public:
            wxPanel* pane;
            
            QuickBoxLayout(wxWindow* component, wxSizer* parent, int orientation=wxHORIZONTAL)
            {
                pane = new wxPanel(component);
                parent->Add(pane,1,wxEXPAND);
                bsizer = new wxBoxSizer(orientation);
            }
            void add(wxWindow* window)
            {
                bsizer->Add(window, 1, wxALL, 10);
            }
            ~QuickBoxLayout()
            {
                pane->SetSizer(bsizer);
                bsizer->Layout();
                //bsizer->SetSizeHints(pane);
            }
        };
    
#ifdef __WXMAC__
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
                    timesigpicker_frame->closeWindow();
                }
                else if(evt.GetKeyCode()==WXK_RETURN)
                {
                    wxCommandEvent dummyEvt;
                    timesigpicker_frame->enterPressed(dummyEvt);
                }
                else wxEvtHandler::ProcessEvent(event);
                    
                return true;
            }
            else
            {
                wxEvtHandler::ProcessEvent(event);
                return true;
            }
    }
        
    };
    #endif
}
using namespace TimeSigPickerNames;
    
void freeTimeSigPicker()
{
		if(timesigpicker_frame != NULL)
		{
			timesigpicker_frame->Destroy();
			timesigpicker_frame = NULL;
		}

}
	
void showTimeSigPicker()
{
	if(timesigpicker_frame == NULL) timesigpicker_frame = new TimeSigPicker();
	timesigpicker_frame->show();
}

TimeSigPicker::TimeSigPicker() : wxDialog(NULL, 0,  _("Key Signature"), wxDefaultPosition, wxSize(50,160), wxSTAY_ON_TOP )
{
    pane = new wxPanel(this);
    
    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;
    
    // FIXME - query actual values from sequence
    valueTextNum=new wxTextCtrl(pane, 1, wxT("4"), wxPoint(0,130), smallsize, wxTE_PROCESS_ENTER);
    wxStaticText* slash = new wxStaticText(pane, wxID_ANY, wxT("/"), wxDefaultPosition, wxSize(15,15));
    slash->SetMinSize(wxSize(15,15));
    slash->SetMaxSize(wxSize(15,15));
    valueTextDenom=new wxTextCtrl(pane, 2, wxT("4"), wxPoint(0,130), smallsize, wxTE_PROCESS_ENTER);
    okbtn = new wxButton(pane, 3, _("OK"));
    okbtn->SetDefault();
    
    wxBoxSizer* vertical = new wxBoxSizer(wxHORIZONTAL);
    
    QuickBoxLayout horizontal(pane, vertical);
    horizontal.add(valueTextNum);
    horizontal.add(slash);
    horizontal.add(valueTextDenom);
    
    vertical->Add(okbtn);
    
    pane->SetSizer(vertical);


#ifdef __WXMAC__
    PushEventHandler( new MyEvtHandler() );
    pane->PushEventHandler( new MyEvtHandler() );
#else
    Connect(GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    slider->Connect(valueTextNum->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    pane->Connect(pane->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    valueText->Connect(valueTextDenom->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
#endif
}

void TimeSigPicker::closed(wxCloseEvent& evt)
{
}

void TimeSigPicker::show()
{
    // show the keysig picking dialog
    // FIXME- use real position, query real values from sequence
    int x=0, y=0;
    SetPosition(wxPoint(x,y));

    returnCode = ShowModal();
}

void TimeSigPicker::textNumChanged(wxCommandEvent& evt)
{
    // FIXME - verify for sanity, or is it done in sequence?
}
void TimeSigPicker::textDenomChanged(wxCommandEvent& evt)
{
    // FIXME - verify for sanity, or is it done in sequence?
}
    
void TimeSigPicker::enterPressed(wxCommandEvent& evt)
{
    int top = atoi_u( valueTextNum->GetValue() );
    int bottom = atoi_u( valueTextDenom->GetValue() );
    
    if(bottom < 1 or top<1 or bottom>32 or top>32 or
      not valueTextNum->GetValue().IsNumber() or not valueTextDenom->GetValue().IsNumber())
    {
        wxBell();
        return;
    }
    
    getMeasureData()->setTimeSig( top, bottom );
    
    closeWindow();
}

void TimeSigPicker::closeWindow()
{
#ifdef __WXMAC__
    PopEventHandler(true);
    pane->PopEventHandler(true);
#endif
    
	wxDialog::EndModal(returnCode);
    Display::requestFocus();
    
	wxCommandEvent event( wxEVT_DESTROY_TIMESIG_PICKER, 100000 );
	getMainFrame()->GetEventHandler()->AddPendingEvent( event );
	
}

void TimeSigPicker::keyPress(wxKeyEvent& evt)
{
    if(evt.GetKeyCode()==WXK_ESCAPE || evt.GetKeyCode()==WXK_CANCEL || evt.GetKeyCode()==WXK_DELETE)
    {
        closeWindow();
    }
    else if(evt.GetKeyCode() == WXK_RETURN)
    {
        wxCommandEvent dummyEvt;
        enterPressed(dummyEvt);
    }
    else evt.Skip(true);
}

}
