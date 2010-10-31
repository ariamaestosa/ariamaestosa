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
#include <wx/stattext.h>
#include <wx/button.h>

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
        wxChoice*   m_combo;
        wxCheckBox* m_checkbox;
        wxSpinCtrl* m_number;
        
        Setting*    m_parent;
        
        SettingWidget(Setting* parent)
        {
            m_parent   = parent;
            m_combo    = NULL;
            m_checkbox = NULL;
            m_number   = NULL;
        }
        
        void updateWidgetFromValue()
        {
            long asLong;
            const bool longValueValid = m_parent->m_value.ToLong(&asLong);
            
            switch (m_parent->m_type)
            { 
                case SETTING_ENUM:
                {
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
                    if (not longValueValid)
                    {
                        std::cerr << "WARNING: invalid boolean value <" << m_parent->m_value.mb_str() 
                                  << "> in preferences\n";
                        return;  
                    }
                    m_checkbox->SetValue(asLong);
                    break;
                }
                    
                case SETTING_INT:
                {
                    if (not longValueValid)
                    {
                        std::cerr << "WARNING: invalid integer value <" << m_parent->m_value.mb_str() 
                                  << "> in preferences\n";
                        return;  
                    }
                    m_number->SetValue(asLong);
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
EVT_BUTTON(2, PreferencesDialog::okClicked)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog(wxWindow* parent, PreferencesData* data) :
wxDialog(parent, wxID_ANY,
         //I18N: - title of the preferences dialog
         _("Preferences"), wxPoint(100,100), wxSize(500, 350), wxCAPTION | wxRESIZE_BORDER)
{
    m_data = data;
    
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
        }
    }
    
    updateWidgetsFromValues();
    
    vert_sizer->AddStretchSpacer();
    
    // -----------------------------------
    ok_btn = new wxButton(this, 2, wxT("OK"));
    vert_sizer->Add( ok_btn, 0, wxALL, 10 );
    
    //I18N: - in the preferences dialog
    wxStaticText* effect_label = new wxStaticText(this, wxID_ANY,  _("Changes will take effect next time you open the app."));
    effect_label->SetFont( wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL) );
    vert_sizer->Add( effect_label, 0, wxALL, 10 );
    
    ok_btn -> SetDefault();
    
    SetSizer( vert_sizer );
    vert_sizer->Layout();
    //vert_sizer->SetSizeHints( this );
}

// ---------------------------------------------------------------------------------------------------------

PreferencesDialog::~PreferencesDialog()
{
}

// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::show()
{
    Center();
    modalCode = ShowModal();
}

// ---------------------------------------------------------------------------------------------------------

void PreferencesDialog::okClicked(wxCommandEvent& evt)
{
    updateValuesFromWidgets();
    m_data->save();
    wxDialog::EndModal(modalCode);
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

