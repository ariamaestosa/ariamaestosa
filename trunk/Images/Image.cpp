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

/*
 * This is a class built on top of OpenGL to go with Drawable. It deals with OpenGL textures.
 */

#include "Images/Image.h"
#include "Images/ImageLoader.h"

#include <iostream>
#include "Config.h"
#include "OpenGL.h"

namespace AriaMaestosa {
	
Image::Image()
{
	INIT_LEAK_CHECK();
}

Image::Image(wxString path)
{
	INIT_LEAK_CHECK();
    load(path);
}

void Image::load(wxString path)
{
    ID=loadImage(path, &width, &height, &textureWidth, &textureHeight);
    
    tex_coord_x= (float)width/(float)textureWidth;
    tex_coord_y= (float)height/(float)textureHeight;
}

GLuint* Image::getID()
{
    return ID;
}

Image::~Image()
{
    glDeleteTextures (1, ID);
}
}
