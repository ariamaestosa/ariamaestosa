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

#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "Utils.h"
#include <iostream>
#include <list>
#include <map>

#include "AriaCore.h"

#include <wx/dc.h>
#include <wx/image.h>
#include <wx/string.h>

namespace AriaMaestosa
{

    AriaRender::ImageState g_state;
        

    void drawable_set_state(AriaRender::ImageState arg)
    {
        g_state = arg;
    }

    class ImageCache
    {
    public:
        AriaRender::ImageState m_state;
        bool m_x_flip, m_y_flip;
        int m_angle;
        wxBitmap m_bitmap;
        
        ImageCache(AriaRender::ImageState state, bool x_flip, bool y_flip, int angle, wxBitmap bmp) :
            m_bitmap(bmp)
        {
            m_state = state;
            m_x_flip = x_flip;
            m_y_flip = y_flip;
            m_angle = angle;
        }
        
        bool operator==(const ImageCache& other) const
        {
            return m_state == other.m_state and m_x_flip == other.m_x_flip and m_y_flip == other.m_y_flip and
                   m_angle == other.m_angle;
        }
    };
    
    std::map<Image*, std::list<ImageCache> > g_cache;
    
}

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

void Drawable::render()
{
    if (m_x_flip or m_y_flip or m_x_scale != 1 or m_y_scale != 1 or m_angle != 0)
    {
        int hotspotX_mod = m_hotspot_x;
        int hotspotY_mod = m_hotspot_y;

        if (m_angle == 90)
        {
            if (not m_x_flip) hotspotX_mod = m_hotspot_x + m_image->width;
            else              hotspotX_mod = m_hotspot_x - m_image->width;
        }
        
        // Look in the cache
        std::list<ImageCache>& cache = g_cache[m_image];
        std::list<ImageCache>::iterator it;
        for (it = cache.begin(); it != cache.end(); it++)
        {
            if (it->m_state == g_state and it->m_x_flip == m_x_flip and it->m_y_flip == m_y_flip and
                it->m_angle == m_angle)
            {
                if (m_x_scale != 1)
                {
                    const int new_w = (int)(m_image->width  * m_x_scale);
                    if (new_w == 0) return;
                    
                    //if (new_w < m_image->width) return;
                    
                    const int max_x = m_x - hotspotX_mod + new_w - m_image->width;
                    for (int x = m_x - hotspotX_mod; x < max_x; x += m_image->width)
                    {
                        Display::renderDC->DrawBitmap( it->m_bitmap, x, m_y - hotspotY_mod);
                    }
                    
                    Display::renderDC->DrawBitmap( it->m_bitmap, max_x, m_y - hotspotY_mod);
                }
                else if (m_y_scale != 1)
                {
                    const int new_h = (int)(m_image->height * m_y_scale);
                    if (new_h == 0) return;
                    
                    //if (new_h < m_image->height) return;
                    
                    const int max_y = m_y - hotspotY_mod + new_h - m_image->height;
                    for (int y = m_y - hotspotY_mod; y < max_y; y += m_image->height)
                    {
                        Display::renderDC->DrawBitmap( it->m_bitmap, m_x - hotspotX_mod, y);
                    }
                    Display::renderDC->DrawBitmap( it->m_bitmap, m_x - hotspotX_mod, max_y);
                }
                else
                {
                    Display::renderDC->DrawBitmap(it->m_bitmap, m_x - hotspotX_mod, m_y - hotspotY_mod);
                }
                
                return;
            }
        }

        // Image not found in cache
        wxImage modimage = m_image->getBitmapForState(g_state)->ConvertToImage();

        if (m_x_flip) modimage = modimage.Mirror();
        if (m_y_flip) modimage = modimage.Mirror(false);

        if (m_angle == 90)
        {
            modimage = modimage.Rotate90();
        }

        wxBitmap modbitmap(modimage);
        cache.push_front( ImageCache(g_state, m_x_flip, m_y_flip, m_angle, modbitmap) );

        if (m_x_scale != 1)
        {
            const int new_w = (int)(m_image->width  * m_x_scale);
            if (new_w == 0) return;
            
            // if (new_w < m_image->width) return;
            
            const int max_x = m_x - hotspotX_mod + new_w - m_image->width;
            for (int x = m_x - hotspotX_mod; x < max_x; x += m_image->width)
            {
                Display::renderDC->DrawBitmap( modbitmap, x, m_y - hotspotY_mod);
            }
            Display::renderDC->DrawBitmap( modbitmap, max_x, m_y - hotspotY_mod);
        }
        else if (m_y_scale != 1)
        {
            const int new_h = (int)(m_image->height * m_y_scale);
            if (new_h == 0) return;
            
            // if (new_h < m_image->height) return;
            
            const int max_y = m_y - hotspotY_mod + new_h - m_image->height;
            for (int y = m_y - hotspotY_mod; y < max_y; y += m_image->height)
            {
                Display::renderDC->DrawBitmap( modbitmap, m_x - hotspotX_mod, y);
            }
            Display::renderDC->DrawBitmap( modbitmap, m_x - hotspotX_mod, max_y);
        }
        else
        {
            Display::renderDC -> DrawBitmap( modbitmap, m_x - hotspotX_mod, m_y - hotspotY_mod);
        }
    }
    else
    {
        Display::renderDC -> DrawBitmap( *m_image->getBitmapForState(g_state),
                                         m_x - m_hotspot_x, m_y - m_hotspot_y);
    }

}

// -------------------------------------------------------------------------------------------------------

#endif
