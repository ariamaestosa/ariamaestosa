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

//#include "wx/scrolbar.h"
//#include "wx/image.h"
#include "wx/spinctrl.h"

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
            // FIXME : instead of switching over type, use polymorphism so that each parameter can handle itself
            switch (m_parent->m_type)
            {
                case SETTING_ENUM:
                    m_combo->Select(m_parent->m_value);
                    break;
                    
                case SETTING_BOOL:
                    m_checkbox->SetValue(m_parent->m_value);
                    break;
                    
                case SETTING_INT:
                    m_number->SetValue(m_parent->m_value);
                    break;
            }
        }
        
        void updateValueFromWidget()
        {
            // FIXME : instead of switching over type, use polymorphism so that each parameter can handle itself
            switch (m_parent->m_type)
            {
                case SETTING_ENUM:
                    m_parent->m_value = m_combo->GetSelection();
                    break;
                    
                case SETTING_BOOL:
                    m_parent->m_value = m_checkbox->GetValue();
                    break;
                    
                case SETTING_INT:
                    m_parent->m_value = m_number->GetValue();
                    break;
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

PreferencesDialog::PreferencesDialog(wxFrame* parent, PreferencesData* data) :
wxDialog(parent, wxID_ANY,
         //I18N: - title of the preferences dialog
         _("Preferences"), wxPoint(100,100), wxSize(500,300), wxCAPTION )
{
    PreferencesDialog::data = data;
    
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
            {
                QuickBoxLayout box(this, vert_sizer);
                box.add(new wxStaticText(box.pane , wxID_ANY, settings[i].m_user_name ));
                
                w->m_combo = new wxChoice(box.pane, 1, wxDefaultPosition, wxDefaultSize, 
                                                   settings[i].m_choices );
                box.add(w->m_combo);
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
                box.add(new wxStaticText(box.pane, wxID_ANY, settings[i].m_user_name ));
                
                w->m_number = new wxSpinCtrl(box.pane, wxID_ANY,
                                                      wxString::Format(wxT("%i"), settings[i].m_value),
                                                      wxDefaultPosition, wxDefaultSize,  wxSP_ARROW_KEYS,
                                                      0, 99999 );
                box.add(w->m_number);
                break;
            }
        }
    }
    
    updateWidgetsFromValues();
    
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
    vert_sizer->SetSizeHints( this );
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
    data->save();
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
