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

#include "Config.h"
#include "AriaCore.h"
#include "wx/wx.h"
#include "GUI/RenderUtils.h"

// FIXME - split in two files
#ifndef NO_OPENGL

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

    glVertex2f(x1-1,y1);
    glVertex2f(x2,y1);

    glEnd();
}


// FIXME- clean up text rendering
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
    // glScissor doesn't seem to follow the coordinate system so I need to manually reverse the Y coord
    glScissor(x, Display::getHeight() - y - height, width, height);
}
void endScissors()
{
    glDisable(GL_SCISSOR_TEST);
}

}
}

#else

#include "Images/Drawable.h"

namespace AriaMaestosa
{
namespace AriaRender
{

    bool mode_primitives = false, mode_images = false;

void primitives()
{
    mode_primitives = true;
    mode_images = false;
}

void images()
{
    mode_primitives = false;
    mode_images = true;
}

unsigned char rc = 255;
unsigned char gc = 255;
unsigned char bc = 255;
unsigned char ac = 255;
int lineWidth_i = 1;
int pointSize_i = 1;

void updatePen()
{
    Display::renderDC -> SetPen( wxPen( wxColour( rc, gc, bc, ac ), lineWidth_i ) );
}
void updateBrush()
{
    Display::renderDC -> SetBrush( wxBrush( wxColour( rc, gc, bc, ac )) );
}
void updateFontColor()
{
    Display::renderDC -> SetTextForeground( wxColour( rc, gc, bc, ac ) );
}

void disablePen()
{
    Display::renderDC -> SetPen( *wxTRANSPARENT_PEN );
}
void disableBrush()
{
    Display::renderDC -> SetBrush( *wxTRANSPARENT_BRUSH );
}

void color(const float r, const float g, const float b)
{
    rc = (unsigned char)(r*255);
    gc = (unsigned char)(g*255);
    bc = (unsigned char)(b*255);

    if(mode_primitives)
    {
        ac = 255;
        updatePen();
        updateBrush();
        updateFontColor();
    }
    else
    {
        drawable_set_color(r,g,b);
    }
}

void color(const float r, const float g, const float b, const float a)
{

    rc = (unsigned char)(r*255);
    gc = (unsigned char)(g*255);
    bc = (unsigned char)(b*255);
    ac = (unsigned char)(a*255);

    if(mode_primitives)
    {
        updatePen();
        updateBrush();
        updateFontColor();
    }
    else
    {
        drawable_set_color(r,g,b,a);
    }
}

void line(const int x1, const int y1, const int x2, const int y2)
{
    Display::renderDC -> DrawLine( x1, y1, x2, y2 );
}

void lineWidth(const int n)
{
    lineWidth_i = n;
}

void lineSmooth(const bool enabled)
{

}

void point(const int x, const int y)
{
    if(pointSize_i == 1) Display::renderDC -> DrawPoint( x, y );
    else
    {
        disablePen();
        Display::renderDC -> DrawRectangle( x-pointSize_i/2, y-pointSize_i/2, pointSize_i, pointSize_i );
        updatePen();
    }
}

void pointSize(const int n)
{
    pointSize_i = n;
}

void rect(const int x1, const int y1, const int x2, const int y2)
{
    disablePen();
    Display::renderDC -> DrawRectangle( x1, y1, x2-x1, y2-y1 );
    updatePen();
}

void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2)
{
    disablePen();
    Display::renderDC -> DrawRectangle( x1, y1, x2-x1+1, y2-y1+1 );

    Display::renderDC -> SetPen( *wxBLACK_PEN );
    //Display::renderDC -> DrawLine( x1-1, y1, x1-1, y2 );
    Display::renderDC -> DrawLine( x2+1, y1, x2+1, y2 );

    Display::renderDC -> DrawLine( x1, y1-1, x2, y1-1 );
    Display::renderDC -> DrawLine( x1, y2+1, x2, y2+1 );

    updatePen();
}

void bordered_rect(const int x1, const int y1, const int x2, const int y2)
{

    disablePen();
    Display::renderDC -> DrawRectangle( x1, y1, x2-x1+1, y2-y1+1 );

    Display::renderDC -> SetPen( *wxBLACK_PEN );
    Display::renderDC -> DrawLine( x1-1, y1, x1-1, y2 );
    Display::renderDC -> DrawLine( x2+1, y1, x2+1, y2 );

    Display::renderDC -> DrawLine( x1, y1-1, x2, y1-1 );
    Display::renderDC -> DrawLine( x1, y2+1, x2, y2+1 );

    updatePen();
}

void hollow_rect(const int x1, const int y1, const int x2, const int y2)
{
    disableBrush();
    Display::renderDC -> DrawRectangle( x1, y1, x2-x1, y2-y1 );
    updateBrush();
}


const int FONTSHIFT = -12;

// FIXME- clean up text rendering
void text(const char* string, const int x, const int y)
{
    Display::renderDC -> SetFont( *wxNORMAL_FONT );
    Display::renderDC -> DrawText( fromCString(string), x, y + FONTSHIFT);
}
void text(wxString* string, const int x, const int y)
{
    Display::renderDC -> SetFont( *wxNORMAL_FONT );
    Display::renderDC -> DrawText( *string, x, y + FONTSHIFT);
}

int text_return_end_x(wxString* string, const int x, const int y)
{
    Display::renderDC -> SetFont( *wxNORMAL_FONT );
    Display::renderDC -> DrawText( *string, x, y + FONTSHIFT);
    int twidth, theight;
    Display::renderDC -> GetTextExtent( *string, &twidth, &theight );

    return twidth + x;
}

// used for track name, in case it's too long
void text_with_bounds(wxString* string, const int x, const int y, const int max_x)
{
    wxString message = *string;

    bool shortened = false;
    while(true)
    {
        int twidth, theight;
        Display::renderDC -> GetTextExtent( message, &twidth, &theight );
        if(x + twidth < max_x) break;
        else
        {
            shortened = true;
            message = message.Mid( 0, message.Length()-2 );
        }
    }
    if(shortened)
    {
        message.Append( wxT("...") );
    }

    Display::renderDC -> SetFont( *wxNORMAL_FONT );
    Display::renderDC -> DrawText( message, x, y + FONTSHIFT );
}

void small_text(const char* string, const int x, const int y)
{
    Display::renderDC -> SetFont( *wxSMALL_FONT );
    Display::renderDC -> DrawText( fromCString(string), x, y + FONTSHIFT);
}
void character(const char character, const int x, const int y)
{
    char buffer[2];
    sprintf(buffer, "%c", character);

    Display::renderDC -> SetFont( *wxNORMAL_FONT );
    Display::renderDC -> DrawText( fromCString(buffer), x, y + FONTSHIFT);
}
void small_character(const char character, const int x, const int y)
{
    char buffer[2];
    sprintf(buffer, "%c", character);

    Display::renderDC -> SetFont( *wxSMALL_FONT );
    Display::renderDC -> DrawText( fromCString(buffer), x, y + FONTSHIFT );
}

void small_text_newline_between_words(const char* string, const int x, const int y)
{
    // FIXME - doesn't do what it says
    Display::renderDC -> SetFont( *wxSMALL_FONT );
    Display::renderDC -> DrawText( fromCString(string), x, y + FONTSHIFT);
}

void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3)
{
    disablePen();
    wxPoint array[] = { wxPoint(x1, y1), wxPoint(x2, y2), wxPoint(x3, y3) };
    Display::renderDC -> DrawPolygon( 3, array );
    updatePen();
}

void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above)
{
    Display::renderDC -> SetPen( wxPen( wxColour( rc, gc, bc, ac ), 1 ) );
    disableBrush();
    Display::renderDC -> DrawEllipticArc( center_x - radius_x, center_y - radius_y, radius_x*2, radius_y*2, 0, (show_above ? 180 : -180) );
    updateBrush();
    updatePen();
}

void quad(const int x1, const int y1,
          const int x2, const int y2,
          const int x3, const int y3,
          const int x4, const int y4)
{
    disablePen();
    wxPoint array[] = { wxPoint(x1, y1), wxPoint(x2, y2), wxPoint(x3, y3), wxPoint(x4, y4) };
    Display::renderDC -> DrawPolygon( 4, array );
    updatePen();
}

//wxDCClipper* dc_clipper = NULL;

void beginScissors(const int x, const int y, const int width, const int height)
{
    //Display::renderDC -> DestroyClippingRegion();
    Display::renderDC -> SetClippingRegion( wxPoint(x, y), wxSize(width, height));
}

void endScissors()
{
    Display::renderDC -> DestroyClippingRegion();
}

}
}
#endif
