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
#include <wx/config.h>
#include "AriaCore.h"

#include "GUI/MainFrame.h"
#include "languages.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa {

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

enum SettingType
{
    SETTING_ENUM,
    SETTING_BOOL
};

class Setting
{
public:
    wxChoice* m_combo;
    wxCheckBox* m_checkbox;

    wxString      m_name;
    wxString      m_user_name;
    wxArrayString m_choices;
    SettingType   m_type;
    int           m_value;

    Setting(wxString name, wxString user_name, SettingType type, int default_value = 0)
    {
        m_name = name;
        m_user_name = user_name;
        m_type = type;
        m_value = default_value;
    }
    void addChoice(wxString choice)
    {
        m_choices.Add(choice);
    }
    void addChoices(wxArrayString choices)
    {
        m_choices = choices;
    }


};

std::vector<Setting> g_settings;


// ---- define all settings
void fillSettingsVector()
{
    // ---- language
    Setting languages(wxT("lang"), wxT("Language"), SETTING_ENUM, getDefaultLanguageAriaID() );
    languages.addChoices( getLanguageList() );
    g_settings.push_back( languages );

    // ---- play during edit
    Setting play(wxT("playDuringEdit"), _("Play during edit (default value)"), SETTING_ENUM, 1 );
    play.addChoice(_("Always"));        // PLAY_ALWAYS = 0,
    play.addChoice(_("On note change"));// PLAY_ON_CHANGE = 1,
    play.addChoice(_("Never"));         // PLAY_NEVER = 2
    g_settings.push_back( play );

    // ---- score view
    Setting scoreview(wxT("scoreview"), _("Default Score View"), SETTING_ENUM, 0 );
    scoreview.addChoice(_("Both Musical and Linear"));
    scoreview.addChoice(_("Musical Only"));
    scoreview.addChoice(_("Linear Only"));
    g_settings.push_back( scoreview );

    // ---- follow playback
    Setting followp(wxT("followPlayback"), _("Follow playback by default"), SETTING_BOOL, 0 );
    g_settings.push_back( followp );
}


BEGIN_EVENT_TABLE(Preferences, wxDialog)
EVT_BUTTON(2, Preferences::okClicked)
END_EVENT_TABLE()

Preferences::Preferences(MainFrame* parent) : wxDialog(parent, wxID_ANY,
                                                       //I18N: - title of the preferences dialog
                                                       _("Preferences"),
                                                       wxPoint(100,100), wxSize(500,300), wxCAPTION )
{
    Preferences::parent = parent;

    fillSettingsVector();

    // ---- add widgets
    vert_sizer = new wxBoxSizer(wxVERTICAL);
    const int settingAmount = g_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        if( g_settings[i].m_type == SETTING_ENUM )
        {
            QuickBoxLayout box(this, vert_sizer);
            //I18N: - in preferences window
            box.add(new wxStaticText(box.pane , wxID_ANY, g_settings[i].m_user_name ));

            g_settings[i].m_combo = new wxChoice(box.pane, 1, wxDefaultPosition, wxDefaultSize,  g_settings[i].m_choices );
            box.add(g_settings[i].m_combo);

            //g_settings[i].m_combo->Connect( g_settings[i].m_combo->GetId(),
            //                                wxEVT_COMMAND_CHOICE_SELECTED,
            //                                wxCommandEventHandler(Preferences::onSettingChanged),
            //                                NULL, this );
        }
        else if( g_settings[i].m_type == SETTING_BOOL )
        {
            g_settings[i].m_checkbox = new wxCheckBox(this, wxID_ANY, g_settings[i].m_user_name, wxDefaultPosition, wxDefaultSize );
            vert_sizer->Add( g_settings[i].m_checkbox, 0, wxALL, 10 );

            //g_settings[i].m_checkbox->Connect(
            //        g_settings[i].m_combo->GetId(),
            //        wxEVT_COMMAND_CHECKBOX_CLICKED,
            //        wxCommandEventHandler(Preferences::onSettingChanged),
            //        NULL, this );
        }
    }


    // --- read values from file
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    long value;
    // for each setting
    for(int i=0; i<settingAmount; i++)
    {
        // see if this value is defined in file
        if(prefs->Read( g_settings[i].m_name, &value) )
        {
            g_settings[i].m_value = value;
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

Preferences::~Preferences()
{
}

void Preferences::updateWidgetsFromValues()
{
    const int settingAmount = g_settings.size();

    // ---- update view to reflect values
    for(int i=0; i<settingAmount; i++)
    {
        if( g_settings[i].m_type == SETTING_ENUM )
        {
            g_settings[i].m_combo->Select(g_settings[i].m_value);
        }
        else if( g_settings[i].m_type == SETTING_BOOL )
        {
            g_settings[i].m_checkbox->SetValue(g_settings[i].m_value);
        }
    }
}

void Preferences::show()
{
    Center();
    modalCode = ShowModal();
}

void Preferences::updateValuesFromWidgets()
{
    // update all values from widgets
    const int settingAmount = g_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        if( g_settings[i].m_type == SETTING_ENUM )
        {
            g_settings[i].m_value = g_settings[i].m_combo->GetSelection();
        }
        else if( g_settings[i].m_type == SETTING_BOOL )
        {
            g_settings[i].m_value = g_settings[i].m_checkbox->GetValue();
        }
    }
}


void Preferences::saveFromValues()
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();

    const int settingAmount = g_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        prefs->Write( g_settings[i].m_name, g_settings[i].m_value );
    }

    prefs->Flush();
}

void Preferences::okClicked(wxCommandEvent& evt)
{
    updateValuesFromWidgets();
    saveFromValues();
    wxDialog::EndModal(modalCode);
}

long Preferences::getValue(wxString entryName)
{

    const int settingAmount = g_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        if( g_settings[i].m_name == entryName )
        {
            std::cout << g_settings[i].m_value << std::endl;
            return g_settings[i].m_value;
        }
    }
    std::cout << "prefs value not found : " << entryName.mb_str() << std::endl;
    return -1;
}

}
