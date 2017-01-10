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

#include "Renderers/GLPane.h"
#include "AriaCore.h"

#include "OpenGL.h"

#include <iostream>
#include <cmath>

#include "GUI/MainFrame.h"

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

GLPane::GLPane(wxWindow* parent, int* args) :
#if wxCHECK_VERSION(3,1,0)
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS)
#else
    wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS, wxT("GLCanvas"),  args)
#endif
{
    m_context = new wxGLContext(this);
    
    //Bind(wxEVT_CHAR, &GLPane::OnCharEvent, this);
    /*
#if wxCHECK_VERSION(2,9,1)
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    #ifdef __WXMSW__
    SetDoubleBuffered(true);
    Bind(wxEVT_ERASE_BACKGROUND, &GLPane::OnEraseBackground, this);
    #endif
#endif
 */
}

// -------------------------------------------------------------------------------------------------------

GLPane::~GLPane()
{
}

// -------------------------------------------------------------------------------------------------------

/*
void GLPane::OnCharEvent(wxKeyEvent& evt)
{
    printf("CHAR EVENT\n");
}
*/
// -------------------------------------------------------------------------------------------------------

void GLPane::resized(wxSizeEvent& evt)
{
#if wxCHECK_VERSION(3,1,0)
#else
    wxGLCanvas::OnSize(evt);
#endif

    initOpenGLFor2D();
    Refresh();

    // FIXME - can it really happen that no sequence is open?
    if (getMainFrame()->getSequenceAmount()>0) DisplayFrame::updateVerticalScrollbar();
}

// -------------------------------------------------------------------------------------------------------

void GLPane::setCurrent()
{
    if (!GetParent()->IsShown()) return;
    wxGLCanvas::SetCurrent(*m_context);
}

// -------------------------------------------------------------------------------------------------------

void GLPane::swapBuffers()
{
    wxGLCanvas::SwapBuffers();
}

// -------------------------------------------------------------------------------------------------------

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

    gluOrtho2D(0, GetSize().x*10.0, GetSize().y*10.0, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// -------------------------------------------------------------------------------------------------------

int GLPane::getWidth()
{

    if (Display::isVisible()) return GetSize().x;
    else return 795; // default value
}

// -------------------------------------------------------------------------------------------------------

int GLPane::getHeight()
{
    // FIXME - is it really necessary to check if it's visible?
    if (Display::isVisible()) return GetSize().y;
    else return 550; // approximately default
}

// -------------------------------------------------------------------------------------------------------

bool GLPane::prepareFrame()
{
    if (not GetParent()->IsShown()) return false;
    wxGLCanvas::SetCurrent(*m_context);
    return true;
}

// -------------------------------------------------------------------------------------------------------

void GLPane::beginFrame()
{
    initOpenGLFor2D();
    glClear(GL_COLOR_BUFFER_BIT);
}

// -------------------------------------------------------------------------------------------------------

void GLPane::endFrame()
{
    glFlush();
    SwapBuffers();
}

// -------------------------------------------------------------------------------------------------------

#endif
