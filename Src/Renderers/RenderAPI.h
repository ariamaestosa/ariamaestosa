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

/**
  * @defgroup renderers
  */
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
    /**
      * @brief   holds the rendering functions of Aria
      * @ingroup renderers
      */
    namespace AriaRender
    {
        /**
          * @ingroup renderers
          */
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
        
        /**
          * @brief to enter "primitive" drawing mode (lines, points, rectangles, etc...)
          */
        void primitives();
        
        /**
          * @brief to enter "image" drawing mode (before rendering a Drawable or text)
          */
        void images();
        
        /**
          * @brief start clipping (nothing will be drawn outside the given rectangle)
          */
        void beginScissors(const int x, const int y, const int width, const int height);
        
        /**
          * @brief terminate the clipping rect set up with a previous call to RenderAPI::beginScissors
          */
        void endScissors();
        
        /**
          * @brief set the drawing color for primitives and text
          */
        void color(const float r, const float g, const float b);
    
        /**
         * @brief set the drawing color for primitives and text
         */
        void color(const float r, const float g, const float b, const float a);
        
        /**
          * @brief set the rendering color/style for images
          */
        void setImageState(const ImageState imgst);
        
        /**
          * @brief render a line between two points
          */
        void line(const int x1, const int y1, const int x2, const int y2);
        
        /**
          * @brief set the width of the line drawn by subsequent calls to RenderAPI::line
          */
        void lineWidth(const int n);
        
        /**
          * @brief enables or disables anti-aliasing when rendering lines
          */
        void lineSmooth(const bool enabled);
        
        /**
          * @brief renders a point at the given coordinate
          */
        void point(const int x, const int y);
        
        /**
          * @brief sets the size of the points rendered by subsequent calls to RenderAPI::point
          */
        void pointSize(const int n);
        
        /**
          * @brief renders a filled rectangle within the given coordinates
          */
        void rect(const int x1, const int y1, const int x2, const int y2);
        
        /**
          * @brief renders a rectangle outline within the given coordinates
          */
        void hollow_rect(const int x1, const int y1, const int x2, const int y2);
        
        void select_rect(const int x1, const int y1, const int x2, const int y2);

        /**
          * @brief renders a rounded rectangle with a black outline within the given coordinates
          */
        void bordered_rect(const int x1, const int y1, const int x2, const int y2);
        
        /**
          * @brief renders a rounded rectangle with a black outline within the given coordinates
          * This call truncates the rectangle on the left (no outline nor rounded edges)
          */
        void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2);
        
        /**
         * @brief renders a number at the given coordinate
         */
        void renderNumber(const int number, const int x, const int y);
        
        /**
         * @brief renders a floating-point number at the given coordinate
         */
        void renderNumber(const float number, const int x, const int y);
        
        /**
         * @brief renders a stringized number at the given coordinate
         */
        void renderNumber(const char* number, const int x, const int y);
        
        /**
         * @brief renders a string at the given coordinates {x,y} 
         * and truncates the string to maxWidth if necessary
         */
        void renderString(const wxString& string, const int x, const int y, const int maxWidth);
        
        /**
         * @brief renders a triangle within the specified 3 points
         */
        void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3);
        
        /**
         * @brief renders an arc around the specified center and radius
         */
        void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above);
        
        /**
         * @brief renders an arbitrary quad within the given coordinates
         */
        void quad(const int x1, const int y1,
                  const int x2, const int y2,
                  const int x3, const int y3,
                  const int x4, const int y4);
    }
}
#endif
