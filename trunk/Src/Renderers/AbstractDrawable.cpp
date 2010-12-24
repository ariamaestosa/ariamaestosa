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

#include "Renderers/AbstractDrawable.h"
#include "Renderers/ImageBase.h"
#include "Utils.h"
#include <iostream>


#include <wx/string.h>

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

AbstractDrawable::AbstractDrawable(Image* image)
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

AbstractDrawable::AbstractDrawable(wxString imagePath)
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

AbstractDrawable::~AbstractDrawable()
{
    if (m_delete_image) delete m_image;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::setFlip(bool xFlip, bool yFlip)
{
    m_x_flip = xFlip;
    m_y_flip = yFlip;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::setHotspot(int hotspotX, int hotspotY)
{
    m_hotspot_x = hotspotX;
    m_hotspot_y = hotspotY;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::move(int x, int y)
{
    m_x = x;
    m_y = y;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::scale(float x, float y)
{
    m_x_scale = x;
    m_y_scale = y;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::scale(float k)
{
    m_x_scale = k;
    m_y_scale = k;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::setImage(Image* image)
{
    m_image = image;
}

// -------------------------------------------------------------------------------------------------------

void AbstractDrawable::rotate(int angle)
{
    m_angle = angle;
}

// -------------------------------------------------------------------------------------------------------
