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


/*
 * The GLPane is an OpenGL panel. It is where all drawing is done.
 * It also catches all mouse and keybard events issued on track area and it dispatches them to the appropriate object.
 * (e.g.: if user clicks on track, the event is  by GLPane. Then GLPane finds out where the click was issued and transfers the event to the clicked track.
 */

#include "GUI/GLPane.h"
#include "main.h"

#include "wx/wx.h"

#include "OpenGL.h"

#include <iostream>
#include <cmath>

#include "Actions/EditAction.h"
#include "Actions/ResizeNotes.h"
#include "Actions/NumberPressed.h"
#include "Actions/ShiftFrets.h"
#include "Actions/ShiftString.h"
#include "Actions/MoveNotes.h"
#include "Actions/DeleteSelected.h"

#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "GUI/MeasureBar.h"
#include "Images/Drawable.h"
#include "Images/Image.h"
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


#include "Config.h"

namespace AriaMaestosa {


const int tab_width=145;
const int tabBarY = 0;
const int measureBarY = 20;

// when this is set to 'true', the app will wait for a new click to be begun to process any mouse events (i.e. current click/drag is not valid anymore)
bool invalidateMouseEvents=false;

// ==========================================================================================
// ==========================================================================================
class MouseDownTimer : public wxTimer {

    GLPane* glPane;

public:

    MouseDownTimer(GLPane* parent) : wxTimer()
	{
        glPane = parent;
    }

    void Notify()
	{
        if(!glPane->isMouseDown())
		{
			Stop();
			return;
		}
        glPane->mouseHeldDown();

    }

    void start()
	{
        Start(10);
    }
};
// ==========================================================================================
// ==========================================================================================


BEGIN_EVENT_TABLE(GLPane, wxGLCanvas)

EVT_MOTION(GLPane::mouseMoved)
EVT_LEFT_DOWN(GLPane::mouseDown)
EVT_LEFT_UP(GLPane::mouseReleased)
EVT_RIGHT_DOWN(GLPane::rightClick)
EVT_LEAVE_WINDOW(GLPane::mouseLeftWindow)

EVT_SIZE(GLPane::resized)

EVT_KEY_DOWN(GLPane::keyPressed)
EVT_KEY_UP(GLPane::keyReleased)

EVT_MENU_RANGE(0+10000,127+10000, GLPane::instrumentPopupSelected)
EVT_MENU_RANGE(0+20000,127+20000, GLPane::drumPopupSelected)

EVT_MOUSEWHEEL(GLPane::mouseWheelMoved)
EVT_PAINT(GLPane::paintEvent)

END_EVENT_TABLE()

GLPane::GLPane(MainFrame* mainFrame, int* args) :
wxGLCanvas(mainFrame, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"),  args)
{

    INIT_LEAK_CHECK();

    currentTick=-1;
    draggingTrack = -1;
    isVisible=false;
    GLPane::mainFrame=mainFrame;
    isMouseDown_bool=false;

    mousex_initial.setValue(0,MIDI);
    mousey_initial = 0;
    mousex_current.setValue(0,WINDOW);
    mousey_current = 0;

    leftArrow=false;
    rightArrow=false;

    mouseDownTimer = new MouseDownTimer(this);

    int argc = 0;
    char** argv = NULL;
	
	std::cout << "calling glutInit" << std::endl;
	
    scrollToPlaybackPosition=false;

    playbackStartTick=0; // tells from where the red line should start when playing back

    glutInit(&argc, argv);
}

GLPane::~GLPane()
{
	delete mouseDownTimer;
}

/*
 * Since instrument picker is shown by GLPane, its events go to GLPane. So, when GLPane catches InstrumentPicker events, it forwards them to it.
 */
void GLPane::instrumentPopupSelected(wxCommandEvent& evt)
{
	mainFrame->instrument_picker->menuSelected( evt );
}
void GLPane::drumPopupSelected(wxCommandEvent& evt)
{
	mainFrame->drumKit_picker->menuSelected( evt );
}


void GLPane::resized(wxSizeEvent& evt)
{

    wxGLCanvas::OnSize(evt);

    initOpenGLFor2D();
    render();

    Refresh();

    // update vertical scrollbar
    if(mainFrame->getSequenceAmount()>0) mainFrame->updateTopBarForSequence( mainFrame->getCurrentSequence() );
}

/*
 * This is called when the song us playing. GLPane needs to know the current tick because when it renders
 * it needs to know where to draw the red line that follows playback.
 */

void GLPane::setCurrentTick(int currentTick)
{
    GLPane::currentTick = currentTick;
}

/*
 * Are key modifiers down on the keyboard?
 */
bool GLPane::isSelectMorePressed(){ return wxGetKeyState(WXK_SHIFT); }
bool GLPane::isSelectLessPressed(){ return wxGetKeyState(WXK_ALT); }

bool GLPane::isCtrlDown(){ return wxGetKeyState(WXK_CONTROL); }
bool GLPane::isMouseDown(){ return isMouseDown_bool; }

/*
 * Returns the ID of the track the user is dragging (in a track reordering process), or -1 if no reoredring is being done
 */
int GLPane::getDraggedTrackID()				{	return draggingTrack;	}

/*
 * Gives information about the location of the mouse in a drag
 */
RelativeXCoord GLPane::getMouseX_current()	{	return mousex_current;	}
int GLPane::getMouseY_current()				{	return mousey_current;	}
RelativeXCoord GLPane::getMouseX_initial()	{	return mousex_initial;	}
int GLPane::getMouseY_initial()				{	return mousey_initial;	}

/*
 * Called when frame has just been made visible. Does things that can't be done without display.
 */

void GLPane::isNowVisible()
{
	mainFrame->addSequence();
    mainFrame->getCurrentSequence()->addTrack();
    isVisible=true;
}

/*
 * Events will be sent regularly to this method when user holds down mouse
 */

void GLPane::mouseHeldDown()
{

    // ----------------------------------- click is in track area ----------------------------

	// check click is within track area
    if(mousey_current < getHeight()-mainFrame->getCurrentSequence()->dockHeight and
       mousey_current > measureBarY+getMeasureBar()->getMeasureBarHeight())
	{

        // dispatch event to sequence
		mainFrame->getCurrentSequence()->mouseHeldDown(mousex_current, mousey_current,
													   mousex_initial, mousey_initial);

    }// end if not on dock

}

/*
 * Event sent whenever user right-clicks on OpenGL pane where everything is drawn.
 */

void GLPane::rightClick(wxMouseEvent& event)
{
	const int measureBarHeight = getMeasureBar()->getMeasureBarHeight();

    SetFocus();

	// check click is not on dock before passing event to tracks
	// dispatch event to all tracks (stop when either of them uses it)
    if(event.GetY() < getHeight()-mainFrame->getCurrentSequence()->dockHeight and
	   event.GetY() > measureBarY+measureBarHeight)
	{
		for(int n=0; n<mainFrame->getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!mainFrame->getCurrentSequence()->getTrack(n)->graphics->processRightMouseClick( RelativeXCoord(event.GetX(),WINDOW) , event.GetY()))
			{
                mainFrame->getCurrentSequence()->setCurrentTrackID(n);
                break;
            }
        }
    }

	// ----------------------------------- click is in measure bar ----------------------------
    if(event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
	{
		getMeasureBar()->rightClick(event.GetX(), event.GetY() - measureBarY);
	}

    render();
}

/*
 * Event sent whenever user clicks on OpenGL pane where everything is drawn.
 */
void GLPane::mouseDown(wxMouseEvent& event)
{
	invalidateMouseEvents = false;
    SetFocus();

    mousex_current.setValue(event.GetX(), WINDOW);
    mousey_current=event.GetY();

    mousex_initial.setValue(event.GetX(), WINDOW);
	mousex_initial.convertTo(MIDI); // we know scrolling may change so better keep it as midi coords
    mousey_initial = mousey_current;

    isMouseDown_bool=true;

	int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
    // ----------------------------------- click is in track area ----------------------------
	// check click is within track area
    if(mousey_current < getHeight()-mainFrame->getCurrentSequence()->dockHeight and
       event.GetY() > measureBarY+measureBarHeight)
	{
        // check if user is moving tracks order
        for(int n=0; n<mainFrame->getCurrentSequence()->getTrackAmount(); n++)
		{
            const int y = mainFrame->getCurrentSequence()->getTrack(n)->graphics->getCurrentEditor()->getTrackYStart();

            if(!mainFrame->getCurrentSequence()->getTrack(n)->graphics->docked and mousey_current>y and mousey_current<y+7)
			{
                draggingTrack = n;
            }

        }

        // dispatch event to all tracks (stop when either of them uses it)
        for(int n=0; n<mainFrame->getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!mainFrame->getCurrentSequence()->getTrack(n)->graphics->processMouseClick( mousex_current, event.GetY()))
                break;
        }
    }// end if not on dock

    // ----------------------------------- click is in dock ----------------------------
    if(event.GetY() > getHeight()-mainFrame->getCurrentSequence()->dockHeight)
	{
        assertExpr( (int)positionsInDock.size()/2 ,==,(int)mainFrame->getCurrentSequence()->dock.size());

        for(unsigned int n=0; n<positionsInDock.size(); n+=2)
		{

            if(event.GetX()>positionsInDock[n] and event.GetX()<positionsInDock[n+1])
			{
                mainFrame->getCurrentSequence()->dock[n/2].docked = false;
                mainFrame->getCurrentSequence()->removeFromDock( &mainFrame->getCurrentSequence()->dock[n/2] );
                mainFrame->updateVerticalScrollbar();
                return;
            }
        }
     }//end if user is clicking on the dock

    // ----------------------------------- click is in tab bar ----------------------------
    if(!PlatformMidiManager::isPlaying() and event.GetY() > tabBarY and event.GetY() < tabBarY+20)
	{

        int start_at_x = 0;
        for(int n=0; n<mainFrame->getSequenceAmount(); n++)
		{

            start_at_x += tab_width+16+16;
            if(event.GetX() < start_at_x)
			{
				//getMeasureBar()->unselect();
				mainFrame->setCurrentSequence(n);
                return;
            }
        }//next
    }//end if

    // ----------------------------------- click is in measure bar ----------------------------
	if(wxGetKeyState(WXK_F1)) std::cout << "A" << std::endl;
    if(event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
	{
        if(wxGetKeyState(WXK_F1)) std::cout << "B" << std::endl;
		if( ! (currentTick!=-1 and (leftArrow or rightArrow)) ) // ignore when playing
		{
			if(wxGetKeyState(WXK_F1)) std::cout << "C" << std::endl;
			getMeasureBar()->mouseDown(mousex_current.getRelativeTo(WINDOW), mousey_current - measureBarY);
		}

    }

    render();

	// ask sequence if it is necessary at this point to be notified of mouse held down events. if so, start a timer that will take of it.
    if(mainFrame->getCurrentSequence()->areMouseHeldDownEventsNeeded()) mouseDownTimer->start();

}


/*
 * Event sent whenever user drags mouse on OpenGL pane where everything is drawn.
 */


void GLPane::mouseMoved(wxMouseEvent& event)
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
            if(event.GetY() < getHeight()-mainFrame->getCurrentSequence()->dockHeight)
				mainFrame->getCurrentSequence()->getCurrentTrack()->graphics->processMouseDrag( mousex_current, event.GetY());

			// ----------------------------------- click is in measure bar ----------------------------
			int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
			if(mousey_initial > measureBarY and mousey_initial < measureBarY+measureBarHeight and
			   event.GetY() > measureBarY and event.GetY() < measureBarY+measureBarHeight)
			{
				getMeasureBar()->mouseDrag(mousex_current.getRelativeTo(WINDOW), mousey_current - measureBarY,
										   mousex_initial.getRelativeTo(WINDOW), mousey_initial - measureBarY);
			}
        }

        render();

    }//end if dragging

}

void GLPane::mouseLeftWindow(wxMouseEvent& event)
{
	// if we are dragging, notify current track that mouse has left the window
	if(isMouseDown_bool)
	{
		mainFrame->
		getCurrentSequence()->
		getCurrentTrack()->
		graphics->
		processMouseExited(mousex_current, mousey_current,
						   mousex_initial, mousey_initial);

		invalidateMouseEvents = true; // ignore all mouse events until a new click/drag is begun
	}
}

void GLPane::mouseReleased(wxMouseEvent& event)
{

	isMouseDown_bool=false;
	if(invalidateMouseEvents) return;


    // if releasing after having dragged a track
    if(draggingTrack!=-1)
	{
        mainFrame->getCurrentSequence()->reorderTracks();
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
		mainFrame->getCurrentSequence()->getCurrentTrack()->graphics->processMouseRelease();
	}

    render();
}

void GLPane::keyReleased(wxKeyEvent& evt)
{
}

void GLPane::keyPressed(wxKeyEvent& evt)
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
        mainFrame->getCurrentSequence()->spacePressed();
    }

    // ---------------- resize notes -----------------
    if(commandDown && !shiftDown)
	{

        if(evt.GetKeyCode()==WXK_LEFT)
		{
			mainFrame->getCurrentSequence()->getCurrentTrack()->
			action( new Action::ResizeNotes(
						-mainFrame->getCurrentSequence()->ticksPerBeat() *
						4 /
						mainFrame->getCurrentSequence()->getCurrentTrack()->getGridDivider() ,
						SELECTED_NOTES)
				   );
            render();
        }

        if(evt.GetKeyCode()==WXK_RIGHT)
		{
			mainFrame->getCurrentSequence()->getCurrentTrack()->
			action( new Action::ResizeNotes(
						mainFrame->getCurrentSequence()->ticksPerBeat() *
						4 /
						mainFrame->getCurrentSequence()->getCurrentTrack()->getGridDivider() ,
						SELECTED_NOTES)
					);
            render();
        }

    }

	// FIXME - belongs to the guitar editor, probably
    if(mainFrame->getCurrentSequence()->getCurrentTrack()->graphics->editorMode == GUITAR)
	{

        // ------------------- numbers -------------------
		// number at the top of the keyboard
        if(evt.GetKeyCode() >= 48 and evt.GetKeyCode() <=57)
		{
            if(shiftDown) mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 48 + 10) );
            else mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 48) );
            render();
        }

		// numpad
        if(evt.GetKeyCode() >= 324 and evt.GetKeyCode() <=333)
		{
            if(shiftDown) mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324 + 10) );
            else mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::NumberPressed(evt.GetKeyCode() - 324) );
            render();
        }

        // ---------------- shift frets -----------------
        if(!commandDown && shiftDown)
		{

            if(evt.GetKeyCode()==WXK_LEFT)
			{
                mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftFrets(-1, SELECTED_NOTES) );
                render();
            }

            if(evt.GetKeyCode()==WXK_RIGHT)
			{
                mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftFrets(1, SELECTED_NOTES) );
                render();
            }

            if(evt.GetKeyCode()==WXK_UP)
			{
                mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftString(-1, SELECTED_NOTES) );
                render();
            }

            if(evt.GetKeyCode()==WXK_DOWN)
			{
                mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::ShiftString(1, SELECTED_NOTES) );
                render();
            }

        }
    }

	// FIXME - move all editor stuff to editor files
    if( !commandDown and (!shiftDown or mainFrame->getCurrentSequence()->getCurrentTrack()->graphics->editorMode == SCORE) )
	{
        // ---------------- move notes -----------------

        if(evt.GetKeyCode()==WXK_LEFT)
		{
            mainFrame->getCurrentSequence()->getCurrentTrack()->
			action( new Action::MoveNotes(
					  -mainFrame->getCurrentSequence()->ticksPerBeat() *
					  4 /
					  mainFrame->getCurrentSequence()->getCurrentTrack()->getGridDivider(), 0, SELECTED_NOTES)
					);
            render();
        }

        if(evt.GetKeyCode()==WXK_RIGHT)
		{
            mainFrame->getCurrentSequence()->getCurrentTrack()->
			action( new Action::MoveNotes(
					  mainFrame->getCurrentSequence()->ticksPerBeat() *
					  4 /
					  mainFrame->getCurrentSequence()->getCurrentTrack()->getGridDivider(), 0, SELECTED_NOTES)
					);
            render();
        }

        if(evt.GetKeyCode()==WXK_UP)
		{
            mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::MoveNotes(0,-1,SELECTED_NOTES) );
            render();
        }

        if(evt.GetKeyCode()==WXK_DOWN)
		{
            mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::MoveNotes(0,1,SELECTED_NOTES) );
            render();
        }

        // ------------------------ delete notes ---------------------

        if(evt.GetKeyCode()==WXK_BACK || evt.GetKeyCode()==WXK_DELETE)
		{

            mainFrame->getCurrentSequence()->getCurrentTrack()->action( new Action::DeleteSelected() );
            render();
        }


    }//end if command down
}


void GLPane::mouseWheelMoved(wxMouseEvent& event)
{
    // event.GetWheelRotation()/1.5;
    // render();
	const int value = event.GetWheelRotation() / event.GetWheelDelta();
	const int my = event.GetY();
	const int mx = event.GetX();
	
	const int measureBarHeight = getMeasureBar()->getMeasureBarHeight();
	
	// ----------------------------------- click is in track area ----------------------------
	// check click is within track area
    if(my < getHeight()-mainFrame->getCurrentSequence()->dockHeight and
       mx > measureBarY+measureBarHeight)
	{

        // dispatch event to all tracks (stop when either of them uses it)
        for(int n=0; n<mainFrame->getCurrentSequence()->getTrackAmount(); n++)
		{
            if(!mainFrame->getCurrentSequence()->getTrack(n)->graphics->mouseWheelMoved(mx, my, value))
                break;
        }
    }// end if not on dock
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

    if(isVisible) return GetSize().x;
    else return 795; // default value
}

int GLPane::getHeight()
{
    if(isVisible) return GetSize().y;
    else return 550; // approximately default
}


void GLPane::paintEvent(wxPaintEvent& evt)
{
    render(true);
}

void GLPane::render(bool paintEvent)
{

	if (!GetParent()->IsShown()) return; 
    //if(!isVisible) return;
    if(!ImageProvider::imagesLoaded()) return;
    if(mainFrame->getCurrentSequence()->importing) return;

    wxGLCanvas::SetCurrent();
    if(paintEvent) wxPaintDC(this);
    else wxClientDC(this);
    
    initOpenGLFor2D();
    glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);


    glEnable(GL_TEXTURE_2D);

    mainFrame->getCurrentSequence()->renderTracks(
												  currentTick, mousex_current,
												  mousey_current, mousey_initial,
												  25 + getMeasureBar()->getMeasureBarHeight());

    glLoadIdentity();

    // -------------------------- draw tab bar at top -------------------------
    // beige background

    glDisable(GL_TEXTURE_2D);
    glColor3f(1, 1, 0.9);
    glBegin(GL_QUADS);
    glVertex2f(0, tabBarY);
    glVertex2f(getWidth(), tabBarY);
    glVertex2f(getWidth(), tabBarY+20);
    glVertex2f(0, tabBarY+20);
    glEnd();

    // draw tab
    int start_at_x = 0;
    for(int n=0; n<mainFrame->getSequenceAmount(); n++)
	{
        glEnable(GL_TEXTURE_2D);

        if(mainFrame->getCurrentSequenceID() == n)
		{
            glColor3f(1,1,1);
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
            glEnable(GL_BLEND);
            glColor4f(1,1,1, 0.4);

            tabBorderDrawable->move(start_at_x, tabBarY+3);
            tabBorderDrawable->setFlip(false, false);
            tabBorderDrawable->render();

            tabDrawable->move(start_at_x+16, tabBarY+3);
            tabDrawable->scale(tab_width/2.0, 1);
            tabDrawable->render();

            tabBorderDrawable->move(start_at_x+16+tab_width, tabBarY+3);
            tabBorderDrawable->setFlip(true, false);
            tabBorderDrawable->render();
            glDisable(GL_BLEND);
        }
        glLoadIdentity();

        glDisable(GL_TEXTURE_2D);

        // draw tab name
        if(mainFrame->getCurrentSequenceID() == n)
		{
            glColor3f(0,0,0);
            glRasterPos2f(start_at_x+10, tabBarY+18);
        }
		else
		{
            glColor3f(0.4, 0.4, 0.4);
            glRasterPos2f(start_at_x+10, tabBarY+18);
        }

        for(unsigned int i=0; i<mainFrame->getSequence(n)->sequenceFileName.size(); i++)
		{
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, mainFrame->getSequence(n)->sequenceFileName[i]);

            // find where text ends, to truncate too long names
            float rasterPos[4];
            glGetFloatv(GL_CURRENT_RASTER_POSITION,rasterPos);

            if(rasterPos[0]>start_at_x+tab_width+12)
			{
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, '.');
                break;
            }
        }//next

        start_at_x += tab_width+16+16;
    }//next

    // -------------------------- draw measure top bar -------------------------

	getMeasureBar()->render(measureBarY);

    // -------------------------- draw dock -------------------------

    if(mainFrame->getCurrentSequence()->dockSize>0)
	{
        mainFrame->getCurrentSequence()->dockHeight = 20;


        glDisable(GL_TEXTURE_2D);
        glColor3f(1, 1, 0.9);
        glBegin(GL_QUADS);
        glVertex2f(0,getHeight());
        glVertex2f(getWidth(),getHeight());
        glVertex2f(getWidth(),getHeight()-mainFrame->getCurrentSequence()->dockHeight);
        glVertex2f(0,getHeight()-mainFrame->getCurrentSequence()->dockHeight);
        glEnd();

        // black line at the top and bottom
        glColor3f(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(0, getHeight()-mainFrame->getCurrentSequence()->dockHeight);
        glVertex2f(getWidth(), getHeight()-mainFrame->getCurrentSequence()->dockHeight);
        glEnd();

        glColor3f(0,0,0);
        glLoadIdentity();
        int x=10;
        glDisable(GL_TEXTURE_2D);

        int x_before = 0;

        positionsInDock.clear();

        for(int n=0; n<mainFrame->getCurrentSequence()->dockSize; n++)
		{

            positionsInDock.push_back(x_before);

            x_before = x;

            glColor3f(0,0,0);
            glRasterPos2f(x+5, getHeight()-5);

            for(unsigned int i=0; i<mainFrame->getCurrentSequence()->dock[n].track->getName().size(); i++)
			{
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, mainFrame->getCurrentSequence()->dock[n].track->getName()[i]);
            }

            // find where text ends
            float rasterPos[4];
            glGetFloatv(GL_CURRENT_RASTER_POSITION,rasterPos);
            x = (int)(
                      rasterPos[0] + 5
                      );

            glColor3f(1, 0.8, 0.7);
            glLineWidth(2);
            glBegin(GL_LINES);
            glVertex2f(x, getHeight()-17);
            glVertex2f(x, getHeight()-1);

            glVertex2f(x_before, getHeight()-1);
            glVertex2f(x_before, getHeight()-17);

            glVertex2f(x, getHeight()-17);
            glVertex2f(x_before, getHeight()-17);

            glVertex2f(x, getHeight()-1);
            glVertex2f(x_before, getHeight()-1);
            glEnd();

            positionsInDock.push_back(x);
        }//next
        glLineWidth(1);
    }
	else
	{
        mainFrame->getCurrentSequence()->dockHeight=0;
    }



    // -------------------------- red line that follows playback, red arrows --------------------------
    if( currentTick!=-1 ) // if playing
	{

        RelativeXCoord tick(currentTick, MIDI);

        const int XStart = getEditorXStart();
        const int XEnd = getWidth();

        glLineWidth(2);
        glColor3f(0.8, 0, 0);

        if(tick.getRelativeTo(WINDOW) < XStart) // current tick is before the visible area
		{

            leftArrow=true;
            rightArrow=false;

            glBegin(GL_LINES);
            glVertex2f( 25, measureBarY + 10);
            glVertex2f( 10, measureBarY + 10);
            glEnd();

            glBegin(GL_TRIANGLE_STRIP);
            glVertex2f( 5, measureBarY + 10);
            glVertex2f( 15, measureBarY + 5);
            glVertex2f( 15, measureBarY + 15);
            glEnd();

        }
		else if(tick.getRelativeTo(WINDOW) > XEnd ) // current tick is after the visible area
		{

            leftArrow=false;
            rightArrow=true;

            glBegin(GL_LINES);
            glVertex2f(XEnd - 15 - 25, measureBarY + 10);
            glVertex2f(XEnd - 15 - 10, measureBarY + 10);
            glEnd();

            glBegin(GL_TRIANGLE_STRIP);
            glVertex2f(XEnd - 15 - 5, measureBarY + 10);
            glVertex2f(XEnd - 15 - 15, measureBarY + 5);
            glVertex2f(XEnd - 15 - 15, measureBarY + 15);
            glEnd();

        }
		else // current tick is inside the visible area
		{

            leftArrow=false;
            rightArrow=false;

            // red line in measure bar
            glBegin(GL_LINES);
            glVertex2f(tick.getRelativeTo(WINDOW), measureBarY + 1);
            glVertex2f(tick.getRelativeTo(WINDOW), measureBarY + 20);
            glEnd();

        }

        glLineWidth(1);

    }
	else
	{ // we're not playing, set arrows to false
		leftArrow=false;
		rightArrow=true;
	}

    glFlush();
    SwapBuffers();
}

// ----------------------- i/o -------------------------

void GLPane::saveToFile(wxFileOutputStream& fileout)
{
    mainFrame->getCurrentSequence()->saveToFile(fileout);
}

void GLPane::enterPlayLoop()
{
    leftArrow = false;
    rightArrow = false;
    timeBeforeFollowingPlayback = getMeasureBar()->defaultMeasureLengthInTicks();
    lastTick = -1;
    activateRenderLoop(true);
}
void GLPane::setPlaybackStartTick(int newValue)
{
    playbackStartTick = newValue;
}
void GLPane::playbackRenderLoop()
{
        const int currentTick = PlatformMidiManager::trackPlaybackProgression();

        // check if song is over
        if(currentTick == -1 or !PlatformMidiManager::isPlaying())
        {
            exitPlayLoop();
            return;
        }
        
        if(lastTick != playbackStartTick + currentTick)
		{ // only draw if it has changed
            
            // if user has clicked on a little red arrow
            if(scrollToPlaybackPosition)
			{
                scrollToPlaybackPosition=false;
                const int x_scroll_in_pixels = (int)( (playbackStartTick + currentTick) *
                    getMainFrame()->getCurrentSequence()->getZoom() );
                getMainFrame()->getCurrentSequence()->setXScrollInPixels(x_scroll_in_pixels);
                getMainFrame()->updateHorizontalScrollbar( playbackStartTick + currentTick );
            }
            
			// if follow playback is checked in the menu
			if(getMainFrame()->getCurrentSequence()->follow_playback)
			{
				int x_scroll_in_pixels = (int)( (playbackStartTick + currentTick - timeBeforeFollowingPlayback) *
                    getMainFrame()->getCurrentSequence()->getZoom() );
				if( x_scroll_in_pixels < 0 ) x_scroll_in_pixels = 0;
                getMainFrame()->getCurrentSequence()->setXScrollInPixels(x_scroll_in_pixels);
				getMainFrame()->updateHorizontalScrollbar( playbackStartTick + currentTick - timeBeforeFollowingPlayback );
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
                render();
            }
            lastTick = playbackStartTick + currentTick;
        }

        //wxYield();
        wxMilliSleep(10);
}

void GLPane::exitPlayLoop()
{
    PlatformMidiManager::stop();
    getMainFrame()->toolsExitPlaybackMode();
    activateRenderLoop(false);
    setCurrentTick( -1 );
    render();
}
void GLPane::scrollNowToPlaybackPosition(){		scrollToPlaybackPosition=true;		}


}
