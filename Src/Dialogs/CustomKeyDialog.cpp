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
#include "Dialogs/CustomKeyDialog.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/KeyPresets.h"
#include "Pickers/KeyPicker.h"
#include "PresetManager.h"

#include "wx/wx.h"
#include "wx/notebook.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

CustomKeyDialog::CustomKeyDialog(wxWindow* parent, GraphicalTrack* gtrack) :
wxDialog(parent, wxID_ANY, _("Custom Key Editor"), wxDefaultPosition,
         wxSize(800,600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    m_parent = gtrack;
    
    const KeyInclusionType* curr_key_notes = gtrack->track->getKeyNotes();
    
    wxBoxSizer* maximizePaneSizer = new wxBoxSizer(wxVERTICAL);
    wxPanel* pane = new wxPanel(this);
    maximizePaneSizer->Add(pane, 1, wxEXPAND);
    SetSizer(maximizePaneSizer);
    
    wxBoxSizer* globalSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* title = new wxStaticText(pane, wxID_ANY,
                                           _("Select which notes should be part of the custom key"));
    globalSizer->Add(title, 0, wxALL, 5);
    
    m_notebook = new wxNotebook( pane, wxID_ANY );
    {
        m_page1_id = wxNewId();
        wxPanel* page = new wxPanel( m_notebook, m_page1_id );
        wxBoxSizer* vsizer = new wxBoxSizer(wxVERTICAL);
        
        Note12 note;
        int    octave;
        
        const int pitchFrom = Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 4);
        const int pitchTo   = Editor::findNotePitch(NOTE_7_C, PITCH_SIGN_NONE, 4);
        
        for (int pitch=pitchFrom; pitch<=pitchTo; pitch++)
        {
            const bool success = Editor::findNoteName(pitch, &note, &octave);
            ASSERT(success);
            wxString label = NOTE_12_NAME[note];
            wxCheckBox* cb = new wxCheckBox(page, wxID_ANY, label, 
                                            wxDefaultPosition, wxDefaultSize,
                                            wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);
            
            switch (curr_key_notes[pitch])
            {
                case KEY_INCLUSION_FULL :
                    cb->Set3StateValue( wxCHK_CHECKED );
                    break;
                case KEY_INCLUSION_ACCIDENTAL :
                    cb->Set3StateValue( wxCHK_UNDETERMINED );
                    break;
                case KEY_INCLUSION_NONE :
                    cb->Set3StateValue( wxCHK_UNCHECKED );
                    break;
            }
            
            vsizer->Add(cb, 0, wxALL, 3);
            
            m_check_boxes_one_octave[pitch - pitchFrom] = cb;
        }
        
        page->SetSizer(vsizer);
        
        //I18N: in custom key editor, this is the option to edit notes on a single octave
        m_notebook->AddPage(page, _("All Octaves are Similar"));
    }
    {
        m_page2_id = wxNewId();
        wxScrolledWindow* scrollpane = new wxScrolledWindow( m_notebook, m_page2_id );
        
        wxBoxSizer* within_scrollpane_sizer = new wxBoxSizer( wxVERTICAL );
        
        Note12 note;
        int    octave;
        
        for (int pitch=4; pitch<=131; pitch++)
        {
            const bool success = Editor::findNoteName(pitch, &note, &octave);
            ASSERT(success);
            
            wxString label = NOTE_12_NAME[note] + wxT(" ") + wxString::Format(wxT("%i"), octave);
            wxCheckBox* cb = new wxCheckBox(scrollpane, wxID_ANY, label,
                                            wxDefaultPosition, wxDefaultSize, 
                                            wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);
            //cb->SetValue( curr_key_notes[pitch] );
            switch (curr_key_notes[pitch])
            {
                case KEY_INCLUSION_FULL :
                    cb->Set3StateValue( wxCHK_CHECKED );
                    break;
                case KEY_INCLUSION_ACCIDENTAL :
                    cb->Set3StateValue( wxCHK_UNDETERMINED );
                    break;
                case KEY_INCLUSION_NONE :
                    cb->Set3StateValue( wxCHK_UNCHECKED );
                    break;
            }
            
            
            within_scrollpane_sizer->Add(cb, 0, wxALL, 3);
            
            m_check_boxes[pitch] = cb;
        }
        
        scrollpane->SetSizer(within_scrollpane_sizer);
        scrollpane->FitInside();
        scrollpane->SetScrollRate(5, 5);
        
        //I18N: in custom key editor, this is the option to edit all octaves independently
        m_notebook->AddPage(scrollpane, _("All Octaves are Independent"));
    }
    globalSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 2);
    
    //I18N: in custom key editor
    //wxButton* copySettingsBtn = new wxButton( pane, wxID_COPY, _("Copy settings from another track...") );
    //over_sizer->Add(copySettingsBtn, 0, wxALL, 5);
    
    //I18N: in the key editor
    globalSizer->Add( new wxStaticText(this, wxID_ANY, _("* Checked notes are included in the key")), 0, wxLEFT | wxTOP, 5 );
    //I18N: in the key editor
    globalSizer->Add( new wxStaticText(this, wxID_ANY, _("* \"Half-checked\" notes are included as accidentals")), 0, wxLEFT, 5 );
    //I18N: in the key editor
    globalSizer->Add( new wxStaticText(this, wxID_ANY, _("* Unchecked notes are excluded (for instance, they could be unplayable on your instrument)")), 0, wxLEFT | wxBOTTOM, 5 );
    
    {
        wxPanel* buttonsPane = new wxPanel(pane);
        globalSizer->Add(buttonsPane, 0, wxEXPAND | wxALL, 5);
        
        wxButton* ok_btn     = new wxButton(buttonsPane, wxID_OK, _("OK"));
        wxButton* cancel_btn = new wxButton(buttonsPane, wxID_CANCEL, _("Cancel"));
        wxButton* presetBtn  = new wxButton(buttonsPane, wxNewId(), _("Save as preset..."));
        presetBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(CustomKeyDialog::saveAsPreset),
                           NULL, this);
        
        wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
        btnSizer->Add(presetBtn, 0, wxALL, 5);
        btnSizer->AddStretchSpacer();
        btnSizer->Add(cancel_btn, 0, wxALL, 5);
        btnSizer->Add(ok_btn, 0, wxALL, 5);
        buttonsPane->SetSizer(btnSizer);
        
        ok_btn->SetDefault();
    }
    
    pane->SetSizer(globalSizer);
    Layout();
    Centre();
}

// ----------------------------------------------------------------------------------------------------------

void CustomKeyDialog::buildKey(KeyInclusionType customKey[131])
{
    customKey[0] = KEY_INCLUSION_NONE;
    customKey[1] = KEY_INCLUSION_NONE;
    customKey[2] = KEY_INCLUSION_NONE;
    customKey[3] = KEY_INCLUSION_NONE;
    
    const int currPage = m_notebook->GetCurrentPage()->GetId();
    if (currPage == m_page1_id)
    {
        for (int pitch=4; pitch<=130; pitch++)
        {
            switch (m_check_boxes_one_octave[pitch % 12]->Get3StateValue())
            {
                case wxCHK_CHECKED:
                    customKey[pitch] = KEY_INCLUSION_FULL;
                    break;
                case wxCHK_UNDETERMINED:
                    customKey[pitch] = KEY_INCLUSION_ACCIDENTAL;
                    break;
                case wxCHK_UNCHECKED:
                    customKey[pitch] = KEY_INCLUSION_NONE;
                    break;
            }
        }
    }
    else if (currPage == m_page2_id)
    {
        for (int pitch=4; pitch<=130; pitch++)
        {
            switch (m_check_boxes[pitch]->Get3StateValue())
            {
                case wxCHK_CHECKED:
                    customKey[pitch] = KEY_INCLUSION_FULL;
                    break;
                case wxCHK_UNDETERMINED:
                    customKey[pitch] = KEY_INCLUSION_ACCIDENTAL;
                    break;
                case wxCHK_UNCHECKED:
                    customKey[pitch] = KEY_INCLUSION_NONE;
                    break;
            }
        }
    }
    else
    {
        assert(false);
        EndModal( GetReturnCode() );
    }
}

// ----------------------------------------------------------------------------------------------------------

void CustomKeyDialog::onOK(wxCommandEvent& evt)
{
    KeyInclusionType customKey[131];
    buildKey(customKey);
    
    m_parent->track->setCustomKey(customKey);
    
    EndModal( GetReturnCode() );
}

// ----------------------------------------------------------------------------------------------------------

void CustomKeyDialog::onCancel(wxCommandEvent& evt)
{
    EndModal( GetReturnCode() );
}

// ----------------------------------------------------------------------------------------------------------

void CustomKeyDialog::saveAsPreset(wxCommandEvent& evt)
{
    KeyInclusionType customKey[131];
    buildKey(customKey);
    
    wxString name = wxGetTextFromUser(_("Enter the name of the preset"));
    if (not name.IsEmpty())
    {
        KeyPreset* newInstance = new KeyPreset(name.mb_str(), customKey);
        KeyPresetGroup::getInstance()->add(newInstance);
        KeyPresetGroup::getInstance()->write();
        Core::getKeyPicker()->updateUserPresetsMenu();
    }
}



BEGIN_EVENT_TABLE(CustomKeyDialog, wxDialog)

EVT_COMMAND(wxID_OK,     wxEVT_COMMAND_BUTTON_CLICKED, CustomKeyDialog::onOK )
EVT_COMMAND(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, CustomKeyDialog::onCancel )
//EVT_COMMAND(wxID_COPY,   wxEVT_COMMAND_BUTTON_CLICKED, CustomKeyDialog::onCopySettings )

END_EVENT_TABLE()
