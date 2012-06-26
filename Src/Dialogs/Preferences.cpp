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


#include "Dialogs/Preferences.h"

#include <iostream>

#include "AriaCore.h"
#include "Midi/Sequence.h"
#include "PreferencesData.h"

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/msgdlg.h>

using namespace AriaMaestosa;

namespace AriaMaestosa
{

    class QuickBoxLayout
    {
        wxBoxSizer* bsizer;
        
    public:
        
        wxPanel* pane;
        
        QuickBoxLayout(wxWindow* component, wxSizer* parent, int orientation=wxHORIZONTAL)
        {
            pane = new wxPanel(component);
            parent->Add(pane, 0, wxEXPAND);
            bsizer = new wxBoxSizer(orientation);
        }
        void add(wxWindow* window, int proportion = 1)
        {
            bsizer->Add(window, proportion, wxALL, 10);
        }
        ~QuickBoxLayout()
        {
            pane->SetSizer(bsizer);
            bsizer->Layout();
        }
    };
    
#if 0
#pragma mark -
#endif
    
    class SettingWidget
    {
    public:
        wxChoice*    m_combo;
        wxCheckBox*  m_checkbox;
        wxSpinCtrl*  m_number;
        wxWindow*    m_textbox;
        
        Setting*    m_parent;
        
        SettingWidget(Setting* parent)
        {
            m_parent   = parent;
            m_combo    = NULL;
            m_checkbox = NULL;
            m_number   = NULL;
            m_textbox  = NULL;
        }
        
        void updateWidgetFromValue()
        {
            switch (m_parent->m_type)
            { 
                case SETTING_ENUM:
                {
                    long asLong;
                    const bool longValueValid = m_parent->m_value.ToLong(&asLong);
                    if (not longValueValid)
                    {
                        std::cerr << "WARNING: invalid integer value <" << m_parent->m_value.mb_str() 
                                  << "> in preferences\n";
                        m_combo->SetSelection(0);
                        return;
                    }
                    m_combo->Select(asLong);
                    break;
                }
                    
                case SETTING_STRING_ENUM:
                {
                    int id = 0;
                    // TODO: handle case where value not found?
                    const int count = m_parent->m_choices.size();
                    for (int n=0; n<count; n++)
                    {
                        if (m_parent->m_choices[n] == m_parent->m_value)
                        {
                            id = n;
                            break;
                        }
                    }
                    m_combo->Select(id);
                    break;
                }
                    
                case SETTING_BOOL:
                {
                    if (m_parent->m_value == wxT("true"))
                    {
                        m_checkbox->SetValue(true);
                    }
                    else if (m_parent->m_value == wxT("false"))
                    {
                        m_checkbox->SetValue(false);
                    }
                    else
                    {
                        long asLong;
                        const bool longValueValid = m_parent->m_value.ToLong(&asLong);
                        if (not longValueValid)
                        {
                            std::cerr << "WARNING: invalid boolean value <" << m_parent->m_value.mb_str() 
                                      << "> in preferences\n";
                            return;  
                        }
                        m_checkbox->SetValue(asLong);
                    }
                    break;
                }
                    
                case SETTING_INT:
                {
                    long asLong;
                    const bool longValueValid = m_parent->m_value.ToLong(&asLong);
                    if (not longValueValid)
                    {
                        std::cerr << "WARNING: invalid integer value <" << m_parent->m_value.mb_str() 
                                  << "> in preferences\n";
                        return;  
                    }
                    m_number->SetValue(asLong);
                    break;
                }
                
                case SETTING_STRING:
                {
                    if (dynamic_cast<wxTextCtrl*>(m_textbox) != NULL)
                    {
                        dynamic_cast<wxTextCtrl*>(m_textbox)->SetValue(m_parent->m_value);
                    }
                    else if (dynamic_cast<wxComboBox*>(m_textbox) != NULL)
                    {
                        dynamic_cast<wxComboBox*>(m_textbox)->SetValue(m_parent->m_value);
                    }
                    break;
                }
                    
                default:
                {
                    ASSERT(false);
                    fprintf(stderr, "Unknown preferences data type : %i\n", m_parent->m_type);
                    return;
                }
            }
        }
        
        void updateValueFromWidget()
        {
            switch (m_parent->m_type)
            {
                case SETTING_ENUM:
                    m_parent->m_value = to_wxString(m_combo->GetSelection());
                    break;
                    
                case SETTING_STRING_ENUM:
                    m_parent->m_value = m_combo->GetStringSelection();
                    break;
                    
                case SETTING_BOOL:
                    m_parent->m_value = to_wxString(m_checkbox->GetValue());
                    break;
                    
                case SETTING_INT:
                    m_parent->m_value = to_wxString(m_number->GetValue());
                    break;
                
                case SETTING_STRING:
                    if (dynamic_cast<wxTextCtrl*>(m_textbox) != NULL)
                    {
                        m_parent->m_value = dynamic_cast<wxTextCtrl*>(m_textbox)->GetValue();
                    }
                    else if (dynamic_cast<wxComboBox*>(m_textbox) != NULL)
                    {
                        m_parent->m_value = dynamic_cast<wxComboBox*>(m_textbox)->GetValue();
                    }
                    break;
                    
                default:
                    ASSERT(false);
                    fprintf(stderr, "Unknown preferences data type : %i\n", m_parent->m_type);
                    return;
            }
        }
    };
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark PreferencesDialog
#endif

BEGIN_EVENT_TABLE(PreferencesDialog, wxDialog)
EVT_BUTTON(wxID_OK, PreferencesDialog::okClicked)
EVT_BUTTON(wxID_CANCEL, PreferencesDialog::onCancel)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(wxWindow* parent, PreferencesData* data) :
wxDialog(parent, wxID_ANY,
         //I18N: - title of the preferences dialog
         _("Preferences"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
{
    m_data = data;
    m_sound_font_selected = false;
    
    ptr_vector<Setting>& settings = PreferencesData::getInstance()->getSettings();
    
    // ---- add widgets
    vert_sizer = new wxBoxSizer(wxVERTICAL);
    const int settingAmount = settings.size();
    for (int i=0; i<settingAmount; i++)
    {
        if (not settings[i].m_visible_in_preferences) continue;
        
        SettingWidget* w = new SettingWidget(settings.get(i));
        m_setting_widgets.push_back(w);
        
        switch (settings[i].m_type)
        {
                
            case SETTING_ENUM:
            case SETTING_STRING_ENUM:
            {
                QuickBoxLayout box(this, vert_sizer);
                box.add(new wxStaticText(box.pane , wxID_ANY, settings[i].m_user_name), 1);
                
                w->m_combo = new wxChoice(box.pane, 1, wxDefaultPosition, wxDefaultSize, 
                                                   settings[i].m_choices );
                box.add(w->m_combo, 1);
                break;
            }
            case SETTING_BOOL:
            {
                w->m_checkbox = new wxCheckBox(this, wxID_ANY, settings[i].m_user_name,
                                                        wxDefaultPosition, wxDefaultSize );
                vert_sizer->Add( w->m_checkbox, 0, wxALL, 10 );
                break;
            }
            case SETTING_INT:
            {
                QuickBoxLayout box(this, vert_sizer);
                box.add(new wxStaticText(box.pane, wxID_ANY, settings[i].m_user_name ), 1);
                
                w->m_number = new wxSpinCtrl(box.pane, wxID_ANY,
                                                      settings[i].m_value,
                                                      wxDefaultPosition, wxDefaultSize,  wxSP_ARROW_KEYS,
                                                      0, 99999 );
                box.add(w->m_number);
                break;
            }
            case SETTING_STRING:
            {
                if (settings[i].m_subtype == SETTING_SUBTYPE_FILE_OR_DEFAULT)
                {
                    QuickBoxLayout box(this, vert_sizer);
                    box.add(new wxStaticText(box.pane, wxID_ANY, settings[i].m_user_name), 1);
                    
                    wxString choices[] = {wxT("System soundbank"), _("Browse...")};
                    wxComboBox* combo = new wxComboBox(box.pane, wxID_ANY, settings[i].m_value,
                                                       wxDefaultPosition, wxDefaultSize,
                                                       2, choices);                    
                    w->m_textbox = combo;
                    combo->Connect(wxEVT_COMMAND_COMBOBOX_SELECTED,
                                   wxCommandEventHandler(PreferencesDialog::onComboSelection), NULL, this);
                    ASSERT(dynamic_cast<wxWindow*>(w->m_textbox) != NULL);
                    box.add(dynamic_cast<wxWindow*>(w->m_textbox));
                }
                else
                {
                    QuickBoxLayout box(this, vert_sizer);
                    box.add(new wxStaticText(box.pane, wxID_ANY, settings[i].m_user_name ), 1);
                    
                    w->m_textbox = new wxTextCtrl(box.pane, wxID_ANY, settings[i].m_value);
                    ASSERT(dynamic_cast<wxWindow*>(w->m_textbox) != NULL);
                    box.add(dynamic_cast<wxWindow*>(w->m_textbox));
                }
                break;
            }
        }
    }
    
    updateWidgetsFromValues();
    
    vert_sizer->AddStretchSpacer();



    // -----------------------------------
    ok_btn = new wxButton(this, wxID_OK, _("OK"));
    cancel_btn = new wxButton(this, wxID_CANCEL, _("Cancel"));

    wxStdDialogButtonSizer* stdDialogButtonSizer = new wxStdDialogButtonSizer();
    stdDialogButtonSizer->AddButton(ok_btn);
    stdDialogButtonSizer->AddButton(cancel_btn);
    stdDialogButtonSizer->Realize();
    vert_sizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);


    //I18N: - in the preferences dialog
    wxStaticText* effect_label = new wxStaticText(this, wxID_ANY, _("Changes will take effect next time you open the app."),
                                                  wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
    effect_label->SetFont( wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL) );
    vert_sizer->Add( effect_label, 0, wxEXPAND | wxALL, 10 );
    
    
    ok_btn -> SetDefault();
    
    SetSizer( vert_sizer );
    vert_sizer->Layout();
    vert_sizer->SetSizeHints( this );
    Center();
}

// ---------------------------------------------------------------------------------------------------------

PreferencesDialog::~PreferencesDialog()
{
}

// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::show()
{
    m_sound_font_selected = false;
    
    Center();
#ifdef __WXOSX_COCOA__
    ShowWindowModal();
    modalCode = GetReturnCode();
#else
    modalCode = ShowModal();
#endif
}

// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::okClicked(wxCommandEvent& evt)
{
    updateValuesFromWidgets();
    m_data->save();
    wxDialog::EndModal(wxID_OK);
    
    if (m_sound_font_selected)
    {
        wxMessageBox( _("You will need to restart Aria Maestosa for changes to be effective.") );
        m_sound_font_selected = false;
    }
}



// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::onCancel(wxCommandEvent& evt)
{
    wxDialog::EndModal(wxID_CANCEL);
}



// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::onComboSelection(wxCommandEvent& evt)
{
    ASSERT(dynamic_cast<wxItemContainerImmutable*>(evt.GetEventObject()) != NULL);
    if (dynamic_cast<wxItemContainerImmutable*>(evt.GetEventObject())->GetSelection() == 1)
    {
        wxString filePath = showFileDialog(this, _("Select file"), wxT(""), wxT(""),
                                           wxString(_("Sound Font"))+wxT("|*.sf2"), false /*open*/);
        if (not filePath.IsEmpty())
        {
            if (dynamic_cast<wxTextCtrl*>(evt.GetEventObject()) != NULL)
            {
                dynamic_cast<wxTextCtrl*>(evt.GetEventObject())->SetValue(filePath);
            }
            else if (dynamic_cast<wxComboBox*>(evt.GetEventObject()) != NULL)
            {
                dynamic_cast<wxComboBox*>(evt.GetEventObject())->SetValue(filePath);
            }
            m_sound_font_selected = true;
        }
        else
        {
            dynamic_cast<wxItemContainer*>(evt.GetEventObject())->SetSelection(0);
        }
    }
    else
    {
        m_sound_font_selected = true;
    }
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Sync
#endif

void PreferencesDialog::updateWidgetsFromValues()
{
    const int settingAmount = m_setting_widgets.size();
    for (int i=0; i<settingAmount; i++)
    {
        m_setting_widgets[i].updateWidgetFromValue();
    }
}

// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::updateValuesFromWidgets()
{    
    // update all values from widgets
    const int settingAmount = m_setting_widgets.size();
    for (int i=0; i<settingAmount; i++)
    {
        m_setting_widgets[i].updateValueFromWidget();
    }
}

// ---------------------------------------------------------------------------------------------------------

