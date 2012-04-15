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

/*
 * This is a class built on top of OpenGL to go with Drawable. It deals with OpenGL textures.
 */

#include "Renderers/ImageBase.h"
#include "Utils.h"

#include <iostream>
#include <cmath>

#include <wx/image.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>

#include <IO/IOUtils.h>

namespace AriaMaestosa
{
    
    GLuint* loadImage(wxString path, int* imageWidth, int* imageHeight, int* textureWidth, int* textureHeight)
    {
        GLuint* ID = new GLuint[1];
        
        glGetError(); // clear error
        
        glGenTextures( 1, &ID[0] );
        
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            fprintf(stderr, "Failed to load image <%s> into OpenGL, glGenTextures error %i (%s)\n",
                    (const char*)path.utf8_str(), err, gluErrorString(err));
            wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (glGenTextures)") );
            ASSERT(false);
            return NULL;
        }
        
        glBindTexture( GL_TEXTURE_2D, *ID );
        
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            fprintf(stderr, "Failed to load image <%s> into OpenGL, glBindTexture error %i (%s)\n",
                    (const char*)path.utf8_str(), err, gluErrorString(err));
            wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (glBindTexture)") );
            ASSERT(FALSE);
            return NULL;
        }
        
        
        // the first time, check resources are there
        static bool is_first_time = true;
        if (is_first_time)
        {
            is_first_time = false;
            if (!wxFileExists(getResourcePrefix()  + path))
            {
                wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (wxFileExists)")  );
                ASSERT(false);
                return NULL;
            }
        }
        
        wxImage* img = new wxImage( getResourcePrefix() + path );
        if (not img->IsOk())
        {
            fprintf(stderr, "WARNING: ***** failed to load image '%s'\n", (const char*)(getResourcePrefix() + path).mb_str() );
            wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (new wxImage)") );
            ASSERT(false);
            return NULL;
        } 

        //std::cout << path.mb_str() << (img->HasAlpha() ? " has alpha" : " does NOT have alpha")
        //          << " and " << (img->HasMask() ? " DOES have a mask" : " does note have a mask") << std::endl;
        
        
        (*imageWidth)  = img->GetWidth();
        (*imageHeight) = img->GetHeight();
        
        printf("image size : %i %i\n", *imageWidth, *imageHeight);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT,   1   );
        
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            fprintf(stderr, "Failed to load image <%s> into OpenGL, glPixelStorei error %i (%s)\n",
                    (const char*)path.utf8_str(), err, gluErrorString(err));
            wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (glPixelStorei)")  );
            ASSERT(false);
            return NULL;
        }
        
        float power_of_two_that_gives_correct_width  = std::log((float)(*imageWidth))/std::log(2.0);
        float power_of_two_that_gives_correct_height = std::log((float)(*imageHeight))/std::log(2.0);
        
        
        // If the image is not a power-of-two, we need to copy it over to a larger power-of-2 image.
        // Also, pack alpha/masks into the bytes to be RGBA (wx provides BGR + A separately)
        int newWidth  = (int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
        int newHeight = (int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );
        
        GLubyte* bitmapData = img->GetData();
        GLubyte* alphaData  = (img->HasAlpha() ? img->GetAlpha() : NULL);
        GLubyte* imageData;
        
        int old_bytesPerPixel = 3;
        int bytesPerPixel = img->HasAlpha() or img->HasMask() ?  4 : 3;
        
        //std::cout << "   bytesPerPixel = " << bytesPerPixel << "\n";
        
        int imageSize = newWidth * newHeight * bytesPerPixel;
        imageData = (GLubyte *)malloc(imageSize);
        
        int rev_val = (*imageHeight)-1;
        
        for (int y=0; y<newHeight; y++)
        {
            for (int x=0; x<newWidth; x++)
            {
                const int destArrayIndex = (x+y*newWidth)*bytesPerPixel;
                
                if (x<(*imageWidth) and y<(*imageHeight))
                {
                    const int srcArrayIndex  = (x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel;
                    ASSERT_E( srcArrayIndex + (bytesPerPixel-1), <, imageSize);
                    
                    imageData[destArrayIndex+0] = bitmapData[srcArrayIndex + 0];
                    imageData[destArrayIndex+1] = bitmapData[srcArrayIndex + 1];
                    imageData[destArrayIndex+2] = bitmapData[srcArrayIndex + 2];
                    
                    if (bytesPerPixel == 4)
                    {
                        if (img->HasMask())
                        {
                            imageData[destArrayIndex+3] = (img->IsTransparent(x, y) ? 0 : 255);
                            //std::cout << "Pixel " << x << ", " << y << " has mask value " << imageData[destArrayIndex+3] << std::endl;
                        }
                        else
                        {
                            ASSERT(alphaData != NULL);
                            imageData[destArrayIndex+3] = alphaData[ x+(rev_val-y)*(*imageWidth) ];
                        }
                    }
                    
                }
                else
                {
                    ASSERT_E( (x+y*newWidth)*bytesPerPixel+(bytesPerPixel-1), <, imageSize);
                    
                    imageData[destArrayIndex+0] = 0;
                    imageData[destArrayIndex+1] = 0;
                    imageData[destArrayIndex+2] = 0;
                    if (bytesPerPixel == 4) imageData[destArrayIndex+3] = 0;
                }
                
            }//next
        }//next
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     (img->HasAlpha() or img->HasMask() ?  4 : 3),
                     newWidth,
                     newHeight,
                     0,
                     (img->HasAlpha() or img->HasMask() ?  GL_RGBA : GL_RGB),
                     GL_UNSIGNED_BYTE,
                     imageData);
        
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            fprintf(stderr, "Failed to load image <%s> into OpenGL, error %i (%s)\n",
                    (const char*)path.utf8_str(), err, gluErrorString(err));
            wxMessageBox( _("Failed to load resource image") + wxString::FromUTF8(" (glTexImage2D)")  );
            ASSERT(false);
            return NULL;
        }
        
        (*textureWidth)=newWidth;
        (*textureHeight)=newHeight;
        
        free(imageData);
        
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
        
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
        delete img;
        
        return ID;
        
    }
    
#if 0
#pramga -
#endif
    
    // -------------------------------------------------------------------------------------------------------

    Image::Image()
    {
        
    }
    
    // -------------------------------------------------------------------------------------------------------
    
    Image::Image(wxString path)
    {
        load(path);
    }
    
    // -------------------------------------------------------------------------------------------------------
    
    void Image::load(wxString path)
    {
        ID=loadImage(path, &width, &height, &textureWidth, &textureHeight);
        
        tex_coord_x = (float)width/(float)textureWidth;
        tex_coord_y = (float)height/(float)fabsf(textureHeight);
    }
    
    // -------------------------------------------------------------------------------------------------------

    GLuint* Image::getID()
    {
        return ID;
    }
    
    // -------------------------------------------------------------------------------------------------------

    Image::~Image()
    {
        glDeleteTextures (1, ID);
    }
    
    // -------------------------------------------------------------------------------------------------------
    
}

#endif
