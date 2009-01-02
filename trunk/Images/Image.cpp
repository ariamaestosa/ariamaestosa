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

namespace AriaMaestosa {

Image::Image()
{

}

Image::Image(wxString path)
{
    load(path);
}

void Image::load(wxString path)
{
#ifndef NO_OPENGL
    ID=loadImage(path, &width, &height, &textureWidth, &textureHeight);

    tex_coord_x= (float)width/(float)textureWidth;
    tex_coord_y= (float)height/(float)fabsf(textureHeight);
#else
    for(int n=0; n<AriaRender::STATE_AMOUNT; n++)
    {
        states[n] = NULL;
        states_bmp[n] = NULL;
    }
    
    path = getResourcePrefix() + path;
    if(!image.LoadFile(path))
    {
        wxMessageBox( wxT("Failed to load ") + path );
        exit(1);
    }
    width = image.GetWidth();
    height = image.GetHeight();
    bitmap = new wxBitmap(image);
#endif
}
    
#ifndef NO_OPENGL
GLuint* Image::getID()
{
    return ID;
}
#endif

Image::~Image()
{
#ifndef NO_OPENGL
    glDeleteTextures (1, ID);
#else
    for(int n=0; n<AriaRender::STATE_AMOUNT; n++)
    {
        if(states[n] != NULL)
        {
            delete states[n];
            delete states_bmp[n];
        }
    }
#endif
}

#ifdef NO_OPENGL
wxImage* Image::getImageForState(AriaRender::ImageState s)
{
    if(s == AriaRender::STATE_NORMAL) return &image;
    if(states[s] != NULL) return states[s];
 
    states[s] = new wxImage( image );
 
    float r = 1, g = 1, b = 1;
    switch(s)
    {
        case AriaRender::STATE_NO_FOCUS :
            r = g = b = 0.5;
            break;
        case AriaRender::STATE_DISABLED :
            r = g = b = 0.4;
            break;
        case AriaRender::STATE_UNSELECTED_TAB :
            r = g = b = 0.5;
            break;
        case AriaRender::STATE_SELECTED_NOTE :
            r = 1;
            g = b = 0;
            break;
        case AriaRender::STATE_NOTE :
            r = g = b = 0;
            break;
        default:break;
    }
    
    const unsigned int pixelcount = image.GetHeight() * image.GetWidth();
    unsigned char* data = states[s]->GetData();
    
    for(unsigned int i=0; i<pixelcount; i++)
    {
        //printf("%i %i %i -> ", data[0], data[1], data[2]);
        data[0]= (unsigned char)( (float)(data[0])*r );
        data[1]= (unsigned char)( (float)(data[1])*g );
        data[2]= (unsigned char)( (float)(data[2])*b );
        //printf("%i %i %i\n", data[0], data[1], data[2]);
        data += 3;
    }
    
    states_bmp[s] = new wxBitmap( *states[s] );
    
    return states[s];
}
    
wxBitmap* Image::getBitmapForState(AriaRender::ImageState s)
{
    if(s == AriaRender::STATE_NORMAL) return bitmap;
    
    if(states_bmp[s] == NULL) getImageForState(s);
    return states_bmp[s];
}
#endif
    
}
