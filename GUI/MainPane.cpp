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

#include "wx/wx.h"

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

#include "GUI/GraphicalTrack.h"
#include "GUI/MeasureBar.h"
#include "GUI/MainFrame.h"
#include "GUI/RenderUtils.h"
#include "Images/Drawable.h"
#include "Images/ImageProvider.h"
#include "Midi/Track.h"
#include "Midi/Note.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/MagneticGrid.h"
#include "Pickers/InstrumentChoice.h"
#include "Pickers/DrumChoice.h"
#include "Editors/RelativeXCoord.h"
#include "Editors/KeyboardEditor.h"

#include "wx/dcbuffer.h"

#include "Config.h"


namespace AriaMaestosa {

#ifndef NO_OPENGL
BEGIN_EVENT_TABLE(MainPane, wxGLCanvas)
#else
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

const int tabBarY = 0;
const int measureBarY = 20;
int tab_width=145;

// when this is set to 'true', the app will wait for a new click to be begun to process any mouse events (i.e. current click/drag is not valid anymore)
bool invalidateMouseEvents=false;

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
        if(!main_pane->isMouseDown())
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
// ==========================================================================================
// ==========================================================================================
#pragma mark -

MainPane::MainPane(MainFrame* mainframe, int* args) : MAINPANE_BASE_CLASS(mainframe, args)
{

    INIT_LEAK_CHECK();

    currentTick=-1;
    draggingTrack = -1;
    isVisible=false;
    isMouseDown_bool=false;

    mousex_initial.setValue(0,MIDI);
    mousey_initial = 0;
    mousex_current.setValue(0,WINDOW);
    mousey_current = 0;

    leftArrow=false;
    rightArrow=false;

    mouseDownTimer = new MouseDownTimer(this);

    scrollToPlaybackPosition=false;
    playbackStartTick=0; // tells from where the red line should start when playing back
}

MainPane::~MainPane()
{
	delete mouseDownTimer;
}

/*
 * Called when frame has just been made visible. Does things that can't be done without display.
 */

void MainPane::isNowVisible()
{
	getMainFrame()->addSequence();
    getCurrentSequence()->addTrack();
    isVisible=true;
}

#pragma mark -

void MainPane::paintEvent(wxPaintEvent& evt)
{
    render(true);
}

void MainPane::render(const bool paintEvent)
{
    if(!prepareFrame()) return;

    if(paintEvent)
    {
        wxAutoBufferedPaintDC mydc(this);

        #ifdef NO_OPENGL
            Display::renderDC = &mydc;
        #endif
        beginFrame();
        if(do_render()) endFrame();
    }
    else
    {
#ifdef __WXMAC__
        wxClientDC mydc(this);
#else
    #ifdef NO_OPENGL
        wxClientDC my_client_dc(this);
        // FIXME - this can maybe be speed up by providing your own bitmap
        wxBufferedDC mydc(static_cast<wxDC*>(&my_client_dc), wxDefaultSize);
    #else
        wxClientDC my_dc(this);
    #endif
#endif
        #ifdef NO_OPENGL
            Display::renderDC = static_cast<wxDC*>(&mydc);
        #endif
        beginFrame();
        if(do_render()) endFrame();
    }

}

bool MainPane::do_render()
{
    
    if(!ImageProvider::imagesLoaded()) return false;
    if(getCurrentSequence()->importing) return false;
    
    AriaRender::images();
    
    getCurrentSequence()->renderTracks( currentTick, mousex_current,
                                        mousey_current, mousey_initial,
                                        25 + getMeasureBar()->getMeasureBarHeight());
    
    
    // -------------------------- draw tab bar at top -------------------------
    // beige background
    
    AriaRender::primitives();
    AriaRender::color(1, 1, 0.9);
    AriaRender::rect(0, tabBarY, getWidth(), tabBarY+20);
    
    // draw tab
    int start_at_x = 0;
    const int seqamount = getMainFrame()->getSequenceAmount();
    const int currentSeqID = getMainFrame()->getCurrentSequenceID();
    
    // if too many tabs for all to be visible, make them smaller
    tab_width = 145;
    if( seqamount*(tab_width+16+16) > Display::getWidth() )
        tab_width = Display::getWidth() / seqamount - 32;
    
    for(int n=0; n<seqamount; n++)
	{
        AriaRender::images();
        
        if(currentSeqID == n)
		{
            AriaRender::color(1,1,1);
            tabBorderDrawable->move(start_at_x, tabBarY);
            tabBorderDrawable->setFlip(false, false);
            tabBorderDrawable->render();
            
            tabDrawable->move(start_at_x+16, tabBarY);
            tabDrawable->scale(tab_width/2.0, 1);
            tabDrawable->render();
            
            tabBorderDrawable->move(start_at_x+16+tab_width, tabBarY);
            tabBorderDrawable->setFlip(true, false);
            tabBorderDrawable->render();
        }
		else
		{
            AriaRender::color(1,1,1, 0.4);
            
            tabBorderDrawable->move(start_at_x, tabBarY+3);
            tabBorderDrawable->setFlip(false, false);
            tabBorderDrawable->render();
            
            tabDrawable->move(start_at_x+16, tabBarY+3);
            tabDrawable->scale(tab_width/2.0, 1);
            tabDrawable->render();
            
            tabBorderDrawable->move(start_at_x+16+tab_width, tabBarY+3);
            tabBorderDrawable->setFlip(true, false);
            tabBorderDrawable->render();
        }
        
        AriaRender::primitives();
        
        // draw tab name
        if(currentSeqID == n)
            AriaRender::color(0,0,0);
		else
            AriaRender::color(0.4, 0.4, 0.4);
        
        AriaRender::text_with_bounds(&getMainFrame()->getSequence(n)->sequenceFileName, start_at_x+10, tabBarY+18, start_at_x+tab_width+12);
        
        start_at_x += tab_width+16+16;
    }//next
    
    // -------------------------- draw measure top bar -------------------------
    
	getMeasureBar()->render(measureBarY);
    
    // -------------------------- draw dock -------------------------
    AriaRender::primitives();
    if(getCurrentSequence()->dockSize>0)
	{
        getCurrentSequence()->dockHeight = 20;
        
        AriaRender::primitives();
        AriaRender::color(1, 1, 0.9);
        AriaRender::rect(0,getHeight()-getCurrentSequence()->dockHeight, getWidth(), getHeight());
        
        // black line at the top and bottom
        AriaRender::color(0, 0, 0);
        AriaRender::line(0, getHeight()-getCurrentSequence()->dockHeight,
                         getWidth(), getHeight()-getCurrentSequence()->dockHeight);
        
        int x=10;
        int x_before = 0;
        
        positionsInDock.clear();
        
        for(int n=0; n<getCurrentSequence()->dockSize; n++)
		{
            
            positionsInDock.push_back(x_before);
            
            x_before = x;
            
            AriaRender::color(0,0,0);
            x = AriaRender::text_return_end_x(&getCurrentSequence()->dock[n].track->getName(), x+5, getHeight()-5);
            x += 5;
            
            AriaRender::color(1, 0.8, 0.7);
            AriaRender::lineWidth(2);
            AriaRender::hollow_rect(x, getHeight()-1, x_before, getHeight()-17);
            
            positionsInDock.push_back(x);
        }//next
        AriaRender::lineWidth(1);
    }
	else
	{
        getCurrentSequence()->dockHeight=0;
    }
    
    
    
    // -------------------------- red line that follows playback, red arrows --------------------------
    if( currentTick!=-1 ) // if playing
	{
        
        RelativeXCoord tick(currentTick, MIDI);
        
        const int XStart = getEditorXStart();
        const int XEnd = getWidth();
        
        AriaRender::lineWidth(2);
        AriaRender::color(0.8, 0, 0);
        
        if(tick.getRelativeTo(WINDOW) < XStart) // current tick is before the visible area
		{
            
            leftArrow=true;
            rightArrow=false;
            
            AriaRender::line(25, measureBarY + 10, 10, measureBarY + 10);
            AriaRender::triangle(5, measureBarY + 10,
                                 15, measureBarY + 5,
                                 15, measureBarY + 15);
        }
		else if(tick.getRelativeTo(WINDOW) > XEnd ) // current tick is after the visible area
		{
            
            leftArrow=false;
            rightArrow=true;
            
            AriaRender::line(XEnd - 15 - 25, measureBarY + 10,
                             XEnd - 15 - 10, measureBarY + 10);
            
            AriaRender::triangle(XEnd - 15 - 5, measureBarY + 10,
                                 XEnd - 15 - 15, measureBarY + 5,
                                 XEnd - 15 - 15, measureBarY + 15);
        }
		else // current tick is inside the visible area
		{
            
            leftArrow=false;
            rightArrow=false;
            
            // red line in measure bar
            AriaRender::line(tick.getRelativeTo(WINDOW), measureBarY + 1,
                             tick.getRelativeTo(WINDOW), measureBarY + 20);
        }
        
        AriaRender::lineWidth(1);
        
    }
	else
	{ // we're not playing, set arrows to false
		leftArrow=false;
		rightArrow=true;
	}
    
    return true;
    
}

#pragma mark -

/*
 * Since instrument picker is shown by MainPane, its events go to MainPane. So, when MainPane catches InstrumentPicker events, it forwards them to it.
 */
void MainPane::instrumentPopupSelected(wxCommandEvent& evt)
{
    Core::getInstrumentPicker()->menuSelected( evt );
}
void MainPane::drumPopupSelected(wxCommandEvent& evt)
{
    Core::getDrumPicker()->menuSelected( evt );
}

#pragma mark -

/*
 * Are key modifiers down on the keyboard?
 */
bool MainPane::isSelectMorePressed(){ return wxGetKeyState(WXK_SHIFT); }
bool MainPane::isSelectLessPressed(){ return wxGetKeyState(WXK_ALT); }

bool MainPane::isCtrlDown(){ return wxGetKeyState(WXK_CONTROL); }
bool MainPane::isMouseDown(){ return isMouseDown_bool; }

/*
 * Gives information about the location of the mouse in a drag
 */
RelativeXCoord MainPane::getMouseX_current()	{	return mousex_current;	}
int MainPane::getMouseY_current()				{	return mousey_current;	}
RelativeXCoord MainPane::getMouseX_initial()	{	return mousex_initial;	}
int MainPane::getMouseY_initial()				{	return mousey_initial;	}


/*
 * Events will be sent regularly to this method when user holds down mouse
 */

void MainPane::mouseHeldDown()
{

    // ----------------------------------- click is in track area ----------------------------

	// check click is within track area
    if(mousey_current < getHeight()-getCurrentSequence()->dockHeight and
       mousey_current > measureBarY+getMeasureBar()->getMeasureBarHeight())
	{

        // dispatch event to sequence
		getCurrentSequence()->mouseHeldDown(mousex_current, mousey_current,
													   mousex_initial, mousey_initial);

    }// end if not on dock

}

/*
 * Event sent whenever user right-clicks on OpenGL pane where everything is drawn.
 */

void MainPane::rightClick(wxMouseEvent& event)
{
	const int measureBarHeight = getMeasureBar()->getMeasureBarHeight();

    Display::requestFocus();

	// check click is not on dock before passing event to tracks
	// dispatch event to all tracks (stop when either of them uses it)
    if(event.GetY() < getHeight()-getCurrentSequence()->dockHeight and
	   event.GetY() > measureBarY+measureBarHeight)
	{
		for(int n=0; n<getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!getCurrentSequence()->getTrack(n)->graphics->processRightMouseClick( RelativeXCoord(event.GetX(),WINDOW) , event.GetY()))
			{
                getCurrentSequence()->setCurrentTrackID(n);
                break;
            }
        }
    }

	// ----------------------------------- click is in measure bar ----------------------------
    if(event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
	{
		getMeasureBar()->rightClick(event.GetX(), event.GetY() - measureBarY);
	}

    Display::render();
}

/*
 * Event sent whenever user clicks on OpenGL pane where everything is drawn.
 */
void MainPane::mouseDown(wxMouseEvent& event)
{
	invalidateMouseEvents = false;
    Display::requestFocus();

    mousex_current.setValue(event.GetX(), WINDOW);
    mousey_current=event.GetY();

    mousex_initial.setValue(event.GetX(), WINDOW);
	mousex_initial.convertTo(MIDI); // we know scrolling may change so better keep it as midi coords
    mousey_initial = mousey_current;

    isMouseDown_bool=true;

	int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
    // ----------------------------------- click is in track area ----------------------------
	// check click is within track area
    if(mousey_current < getHeight()-getCurrentSequence()->dockHeight and
       event.GetY() > measureBarY+measureBarHeight)
	{
        // check if user is moving tracks order
        for(int n=0; n<getCurrentSequence()->getTrackAmount(); n++)
		{
            const int y = getCurrentSequence()->getTrack(n)->graphics->getCurrentEditor()->getTrackYStart();

            if(!getCurrentSequence()->getTrack(n)->graphics->docked and mousey_current>y and mousey_current<y+7)
			{
                draggingTrack = n;
            }

        }

        // dispatch event to all tracks (stop when either of them uses it)
        for(int n=0; n<getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!getCurrentSequence()->getTrack(n)->graphics->processMouseClick( mousex_current, event.GetY()))
                break;
        }
    }// end if not on dock

    // ----------------------------------- click is in dock ----------------------------
    if(event.GetY() > getHeight()-getCurrentSequence()->dockHeight)
	{
        assertExpr( (int)positionsInDock.size()/2 ,==,(int)getCurrentSequence()->dock.size());

        for(unsigned int n=0; n<positionsInDock.size(); n+=2)
		{

            if(event.GetX()>positionsInDock[n] and event.GetX()<positionsInDock[n+1])
			{
                getCurrentSequence()->dock[n/2].docked = false;
                getCurrentSequence()->removeFromDock( &getCurrentSequence()->dock[n/2] );
                DisplayFrame::updateVerticalScrollbar();
                return;
            }
        }
     }//end if user is clicking on the dock

    // ----------------------------------- click is in tab bar ----------------------------
    if(!PlatformMidiManager::isPlaying() and event.GetY() > tabBarY and event.GetY() < tabBarY+20)
	{

        int start_at_x = 0;
        for(int n=0; n<getMainFrame()->getSequenceAmount(); n++)
		{

            start_at_x += tab_width+16+16;
            if(event.GetX() < start_at_x)
			{
				//getMeasureBar()->unselect();
				getMainFrame()->setCurrentSequence(n);
                return;
            }
        }//next
    }//end if

    // ----------------------------------- click is in measure bar ----------------------------
    if(event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
	{
		if( ! (currentTick!=-1 and (leftArrow or rightArrow)) ) // ignore when playing
		{
			getMeasureBar()->mouseDown(mousex_current.getRelativeTo(WINDOW), mousey_current - measureBarY);
		}

    }

    Display::render();

	// ask sequence if it is necessary at this point to be notified of mouse held down events. if so, start a timer that will take of it.
    if(getCurrentSequence()->areMouseHeldDownEventsNeeded()) mouseDownTimer->start();

}


/*
 * Event sent whenever user drags mouse on OpenGL pane where everything is drawn.
 */


void MainPane::mouseMoved(wxMouseEvent& event)
{
	if(invalidateMouseEvents) return;

	mousex_current.setValue(event.GetX(),WINDOW);
	//mousex_current.convertTo(MIDI); //!@#$

    mousey_current=event.GetY();

    if(event.Dragging())
	{

		// we are not reordering tracks
        if(draggingTrack==-1)
		{
			// ----------------------------------- click is in track area ----------------------------
            // check click is not on dock before passing event to current track
            if(event.GetY() < getHeight()-getCurrentSequence()->dockHeight)
				getCurrentSequence()->getCurrentTrack()->graphics->processMouseDrag( mousex_current, event.GetY());

			// ----------------------------------- click is in measure bar ----------------------------
			int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
			if(mousey_initial > measureBarY and mousey_initial < measureBarY+measureBarHeight and
			   event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
			{
				getMeasureBar()->mouseDrag(mousex_current.getRelativeTo(WINDOW), mousey_current - measureBarY,
										   mousex_initial.getRelativeTo(WINDOW), mousey_initial - measureBarY);
			}
        }

        Display::render();

    }//end if dragging

}

void MainPane::mouseLeftWindow(wxMouseEvent& event)
{
	// if we are dragging, notify current track that mouse has left the window
	if(isMouseDown_bool)
	{
		getMainFrame()->
		getCurrentSequence()->
		getCurrentTrack()->
		graphics->
		processMouseExited(mousex_current, mousey_current,
						   mousex_initial, mousey_initial);

		invalidateMouseEvents = true; // ignore all mouse events until a new click/drag is begun
	}
}

void MainPane::mouseReleased(wxMouseEvent& event)
{

	isMouseDown_bool=false;
	if(invalidateMouseEvents) return;


    // if releasing after having dragged a track
    if(draggingTrack!=-1)
	{
        getCurrentSequence()->reorderTracks();
        draggingTrack=-1;
    }//end if

	// ----------------------------------- click is in measure bar ----------------------------
	int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
	if(mousey_initial > measureBarY and mousey_initial < measureBarY+measureBarHeight and
	   event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
	{
		// check if user is clicking on red arrow that scrolls to current playback location
        if(leftArrow)
		{
            if(mousex_current.getRelativeTo(WINDOW)>5 and mousex_current.getRelativeTo(WINDOW)<25)
			{
                scrollNowToPlaybackPosition();
            }
        }
        else if(rightArrow)
		{
            if(mousex_current.getRelativeTo(WINDOW)>getWidth()-45 and mousex_current.getRelativeTo(WINDOW)<getWidth()-20)
				scrollNowToPlaybackPosition();
        }

		// measure selection
		if( ! (currentTick!=-1 and (leftArrow or rightArrow)) ) // ignore when playing
		{
			getMeasureBar()->mouseUp(mousex_current.getRelativeTo(WINDOW), mousey_current - measureBarY,
									 mousex_initial.getRelativeTo(WINDOW), mousey_initial - measureBarY);
		}
	}
	else
	{
	    // disptach mouse up event to current track
		getCurrentSequence()->getCurrentTrack()->graphics->processMouseRelease();
	}

    Display::render();
}

void MainPane::keyReleased(wxKeyEvent& evt)
{
}

void MainPane::keyPressed(wxKeyEvent& evt)
{

#ifdef __WXMAC__
    const bool commandDown=evt.MetaDown() || evt.ControlDown();
#else
    const bool commandDown= evt.ControlDown();
#endif
    const bool shiftDown=evt.ShiftDown();

    // ---------------- play selected notes -----------------
    if(evt.GetKeyCode() == WXK_SPACE)
	{
        getCurrentSequence()->spacePressed();
    }

    // ---------------- resize notes -----------------
    if(commandDown && !shiftDown)
	{

        if(evt.GetKeyCode()==WXK_LEFT)
		{
			getCurrentSequence()->getCurrentTrack()->
			action( new Action::ResizeNotes(
						-getCurrentSequence()->ticksPerBeat() *
						4 /
						getCurrentSequence()->getCurrentTrack()->getGridDivider() ,
						SELECTED_NOTES)
				   );
            Display::render();
        }

        if(evt.GetKeyCode()==WXK_RIGHT)
		{
			getCurrentSequence()->getCurrentTrack()->
			action( new Action::ResizeNotes(
						getCurrentSequence()->ticksPerBeat() *
						4 /
						getCurrentSequence()->getCurrentTrack()->getGridDivider() ,
						SELECTED_NOTES)
					);
            Display::render();
        }

    }

    // perform editor-specific event filtering

    // FIXME - belongs to the editor, probably
    // FIXME - move all editor stuff to editor files
    // FIXME - too many renders there, maybe even actions do render

    const int current_editor = getCurrentSequence()->getCurrentTrack()->graphics->editorMode;
    if(current_editor == GUITAR)
	{

        // ------------------- numbers -------------------
		// number at the top of the keyboard
        if(evt.GetKeyCode() >= 48 and evt.GetKeyCode() <=57)
		{
            if(shiftDown) getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 48 + 10) );
            else getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 48) );
            Display::render();
        }

		// numpad
        if(evt.GetKeyCode() >= 324 and evt.GetKeyCode() <=333)
		{
            if(shiftDown) getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324 + 10) );
            else getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324) );
            Display::render();
        }

        // ---------------- shift frets -----------------
        if(!commandDown && shiftDown)
		{

            if(evt.GetKeyCode()==WXK_LEFT)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftFrets(-1, SELECTED_NOTES) );
                Display::render();
            }

            if(evt.GetKeyCode()==WXK_RIGHT)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftFrets(1, SELECTED_NOTES) );
                Display::render();
            }

            if(evt.GetKeyCode()==WXK_UP)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftString(-1, SELECTED_NOTES) );
                Display::render();
            }

            if(evt.GetKeyCode()==WXK_DOWN)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftString(1, SELECTED_NOTES) );
                Display::render();
            }

        }
    }

    if(current_editor == SCORE and (commandDown or shiftDown) )
    {
            if(evt.GetKeyCode()==WXK_UP)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftBySemiTone(-1, SELECTED_NOTES) );
                Display::render();
                return;
            }

            if(evt.GetKeyCode()==WXK_DOWN)
			{
                getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftBySemiTone(1, SELECTED_NOTES) );
                Display::render();
                return;
            }

    }

    if( !commandDown and (!shiftDown or current_editor == SCORE) )
	{
        // ---------------- move notes -----------------

        if(evt.GetKeyCode()==WXK_LEFT)
		{
            getCurrentSequence()->getCurrentTrack()->
			action( new Action::MoveNotes(
					  -getCurrentSequence()->ticksPerBeat() * 4 /
					  getCurrentSequence()->getCurrentTrack()->getGridDivider(), 0, SELECTED_NOTES)
					);
            Display::render();
        }

        if(evt.GetKeyCode()==WXK_RIGHT)
		{
            getCurrentSequence()->getCurrentTrack()->
			action( new Action::MoveNotes(
					  getCurrentSequence()->ticksPerBeat() * 4 /
					  getCurrentSequence()->getCurrentTrack()->getGridDivider(), 0, SELECTED_NOTES)
					);
            Display::render();
        }

        if(evt.GetKeyCode()==WXK_UP)
		{
            getCurrentSequence()->getCurrentTrack()->action( new Action::MoveNotes(0,-1,SELECTED_NOTES) );
            Display::render();
        }

        if(evt.GetKeyCode()==WXK_DOWN)
		{
            getCurrentSequence()->getCurrentTrack()->action( new Action::MoveNotes(0,1,SELECTED_NOTES) );
            Display::render();
        }

        // ------------------------ delete notes ---------------------

        if(evt.GetKeyCode()==WXK_BACK || evt.GetKeyCode()==WXK_DELETE)
		{

            getCurrentSequence()->getCurrentTrack()->action( new Action::DeleteSelected() );
            Display::render();
        }


    }//end if command down
}


void MainPane::mouseWheelMoved(wxMouseEvent& event)
{
    // event.GetWheelRotation()/1.5;
    // render();
	const int value = event.GetWheelRotation() / event.GetWheelDelta();
	const int my = event.GetY();
	const int mx = event.GetX();

	const int measureBarHeight = getMeasureBar()->getMeasureBarHeight();

	// ----------------------------------- click is in track area ----------------------------
	// check click is within track area
    if(my < getHeight()-getCurrentSequence()->dockHeight and
       mx > measureBarY+measureBarHeight)
	{

        // dispatch event to all tracks (stop when either of them uses it)
        for(int n=0; n<getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!getCurrentSequence()->getTrack(n)->graphics->mouseWheelMoved(mx, my, value))
                break;
        }
    }// end if not on dock
}


#pragma mark -

/*
 * Returns the ID of the track the user is dragging (in a track reordering process), or -1 if no reoredring is being done
 */
int MainPane::getDraggedTrackID()				{	return draggingTrack;	}


#pragma mark -

void MainPane::enterPlayLoop()
{
    leftArrow = false;
    rightArrow = false;
    followPlaybackTime = getMeasureBar()->defaultMeasureLengthInTicks();
    lastTick = -1;
    Core::activateRenderLoop(true);
}
void MainPane::exitPlayLoop()
{
    PlatformMidiManager::stop();
    getMainFrame()->toolsExitPlaybackMode();
    Core::activateRenderLoop(false);
    setCurrentTick( -1 );
    Display::render();
}

void MainPane::setPlaybackStartTick(int newValue)
{
    playbackStartTick = newValue;
}

/*
 *  This method is called repeatedly during playback
 */
void MainPane::playbackRenderLoop()
{
        const int currentTick = PlatformMidiManager::trackPlaybackProgression();

        // check if song is over
        if(currentTick == -1 or !PlatformMidiManager::isPlaying())
        {
            exitPlayLoop();
            return;
        }

        // only draw if it has changed
        if(lastTick != playbackStartTick + currentTick)
		{

            // if user has clicked on a little red arrow
            if(scrollToPlaybackPosition)
			{
                scrollToPlaybackPosition=false;
                const int x_scroll_in_pixels = (int)( (playbackStartTick + currentTick) *
                    getCurrentSequence()->getZoom() );
                getCurrentSequence()->setXScrollInPixels(x_scroll_in_pixels);
                DisplayFrame::updateHorizontalScrollbar( playbackStartTick + currentTick );
            }

			// if follow playback is checked in the menu
			if(getCurrentSequence()->follow_playback)
			{
                RelativeXCoord tick(playbackStartTick + currentTick, MIDI);
                const int current_pixel = tick.getRelativeTo(WINDOW);
                
                const float zoom = getCurrentSequence()->getZoom();
                const int XStart = getEditorXStart();
                const int XEnd = getWidth() - followPlaybackTime*zoom;
                
                if(current_pixel < XStart or current_pixel > XEnd)
                {
                    int new_scroll_in_pixels = (playbackStartTick + currentTick) * getCurrentSequence()->getZoom();
                    if(new_scroll_in_pixels < 0) new_scroll_in_pixels=0;
                    // FIXME - we need a single call to update both data and widget
                    getCurrentSequence()->setXScrollInPixels(new_scroll_in_pixels);
                    DisplayFrame::updateHorizontalScrollbar( playbackStartTick + currentTick );
                }
                
                /*
				int x_scroll_in_pixels = (int)( (playbackStartTick + currentTick - followPlaybackTime) *
                    getCurrentSequence()->getZoom() );
				if( x_scroll_in_pixels < 0 ) x_scroll_in_pixels = 0;
                getCurrentSequence()->setXScrollInPixels(x_scroll_in_pixels);
                DisplayFrame::updateHorizontalScrollbar( playbackStartTick + currentTick - followPlaybackTime );
                     */
			}

            setCurrentTick( playbackStartTick + currentTick );

            RelativeXCoord tick(this->currentTick, MIDI);
            const int XStart = getEditorXStart();
            const int XEnd = getWidth();
            const int tick_pixel = tick.getRelativeTo(WINDOW);

            if(tick_pixel < XStart and leftArrow)
            {
                // current tick is before the visible area and arrow already there. no need to render again.
            }
            else if(tick_pixel > XEnd and rightArrow)
            {
                // current tick is after the visible area and arrow already there. no need to render again.
            }
            else
            {
                Display::render();
            }
            lastTick = playbackStartTick + currentTick;
        }

        wxMilliSleep(10);
}

// sets a flag that will be picked by the playback loop
void MainPane::scrollNowToPlaybackPosition(){		scrollToPlaybackPosition=true;		}

/*
 * This is called when the song us playing. MainPane needs to know the current tick because when it renders
 * it needs to know where to draw the red line that follows playback.
 */

void MainPane::setCurrentTick(int currentTick)
{
    MainPane::currentTick = currentTick;
}


#pragma mark -

void MainPane::saveToFile(wxFileOutputStream& fileout)
{
    getCurrentSequence()->saveToFile(fileout);
}

}
