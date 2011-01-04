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


#include "GUI/MainPane.h"
#include "AriaCore.h"

#include <iostream>
#include <cmath>

#include "Actions/EditAction.h"
#include "Actions/ResizeNotes.h"
#include "Actions/NumberPressed.h"
#include "Actions/ShiftFrets.h"
#include "Actions/ShiftString.h"
#include "Actions/MoveNotes.h"
#include "Actions/DeleteSelected.h"
#include "Actions/ShiftBySemiTone.h"

#include "GUI/MeasureBar.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "Renderers/RenderAPI.h"
#include "Renderers/Drawable.h"
#include "GUI/ImageProvider.h"
#include "Midi/Track.h"
#include "Midi/Note.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/MagneticGridPicker.h"
#include "Pickers/InstrumentPicker.h"
#include "Pickers/DrumPicker.h"
#include "Editors/RelativeXCoord.h"
#include "Editors/KeyboardEditor.h"

#include <wx/dcbuffer.h>
#include <wx/timer.h>
#include <wx/spinctrl.h> // for wxSpinEvent

#include "Utils.h"

using namespace AriaMaestosa;

#ifdef RENDERER_OPENGL
BEGIN_EVENT_TABLE(MainPane, wxGLCanvas)
#elif defined(RENDERER_WXWIDGETS)
BEGIN_EVENT_TABLE(MainPane, wxPanel)
#endif

EVT_MOTION(MainPane::mouseMoved)
EVT_LEFT_DOWN(MainPane::mouseDown)
EVT_LEFT_UP(MainPane::mouseReleased)
EVT_RIGHT_DOWN(MainPane::rightClick)
EVT_LEAVE_WINDOW(MainPane::mouseLeftWindow)

EVT_SIZE(MainPane::resized)

EVT_KEY_DOWN(MainPane::keyPressed)
EVT_KEY_UP(MainPane::keyReleased)

EVT_MENU_RANGE(0+10000,127+10000, MainPane::instrumentPopupSelected)
EVT_MENU_RANGE(0+20000,127+20000, MainPane::drumPopupSelected)

EVT_MOUSEWHEEL(MainPane::mouseWheelMoved)
EVT_PAINT(MainPane::paintEvent)

END_EVENT_TABLE()

namespace AriaMaestosa
{
    const int TAB_BAR_Y      = 0;
    
    const int TAB_SIDE_WIDTH = 16;
    const int CLOSE_BUTTON_SPACE_FROM_RIGHT = 8;

    const int WHEEL_ZOOM_INCREMENT = 10;
    
    /** By how many pixels does a step horizontally with the mouse wheel move scrolling */
    const int WHEEL_X_SPEED = 25;

    
    int tab_width = 145;

    /** when this is set to 'true', the app will wait for a new click to be begun to process any mouse events
      * (i.e. current click/drag is not valid anymore)
      */
    bool invalidateMouseEvents = false;

    // ==========================================================================================
    // ==========================================================================================
    class MouseDownTimer : public wxTimer
    {
        MainPane* main_pane;

    public:

        MouseDownTimer(MainPane* parent) : wxTimer()
        {
            main_pane = parent;
        }

        void Notify()
        {
            if (!main_pane->isMouseDown())
            {
                Stop();
                return;
            }
            main_pane->mouseHeldDown();

        }

        void start()
        {
            Start(10);
        }
    };
}

// ==========================================================================================================
// ==========================================================================================================
#if 0
#pragma mark -
#endif

MainPane::MainPane(wxWindow* parent, int* args) :
    RenderPane(parent, args),
    m_mouse_x_initial(NULL),
    m_mouse_x_current(NULL)
{
    m_current_tick        = -1;
    m_dragged_track_id    = -1;
    m_is_visible          = false;
    m_is_mouse_down       = false;
    m_mouse_hovering_tabs = false;

    m_mouse_x_initial.setValue(0, MIDI);
    m_mouse_y_initial = 0;
    m_mouse_x_current.setValue(0, WINDOW);
    m_mouse_y_current = 0;

    m_left_arrow  = false;
    m_right_arrow = false;

    m_mouse_down_timer = new MouseDownTimer(this);

    m_scroll_to_playback_position = false;
}

MainPane::~MainPane()
{
}

/**
 * Called when frame has just been made visible. Does things that can't be done without display.
 */
void MainPane::isNowVisible()
{
    MainFrame* mf = getMainFrame();
    mf->addSequence();
    mf->getCurrentSequence()->addTrack();
    m_is_visible = true;
}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Rendering
#endif

void MainPane::paintEvent(wxPaintEvent& evt)
{
    render(true);
}

// --------------------------------------------------------------------------------------------------

void MainPane::render(const bool isPaintEvent)
{
    if (not prepareFrame()) return;
    if (not m_is_visible) return;

    if (isPaintEvent)
    {
        wxAutoBufferedPaintDC mydc(this);

        Display::renderDC = &mydc;

        beginFrame();
        if (do_render()) endFrame();
    }
    else
    {
        Refresh();
        return;
    }

}

// --------------------------------------------------------------------------------------------------

bool MainPane::do_render()
{
    MainFrame* mf = getMainFrame();
    
    if (not ImageProvider::imagesLoaded())  return false;
    if (mf->getCurrentSequence()->importing) return false;

    AriaRender::images();

    GraphicalSequence* gseq = mf->getCurrentGraphicalSequence();
    
    m_mouse_x_initial.setSequence(gseq);
    m_mouse_x_current.setSequence(gseq);
    
    gseq->renderTracks(m_current_tick,
                       m_mouse_x_current,
                       m_mouse_y_current,
                       m_mouse_y_initial,
                       25 + gseq->getMeasureBar()->getMeasureBarHeight());


    // -------------------------- draw tab bar at top -------------------------
    // beige background

    AriaRender::primitives();
    AriaRender::color(1, 1, 0.9);
    AriaRender::rect(0, TAB_BAR_Y, getWidth(), TAB_BAR_Y+20);

    // draw tab
    int start_at_x = 0;
    const int seqamount    = getMainFrame()->getSequenceAmount();
    const int currentSeqID = getMainFrame()->getCurrentSequenceID();

    // if too many tabs for all to be visible, make them smaller
    tab_width = 145;
    if (seqamount*(TAB_SIDE_WIDTH + tab_width + TAB_SIDE_WIDTH) > Display::getWidth())
    {
        tab_width = Display::getWidth() / seqamount - TAB_SIDE_WIDTH*2;
    }

    for (int n=0; n<seqamount; n++)
    {
        AriaRender::images();

        if (currentSeqID == n)
        {
            AriaRender::setImageState(AriaRender::STATE_NORMAL);
            tabBorderDrawable->move(start_at_x, TAB_BAR_Y);
            tabBorderDrawable->setFlip(false, false);
            tabBorderDrawable->render();

            tabDrawable->move(start_at_x+TAB_SIDE_WIDTH, TAB_BAR_Y);
            tabDrawable->scale(tab_width/2.0, 1);
            tabDrawable->render();

            tabBorderDrawable->move(start_at_x+TAB_SIDE_WIDTH+tab_width, TAB_BAR_Y);
            tabBorderDrawable->setFlip(true, false);
            tabBorderDrawable->render();

            if (m_mouse_hovering_tabs)
            {
                // draw close button
                tabCloseDrawable->move(start_at_x + tab_width + TAB_SIDE_WIDTH*2 -
                                       tabCloseDrawable->getImageWidth() - CLOSE_BUTTON_SPACE_FROM_RIGHT,
                                       TAB_BAR_Y + 5);
                tabCloseDrawable->render();
            }
        }
        else
        {
            AriaRender::setImageState(AriaRender::STATE_UNSELECTED_TAB);

            tabBorderDrawable->move(start_at_x, TAB_BAR_Y+3);
            tabBorderDrawable->setFlip(false, false);
            tabBorderDrawable->render();

            tabDrawable->move(start_at_x+TAB_SIDE_WIDTH, TAB_BAR_Y+3);
            tabDrawable->scale(tab_width/2.0, 1);
            tabDrawable->render();

            tabBorderDrawable->move(start_at_x+TAB_SIDE_WIDTH+tab_width, TAB_BAR_Y+3);
            tabBorderDrawable->setFlip(true, false);
            tabBorderDrawable->render();

            if (m_mouse_hovering_tabs)
            {
                wxPoint mouse = this->ScreenToClient(wxGetMousePosition());
                if (mouse.x > start_at_x and mouse.x < start_at_x + TAB_SIDE_WIDTH + tab_width + TAB_SIDE_WIDTH)
                {
                    // draw close button
                    tabCloseDrawable->move(start_at_x + tab_width + TAB_SIDE_WIDTH*2 -
                                           tabCloseDrawable->getImageWidth() - CLOSE_BUTTON_SPACE_FROM_RIGHT,
                                           TAB_BAR_Y + 6);
                    tabCloseDrawable->render();
                }
            }
        }

        AriaRender::images();

        // draw tab name
        if (currentSeqID == n)  AriaRender::color(0,0,0);
        else                    AriaRender::color(0.4, 0.4, 0.4);

        AriaRenderString& seq_name = getMainFrame()->getGraphicalSequence(n)->getNameRenderer();
        seq_name.bind();

        seq_name.render( start_at_x+10, TAB_BAR_Y+20);

        start_at_x += TAB_SIDE_WIDTH+tab_width+TAB_SIDE_WIDTH;
    }//next

    // -------------------------- draw measure top bar -------------------------
    
    gseq->getMeasureBar()->render(MEASURE_BAR_Y);

    // -------------------------- draw dock -------------------------
    AriaRender::primitives();
    const int docksize = gseq->getDockedTrackAmount();
    if (docksize > 0)
    {
        // Make the dock visible
        gseq->setDockVisible(true);

        AriaRender::primitives();
        AriaRender::color(1, 1, 0.9);
        AriaRender::rect(0, getHeight() - gseq->getDockHeight(), getWidth(), getHeight());

        // black line at the top and bottom
        AriaRender::color(0, 0, 0);
        AriaRender::line(0,          getHeight() - gseq->getDockHeight(),
                         getWidth(), getHeight() - gseq->getDockHeight());

        int x = 10;
        int x_before = 0;

        m_positions_in_dock.clear();

        for (int n=0; n<docksize; n++)
        {

            m_positions_in_dock.push_back(x_before);

            x_before = x;

            AriaRender::images();
            AriaRender::color(0,0,0);
            AriaRenderString& trackname = gseq->getDockedTrack(n)->getNameRenderer();
            trackname.bind();
            trackname.render(x+5, getHeight()-2);
            x += trackname.getWidth() + 10;

            AriaRender::primitives();
            AriaRender::color(1, 0.8, 0.7);
            AriaRender::lineWidth(2);
            AriaRender::hollow_rect(x, getHeight() - 1, x_before, getHeight() - 17);

            m_positions_in_dock.push_back(x);
        }//next
        AriaRender::lineWidth(1);
    }
    else
    {
        gseq->setDockVisible(false);
    }



    // -------------------------- red line that follows playback, red arrows --------------------------
    if (m_current_tick != -1) // if playing
    {

        RelativeXCoord tick(m_current_tick, MIDI, gseq);

        const int XStart = Editor::getEditorXStart();
        const int XEnd = getWidth();

        AriaRender::lineWidth(2);
        AriaRender::color(0.8, 0, 0);

        if (tick.getRelativeTo(WINDOW) < XStart) // current tick is before the visible area
        {

            m_left_arrow  = true;
            m_right_arrow = false;

            AriaRender::line(25, MEASURE_BAR_Y + 10, 10, MEASURE_BAR_Y + 10);
            AriaRender::triangle(5,  MEASURE_BAR_Y + 10,
                                 15, MEASURE_BAR_Y + 5,
                                 15, MEASURE_BAR_Y + 15);
        }
        else if (tick.getRelativeTo(WINDOW) > XEnd) // current tick is after the visible area
        {

            m_left_arrow  = false;
            m_right_arrow = true;

            AriaRender::line(XEnd - 15 - 25, MEASURE_BAR_Y + 10,
                             XEnd - 15 - 10, MEASURE_BAR_Y + 10);

            AriaRender::triangle(XEnd - 15 - 5, MEASURE_BAR_Y + 10,
                                 XEnd - 15 - 15, MEASURE_BAR_Y + 5,
                                 XEnd - 15 - 15, MEASURE_BAR_Y + 15);
        }
        else // current tick is inside the visible area
        {

            m_left_arrow  = false;
            m_right_arrow = false;

            // red line in measure bar
            AriaRender::line(tick.getRelativeTo(WINDOW), MEASURE_BAR_Y + 1,
                             tick.getRelativeTo(WINDOW), MEASURE_BAR_Y + 20);
        }

        AriaRender::lineWidth(1);

    }
    else
    { // we're not playing, set arrows to false
        m_left_arrow  = false;
        m_right_arrow = true;
    }

    return true;

}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Pop-ups events
#endif

/**
 * Since instrument picker is shown by MainPane, its events go to MainPane. So, when MainPane catches
 * InstrumentPicker events, it forwards them to it. (FIXME: find better way)
 */
void MainPane::instrumentPopupSelected(wxCommandEvent& evt)
{
    Core::getInstrumentPicker()->menuSelected( evt );
}
// --------------------------------------------------------------------------------------------------

void MainPane::drumPopupSelected(wxCommandEvent& evt)
{
    Core::getDrumPicker()->menuSelected( evt );
}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Input
#endif

/**
 * Are key modifiers down on the keyboard?
 * FIXME: should be static methods
 */
bool MainPane::isSelectMorePressed() const { return wxGetKeyState(WXK_SHIFT);   }
bool MainPane::isSelectLessPressed() const { return wxGetKeyState(WXK_ALT);     }
bool MainPane::isCtrlDown         () const { return wxGetKeyState(WXK_CONTROL); }


// --------------------------------------------------------------------------------------------------

void MainPane::mouseHeldDown()
{
    MainFrame* mf = getMainFrame();
    GraphicalSequence* gseq = mf->getCurrentGraphicalSequence();
    
    // check click is within track area
    if (m_mouse_y_current < getHeight() - gseq->getDockHeight() and
        m_mouse_y_current > MEASURE_BAR_Y + gseq->getMeasureBar()->getMeasureBarHeight())
    {

        // dispatch event to sequence
        gseq->mouseHeldDown(m_mouse_x_current, m_mouse_y_current,
                            m_mouse_x_initial, m_mouse_y_initial);

    }// end if not on dock
}

// --------------------------------------------------------------------------------------------------

void MainPane::rightClick(wxMouseEvent& event)
{
    MainFrame* mf = getMainFrame();
    GraphicalSequence* gseq   = mf->getCurrentGraphicalSequence();
    Sequence* seq = gseq->getModel();
    const int measureBarHeight = gseq->getMeasureBar()->getMeasureBarHeight();

    Display::requestFocus();

    // check click is not on dock before passing event to tracks
    // dispatch event to all tracks (stop when either of them uses it)
    if (event.GetY() < getHeight() - gseq->getDockHeight() and
        event.GetY() > MEASURE_BAR_Y + measureBarHeight)
    {
        const int count = seq->getTrackAmount();
        for (int n=0; n<count; n++)
        {
            if (not seq->getTrack(n)->getGraphics()->processRightMouseClick( RelativeXCoord(event.GetX(), WINDOW, gseq) , event.GetY()))
            {
                seq->setCurrentTrackID(n);
                break;
            }
        }
    }

    // ---- click is in measure bar
    if (event.GetY() > MEASURE_BAR_Y and event.GetY() < MEASURE_BAR_Y + measureBarHeight)
    {
        gseq->getMeasureBar()->rightClick(event.GetX(), event.GetY() - MEASURE_BAR_Y);
    }

    Display::render();
}

// --------------------------------------------------------------------------------------------------

void MainPane::mouseDown(wxMouseEvent& event)
{
    invalidateMouseEvents = false;
    Display::requestFocus();

    MainFrame* mf = getMainFrame();
    GraphicalSequence* gseq = mf->getCurrentGraphicalSequence();
    Sequence* seq = gseq->getModel();
    
    m_mouse_x_current.setSequence(gseq);
    m_mouse_x_initial.setSequence(gseq);
    
    m_mouse_x_current.setValue(event.GetX(), WINDOW);
    m_mouse_y_current = event.GetY();

    m_mouse_x_initial.setValue(event.GetX(), WINDOW);
    m_mouse_x_initial.convertTo(MIDI); // we know scrolling may change so better keep it as midi coords
    m_mouse_y_initial = m_mouse_y_current;

    m_is_mouse_down=true;

    int measureBarHeight = gseq->getMeasureBar()->getMeasureBarHeight();

    // ----------------------------------- click is in track area ----------------------------
    // check click is within track area
    if (m_mouse_y_current < getHeight() - gseq->getDockHeight() and
        event.GetY() > MEASURE_BAR_Y + measureBarHeight)
    {
        m_click_area = CLICK_TRACK;

        // dispatch event to all tracks (stop when either of them uses it)
        m_click_in_track = -1;
        const unsigned int track_amount = gseq->getTrackAmount();
        for (unsigned int n=0; n<track_amount; n++)
        {
            GraphicalTrack* gtrack = gseq->getTrack(n);
            const int y = gtrack->getCurrentEditor()->getTrackYStart();

            // check if user is moving this track
            if (not gtrack->isDocked() and
                m_mouse_y_current > y and m_mouse_y_current < y+7)
            {
                m_click_area = CLICK_REORDER;
                m_dragged_track_id = n;
            }
            else
            { 
                // otherwise ask the track to check if it has something to do with this event
                const bool event_processed = not gtrack->processMouseClick(m_mouse_x_current, event.GetY());
                if (event_processed)
                {
                    m_click_in_track = n;
                    break;
                }
            }
        }
    }// end if not on dock

    // ----------------------------------- click is in dock ----------------------------
    if (event.GetY() > getHeight() - gseq->getDockHeight())
    {
        m_click_area = CLICK_DOCK;
        ASSERT_E( (int)m_positions_in_dock.size()/2 ,==,(int)gseq->getDockedTrackAmount());

        for (unsigned int n=0; n<m_positions_in_dock.size(); n+=2)
        {

            if (event.GetX()>m_positions_in_dock[n] and event.GetX()<m_positions_in_dock[n+1])
            {
                if (gseq->isTrackMaximized())
                {
                    const int track_amount = seq->getTrackAmount();
                    GraphicalTrack* undocked_gtrack = gseq->getDockedTrack(n/2);
                    Track* undocked = undocked_gtrack->getTrack();

                    for (int i=0; i<track_amount; i++)
                    {
                        Track* track = seq->getTrack(i);
                        GraphicalTrack* gtrack = gseq->getGraphicsFor(track);
                        if (gtrack == undocked_gtrack)
                        {
                            undocked_gtrack->dock(false);
                            gtrack->maximizeHeight();
                            continue;
                        }
                        else if (not gtrack->isDocked())
                        {
                            gtrack->dock();
                        }
                    }
                    if (undocked_gtrack->isCollapsed())
                    {
                        undocked_gtrack->setCollapsed(false);
                    }
                    
                    DisplayFrame::updateVerticalScrollbar();
                    seq->setCurrentTrack(undocked);
                }
                else
                {
                    gseq->getDockedTrack(n/2)->dock(false);
                    DisplayFrame::updateVerticalScrollbar();
                }
                return;
            }
        }
     }//end if user is clicking on the dock

    // ----------------------------------- click is in tab bar ----------------------------
    if (not PlatformMidiManager::get()->isPlaying() and event.GetY() > TAB_BAR_Y and
        event.GetY() < TAB_BAR_Y+20)
    {
        m_click_area = CLICK_TAB_BAR;

        int start_at_x = 0;
        const int seqAmount = mf->getSequenceAmount();
        for (int n=0; n<seqAmount; n++)
        {
            start_at_x += TAB_SIDE_WIDTH + tab_width + TAB_SIDE_WIDTH;
            if (event.GetX() < start_at_x)
            {
                if (event.GetX() > start_at_x - tabCloseDrawable->getImageWidth() - CLOSE_BUTTON_SPACE_FROM_RIGHT and
                    event.GetX() < start_at_x - CLOSE_BUTTON_SPACE_FROM_RIGHT)
                {
                    // click is on close button
                    mf->closeSequence(n);
                    return;
                }
                else
                {
                    // click is on tab body
                    mf->setCurrentSequence(n);
                    return;
                }
            }
        }//next
    }//end if

    // ----------------------------------- click is in measure bar ----------------------------
    if (event.GetY() > MEASURE_BAR_Y and event.GetY() < MEASURE_BAR_Y+measureBarHeight)
    {
        m_click_area = CLICK_MEASURE_BAR;

        if (not (m_current_tick != -1 and (m_left_arrow or m_right_arrow))) // ignore when playing
        {
            gseq->getMeasureBar()->mouseDown(m_mouse_x_current.getRelativeTo(WINDOW),
                                             m_mouse_y_current - MEASURE_BAR_Y);
        }

    }

    Display::render();

    // ask sequence if it is necessary at this point to be notified of mouse held down events.
    // if so, start a timer that will take of it.
    if (gseq->areMouseHeldDownEventsNeeded()) m_mouse_down_timer->start();

}

// --------------------------------------------------------------------------------------------------

void MainPane::mouseMoved(wxMouseEvent& event)
{
    if (invalidateMouseEvents) return;

    m_mouse_x_current.setValue(event.GetX(),WINDOW);
    m_mouse_y_current = event.GetY();

    if (event.Dragging())
    {
        // we are not reordering tracks
        if (m_dragged_track_id == -1)
        {
            GraphicalSequence* gseq = getMainFrame()->getCurrentGraphicalSequence();

            
            // ----------------------------------- click is in track area ----------------------------
            if (m_click_area == CLICK_TRACK and m_click_in_track != -1)
            {
                Sequence* seq = getMainFrame()->getCurrentSequence();
                Track* track = seq->getTrack(m_click_in_track);
                gseq->getGraphicsFor(track)->processMouseDrag(m_mouse_x_current, event.GetY());
            }

            // ----------------------------------- click is in measure bar ----------------------------
            if (m_click_area == CLICK_MEASURE_BAR)
            {
                gseq->getMeasureBar()->mouseDrag(m_mouse_x_current.getRelativeTo(WINDOW),
                                                 m_mouse_y_current - MEASURE_BAR_Y,
                                                 m_mouse_x_initial.getRelativeTo(WINDOW),
                                                 m_mouse_y_initial - MEASURE_BAR_Y);
            }
        }

        Display::render();

    } // end if dragging
    else
    {
        const bool mouse_hovering_tabs =  (event.GetY() > TAB_BAR_Y and event.GetY() < TAB_BAR_Y+20);

        if (mouse_hovering_tabs and not m_mouse_hovering_tabs)
        {
            m_mouse_hovering_tabs = true;
            Display::render();
            return;
        }
        else if (m_mouse_hovering_tabs and not mouse_hovering_tabs)
        {
            m_mouse_hovering_tabs = false;
            Display::render();
            return;
        }
        if (m_mouse_hovering_tabs)
        {
            Display::render();
        }
    }

}

// --------------------------------------------------------------------------------------------------

void MainPane::mouseLeftWindow(wxMouseEvent& event)
{
    // if we are dragging, notify current track that mouse has left the window
    if (m_is_mouse_down)
    {
        getMainFrame()->
        getCurrentGraphicalSequence()->
        getCurrentTrack()->
        processMouseExited(m_mouse_x_current, m_mouse_y_current,
                           m_mouse_x_initial, m_mouse_y_initial);

        invalidateMouseEvents = true; // ignore all mouse events until a new click/drag is begun
    }

    if (m_mouse_hovering_tabs)
    {
        m_mouse_hovering_tabs = false;
        Display::render();
    }
}

// --------------------------------------------------------------------------------------------------

void MainPane::mouseReleased(wxMouseEvent& event)
{

    m_is_mouse_down = false;
    if (invalidateMouseEvents) return;

    GraphicalSequence* gseq = getMainFrame()->getCurrentGraphicalSequence();
    
    // if releasing after having dragged a track
    if (m_dragged_track_id != -1)
    {
        gseq->reorderTracks();
        m_dragged_track_id = -1;
    }

    // ---- click is in measure bar
    if (m_click_area == CLICK_MEASURE_BAR)
    {
        // check if user is clicking on red arrow that scrolls to current playback location
        if (m_left_arrow)
        {
            if (m_mouse_x_current.getRelativeTo(WINDOW)>5 and m_mouse_x_current.getRelativeTo(WINDOW)<25)
            {
                scrollNowToPlaybackPosition();
            }
        }
        else if (m_right_arrow)
        {
            if (m_mouse_x_current.getRelativeTo(WINDOW) > getWidth()-45 and
                m_mouse_x_current.getRelativeTo(WINDOW) < getWidth()-20)
                scrollNowToPlaybackPosition();
        }

        // measure selection
        if (not (m_current_tick!=-1 and (m_left_arrow or m_right_arrow)) ) // ignore when playing
        {
            gseq->getMeasureBar()->mouseUp(m_mouse_x_current.getRelativeTo(WINDOW),
                                           m_mouse_y_current - MEASURE_BAR_Y,
                                           m_mouse_x_initial.getRelativeTo(WINDOW),
                                           m_mouse_y_initial - MEASURE_BAR_Y);
        }
    }
    else if (m_click_area == CLICK_TRACK and m_click_in_track != -1)
    {
        // disptach mouse up event to current track
        Track* track = getMainFrame()->getCurrentSequence()->getTrack(m_click_in_track);
        gseq->getGraphicsFor(track)->processMouseRelease();
    }

    Display::render();
}

// --------------------------------------------------------------------------------------------------

void MainPane::keyReleased(wxKeyEvent& evt)
{
    /*
    if (isSelectMorePressed())
    {
        SetCursor( wxCursor(wxCURSOR_COPY_ARROW) );
    }
    else
    {
        SetCursor( wxNullCursor );
    }
     */
}

// --------------------------------------------------------------------------------------------------

void MainPane::keyPressed(wxKeyEvent& evt)
{
    /*
    if (isSelectMorePressed())
    {
        SetCursor( wxCursor(wxCURSOR_COPY_ARROW) );
    }
    else
    {
        SetCursor( wxNullCursor );
    }*/
    
    
    MainFrame* mf = getMainFrame();
    GraphicalSequence* gseq = mf->getCurrentGraphicalSequence();
    Sequence* seq = gseq->getModel();
    
#ifdef __WXMAC__
    const bool commandDown = evt.MetaDown() or evt.ControlDown();
#else
    const bool commandDown = evt.ControlDown();
#endif
    const bool shiftDown   = evt.ShiftDown();

    // ---------------- play selected notes -----------------
    if (evt.GetKeyCode() == WXK_SPACE)
    {
        seq->spacePressed();
    }
    
#ifdef _MORE_DEBUG_CHECKS
    if (evt.GetKeyCode() == WXK_F3)
    {
        wxPoint p = wxGetMousePosition();
        RelativeXCoord x(gseq);
        x.setValue(ScreenToClient(p).x, WINDOW);
        printf("Tick : %i\n", x.getRelativeTo(MIDI));
    }
#endif

    // ---------------- resize notes -----------------
    if (commandDown and not shiftDown)
    {

        if (evt.GetKeyCode() == WXK_LEFT)
        {
            seq->getCurrentTrack()->
            action( new Action::ResizeNotes(
                        -seq->ticksPerBeat() *
                        4 /
                        seq->getCurrentTrack()->getMagneticGrid()->getDivider(),
                        SELECTED_NOTES)
                   );
            Display::render();
        }

        if (evt.GetKeyCode() == WXK_RIGHT)
        {
            seq->getCurrentTrack()->
            action( new Action::ResizeNotes(
                        seq->ticksPerBeat() *
                        4 /
                        seq->getCurrentTrack()->getMagneticGrid()->getDivider(),
                        SELECTED_NOTES)
                    );
            Display::render();
        }

    }

    // perform editor-specific event filtering

    // FIXME(DESIGN) - belongs to the editor, probably (move all editor stuff to editor files)
    // FIXME - too many renders there, maybe even actions do render

    const NotationType current_editor = seq->getCurrentTrack()->getNotationType();

    // --------------- move by 1 measure ------------
    if (current_editor != GUITAR and not commandDown and shiftDown)
    {
        if (evt.GetKeyCode() == WXK_RIGHT or evt.GetKeyCode() == WXK_LEFT)
        {
            Track* track = seq->getCurrentTrack();
            const int noteID = track->getFirstSelectedNote();
            if (noteID == -1)
            {
                wxBell();
            }
            else
            {
                const int tick = track->getNoteStartInMidiTicks(noteID);
                const int measure = seq->getMeasureData()->measureAtTick(tick);
                
                const int factor = (evt.GetKeyCode() == WXK_LEFT ? -1 : 1);
                
                seq->getCurrentTrack()->action(
                        new Action::MoveNotes(factor*seq->getMeasureData()->measureLengthInTicks(measure),
                                              0,
                                              SELECTED_NOTES)
                        );
                Display::render();
            }
            return;
        }
    }

    // ---------------- shift by octave -------------
    if (current_editor == KEYBOARD and not commandDown and shiftDown)
    {
        if (evt.GetKeyCode() == WXK_UP)
        {
            seq->getCurrentTrack()->action(new Action::MoveNotes(0, -12, SELECTED_NOTES));
            Display::render();
            return;
        }

        if (evt.GetKeyCode() == WXK_DOWN)
        {
            seq->getCurrentTrack()->action(new Action::MoveNotes(0, 12, SELECTED_NOTES));
            Display::render();
            return;
        }
    }

    if (current_editor == GUITAR)
    {

        // ------------------- numbers -------------------
        // number at the top of the keyboard
        if (evt.GetKeyCode() >= 48 and evt.GetKeyCode() <= 57)
        {
            if (shiftDown)
            {
                seq->getCurrentTrack()->action(new Action::NumberPressed(evt.GetKeyCode() - 48 + 10) );
            }
            else
            {
                seq->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 48) );
            }
            Display::render();
        }

        // numpad
        if (evt.GetKeyCode() >= 324 and evt.GetKeyCode() <= 333)
        {
            if (shiftDown)
            {
                seq->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324 + 10) );
            }
            else
            {
                seq->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324) );
            }
            Display::render();
        }

        // ---------------- shift frets -----------------
        if (not commandDown and shiftDown)
        {

            if (evt.GetKeyCode() == WXK_LEFT)
            {
                seq->getCurrentTrack()->action( new Action::ShiftFrets(-1, SELECTED_NOTES) );
                Display::render();
            }

            if (evt.GetKeyCode() == WXK_RIGHT)
            {
                seq->getCurrentTrack()->action( new Action::ShiftFrets(1, SELECTED_NOTES) );
                Display::render();
            }

            if (evt.GetKeyCode() == WXK_UP)
            {
                seq->getCurrentTrack()->action( new Action::ShiftString(-1, SELECTED_NOTES) );
                Display::render();
            }

            if (evt.GetKeyCode() == WXK_DOWN)
            {
                seq->getCurrentTrack()->action( new Action::ShiftString(1, SELECTED_NOTES) );
                Display::render();
            }

        }
    }

    if (current_editor == SCORE and (commandDown or shiftDown) )
    {
            if (evt.GetKeyCode() == WXK_UP)
            {
                seq->getCurrentTrack()->action( new Action::ShiftBySemiTone(-1, SELECTED_NOTES) );
                Display::render();
                return;
            }

            if (evt.GetKeyCode() == WXK_DOWN)
            {
                seq->getCurrentTrack()->action( new Action::ShiftBySemiTone(1, SELECTED_NOTES) );
                Display::render();
                return;
            }

    }

    if (not commandDown and (not shiftDown or current_editor == SCORE))
    {
        // ---------------- move notes -----------------

        if (evt.GetKeyCode() == WXK_LEFT)
        {
            seq->getCurrentTrack()->
            action( new Action::MoveNotes(
                      -seq->ticksPerBeat() * 4 /
                      seq->getCurrentTrack()->getMagneticGrid()->getDivider(), 0, SELECTED_NOTES)
                    );
            Display::render();
        }

        if (evt.GetKeyCode() == WXK_RIGHT)
        {
            seq->getCurrentTrack()->
            action( new Action::MoveNotes(
                      seq->ticksPerBeat() * 4 /
                      seq->getCurrentTrack()->getMagneticGrid()->getDivider(), 0, SELECTED_NOTES)
                    );
            Display::render();
        }

        if (evt.GetKeyCode() == WXK_UP)
        {
            seq->getCurrentTrack()->action( new Action::MoveNotes(0,-1,SELECTED_NOTES) );
            Display::render();
        }

        if (evt.GetKeyCode() == WXK_DOWN)
        {
            seq->getCurrentTrack()->action( new Action::MoveNotes(0,1,SELECTED_NOTES) );
            Display::render();
        }

        // ------------------------ delete notes ---------------------

        if (evt.GetKeyCode() == WXK_BACK or evt.GetKeyCode() == WXK_DELETE)
        {

            seq->getCurrentTrack()->action( new Action::DeleteSelected() );
            Display::render();
        }


    }//end if command down
}

// ----------------------------------------------------------------------------------------------------------

void MainPane::mouseWheelMoved(wxMouseEvent& event)
{
    const int value = event.GetWheelRotation() / event.GetWheelDelta();
    GraphicalSequence* gseq = getMainFrame()->getCurrentGraphicalSequence();
    
#if wxCHECK_VERSION(2,9,1)
    // horizontal scrolling
    if (event.GetWheelAxis() == 1)
    {
        gseq->setXScrollInPixels( gseq->getXScrollInPixels() - value*WHEEL_X_SPEED );
        DisplayFrame::updateHorizontalScrollbar();
        return;
    }
#endif
    
    if (event.m_controlDown)
	{
	    // Ctrl key hold: Zoom in/out
        //if (not changingValues)
        {
            int newZoom;
            
            newZoom = gseq->getZoomInPercent();
            
            if (value > 0)
            {
                newZoom += WHEEL_ZOOM_INCREMENT;
            }
            else
            {
                newZoom -= WHEEL_ZOOM_INCREMENT;
            }
            
            if (newZoom > 1 and newZoom < 500)
            {
                const int ticks = gseq->getXScrollInMidiTicks();
                
                // FIXME(DESIGN): scrolling should not need to be manually changed when zoom is...
                //                A possible solution is to store a midi tick scroll instead of the current pixel scroll
                gseq->setZoom(newZoom);
                
                // update scrolling to new zoom
                gseq->setXScrollInMidiTicks( ticks );
                                
                getMainFrame()->updateTopBarAndScrollbarsForSequence(gseq);
                
                Display::render();
            }
        }
        return;
	}
    
    const int my = event.GetY();
    const int mx = event.GetX();

    const int measureBarHeight = gseq->getMeasureBar()->getMeasureBarHeight();

    // check pointer is within tracks area
    if (my < getHeight() - gseq->getDockHeight() and
        mx > MEASURE_BAR_Y + measureBarHeight)
    {

        // dispatch event to all tracks (stop when either of them uses it)
        Sequence* seq = getMainFrame()->getCurrentSequence();
        const int count = seq->getTrackAmount();
        for (int n=0; n<count; n++)
        {
            if (not seq->getTrack(n)->getGraphics()->mouseWheelMoved(mx, my, value))
            {
                break;
            }
        }
    }// end if not on dock
}

// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Playback Loop
#endif

void MainPane::enterPlayLoop()
{
    m_left_arrow  = false;
    m_right_arrow = false;
    m_follow_playback_time = getMainFrame()->getCurrentSequence()->getMeasureData()->defaultMeasureLengthInTicks();
    m_last_tick = -1;
    Core::activateRenderLoop(true);
}

// ----------------------------------------------------------------------------------------------------------

void MainPane::exitPlayLoop()
{
    PlatformMidiManager::get()->stop();
    getMainFrame()->toolsExitPlaybackMode();
    Core::activateRenderLoop(false);
    setCurrentTick( -1 );
    render();
}

// --------------------------------------------------------------------------------------------------

void MainPane::playbackRenderLoop()
{
    const int currentTick = PlatformMidiManager::get()->trackPlaybackProgression();

    // check if song is over
    if (currentTick == -1 or !PlatformMidiManager::get()->isPlaying())
    {
        exitPlayLoop();
        return;
    }

    GraphicalSequence* gseq = getMainFrame()->getCurrentGraphicalSequence();
    Sequence* seq = gseq->getModel();
    const int startTick = seq->getPlaybackStartTick();
    
    // only draw if it has changed
    if (m_last_tick != startTick + currentTick)
    {

        // if user has clicked on a little red arrow
        if (m_scroll_to_playback_position)
        {
            m_scroll_to_playback_position=false;
            const int x_scroll_in_pixels = (int)( (startTick + currentTick) * gseq->getZoom() );
            gseq->setXScrollInPixels(x_scroll_in_pixels);
            DisplayFrame::updateHorizontalScrollbar( startTick + currentTick );
        }

        // if follow playback is checked in the menu
        if (seq->isFollowPlaybackEnabled())
        {
            RelativeXCoord tick(startTick + currentTick, MIDI, gseq);
            const int current_pixel = tick.getRelativeTo(WINDOW);

            //const float zoom = getCurrentSequence()->getZoom();
            const int XStart = Editor::getEditorXStart();
            const int XEnd = getWidth() - 50; // 50 is somewhat arbitrary
            const int last_visible_measure = gseq->getMeasureBar()->measureAtPixel( XEnd );
            const int current_measure = seq->getMeasureData()->measureAtTick(startTick + currentTick);

            if (current_pixel < XStart or current_measure >= last_visible_measure)
            {
                int new_scroll_in_pixels = (startTick + currentTick) * gseq->getZoom();
                if (new_scroll_in_pixels < 0) new_scroll_in_pixels=0;
                // FIXME(DESIGN) - the GUI should not be updated independently of the model
                gseq->setXScrollInPixels(new_scroll_in_pixels);
                DisplayFrame::updateHorizontalScrollbar( startTick + currentTick );
            }

            /*
            int x_scroll_in_pixels = (int)( (m_playback_start_tick + currentTick - m_follow_playback_time) *
                getCurrentSequence()->getZoom() );
            if ( x_scroll_in_pixels < 0 ) x_scroll_in_pixels = 0;
            getCurrentSequence()->setXScrollInPixels(x_scroll_in_pixels);
            DisplayFrame::updateHorizontalScrollbar( m_playback_start_tick + currentTick - m_follow_playback_time );
                 */
        }

        setCurrentTick( startTick + currentTick );

        RelativeXCoord tick(m_current_tick, MIDI, gseq);
        const int XStart = Editor::getEditorXStart();
        const int XEnd = getWidth();
        const int tick_pixel = tick.getRelativeTo(WINDOW);

        if (tick_pixel < XStart and m_left_arrow)
        {
            // current tick is before the visible area and arrow already there. no need to render again.
        }
        else if (tick_pixel > XEnd and m_right_arrow)
        {
            // current tick is after the visible area and arrow already there. no need to render again.
        }
        else
        {
            Display::render();
        }
        m_last_tick = startTick + currentTick;
    }

    // FIXME - why pause the main thread, aren't there better ways?
    wxMilliSleep(10);
}

// --------------------------------------------------------------------------------------------------

void MainPane::scrollNowToPlaybackPosition()
{
    // set a flag that will be picked by the playback loop
    m_scroll_to_playback_position = true;
}

// --------------------------------------------------------------------------------------------------

void MainPane::setCurrentTick(int currentTick)
{
    m_current_tick = currentTick;
}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Serialization
#endif

void MainPane::saveToFile(wxFileOutputStream& fileout)
{
    getMainFrame()->getCurrentGraphicalSequence()->saveToFile(fileout);
}

