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

#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__

#include "Utils.h"
#include "Renderers/ImageBase.h"
#include "Renderers/RenderAPI.h"

namespace AriaMaestosa
{
    
    class Image;
    
#ifdef RENDERER_WXWIDGETS
    // FIXME - remove this ifdef
    void drawable_set_state(AriaRender::ImageState arg);
#endif
    
    class Drawable
    {
        int m_x, m_y;
        int m_angle;
        int m_hotspot_x, m_hotspot_y;
        float m_x_scale, m_y_scale;
        Image* m_image;
        bool m_x_flip, m_y_flip;
        bool m_delete_image;
        
    public:
        LEAK_CHECK();

        
        Drawable(Image* image=(Image*)0);
        Drawable(wxString imagePath);
        ~Drawable();
        
        void setFlip(bool x, bool y);
        
        int getImageWidth()  const { return m_image->width;  }
        int getImageHeight() const { return m_image->height; }
        
        int getX() const { return m_x; }
        int getY() const { return m_y; }
        
        int getHotspotX() const { return m_hotspot_x; }
        int getHotspotY() const { return m_hotspot_y; }

        void move(int x, int y);
        void setHotspot(int x, int y);
        void scale(float x, float y);
        void scale(float k);
        void setImage(Image* image);
        void render();
        void rotate(int angle);
        
    };
    
}


#endif
