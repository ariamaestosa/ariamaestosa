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
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/fileconf.h>

#include "Midi/Players/PlatformMidiManager.h"


using namespace AriaMaestosa; 

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark Setting
#endif

Setting::Setting(wxString name, wxString user_name, SettingType type,
                 bool visibleInPreferences, wxString default_value,
                 SettingSubType subtype)
{
    m_name                   = name;
    m_user_name              = user_name;
    m_type                   = type;
    m_subtype                = subtype;
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

wxString prefsFile; 
wxString prefsDir;

PreferencesData::PreferencesData()
{
#ifdef __WXGTK__
    prefsDir = wxStandardPaths::Get().GetUserConfigDir() + wxT("/.config/ariamaestosa/");
#else
    prefsDir = wxStandardPaths::Get().GetUserConfigDir() + wxFileName::GetPathSeparator() +
                        wxT("AriaMaestosa") + wxFileName::GetPathSeparator();
#endif

    prefsFile = prefsDir + wxT("preferences.txt");
    printf("<%s>\n", (const char*)prefsFile.utf8_str());
    wxFile f(prefsFile, wxFile::read_write);
    if (not wxFileExists(prefsFile))
    {
        if (not wxMkdir(prefsDir))
        {
            fprintf(stderr, "Cannot create <%s>\n", (const char*)prefsDir.utf8_str());
        }
        f.Create(prefsFile);
    }
    
    wxFileInputStream fis(f);
    wxConfig::Set(new wxFileConfig(fis));
    
    
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
    //I18N: In preferences
    Setting* play = new Setting(fromCString(SETTING_ID_PLAY_DURING_EDIT), _("Play during edit (default value)"),
                                SETTING_ENUM, true /* show in preferences */, wxT("1") );
    play->addChoice(_("Always"));        // PLAY_ALWAYS = 0,
    play->addChoice(_("On note change"));// PLAY_ON_CHANGE = 1,
    play->addChoice(_("Never"));         // PLAY_NEVER = 2
    m_settings.push_back( play );
    
    // ---- score view
    //I18N: In preferences
    Setting* scoreview = new Setting(fromCString(SETTING_ID_SCORE_VIEW), _("Default Score View"),
                                     SETTING_ENUM, true /* show in preferences */, wxT("0") );
    scoreview->addChoice(_("Both Musical and Linear"));
    scoreview->addChoice(_("Musical Only"));
    scoreview->addChoice(_("Linear Only"));
    m_settings.push_back( scoreview );
    
    // ---- default editor
    //I18N: In preferences
    Setting* defaultEditor = new Setting(fromCString(SETTING_ID_DEFAULT_EDITOR), _("Default Editor View"),
                                         SETTING_ENUM, true /* show in preferences */, wxT("0") );
    defaultEditor->addChoice(_("Keyboard"));  // 0
    defaultEditor->addChoice(_("Score"));     // 1
    defaultEditor->addChoice(_("Tablature")); // 2
    m_settings.push_back( defaultEditor );


    // ---- default editor
    //I18N: In preferences
    Setting* instrumentClassification = new Setting(fromCString(SETTING_ID_INSTRUMENT_CLASSIFICATION), _("Instrument Classification"),
                                         SETTING_ENUM, true /* show in preferences */, wxT("0") );
    instrumentClassification->addChoice(_("MIDI Standard"));  // 0
    instrumentClassification->addChoice(wxT("Aria"));     // 1
    instrumentClassification->addChoice(wxT("Buzzwood"));     // 2
    m_settings.push_back( instrumentClassification );



    
#ifdef __APPLE__
    // ---- soundbank
    Setting* soundbank = new Setting(fromCString(SETTING_ID_SOUNDBANK), _("Soundfont"),
                                     SETTING_STRING, true /* show in preferences */, wxT("System soundbank"),
                                     SETTING_SUBTYPE_FILE_OR_DEFAULT);
    m_settings.push_back( soundbank );
#endif
    
    // ---- follow playback
    Setting* followp = new Setting(fromCString(SETTING_ID_FOLLOW_PLAYBACK), _("Follow playback by default"),
                                   SETTING_BOOL, true /* show in preferences */, wxT("0") );
    m_settings.push_back( followp );
    
    // ---- playthrough
    Setting* playthrough = new Setting(fromCString(SETTING_ID_PLAYTHROUGH), _("Enable playthrough when recording by default"),
                                       SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back( playthrough );

    // ---- check for new version
    Setting* newversion = new Setting(fromCString(SETTING_ID_CHECK_NEW_VERSION), _("Check online for new versions"),
                                       SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back( newversion );
    
    // ---- Remember window location
    Setting* windowloc = new Setting(fromCString(SETTING_ID_REMEMBER_WINDOW_POS), _("Remember window location"),
                                     SETTING_BOOL, true /* show in preferences */, wxT("0") );
    m_settings.push_back( windowloc );
    
    Setting* window_x = new Setting(fromCString(SETTING_ID_WINDOW_X), wxT("Window X"),
                                     SETTING_INT, false /* show in preferences */, wxT("0") );
    m_settings.push_back( window_x );
    
    Setting* window_y = new Setting(fromCString(SETTING_ID_WINDOW_Y), wxT("Window Y"),
                                     SETTING_INT, false /* show in preferences */, wxT("0") );
    m_settings.push_back( window_y );
    
    Setting* window_w = new Setting(fromCString(SETTING_ID_WINDOW_W), wxT("Window W"),
                                     SETTING_INT, false /* show in preferences */, wxT("800") );
    m_settings.push_back( window_w );
    
    Setting* window_h = new Setting(fromCString(SETTING_ID_WINDOW_H), wxT("Window H"),
                                     SETTING_INT, false /* show in preferences */, wxT("600") );
    m_settings.push_back( window_h );
    
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
                                     _("Automatically launch TiMidity if needed"),
                                     SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back( launchTim );
#endif

#ifndef __WXMAC__
    Setting* singleInstance = new Setting(fromCString(SETTING_ID_SINGLE_INSTANCE_APPLICATION),
                                     _("Single-instance application"),
                                     SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back( singleInstance );
#endif

    Setting* showNoteNames = new Setting(fromCString(SETTING_ID_SHOW_NOTE_NAMES),
                                     _("Show note names in piano-roll"),
                                     SETTING_BOOL, true /* show in preferences */, wxT("1") );
    m_settings.push_back(showNoteNames);
    
    
    Setting* loadLastSession = new Setting(fromCString(SETTING_ID_LOAD_LAST_SESSION),
                                     _("Load last session files"),
                                     SETTING_BOOL, true /* show in preferences */, wxT("0") );
    m_settings.push_back(loadLastSession); 
    
    
    Setting* lastSessionFiles = new Setting(fromCString(SETTING_ID_LAST_SESSION_FILES),
                                     wxT("Last session files"),
                                     SETTING_STRING, false, wxT("") );
    m_settings.push_back(lastSessionFiles); 
    
    
    Setting* lastCurrentSequence = new Setting(fromCString(SETTING_ID_LAST_CURRENT_SEQUENCE),
                                     wxT("Last Current Sequence"),
                                     SETTING_INT, false, wxT("0"));
    m_settings.push_back(lastCurrentSequence); 
    
    

    // TODO: make default value paltform-specific
    Setting* output = new Setting(fromCString(SETTING_ID_MIDI_OUTPUT), wxT(""),
                                  SETTING_STRING, false /* show in preferences */, wxT("default") );
    m_settings.push_back( output );
    
    Setting* input = new Setting(fromCString(SETTING_ID_MIDI_INPUT), wxT(""),
                                 // NOTE: there is an identical string in MainFrameMenuBar that must be changed too if changed here
                                 SETTING_STRING, false /* show in preferences */, _("No MIDI input") );
    m_settings.push_back( input );
    
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
    

    wxFile f(prefsFile, wxFile::read_write);
    if (not wxFileExists(prefsFile))
    {
        if (not wxMkdir(prefsDir))
        {
            fprintf(stderr, "Cannot create <%s>\n", (const char*)prefsDir.utf8_str());
        }
        f.Create(prefsFile);
        f.Open(prefsFile, wxFile::read_write);
    }
    else
    {
        f.Create(prefsFile, true); /* overwrite previous content */
    }
    
    ASSERT(f.IsOpened());
    
    wxFileOutputStream fos(f);
    wxFileConfig* fc = (wxFileConfig*)prefs;
    fc->Save(fos);
    //prefs->Flush();
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

bool PreferencesData::getBoolValue(const char* entryName, bool defaultVal) const
{
    wxString asString = getValue(entryName);
    if (asString.IsEmpty()) return defaultVal;
    
    if (asString == wxT("true")) return true;
    else if (asString == wxT("false")) return false;
    
    // otherwise try 0/1
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

wxFont AriaMaestosa::getDrumNamesFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMSW__)
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}


wxFont AriaMaestosa::getNoteNamesFont()
{
#ifdef __WXMAC__
    return wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMSW__)
    return wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}



wxFont AriaMaestosa::getInstrumentNameFont()
{
#ifdef __WXMAC__
    return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMSW__)
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
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
#elif defined(__WXMSW__)
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getControllerFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMSW__)
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getStringNameFont()
{
#ifdef __WXMAC__
    return wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMSW__)
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}


wxFont AriaMaestosa::getTimeSigPrintFont()
{
#ifdef __WXMSW__
    // FIXME: See http://trac.wxwidgets.org/ticket/14136
    // fonts are too small on Windows
    return wxFont(17,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#elif defined(__WXMAC__)
    return wxFont(150,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#else
    return wxFont(100,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
#endif
}

wxFont AriaMaestosa::getPrintTabHeaderFont()
{
#ifdef __WXMSW__
    // FIXME: See http://trac.wxwidgets.org/ticket/14136
    // fonts are too small on Windows
    return wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMAC__)
    return wxFont(85, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(65,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getPrintFont()
{
#ifdef __WXMSW__
    // FIXME: See http://trac.wxwidgets.org/ticket/14136
    // fonts are too small on Windows
    return wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMAC__)
    return wxFont(75, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(50,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getPrintTitleFont()
{
#ifdef __WXMSW__
    // FIXME: See http://trac.wxwidgets.org/ticket/14136
    // fonts are too small on Windows
    return wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD  );
#else
    return wxFont(130, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD  );
#endif
}

wxFont AriaMaestosa::getPrintSubtitleFont()
{
#ifdef __WXMSW__
    // FIXME: See http://trac.wxwidgets.org/ticket/14136
    // fonts are too small on Windows
    return wxFont(13, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#elif defined(__WXMAC__)
    return wxFont (90,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont (60,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getNumberFont()
{
#if defined(__WXMAC__)
    return wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
#elif defined(__WXGTK__)
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

wxFont AriaMaestosa::getWelcomeMenuFont()
{
#ifdef __WXMAC__
    return wxFont (18,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#else
    return wxFont (17,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
#endif
}

