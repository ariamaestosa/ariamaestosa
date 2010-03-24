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

#define DEFINE_SETTING_NAMES        // X-macros
#include "PreferencesData.h"
#undef DEFINE_SETTING_NAMES

#include "languages.h"
#include <wx/config.h>


using namespace AriaMaestosa; 

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

#if 0
#pragma mark Setting
#endif

Setting::Setting(wxString name, wxString user_name, SettingType type,
                 bool visibleInPreferences, int default_value)
{
    m_name                   = name;
    m_user_name              = user_name;
    m_type                   = type;
    m_value                  = default_value;
    m_visible_in_preferences = visibleInPreferences;
}

// ----------------------------------------------------------------------------------------------------

void Setting::addChoice(wxString choice)
{
    m_choices.Add(choice);
}

// ----------------------------------------------------------------------------------------------------

void Setting::setChoices(wxArrayString choices)
{
    m_choices = choices;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark PreferencesData
#endif

DEFINE_SINGLETON(AriaMaestosa::PreferencesData);

PreferencesData::PreferencesData()
{
    fillSettingsVector();
    
    // --- read values from file
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    long value;
    
    // for each setting
    const int settingAmount = m_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        // see if this value is defined in file
        if (prefs->Read( m_settings[i].m_name, &value) )
        {
            m_settings[i].m_value = value;
        }
    }
}

// ----------------------------------------------------------------------------------------------------

void PreferencesData::fillSettingsVector()
{
    // ---- language
    Setting* languages = new Setting(fromCString(SETTING_ID_LANGUAGE), wxT("Language"), SETTING_ENUM, true,
                                     getDefaultLanguageAriaID() );
    languages->setChoices( getLanguageList() );
    m_settings.push_back( languages );
    
    // ---- play during edit
    Setting* play = new Setting(fromCString(SETTING_ID_PLAY_DURING_EDIT), _("Play during edit (default value)"),
                                SETTING_ENUM, true, 1 );
    play->addChoice(_("Always"));        // PLAY_ALWAYS = 0,
    play->addChoice(_("On note change"));// PLAY_ON_CHANGE = 1,
    play->addChoice(_("Never"));         // PLAY_NEVER = 2
    m_settings.push_back( play );
    
    // ---- score view
    Setting* scoreview = new Setting(fromCString(SETTING_ID_SCORE_VIEW), _("Default Score View"),
                                     SETTING_ENUM, true, 0 );
    scoreview->addChoice(_("Both Musical and Linear"));
    scoreview->addChoice(_("Musical Only"));
    scoreview->addChoice(_("Linear Only"));
    m_settings.push_back( scoreview );
    
    // ---- follow playback
    Setting* followp = new Setting(fromCString(SETTING_ID_FOLLOW_PLAYBACK), _("Follow playback by default"),
                                   SETTING_BOOL, true, 0 );
    m_settings.push_back( followp );
    
#ifdef __WXGTK__
    /*
     Setting usePort(wxT("usePort"), _("Automatically use this Alsa port (format 'Client:Port')"),
     SETTING_BOOL, 0 );
     m_settings.push_back( usePort );
     
     Setting alsaClient(wxT("alsaClient"), _("Alsa Client"), SETTING_INT, 128 );
     m_settings.push_back( alsaClient );
     Setting alsaPort(wxT("alsaPort"), _("Alsa Port"), SETTING_INT, 0 );
     m_settings.push_back( alsaPort );
     */
    Setting* launchTim = new Setting(fromCString(SETTING_ID_LAUNCH_TIMIDITY),
                                     _("Automatically launch TiMidity and pick a port"),
                                     SETTING_BOOL, false, 1 );
    m_settings.push_back( launchTim );
#endif
    
    // ---- printing
    Setting* marginLeft = new Setting(fromCString(SETTING_ID_MARGIN_LEFT), wxT(""),
                                      SETTING_INT, false, 12 );
    m_settings.push_back( marginLeft );
    
    Setting* marginRight = new Setting(fromCString(SETTING_ID_MARGIN_RIGHT), wxT(""),
                                      SETTING_INT, false, 12 );
    m_settings.push_back( marginRight );
    
    Setting* marginTop = new Setting(fromCString(SETTING_ID_MARGIN_TOP), wxT(""),
                                      SETTING_INT, false, 12 );
    m_settings.push_back( marginTop );
    
    Setting* marginBottom = new Setting(fromCString(SETTING_ID_MARGIN_BOTTOM), wxT(""),
                                      SETTING_INT, false, 16 );
    m_settings.push_back( marginBottom );
}

// ----------------------------------------------------------------------------------------------------

void PreferencesData::save()
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    const int settingAmount = m_settings.size();
    for(int i=0; i<settingAmount; i++)
    {
        prefs->Write( m_settings[i].m_name, m_settings[i].m_value );
    }
    
    prefs->Flush();
}

// ----------------------------------------------------------------------------------------------------

long PreferencesData::getValue(wxString entryName) const
{
    
    const int settingAmount = m_settings.size();
    for (int i=0; i<settingAmount; i++)
    {
        if ( m_settings[i].m_name == entryName )
        {
            //std::cout << m_settings[i].m_value << std::endl;
            return m_settings[i].m_value;
        }
    }
    std::cout << "prefs value not found : " << entryName.mb_str() << std::endl;
    return -1;
}

// ----------------------------------------------------------------------------------------------------

void PreferencesData::setValue(wxString entryName, long newValue)
{
    const int settingAmount = m_settings.size();
    for (int i=0; i<settingAmount; i++)
    {
        if ( m_settings[i].m_name == entryName )
        {
            m_settings[i].m_value = newValue;
            return;
        }
    }
    assert(false);
}

// ----------------------------------------------------------------------------------------------------
