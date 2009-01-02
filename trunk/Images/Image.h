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

#ifndef _image_
#define _image_

#include "Config.h"
#include "GUI/RenderUtils.h"

#ifndef NO_OPENGL
#include "OpenGL.h"
#else
#include "wx/wx.h"
#endif


namespace AriaMaestosa {

class Image {

#ifndef NO_OPENGL
    GLuint* ID;
    public:
#else
    public:
    wxImage image;
    OwnerPtr<wxBitmap>  bitmap;
    
    wxImage* states[AriaRender::STATE_AMOUNT];
    wxBitmap* states_bmp[AriaRender::STATE_AMOUNT];
    
    wxImage* getImageForState(AriaRender::ImageState);
    wxBitmap* getBitmapForState(AriaRender::ImageState);
#endif

    LEAK_CHECK(Image);

    int width, height;
    
#ifndef NO_OPENGL
    int textureWidth, textureHeight;
    float tex_coord_x;
    float tex_coord_y;

    GLuint* getID();
#endif

    Image();
    Image(wxString path);
    ~Image();
    void load(wxString path);

};


}

#endif
