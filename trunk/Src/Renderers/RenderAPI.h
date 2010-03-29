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

#ifndef __RENDER_API_H__
#define __RENDER_API_H__

#ifdef RENDERER_OPENGL
#include "Renderers/GLwxString.h"
#elif defined(RENDERER_WXWIDGETS)
#include "Renderers/wxDCString.h"
#else
#error No renderer defined!
#endif

class wxString;

namespace AriaMaestosa
{
    namespace AriaRender
    {
        
        enum ImageState
        {
            STATE_NORMAL = 99,
            STATE_NO_FOCUS = 0, // to allow setting up an array of modified images, where the the state ID is also the array index
            STATE_DISABLED,
            STATE_GHOST,
            STATE_NOTE,
            STATE_SELECTED_NOTE,
            STATE_UNSELECTED_TAB,
            
            STATE_AMOUNT
        };
        
        // enter mode
        void primitives();
        void images();
        
        void beginScissors(const int x, const int y, const int width, const int height);
        void endScissors();
        
        void color(const float r, const float g, const float b);
        void color(const float r, const float g, const float b, const float a);
        void setImageState(const ImageState imgst);
        
        void line(const int x1, const int y1, const int x2, const int y2);
        void lineWidth(const int n);
        void lineSmooth(const bool enabled);
        
        void point(const int x, const int y);
        void pointSize(const int n);
        
        void rect(const int x1, const int y1, const int x2, const int y2);
        void hollow_rect(const int x1, const int y1, const int x2, const int y2);
        void bordered_rect(const int x1, const int y1, const int x2, const int y2);
        void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2);
        
        void renderNumber(const int number, const int x, const int y);
        void renderNumber(const float number, const int x, const int y);
        void renderNumber(const wxString number, const int x, const int y);
        
        void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3);
        
        void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above);
        
        void quad(const int x1, const int y1,
                  const int x2, const int y2,
                  const int x3, const int y3,
                  const int x4, const int y4);
    }
}
#endif
