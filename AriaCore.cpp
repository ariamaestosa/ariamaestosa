#include "AriaCore.h"
#include "main.h"
#include "GUI/GLPane.h"
#include "GUI/MainFrame.h"
#include "Midi/Sequence.h"
#include "wx/wx.h"

/*
 *  Stuff global to the app.
 *  This mostly wraps glPane methods. The goal of this is that by including AriaCore.h instead of including
 *  glPane.h you don't by the way include wxWidgets and OpenGL and GLUT headers, making compilation possibly faster
 */

namespace AriaMaestosa
{
    
    GLPane* glPane = NULL;
    void setGLPane(GLPane* pane)
    {
        glPane = pane;
    }

    
void activateRenderLoop(bool on)
{
    wxGetApp().activateRenderLoop(on);
}
    
MainFrame* getMainFrame()
{
    return wxGetApp().frame;
}

GLPane* getGLPane()
{
    return glPane;
}

MeasureBar* getMeasureBar()
{
    return wxGetApp().frame->getCurrentSequence()->measureBar;
}
    
namespace Display
{
    void render()
    {
        glPane->render();
    }
    int getWidth()
    {
        return glPane->getWidth();
    }
    int getHeight()
    {
        return glPane->getHeight();
    }
    bool isMouseDown()
    {
        return glPane->isMouseDown();
    }
    bool isSelectLessPressed()
    {
        return glPane->isSelectLessPressed();
    }
    bool isSelectMorePressed()
    {
        return glPane->isSelectMorePressed();
    }
    bool isCtrlDown()
    {
        return glPane->isCtrlDown();
    }
    
    void popupMenu(wxMenu* menu, const int x, const int y)
    {
        glPane->PopupMenu(menu, x, y);
    }
    
    
    RelativeXCoord getMouseX_current()
    {
        return glPane->getMouseX_current();
    }
    int getMouseY_current()
    {
        return glPane->getMouseY_current();
    }
    RelativeXCoord getMouseX_initial()
    {
        return glPane->getMouseX_initial();
    }
    int getMouseY_initial()
    {
        return glPane->getMouseY_initial();
    }
    
    bool leftArrow()
    {
        return glPane->leftArrow;
    }
    bool rightArrow()
    {
        return glPane->rightArrow;
    }
    
    bool isVisible()
    {
        return glPane->isVisible;
    }
    
    void clientToScreen(const int x_in, const int y_in, int* x_out, int* y_out)
    {
        wxPoint winCoord = glPane->ClientToScreen(wxPoint(x_in,y_in));
        *x_out = winCoord.x;
        *y_out = winCoord.y;
    }
    void screenToClient(const int x_in, const int y_in, int* x_out, int* y_out)
    {
        wxPoint screenCoord = glPane->ScreenToClient(wxPoint(x_in,y_in));
        *x_out = screenCoord.x;
        *y_out = screenCoord.y;
    }
    
    void enterPlayLoop()
    {
        glPane->enterPlayLoop();
    }
    void exitPlayLoop()
    {
        glPane->exitPlayLoop();
    }
    
    void requestFocus()
    {
        glPane->SetFocus();
    }
    
    int getDraggedTrackID()
    {
        return glPane->getDraggedTrackID();
    }
    
    void setPlaybackStartTick(const int tick)
    {
        glPane->setPlaybackStartTick(tick);
    }
}
    
}