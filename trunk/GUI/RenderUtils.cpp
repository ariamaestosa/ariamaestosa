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

void text(const char* string, const int x, const int y)
{
    glRasterPos2f(x, y);
    
    for(int i=0; string[i]; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
    }
}
void text_small(const char* string, const int x, const int y)
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
void character_small(const char character, const int x, const int y)
{
    glRasterPos2f(x, y);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, character);
}

void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

}
}