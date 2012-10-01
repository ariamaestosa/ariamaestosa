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
#include "GUI/MainFrame.h"
#include "Actions/EditAction.h"
#include "Actions/SetNoteVolume.h"
#include "Actions/SetTrackVolume.h"

#include <iostream>
#include "Utils.h"
#include "AriaCore.h"

#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/slider.h>
#include <wx/textctrl.h>

namespace AriaMaestosa
{
    
    /**
      * @ingroup pickers
      * @brief small frame used to pick a note volume (velocity)
      */
    class VolumeSlider : public wxDialog
    {
        wxSlider* m_slider;
        wxTextCtrl* m_value_text;
        wxPanel* m_pane;
        int m_return_code;
        int m_note_ID;
        bool m_set_track_volume;
        wxString m_percent_string;
        Track* m_current_track;
        
        void launchPicker(int x, int y, Track* track, int value);
        void updateValue(int value);
        void updateValueText(int value);
        
    public:
        LEAK_CHECK();
        
        VolumeSlider();
        
        void show(int x, int y, int noteID, Track* track);
        void show(int x, int y, Track* track);
        void closeWindow();
        
        void volumeSlideChanging(wxScrollEvent& evt);
        void volumeSlideChanged(wxScrollEvent& evt);
        void volumeTextChanged(wxCommandEvent& evt);
        void enterPressed(wxCommandEvent& evt);
        
        void closed(wxCloseEvent& evt);
        void keyPress(wxKeyEvent& evt);
        void onCancel(wxCommandEvent& evt);
    };
    
    
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_DESTROY_VOLUME_SLIDER)
    
    VolumeSlider* sliderframe = NULL;
    
#if 0
#pragma mark Functions
#endif
    
    void freeVolumeSlider()
    {
        if (sliderframe != NULL)
        {
            sliderframe->Destroy();
            sliderframe = NULL;
        }
        
    }
    
    void showVolumeSlider(int x, int y, int noteID, Track* track)
    {
        if (sliderframe == NULL) sliderframe = new VolumeSlider();
        sliderframe->show(x,y,noteID, track);
    }
    
    void showVolumeSlider(int x, int y, Track* track)
    {
        if (sliderframe == NULL) sliderframe = new VolumeSlider();
        sliderframe->show(x, y, track);
    }
    
}

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Class implementation
#endif

static const int VOLUME_SLIDER_FRAME_WIDTH = 50;
static const int VOLUME_SLIDER_FRAME_HEIGHT = 160;
static const int SLIDER_MIN_VALUE = 0;
static const int SLIDER_MAX_VALUE = 127;
static const int MAX_TRACK_VOLUME = 200;


VolumeSlider::VolumeSlider() : wxDialog(NULL, wxNewId(),  _("volume"), wxDefaultPosition,
                                        wxSize(VOLUME_SLIDER_FRAME_WIDTH, VOLUME_SLIDER_FRAME_HEIGHT),
                                        wxSTAY_ON_TOP | wxWANTS_CHARS | wxBORDER_NONE )
{
    m_pane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(VOLUME_SLIDER_FRAME_WIDTH,
                                                                   VOLUME_SLIDER_FRAME_HEIGHT));
    
    m_slider = new wxSlider(m_pane, wxNewId(), 60, SLIDER_MIN_VALUE, SLIDER_MAX_VALUE,
                            wxDefaultPosition, wxSize(50, SLIDER_MAX_VALUE+1),
                            wxSL_VERTICAL | wxSL_INVERSE | wxWANTS_CHARS);
    
    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;
    
    m_value_text = new wxTextCtrl(m_pane, wxNewId(), wxT("0"), wxPoint(0,130), smallsize,
                                  wxTE_PROCESS_ENTER | wxWANTS_CHARS);
    m_value_text->SetMinSize(smallsize);
    m_value_text->SetMaxSize(smallsize);
    m_note_ID = -1;
    m_current_track = NULL;
    m_percent_string = _("%");

    // Connect all widgets to receive keypress events no matter where keyboard focus is
    this        ->Connect(GetId(),               wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_slider    ->Connect(m_slider->GetId(),     wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_pane      ->Connect(m_pane->GetId(),       wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_value_text->Connect(m_value_text->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    
    this        ->Connect(GetId(),               wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_slider    ->Connect(m_slider->GetId(),     wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_pane      ->Connect(m_pane->GetId(),       wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_value_text->Connect(m_value_text->GetId(), wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    
    Connect(m_slider->GetId(), wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(VolumeSlider::volumeSlideChanging), NULL, this);
    Connect(m_slider->GetId(), wxEVT_SCROLL_LINEUP,     wxScrollEventHandler(VolumeSlider::volumeSlideChanging), NULL, this);
    Connect(m_slider->GetId(), wxEVT_SCROLL_LINEDOWN,   wxScrollEventHandler(VolumeSlider::volumeSlideChanging), NULL, this);
    Connect(m_slider->GetId(), wxEVT_SCROLL_PAGEUP,     wxScrollEventHandler(VolumeSlider::volumeSlideChanging), NULL, this);
    Connect(m_slider->GetId(), wxEVT_SCROLL_PAGEDOWN,   wxScrollEventHandler(VolumeSlider::volumeSlideChanging), NULL, this);
    
    Connect(m_slider->GetId(), wxEVT_SCROLL_THUMBRELEASE,    wxScrollEventHandler(VolumeSlider::volumeSlideChanged), NULL, this);

    Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(VolumeSlider::closed), NULL, this);
    Connect(m_value_text->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(VolumeSlider::volumeTextChanged), NULL, this);
    Connect(m_value_text->GetId(), wxEVT_COMMAND_TEXT_ENTER,   wxCommandEventHandler(VolumeSlider::enterPressed),      NULL, this);
    
    Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VolumeSlider::onCancel), NULL, this);
    
    SetMinSize(wxSize(VOLUME_SLIDER_FRAME_WIDTH, VOLUME_SLIDER_FRAME_HEIGHT));
    SetMaxSize(wxSize(VOLUME_SLIDER_FRAME_WIDTH, VOLUME_SLIDER_FRAME_HEIGHT));
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::closed(wxCloseEvent& evt)
{
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::onCancel(wxCommandEvent& evt)
{
    closeWindow();
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::show(int x, int y, int noteID, Track* track)
{
    ASSERT(track!=NULL);
    m_note_ID = noteID;
    m_set_track_volume = false;
    launchPicker(x, y, track, track->getNoteVolume(noteID));
}


// -------------------------------------------------------------------------------------------------------
void VolumeSlider::show(int x, int y, Track* track)
{
    ASSERT(track!=NULL);
    m_set_track_volume = true;
    launchPicker(x, y, track, track->getVolume()* SLIDER_MAX_VALUE / MAX_TRACK_VOLUME);
}



// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeSlideChanged(wxScrollEvent& evt)
{
    updateValue(m_slider->GetValue());
    
    // close the volume picking dialog
    closeWindow();
    return;
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeSlideChanging(wxScrollEvent& evt)
{
    const int newValue = m_slider->GetValue();
    updateValueText(newValue);
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeTextChanged(wxCommandEvent& evt)
{
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::enterPressed(wxCommandEvent& evt)
{
    wxString valueText;
 
    valueText = m_value_text->GetValue();
    
    if (m_set_track_volume)
    {
        valueText.Replace(m_percent_string, wxT(""));
    }
    
    if (valueText.IsNumber())
    {
        int newValue = atoi_u(valueText);
        
        if (m_set_track_volume)
        {
            newValue = newValue* SLIDER_MAX_VALUE / MAX_TRACK_VOLUME;
        }
        
        if (newValue<SLIDER_MIN_VALUE)
        {
            wxBell();
            updateValueText(SLIDER_MIN_VALUE);
        }
        else if (newValue>SLIDER_MAX_VALUE)
        {
            wxBell();
            updateValueText(SLIDER_MAX_VALUE);
        }
        else
        {
            updateValue(newValue);
            closeWindow();
        }
    }
    else
    {
        wxBell();
        const int newValue = m_slider->GetValue();
        updateValueText(newValue);
    }

}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::closeWindow()
{
    wxDialog::EndModal(m_return_code);
    m_current_track = NULL;
    Display::requestFocus();
    
    wxCommandEvent event( wxEVT_DESTROY_VOLUME_SLIDER, DESTROY_SLIDER_EVENT_ID );
    getMainFrame()->GetEventHandler()->AddPendingEvent( event );
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::keyPress(wxKeyEvent& evt)
{
    if (evt.GetKeyCode()==WXK_ESCAPE or evt.GetKeyCode()==WXK_CANCEL or evt.GetKeyCode()==WXK_DELETE)
    {
        closeWindow();
    }
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

// -------------------------------------------------------------------------------------------------------
void VolumeSlider::launchPicker(int x, int y, Track* track, int value)
{
    if (track != NULL)
    {
        m_current_track = track;
        
        SetPosition(wxPoint(x,y));
        m_slider->SetValue(value);
        
        updateValueText(value);
        m_value_text->SetFocus();
        m_value_text->SetSelection(-1,-1); // select all
        
        m_return_code = ShowModal(); 
    }
}


void VolumeSlider::updateValue(int value)
{
    ASSERT(m_current_track != NULL);
    
    if (m_set_track_volume)
    {
        m_current_track->action( new Action::SetTrackVolume(value* MAX_TRACK_VOLUME / SLIDER_MAX_VALUE) );
    }
    else
    {
        int target;
        
        ASSERT(m_note_ID != -1);
    
        target = m_current_track->isNoteSelected(m_note_ID) ? SELECTED_NOTES : m_note_ID;
        m_current_track->action( new Action::SetNoteVolume(value, target) );
    }
}


void VolumeSlider::updateValueText(int value)
{
    wxString display;
    
    if (m_set_track_volume)
    {
        display = to_wxString(value * MAX_TRACK_VOLUME / SLIDER_MAX_VALUE) + m_percent_string;
    }
    else
    {
        display = to_wxString(value);
    }
    m_value_text->SetValue(display);
}

