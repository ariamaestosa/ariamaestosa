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
#include <wx/intl.h>
#include <wx/font.h>
#include <wx/settings.h>

#include "Midi/Players/PlatformMidiManager.h"


using namespace AriaMaestosa; 

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark Setting
#endif

Setting::Setting(wxString name, wxString user_name, SettingType type,
                 bool visibleInPreferences, wxString default_value)
{
    m_name                   = name;
    m_user_name              = user_name;
    m_type                   = type;
    m_value                  = default_value;
    m_visible_in_preferences = visibleInPreferences;
}

// ----------------------------------------------------------------------------------------------------------

void Setting::addChoice(wxString choice)
{
    m_choices.Add(choice);
}

// ----------------------------------------------------------------------------------------------------------

void Setting::setChoices(wxArrayString choices)
{
    m_choices = choices;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark PreferencesData
#endif

DEFINE_SINGLETON(AriaMaestosa::PreferencesData);

PreferencesData::PreferencesData()
{
    m_inited = false;
}

void PreferencesData::init()
{
    if (m_inited) return;
    
    prepareLanguageEntry();
    readValues();
    
    initLanguageSupport();
    fillSettingsVector();
    readValues();
    
    m_inited = true;
}

// ----------------------------------------------------------------------------------------------------------

void PreferencesData::readValues()
{
    // --- read values from file
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    //long value;
    
    // for each setting
    const int settingAmount = m_settings.size();
    for (int i=0; i<settingAmount; i++)
    {
        // see if this value is defined in file
        if (prefs->Read( m_settings[i].m_name, &m_settings[i].m_value) )
        {
            //m_settings[i].m_value = value;
        }
    }    
}

// ----------------------------------------------------------------------------------------------------------

void PreferencesData::prepareLanguageEntry()
{
    // ---- language
    Setting* languages = new Setting(fromCString(SETTING_ID_LANGUAGE), _("Language"), SETTING_ENUM, true,
                                     to_wxString(getDefaultLanguageAriaID()) );
    languages->setChoices( getLanguageList() );
    m_settings.push_back( languages );
}

// ----------------------------------------------------------------------------------------------------------

void PreferencesData::fillSettingsVector()
{
    // FIXME: don't hardcode driver names?
#if defined(__WXMAC__)
    wxString defaultMidiDriver(wxT("QuickTime/AudioToolkit"));
#elif defined(__WXGTK__)
    #ifdef _ALSA
    wxString defaultMidiDriver(wxT("Linux/ALSA"));
    #else
    wxString defaultMidiDriver(wxT("Jack"));
    #endif
#elif defined(__WXMSW__)
    wxString defaultMidiDriver(wxT("Windows"));
#else
    wxString defaultMidiDriver(wxT("No Sound"));
#endif
    
    // ---- Midi Driver
    Setting* midiDriver = new Setting(fromCString(SETTING_ID_MIDI_DRIVER), _("MIDI Driver"),
                                SETTING_STRING_ENUM, true /* show in preferences */, defaultMidiDriver );
    std::vector<wxString> midiDrivers = PlatformMidiManager::getChoices();
    const int count = midiDrivers.size();
    for (int n=0; n<count; n++)
    {
        midiDriver->addChoice(midiDrivers[n]);
    }
    m_settings.push_back( midiDriver );

    // ---- play during edit
    Setting* play = new Setting(fromCString(SETTING_ID_PLAY_DURING_EDIT), _("Play during edit (default value)"),
                                SETTING_ENUM, true /* show in preferences */, wxT("1") );
    play->addChoice(_("Always"));        // PLAY_ALWAYS = 0,
    play->addChoice(_("On note change"));// PLAY_ON_CHANGE = 1,
    play->addChoice(_("Never"));         // PLAY_NEVER = 2
    m_settings.push_back( play );
    
    // ---- score view
    Setting* scoreview = new Setting(fromCString(SETTING_ID_SCORE_VIEW), _("Default Score View"),
                                     SETTING_ENUM, true /* show in preferences */, wxT("0") );
    scoreview->addChoice(_("Both Musical and Linear"));
    scoreview->addChoice(_("Musical Only"));
    scoreview->addChoice(_("Linear Only"));
    m_settings.push_back( scoreview );
    
    // ---- follow playback
    Setting* followp = new Setting(fromCString(SETTING_ID_FOLLOW_PLAYBACK), _("Follow playback by default"),
                                   SETTING_BOOL, true /* show in preferences */, wxT("0") );
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
                                     SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back( launchTim );
#endif
    
    
    // TODO: make default value paltform-specific
    Setting* output = new Setting(fromCString(SETTING_ID_MIDI_OUTPUT), wxT(""),
                                      SETTING_STRING, false /* show in preferences */, wxT("default") );
    m_settings.push_back( output );
    
    // ---- printing
    Setting* marginLeft = new Setting(fromCString(SETTING_ID_MARGIN_LEFT), wxT(""),
                                      SETTING_INT, false /* show in preferences */, wxT("12") );
    m_settings.push_back( marginLeft );
    
    Setting* marginRight = new Setting(fromCString(SETTING_ID_MARGIN_RIGHT), wxT(""),
                                      SETTING_INT, false /* show in preferences */, wxT("12") );
    m_settings.push_back( marginRight );
    
    Setting* marginTop = new Setting(fromCString(SETTING_ID_MARGIN_TOP), wxT(""),
                                      SETTING_INT, false /* show in preferences */, wxT("12") );
    m_settings.push_back( marginTop );
    
    Setting* marginBottom = new Setting(fromCString(SETTING_ID_MARGIN_BOTTOM), wxT(""),
                                      SETTING_INT, false /* show in preferences */, wxT("16") );
    m_settings.push_back( marginBottom );
    
    //FIXME: hope wx enum values don't change...
    Setting* paperType = new Setting(fromCString(SETTING_ID_PAPER_TYPE), wxT(""),
                                     SETTING_INT, false /* show in preferences */, to_wxString(wxPAPER_LETTER) );
    m_settings.push_back( paperType );
}

// ----------------------------------------------------------------------------------------------------------

void PreferencesData::save()
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    const int settingAmount = m_settings.size();
    for (int i=0; i<settingAmount; i++)
    {
        prefs->Write( m_settings[i].m_name, m_settings[i].m_value );
    }
    
    prefs->Flush();
}

// ----------------------------------------------------------------------------------------------------------

wxString PreferencesData::getValue(wxString entryName) const
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
    return wxEmptyString;
}

// ----------------------------------------------------------------------------------------------------------

long PreferencesData::getIntValue(wxString entryName) const
{
    wxString asString = getValue(entryName);
    ASSERT(not asString.IsEmpty());
    
    long asInt = -1;
    if (not asString.ToLong(&asInt))
    {
        ASSERT(false);
    }
    return asInt;
}

// ----------------------------------------------------------------------------------------------------------

void PreferencesData::setValue(wxString entryName, wxString newValue)
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
    ASSERT(false);
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Fonts
#endif

// For now fonts are not configurable and do not appear in the config file but eventually this could be done

// FIXME: find why fonts are so different on OSX

wxFont AriaMaestosa::getTabHeaderFont()
{
    return wxFont(100,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD);
}

wxFont AriaMaestosa::getDrumNamesFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getInstrumentNameFont()
{
#ifdef __WXMAC__
    return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(9,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getSequenceFilenameFont()
{
#ifdef __WXMAC__
    return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getTrackNameFont()
{
#ifdef __WXMAC__
    return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getControllerFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getStringNameFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}


wxFont AriaMaestosa::getTimeSigPrintFont()
{
#ifdef __WXMAC__
    return wxFont(150,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#else
    return wxFont(100,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#endif
}

wxFont AriaMaestosa::getPrintFont()
{
#ifdef __WXMAC__
    return wxFont(75, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(50,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getPrintTitleFont()
{
    return wxFont(130, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD  );
}

wxFont AriaMaestosa::getPrintSubtitleFont()
{
#ifdef __WXMAC__
    return wxFont (90,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont (60,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getNumberFont()
{
#if defined(__WXMAC__)
    return wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

