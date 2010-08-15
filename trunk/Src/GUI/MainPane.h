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

#ifndef __MAIN_PANE_H__
#define __MAIN_PANE_H__

#include "Utils.h"
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

    /**
      * @ingroup gui
      */
    enum ClickArea
    {
        CLICK_DOCK,
        CLICK_MEASURE_BAR,
        CLICK_REORDER,
        CLICK_TRACK,
        CLICK_TAB_BAR
    };

    /**
      * @ingroup gui
      */
    class MainPane : public RenderPane
    {
        /** To send events repeatedly when the mouse is held down */
        OwnerPtr<MouseDownTimer> m_mouse_down_timer;

        /** Gives information about the location of the mouse in a drag */
        RelativeXCoord m_mouse_x_initial;

        /** Gives information about the location of the mouse in a drag */
        int m_mouse_y_initial;

        /** Gives information about the location of the mouse in a drag */
        RelativeXCoord m_mouse_x_current;

        /** Gives information about the location of the mouse in a drag */
        int m_mouse_y_current;

        /** Gives information about the location of the mouse in a drag */
        bool m_is_mouse_down;

        int currentTick;

        /** which track the user is dragging (in a track reordering process), or -1 if none */
        int m_dragged_track_id;

        std::vector<int> m_positions_in_dock;

        // used during playback
        int m_follow_playback_time;
        int lastTick;
        int m_playback_start_tick;

        bool m_scroll_to_playback_position;

        ClickArea click_area;

        /** if click_area == CLICK_TRACK, contains the ID of the track */
        int m_click_in_track;

        bool do_render();

        /** Whether the mouse cursor is currently hovering the tab bar */
        bool m_mouse_hovering_tabs;

    public:
        LEAK_CHECK();

        MainPane(wxWindow* parent, int* args);
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

        /**
          * @return the ID of the track the user is dragging (in a track reordering process),
          *         or -1 if no reordering is currently being done
          */
        int getDraggedTrackID() const { return m_dragged_track_id; }

        /** @brief called when frame is made visible */
        void isNowVisible();

        // ---- events

        /** @brief Event sent whenever user drags mouse on OpenGL pane where everything is drawn. */
        void mouseMoved(wxMouseEvent& event);

        /** @brief Event sent whenever user clicks on OpenGL pane where everything is drawn. */
        void mouseDown(wxMouseEvent& event);

        void mouseWheelMoved(wxMouseEvent& event);

        void mouseReleased(wxMouseEvent& event);

        /** @brief Event sent whenever user right-clicks on OpenGL pane where everything is drawn. */
        void rightClick(wxMouseEvent& event);

        void mouseLeftWindow(wxMouseEvent& event);

        void keyPressed(wxKeyEvent& event);
        void keyReleased(wxKeyEvent& event);

        void instrumentPopupSelected(wxCommandEvent& evt);
        void drumPopupSelected(wxCommandEvent& evt);

        /** @brief events will be sent regularly to this method when user holds down mouse */
        void mouseHeldDown();

        /** @brief ives information about the location of the mouse in a drag */
        bool isMouseDown() const { return m_is_mouse_down; }

        /** @brief Gives information about the location of the mouse in a drag */
        RelativeXCoord getMouseX_current() const { return m_mouse_x_current; }

        /** @brief Gives information about the location of the mouse in a drag */
        int getMouseY_current()            const { return m_mouse_y_current; }

        /** @brief Gives information about the location of the mouse in a drag */
        RelativeXCoord getMouseX_initial() const { return m_mouse_x_initial; }

        /** @brief Gives information about the location of the mouse in a drag */
        int getMouseY_initial()            const { return m_mouse_y_initial; }

        bool isSelectMorePressed() const;
        bool isSelectLessPressed() const;
        bool isCtrlDown         () const;

        /** to be called e.g. when a track is deleted */
        //FIXME: that's ugly but it does the job...
        void forgetClickData()
        {
            m_click_in_track   = -1;
            m_dragged_track_id = -1;
        }

        // ---- rendering
        void render(const bool paintEvent = false);
        void paintEvent(wxPaintEvent& evt);

        // ---- serialization
        void saveToFile(wxFileOutputStream& fileout);

        DECLARE_EVENT_TABLE()
    };

}
#endif
