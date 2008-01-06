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


#include "GUI/RenderUtils.h"
#include "OpenGL.h"
#include <cmath>
#include "wx/wx.h"
#include <iostream>

namespace AriaMaestosa
{

namespace AriaRender
{

void primitives()
{
    glDisable(GL_TEXTURE_2D);
    glLoadIdentity();
}

void images()
{
    glEnable(GL_TEXTURE_2D);
}
    
void color(const float r, const float g, const float b)
{
    glColor3f(r,g,b);
}

void color(const float r, const float g, const float b, const float a)
{
    glColor4f(r,g,b,a);
}

void line(const int x1, const int y1, const int x2, const int y2)
{
    glBegin(GL_LINES);
    glVertex2f(x1,y1);
    glVertex2f(x2,y2);
    glEnd();
}

void lineWidth(const int n)
{
    glLineWidth(n);
}

void lineSmooth(const bool enabled)
{
    if(enabled) glEnable (GL_LINE_SMOOTH);
    else glDisable (GL_LINE_SMOOTH);
}

void point(const int x, const int y)
{
    glBegin(GL_POINTS);
    glVertex2f(x,y);
    glEnd();
}

void pointSize(const int n)
{
    glPointSize(n);
}

void rect(const int x1, const int y1, const int x2, const int y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2)
{
    rect(x1,y1,x2,y2);
    
    glColor3f(0,0,0);
    glLineWidth(1);
    glBegin(GL_LINES);
    
    //glVertex2f(x1,y1);
    //glvertex2f(x1,y2);
    
    glVertex2f(x1,y2+1);
    glVertex2f(x2,y2+1);
    
    glVertex2f(x2+1,y2);
    glVertex2f(x2+1,y1);
    
    glVertex2f(x1,y1);
    glVertex2f(x2,y1);
    
    glEnd();
}

void bordered_rect(const int x1, const int y1, const int x2, const int y2)
{
    rect(x1,y1,x2,y2);
    
    glColor3f(0,0,0);
    glLineWidth(1);
    glBegin(GL_LINES);
    
    glVertex2f(x1,y1);
    glVertex2f(x1,y2);
    
    glVertex2f(x1,y2+1);
    glVertex2f(x2,y2+1);
    
    glVertex2f(x2+1,y2);
    glVertex2f(x2+1,y1);
    
    glVertex2f(x1,y1);
    glVertex2f(x2,y1);
    
    glEnd();
}

void hollow_rect(const int x1, const int y1, const int x2, const int y2)
{
    glBegin(GL_LINES);
    
    glVertex2f(x1,y1);
    glVertex2f(x1,y2);
    
    glVertex2f(x1,y2);
    glVertex2f(x2,y2);
    
    glVertex2f(x2,y2);
    glVertex2f(x2,y1);
    
    glVertex2f(x1,y1);
    glVertex2f(x2,y1);
    
    glEnd();
}


// FIXME- clean up test rendering
void text(const char* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    for(int i=0; string[i]; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
    }
}
void text(wxString* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    const int len = string->Length();
    for(int i=0; i<len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (*string)[i]);
    }
}

int text_return_end_x(wxString* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    const int len = string->Length();
    for(int i=0; i<len; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (*string)[i]);
    }
    
    // find where text ends
    float rasterPos[4];
    glGetFloatv(GL_CURRENT_RASTER_POSITION,rasterPos);
    return (int)( rasterPos[0] );
    
}

// used for track name, in case it's too long
void text_with_bounds(wxString* string, const int x, const int y, const int max_x)
{
    glRasterPos2f(x, y);
    const int len = string->Length();
    
    for(int i=0; i<len; i++)
    {
       // std::cout << ">" << (*string)[i] << "<" << std::endl;
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, (*string)[i]);
        
        // if text is too long
        float rasterPos[4];
        glGetFloatv(GL_CURRENT_RASTER_POSITION,rasterPos);
        if(rasterPos[0] > max_x)
		{
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
            return;
        }
    }
}

// currently used a guitar editor
void text_append(const char* string)
{
    for(int i=0; string[i]; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
    }
}

void small_text(const char* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    for(int i=0; string[i]; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
    }
}
void character(const char character, const int x, const int y)
{
    glRasterPos2f(x, y);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
}
void small_character(const char character, const int x, const int y)
{
    glRasterPos2f(x, y);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, character);
}

void small_text_newline_between_words(const char* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    int line=0;
    for(int i=0; string[i]; i++)
    {
        if(string[i]==' ')
		{
            line++;
            glRasterPos2f(x, y+line*15);
        }
		else
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
    }
}

void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above)
{
	glLoadIdentity();
    
	const int y_mult = (show_above ? -radius_y : radius_y);
	
	glColor3f(0,0,0);
	glBegin(GL_LINES);
	for(float angle = 0.2; angle<=M_PI; angle +=0.2)
	{
		glVertex2f( center_x + std::cos(angle)*radius_x, center_y + std::sin(angle)*y_mult );
		glVertex2f( center_x + std::cos(angle-0.2)*radius_x, center_y + std::sin(angle-0.2)*y_mult );
	}
	glEnd();	
}

void quad(const int x1, const int y1,
          const int x2, const int y2,
          const int x3, const int y3,
          const int x4, const int y4)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glVertex2f(x4, y4);
    glEnd();
    
}

void beginScissors(const int x, const int y, const int width, const int height)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x,y,width,height);
    
}
void endScissors()
{
    glDisable(GL_SCISSOR_TEST);
}

}
}