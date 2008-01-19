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


/*
 * The GLPane is an OpenGL panel. It is where all drawing is done.
 * It also catches all mouse and keybard events issued on track area and it dispatches them to the appropriate object.
 * (e.g.: if user clicks on track, the event is  by GLPane. Then GLPane finds out where the click was issued and transfers the event to the clicked track.
 */

#include "GUI/GLPane.h"
#include "AriaCore.h"

#include "wx/wx.h"

#include "OpenGL.h"

#include <iostream>
#include <cmath>

#include "GUI/GLPane.h"
#include "GUI/MainFrame.h"
#include "Config.h"

namespace AriaMaestosa {

// ==========================================================================================
// ==========================================================================================


BEGIN_EVENT_TABLE(GLPane, wxGLCanvas)
EVT_MOTION(GLPane::mouseMoved)
EVT_LEFT_DOWN(GLPane::mouseDown)
EVT_LEFT_UP(GLPane::mouseReleased)
EVT_RIGHT_DOWN(GLPane::rightClick)
EVT_LEAVE_WINDOW(GLPane::mouseLeftWindow)

EVT_SIZE(GLPane::resized)

EVT_KEY_DOWN(GLPane::keyPressed)
EVT_KEY_UP(GLPane::keyReleased)

EVT_MENU_RANGE(0+10000,127+10000, GLPane::instrumentPopupSelected)
EVT_MENU_RANGE(0+20000,127+20000, GLPane::drumPopupSelected)

EVT_MOUSEWHEEL(GLPane::mouseWheelMoved)
EVT_PAINT(GLPane::paintEvent)

END_EVENT_TABLE()

GLPane::GLPane(MainFrame* mainFrame, int* args) :
    wxGLCanvas(mainFrame, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"),  args), MainPane()
{

    INIT_LEAK_CHECK();

    int argc = 0;
    char** argv = NULL;
	
	std::cout << "calling glutInit" << std::endl;
    glutInit(&argc, argv);
}

GLPane::~GLPane()
{
}


// FIXME - find better solution
void GLPane::mouseMoved(wxMouseEvent& event)
{
    MainPane::mouseMoved(event);
}
void GLPane::mouseDown(wxMouseEvent& event)
{
    MainPane::mouseDown(event);
}
void GLPane::mouseWheelMoved(wxMouseEvent& event)
{
    MainPane::mouseWheelMoved(event);
}
void GLPane::mouseReleased(wxMouseEvent& event)
{
    MainPane::mouseReleased(event);
}
void GLPane::rightClick(wxMouseEvent& event)
{
    MainPane::rightClick(event);
}
void GLPane::mouseLeftWindow(wxMouseEvent& event)
{
    MainPane::mouseLeftWindow(event);
}

void GLPane::keyPressed(wxKeyEvent& event)
{
    MainPane::keyPressed(event);
}
void GLPane::keyReleased(wxKeyEvent& event)
{
    MainPane::keyReleased(event);
}
void GLPane::instrumentPopupSelected(wxCommandEvent& evt)
{
    MainPane::instrumentPopupSelected(evt);
}
void GLPane::drumPopupSelected(wxCommandEvent& evt)
{
    MainPane::drumPopupSelected(evt);
}

void GLPane::resized(wxSizeEvent& evt)
{

    wxGLCanvas::OnSize(evt);

    initOpenGLFor2D();
    //render();

    Refresh();

    // update vertical scrollbar
    // FIXME belongs to mainpane
    if(getMainFrame()->getSequenceAmount()>0) getMainFrame()->updateTopBarForSequence( getCurrentSequence() );
}


void GLPane::setCurrent()
{
	if (!GetParent()->IsShown()) return; 
    wxGLCanvas::SetCurrent();
}

void GLPane::swapBuffers()
{
    wxGLCanvas::SwapBuffers();
}

void GLPane::initOpenGLFor3D()
{
    /*
     *  Inits the OpenGL viewport for drawing in 3D
     */

    //glShadeModel(GL_SMOOTH);	// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_AUTO_NORMAL);
    //glEnable(GL_VERTEX_ARRAY);
    //glEnable(GL_TEXTURE_COORD_ARRAY);
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_ALPHA_TEST);

    glViewport(0, 0, 640, 480);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 3D
    gluPerspective(45 /*view angle*/, 640.0/480.0, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void GLPane::initOpenGLFor2D()
{

    /*
     *  Inits the OpenGL viewport for drawing in 2D
     */

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background

    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER,0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, GetSize().x, GetSize().y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //2D

    gluOrtho2D(0, GetSize().x, GetSize().y, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int GLPane::getWidth()
{

    if(isVisible) return GetSize().x;
    else return 795; // default value
}

int GLPane::getHeight()
{
    if(isVisible) return GetSize().y;
    else return 550; // approximately default
}


void GLPane::paintEvent(wxPaintEvent& evt)
{
    render(true);
}
void GLPane::render(const bool paintEvent)
{
    if(!GetParent()->IsShown()) return; 
    wxGLCanvas::SetCurrent();
    
    if(paintEvent)
    {
        wxPaintDC(this);
        initOpenGLFor2D();
        glClear(GL_COLOR_BUFFER_BIT);
        if(do_render())
        {
            glFlush();
            SwapBuffers();
        }
    }
    else
    {
        wxClientDC(this);
        initOpenGLFor2D();
        glClear(GL_COLOR_BUFFER_BIT);
        if(do_render())
        {
            glFlush();
            SwapBuffers();
        }
    }
}


}
