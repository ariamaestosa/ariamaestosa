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

#ifdef RENDERER_WXWIDGETS
#include "Utils.h"

#ifndef _wxrender_
#define _wxrender_

#include "wx/wx.h"

namespace AriaMaestosa
{
    
    class MainFrame;
    
    /**
     * @brief   wxWidgets render backend : main render panel
     * @ingroup renderers
     */
    class wxRenderPane : public wxPanel
    {
        
    public:
        LEAK_CHECK();
        
        wxRenderPane(MainFrame* parent, int* args);
        ~wxRenderPane();
        
        void resized(wxSizeEvent& evt);
        
        // size
        int getWidth();
        int getHeight();
        
        bool prepareFrame();
        void beginFrame();
        void endFrame();
        
    };
    
    typedef wxRenderPane RenderPane;
    
}
#endif
#endif