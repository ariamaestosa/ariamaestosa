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

#ifdef RENDERER_OPENGL

#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "Utils.h"
#include <iostream>

#include "OpenGL.h"

#include <wx/string.h>

using namespace AriaMaestosa;


// -------------------------------------------------------------------------------------------------------

void Drawable::render()
{
    ASSERT(m_image != NULL);
    
    glLoadIdentity();
    
    glTranslatef(m_x*10.0, m_y*10.0, 0);
    
    if (m_x_scale != 1 or m_y_scale != 1)
    {
        glScalef(m_x_scale, m_y_scale, 1);
    }
    
    if (m_angle != 0)
    {
        glRotatef(m_angle, 0,0,1);
    }
    
    bool do_yflip = m_y_flip;
    // hack, textureHeight made smaller than zero when image was power of two.
    // in these cases, the image will be upside down so we need to flip it
    if (m_image->textureHeight < 0) do_yflip = not m_y_flip;
    
    glBindTexture(GL_TEXTURE_2D, m_image->getID()[0] );
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(m_x_flip? m_image->tex_coord_x : 0, do_yflip? 0 : m_image->tex_coord_y);
    glVertex2f( -m_hotspot_x*10.0, -m_hotspot_y*10.0 );
    
    glTexCoord2f(m_x_flip? 0 : m_image->tex_coord_x, do_yflip? 0 : m_image->tex_coord_y);
    glVertex2f( (m_image->width-m_hotspot_x)*10.0, -m_hotspot_y*10.0 );
    
    glTexCoord2f(m_x_flip? 0 : m_image->tex_coord_x, do_yflip? m_image->tex_coord_y : 0);
    glVertex2f( (m_image->width-m_hotspot_x)*10.0, (m_image->height-m_hotspot_y)*10.0 );
    
    glTexCoord2f(m_x_flip? m_image->tex_coord_x : 0, do_yflip? m_image->tex_coord_y : 0);
    glVertex2f( -m_hotspot_x*10.0, (m_image->height-m_hotspot_y)*10.0 );
    
    glEnd();
}

// -------------------------------------------------------------------------------------------------------

#endif