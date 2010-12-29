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

#include <wx/checkbox.h>
#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/minifram.h>
#include <wx/spinbutt.h>

namespace AriaMaestosa
{

    /**
     * @ingroup pickers
     * @brief small floating frame where the time signature can be entered
     */
    class TimeSigPicker : public wxMiniFrame
    {
        wxTextCtrl* m_value_text_num;
        wxTextCtrl* m_value_text_denom;
        wxButton*   m_ok_btn;
        wxPanel*    m_pane;
        wxCheckBox* m_variable;
        
        GraphicalSequence* m_gseq;
        
    public:
        LEAK_CHECK();
        
        TimeSigPicker();
        
        void show(GraphicalSequence* parent, const int x, const int y, const int num, const int denom);
        void closeWindow();
        
        void enterPressed(wxCommandEvent& evt);
        
        void closed(wxCloseEvent& evt);
        void keyPress(wxKeyEvent& evt);
        void onFocus(wxFocusEvent& evt);
    };
    
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_DESTROY_TIMESIG_PICKER)
    
    TimeSigPicker* timesigpicker_frame = NULL;
    
    namespace TimeSigPickerNames
    {
        
        //FIXME: preferences use that too, move it out of here and make it common
        class QuickBoxLayout
        {
            wxBoxSizer* bsizer;
        public:
            wxPanel* pane;
            
            QuickBoxLayout(wxWindow* component, wxSizer* parent, int orientation=wxHORIZONTAL, int proportion=1)
            {
                pane = new wxPanel(component);
                parent->Add(pane,proportion,wxEXPAND);
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
}

using namespace AriaMaestosa;
using namespace TimeSigPickerNames;

// --------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Functions
#endif

void AriaMaestosa::freeTimeSigPicker()
{
    if (timesigpicker_frame != NULL)
    {
        timesigpicker_frame->Destroy();
        timesigpicker_frame = NULL;
    }

}

// --------------------------------------------------------------------------------------------------------

void AriaMaestosa::showTimeSigPicker(GraphicalSequence* parent, const int x, const int y, const int num, const int denom)
{
    if (timesigpicker_frame == NULL) timesigpicker_frame = new TimeSigPicker();
    timesigpicker_frame->show(parent, x, y, num, denom);
}

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Class implementation
#endif

TimeSigPicker::TimeSigPicker() : wxMiniFrame(getMainFrame(), wxNewId(),  _("Time Signature"),
                                         wxDefaultPosition, wxSize(200,130),
                                         wxCAPTION | wxCLOSE_BOX | wxWANTS_CHARS | wxFRAME_FLOAT_ON_PARENT)
{
    m_pane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS | wxTAB_TRAVERSAL);
    m_pane->SetMinSize( wxSize(1,1) );
    wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
    panelSizer->Add(m_pane, 1, wxEXPAND);

    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;

    wxBoxSizer* vertical = new wxBoxSizer(wxVERTICAL);

    vertical->AddSpacer(5);

    QuickBoxLayout horizontal(m_pane, vertical, wxHORIZONTAL, 0);
    m_value_text_num = new wxTextCtrl(horizontal.pane, wxNewId(), wxT("4"), wxPoint(0,130), smallsize,
                                      wxTE_PROCESS_ENTER | wxWANTS_CHARS);
    
    wxStaticText* slash = new wxStaticText(horizontal.pane, wxID_ANY, wxT("/"), wxDefaultPosition, wxSize(15,15));
    slash->SetMinSize(wxSize(15,15));
    slash->SetMaxSize(wxSize(15,15));
    
    m_value_text_denom = new wxTextCtrl(horizontal.pane, wxNewId(), wxT("4"), wxPoint(0,130), smallsize,
                                        wxTE_PROCESS_ENTER | wxWANTS_CHARS);
    
    m_value_text_num  ->Connect(m_value_text_num->GetId(), wxEVT_SET_FOCUS,
                                wxFocusEventHandler(TimeSigPicker::onFocus), 0, this);
    m_value_text_denom->Connect(m_value_text_denom->GetId(), wxEVT_SET_FOCUS,
                                wxFocusEventHandler(TimeSigPicker::onFocus), 0, this);

    horizontal.add(m_value_text_num, 5, 0, wxLEFT | wxRIGHT);
    horizontal.add(slash, 0, 0);
    horizontal.add(m_value_text_denom, 5, 0, wxRIGHT);

    vertical->AddSpacer(7);

    //I18N: - when setting time signature, to indicate it's not constant through song
    m_variable = new wxCheckBox(m_pane, wxNewId(), _("Varies throughout song"));
    vertical->Add(m_variable, 0, wxALL, 5);
    
    m_ok_btn = new wxButton(m_pane, wxNewId(), _("OK"));
    m_ok_btn->SetDefault();
    vertical->Add(m_ok_btn, 0, wxALL | wxALIGN_CENTER, 5);

    m_pane->SetSizer(vertical);
    vertical->Layout();
    vertical->SetSizeHints(m_pane);

    // Connect all widgets in order to catch key events like escape and enter no matter where keyboard focus is
    this              ->Connect(this->GetId(),               wxEVT_KEY_DOWN,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_pane            ->Connect(m_pane->GetId(),             wxEVT_KEY_DOWN,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_value_text_denom->Connect(m_value_text_denom->GetId(), wxEVT_KEY_DOWN,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_value_text_num  ->Connect(m_value_text_num->GetId(),   wxEVT_KEY_DOWN,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);

    this              ->Connect(this->GetId(),               wxEVT_CHAR,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_pane            ->Connect(m_pane->GetId(),             wxEVT_CHAR,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_value_text_denom->Connect(m_value_text_denom->GetId(), wxEVT_CHAR,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);
    m_value_text_num  ->Connect(m_value_text_num->GetId(),   wxEVT_CHAR,
                                wxKeyEventHandler(TimeSigPicker::keyPress), NULL, this);

    m_value_text_num  ->Connect(m_value_text_num->GetId(),   wxEVT_COMMAND_TEXT_ENTER,
                                wxCommandEventHandler(TimeSigPicker::enterPressed), NULL, this);
    m_value_text_denom->Connect(m_value_text_denom->GetId(), wxEVT_COMMAND_TEXT_ENTER,
                                wxCommandEventHandler(TimeSigPicker::enterPressed), NULL, this);
    m_ok_btn          ->Connect(m_ok_btn->GetId(),           wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(TimeSigPicker::enterPressed), NULL, this);
    
    Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(TimeSigPicker::closed), NULL, this);
    
    SetSizer(panelSizer);
    // FIXME: I should be able to Fit here, find why Fit allocates way too much space vertically
    //panelSizer->SetSizeHints(this);
    //Fit();
}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::closed(wxCloseEvent& evt)
{
    if (IsShown())
    {
        wxCommandEvent event( wxEVT_DESTROY_TIMESIG_PICKER, DESTROY_TIMESIG_EVENT_ID );
        getMainFrame()->GetEventHandler()->AddPendingEvent( event );
        Hide();
    }
}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::onFocus(wxFocusEvent& evt)
{
    wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(FindFocus());
    if (ctrl != NULL)
    {
      ctrl->SetFocus();
      ctrl->SetSelection(-1, -1);
    }
}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::show(GraphicalSequence* parent, const int x, const int y, const int num, const int denom)
{
    m_gseq = parent;
    
    // show the keysig picking dialog
    SetPosition(wxPoint(x,y));

    m_value_text_num->SetValue( to_wxString(num) );
    m_value_text_denom->SetValue( to_wxString(denom) );
    m_variable->SetValue( parent->getModel()->getMeasureData()->isExpandedMode() );
    Show();
    m_value_text_num->SetFocus();
    m_value_text_num->SetSelection( -1, -1 ); // select everything
}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::enterPressed(wxCommandEvent& evt)
{
    int top    = atoi_u( m_value_text_num->GetValue() );
    int bottom = atoi_u( m_value_text_denom->GetValue() );

    if (bottom < 1 or top < 1 or bottom > 32 or top > 32 or
        not m_value_text_num->GetValue().IsNumber() or not m_value_text_denom->GetValue().IsNumber())
    {
        wxBell();
        return;
    }

    float denom_check = (float)log(bottom)/(float)log(2);
    if ((int)denom_check != (float)denom_check)
    {
        wxBell();
        //I18N: - when setting a wrong time signature
        wxMessageBox(  _("Denominator must be a power of 2") );
        return;
    }
    
    MeasureData* measures = m_gseq->getModel()->getMeasureData();
    
    if (not m_variable->IsChecked() and measures->isExpandedMode())
    {
        const int answer = wxMessageBox(_("Are you sure you want to go back to having a single time signature? Any time sig events you may have added will be lost. This cannot be undone."),
                                        _("Confirm"), wxYES_NO);
        if (answer == wxNO) return;
    }
    
    {
        ScopedMeasureTransaction tr(measures->startTransaction());
        tr->setTimeSig( top, bottom );
        
        // check if user changed measure mode
        if (m_variable->IsChecked() != measures->isExpandedMode())
        {
            tr->setExpandedMode( m_variable->IsChecked() );
        }
        
    } // end transaction

    closeWindow();
}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::closeWindow()
{
    Hide();
    Display::requestFocus();

    // schedule delete event
    wxCommandEvent event( wxEVT_DESTROY_TIMESIG_PICKER, DESTROY_TIMESIG_EVENT_ID );
    getMainFrame()->GetEventHandler()->AddPendingEvent( event );

}

// --------------------------------------------------------------------------------------------------------

void TimeSigPicker::keyPress(wxKeyEvent& evt)
{
    if (evt.GetKeyCode()==WXK_ESCAPE or evt.GetKeyCode()==WXK_CANCEL or evt.GetKeyCode()==WXK_DELETE)
    {
        closeWindow();
    }
    //FIXME: EVT_TEXT_ENTER should be sufficient, verify that this is indeed not necessary and remove it
    else if (evt.GetKeyCode() == WXK_RETURN)
    {
        wxCommandEvent dummyEvt;
        enterPressed(dummyEvt);
    }
    else
    {
        evt.Skip(true);
    }
}

// --------------------------------------------------------------------------------------------------------
