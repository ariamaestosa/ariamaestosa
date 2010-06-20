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
#include "Dialogs/PresetEditor.h"
#include "Pickers/KeyPicker.h"
#include "PresetManager.h"

#include "wx/wx.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

PresetEditor::PresetEditor(wxWindow* parent, PresetGroup* presets) :
        wxDialog(parent, wxID_ANY, _("Presets"), wxDefaultPosition, wxSize(500,400),
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    m_preset_group = presets;
    
    wxArrayString choices;
    const int count = presets->getPresets().size();
    ASSERT_E(count, >, 0);
    
    m_presets = new IPreset*[count];
    m_preset_count = count;
    
    for (int n=0; n<count; n++)
    {
        choices.Add(presets->getPresets()[n].getName());
        m_presets[n] = presets->getPresets().get(n);
    }
    
    wxFlexGridSizer* sizer = new wxFlexGridSizer(2,2,5,5);
    sizer->AddGrowableCol(0, 1);
    sizer->AddGrowableRow(0, 1);

    m_list = new  wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
    m_list->SetSelection(0);
    sizer->Add(m_list, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 10);
    
    wxBoxSizer* btnSizer = new wxBoxSizer(wxVERTICAL);
    wxButton* duplicateBtn = new wxButton(this, wxID_ANY, _("Duplicate"));
    btnSizer->Add(duplicateBtn, 0, wxEXPAND | wxALL, 5);
    wxButton* deleteBtn = new wxButton(this, wxID_ANY, _("Delete"));
    btnSizer->Add(deleteBtn, 0, wxEXPAND | wxALL, 5);
    sizer->Add(btnSizer, 0, wxTOP | wxRIGHT, 5);
    
    wxButton* okBtn = new wxButton(this, wxID_ANY, _("Close"));
    okBtn->SetDefault();
    sizer->Add(okBtn, 0, wxLEFT | wxBOTTOM, 10);

    
    deleteBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PresetEditor::onDelete), NULL, this);
    duplicateBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PresetEditor::onDuplicate), NULL, this);
    okBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PresetEditor::onClose), NULL, this);

    SetSizer(sizer);
    Centre();
    ShowModal();
}

// ----------------------------------------------------------------------------------------------------------

PresetEditor::~PresetEditor()
{
    delete[] m_presets;
}

// ----------------------------------------------------------------------------------------------------------

void PresetEditor::onDuplicate(wxCommandEvent& evt)
{
    const int selection = m_list->GetSelection();
    if (selection >= 0 and selection < m_preset_count)
    {
        IPreset* preset = m_presets[selection];
        
        // I18N: when duplicating a preset
        wxString name = wxGetTextFromUser(_("Enter the name of the duplicate"));
        if (not name.IsEmpty())
        {
            m_preset_group->add(preset->clone(name));
            m_preset_group->write();
        }
        updateList();
        Core::getKeyPicker()->updateUserPresetsMenu();
    }
}

// ----------------------------------------------------------------------------------------------------------

void PresetEditor::onDelete(wxCommandEvent& evt)
{
    const int selection = m_list->GetSelection();
    if (selection >= 0 and selection < m_preset_count)
    {
        IPreset* preset = m_presets[selection];
        
        const int answer = wxMessageBox(_("Are you sure you want to delete this preset? This cannot be undone."),
                                        _("Confirm"), wxYES_NO);
        if (answer == wxNO) return;
        
        m_preset_group->remove(preset);
        m_preset_group->write();

        updateList();
        Core::getKeyPicker()->updateUserPresetsMenu();
    }
}

// ----------------------------------------------------------------------------------------------------------

void PresetEditor::updateList()
{
    delete[] m_presets;
    m_list->Clear();
    
    wxArrayString choices;
    const int count = m_preset_group->getPresets().size();
    ASSERT_E(count, >, 0);
    
    m_presets = new IPreset*[count];
    m_preset_count = count;
    
    for (int n=0; n<count; n++)
    {
        m_list->Append(m_preset_group->getPresets()[n].getName());
        m_presets[n] = m_preset_group->getPresets().get(n);
    }
    
    m_list->SetSelection(0);
}

// ----------------------------------------------------------------------------------------------------------

void PresetEditor::onClose(wxCommandEvent& evt)
{
    EndModal(GetReturnCode());
}

// ----------------------------------------------------------------------------------------------------------
