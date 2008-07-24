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
#ifndef NO_OPENGL

#include "GUI/GLPane.h"
#include "AriaCore.h"

#include "wx/wx.h"

#include "OpenGL.h"

#include <iostream>
#include <cmath>

#include "GUI/MainFrame.h"

namespace AriaMaestosa {

// ==========================================================================================
// ==========================================================================================

GLPane::GLPane(MainFrame* mainFrame, int* args) :
    wxGLCanvas(mainFrame, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"),  args)
{

    

    int argc = 0;
    char** argv = NULL;

	std::cout << "calling glutInit" << std::endl;
    glutInit(&argc, argv);
}

GLPane::~GLPane()
{
}



void GLPane::resized(wxSizeEvent& evt)
{

    wxGLCanvas::OnSize(evt);

    initOpenGLFor2D();
    Refresh();

    // FIXME - can it really happen that no sequence is open?
    if(getMainFrame()->getSequenceAmount()>0) DisplayFrame::updateVerticalScrollbar();
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

    if(Display::isVisible()) return GetSize().x;
    else return 795; // default value
}

int GLPane::getHeight()
{
    // FIXME - is it really necessary to check if it's visible?
    if(Display::isVisible()) return GetSize().y;
    else return 550; // approximately default
}

bool GLPane::prepareFrame()
{
    if(!GetParent()->IsShown()) return false;
    wxGLCanvas::SetCurrent();
    return true;
}

void GLPane::beginFrame()
{
    initOpenGLFor2D();
    glClear(GL_COLOR_BUFFER_BIT);
}
void GLPane::endFrame()
{
    glFlush();
    SwapBuffers();
}


}

#endif
