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

#include "Utils.h"
#include "AriaCore.h"
#include "Singleton.h"
#include "Renderers/RenderAPI.h"

#include "OpenGL.h"
#include <cmath>
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
    glLoadIdentity();
}

void setImageState(const ImageState imgst)
{
    switch(imgst)
    {
        case STATE_NO_FOCUS :
            color(0.5, 0.5, 0.5);
            break;
        case STATE_DISABLED :
            color(0.4, 0.4, 0.4);
            break;
        case STATE_UNSELECTED_TAB :
            color(1, 1, 1, 0.4);
            break;
        case STATE_NORMAL :
            color(1,1,1);
            break;
        case STATE_SELECTED_NOTE :
            color(1,0,0);
            break;
        case STATE_NOTE :
            color(0,0,0);
            break;
        case STATE_GHOST :
            color(1.0, 1.0, 1.0, 0.5); // not a real gray out, more a fade out...
            break;
        default: ASSERT(false);
    }
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
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    glEnd();
}

void lineWidth(const int n)
{
    glLineWidth(n);
}

void lineSmooth(const bool enabled)
{
    if (enabled) glEnable (GL_LINE_SMOOTH);
    else glDisable (GL_LINE_SMOOTH);
}

void point(const int x, const int y)
{
    glBegin(GL_POINTS);
    glVertex2f(x*10.0, y*10.0);
    glEnd();
}

void pointSize(const int n)
{
    glPointSize(n);
}

void rect(const int x1, const int y1, const int x2, const int y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x2*10.0, y1*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x1*10.0, y2*10.0);
    glEnd();
}

void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2)
{
    rect(x1,y1,x2,y2);

    glColor3f(0,0,0);
    glLineWidth(1);
    glBegin(GL_LINES);

    glVertex2f(round(x1*10.0), round((y2+0.5)*10.0));
    glVertex2f(round(x2*10.0), round((y2+0.5)*10.0));

    glVertex2f(round((x2+1)*10.0), round(y2*10.0));
    glVertex2f(round((x2+1)*10.0), round((y1+0.5)*10.0));

    glVertex2f(round((x1+0.5)*10.0), round(y1*10.0));
    glVertex2f(round(x2*10.0), round(y1*10.0));

    glEnd();
}

void bordered_rect(const int x1, const int y1, const int x2, const int y2)
{
    rect(x1,y1,x2,y2);

    /** the numbers below were determined by trial-and-error in order
        to work on all computers i have access to... damn those graphics
        drivers and their inconsistent rounding!*/

    glColor3f(0,0,0);
    glLineWidth(1);
    glBegin(GL_LINES);

    glVertex2f(round(x1*10.0), round(y2*10.0));
    glVertex2f(round(x1*10.0), round((y1+0.549)*10.0));

    glVertex2f(round(x1*10.0), round((y2+0.5)*10.0));
    glVertex2f(round(x2*10.0), round((y2+0.5)*10.0));

    glVertex2f(round((x2+1)*10.0), round(y2*10.0));
    glVertex2f(round((x2+1)*10.0), round((y1+0.5)*10.0));

    glVertex2f(round((x1+0.5)*10.0), round(y1*10.0));
    glVertex2f(round(x2*10.0), round(y1*10.0));

    glEnd();
}

void hollow_rect(const int x1, const int y1, const int x2, const int y2)
{
    glBegin(GL_LINES);

    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x1*10.0, y2*10.0);

    glVertex2f(round(x1-1.0)*10.0, y2*10.0);
    glVertex2f(x2*10.0, y2*10.0);

    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x2*10.0, y1*10.0);

    glVertex2f(round(x1-1.0)*10.0, y1*10.0);
    glVertex2f(x2*10.0, y1*10.0);

    glEnd();
}

    
void select_rect(const int x1, const int y1, const int x2, const int y2)
{
    glColor4f(0.0f, 0.83f, 0.16f, 0.3f);
    
    glBegin(GL_QUADS);
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x2*10.0, y1*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x1*10.0, y2*10.0);
    glEnd();
    
    glColor4f(0.0f, 0.83f, 0.16, 1.0f);

    glBegin(GL_LINES);
    
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x1*10.0, y2*10.0);
    
    glVertex2f(round(x1-1.0)*10.0, y2*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    
    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x2*10.0, y1*10.0);
    
    glVertex2f(round(x1-1.0)*10.0, y1*10.0);
    glVertex2f(x2*10.0, y1*10.0);
    
    glEnd();
}

void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x3*10.0, y3*10.0);
    glEnd();
}

void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above)
{
    glLoadIdentity();

    const int y_mult = (show_above ? -radius_y*10.0 : radius_y*10.0);
    center_x *= 10.0f;
    center_y *= 10.0f;
    radius_x *= 10.0f;

    glColor3f(0,0,0);
    glBegin(GL_LINES);
    for (float angle = 0.2; angle<=M_PI; angle +=0.2)
    {
        glVertex2f( center_x + std::cos(angle)    *radius_x, center_y + std::sin(angle)*y_mult );
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
    glVertex2f(x1*10.0, y1*10.0);
    glVertex2f(x2*10.0, y2*10.0);
    glVertex2f(x3*10.0, y3*10.0);
    glVertex2f(x4*10.0, y4*10.0);
    glEnd();

}

class NumberRendererSingleton : public wxGLNumberRenderer, public Singleton<NumberRendererSingleton>
{   
public:
    NumberRendererSingleton() : wxGLNumberRenderer()
    {
    }
    
    virtual ~NumberRendererSingleton()
    {
    }
};
    DEFINE_SINGLETON( NumberRendererSingleton );
 
void renderNumber(const int number, const int x, const int y)
{
    const char* number_c;
    switch (number)
    {
        // We have many pre-defined strings to avoid the costly call to convert int to string
        case 0: number_c = "0"; break;
        case 1: number_c = "1"; break;
        case 2: number_c = "2"; break;
        case 3: number_c = "3"; break;
        case 4: number_c = "4"; break;
        case 5: number_c = "5"; break;
        case 6: number_c = "6"; break;
        case 7: number_c = "7"; break;
        case 8: number_c = "8"; break;
        case 9: number_c = "9"; break;
        case 10: number_c = "10"; break;
        case 11: number_c = "11"; break;
        case 12: number_c = "12"; break;
        case 13: number_c = "13"; break;
        case 14: number_c = "14"; break;
        case 15: number_c = "15"; break;
        case 16: number_c = "16"; break;
        case 17: number_c = "17"; break;
        case 18: number_c = "18"; break;
        case 19: number_c = "19"; break;
        
        default:
        {
            char buffer[32];
            sprintf(buffer,"%i",number);
            renderNumber( buffer, x, y );
            return;
        }
    }

    renderNumber( number_c, x, y );
}
void renderNumber(const float number, const int x, const int y)
{
    renderNumber( to_wxString(number), x, y );
}
void renderNumber(const char* number, const int x, const int y)
{
    NumberRendererSingleton* singleton = NumberRendererSingleton::getInstance();
    singleton->bind();
    singleton->renderNumber(number, x, y);
}
    
void beginScissors(const int x, const int y, const int width, const int height)
{
    glEnable(GL_SCISSOR_TEST);
    // glScissor doesn't seem to follow the coordinate system so I need to manually reverse the Y coord
    glScissor(x, (Display::getHeight() - y - height), width, height);
}
void endScissors()
{
    glDisable(GL_SCISSOR_TEST);
}

}
}

#endif