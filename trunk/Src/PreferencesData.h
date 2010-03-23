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

#include "wx/wx.h"

namespace AriaMaestosa
{
    
    enum SettingType
    {
        SETTING_ENUM,
        SETTING_BOOL,
        SETTING_INT
    };
    
#ifdef DEFINE_SETTING_NAMES
#define EXTERN
#define DEFAULT(X) = X
#else
#define EXTERN extern
#define DEFAULT(X)
#endif
    
    EXTERN const char* SETTING_ID_FOLLOW_PLAYBACK  DEFAULT("followPlayback");
    EXTERN const char* SETTING_ID_SCORE_VIEW       DEFAULT("scoreview");
    EXTERN const char* SETTING_ID_PLAY_DURING_EDIT DEFAULT("playDuringEdit");
    EXTERN const char* SETTING_ID_LANGUAGE         DEFAULT("lang");
    EXTERN const char* SETTING_ID_LAUNCH_TIMIDITY  DEFAULT("launchTimidity");
    
#undef EXTERN
#undef DEFAULT
    
    class Setting
    {
    public:
        
        wxString      m_name;
        wxString      m_user_name;
        wxArrayString m_choices;
        SettingType   m_type;
        int           m_value;
        bool          m_visible_in_preferences;
        
        Setting(wxString name, wxString user_name, SettingType type, bool visibleInPreferences,
                int default_value = 0);
        void addChoice(wxString choice);
        void setChoices(wxArrayString choices);
    };
    
    class PreferencesData : public Singleton<PreferencesData>
    {
        friend class Singleton<PreferencesData>;
        
        ptr_vector<Setting> m_settings;
        
        /** Add and init preferences values */
        void fillSettingsVector();
        
        /** Private constructor */
        PreferencesData();

    public:
                
        long getValue(wxString entryName);
        
        /** write config file */
        void save();

        ptr_vector<Setting>& getSettings() { return m_settings; }
    };
    
}

#endif
