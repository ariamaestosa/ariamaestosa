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

#include "wx/string.h"

namespace AriaMaestosa
{
    AriaRender::ImageState g_state;
}

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

Drawable::Drawable(Image* image)
{
    m_x = 0;
    m_y = 0;
    m_hotspot_x = 0;
    m_hotspot_y = 0;
    m_angle = 0;
    
    m_x_scale = 1;
    m_y_scale = 1;
    
    m_x_flip = false;
    m_y_flip = false;
    
    m_delete_image = false;
    
    if (image != NULL) setImage(image);
    else               m_image = NULL;
}

// -------------------------------------------------------------------------------------------------------

Drawable::Drawable(wxString imagePath)
{
    m_x = 0;
    m_y = 0;
    m_hotspot_x = 0;
    m_hotspot_y = 0;
    m_angle = 0;
    
    m_x_scale = 1;
    m_y_scale = 1;
    
    m_x_flip = false;
    m_y_flip = false;
    
    m_delete_image = true;
    
    m_image = new Image(imagePath);
}

// -------------------------------------------------------------------------------------------------------

Drawable::~Drawable()
{
    if (m_delete_image) delete m_image;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::setFlip(bool xFlip, bool yFlip)
{
    m_x_flip = xFlip;
    m_y_flip = yFlip;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::setHotspot(int hotspotX, int hotspotY)
{
    m_hotspot_x = hotspotX;
    m_hotspot_y = hotspotY;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::move(int x, int y)
{
    m_x = x;
    m_y = y;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::scale(float x, float y)
{
    m_x_scale = x;
    m_y_scale = y;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::scale(float k)
{
    m_x_scale = k;
    m_y_scale = k;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::setImage(Image* image)
{
    m_image = image;
}

// -------------------------------------------------------------------------------------------------------

void Drawable::rotate(int angle)
{
    m_angle = angle;
}

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