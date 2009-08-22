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
#include "Config.h"
#include <iostream>

#include "AriaCore.h"

#include "wx/wx.h"


namespace AriaMaestosa {

AriaRender::ImageState g_state;
    

void drawable_set_state(AriaRender::ImageState arg)
{
    g_state = arg;
}


Drawable::Drawable(Image* image_arg)
{
    x=0;
    y=0;
    hotspotX=0;
    hotspotY=0;
    angle=0;

    xscale=1;
    yscale=1;

    xflip=false;
    yflip=false;

    delete_image = false;

    if (image_arg!=NULL) setImage(image_arg);
    else image=NULL;
}

Drawable::Drawable(wxString imagePath)
{
    x=0;
    y=0;
    hotspotX=0;
    hotspotY=0;
    angle=0;

    xscale=1;
    yscale=1;

    xflip=false;
    yflip=false;

    delete_image = true;

    image = new Image(imagePath);
}

Drawable::~Drawable()
{
    if (delete_image) delete image;
}

void Drawable::setFlip(bool x, bool y)
{
    xflip=x;
    yflip=y;
}

void Drawable::setHotspot(int x, int y)
{
    hotspotX=x;
    hotspotY=y;
}

void Drawable::move(int x, int y)
{
    Drawable::x=x;
    Drawable::y=y;
}

void Drawable::scale(float x, float y)
{
    Drawable::xscale=x;
    Drawable::yscale=y;
}

void Drawable::scale(float k)
{
    Drawable::xscale=k;
    Drawable::yscale=k;
}

void Drawable::setImage(Image* image)
{
    Drawable::image=image;
}

void Drawable::rotate(int angle)
{
    Drawable::angle=angle;
}

void Drawable::render()
{
    if (xflip or yflip or xscale != 1 or yscale != 1 or angle!=0)
    {
        int hotspotX_mod = hotspotX;
        int hotspotY_mod = hotspotY;

        wxImage modimage = image->getBitmapForState(g_state)->ConvertToImage();

        if (xflip) modimage = modimage.Mirror();
        if (yflip) modimage = modimage.Mirror(false);

        if (angle == 90)
        {
            modimage = modimage.Rotate90();
            if (!xflip) hotspotX_mod = hotspotX + image->width;
            else hotspotX_mod = hotspotX - image->width;
            //hotspotY_mod = hotspotY - image->height;
        }

        if (xscale != 1 or yscale != 1)
            modimage.Rescale( (int)(image->width * xscale),
                              (int)(image->height * yscale) );

        wxBitmap modbitmap(modimage);
        Display::renderDC -> DrawBitmap( modbitmap, x - hotspotX_mod, y - hotspotY_mod);
    }
    else
    {
        Display::renderDC -> DrawBitmap( *image->getBitmapForState(g_state), x - hotspotX, y - hotspotY);
    }

}

int Drawable::getImageWidth()
{
    return image->width;
}
int Drawable::getImageHeight()
{
    return image->height;
}


}

#endif
