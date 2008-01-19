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

#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "wx/wfstream.h"

#include "irrXML/irrXML.h"

#include "ptr_vector.h"
#include "Editors/RelativeXCoord.h"
#include "Config.h"
#include "GUI/MainPane.h"
#include <vector>

namespace AriaMaestosa {

    class MainFrame;
    
class GLPane : public wxGLCanvas, public MainPane
{
    
	DECLARE_LEAK_CHECK();

public:
        
	
    GLPane(MainFrame* parent, int* args);
    ~GLPane();

	void resized(wxSizeEvent& evt);

    // size
    int getWidth();
    int getHeight();
    
    // OpenGL stuff
    void setCurrent();
    void swapBuffers();
    void initOpenGLFor3D();
    void initOpenGLFor2D();
    
    // events
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseWheelMoved(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void rightClick(wxMouseEvent& event);
    void mouseLeftWindow(wxMouseEvent& event);
    void mouseHeldDown(); // events will be sent regularly to this method when user holds down mouse
    void keyPressed(wxKeyEvent& event);
    void keyReleased(wxKeyEvent& event);
	void instrumentPopupSelected(wxCommandEvent& evt);
	void drumPopupSelected(wxCommandEvent& evt);
    
    void render(const bool paintEvent = false);
    
    void paintEvent(wxPaintEvent& evt);

    DECLARE_EVENT_TABLE()
};

}
#endif
