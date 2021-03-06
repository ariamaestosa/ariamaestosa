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
/*
 * This is a class built on top of OpenGL to go with Drawable. It deals with OpenGL textures.
 */

#include "Renderers/ImageBase.h"
#include <iostream>
#include "Utils.h"

#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>

using namespace AriaMaestosa;

Image::Image()
{

}

Image::Image(wxString path)
{
    load(path);
}

void Image::load(wxString path)
{
    for (int n=0; n<AriaRender::STATE_AMOUNT; n++)
    {
        states[n] = NULL;
        states_bmp[n] = NULL;
    }

    path = getResourcePrefix() + path;
    if (!image.LoadFile(path))
    {
        wxMessageBox( _("Failed to load ") + path );
        fprintf(stderr, "Failed to load %s\n", (const char*)path.mb_str());
        exit(1);
    }
    width = image.GetWidth();
    height = image.GetHeight();
    bitmap = new wxBitmap(image);
}

Image::~Image()
{
    for (int n=0; n<AriaRender::STATE_AMOUNT; n++)
    {
        if (states[n] != NULL)
        {
            delete states[n];
            delete states_bmp[n];
        }
    }
}

wxImage* Image::getImageForState(AriaRender::ImageState s)
{
    if (s == AriaRender::STATE_NORMAL) return &image;
    if (states[s] != NULL) return states[s];

    states[s] = new wxImage( image );

    static const int MODE_MULTIPLY = 0;
    static const int MODE_FADE_TO = 1;

    int mode = MODE_MULTIPLY;

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
            r = g = 1;
            b = 0.9;
            mode = MODE_FADE_TO;
            break;
        case AriaRender::STATE_SELECTED_NOTE :
            r = 1;
            g = b = 0;
            break;
        case AriaRender::STATE_NOTE :
            r = g = b = 0;
            break;
        case AriaRender::STATE_GHOST:
            r = 1.0f;
            g = 1.0f;
            b = 1.0f;
            mode = MODE_FADE_TO;
            break;
        default:break;
    }

    const unsigned int pixelcount = image.GetHeight() * image.GetWidth();
    unsigned char* data = states[s]->GetData();

    if (mode == MODE_MULTIPLY)
    {
        for (unsigned int i=0; i<pixelcount; i++)
        {
            data[0]= (unsigned char)( (float)(data[0])*r );
            data[1]= (unsigned char)( (float)(data[1])*g );
            data[2]= (unsigned char)( (float)(data[2])*b );
            data += 3;
        }
    }
    else if (mode == MODE_FADE_TO)
    {
        for (unsigned int i=0; i<pixelcount; i++)
        {
            float pr = data[0]/255.0f;
            float pg = data[1]/255.0f;
            float pb = data[2]/255.0f;

            pr = (pr + r) / 2.0;
            pg = (pg + g) / 2.0;
            pb = (pb + b) / 2.0;

            data[0]= (unsigned char)( pr*255 );
            data[1]= (unsigned char)( pg*255 );
            data[2]= (unsigned char)( pb*255 );
            data += 3;
        }
    }

    states_bmp[s] = new wxBitmap( *states[s] );

    return states[s];
}

wxBitmap* Image::getBitmapForState(AriaRender::ImageState s)
{
    if (s == AriaRender::STATE_NORMAL) return bitmap;

    if (states_bmp[s] == NULL) getImageForState(s);
    return states_bmp[s];
}


#endif
