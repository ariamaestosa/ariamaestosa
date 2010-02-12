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

#ifndef _mainpane_
#define _mainpane_

#include "Config.h"
#include "Editors/RelativeXCoord.h"

#ifdef RENDERER_OPENGL
#include "Renderers/GLPane.h"
#elif defined(RENDERER_WXWIDGETS)
#include "Renderers/wxRenderPane.h"
#else
#error No renderer defined
#endif

#include <vector>

namespace AriaMaestosa
{

    class MouseDownTimer;
    class MainFrame;

enum ClickArea
{
    CLICK_DOCK,
    CLICK_MEASURE_BAR,
    CLICK_REORDER,
    CLICK_TRACK,
    CLICK_TAB_BAR
};
    
class MainPane : public RenderPane
{
    /** To send events repeatedly when the mouse is held down */
    OwnerPtr<MouseDownTimer>  mouseDownTimer;

    /** Gives information about the location of the mouse in a drag */
    RelativeXCoord mousex_initial;
    
    /** Gives information about the location of the mouse in a drag */
    int mousey_initial;
    
    /** Gives information about the location of the mouse in a drag */
    RelativeXCoord mousex_current;
    
    /** Gives information about the location of the mouse in a drag */
    int mousey_current;
    
    /** Gives information about the location of the mouse in a drag */
    bool isMouseDown_bool;

    int currentTick;
    
    /** which track the user is dragging (in a track reordering process), or -1 if none */
    int draggingTrack;

    std::vector<int> positionsInDock;

    // used during playback
    int followPlaybackTime;
    int lastTick;
    int playbackStartTick;

    bool scrollToPlaybackPosition;

    ClickArea click_area;
    
    /** if click_area == CLICK_TRACK, contains the ID of the track */
    int click_in_track;
    
    bool do_render();
    
    /** Whether the mouse cursor is currently hovering the tab bar */
    bool m_mouse_hovering_tabs;

public:
    LEAK_CHECK();

    MainPane(MainFrame* mainframe, int* args);
    ~MainPane();

    // --------------------- read-only --------------------

    /** is frame shown */
    bool isVisible;
    
    bool leftArrow;
    bool rightArrow;

    // -----------------------------------------------------
    // render loop
    
    void enterPlayLoop();
    
    /** This method is called repeatedly during playback */
    void playbackRenderLoop();
    
    void setPlaybackStartTick(int newValue);
    
    /** This is called when the song us playing. MainPane needs to know the current tick because when it renders
      * it needs to know where to draw the red line that follows playback. */
    void setCurrentTick(int currentTick=-1);
    
    int  getCurrentTick() const;
    void exitPlayLoop();
    void scrollNowToPlaybackPosition();

    int getDraggedTrackID();

    /** called when frame is made visible */
    void isNowVisible();

    // ---- events
    
    /** Event sent whenever user drags mouse on OpenGL pane where everything is drawn. */
    void mouseMoved(wxMouseEvent& event);
    
    /** Event sent whenever user clicks on OpenGL pane where everything is drawn. */
    void mouseDown(wxMouseEvent& event);
    
    void mouseWheelMoved(wxMouseEvent& event);
    
    void mouseReleased(wxMouseEvent& event);
    
    /** Event sent whenever user right-clicks on OpenGL pane where everything is drawn. */
    void rightClick(wxMouseEvent& event);
    
    void mouseLeftWindow(wxMouseEvent& event);
    
    void keyPressed(wxKeyEvent& event);
    void keyReleased(wxKeyEvent& event);
    
    void instrumentPopupSelected(wxCommandEvent& evt);
    void drumPopupSelected(wxCommandEvent& evt);

    /** events will be sent regularly to this method when user holds down mouse */
    void mouseHeldDown();
    
    /** Gives information about the location of the mouse in a drag */
    bool isMouseDown();
    
    /** Gives information about the location of the mouse in a drag */
    RelativeXCoord getMouseX_current();
    
    /** Gives information about the location of the mouse in a drag */
    int getMouseY_current();
    
    /** Gives information about the location of the mouse in a drag */
    RelativeXCoord getMouseX_initial();
    
    /** Gives information about the location of the mouse in a drag */
    int getMouseY_initial();

    bool isSelectMorePressed();
    bool isSelectLessPressed();
    bool isCtrlDown();

    // ---- rendering
    void render(const bool paintEvent = false);
    void paintEvent(wxPaintEvent& evt);

    // ---- serialization
    void saveToFile(wxFileOutputStream& fileout);

    DECLARE_EVENT_TABLE()
};

}
#endif
