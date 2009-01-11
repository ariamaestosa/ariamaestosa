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
 * The graphical part of a track (the data being held in Track)
 */

#include <iostream>
#include <wx/numdlg.h>

#include "Config.h"

#include "AriaCore.h"
#include "Actions/SetAccidentalSign.h"
#include "Pickers/InstrumentChoice.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/MagneticGrid.h"
#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "GUI/ImageProvider.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/MeasureData.h"
#include "Editors/KeyboardEditor.h"
#include "Editors/Editor.h"
#include "Editors/GuitarEditor.h"
#include "Editors/DrumEditor.h"
#include "Editors/RelativeXCoord.h"
#include "Editors/ControllerEditor.h"
#include "Editors/ScoreEditor.h"
#include "GUI/GraphicalTrack.h"
#include "Renderers/RenderAPI.h"
#include "IO/IOUtils.h"

namespace AriaMaestosa {


class AriaWidget
{
protected:
    int x, y, width;
    bool hidden;
public:
    LEAK_CHECK(AriaWidget);

    AriaWidget(int width){ AriaWidget::x = x; AriaWidget::width = width; hidden = false;}
    int getX(){ return x; }
    int getY(){ return y; }
    int getWidth(){ return width; }

    bool isHidden(){ return hidden; }
    void show(bool shown){ hidden = !shown; }

    // don't call this, let WidgetLayoutManager do it
    void setX(const int x){ AriaWidget::x = x; }
    void setY(const int y){ AriaWidget::y = y; }

    bool clickIsOnThisWidget(const int mx, const int my)
    {
        return (not hidden) and ( mx > x and my > y and mx < x+width and my < y+30);
    }

    virtual void render(){}
    virtual ~AriaWidget(){}
};

class BlankField : public AriaWidget
{
public:
    BlankField(int width) : AriaWidget(width){}
    virtual ~BlankField(){}

    void render()
    {
        if(hidden) return;
        comboBorderDrawable->move(x, y+7);
        comboBorderDrawable->setFlip(false, false);
        comboBorderDrawable->render();

        comboBodyDrawable->move(x + 14, y+7);
        comboBodyDrawable->scale((width-28)/4.0 , 1);
        comboBodyDrawable->render();

        comboBorderDrawable->move(x + width - 14, y+7 );
        comboBorderDrawable->setFlip(true,false);
        comboBorderDrawable->render();
    }
};

class ComboBox : public AriaWidget
{
public:
    ComboBox(int width) : AriaWidget(width){}
    virtual ~ComboBox(){}

    void render()
    {
        if(hidden) return;
        comboBorderDrawable->move(x, y+7);
        comboBorderDrawable->setFlip(false, false);
        comboBorderDrawable->render();

        comboBodyDrawable->move(x+14, y+7);
        comboBodyDrawable->scale((width-28-18)/4.0, 1);
        comboBodyDrawable->render();

        comboSelectDrawable->move(x+width-14-18, y+7);
        comboSelectDrawable->render();
    }
};

class BitmapButton : public AriaWidget
{
    int y_offset;
    bool enabled;
    bool toggleBtn;
    bool centerX;
    AriaRender::ImageState state;
public:
    Drawable* drawable;

    BitmapButton(int width, int y_offset, Drawable* drawable, bool toggleBtn=false, bool centerX=false) : AriaWidget(width)
    {
        BitmapButton::drawable = drawable;
        BitmapButton::y_offset = y_offset;
        enabled = true;
        BitmapButton::toggleBtn = toggleBtn;
        BitmapButton::centerX = centerX;
        state = AriaRender::STATE_NORMAL;
    }
    virtual ~BitmapButton(){}

    BitmapButton* setImageState(AriaRender::ImageState state)
    {
        BitmapButton::state = state;
        return this;
    }

    void render()
    {
        if(hidden) return;

        if(state != AriaRender::STATE_NORMAL)
            AriaRender::setImageState(state);
        else if(not enabled)
            AriaRender::setImageState(AriaRender::STATE_DISABLED);

        if(centerX and drawable->image->width < width)
        {
            const int ajust = (width - drawable->image->width)/2;
            drawable->move(x + drawable->hotspotX + ajust, y+y_offset);
        }
        else
            drawable->move(x + drawable->hotspotX, y+y_offset);

        drawable->render();
    }

    void enable(const bool enabled)
    {
        BitmapButton::enabled = enabled;
    }
};

template<typename PARENT>
class ToolBar : public PARENT
{
    ptr_vector<BitmapButton, HOLD> contents;
    std::vector<int> margin;
public:
    ToolBar() : PARENT(22)
    {
    }
    void addItem(BitmapButton* btn, int margin_after)
    {
        contents.push_back(btn);
        margin.push_back(margin_after);
    }
    void layout()
    {
        if(PARENT::hidden) return;
        PARENT::width = 22;
        int currentX = PARENT::x + 11;

        const int amount = contents.size();
        for(int n=0; n<amount; n++)
        {
            contents[n].setX(currentX);
            contents[n].setY(PARENT::y);

            currentX += contents[n].getWidth() + margin[n];
            PARENT::width += contents[n].getWidth() + margin[n];
        }
    }

    BitmapButton& getItem(const int item)
    {
        return contents[item];
    }

    void render()
    {
        if(PARENT::hidden) return;

        // render background
        PARENT::render();

        // render buttons
        const int amount = contents.size();

        for(int n=0; n<amount; n++)
        {
            contents[n].render();
        }
        AriaRender::setImageState(AriaRender::STATE_NORMAL);
    }
};

class WidgetLayoutManager
{
    ptr_vector<AriaWidget, HOLD> widgetsLeft;
    ptr_vector<AriaWidget, HOLD> widgetsRight;
public:
    LEAK_CHECK(WidgetLayoutManager);

    WidgetLayoutManager()
    {
    }
    void addFromLeft(AriaWidget* w)
    {
        widgetsLeft.push_back(w);
    }
    void addFromRight(AriaWidget* w)
    {
        widgetsRight.push_back(w);
    }
    void layout(const int x_origin, const int y_origin)
    {
        const int lamount = widgetsLeft.size();
        int lx = x_origin;
        for(int n=0; n<lamount; n++)
        {
            widgetsLeft[n].setX(lx);
            widgetsLeft[n].setY(y_origin);
            lx += widgetsLeft[n].getWidth();
        }

        const int ramount = widgetsRight.size();
        int rx = Display::getWidth() - 17;
        for(int n=0; n<ramount; n++)
        {
            rx -= widgetsRight[n].getWidth();
            widgetsRight[n].setX(rx);
            widgetsRight[n].setY(y_origin);
        }
    }
    void renderAll(bool focus)
    {
        AriaRender::images();

        const int lamount = widgetsLeft.size();
        for(int n=0; n<lamount; n++)
        {
            if(!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
            else AriaRender::setImageState(AriaRender::STATE_NORMAL);

            widgetsLeft.get(n)->render();
        }

        const int ramount = widgetsRight.size();
        for(int n=0; n<ramount; n++)
        {
            if(!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
            else AriaRender::setImageState(AriaRender::STATE_NORMAL);

            widgetsRight.get(n)->render();
        }
    }
};

#if 0
#pragma mark -
#endif
// ------------------------------------------------------------------------

const int EXPANDED_BAR_HEIGHT = 20;
const int COLLAPSED_BAR_HEIGHT = 5;

GraphicalTrack::GraphicalTrack(Track* track, Sequence* seq)
{
    keyboardEditor     = NULL;
    guitarEditor       = NULL;
    drumEditor         = NULL;
    controllerEditor   = NULL;
    scoreEditor        = NULL;

    sequence = seq;
    GraphicalTrack::track = track;

    assert(track);

    grid = new MagneticGrid(this);

    lastMouseY=0;

    collapsed=false;
    dragging_resize=false;
    muted=false;
    docked = false;
    editorMode=KEYBOARD;

    height=128;

    // create widgets
    components = new WidgetLayoutManager();

    collapseButton = new BitmapButton(28, 15, collapseDrawable);
    components->addFromLeft(collapseButton);

    muteButton = new BitmapButton(28, 10, muteDrawable);
    components->addFromLeft(muteButton);

    dockToolBar = new ToolBar<BlankField>();
    dockToolBar->addItem( new BitmapButton( 16, 14, maximizeTrackDrawable,false, false), 0 );
    dockToolBar->addItem( new BitmapButton( 16, 14, dockTrackDrawable,    false, false), 0 );
    components->addFromLeft(dockToolBar);
    dockToolBar->layout();

    trackName = new BlankField(140);
    components->addFromLeft(trackName);

    gridCombo = new ToolBar<ComboBox>();
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_1,  false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_2,  false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_4,  false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_8,  false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_16, false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_32, false, true ), 0 );
    gridCombo->addItem( new BitmapButton( 16, 14, mgrid_triplet, false, true ), 25 );
    components->addFromLeft(gridCombo);
    gridCombo->layout();

    scoreButton = new BitmapButton(32, 7, score_view, true);
    components->addFromLeft(scoreButton);
    pianoButton = new BitmapButton(32, 7, keyboard_view, true);
    components->addFromLeft(pianoButton);
    tabButton = new BitmapButton(32, 7, guitar_view, true);
    components->addFromLeft(tabButton);
    drumButton = new BitmapButton(32, 7, drum_view, true);
    components->addFromLeft(drumButton);
    ctrlButton = new BitmapButton(32, 7, controller_view, true);
    components->addFromLeft(ctrlButton);

    sharpFlatPicker = new ToolBar<BlankField>();
    sharpFlatPicker->addItem( (new BitmapButton( 14, 21, sharpSign,   false, true ))->setImageState(AriaRender::STATE_NOTE), 6 );
    sharpFlatPicker->addItem( (new BitmapButton( 14, 24, flatSign,    false, true ))->setImageState(AriaRender::STATE_NOTE), 6 );
    sharpFlatPicker->addItem( (new BitmapButton( 14, 21, naturalSign, false, true ))->setImageState(AriaRender::STATE_NOTE), 0 );
    components->addFromLeft(sharpFlatPicker);

    instrumentName = new BlankField(144);
    components->addFromRight(instrumentName);

    channelButton = new BlankField(28);
    components->addFromRight(channelButton);
}

GraphicalTrack::~GraphicalTrack()
{
}


void GraphicalTrack::createEditors()
{
     keyboardEditor     = new KeyboardEditor(track);
     guitarEditor       = new GuitarEditor(track);
     drumEditor         = new DrumEditor(track);
     controllerEditor   = new ControllerEditor(track);
     scoreEditor        = new ScoreEditor(track);
}

bool GraphicalTrack::mouseWheelMoved(int mx, int my, int value)
{
    if(my>from_y and my<to_y)
    {

        getCurrentEditor()->scroll( value / 75.0 );
        Display::render();

        return false; // event belongs to this track and was processed, stop searching
    }
    else
    {
        return true; // event does not belong to this track, continue searching
    }
}

// return value :
//  true: the event does not belong to this track and so the program should continue searching to whom the event belongs.
//  false: the event belongs to this track and was processed
bool GraphicalTrack::processMouseClick(RelativeXCoord mousex, int mousey)
{
    dragging_resize=false;

    lastMouseY = mousey;

    if(mousey>from_y and mousey<to_y)
    {

        sequence->setCurrentTrack(track);

        // resize drag
        if(mousey>to_y-15 and mousey<to_y-5)
        {
            dragging_resize = true;
            return false;
        }

        // if track is not collapsed, let the editor handle the mouse event too
        if(!collapsed) getCurrentEditor()->mouseDown(mousex, mousey);

        if(!ImageProvider::imagesLoaded()) return true;
        assert(collapseDrawable->image!=NULL);
        assert(muteDrawable->image!=NULL);

        const int winX = mousex.getRelativeTo(WINDOW);
        // collapse
        if( collapseButton->clickIsOnThisWidget(winX, mousey) )
        {
            collapsed = !collapsed;
            DisplayFrame::updateVerticalScrollbar();
        }

        // dock
        if( dockToolBar->getItem(0).clickIsOnThisWidget(winX, mousey) )
        {
            if(not getCurrentSequence()->maximize_track_mode)
            {
                const int track_amount = getCurrentSequence()->getTrackAmount();
                for(int n=0; n<track_amount; n++)
                {
                    Track* track = getCurrentSequence()->getTrack(n);
                    if(track->graphics == this)
                    {
                        maximizeHeight();
                        continue;
                    }
                    track->graphics->dock();
                }
                getCurrentSequence()->setYScroll(0);
                DisplayFrame::updateVerticalScrollbar();
                getCurrentSequence()->maximize_track_mode = true;
            }
            else
            {
                // switch off maximize mode.
                const int track_amount = getCurrentSequence()->getTrackAmount();
                for(int n=0; n<track_amount; n++)
                {
                    Track* track = getCurrentSequence()->getTrack(n);
                    if(track->graphics->docked) track->graphics->dock(false);
                    track->graphics->maximizeHeight(false);
                }
                DisplayFrame::updateVerticalScrollbar();
                getCurrentSequence()->maximize_track_mode = false;
            }
        }
        if( dockToolBar->getItem(1).clickIsOnThisWidget(winX, mousey) )
        {
            if(getCurrentSequence()->maximize_track_mode)
            {
                // switch off maximize mode. FIXME - duplicate code, see above
                const int track_amount = getCurrentSequence()->getTrackAmount();
                for(int n=0; n<track_amount; n++)
                {
                    Track* track = getCurrentSequence()->getTrack(n);
                    if(track->graphics->docked) track->graphics->dock(false);
                    track->graphics->maximizeHeight(false);
                }
                DisplayFrame::updateVerticalScrollbar();
                getCurrentSequence()->maximize_track_mode = false;
            }
            else
            {
                dock();
                DisplayFrame::updateVerticalScrollbar();
            }
        }

        // mute
        if( muteButton->clickIsOnThisWidget(winX, mousey) )
        {
            muted = !muted;
            DisplayFrame::updateVerticalScrollbar();
        }

        // track name
        if( trackName->clickIsOnThisWidget(winX, mousey) )
        {
            wxString msg=wxGetTextFromUser( _("Choose a new track title."), wxT("Aria Maestosa"), track->getName() );
            if(msg.Length()>0) track->setName( msg );
            Display::render();
        }

        // grid
        if( gridCombo->clickIsOnThisWidget(winX, mousey) )
        {
            wxCommandEvent fake_event;

            if( gridCombo->getItem(0).clickIsOnThisWidget(winX, mousey) )
                grid->grid1selected(fake_event);
            else if( gridCombo->getItem(1).clickIsOnThisWidget(winX, mousey) )
                grid->grid2selected(fake_event);
            else if( gridCombo->getItem(2).clickIsOnThisWidget(winX, mousey) )
                grid->grid4selected(fake_event);
            else if( gridCombo->getItem(3).clickIsOnThisWidget(winX, mousey) )
                grid->grid8selected(fake_event);
            else if( gridCombo->getItem(4).clickIsOnThisWidget(winX, mousey) )
                grid->grid16selected(fake_event);
            else if( gridCombo->getItem(5).clickIsOnThisWidget(winX, mousey) )
                grid->grid32selected(fake_event);
            else if( gridCombo->getItem(6).clickIsOnThisWidget(winX, mousey) )
                grid->toggleTriplet();
            else if( winX > gridCombo->getItem(5).getX() + 16)
                Display::popupMenu(grid, gridCombo->getX()+5, from_y+30);
        }


        // instrument
        if( instrumentName->clickIsOnThisWidget(winX, mousey) )
        {
            if(editorMode==DRUM)
            {
                Core::getDrumPicker()->setParent(track);
                Display::popupMenu((wxMenu*)(Core::getDrumPicker()), Display::getWidth() - 175, from_y+30);
            }
            else
            {
                Core::getInstrumentPicker()->setParent(track);
                Display::popupMenu((wxMenu*)(Core::getInstrumentPicker()), Display::getWidth() - 175, from_y+30);
            }
        }

        // channel
        if(sequence->getChannelManagementType() == CHANNEL_MANUAL)
        {

            if( channelButton->clickIsOnThisWidget(winX, mousey) )
            {
                const int channel = wxGetNumberFromUser( _("Enter the ID of the channel this track should play in"),
                                                         wxT(""),
                                                         _("Channel choice"),
                                                         track->getChannel(),
                                                         0,
                                                         15 );
                if(channel>=0 and channel<=15)
                {
                    track->setChannel(channel);

                    // check what is the instrument currently used in this channel, if any
                    const int trackAmount = sequence->getTrackAmount();
                    for(int n=0; n<trackAmount; n++) // find another track that has same channel and use the same instrument
                    {
                        if( sequence->getTrack(n)->getChannel() == channel )
                        {
                            track->setInstrument(sequence->getTrack(n)->getInstrument(), true);
                            break;
                        }
                    }//next

                    Display::render();
                }
            }
        }


        if(mousey > from_y+10 and mousey < from_y+40)
        {
            // modes
            if(winX > scoreButton->getX() and winX < scoreButton->getX()+30)
            {
                editorMode=SCORE;
            }
            else
            if(winX > pianoButton->getX() and winX < pianoButton->getX()+30)
            {
                // in midi, drums go to channel 9. So, if we exit drums, change channel so that it's not 9 anymore.
                if(editorMode == DRUM and sequence->getChannelManagementType() == CHANNEL_MANUAL) track->setChannel(0);

                editorMode=KEYBOARD;
            }
            else if(winX > tabButton->getX() and winX < tabButton->getX()+30)
            {
                // in midi, drums go to channel 9. So, if we exit drums, change channel so that it's not 9 anymore.
                if(editorMode == DRUM and sequence->getChannelManagementType() == CHANNEL_MANUAL) track->setChannel(0);

                editorMode=GUITAR;
                track->prepareNotesForGuitarEditor();
            }
            else if(winX > drumButton->getX() and winX < drumButton->getX()+30)
            {
                // in midi, drums go to channel 9 (10 if you start from one)
                if(sequence->getChannelManagementType() == CHANNEL_MANUAL) track->setChannel(9);

                editorMode=DRUM;
            }
            else if(winX > ctrlButton->getX() and winX < ctrlButton->getX()+30)
            {
                editorMode=CONTROLLER;
            }
        }

        if(editorMode==SCORE and mousey > from_y+15 and mousey < from_y+30)
        {
            // sharp/flat signs
            if( sharpFlatPicker->getItem(0).clickIsOnThisWidget(winX, mousey) )
                track->action( new Action::SetAccidentalSign(SHARP) );
            else if( sharpFlatPicker->getItem(1).clickIsOnThisWidget(winX, mousey) )
                track->action( new Action::SetAccidentalSign(FLAT) );
            else if( sharpFlatPicker->getItem(2).clickIsOnThisWidget(winX, mousey) )
                track->action( new Action::SetAccidentalSign(NATURAL) );
        }

        return false;
    }
    else
    {
        return true;
    }
}

bool GraphicalTrack::processRightMouseClick(RelativeXCoord x, int y)
{
    if(y>from_y and y<to_y)
    {
        getCurrentEditor()->rightClick(x,y);
        return false;
    }
    else
    {
        return true;
    }

}

void GraphicalTrack::processMouseRelease()
{
    //std::cout << "mouse up GraphicalTrack" << std::endl;

    if(!dragging_resize) getCurrentEditor()->mouseUp(Display::getMouseX_current(), Display:: getMouseY_current(),
                                                     Display::getMouseX_initial(), Display:: getMouseY_initial());

    if(dragging_resize)
    {
        dragging_resize=false;
        DisplayFrame::updateVerticalScrollbar();
    }
}

void GraphicalTrack::processMouseExited(RelativeXCoord x_now, int y_now, RelativeXCoord x_initial, int y_initial)
{
    getCurrentEditor()->TrackPropertiesDialog(x_now, y_now, x_initial, y_initial);
}

bool GraphicalTrack::processMouseDrag(RelativeXCoord x, int y)
{

    if((y>from_y and y<to_y) || dragging_resize)
    {

        /*
         *
         * until the end of the method, mousex_current/mousey_current contain the location of the mouse last time this event was thrown in the dragging process.
         * This can be used to determine the movement of the mouse.
         * At the end of the method, mousex_current/mousey_current are set to the current values.
         *
         */

        int barHeight=EXPANDED_BAR_HEIGHT;
        if(collapsed) barHeight=COLLAPSED_BAR_HEIGHT;

        if(!dragging_resize) getCurrentEditor()->mouseDrag(x, y, Display::getMouseX_initial(), Display:: getMouseY_initial());

        // resize drag
        if(dragging_resize)
        {

            if(height==35)
            { // if it has reached minimal size, wait until mouse comes back over before resizing again
                if(y>to_y-15 and y<to_y-5 and (y-lastMouseY)>0 ) height+=(y-lastMouseY);

            }
            else
            { // resize the track and check if it's not too small
                height+=(y-lastMouseY);
                if(height<35) height=35; //minimum size
            }
            DisplayFrame::updateVerticalScrollbar();

        }

        lastMouseY=y;


        return false;

    }
    else
    {
        return true;
    }
}

void GraphicalTrack::setCollapsed(const bool collapsed)
{
    GraphicalTrack::collapsed = collapsed;
}
void GraphicalTrack::setHeight(const int height)
{
    GraphicalTrack::height = height;
}
void GraphicalTrack::maximizeHeight(bool maximize)
{
    if(maximize)
    {
        setHeight(Display::getHeight() - getCurrentSequence()->dockHeight -
                  (getMeasureData()->isExpandedMode() ? 150 : 130)  ); // FIXME - don't hardcode values
    }
    else
    {
        if(height > 200) height = 200;
    }
}
void GraphicalTrack::dock(const bool dock)
{
    if(dock)
    {
        docked = true;
        getCurrentSequence()->addToDock( this );
    }
    else
    {
        docked = false;
        getCurrentSequence()->removeFromDock( this );
    }
}

int GraphicalTrack::getTotalHeight()
{

    if(docked) return 0;

    if(collapsed)
    {
        return 45; // COLLAPSED_BAR_HEIGHT
    }
    else
    {
        return EXPANDED_BAR_HEIGHT+50+height;
    }

}

int GraphicalTrack::getEditorHeight(){        return height;        }

void GraphicalTrack::renderHeader(const int x, const int y, const bool closed, const bool focus)
{

    const bool channel_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;

    int barHeight=EXPANDED_BAR_HEIGHT;
    if(closed) barHeight=COLLAPSED_BAR_HEIGHT;

    if(!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else AriaRender::setImageState(AriaRender::STATE_NORMAL);

    AriaRender::images();

    // top left corner
    cornerDrawable->move(x+10,y);
    cornerDrawable->setFlip(false, false);
    cornerDrawable->render();

    // top border
    borderDrawable->move(x+30, y);
    borderDrawable->setFlip(false, false);
    borderDrawable->rotate(0);
    borderDrawable->scale((Display::getWidth() - 5 /*margin*/ -20 /*left round cornerDrawable*/ - 20 /*right round cornerDrawable*/)/20.0, 1 );
    borderDrawable->render();

    // top right corner
    cornerDrawable->move(x+ Display::getWidth() - 5 /* margin*/ -20 /*left round cornerDrawable*/, y);
    cornerDrawable->setFlip(true, false);
    cornerDrawable->render();

    // --------------------------------------------------

    // left border
    borderDrawable->move(x+ 30, y+20);
    borderDrawable->setFlip(false, true);
    borderDrawable->rotate(90);
    borderDrawable->scale(1, barHeight /*number of pixels high*/ /20.0 );
    borderDrawable->render();

    // right border
    borderDrawable->move(x+ Display::getWidth() - 5 /*margin*/ - 20 /*left round cornerDrawable*/ + 20 /*due to rotation of 90 degrees*/, y+20);
    borderDrawable->setFlip(false, false);
    borderDrawable->rotate(90);
    borderDrawable->scale(1, barHeight /*number of pixels high*/ /20.0 );
    borderDrawable->render();

    // --------------------------------------------------

    // center
    AriaRender::primitives();

    if(!focus) AriaRender::color(0.31/2, 0.31/2, 0.31/2);
    else AriaRender::color(0.31, 0.31, 0.31);

    AriaRender::rect(x+ 30, y+20, x+ Display::getWidth() - 5 /* margin*/ - 20 /*right round cornerDrawable*/, y+20+barHeight);


    // --------------------------------------------------

    if(closed)
    {
         AriaRender::images();

        if(!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
        else AriaRender::setImageState(AriaRender::STATE_NORMAL);

        // bottom left corner
        cornerDrawable->move(x+10, y+20+barHeight);
        cornerDrawable->setFlip(false, true);
        cornerDrawable->render();

        // bottom border
        borderDrawable->move(x+30, y+20+barHeight);
        borderDrawable->setFlip(false, true);
        borderDrawable->rotate(0);
        borderDrawable->scale((Display::getWidth() - 5 /*margin*/ -20 /*left round cornerDrawable*/ - 20 /*right round cornerDrawable*/)/20.0, 1 );
        borderDrawable->render();

        // bottom right corner
        cornerDrawable->move(x+ Display::getWidth() - 5 /*margin*/ -20 /*left round cornerDrawable*/, y+20+barHeight);
        cornerDrawable->setFlip(true, true);
        cornerDrawable->render();

        AriaRender::setImageState(AriaRender::STATE_NORMAL);

    }
    else
    {
        // white area
        if(editorMode != KEYBOARD) // keyboard editor draws its own backgound, so no need to draw it twice // FIXME no more true
        {
            AriaRender::primitives();
            AriaRender::color(1, 1, 1);

            AriaRender::rect(x+10, y+barHeight+20, x+Display::getWidth() - 5 , y+barHeight+40+height);
        }//end if
    }//end if

    // ------------------ prepare to draw components ------------------
    if(collapsed) collapseButton->drawable->setImage( expandImg );
    else collapseButton->drawable->setImage( collapseImg );

    if(muted) muteButton->drawable->setImage( muteOnImg );
    else muteButton->drawable->setImage( muteOffImg );

    scoreButton -> enable( editorMode == SCORE      and focus );
    pianoButton -> enable( editorMode == KEYBOARD   and focus );
    tabButton   -> enable( editorMode == GUITAR     and focus );
    drumButton  -> enable( editorMode == DRUM       and focus );
    ctrlButton  -> enable( editorMode == CONTROLLER and focus );

    sharpFlatPicker->show(editorMode==SCORE);

    channelButton->show(channel_mode);

    // ------------------ layout and draw components ------------------
    components->layout(20, y);
    sharpFlatPicker->layout();
    gridCombo->layout();
    dockToolBar->layout();
    components->renderAll(focus);

    //  ------------------ post-drawing  ------------------

    // draw track name
    AriaRender::images();
    AriaRender::color(0,0,0);
    AriaRenderString& track_name = track->getName();
    track_name.bind();
    track_name.render(trackName->getX()+11, y+30);
    
    // draw grid label
    int grid_selection_x;
    switch(grid->divider)
    {
        case 1:
            grid_selection_x = mgrid_1->x;
            break;
        case 2:
        case 3:
            grid_selection_x = mgrid_2->x;
            break;
        case 4:
        case 6:
            grid_selection_x = mgrid_4->x;
            break;
        case 8:
        case 12:
            grid_selection_x = mgrid_8->x;
            break;
        case 16:
        case 24:
            grid_selection_x = mgrid_16->x;
            break;
        case 32:
        case 48:
            grid_selection_x = mgrid_32->x;
            break;
        default: // Should not happen
            assert(false);
            grid_selection_x = mgrid_8->x;
    }
    AriaRender::primitives();
    AriaRender::color(0,0,0);
    AriaRender::hollow_rect(grid_selection_x, y+15, grid_selection_x+16, y+30);
    if(grid->isTriplet()) AriaRender::hollow_rect(mgrid_triplet->x, y+15, mgrid_triplet->x+16, y+30);

    // draw instrument name
    AriaRender::images();
    AriaRender::color(0,0,0);
    
    if(editorMode == DRUM) 
        Core::getDrumPicker()->renderDrumKitName( track->getDrumKit(), instrumentName->getX()+11 ,y+30);
    else
    {
        track->instrument_name.bind();
        track->instrument_name.render(instrumentName->getX()+11 ,y+30);
    }
        //Core::getInstrumentPicker()->renderInstrumentName( track->getInstrument(), instrumentName->getX()+11 ,y+30 );

    AriaRender::images();
    
    // draw channel number
    if(channel_mode)
    {
        wxString channelName = to_wxString(track->getChannel());

        AriaRender::color(0,0,0);
        
        const int char_amount_in_channel_name = channelName.size();
        if(char_amount_in_channel_name == 1) AriaRender::renderNumber(channelName, channelButton->getX()+10, y+28);
        else AriaRender::renderNumber(channelName, channelButton->getX()+7, y+28);
    }

}// end func

int GraphicalTrack::render(const int y, const int currentTick, const bool focus)
{

    if(!ImageProvider::imagesLoaded()) return 0;

    // docked tracks are not drawn
    if(docked)
    {
        from_y=-1;
        to_y=-1;
        return y;
    }

    int barHeight=EXPANDED_BAR_HEIGHT;
    if(collapsed) barHeight=COLLAPSED_BAR_HEIGHT;

    from_y=y;

    if(collapsed) to_y = from_y + 45;
    else to_y = y+barHeight+50+height;

    // tell the editor about its new location
    getCurrentEditor()->updatePosition(from_y, to_y, Display::getWidth(), height, barHeight);

    // don't waste time drawing it if out of bounds
    if(to_y < 0) return to_y;
    if(from_y > Display::getHeight()) return to_y;

    renderHeader(0, y, collapsed, focus);

    if(!collapsed)
    {
        // --------------------------------------------------
        // render editor
        getCurrentEditor()->render(Display::getMouseX_current(),
                                   Display:: getMouseY_current(),
                                   Display::getMouseX_initial(),
                                   Display:: getMouseY_initial(), focus);
        // --------------------------------------------------
        // render playback progress line

        AriaRender::primitives();

        if( currentTick!=-1 and not Display::leftArrow() and not Display::rightArrow())
        {
            AriaRender::color(0.8, 0, 0);

            RelativeXCoord tick(currentTick, MIDI);
            const int x_coord = tick.getRelativeTo(WINDOW);

            AriaRender::lineWidth(1);

            AriaRender::line(x_coord, getCurrentEditor()->getEditorYStart(),
                             x_coord, getCurrentEditor()->getYEnd());

        }
        AriaRender::images();

        // --------------------------------------------------
        // render track borders

        if(!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
        else AriaRender::setImageState(AriaRender::STATE_NORMAL);

        // bottom left corner
        whiteCornerDrawable->move(10,y+20+barHeight+height);
        whiteCornerDrawable->setFlip(false, false);
        whiteCornerDrawable->render();

        // bottom border
        whiteBorderDrawable->move(30, y+20+barHeight+height);
        whiteBorderDrawable->setFlip(false, false);
        whiteBorderDrawable->rotate(0);
        whiteBorderDrawable->scale((Display::getWidth() - 5 /* margin*/ - 20 /*left round cornerDrawable*/ - 20 /*right round cornerDrawable*/)/20.0, 1 );
        whiteBorderDrawable->render();

        // bottom right corner
        whiteCornerDrawable->move(Display::getWidth() - 5 /* margin*/ - 20 /*left round cornerDrawable*/, y+20+barHeight+height);
        whiteCornerDrawable->setFlip(true, false);
        whiteCornerDrawable->render();

        // --------------------------------------------------

        // left borderDrawable
        whiteBorderDrawable->move(30, y+barHeight+20);
        whiteBorderDrawable->setFlip(false, false);
        whiteBorderDrawable->rotate(90);
        whiteBorderDrawable->scale(1, height /*number of pixels high*/ /20.0 );
        whiteBorderDrawable->render();

        // right borderDrawable
        whiteBorderDrawable->move(Display::getWidth() - 5 , y+barHeight+20);
        whiteBorderDrawable->setFlip(false, true);
        whiteBorderDrawable->rotate(90);
        whiteBorderDrawable->scale(1, height /*number of pixels high*/ /20.0 );
        whiteBorderDrawable->render();

    }

    AriaRender::images();

    // done
    return to_y;

}

Editor* GraphicalTrack::getCurrentEditor()
{
    if(editorMode==KEYBOARD) return keyboardEditor;
    else if(editorMode==GUITAR) return guitarEditor;
    else if(editorMode==DRUM) return drumEditor;
    else if(editorMode==CONTROLLER) return controllerEditor;
    else if(editorMode==SCORE) return scoreEditor;
    else
    {
        std::cout << "No such editor!" << std::endl;
        assert(false);
        return NULL; // shut up warnings
    }
}

void GraphicalTrack::setEditorMode(int mode)
{
    editorMode = mode;
}

// ---------------------------------------- serialization -----------------------------------------
void GraphicalTrack::saveToFile(wxFileOutputStream& fileout)
{
    const int octave_shift = scoreEditor->getScoreMidiConverter()->getOctaveShift();

    writeData( wxT("<editor mode=\"") + to_wxString(editorMode) +
               wxT("\" height=\"") + to_wxString(height) +
               (collapsed ? wxT("\" collapsed=\"true") : wxT("")) +
               (muted ? wxT("\" muted=\"true") : wxT("") ) +
               wxT("\" g_clef=\"") + (scoreEditor->isGClefEnabled()?wxT("true"):wxT("false")) +
               wxT("\" f_clef=\"") + (scoreEditor->isFClefEnabled()?wxT("true"):wxT("false")) +
               ( octave_shift != 0 ? wxT("\" octave_shift=\"")+to_wxString(octave_shift) : wxT("")) +
               wxT("\"/>\n")
               , fileout );

    grid->saveToFile(fileout);
    //keyboardEditor->instrument->saveToFile(fileout);
    //drumEditor->drumKit->saveToFile(fileout);

    writeData( wxT("<instrument id=\"") + to_wxString( track->getInstrument() ) + wxT("\"/>\n"), fileout);
    writeData( wxT("<drumkit id=\"") + to_wxString( track->getDrumKit() ) + wxT("\"/>\n"), fileout);

    writeData( wxT("<key sharps=\"") + to_wxString( scoreEditor->getKeySharpsAmount() ) +
               wxT("\" flats=\"") + to_wxString( scoreEditor->getKeyFlatsAmount() ) +
               + wxT("\"/>\n"), fileout);

    // guitar tuning
    writeData( wxT("<guitartuning "), fileout);
    for(unsigned int n=0; n<guitarEditor->tuning.size(); n++)
    {
        writeData( wxT(" string")+ to_wxString((int)n) + wxT("=\"") + to_wxString((int)guitarEditor->tuning[n]) + wxT("\""), fileout );
    }

    writeData( wxT("/>\n\n"), fileout);

}

bool GraphicalTrack::readFromFile(irr::io::IrrXMLReader* xml)
{

    if (!strcmp("editor", xml->getNodeName()))
    {

        const char* mode_c = xml->getAttributeValue("mode");
        if( mode_c != NULL ) editorMode = atoi( mode_c );
        else
        {
            editorMode = KEYBOARD;
            std::cout << "Missing info from file: editor mode" << std::endl;
        }

        const char* height_c = xml->getAttributeValue("height");
        if( height_c != NULL ) height = atoi( height_c );
        else
        {
            std::cout << "Missing info from file: track height" << std::endl;
            height = 200;
        }

        const char* collapsed_c = xml->getAttributeValue("collapsed");
        if( collapsed_c != NULL )
        {
            if(!strcmp(collapsed_c, "true")) collapsed = true;
            else if(!strcmp(collapsed_c, "false")) collapsed = false;
            else
            {
                std::cout << "Unknown keyword for attribute 'collapsed' in track: " << collapsed_c << std::endl;
                collapsed = false;
            }

        }
        else
        {
            collapsed = false;
        }

        const char* muted_c = xml->getAttributeValue("muted");
        if( muted_c != NULL )
        {
            if(!strcmp(muted_c, "true")) muted = true;
            else if(!strcmp(muted_c, "false")) muted = false;
            else
            {
                muted = false;
                std::cout << "Unknown keyword for attribute 'muted' in track: " << muted_c << std::endl;
            }

        }
        else
        {
            muted = false;
        }

        const char* g_clef_c = xml->getAttributeValue("g_clef");
        if( g_clef_c != NULL )
        {
            if(!strcmp(g_clef_c, "true")) scoreEditor->enableGClef(true);
            else if(!strcmp(g_clef_c, "false")) scoreEditor->enableGClef(false);
            else
            {
                std::cout << "Unknown keyword for attribute 'g_clef' in track: " << g_clef_c << std::endl;
            }

        }
        const char* f_clef_c = xml->getAttributeValue("f_clef");
        if( f_clef_c != NULL )
        {
            if(!strcmp(f_clef_c, "true")) scoreEditor->enableFClef(true);
            else if(!strcmp(f_clef_c, "false")) scoreEditor->enableFClef(false);
            else
            {
                std::cout << "Unknown keyword for attribute 'f_clef' in track: " << f_clef_c << std::endl;
            }

        }
        const char* octave_shift_c = xml->getAttributeValue("octave_shift");
        if( octave_shift_c != NULL )
        {
            int new_value = atoi( octave_shift_c );
            if(new_value != 0) scoreEditor->getScoreMidiConverter()->setOctaveShift(new_value);
        }

    }
    else if (!strcmp("magneticgrid", xml->getNodeName()))
    {

        if(! grid->readFromFile(xml) )
            return false;

    }
    else if (!strcmp("key", xml->getNodeName()))
    {
        //std::cout << "Found 'key'!" << std::endl;
        char* flats_c = (char*)xml->getAttributeValue("flats");
        char* sharps_c = (char*)xml->getAttributeValue("sharps");

        int sharps = 0, flats = 0;
        if(flats_c == NULL and sharps_c == NULL);
        else
        {
            if(flats_c != NULL) flats = atoi(flats_c);
            if(sharps_c != NULL) sharps = atoi(sharps_c);

            //std::cout << "sharps = " << sharps << " flats = " << flats << std::endl;

            if(sharps > flats)
            {
                scoreEditor->loadKey(SHARP, sharps);
                keyboardEditor->loadKey(SHARP, sharps);
            }
            else
            {
                scoreEditor->loadKey(FLAT, flats);
                keyboardEditor->loadKey(FLAT, flats);
            }
        }

    }
    else if (!strcmp("guitartuning", xml->getNodeName()))
    {

        guitarEditor->tuning.clear();

        int n=0;
        char* string_v = (char*)xml->getAttributeValue("string0");

        while(string_v != NULL)
        {
            guitarEditor->tuning.push_back( atoi(string_v) );

            n++;
            wxString tmp = wxT("string") + to_wxString(n);
            string_v = (char*)xml->getAttributeValue( tmp.mb_str() );
        }

        if(guitarEditor->tuning.size() < 3)
        {
            std::cout << "FATAL ERROR: Invalid tuning!! only " << guitarEditor->tuning.size() << " strings found" << std::endl;
            return false;
        }

        guitarEditor->tuningUpdated(false);
    }

    return true;

}

}
