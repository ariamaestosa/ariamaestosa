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


#include "AriaCore.h"

#include "Actions/EditAction.h"
#include "Actions/AddControlEvent.h"
#include "Actions/AddControllerSlide.h"
#include "Editors/ControllerEditor.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/ImageProvider.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/ControllerChoice.h"
#include "Pickers/TuningPicker.h"
#include "Renderers/RenderAPI.h"

#include <string>
#include <cmath>


using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

ControllerEditor::ControllerEditor(Track* track) : Editor(track)
{
    m_mouse_is_in_editor=false;

    m_selection_begin = -1;
    m_selection_end = -1;

    m_has_been_resizing = false;

    m_controller_choice = new ControllerChoice();
}

// ----------------------------------------------------------------------------------------------------------

ControllerEditor::~ControllerEditor()
{
}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::renderEvents()
{
    const int area_from_y = getEditorYStart()+7;
    const int area_to_y = getYEnd()-15;
    const float y_zoom = (float)( area_to_y-area_from_y ) / 127.0;

    AriaRender::color(0, 0.4, 1);
    AriaRender::lineWidth(3);

    ControllerEvent* tmp;
    int previous_location=-1, previous_value=-1;

    const int currentController = m_controller_choice->getControllerID();

    const int eventAmount = m_track->getControllerEventAmount( Track::isTempoController(currentController) );
    const int x_scroll = m_gsequence->getXScrollInPixels();


    int eventsOfThisType=0;
    for (int n=0; n<eventAmount; n++)
    {
        tmp = m_track->getControllerEvent(n, currentController);
        if (tmp->getController() != currentController) continue; // only draw events of this controller
        eventsOfThisType++;

        const int xloc = tmp->getPositionInPixels(m_gsequence);
        const unsigned short value = tmp->getValue();

        if (previous_location - x_scroll > getXEnd()) // if events are no more visible, stop drawing
            return;

        if (xloc - x_scroll > Editor::getEditorXStart())
        {
            if (previous_location != -1 and previous_value != -1)
            {
                AriaRender::line(previous_location - x_scroll,
                                 area_from_y + previous_value*y_zoom,
                                 xloc - x_scroll,
                                 area_from_y + previous_value*y_zoom);
            }
        }

        previous_location = xloc;
        previous_value = value;
    }// next

    // draw horizontal line drom last event to end of visible area
    if (eventsOfThisType>0)
    {
        AriaRender::line(previous_location - x_scroll,
                         area_from_y + previous_value*y_zoom,
                         getXEnd(),
                         area_from_y + previous_value*y_zoom);
    }

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::render(RelativeXCoord mousex_current, int mousey_current,
                              RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    AriaRender::beginScissors(10, getEditorYStart(), m_width - 15, 20 + m_height);

    // -------------------------------- background ----------------------------

    const int area_from_y = getEditorYStart()+7;
    const int area_to_y = getYEnd()-15;

    AriaRender::primitives();

    AriaRender::color(0.9, 0.9, 0.9);
    AriaRender::rect(0, 0, getXEnd(), area_from_y);
    AriaRender::rect(0, area_to_y,getXEnd(), getYEnd());

    AriaRender::color(1,1,1);
    AriaRender::rect(0, area_from_y, getXEnd(), area_to_y);

    // -------------------------- selection ------------------------

    if (m_selection_begin != -1 and focus)
    {

        RelativeXCoord selectX1(m_selection_begin, MIDI, m_gsequence);
        RelativeXCoord selectX2(m_selection_end, MIDI, m_gsequence);

        AriaRender::color(0.8, 0.9, 1);
        AriaRender::rect(selectX1.getRelativeTo(WINDOW) , area_from_y,
                         selectX2.getRelativeTo(WINDOW) , area_to_y);

    }

    // ----------------------------------- middle line -----------------------
    AriaRender::color(0.9, 0.9, 0.9);

    // tempo
    if (m_controller_choice->getControllerID() == 201)
    {

        // top value is 500, bottom value is 0, find where to put middle value for it to be main tempo
        const int liney = (int)(
                                area_to_y - (area_to_y-area_from_y)*((m_sequence->getTempo()-20) / 380.0)
                                );
        AriaRender::line(0, liney, getXEnd(), liney);
    }
    else
        // all others
    {
        AriaRender::line(0, (area_from_y+area_to_y)/2, getXEnd(), (area_from_y+area_to_y)/2);
    }

    drawVerticalMeasureLines(area_from_y, area_to_y);

    // ------------------------ min/max, on/off, left/right, etc. -------------------
    AriaRender::images();
    AriaRender::color(0.5, 0.5, 0.5);

    m_controller_choice->renderTopLabel(Editor::getEditorXStart()+5 , area_from_y + 13);
    m_controller_choice->renderBottomLabel(Editor::getEditorXStart()+5 , area_to_y);
    
    AriaRender::primitives();
    
    // -------------------------- draw controller events ---------------------
    renderEvents();

    // ----------------------- add controller events (preview) -------------------
    if (m_track->graphics->m_dragging_resize) m_has_been_resizing = true;

    const bool on_off = m_controller_choice->isOnOffController( m_controller_choice->getControllerID() );
    if (m_mouse_is_in_editor and m_selection_begin == -1 and not on_off)
    {

        AriaRender::lineWidth(3);
        AriaRender::color(0, 0.4, 1);

        if (mousey_initial >= area_from_y and mousey_initial <= area_to_y and not m_has_been_resizing)
        {

            // if out of bounds
            if (mousey_current<area_from_y) mousey_current=area_from_y;
            if (mousey_current>area_to_y) mousey_current=area_to_y;

            int tick1 = snapMidiTickToGrid( mousex_initial.getRelativeTo(MIDI));
            int tick2 = snapMidiTickToGrid( mousex_current.getRelativeTo(MIDI));

            if (tick2 < 0) tick2 = 0;
            if (tick1 < 0) tick1 = 0;

            AriaRender::line((tick1 - m_gsequence->getXScrollInMidiTicks()) *
                             m_gsequence->getZoom() + Editor::getEditorXStart(), mousey_initial,
                             (tick2 - m_gsequence->getXScrollInMidiTicks()) *
                             m_gsequence->getZoom() + Editor::getEditorXStart(), mousey_current);
        }
    }
    AriaRender::lineWidth(1);

    // -----------------------------------------------------------------
    // left part with names
    // -----------------------------------------------------------------

    // grey background
    if (not focus) AriaRender::color(0.4, 0.4, 0.4);
    else           AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0, getEditorYStart(),
                     Editor::getEditorXStart(), getYEnd());

    // controller name
    AriaRender::images();
    AriaRender::color(0,0,0);

    m_controller_choice->renderControllerName(15, getEditorYStart() + 15);
    
    AriaRender::endScissors();

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::mouseDown(RelativeXCoord x, const int y)
{

    m_has_been_resizing = false;

    // prepare coords
    m_selection_begin = -1;
    m_selection_end   = -1;

    // check if user is dragging on this track
    m_mouse_is_in_editor = false;
    
    if (y < getYEnd()-15 and y > getEditorYStart() and
       x.getRelativeTo(WINDOW) < getWidth() - 24 and
       x.getRelativeTo(EDITOR) > -1)
    {
        m_mouse_is_in_editor = true;
    }

    // check whether we're selecting
    m_selecting = false;
    if (m_mouse_is_in_editor and Display::isSelectMorePressed())
    {
        m_selecting = true;
    }

    if (x.getRelativeTo(WINDOW) < Editor::getEditorXStart() and y > getEditorYStart() and
        not m_track->graphics->isCollapsed() )
    {
        Display::popupMenu(m_controller_choice,x.getRelativeTo(WINDOW),y+15);
    }

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::mouseDrag(RelativeXCoord mousex_current, const int mousey_current,
                                 RelativeXCoord mousex_initial, const int mousey_initial)
{

    if (m_mouse_is_in_editor and m_selecting)
    {

        // ------------------------ select ---------------------
        m_selection_begin = snapMidiTickToGrid( mousex_initial.getRelativeTo(MIDI) );
        m_selection_end   = snapMidiTickToGrid( mousex_current.getRelativeTo(MIDI) );

    }

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
                               RelativeXCoord mousex_initial, int mousey_initial)
{

    if (m_mouse_is_in_editor)
    {
        if (m_selecting)
        {
            // ------------------------ select ---------------------

            m_selection_begin = snapMidiTickToGrid( mousex_initial.getRelativeTo(MIDI) );
            m_selection_end   = snapMidiTickToGrid( mousex_current.getRelativeTo(MIDI) );

            // no selection
            if (m_selection_begin == m_selection_end)
            {
                m_selection_begin = -1;
                m_selection_end = -1;
            }
            m_selecting = false;
        }
        else
        {

            m_selection_begin = -1;
            m_selection_end   = -1;

            const int area_from_y = getEditorYStart() + 7;
            const int area_to_y   = getYEnd() - 15;
            const float y_zoom    = (float)( area_to_y - area_from_y ) / 127.0;

            // ------------------------ add controller events ---------------------

            // if mouse is out of bounds
            if (mousex_initial.getRelativeTo(WINDOW) < Editor::getEditorXStart()) return;


            if (mousey_initial < area_from_y) return;
            if (mousey_initial > area_to_y)   return;
            if (m_track->graphics->m_dragging_resize or m_has_been_resizing) return;
            if (mousey_current < area_from_y) mousey_current=area_from_y;
            if (mousey_current > area_to_y) mousey_current=area_to_y;

            int tick1 = snapMidiTickToGrid( mousex_initial.getRelativeTo(MIDI) );
            int tick2 = snapMidiTickToGrid( mousex_current.getRelativeTo(MIDI) );

            if (tick2 < 0) tick2 = 0;
            if (tick1 < 0) tick1 = 0;

            const bool on_off = m_controller_choice->isOnOffController( m_controller_choice->getControllerID() );
            if (tick1 == tick2)
            {
                int y_value = (int)round((float)(mousey_initial-area_from_y)/y_zoom);
                if ( on_off )
                {
                    // on/off controllers should only use values 0 and 127
                    if  (y_value < 64) y_value = 0;
                    else               y_value = 127;
                }

                m_track->action( new Action::AddControlEvent(tick1,
                                                             y_value,
                                                             m_controller_choice->getControllerID()) );
            }
            else if (not on_off) // on/off controllers can't have slides
            {
                if (tick1 < tick2) 
                {
                    m_track->action( new Action::AddControllerSlide(tick1,
                                                       (int)( (mousey_initial - area_from_y)/y_zoom ),
                                                       tick2,
                                                       (int)( (mousey_current - area_from_y)/y_zoom ),
                                                       m_controller_choice->getControllerID()) );
                }
                else
                {
                    m_track->action( new Action::AddControllerSlide(tick2,
                                                       (int)( (mousey_current - area_from_y)/y_zoom ),
                                                       tick1,
                                                       (int)( (mousey_initial - area_from_y)/y_zoom ),
                                                       m_controller_choice->getControllerID()) );
                }
            } // end if tick1==tick2

        } // end if meta down
    }

    m_mouse_is_in_editor=false;


    Display::render();

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::rightClick(RelativeXCoord x, const int y)
{

}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
                                   RelativeXCoord mousex_initial, int mousey_initial)
{
    // if mouse leaves the frame, it has the same effect as if it was released (terminate drag, terminate selection, etc.)
    this->mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void ControllerEditor::selectAll( bool selected )
{

    // Select none
    if (not selected)
    {
        m_selection_begin = -1;
        m_selection_end   = -1;
    }
    else
    {
        // Select all
        m_selection_begin = 0;
        m_selection_end   = m_sequence->getMeasureData()->getTotalTickAmount();
    }

}

// ----------------------------------------------------------------------------------------------------------
