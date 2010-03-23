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
#include "Utils.h"
#include "AriaCore.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa {


DEFINE_EVENT_TYPE(wxEVT_DESTROY_TIMESIG_PICKER) // actually do something with this event

BEGIN_EVENT_TABLE(TimeSigPicker, wxFrame)

//EVT_TEXT(1, TimeSigPicker::textNumChanged)
//EVT_TEXT(2, TimeSigPicker::textDenomChanged)
EVT_TEXT_ENTER(1, TimeSigPicker::enterPressed)
EVT_TEXT_ENTER(2, TimeSigPicker::enterPressed)
EVT_BUTTON(3, TimeSigPicker::enterPressed)

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
            void add(wxWindow* window, int margin=2, int proportion=1, long margins = wxALL)
            {
                bsizer->Add(window, proportion, margins | wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL, margin);
            }
            ~QuickBoxLayout()
            {
                pane->SetSizer(bsizer);
                bsizer->Layout();
                //bsizer->SetSizeHints(pane);
            }
        };

}
using namespace TimeSigPickerNames;

void freeTimeSigPicker()
{
    if (timesigpicker_frame != NULL)
    {
        timesigpicker_frame->Destroy();
        timesigpicker_frame = NULL;
    }

}

void showTimeSigPicker(const int x, const int y, const int num, const int denom)
{
    if (timesigpicker_frame == NULL) timesigpicker_frame = new TimeSigPicker();
    timesigpicker_frame->show(x, y, num, denom);
}

TimeSigPicker::TimeSigPicker() : wxFrame(getMainFrame(), 0,  _("Time Signature"), wxDefaultPosition, wxSize(185,130),
                                         wxCAPTION | wxCLOSE_BOX | wxFRAME_TOOL_WINDOW | wxWANTS_CHARS)
{
    pane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS);

    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;

    wxBoxSizer* vertical = new wxBoxSizer(wxVERTICAL);
    QuickBoxLayout horizontal(pane, vertical);
    valueTextNum=new wxTextCtrl(horizontal.pane, 1, wxT("4"), wxPoint(0,130), smallsize, wxTE_PROCESS_ENTER | wxWANTS_CHARS | wxTAB_TRAVERSAL);
    wxStaticText* slash = new wxStaticText(horizontal.pane, wxID_ANY, wxT("/"), wxDefaultPosition, wxSize(15,15));
    slash->SetMinSize(wxSize(15,15));
    slash->SetMaxSize(wxSize(15,15));
    valueTextDenom=new wxTextCtrl(horizontal.pane, 2, wxT("4"), wxPoint(0,130), smallsize, wxTE_PROCESS_ENTER | wxWANTS_CHARS | wxTAB_TRAVERSAL);
    okbtn = new wxButton(pane, 3, _("OK"));
    okbtn->SetDefault();

    valueTextNum  ->Connect( valueTextNum  ->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(TimeSigPicker::onFocus), 0, this);
    valueTextDenom->Connect( valueTextDenom->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(TimeSigPicker::onFocus), 0, this);

    horizontal.add(valueTextNum, 5, 0, wxLEFT | wxRIGHT);
    horizontal.add(slash, 0, 0);
    horizontal.add(valueTextDenom, 5, 0, wxRIGHT);

    //I18N: - when setting time signature, to indicate it's not constant through song
    variable = new wxCheckBox(pane, 4, _("Varies throughout song"));

    vertical->Add(variable, 0, wxALL, 5);
    vertical->Add(okbtn, 0, wxALL | wxALIGN_CENTER, 5);

    pane->SetSizer(vertical);


    Connect(GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    pane->Connect(pane->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    valueTextNum->Connect(valueTextDenom->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    valueTextNum->Connect(valueTextNum->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);

    Connect(GetId(), wxEVT_CHAR, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    pane->Connect(pane->GetId(), wxEVT_CHAR, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    valueTextNum->Connect(valueTextDenom->GetId(), wxEVT_CHAR, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    valueTextNum->Connect(valueTextNum->GetId(), wxEVT_CHAR, wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
}

void TimeSigPicker::closed(wxCloseEvent& evt)
{
    if (IsShown())
    {
        wxCommandEvent event( wxEVT_DESTROY_TIMESIG_PICKER, 100000 );
        getMainFrame()->GetEventHandler()->AddPendingEvent( event );
        Hide();
    }
}

void TimeSigPicker::onFocus(wxFocusEvent& evt)
{
    wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(FindFocus());
    if (ctrl != NULL)
    {
      ctrl->SetFocus();
      ctrl->SetSelection(-1, -1);
    }
}


void TimeSigPicker::show(const int x, const int y, const int num, const int denom)
{
    // show the keysig picking dialog
    SetPosition(wxPoint(x,y));

    valueTextNum->SetValue( to_wxString(num) );
    valueTextDenom->SetValue( to_wxString(denom) );
    variable->SetValue( getMeasureData()->isExpandedMode() );
    Show();
    valueTextNum->SetFocus();
    valueTextNum->SetSelection( -1, -1 );
}

void TimeSigPicker::enterPressed(wxCommandEvent& evt)
{
    int top = atoi_u( valueTextNum->GetValue() );
    int bottom = atoi_u( valueTextDenom->GetValue() );

    if (bottom < 1 or top<1 or bottom>32 or top>32 or
      not valueTextNum->GetValue().IsNumber() or not valueTextDenom->GetValue().IsNumber())
    {
        wxBell();
        return;
    }

    float denom_check = (float)log(bottom)/(float)log(2);
    if ( (int)denom_check != (float)denom_check )
    {
        wxBell();
        //I18N: - when setting a wrong time signature
        wxMessageBox(  _("Denominator must be a power of 2") );
        return;
    }

    getMeasureData()->setTimeSig( top, bottom );
    getMainFrame()->changeShownTimeSig( top, bottom );

    // check if user changed measure mode
    if (variable->IsChecked() != getMeasureData()->isExpandedMode())
    {
        getCurrentSequence()->measureData->setExpandedMode( variable->IsChecked() );
        getMainFrame()->updateTopBarAndScrollbarsForSequence( getCurrentSequence() );
        getMainFrame()->updateMenuBarToSequence();
    }

    closeWindow();
}

void TimeSigPicker::closeWindow()
{
    Hide();
    Display::requestFocus();

    wxCommandEvent event( wxEVT_DESTROY_TIMESIG_PICKER, 100000 );
    getMainFrame()->GetEventHandler()->AddPendingEvent( event );

}

void TimeSigPicker::keyPress(wxKeyEvent& evt)
{
    if (evt.GetKeyCode()==WXK_ESCAPE || evt.GetKeyCode()==WXK_CANCEL || evt.GetKeyCode()==WXK_DELETE)
    {
        closeWindow();
    }
    else if (evt.GetKeyCode() == WXK_RETURN)
    {
        wxCommandEvent dummyEvt;
        enterPressed(dummyEvt);
    }
    else evt.Skip(true);
}

}
