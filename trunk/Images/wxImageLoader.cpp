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

#include "Images/ImageLoader.h"
#include "Config.h"

#include <cmath>

#include "wx/image.h"
#include "wx/wx.h"

#include <IO/IOUtils.h>

namespace AriaMaestosa {
	
	//static bool wxImageHandlers_inited = false;
	
	GLuint* loadImage(wxString path, int* imageWidth, int* imageHeight, int* textureWidth, int* textureHeight)
	{
		/*
		if(!wxImageHandlers_inited)
		{
			wxInitAllImageHandlers();
			wxImageHandlers_inited=true;
		}*/
		
		GLuint* ID=new GLuint[1];
		glGenTextures( 1, &ID[0] );
		
		glBindTexture( GL_TEXTURE_2D, *ID );
		
		// the first time, check resources are there
		static bool is_first_time = true;
		if(is_first_time)
		{
			is_first_time = false;
			if(!wxFileExists(getResourcePrefix()  + path))
			{
				wxMessageBox( _("Failed to load resource image") );
				assert(false);	
			}
		}
		
		wxImage* img=new wxImage( getResourcePrefix()  + path );
		
		(*imageWidth)=img->GetWidth();
		(*imageHeight)=img->GetHeight();
		
		glPixelStorei(GL_UNPACK_ALIGNMENT,   1   );
		
		float power_of_two_that_gives_correct_width=std::log((float)(*imageWidth))/std::log(2.0);
		float power_of_two_that_gives_correct_height=std::log((float)(*imageHeight))/std::log(2.0);
		
		// check if image bounds are a power of two
		if( (int)power_of_two_that_gives_correct_width == power_of_two_that_gives_correct_width and
			(int)power_of_two_that_gives_correct_height == power_of_two_that_gives_correct_height){
			
			// if yes, everything is fine
			
			glTexImage2D(GL_TEXTURE_2D,
						 0,
						 //img->HasAlpha() ?  GL_RGBA : GL_RGB, 
						 img->HasAlpha() ?  4 : 3, 
						 *imageWidth,
						 *imageHeight,
						 0, 
						 // here's where we have the format of the item which is RGB
						 img->HasAlpha() ?  GL_RGBA : GL_RGB,
						 GL_UNSIGNED_BYTE,
						 img->GetData());
			
			(*textureWidth)  = (*imageWidth);
			(*textureHeight) = (*imageHeight);
			
		}
		else // ----------------- Texture is not a power of two. We need to resize it --------------------
		{
			
			int newWidth=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
			int newHeight=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );
			
			//printf("Unsupported image size. Recommand values: %i %i\n",newWidth,newHeight);   
			
			GLubyte	*bitmapData=img->GetData();
			GLubyte        *alphaData=img->GetAlpha();
			GLubyte	*imageData;
			
			int old_bytesPerPixel = 3;
			int bytesPerPixel = img->HasAlpha() ?  4 : 3;
			
			//int oldImageSize = (*imageWidth) * (*imageHeight) * old_bytesPerPixel;
			
			int imageSize = newWidth * newHeight * bytesPerPixel;
			imageData=(GLubyte *)malloc(imageSize);
			
			int rev_val=(*imageHeight)-1;
			
			for(int y=0; y<newHeight; y++)
			{
				for(int x=0; x<newWidth; x++)
				{
					
					if( x<(*imageWidth) && y<(*imageHeight) ){
						assertExpr( (x+y*newWidth)*bytesPerPixel+ (bytesPerPixel-1), <, imageSize);
						/*
						assertExpr( (x+y*(*imageWidth))*old_bytesPerPixel + (old_bytesPerPixel-1), <, oldImageSize);
			*/			
						imageData[(x+y*newWidth)*bytesPerPixel+0]=
							bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 0];
						
						imageData[(x+y*newWidth)*bytesPerPixel+1]=
							bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 1];
						
						imageData[(x+y*newWidth)*bytesPerPixel+2]=
							bitmapData[( x+(rev_val-y)*(*imageWidth))*old_bytesPerPixel + 2];
						
						if(bytesPerPixel==4) imageData[(x+y*newWidth)*bytesPerPixel+3]=
							alphaData[ x+(rev_val-y)*(*imageWidth) ];
						
					}
					else
					{
						assertExpr( (x+y*newWidth)*bytesPerPixel+(bytesPerPixel-1), <, imageSize);
						
						imageData[(x+y*newWidth)*bytesPerPixel+0] = 0;
						imageData[(x+y*newWidth)*bytesPerPixel+1] = 0;
						imageData[(x+y*newWidth)*bytesPerPixel+2] = 0;
						if(bytesPerPixel==4) imageData[(x+y*newWidth)*bytesPerPixel+3] = 0;
					}
					
				}//next
			}//next
			
			glTexImage2D(GL_TEXTURE_2D,
						 0,
						 //img->HasAlpha() ?  GL_RGBA : GL_RGB,  
						 img->HasAlpha() ?  4 : 3,
						 newWidth,
						 newHeight,
						 0, 
						 img->HasAlpha() ?  GL_RGBA : GL_RGB, 
						 GL_UNSIGNED_BYTE,
						 imageData);
			
			(*textureWidth)=newWidth;
			(*textureHeight)=newHeight;
			
			free(imageData);
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		return ID;
		
	}
	
}

