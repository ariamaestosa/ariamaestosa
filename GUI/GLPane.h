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

#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"

#include "ptr_vector.h"
#include "Editors/RelativeXCoord.h"
#include <vector>

namespace AriaMaestosa {

    class MainFrame;

class GLPane : public wxGLCanvas
{
public:
    LEAK_CHECK(GLPane);

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

    bool prepareFrame();
    void beginFrame();
    void endFrame();

};

}
#endif
#endif
