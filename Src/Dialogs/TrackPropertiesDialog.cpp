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

#include "AriaCore.h"
#include "Dialogs/TrackPropertiesDialog.h"
#include "Editors/Editor.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/DrumChoice.h"
#include "Midi/InstrumentChoice.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"

#include <wx/tokenzr.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/utils.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <iostream>

namespace AriaMaestosa
{
    
    
    class BackgroundChoicePanel : public wxPanel
    {
    public:
        LEAK_CHECK();
        
        wxBoxSizer* sizer;
        wxCheckBox* active;
        
        BackgroundChoicePanel(wxWindow* parent, const int trackID, Track* track, const bool activated=false, const bool enabled = false) : wxPanel(parent)
        {
            sizer = new wxBoxSizer(wxHORIZONTAL);
            
            const EditorType editorMode = track->graphics->getEditorMode();
            
            // checkbox
            active = new wxCheckBox(this, 200, wxT(" "));
            sizer->Add(active, 0, wxALL, 5);
            
            if (not enabled or editorMode == DRUM)  active->Enable(false);
            else if (activated)                     active->SetValue(true);
            
            wxString instrumentname;
            if (editorMode == DRUM) instrumentname = DrumChoice::getDrumkitName( track->getDrumKit() );
            else                    instrumentname = InstrumentChoice::getInstrumentName( track->getInstrument() );
            
            sizer->Add( new wxStaticText(this, wxID_ANY, to_wxString(trackID) + wxT(" : ") + track->getName() +
                                         wxT(" (") + instrumentname + wxT(")")) , 1, wxALL, 5);
            
            SetSizer(sizer);
            sizer->Layout();
            sizer->SetSizeHints(this); // resize to take ideal space
        }
        
        bool isChecked()
        {
            return active->GetValue();
        }
        
    };
    
    /**
      * @ingroup dialogs
      * @brief The dialog where per-track properties may be edited
      * @note this is a private class, it won't be instanciated directly
      * @see TrackProperties::show
      */
    class TrackPropertiesDialog : public wxDialog
    {
        wxButton* ok_btn;
        wxButton* cancel_btn;
        
        wxBoxSizer* sizer;
        
        wxTextCtrl* volume_text;
        wxSlider* volume_slider;
        
        ptr_vector<BackgroundChoicePanel> m_choice_panels;
        
        Track* m_parent;
        
        int modalid;
        
        bool m_ignore_events;
        
    public:
        LEAK_CHECK();
        
        ~TrackPropertiesDialog()
        {
        }
        
        TrackPropertiesDialog(Track* parent) :
        wxDialog(NULL, wxID_ANY,  _("Track Properties"), wxPoint(100,100), wxSize(700,300), wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP )
        {
            m_ignore_events = true;
            m_parent = parent;
            
            Sequence* seq = parent->getSequence();
            
            modalid = -1;
            
            sizer = new wxBoxSizer(wxVERTICAL);
            
            wxPanel* properties_panel = new wxPanel(this);
            wxPanel* buttonPane = new wxPanel(this);
            sizer->Add(properties_panel, 1, wxEXPAND | wxALL, 5);
            sizer->Add(buttonPane, 0, wxALL, 5);
            
            Editor* editor = parent->graphics->getCurrentEditor();
            
            wxBoxSizer* props_sizer = new wxBoxSizer(wxHORIZONTAL);
            
            // ------ track background -----
            wxStaticBoxSizer* bg_subsizer = new wxStaticBoxSizer(wxVERTICAL, properties_panel, _("Track Background"));
            
            const int trackAmount = seq->getTrackAmount();
            for (int n=0; n<trackAmount; n++)
            {
                Track* track = seq->getTrack(n);
                bool enabled = true;
                if (track == parent) enabled = false; // can't be background of itself
                
                bool activated = false;
                if (editor->hasAsBackground(track)) activated = true;
                
                BackgroundChoicePanel* bcp = new BackgroundChoicePanel(properties_panel, n, track, activated, enabled);
                bg_subsizer->Add(bcp, 0, wxALL, 5);
                m_choice_panels.push_back(bcp);
            }
            props_sizer->Add(bg_subsizer, 0, wxALL, 5);
            
            bg_subsizer->Layout();
            
            // ------ other properties ------
            wxBoxSizer* right_subsizer = new wxBoxSizer(wxVERTICAL);
            props_sizer->Add(right_subsizer, 0, wxALL, 5);
            
            wxStaticBoxSizer* default_volume_subsizer = new wxStaticBoxSizer(wxHORIZONTAL, properties_panel, _("Default Volume"));
            
            volume_slider = new wxSlider(properties_panel, 300 /* ID */, 80 /* current */, 0 /* min */, 127 /* max */);
            default_volume_subsizer->Add(volume_slider, 0, wxALL, 5);
#ifdef __WXGTK__
            volume_slider->SetMinSize( wxSize(127,-1) );
#endif
            
            wxSize smallsize = wxDefaultSize;
            smallsize.x = 50;
            volume_text = new wxTextCtrl(properties_panel, 301, wxT("80"), wxDefaultPosition, smallsize);
            default_volume_subsizer->Add(volume_text, 0, wxALL, 5);
            
            right_subsizer->Add(default_volume_subsizer, 0, wxALL, 0);
            
            //right_subsizer->Add( new wxButton(properties_panel, wxID_ANY, wxT("Editor-specific options")), 1, wxALL, 5 );
            
            properties_panel->SetSizer(props_sizer);
            
            default_volume_subsizer->Layout();
            right_subsizer->Layout();
            props_sizer->Layout();
            props_sizer->SetSizeHints(properties_panel);
            
            // ------ bottom OK/cancel buttons ----
            wxBoxSizer* buttonsizer = new wxBoxSizer(wxHORIZONTAL);
            
            ok_btn = new wxButton(buttonPane, 200, wxT("OK"));
            ok_btn->SetDefault();
            buttonsizer->Add(ok_btn, 0, wxALL, 5);
            
            cancel_btn = new wxButton(buttonPane, 202,  _("Cancel"));
            buttonsizer->Add(cancel_btn, 0, wxALL, 5);
            
            buttonPane->SetSizer(buttonsizer);
            
            SetSizer(sizer);
            sizer->Layout();
            sizer->SetSizeHints(this); // resize window to take ideal space
                                       // FIXME - if too many tracks for current screen space, may cause problems
            
            m_ignore_events = false;
        }
        
        void show()
        {
            Center();
            
            Editor* editor = m_parent->graphics->getCurrentEditor();
            volume_text->SetValue( to_wxString(editor->getDefaultVolume()) );
            volume_slider->SetValue( editor->getDefaultVolume() );
            
            modalid = ShowModal();
        }
        
        // when Cancel button of the tuning picker is pressed
        void cancelButton(wxCommandEvent& evt)
        {
            wxDialog::EndModal(modalid);
        }
        
        // when OK button of the tuning picker is pressed
        void okButton(wxCommandEvent& evt)
        {
            Editor* editor = m_parent->graphics->getCurrentEditor();
            const int value = atoi_u(volume_text->GetValue());
            if (value >=0 and value < 128) editor->setDefaultVolume( value );
            else wxBell();
            
            const int amount = m_choice_panels.size();
            Sequence* seq = m_parent->getSequence();
            
            editor->clearBackgroundTracks();
            
            for (int n=0; n<amount; n++)
            {
                if (m_choice_panels[n].isChecked())
                {
                    editor->addBackgroundTrack( seq->getTrack(n) );
                    std::cout << "Adding track " << n << " as background to track " << std::endl;
                }
            }
                        
            wxDialog::EndModal(modalid);
            Display::render();
        }
        
        void volumeSlideChanging(wxScrollEvent& evt)
        {
            // FIXME: an apparent wxGTK bug sends events before the constructor even returned
            if (m_ignore_events) return;
            
            const int value = volume_slider->GetValue();
            if (value >=0 and value < 128) volume_text->SetValue( to_wxString(value) );
        }
        
        void volumeTextChanged(wxCommandEvent& evt)
        {
            // FIXME: an apparent wxGTK/wxMSW bug(?) sends events before the constructor even returned
            if (m_ignore_events) return;
            
            const int value = atoi_u(volume_text->GetValue());
            volume_slider->SetValue( value );
        }
        DECLARE_EVENT_TABLE()
    };
    
    BEGIN_EVENT_TABLE(TrackPropertiesDialog, wxDialog)
    EVT_BUTTON(200, TrackPropertiesDialog::okButton)
    EVT_BUTTON(202, TrackPropertiesDialog::cancelButton)
    
    EVT_COMMAND_SCROLL_THUMBTRACK(300, TrackPropertiesDialog::volumeSlideChanging)
    EVT_COMMAND_SCROLL_THUMBRELEASE(300, TrackPropertiesDialog::volumeSlideChanging)
    EVT_COMMAND_SCROLL_LINEUP(300, TrackPropertiesDialog::volumeSlideChanging)
    EVT_COMMAND_SCROLL_LINEDOWN(300, TrackPropertiesDialog::volumeSlideChanging)
    EVT_COMMAND_SCROLL_PAGEUP(300, TrackPropertiesDialog::volumeSlideChanging)
    EVT_COMMAND_SCROLL_PAGEDOWN(300, TrackPropertiesDialog::volumeSlideChanging)
    
    EVT_TEXT(301, TrackPropertiesDialog::volumeTextChanged)
    
    END_EVENT_TABLE()
    
    
    
    namespace TrackProperties
    {
        
        
        void show(Track* parent)
        {
            TrackPropertiesDialog frame(parent);
            frame.show();
        }
        
    }
    
    
}
