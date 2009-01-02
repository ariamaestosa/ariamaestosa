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

#ifndef _wximage_
#define _wximage_

#include "Config.h"
#include "Renderers/ImageBase.h"
#include "wx/wx.h"


namespace AriaMaestosa {

class Image
{
    wxImage image;
    OwnerPtr<wxBitmap>  bitmap;
    
    wxImage* states[AriaRender::STATE_AMOUNT];
    wxBitmap* states_bmp[AriaRender::STATE_AMOUNT];
public:
    LEAK_CHECK(ImageBase);
    
    int width, height;
    
    Image();
    Image(wxString path);
    ~Image();
    
    void load(wxString path);
    
    wxImage* getImageForState(AriaRender::ImageState);
    wxBitmap* getBitmapForState(AriaRender::ImageState);
};


}

#endif
#endif