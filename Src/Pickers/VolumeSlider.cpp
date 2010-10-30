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

#include <iostream>
#include "Utils.h"
#include "AriaCore.h"

#include <wx/dialog.h>
#include <wx/slider.h>
#include <wx/textctrl.h>

namespace AriaMaestosa
{
    
    enum
    {
        ID_SLIDER = wxID_HIGHEST + 1000, //hope there are no conflicts... FIXME: switch to wxNewID() + Bind when 3.0 is out
        ID_TEXT_AREA
    };
    
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
        Track* m_current_track;
        
    public:
        LEAK_CHECK();
        
        VolumeSlider();
        
        void show(int x, int y, int noteID, Track* track);
        void closeWindow();
        
        void volumeSlideChanging(wxScrollEvent& evt);
        void volumeSlideChanged(wxScrollEvent& evt);
        void volumeTextChanged(wxCommandEvent& evt);
        void enterPressed(wxCommandEvent& evt);
        
        void closed(wxCloseEvent& evt);
        void keyPress(wxKeyEvent& evt);
        void onCancel(wxCommandEvent& evt);
        
        DECLARE_EVENT_TABLE();
    };
    
    
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_DESTROY_VOLUME_SLIDER)
    
    //FIXME: don't mix event tables and Connect!
    BEGIN_EVENT_TABLE(VolumeSlider, wxDialog)
    
    EVT_COMMAND_SCROLL_THUMBTRACK  (ID_SLIDER, VolumeSlider::volumeSlideChanging)
    EVT_COMMAND_SCROLL_THUMBRELEASE(ID_SLIDER, VolumeSlider::volumeSlideChanged)
    EVT_COMMAND_SCROLL_LINEUP      (ID_SLIDER, VolumeSlider::volumeSlideChanging)
    EVT_COMMAND_SCROLL_LINEDOWN    (ID_SLIDER, VolumeSlider::volumeSlideChanging)
    EVT_COMMAND_SCROLL_PAGEUP      (ID_SLIDER, VolumeSlider::volumeSlideChanging)
    EVT_COMMAND_SCROLL_PAGEDOWN    (ID_SLIDER, VolumeSlider::volumeSlideChanging)
    
    EVT_TEXT      (ID_TEXT_AREA, VolumeSlider::volumeTextChanged)
    EVT_TEXT_ENTER(ID_TEXT_AREA, VolumeSlider::enterPressed)
    
    EVT_CLOSE(VolumeSlider::closed)
    EVT_COMMAND(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, VolumeSlider::onCancel )
    
    END_EVENT_TABLE()
    
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
    
}

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Class implementation
#endif

const int VOLUME_SLIDER_FRAME_WIDTH = 50;
const int VOLUME_SLIDER_FRAME_HEIGHT = 160;

VolumeSlider::VolumeSlider() : wxDialog(NULL, wxNewId(),  _("volume"), wxDefaultPosition,
                                        wxSize(VOLUME_SLIDER_FRAME_WIDTH, VOLUME_SLIDER_FRAME_HEIGHT),
                                        wxSTAY_ON_TOP | wxWANTS_CHARS )
{
    m_pane = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(VOLUME_SLIDER_FRAME_WIDTH,
                                                                   VOLUME_SLIDER_FRAME_HEIGHT));
    
    m_slider = new wxSlider(m_pane, ID_SLIDER, 60, 0, 127, wxDefaultPosition, wxSize(50,128),
                            wxSL_VERTICAL | wxSL_INVERSE | wxWANTS_CHARS);
    
    wxSize smallsize = wxDefaultSize;
    smallsize.x = 50;
    
    m_value_text = new wxTextCtrl(m_pane, ID_TEXT_AREA, wxT("0"), wxPoint(0,130), smallsize,
                                  wxTE_PROCESS_ENTER | wxWANTS_CHARS);
    m_note_ID = -1;
    m_current_track = NULL;
    
    
    // Connect all widgets to receive keypress events no matter where keyboard focus is
    this        ->Connect(GetId(),               wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_slider    ->Connect(m_slider->GetId(),     wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_pane      ->Connect(m_pane->GetId(),       wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_value_text->Connect(m_value_text->GetId(), wxEVT_KEY_DOWN, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    
    this        ->Connect(GetId(),               wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_slider    ->Connect(m_slider->GetId(),     wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_pane      ->Connect(m_pane->GetId(),       wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    m_value_text->Connect(m_value_text->GetId(), wxEVT_CHAR, wxKeyEventHandler(VolumeSlider::keyPress), NULL, this);
    
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
    // show the volume picking dialog
    
    if (track == NULL) return;
    
    m_current_track = track;
    m_note_ID       = noteID;
    
    SetPosition(wxPoint(x,y));
    m_slider->SetValue(m_current_track->getNoteVolume(noteID));
    
    m_value_text->SetValue( to_wxString(m_current_track->getNoteVolume(noteID)) );
    
    m_slider->SetFocus();
    m_return_code = ShowModal();
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeSlideChanged(wxScrollEvent& evt)
{
    
    if (m_current_track == NULL) return;
    
    if (m_note_ID!=-1)
    {
        
        if (m_current_track->isNoteSelected(m_note_ID))
            m_current_track->action( new Action::SetNoteVolume(m_slider->GetValue(), SELECTED_NOTES) );
        else
            m_current_track->action( new Action::SetNoteVolume(m_slider->GetValue(), m_note_ID) );
        
    }
    
    // close the volume picking dialog
    closeWindow();
    return;
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeSlideChanging(wxScrollEvent& evt)
{
    const int newValue = m_slider->GetValue();
    m_value_text->SetValue( to_wxString(newValue) );
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::volumeTextChanged(wxCommandEvent& evt)
{
}

// -------------------------------------------------------------------------------------------------------

void VolumeSlider::enterPressed(wxCommandEvent& evt)
{
    
    if (!m_value_text->GetValue().IsNumber())
    {
        wxBell();
        const int newValue = m_slider->GetValue();
        m_value_text->SetValue( to_wxString(newValue) );
        return;
    }
    
    const int newValue = atoi_u(m_value_text->GetValue());
    if (newValue<0)
    {
        wxBell();
        m_value_text->SetValue(wxT("0"));
        return;
    }
    
    if (newValue>127)
    {
        wxBell();
        m_value_text->SetValue(wxT("127"));
        return;
    }
    
    ASSERT(m_note_ID       != -1  );
    ASSERT(m_current_track != NULL);
    
    if (m_current_track->isNoteSelected(m_note_ID))
        m_current_track->action( new Action::SetNoteVolume(newValue, SELECTED_NOTES) );
    else
        m_current_track->action( new Action::SetNoteVolume(newValue, m_note_ID) );
    
    closeWindow();
    
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
