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

#include "OpenGL.h"
#include "Config.h"

namespace AriaMaestosa {
	
class Image {
    GLuint* ID;
    
public:
	DECLARE_LEAK_CHECK();
	
    int width, height, textureWidth, textureHeight;
    
    float tex_coord_x;
    float tex_coord_y;
    
    GLuint* getID();
    Image();
    Image(wxString path);
    ~Image();
    void load(wxString path);
        
};


}

#endif
