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

#ifndef __PREFERENCES_DATA_H__
#define __PREFERENCES_DATA_H__

#include "Singleton.h"
#include "ptr_vector.h"

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/font.h>
#include <wx/intl.h>

namespace AriaMaestosa
{
    
    enum SettingType
    {
        SETTING_ENUM,
        SETTING_STRING,
        SETTING_STRING_ENUM,
        SETTING_BOOL,
        SETTING_INT
    };
    
    enum SettingSubType
    {
        SETTING_SUBTYPE_NONE,
        /** Only for 'SETTING_STRING */
        SETTING_SUBTYPE_FILE_OR_DEFAULT
    };
    
#ifdef DEFINE_SETTING_NAMES
#define EXTERN
#define DEFAULT(X) = X
#else
#define EXTERN extern
#define DEFAULT(X)
#endif


#ifndef __WXMSW__ 
    static const wxString SYSTEM_BANK = _("System soundbank");
#endif
    
#ifdef __WXGTK__
    static const wxString DEFAULT_SOUNDFONT_PATH = wxT("/usr/share/sounds/sf2/FluidR3_GM.sf2");
#endif

    // TODO: make default value platform-specific
    static const wxString DEFAULT_PORT = wxT("default");
    
    EXTERN const char* SETTING_ID_FOLLOW_PLAYBACK  DEFAULT("followPlayback");
    EXTERN const char* SETTING_ID_SCORE_VIEW       DEFAULT("scoreview");
    EXTERN const char* SETTING_ID_PLAY_DURING_EDIT DEFAULT("playDuringEdit");
    EXTERN const char* SETTING_ID_LANGUAGE         DEFAULT("lang");
    EXTERN const char* SETTING_ID_LAUNCH_FLUIDSYNTH  DEFAULT("launchFluidSynth");
    
#ifndef __WXMAC__
    EXTERN const char* SETTING_ID_SINGLE_INSTANCE_APPLICATION  DEFAULT("singleInstanceApplication");
#endif
    
    EXTERN const char* SETTING_ID_PLAYTHROUGH      DEFAULT("playthrough");

    EXTERN const char* SETTING_ID_MARGIN_LEFT      DEFAULT("marginLeft");
    EXTERN const char* SETTING_ID_MARGIN_RIGHT     DEFAULT("marginRight");
    EXTERN const char* SETTING_ID_MARGIN_TOP       DEFAULT("marginTop");
    EXTERN const char* SETTING_ID_MARGIN_BOTTOM    DEFAULT("marginBottom");

    EXTERN const char* SETTING_ID_PAPER_TYPE       DEFAULT("paperType");
    EXTERN const char* SETTING_ID_MIDI_DRIVER      DEFAULT("midiDriver");
    EXTERN const char* SETTING_ID_MIDI_OUTPUT      DEFAULT("midiOutput");
    EXTERN const char* SETTING_ID_MIDI_INPUT       DEFAULT("midiInput");

    EXTERN const char* SETTING_ID_DEFAULT_EDITOR   DEFAULT("defaultEditor");

    EXTERN const char* SETTING_ID_INSTRUMENT_CLASSIFICATION   DEFAULT("instrumentClassification");
    
    EXTERN const char* SETTING_ID_SHOW_NOTE_NAMES   DEFAULT("showNoteNames");
    
    EXTERN const char* SETTING_ID_LOAD_LAST_SESSION      DEFAULT("loadLastSessionFiles");
    EXTERN const char* SETTING_ID_LAST_SESSION_FILES     DEFAULT("lastSessionFiles");
    EXTERN const char* SETTING_ID_LAST_CURRENT_SEQUENCE  DEFAULT("0");
    
    EXTERN const char* SETTING_ID_RECENT_FILES     DEFAULT("recentFiles");
    
    EXTERN const char* SETTING_ID_CHECK_NEW_VERSION DEFAULT("checkForNewVersion");
    
    EXTERN const char* SETTING_ID_REMEMBER_WINDOW_POS DEFAULT("rememberWindowLocation");
    EXTERN const char* SETTING_ID_WINDOW_X DEFAULT("window_x");
    EXTERN const char* SETTING_ID_WINDOW_Y DEFAULT("window_y");
    EXTERN const char* SETTING_ID_WINDOW_W DEFAULT("window_w");
    EXTERN const char* SETTING_ID_WINDOW_H DEFAULT("window_h");
    
#ifndef __WXMSW__
    EXTERN const char* SETTING_ID_AUDIO_EXPORT_ENGINE DEFAULT("audioExportEngine");
    EXTERN const char* SETTING_ID_FLUIDSYNTH_SOUNDFONT_PATH DEFAULT("fluidsynthSoundfontPath");
    EXTERN const char* SETTING_ID_SOUNDBANK        DEFAULT("soundbank");
#endif
    
#undef EXTERN
#undef DEFAULT
    
    class Setting
    {
    public:
        
        wxString       m_name;
        wxString       m_user_name;
        wxArrayString  m_choices;
        SettingType    m_type;
        SettingSubType m_subtype;
        wxString       m_value;
        bool           m_visible_in_preferences;
        
        Setting(wxString name, wxString user_name, SettingType type, bool visibleInPreferences,
                wxString default_value = wxEmptyString, SettingSubType subtype = SETTING_SUBTYPE_NONE);
        void addChoice(wxString choice);
        void setChoices(wxArrayString choices);
    };
    
    class PreferencesData : public Singleton<PreferencesData>
    {
        friend class Singleton<PreferencesData>;
        
        ptr_vector<Setting> m_settings;
        
        /** Add and init preferences values */
        void fillSettingsVector();
        
        void prepareLanguageEntry();
        
        /** Private constructor */
        PreferencesData();
        
        /** Fills the setting objects with the user's saved values, if any, replacing the defaults */
        void readValues();

        bool m_inited;
        
    public:
                
        /** call early */
        void init();
        
        long getIntValue(const char* entryName) const { return getIntValue(wxString(entryName, wxConvUTF8)); }
        bool getBoolValue(const char* entryName, bool defaultValue) const;
        wxString getValue(const char* entryName) const { return getValue(wxString(entryName, wxConvUTF8)); }
        wxString getValue(wxString entryName) const;
        long getIntValue(wxString entryName) const;

        void setValue(const char* entryName, wxString newValue)
        {
            setValue( wxString(entryName, wxConvUTF8), newValue );
        }
        void setValue(wxString entryName, wxString newValue);
        
        /** write config file */
        void save();

        ptr_vector<Setting>& getSettings() { return m_settings; }
    };
    
    wxFont getDrumNamesFont();
    wxFont getNoteNamesFont();
    wxFont getInstrumentNameFont();
    wxFont getWelcomeMenuFont();
    wxFont getSequenceFilenameFont();
    wxFont getTrackNameFont();
    wxFont getControllerFont();
    wxFont getTimeSigPrintFont();
    wxFont getPrintFont();
    wxFont getPrintTitleFont();
    wxFont getPrintSubtitleFont();
    wxFont getNumberFont();
    wxFont getStringNameFont();
    wxFont getPrintTabHeaderFont();
    
}

#endif
