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
#include "Config.h"
#include <iostream>

#include "OpenGL.h"

#include "wx/wx.h"

/*
 * This is a simple class built on top of OpenGL that manages drawing images in a higher-level and quicker way.
 */

namespace AriaMaestosa {

AriaRender::ImageState g_state;

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

    if(image_arg!=NULL) setImage(image_arg);
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
    if(delete_image) delete image;
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
    assert(image!=NULL);

    glLoadIdentity();

    glTranslatef(x*10.0, y*10.0, 0);

    if(xscale!=1 || yscale!=1)
    {
        glScalef(xscale, yscale, 1);
    }

     // unused
    if(angle!=0)
    {
        glRotatef(angle, 0,0,1);
    }

    bool do_yflip = yflip;
    // hack, textureHeight made smaller than zero when image was power of two.
    // in these cases, the image will be upside down so we need to flip it
    if(image->textureHeight < 0) do_yflip = !yflip;

    glBindTexture(GL_TEXTURE_2D, image->getID()[0] );

    glBegin(GL_QUADS);

    glTexCoord2f(xflip? image->tex_coord_x : 0, do_yflip? 0 : image->tex_coord_y);
    glVertex2f( -hotspotX*10.0, -hotspotY*10.0 );

    glTexCoord2f(xflip? 0 : image->tex_coord_x, do_yflip? 0 : image->tex_coord_y);
    glVertex2f( (image->width-hotspotX)*10.0, -hotspotY*10.0 );

    glTexCoord2f(xflip? 0 : image->tex_coord_x, do_yflip? image->tex_coord_y : 0);
    glVertex2f( (image->width-hotspotX)*10.0, (image->height-hotspotY)*10.0 );

    glTexCoord2f(xflip? image->tex_coord_x : 0, do_yflip? image->tex_coord_y : 0);
    glVertex2f( -hotspotX*10.0, (image->height-hotspotY)*10.0 );

    glEnd();
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