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

#include "Renderers/RenderAPI.h"

#include <vector>

namespace AriaMaestosa
{
    const int MEASURE_BAR_Y  = 20;
    const int MEASURE_BAR_H  = 20;
    const int EXPANDED_MEASURE_BAR_H  = 40;
    
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
        enum WelcomeResult
        {
            NOTHING_SPECIAL,
            ABORT_RENDER,
            RENDER_AGAIN
        };
        
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

        /** During playback, the current tick; -1 otherwise */
        int m_current_tick;

        /** which track the user is dragging (in a track reordering process), or -1 if none */
        int m_dragged_track_id;

        std::vector<int> m_positions_in_dock;

        // used during playback
        int m_follow_playback_time;
        int m_last_tick;

        bool m_scroll_to_playback_position;

        ClickArea m_click_area;

        /** if click_area == CLICK_TRACK, contains the ID of the track */
        int m_click_in_track;

        /** Whether the mouse cursor is currently hovering the tab bar */
        bool m_mouse_hovering_tabs;

        /** is frame shown */
        bool m_is_visible;
        
        bool m_left_arrow;
        bool m_right_arrow;
        
        AriaRenderString m_new_sequence_label;
        AriaRenderString m_open_label;
        AriaRenderString m_import_label;
        AriaRenderString m_configure_label;
        AriaRenderString m_help_label;
        AriaRenderString m_quit_label;

        bool do_render();
        
        WelcomeResult drawWelcomeMenu();
        
        AriaRenderString m_star;
        
    public:
        LEAK_CHECK();

        MainPane(wxWindow* parent, int* args);
        ~MainPane();

        // render loop

        void enterPlayLoop();

        /** This method is called repeatedly during playback */
        void playbackRenderLoop();

        /** This is called when the song us playing. MainPane needs to know the current tick because when it renders
         * it needs to know where to draw the red line that follows playback. */
        void setCurrentTick(int currentTick=-1);

        int  getCurrentTick() const { return m_current_tick; }
        void exitPlayLoop();
        void scrollNowToPlaybackPosition();

        /**
          * @return the ID of the track the user is dragging (in a track reordering process),
          *         or -1 if no reordering is currently being done
          */
        int getDraggedTrackID() const { return m_dragged_track_id; }

        /** @brief called when frame is made visible */
        void isNowVisible();

        bool isLeftArrowVisible () const { return m_left_arrow;  }
        bool isRightArrowVisible() const { return m_right_arrow; }

        
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

        static bool isSelectMorePressed();
        static bool isSelectLessPressed();
        static bool isCtrlDown         ();

        /** to be called e.g. when a track is deleted */
        //FIXME: that's ugly but it does the job...
        void forgetClickData()
        {
            m_click_in_track   = -1;
            m_dragged_track_id = -1;
        }

        // ---- rendering
        bool isVisible() const { return m_is_visible; }
        void render(const bool paintEvent = false);
        void paintEvent(wxPaintEvent& evt);

        // ---- serialization
        void saveToFile(wxFileOutputStream& fileout);

        DECLARE_EVENT_TABLE()
    };

}
#endif
