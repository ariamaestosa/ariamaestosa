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


#include <iostream>
#include <wx/numdlg.h>
#include <wx/wfstream.h>
#include <wx/textdlg.h>

#include "Utils.h"
#include "GUI/GraphicalTrack.h"

#include "AriaCore.h"
#include "Actions/SetAccidentalSign.h"
#include "Editors/KeyboardEditor.h"
#include "Editors/Editor.h"
#include "Editors/GuitarEditor.h"
#include "Editors/DrumEditor.h"
#include "Editors/RelativeXCoord.h"
#include "Editors/ControllerEditor.h"
#include "Editors/ScoreEditor.h"
#include "GUI/ImageProvider.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "IO/IOUtils.h"
#include "Midi/DrumChoice.h"
#include "Midi/InstrumentChoice.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/DrumPicker.h"
#include "Pickers/InstrumentPicker.h"
#include "Pickers/MagneticGrid.h"
#include "PreferencesData.h"
#include "Renderers/Drawable.h"
#include "Renderers/ImageBase.h"
#include "Renderers/RenderAPI.h"

#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

namespace AriaMaestosa
{
    class AriaWidget
    {
    protected:
        int m_x, m_y;
        int m_width;
        bool m_hidden;
    public:
        LEAK_CHECK();
        
        AriaWidget(int width)
        {
            m_x = 0;
            m_width = width;
            m_hidden = false;
        }
        
        int getX() const { return m_x; }
        int getY() const { return m_y; }
        int getWidth() const { return m_width; }
        
        bool isHidden() const { return m_hidden; }
        void show(bool shown){ m_hidden = not shown; }
        
        // don't call this, let WidgetLayoutManager do it
        void setX(const int x){ m_x = x; }
        void setY(const int y){ m_y = y; }
        
        bool clickIsOnThisWidget(const int mx, const int my)
        {
            return (not m_hidden) and ( mx > m_x and my > m_y and mx < m_x + m_width and my < m_y + 30);
        }
        
        virtual void render(){}
        virtual ~AriaWidget(){}
    };
    
    // --------------------------------------------------------------------------------------------------
    
    class BlankField : public AriaWidget
    {
    public:
        BlankField(int width) : AriaWidget(width){}
        virtual ~BlankField(){}
        
        void render()
        {
            if (m_hidden) return;
            comboBorderDrawable->move(m_x, m_y + 7);
            comboBorderDrawable->setFlip(false, false);
            comboBorderDrawable->render();
            
            comboBodyDrawable->move(m_x + 14, m_y + 7);
            comboBodyDrawable->scale((m_width - 28)/4.0 , 1);
            comboBodyDrawable->render();
            
            comboBorderDrawable->move(m_x + m_width - 14, m_y + 7 );
            comboBorderDrawable->setFlip(true,false);
            comboBorderDrawable->render();
        }
    };
    
    // --------------------------------------------------------------------------------------------------
    
    class ComboBox : public AriaWidget
    {
    public:
        ComboBox(int width) : AriaWidget(width){}
        virtual ~ComboBox(){}
        
        void render()
        {
            if (m_hidden) return;
            comboBorderDrawable->move(m_x, m_y + 7);
            comboBorderDrawable->setFlip(false, false);
            comboBorderDrawable->render();
            
            comboBodyDrawable->move(m_x + 14, m_y + 7);
            comboBodyDrawable->scale((m_width - 28 - 18)/4.0, 1);
            comboBodyDrawable->render();
            
            comboSelectDrawable->move(m_x + m_width - 14 - 18, m_y + 7);
            comboSelectDrawable->render();
        }
    };
    
    // --------------------------------------------------------------------------------------------------
    
    class BitmapButton : public AriaWidget
    {
        int m_y_offset;
        bool m_enabled;
        bool m_center_x;
        AriaRender::ImageState m_state;
        
    public:
        Drawable* m_drawable;
        
        BitmapButton(int width, int y_offset, Drawable* drawable, bool centerX=false) : AriaWidget(width)
        {
            m_drawable = drawable;
            m_y_offset = y_offset;
            m_enabled = true;

            m_center_x = centerX;
            m_state = AriaRender::STATE_NORMAL;
        }
        virtual ~BitmapButton(){}
        
        BitmapButton* setImageState(AriaRender::ImageState state)
        {
            m_state = state;
            return this;
        }
        
        void render()
        {
            if (m_hidden) return;
            
            if (m_state != AriaRender::STATE_NORMAL)
            {
                AriaRender::setImageState(m_state);
            }
            else if (not m_enabled)
            {
                AriaRender::setImageState(AriaRender::STATE_DISABLED);
            }
            
            if (m_center_x and m_drawable->getImageWidth() < m_width)
            {
                const int ajust = (m_width - m_drawable->getImageWidth())/2;
                m_drawable->move(m_x + m_drawable->getHotspotX() + ajust, m_y + m_y_offset);
            }
            else
            {
                m_drawable->move(m_x + m_drawable->getHotspotX(), m_y + m_y_offset);
            }
            
            m_drawable->render();
        }
        
        void enable(const bool enabled)
        {
            m_enabled = enabled;
        }
    };
    
    // --------------------------------------------------------------------------------------------------
    
    template<typename PARENT>
    class ToolBar : public PARENT
    {
        ptr_vector<BitmapButton, HOLD> m_contents;
        std::vector<int> m_margin;
        
    public:
        ToolBar() : PARENT(22)
        {
        }
        void addItem(BitmapButton* btn, int margin_after)
        {
            m_contents.push_back(btn);
            m_margin.push_back(margin_after);
        }
        void layout()
        {
            if (PARENT::m_hidden) return;
            
            PARENT::m_width = 22;
            int currentX = PARENT::m_x + 11;
            
            const int amount = m_contents.size();
            for (int n=0; n<amount; n++)
            {
                m_contents[n].setX(currentX);
                m_contents[n].setY(PARENT::m_y);
                
                currentX        += m_contents[n].getWidth() + m_margin[n];
                PARENT::m_width += m_contents[n].getWidth() + m_margin[n];
            }
        }
        
        BitmapButton& getItem(const int item)
        {
            return m_contents[item];
        }
        
        void render()
        {
            if (PARENT::m_hidden) return;
            
            // render background
            PARENT::render();
            
            // render buttons
            const int amount = m_contents.size();
            
            for (int n=0; n<amount; n++)
            {
                m_contents[n].render();
            }
            AriaRender::setImageState(AriaRender::STATE_NORMAL);
        }
    };
    
    // --------------------------------------------------------------------------------------------------
    
    class WidgetLayoutManager
    {
        ptr_vector<AriaWidget, HOLD> m_widgets_left;
        ptr_vector<AriaWidget, HOLD> m_widgets_right;
        
    public:
        LEAK_CHECK();
        
        WidgetLayoutManager()
        {
        }
        void addFromLeft(AriaWidget* w)
        {
            m_widgets_left.push_back(w);
        }
        void addFromRight(AriaWidget* w)
        {
            m_widgets_right.push_back(w);
        }
        void layout(const int x_origin, const int y_origin)
        {
            const int lamount = m_widgets_left.size();
            int lx = x_origin;
            for (int n=0; n<lamount; n++)
            {
                m_widgets_left[n].setX(lx);
                m_widgets_left[n].setY(y_origin);
                lx += m_widgets_left[n].getWidth();
            }
            
            const int ramount = m_widgets_right.size();
            int rx = Display::getWidth() - 17;
            
            for (int n=0; n<ramount; n++)
            {
                rx -= m_widgets_right[n].getWidth();
                m_widgets_right[n].setX(rx);
                m_widgets_right[n].setY(y_origin);
            }
        }
        
        void renderAll(bool focus)
        {
            AriaRender::images();
            
            const int lamount = m_widgets_left.size();
            for (int n=0; n<lamount; n++)
            {
                if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
                else           AriaRender::setImageState(AriaRender::STATE_NORMAL);
                
                m_widgets_left.get(n)->render();
            }
            
            const int ramount = m_widgets_right.size();
            for (int n=0; n<ramount; n++)
            {
                if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
                else           AriaRender::setImageState(AriaRender::STATE_NORMAL);
                
                m_widgets_right.get(n)->render();
            }
        }
    };
}

#if 0
#pragma mark -
#pragma mark GraphicalTrack (ctor & dtor)
#endif
    
// --------------------------------------------------------------------------------------------------
    
const int EXPANDED_BAR_HEIGHT = 20;
const int COLLAPSED_BAR_HEIGHT = 5; //FIXME: what's that?? a collapsed bar is not 5 pixels high?? */

GraphicalTrack::GraphicalTrack(Track* track, GraphicalSequence* seq)
{
    m_keyboard_editor     = NULL;
    m_guitar_editor       = NULL;
    m_drum_editor         = NULL;
    m_controller_editor   = NULL;
    m_score_editor        = NULL;

    m_gsequence = seq;
    m_track = track;
    
    track->setListener(this);

    ASSERT(track);

    m_grid = new MagneticGrid(this);

    m_last_mouse_y = 0;

    m_collapsed       = false;
    m_dragging_resize = false;
    m_docked          = false;

    m_height = 128;

    // create widgets
    m_components = new WidgetLayoutManager();

    m_collapse_button = new BitmapButton(28, 15, collapseDrawable);
    m_components->addFromLeft(m_collapse_button);

    m_mute_button = new BitmapButton(28, 10, muteDrawable);
    m_components->addFromLeft(m_mute_button);

    m_dock_toolbar = new ToolBar<BlankField>();
    m_dock_toolbar->addItem( new BitmapButton( 16, 14, maximizeTrackDrawable, false), 0 );
    m_dock_toolbar->addItem( new BitmapButton( 16, 14, dockTrackDrawable    , false), 0 );
    m_components->addFromLeft(m_dock_toolbar);
    m_dock_toolbar->layout();

    m_track_name = new BlankField(140);
    m_components->addFromLeft(m_track_name);

    m_grid_combo = new ToolBar<ComboBox>();
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_1, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_2, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_4, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_8, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_16, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_32, true ), 0 );
    m_grid_combo->addItem( new BitmapButton( 16, 14, mgrid_triplet, true ), 25 );
    m_components->addFromLeft(m_grid_combo);
    m_grid_combo->layout();

    m_score_button = new BitmapButton(32, 7, score_view);
    m_components->addFromLeft(m_score_button);
    m_piano_button = new BitmapButton(32, 7, keyboard_view);
    m_components->addFromLeft(m_piano_button);
    m_tab_button = new BitmapButton(32, 7, guitar_view);
    m_components->addFromLeft(m_tab_button);
    m_drum_button = new BitmapButton(32, 7, drum_view);
    m_components->addFromLeft(m_drum_button);
    m_ctrl_button = new BitmapButton(32, 7, controller_view);
    m_components->addFromLeft(m_ctrl_button);

    m_sharp_flat_picker = new ToolBar<BlankField>();
    m_sharp_flat_picker->addItem( (new BitmapButton( 14, 21, sharpSign,   true ))->setImageState(AriaRender::STATE_NOTE), 6 );
    m_sharp_flat_picker->addItem( (new BitmapButton( 14, 24, flatSign,    true ))->setImageState(AriaRender::STATE_NOTE), 6 );
    m_sharp_flat_picker->addItem( (new BitmapButton( 14, 21, naturalSign, true ))->setImageState(AriaRender::STATE_NOTE), 0 );
    m_components->addFromLeft(m_sharp_flat_picker);

    m_instrument_field = new BlankField(144);
    m_components->addFromRight(m_instrument_field);

    m_channel_field = new BlankField(28);
    m_components->addFromRight(m_channel_field);
    
    if (m_track->getNotationType() == DRUM)
    {
        m_instrument_name.set(DrumChoice::getDrumkitName( m_track->getDrumKit() ));
    }
    else
    {
        m_instrument_name.set(InstrumentChoice::getInstrumentName( m_track->getInstrument() ));
    }
    
    m_instrument_name.setFont( getInstrumentNameFont() );
}

// --------------------------------------------------------------------------------------------------
    
GraphicalTrack::~GraphicalTrack()
{
}

// --------------------------------------------------------------------------------------------------
    
void GraphicalTrack::createEditors()
{
    ASSERT(m_all_editors.size() == 0); // function to be called once per object only
    
    m_keyboard_editor = new KeyboardEditor(m_track);
    m_all_editors.push_back(m_keyboard_editor);
    
    m_guitar_editor = new GuitarEditor(m_track);
    m_all_editors.push_back(m_guitar_editor);
    
    m_drum_editor = new DrumEditor(m_track);
    m_all_editors.push_back(m_drum_editor);

    m_controller_editor = new ControllerEditor(m_track);
    m_all_editors.push_back(m_controller_editor);

    m_score_editor = new ScoreEditor(m_track);
    m_all_editors.push_back(m_score_editor);
}

// --------------------------------------------------------------------------------------------------
    
#if 0
#pragma mark -
#pragma mark Events
#endif

bool GraphicalTrack::mouseWheelMoved(int mx, int my, int value)
{
    if (my > m_from_y and my < m_to_y)
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

// --------------------------------------------------------------------------------------------------
    
bool GraphicalTrack::processMouseClick(RelativeXCoord mousex, int mousey)
{
    m_dragging_resize = false;

    m_last_mouse_y = mousey;

    if (mousey > m_from_y and mousey < m_to_y)
    {
        m_gsequence->getModel()->setCurrentTrack( m_track );

        // resize drag
        if (mousey > m_to_y - 15 and mousey < m_to_y - 5)
        {
            m_dragging_resize = true;
            return false;
        }

        // if track is not collapsed, let the editor handle the mouse event too
        if (not m_collapsed) getCurrentEditor()->mouseDown(mousex, mousey);

        if (not ImageProvider::imagesLoaded()) return true;

        const int winX = mousex.getRelativeTo(WINDOW);
        // collapse
        if (m_collapse_button->clickIsOnThisWidget(winX, mousey))
        {
            m_collapsed = not m_collapsed;
            DisplayFrame::updateVerticalScrollbar();
        }

        // maximize button
        if (m_dock_toolbar->getItem(0).clickIsOnThisWidget(winX, mousey))
        {
            Sequence* seq = m_gsequence->getModel();

            if (not m_gsequence->isTrackMaximized())
            {
                
                // switch on maximize mode
                const int track_amount = seq->getTrackAmount();
                for (int n=0; n<track_amount; n++)
                {
                    Track* track = seq->getTrack(n);
                    if (track->getGraphics() == this)
                    {
                        maximizeHeight();
                        continue;
                    }
                    track->getGraphics()->dock();
                }
                m_gsequence->setYScroll(0);
                DisplayFrame::updateVerticalScrollbar();
                m_gsequence->setTrackMaximized(true);
            }
            else
            {
                // switch off maximize mode.
                const int track_amount = seq->getTrackAmount();
                for (int n=0; n<track_amount; n++)
                {
                    Track* track = seq->getTrack(n);
                    if (track->getGraphics()->m_docked) track->getGraphics()->dock(false);
                    track->getGraphics()->maximizeHeight(false);
                }
                DisplayFrame::updateVerticalScrollbar();
                m_gsequence->setTrackMaximized(false);
            }
        }
        // dock button
        else if ( m_dock_toolbar->getItem(1).clickIsOnThisWidget(winX, mousey) )
        {
            // This button is disabled in maximized mode
            if (not m_gsequence->isTrackMaximized())
            {
                dock();
                DisplayFrame::updateVerticalScrollbar();
            }
        }

        // mute
        if (m_mute_button->clickIsOnThisWidget(winX, mousey))
        {
            m_track->toggleMuted();
            DisplayFrame::updateVerticalScrollbar();
        }

        // track name
        if (m_track_name->clickIsOnThisWidget(winX, mousey))
        {
            wxString msg = wxGetTextFromUser(_("Choose a new track title."), wxT("Aria Maestosa"),
                                             m_track->getName() );
            if (msg.Length() > 0) m_track->setName( msg );
            Display::render();
        }

        // grid
        if (m_grid_combo->clickIsOnThisWidget(winX, mousey))
        {
            wxCommandEvent fake_event;

            if ( m_grid_combo->getItem(0).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid1selected(fake_event);
            else if ( m_grid_combo->getItem(1).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid2selected(fake_event);
            else if ( m_grid_combo->getItem(2).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid4selected(fake_event);
            else if ( m_grid_combo->getItem(3).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid8selected(fake_event);
            else if ( m_grid_combo->getItem(4).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid16selected(fake_event);
            else if ( m_grid_combo->getItem(5).clickIsOnThisWidget(winX, mousey) )
                m_grid->grid32selected(fake_event);
            else if ( m_grid_combo->getItem(6).clickIsOnThisWidget(winX, mousey) )
                m_grid->toggleTriplet();
            else if ( winX > m_grid_combo->getItem(5).getX() + 16)
                Display::popupMenu(m_grid, m_grid_combo->getX() + 5, m_from_y + 30);
        }


        // instrument
        if (m_instrument_field->clickIsOnThisWidget(winX, mousey))
        {
            if (m_track->getNotationType() == DRUM)
            {
                Core::getDrumPicker()->setModel(m_track->getDrumkitModel());
                Display::popupMenu((wxMenu*)(Core::getDrumPicker()), Display::getWidth() - 175, m_from_y + 30);
            }
            else
            {
                Core::getInstrumentPicker()->setModel(m_track->getInstrumentModel());
                Display::popupMenu((wxMenu*)(Core::getInstrumentPicker()),
                                   Display::getWidth() - 175, m_from_y + 30);
            }
        }

        // channel
        if (m_gsequence->getModel()->getChannelManagementType() == CHANNEL_MANUAL)
        {

            if (m_channel_field->clickIsOnThisWidget(winX, mousey))
            {
                const int channel = wxGetNumberFromUser( _("Enter the ID of the channel this track should play in"),
                                                         wxT(""),
                                                         _("Channel choice"),
                                                         m_track->getChannel(),
                                                         0,
                                                         15 );
                if (channel >= 0 and channel <= 15)
                {
                    m_track->setChannel(channel);
                    Display::render();
                }
            }
        }


        if (mousey > m_from_y + 10 and mousey < m_from_y + 40)
        {
            // modes
            if (winX > m_score_button->getX() and winX < m_score_button->getX() + 30)
            {
                m_track->setNotationType(SCORE);
            }
            else if (winX > m_piano_button->getX() and winX < m_piano_button->getX()+30)
            {
                // in midi, drums go to channel 9. So, if we exit drums, change channel so that it's not 9 anymore.
                if (m_track->getNotationType() == DRUM and
                    m_gsequence->getModel()->getChannelManagementType() == CHANNEL_MANUAL)
                {
                    m_track->setChannel(0);
                }

                m_track->setNotationType(KEYBOARD);
            }
            else if (winX > m_tab_button->getX() and winX < m_tab_button->getX() + 30)
            {
                // in midi, drums go to channel 9. So, if we exit drums, change channel so that it's not 9 anymore.
                if (m_track->getNotationType() == DRUM and
                    m_gsequence->getModel()->getChannelManagementType() == CHANNEL_MANUAL)
                {
                    m_track->setChannel(0);
                }

                m_track->setNotationType(GUITAR);
                m_track->prepareNotesForGuitarEditor(); // FIXME(DESIGN): there should be no need to manually call this
            }
            else if (winX > m_drum_button->getX() and winX < m_drum_button->getX() + 30)
            {
                // in midi, drums go to channel 9 (10 if you start from one)
                if (m_gsequence->getModel()->getChannelManagementType() == CHANNEL_MANUAL)
                {
                    m_track->setChannel(9);
                }

                m_track->setNotationType(DRUM);
            }
            else if (winX > m_ctrl_button->getX() and winX < m_ctrl_button->getX() + 30)
            {
                m_track->setNotationType(CONTROLLER);
            }
        }

        if (m_track->getNotationType() == SCORE and mousey > m_from_y + 15 and mousey < m_from_y + 30)
        {
            // sharp/flat signs
            if ( m_sharp_flat_picker->getItem(0).clickIsOnThisWidget(winX, mousey) )
            {
                m_track->action( new Action::SetAccidentalSign(SHARP) );
            }
            else if ( m_sharp_flat_picker->getItem(1).clickIsOnThisWidget(winX, mousey) )
            {
                m_track->action( new Action::SetAccidentalSign(FLAT) );
            }
            else if ( m_sharp_flat_picker->getItem(2).clickIsOnThisWidget(winX, mousey) )
            {
                m_track->action( new Action::SetAccidentalSign(NATURAL) );
            }
        }

        return false;
    }
    else
    {
        return true;
    }
}

// --------------------------------------------------------------------------------------------------

bool GraphicalTrack::processRightMouseClick(RelativeXCoord x, int y)
{
    if (y > m_from_y and y < m_to_y)
    {
        getCurrentEditor()->rightClick(x,y);
        return false;
    }
    else
    {
        return true;
    }

}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::processMouseRelease()
{
    //std::cout << "mouse up GraphicalTrack" << std::endl;

    if (not m_dragging_resize)
    {
        getCurrentEditor()->mouseUp(Display::getMouseX_current(), Display:: getMouseY_current(),
                                    Display::getMouseX_initial(), Display:: getMouseY_initial());
    }

    if (m_dragging_resize)
    {
        m_dragging_resize = false;
        DisplayFrame::updateVerticalScrollbar();
    }
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::processMouseExited(RelativeXCoord x_now, int y_now, RelativeXCoord x_initial, int y_initial)
{
    getCurrentEditor()->mouseExited(x_now, y_now, x_initial, y_initial);
}

// --------------------------------------------------------------------------------------------------

bool GraphicalTrack::processMouseDrag(RelativeXCoord x, int y)
{

    if ((y > m_from_y and y < m_to_y) or m_dragging_resize)
    {
        // until the end of the method, mousex_current/mousey_current contain the location of the mouse last time this event was thrown in the dragging process.
        // This can be used to determine the movement of the mouse.
        // At the end of the method, mousex_current/mousey_current are set to the current values.

        int barHeight = EXPANDED_BAR_HEIGHT;
        if (m_collapsed) barHeight = COLLAPSED_BAR_HEIGHT;

        if (not m_dragging_resize)
        {
            getCurrentEditor()->mouseDrag(x, y,
                                          Display::getMouseX_initial(),
                                          Display::getMouseY_initial());
        }

        // resize drag
        if (m_dragging_resize)
        {
            const int TRACK_MIN_SIZE = 35;
            
            if (m_height == TRACK_MIN_SIZE)
            { 
                // if it has reached minimal size, wait until mouse comes back over before resizing again
                if (y > m_to_y - 15 and y < m_to_y - 5 and (y - m_last_mouse_y) > 0)
                {
                    m_height += (y - m_last_mouse_y);
                }
            }
            else
            {
                // resize the track and check if it's not too small
                m_height += (y - m_last_mouse_y);
                if (m_height < TRACK_MIN_SIZE) m_height = TRACK_MIN_SIZE; // enforce minimum size
            }
            
            DisplayFrame::updateVerticalScrollbar();
        }

        m_last_mouse_y = y;

        return false;
    }
    else
    {
        return true;
    }
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::onKeyChange(const int symbolAmount, const KeyType type)
{
    for (int n=0; n<m_all_editors.size(); n++)
    {
        m_all_editors[n].onKeyChange(symbolAmount, type);
    }
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::onInstrumentChange(const int newInstrument, const bool isDrumKit)
{
    if (isDrumKit)
    {
        m_instrument_name.set(DrumChoice::getDrumkitName( newInstrument ));
    }
    else
    {
        m_instrument_name.set(InstrumentChoice::getInstrumentName( newInstrument ));
    }
}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Getters/Setters
#endif

void GraphicalTrack::setCollapsed(const bool collapsed)
{
    m_collapsed = collapsed;
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::setHeight(const int height)
{
    m_height = height;
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::maximizeHeight(bool maximize)
{
    if (maximize)
    {
        setHeight(Display::getHeight() - m_gsequence->getDockHeight() -
                  (m_gsequence->getModel()->getMeasureData()->isExpandedMode() ? 150 : 130)  ); // FIXME - don't hardcode values
    }
    else
    {
        if (m_height > 200) m_height = 200;
    }
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::dock(const bool setDocked)
{
    if (setDocked)
    {
        m_docked = true;
        m_gsequence->addToDock( this );
    }
    else
    {
        m_docked = false;
        m_gsequence->removeFromDock( this );
    }
}

// --------------------------------------------------------------------------------------------------

int GraphicalTrack::getTotalHeight()
{

    if (m_docked) return 0;

    if (m_collapsed)
    {
        return 45; // COLLAPSED_BAR_HEIGHT
    }
    else
    {
        return EXPANDED_BAR_HEIGHT + 50 + m_height;
    }

}

// --------------------------------------------------------------------------------------------------

int GraphicalTrack::getEditorHeight()
{
    return m_height;
}

// --------------------------------------------------------------------------------------------------

Editor* GraphicalTrack::getCurrentEditor()
{
    switch (m_track->getNotationType())
    {
        case KEYBOARD:      return m_keyboard_editor;
        case GUITAR:        return m_guitar_editor;
        case DRUM:          return m_drum_editor;
        case CONTROLLER:    return m_controller_editor;
        case SCORE:         return m_score_editor;
        default :
            std::cerr << "[GraphicalTrack] getCurrentEditor : unknown mode!" << std::endl;
            ASSERT(false);
            return NULL; // shut up warnings
    }
}

// --------------------------------------------------------------------------------------------------

void GraphicalTrack::onNotationTypeChange()
{    
    if (m_track->getNotationType() == DRUM)
    {
        m_instrument_name.set(DrumChoice::getDrumkitName( m_track->getDrumKit() ));
    }
    else
    {
        // only call 'set' if the string really changed, the OpenGL implementation of 'set' involves
        // RTT and is quite expensive.
        wxString name = InstrumentChoice::getInstrumentName( m_track->getInstrument() );
        if (m_instrument_name != name)
        {
            m_instrument_name.set(name);
        }
    }
}

// -------------------------------------------------------------------------------------------------------

int GraphicalTrack::getNoteStartInPixels(const int id) const
{
    return (int)round( m_track->getNoteStartInMidiTicks(id) * m_gsequence->getZoom() );
}

// -------------------------------------------------------------------------------------------------------

int GraphicalTrack::getNoteEndInPixels(const int id) const
{
    return (int)round( m_track->getNoteEndInMidiTicks(id) * m_gsequence->getZoom() );
}

// -------------------------------------------------------------------------------------------------------

void GraphicalTrack::selectNote(const int id, const bool selected, bool ignoreModifiers)
{    
    ASSERT(id != SELECTED_NOTES); // not supported in this function
    
    m_track->selectNote(id, selected, ignoreModifiers);

    // ---- select/deselect all notes
    if (id == ALL_NOTES)
    {
        // if this is a 'select none' command, unselect any selected measures in the top bar
        if (not selected) getSequence()->getMeasureBar()->unselect();
        
        if (m_track->getNotationType() == CONTROLLER)
        {
            // FIXME(DESIGN): controller editor must be handled differently (special case)
            m_controller_editor->selectAll( selected );
        }
    }
}


// --------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Rendering
#endif

void GraphicalTrack::renderHeader(const int x, const int y, const bool closed, const bool focus)
{
    // mark 'dock' button as disabled when maximize mode is activated
    m_dock_toolbar->getItem(1).setImageState(m_gsequence->isTrackMaximized() ?
                                             AriaRender::STATE_GHOST :
                                             AriaRender::STATE_NORMAL );
    
    const bool channel_mode = (m_gsequence->getModel()->getChannelManagementType() == CHANNEL_MANUAL);
    
    int barHeight = EXPANDED_BAR_HEIGHT;
    if (closed) barHeight = COLLAPSED_BAR_HEIGHT;
    
    if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else          AriaRender::setImageState(AriaRender::STATE_NORMAL);
    
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
    cornerDrawable->move(x+Display::getWidth() - 5 /* margin*/ -20 /*left round cornerDrawable*/, y);
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
    
    if (!focus) AriaRender::color(0.31/2, 0.31/2, 0.31/2);
    else AriaRender::color(0.31, 0.31, 0.31);
    
    AriaRender::rect(x+30, y+20, x+ Display::getWidth() - 5 /* margin*/ - 20 /*right round cornerDrawable*/, y+20+barHeight);
    
    
    // --------------------------------------------------
    
    if (closed)
    {
        AriaRender::images();
        
        if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
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
        if (m_track->getNotationType() != KEYBOARD) // keyboard editor draws its own backgound, so no need to draw it twice // FIXME no more true
        {
            AriaRender::primitives();
            AriaRender::color(1, 1, 1);
            AriaRender::rect(x + 10,
                             y + barHeight + 20,
                             x + Display::getWidth() - 5 ,
                             y + barHeight + 40 + m_height);
        }//end if
    }//end if
    
    // ------------------ prepare to draw components ------------------
    if (m_collapsed) m_collapse_button->m_drawable->setImage( expandImg );
    else             m_collapse_button->m_drawable->setImage( collapseImg );
    
    if (m_track->isMuted()) m_mute_button->m_drawable->setImage( muteOnImg );
    else                    m_mute_button->m_drawable->setImage( muteOffImg );
    
    const NotationType notationType = m_track->getNotationType();
    m_score_button -> enable( notationType == SCORE      and focus );
    m_piano_button -> enable( notationType == KEYBOARD   and focus );
    m_tab_button   -> enable( notationType == GUITAR     and focus );
    m_drum_button  -> enable( notationType == DRUM       and focus );
    m_ctrl_button  -> enable( notationType == CONTROLLER and focus );
    
    m_sharp_flat_picker->show(m_track->getNotationType() == SCORE);
    
    m_channel_field->show(channel_mode);
    
    // ------------------ layout and draw components ------------------
    m_components->layout(20, y);
    m_sharp_flat_picker->layout();
    m_grid_combo->layout();
    m_dock_toolbar->layout();
    m_components->renderAll(focus);
    
    //  ------------------ post-drawing  ------------------
    
    // draw track name
    AriaRender::images();
    AriaRender::color(0,0,0);
    AriaRenderString& track_name = m_track->getNameRenderer();
    track_name.bind();
    track_name.render(m_track_name->getX()+11, y+29);
    
    // draw grid label
    int grid_selection_x;
    switch (m_grid->divider)
    {
        case 1:
            grid_selection_x = mgrid_1->getX();
            break;
        case 2:
        case 3:
            grid_selection_x = mgrid_2->getX();
            break;
        case 4:
        case 6:
            grid_selection_x = mgrid_4->getX();
            break;
        case 8:
        case 12:
            grid_selection_x = mgrid_8->getX();
            break;
        case 16:
        case 24:
            grid_selection_x = mgrid_16->getX();
            break;
        case 32:
        case 48:
            grid_selection_x = mgrid_32->getX();
            break;
        default: // length is chosen from drop-down menu
            grid_selection_x = -1;
    }
    
    AriaRender::primitives();
    AriaRender::color(0,0,0);
    AriaRender::hollow_rect(grid_selection_x, y+15, grid_selection_x+16, y+30);
    if (m_grid->isTriplet()) AriaRender::hollow_rect(mgrid_triplet->getX(),      y + 15,
                                                     mgrid_triplet->getX() + 16, y + 30);
    
    // mark maximize mode as on if relevant
    if (m_gsequence->isTrackMaximized())
    {
        const int rectx = m_dock_toolbar->getItem(0).getX();
        AriaRender::hollow_rect(rectx, y+13, rectx+16, y+29);
    }
    
    // draw instrument name
    AriaRender::images();
    AriaRender::color(0,0,0);
    
    m_instrument_name.bind();
    m_instrument_name.render(m_instrument_field->getX()+11 ,y+29);
        
    // draw channel number
    if (channel_mode)
    {
        wxString channelName = to_wxString(m_track->getChannel());
        
        AriaRender::color(0,0,0);
        
        const int char_amount_in_channel_name = channelName.size();
        if (char_amount_in_channel_name == 1)
            AriaRender::renderNumber(channelName, m_channel_field->getX()+10, y+28);
        else
            AriaRender::renderNumber(channelName, m_channel_field->getX()+7, y+28);
    }
    
}

// --------------------------------------------------------------------------------------------------

int GraphicalTrack::render(const int y, const int currentTick, const bool focus)
{
    
    if (not ImageProvider::imagesLoaded()) return 0;
    
    // docked tracks are not drawn
    if (m_docked)
    {
        m_from_y = -1;
        m_to_y = -1;
        return y;
    }
    
    m_from_y = y;
    
    if (m_collapsed) m_to_y = m_from_y + 45;
    else             m_to_y = y + EXPANDED_BAR_HEIGHT + 50 + m_height;
    
    // tell the editor about its new location
    getCurrentEditor()->updatePosition(m_from_y, m_to_y, Display::getWidth(), m_height, EXPANDED_BAR_HEIGHT);
    
    // don't waste time drawing it if out of bounds
    if (m_to_y < 0) return m_to_y;
    if (m_from_y > Display::getHeight()) return m_to_y;
    
    renderHeader(0, y, m_collapsed, focus);
    
    if (not m_collapsed)
    {
        // --------------------------------------------------
        // render editor
        getCurrentEditor()->render(Display::getMouseX_current(),
                                   Display::getMouseY_current(),
                                   Display::getMouseX_initial(),
                                   Display::getMouseY_initial(), focus);
        // --------------------------------------------------
        // render playback progress line
        
        AriaRender::primitives();
        
        if (currentTick != -1 and not Display::leftArrow() and not Display::rightArrow())
        {
            AriaRender::color(0.8, 0, 0);
            
            RelativeXCoord tick(currentTick, MIDI, m_gsequence);
            const int x_coord = tick.getRelativeTo(WINDOW);
            
            AriaRender::lineWidth(1);
            
            AriaRender::line(x_coord, getCurrentEditor()->getEditorYStart(),
                             x_coord, getCurrentEditor()->getYEnd());
            
        }
        
        // --------------------------------------------------
        // render track borders
        
        AriaRender::images();
        
        
        int barHeight = EXPANDED_BAR_HEIGHT;
        if (m_collapsed) barHeight = COLLAPSED_BAR_HEIGHT;
        
        if (not focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
        else           AriaRender::setImageState(AriaRender::STATE_NORMAL);
        
        // bottom left corner (FIXME: remove those hardcoded numbers; they are in Editor too so avoid duplicating)
        whiteCornerDrawable->move(10, y + 20 + barHeight + m_height);
        whiteCornerDrawable->setFlip(false, false);
        whiteCornerDrawable->render();
        
        // bottom border (FIXME: remove those hardcoded numbers; they are in Editor too so avoid duplicating)
        whiteBorderDrawable->move(30, y + 20 + barHeight + m_height);
        whiteBorderDrawable->setFlip(false, false);
        whiteBorderDrawable->rotate(0);
        whiteBorderDrawable->scale((Display::getWidth() - 5 /* margin*/ - 20 /*left round cornerDrawable*/ - 20 /*right round cornerDrawable*/)/20.0, 1 );
        whiteBorderDrawable->render();
        
        // bottom right corner (FIXME: remove those hardcoded numbers; they are in Editor too so avoid duplicating)
        whiteCornerDrawable->move(Display::getWidth() - 5 /* margin*/ - 20 /*left round cornerDrawable*/,
                                  y + 20 + barHeight + m_height);
        whiteCornerDrawable->setFlip(true, false);
        whiteCornerDrawable->render();
        
        // --------------------------------------------------
        
        // left borderDrawable (FIXME: remove those hardcoded numbers; they are in Editor too so avoid duplicating)
        whiteBorderDrawable->move(30, y + barHeight + 20);
        whiteBorderDrawable->setFlip(false, false);
        whiteBorderDrawable->rotate(90);
        whiteBorderDrawable->scale(1, m_height / 20.0 );
        whiteBorderDrawable->render();
        
        // right borderDrawable (FIXME: remove those hardcoded numbers; they are in Editor too so avoid duplicating)
        whiteBorderDrawable->move(Display::getWidth() - 5 , y+barHeight+20);
        whiteBorderDrawable->setFlip(false, true);
        whiteBorderDrawable->rotate(90);
        whiteBorderDrawable->scale(1, m_height / 20.0 );
        whiteBorderDrawable->render();
        
    }
    
    AriaRender::images();
    
    // done
    return m_to_y;
}

// --------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Serialization
#endif

void GraphicalTrack::saveToFile(wxFileOutputStream& fileout)
{
    const int octave_shift = m_score_editor->getScoreMidiConverter()->getOctaveShift();

    // TODO: move notation type to "Track"
    writeData( wxT("<editor mode=\"") + to_wxString(m_track->getNotationType()) +
               wxT("\" height=\"") + to_wxString(m_height) +
               (m_collapsed ? wxT("\" collapsed=\"true") : wxT("")) +
               wxT("\" g_clef=\"") + (m_score_editor->isGClefEnabled()?wxT("true"):wxT("false")) +
               wxT("\" f_clef=\"") + (m_score_editor->isFClefEnabled()?wxT("true"):wxT("false")) +
               ( octave_shift != 0 ? wxT("\" octave_shift=\"")+to_wxString(octave_shift) : wxT("")) +
               wxT("\"/>\n")
               , fileout );

    m_grid->saveToFile( fileout );
    //keyboardEditor->instrument->saveToFile(fileout);
    //drumEditor->drumKit->saveToFile(fileout);

    // TODO: move this to 'Track', has nothing to do here in GraphicalTrack
    writeData( wxT("<instrument id=\"") + to_wxString( m_track->getInstrument() ) + wxT("\"/>\n"), fileout);
    writeData( wxT("<drumkit id=\"") + to_wxString( m_track->getDrumKit() ) + wxT("\"/>\n"), fileout);

    // guitar tuning (FIXME: move this out of here)
    writeData( wxT("<guitartuning "), fileout);
    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    const int stringCount = tuning->tuning.size();
    for (int n=0; n<stringCount; n++)
    {
        writeData(wxT(" string")+ to_wxString((int)n) + wxT("=\"") +
                  to_wxString((int)tuning->tuning[n]) + wxT("\""), fileout );
    }

    writeData( wxT("/>\n\n"), fileout);

}

// --------------------------------------------------------------------------------------------------

bool GraphicalTrack::readFromFile(irr::io::IrrXMLReader* xml)
{

    if (strcmp("editor", xml->getNodeName()) == 0)
    {

        // TODO: move notation type to "Track"
        const char* mode_c = xml->getAttributeValue("mode");
        if ( mode_c != NULL )
        {
            m_track->setNotationType((NotationType)atoi( mode_c ));
        }
        else
        {
            m_track->setNotationType(KEYBOARD);
            std::cout << "Missing info from file: editor mode" << std::endl;
        }

        const char* height_c = xml->getAttributeValue("height");
        if (height_c != NULL)
        {
            m_height = atoi( height_c );
        }
        else
        {
            std::cout << "Missing info from file: track height" << std::endl;
            m_height = 200;
        }

        const char* collapsed_c = xml->getAttributeValue("collapsed");
        if (collapsed_c != NULL)
        {
            if (strcmp(collapsed_c, "true") == 0)
            {
                m_collapsed = true;
            }
            else if (strcmp(collapsed_c, "false") == 0)
            {
                m_collapsed = false;
            }
            else
            {
                std::cout << "Unknown keyword for attribute 'collapsed' in track: " << collapsed_c << std::endl;
                m_collapsed = false;
            }

        }
        else
        {
            m_collapsed = false;
        }


        const char* g_clef_c = xml->getAttributeValue("g_clef");
        if (g_clef_c != NULL)
        {
            if (strcmp(g_clef_c, "true") == 0)
            {
                m_score_editor->enableGClef(true);
            }
            else if (strcmp(g_clef_c, "false") == 0)
            {
                m_score_editor->enableGClef(false);
            }
            else
            {
                std::cout << "Unknown keyword for attribute 'g_clef' in track: " << g_clef_c << std::endl;
            }

        }
        const char* f_clef_c = xml->getAttributeValue("f_clef");
        if (f_clef_c != NULL)
        {
            if (strcmp(f_clef_c, "true") == 0)
            {
                m_score_editor->enableFClef(true);
            }
            else if (strcmp(f_clef_c, "false") == 0)
            {
                m_score_editor->enableFClef(false);
            }
            else
            {
                std::cerr << "[GraphicalTrack] readFromFile() : Unknown keyword for attribute 'f_clef' in track: " << f_clef_c << std::endl;
            }

        }
        const char* octave_shift_c = xml->getAttributeValue("octave_shift");
        if ( octave_shift_c != NULL )
        {
            int new_value = atoi( octave_shift_c );
            if (new_value != 0) m_score_editor->getScoreMidiConverter()->setOctaveShift(new_value);
        }
        
        // compatibility code for older versions of .Aria file format (TODO: eventuall remove)
        const char* muted_c = xml->getAttributeValue("muted");
        if (muted_c != NULL)
        {
            if (strcmp(muted_c, "true") == 0)
            {
                m_track->setMuted(true);
            }
            else if (strcmp(muted_c, "false") == 0)
            {
                m_track->setMuted(false);
            }
            else
            {
                std::cerr << "Unknown keyword for attribute 'muted' in track: " << muted_c << std::endl;
            }
            
        }

    }
    else if (strcmp("magneticgrid", xml->getNodeName()) == 0)
    {
        if (not m_grid->readFromFile(xml)) return false;
    }
    else if (strcmp("guitartuning", xml->getNodeName()) == 0)
    {
        GuitarTuning* tuning = m_track->getGuitarTuning();

        std::vector<int> newTuning;
        
        int n=0;
        char* string_v = (char*)xml->getAttributeValue("string0");

        while (string_v != NULL)
        {
            newTuning.push_back( atoi(string_v) );

            n++;
            wxString tmp = wxT("string") + to_wxString(n);
            string_v = (char*)xml->getAttributeValue( tmp.mb_str() );
        }

        if (newTuning.size() < 3)
        {
            std::cout << "FATAL ERROR: Invalid tuning!! only " << newTuning.size() 
                      << " strings found" << std::endl;
            return false;
        }

        tuning->setTuning(newTuning, false);
    }

    return true;

}

int GraphicalTrack::getGridDivider() const
{
    return m_grid->divider;
}


