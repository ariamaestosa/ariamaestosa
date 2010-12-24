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

}

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

void Drawable::render()
{
    if (m_x_flip or m_y_flip or m_x_scale != 1 or m_y_scale != 1 or m_angle != 0)
    {
        int hotspotX_mod = m_hotspot_x;
        int hotspotY_mod = m_hotspot_y;

        wxImage modimage = m_image->getBitmapForState(g_state)->ConvertToImage();

        if (m_x_flip) modimage = modimage.Mirror();
        if (m_y_flip) modimage = modimage.Mirror(false);

        if (m_angle == 90)
        {
            modimage = modimage.Rotate90();
            if (not m_x_flip) hotspotX_mod = m_hotspot_x + m_image->width;
            else              hotspotX_mod = m_hotspot_x - m_image->width;
        }

        if (m_x_scale != 1 or m_y_scale != 1)
        {
            const int new_w = (int)(m_image->width  * m_x_scale);
            const int new_h = (int)(m_image->height * m_y_scale);
            if (new_w == 0 or new_h == 0) return;
            modimage.Rescale( new_w, new_h );
        }

        wxBitmap modbitmap(modimage);
        Display::renderDC -> DrawBitmap( modbitmap, m_x - hotspotX_mod, m_y - hotspotY_mod);
    }
    else
    {
        Display::renderDC -> DrawBitmap( *m_image->getBitmapForState(g_state),
                                         m_x - m_hotspot_x, m_y - m_hotspot_y);
    }

}

// -------------------------------------------------------------------------------------------------------

#endif
