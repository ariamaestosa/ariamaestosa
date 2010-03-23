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


#ifndef _preferences_
#define _preferences_

#include "wx/wx.h"
#include "PreferencesData.h"
#include "Utils.h"

namespace AriaMaestosa
{

    class MainFrame;
    class SettingWidget;

    class PreferencesDialog : public wxDialog
    {
        PreferencesData* data;
        int modalCode;

        wxBoxSizer* vert_sizer;

        wxButton* ok_btn;

        MainFrame* parent;
        
        ptr_vector<SettingWidget> m_setting_widgets;
        
    public:
        LEAK_CHECK();

        PreferencesDialog(MainFrame* parent, PreferencesData* data);
        ~PreferencesDialog();
        void show();

        //void languageSelected(wxCommandEvent& evt);
        void okClicked(wxCommandEvent& evt);
        //void playSelected(wxCommandEvent& evt);
        //void followPlaybackChecked(wxCommandEvent& evt);
        //void scoreViewSelected(wxCommandEvent& evt);

        void updateValuesFromWidgets();
        
        /** update view to reflect values */
        void updateWidgetsFromValues();
        
        void saveFromValues();

        long getValue(wxString entryName);

        DECLARE_EVENT_TABLE()
    };

    //bool followPlaybackByDefault();
    //int playDuringEditByDefault();
    //int showLinearViewByDefault();
    //int showMusicalViewByDefault();


}

#endif
