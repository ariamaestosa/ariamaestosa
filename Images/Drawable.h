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

#ifndef _drawable_
#define _drawable_

#include "Config.h"
#include "GUI/RenderUtils.h"

namespace AriaMaestosa {

    class Image;

#ifdef NO_OPENGL
void drawable_set_state(AriaRender::ImageState arg);
#endif

class Drawable
{
public:
    LEAK_CHECK(Drawable);

    // ------------ read-only fields -------------
    int x,y, angle, hotspotX, hotspotY;
    float xscale, yscale;
    Image* image;
    bool xflip, yflip;
    bool delete_image;
    // -------------------------------------------


    Drawable(Image* image=(Image*)0);
    Drawable(wxString imagePath);
    ~Drawable();

    void setFlip(bool x, bool y);

    int getImageWidth();
    int getImageHeight();

    void move(int x, int y);
    void setHotspot(int x, int y);
    void scale(float x, float y);
    void scale(float k);
    void setImage(Image* image);
    void render();
    void rotate(int angle);

};

}


#endif
