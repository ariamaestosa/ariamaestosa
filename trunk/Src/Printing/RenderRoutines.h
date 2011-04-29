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


#ifndef __RENDER_ROUTINES_H__
#define __RENDER_ROUTINES_H__

#include "Range.h"

#include <wx/gdicmn.h>

class wxDC;
class wxImage;
class wxGraphicsContext;

namespace AriaMaestosa
{
    /** Size of a note's head */
    const int HEAD_RADIUS = 36;
    
    /** Width of 1/1 and 1/2 silences */
    const int RECTANGULAR_SILENCE_SIZE = 80;
    
    /** How much margin space to leave at the left of 1/1 and 1/2 silences*/
    const int RECTANGULAR_SILENCE_LEFT_MARGIN = 40;
    
    /** size of the dot, for dotted notes */
    const int DOT_SIZE = 14;
    
    /**
      * @brief group of rendering helper functions available globally to everything printing-related
      * @ingroup printing
      */
    namespace RenderRoutines
    {
        
        /**
          * @brief utility function to render a note's head
          */
        void drawNoteHead(wxDC& dc, const wxPoint headCenter, const bool hollowHead);
        
        /** @brief Utility function : renders an arc (half an ellipse) at the given coordinates */
        void renderArc(wxDC& dc, const int center_x, const int center_y,
                       const int radius_x, const int radius_y);
        
        /** @brief Utility function : renders an arc (half an ellipse) at the given coordinates */
        void renderArc(wxGraphicsContext& gc, const int center_x, const int center_y,
                       const int radius_x, const int radius_y);
        
        void drawFlag(wxDC* dc, wxGraphicsContext* gc, const int flag_x_origin, const int flag_y, const int orient);
        
        /**
         * @brief utility function to render a silence at a given location
         */
        void drawSilence(wxDC* dc, const Range<int> x, const int y, const int level_height,
                         const int type, const bool triplet, const bool dotted);
        
        /**
         * @brief utility function to render a silence at a given location
         */
        void drawSilence(wxGraphicsContext& dc, const Range<int> x, const int y, const int level_height,
                         const int type, const bool triplet, const bool dotted);
        
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
        void paintTreble(wxGraphicsContext& painter, int x, int B_y, int E_y);
        void paintBass(wxGraphicsContext& painter, int x, int score_top, int E_y);
#endif
        
        /**
         * @brief  loads an image from file and scales it
         * @return the scaled image
         */
        wxBitmap getScaledBitmap(const wxString& fileName, float scale);
        
        /**
         * @brief  make an image more print-friendly on Windows
         */
        wxImage getPrintableImage(const wxImage& image);
        
    }
    
}

#endif
