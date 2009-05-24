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

#ifndef _mainh_
#define _mainh_

#include "wx/wx.h"
#include "Config.h"

namespace AriaMaestosa {

    class MainFrame;
    class MeasureBar;
    class PreferencesData;

class wxWidgetApp : public wxApp {
public:
    MainFrame* frame;
    OwnerPtr<PreferencesData>  prefs;

    bool OnInit();
    int OnExit();
    void MacOpenFile(const wxString &fileName);

    void activateRenderLoop(bool on);
    void onIdle(wxIdleEvent& evt);

    void onActivate(wxActivateEvent& evt);

    DECLARE_EVENT_TABLE();
};

}


DECLARE_APP(AriaMaestosa::wxWidgetApp)
#endif

