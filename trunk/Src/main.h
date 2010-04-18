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

#ifndef __MAIN_H__
#define __MAIN_H__

#if defined(__WXMSW__) && !defined(NDEBUG)
#define USE_WX_LOGGING 1
#else
#define USE_WX_LOGGING 0
#endif

#include "wx/wx.h"
#include "Utils.h"

namespace AriaMaestosa
{
    
    class MainFrame;
    class MeasureBar;
    class PreferencesData;
    
    class wxWidgetApp : public wxApp
    {
    public:
        MainFrame* frame;
        PreferencesData*  prefs;
        
        /** implement callback from wxApp */
        bool OnInit();
        
        /** implement callback from wxApp */
        virtual int  OnExit();
        
        /** implement callback from wxApp */
        void MacOpenFile(const wxString &fileName);
        
        /** call to activate or deactivate the render loop (which is based on idle events) */
        void activateRenderLoop(bool on);
        
        /** callback : called on idle */
        void onIdle(wxIdleEvent& evt);
        
        /** callback : called when app is activated */
        void onActivate(wxActivateEvent& evt);
        
        DECLARE_EVENT_TABLE();
        
#if USE_WX_LOGGING
        int CloseLogWindow();
#endif
        
    private:
        
#if USE_WX_LOGGING        
        wxLogWindow* m_log_window;
        wxFrame*     m_log_frame;
#endif
    };
    
}


DECLARE_APP(AriaMaestosa::wxWidgetApp)
#endif

