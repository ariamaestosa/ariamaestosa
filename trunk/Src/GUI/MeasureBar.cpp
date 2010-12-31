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

#include "Utils.h"

#include <wx/numdlg.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include "Actions/EditAction.h"
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/RemoveMeasures.h"
#include "AriaCore.h"
#include "Editors/Editor.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "GUI/MeasureBar.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/TimeSigChange.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/TimeSigPicker.h"
#include "Renderers/RenderAPI.h"

#include <iostream>


namespace AriaMaestosa
{

  int insert_at_measure = -1;
  int remove_from = -1;
  int remove_to = -1;

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
    
/**
  * @brief This pop-up menu that is shown if you right-click on a selected measure in the measure bar
  */
class SelectedMenu : public wxMenu
{
    GraphicalSequence* m_gseq;

public:
    LEAK_CHECK();
    
    SelectedMenu(GraphicalSequence* parent) : wxMenu()
    {
        Append(1, _("Remove selected measures"));
        m_gseq = parent;
    }

    /**
      * @brief Removes selected measures
      */
    void removeSelected(wxCommandEvent& event)
    {
        // ask confirmation
        int answer = wxMessageBox(  _("Do you really want to remove selected measures?"), wxT(""), wxYES_NO);
        if (answer == wxNO or answer == wxCANCEL) return;
        if (answer != wxYES and answer != wxOK)
        {
            std::cerr << "Unknown answer from wxMessageBox in SelectedMenu::removeSelected() : "
                      << answer << std::endl;
        }

        m_gseq->getMeasureBar()->unselect();
        
        Sequence* seq = m_gseq->getModel();
        
        seq->action( new Action::RemoveMeasures(remove_from, remove_to) );
        m_gseq->getMeasureBar()->selectTimeSig(0);
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(SelectedMenu, wxMenu)
EVT_MENU(1,SelectedMenu::removeSelected)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

/**
  * @brief This pop-up menu that is shown if you right-click on an unselected measure in the measure bar
  */
class UnselectedMenu : public wxMenu
{
    wxMenuItem* deleteTimeSig;
    int remove_timeSigID;
    
    GraphicalSequence* m_gseq;

public:
    LEAK_CHECK();

    UnselectedMenu(GraphicalSequence* gseq) : wxMenu()
    {
        Append(2, _("Insert measures"));
        remove_timeSigID = -1;
        deleteTimeSig = Append(3, _("Remove time sig change"));
        m_gseq = gseq;
    }

    void insert(wxCommandEvent& event)
    {
        int number = wxGetNumberFromUser(  _("Insert empty measures between measures ") +
                                           to_wxString(insert_at_measure+1) +
                                           _(" and ") + to_wxString(insert_at_measure+2),
                                           _("Amount: "), wxT(""), 4 /*default*/, 1 /*min*/);
        if (number==-1) return;

        Sequence* seq = m_gseq->getModel();

        // --------- move notes in all tracks -----------

        seq->action( new Action::InsertEmptyMeasures(insert_at_measure+1, number) );

    }

    void enable_deleteTimeSig_item(bool enabled, int timeSigID=-1)
    {
        deleteTimeSig->Enable(enabled);
        if (enabled) remove_timeSigID = timeSigID;
    }

    void removeTimeSig(wxCommandEvent& event)
    {
        int answer = wxMessageBox(_("Do you really want to remove this time sig change?"), wxT(""),
                                  wxYES_NO);
        if (answer == wxNO or answer == wxCANCEL) return;

        if (remove_timeSigID == 0)
        {
            wxMessageBox( _("You can't remove the first time signature change."));
            return;
        }
        
        ASSERT(remove_timeSigID != -1);

        Sequence* seq = m_gseq->getModel();
        MeasureData* md = seq->getMeasureData();
        
        {
            ScopedMeasureTransaction tr(md->startTransaction());
            tr->eraseTimeSig(remove_timeSigID);
        }
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(UnselectedMenu, wxMenu)
EVT_MENU(2,UnselectedMenu::insert)
EVT_MENU(3,UnselectedMenu::removeTimeSig)
END_EVENT_TABLE()

}

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

MeasureBar::MeasureBar(MeasureData* parent, GraphicalSequence* gseq)
{
    m_selected_menu   = new SelectedMenu(gseq);
    m_unselected_menu = new UnselectedMenu(gseq);

    m_last_measure_in_drag = -1;

    m_data = parent;
    m_gseq = gseq;
}

// ----------------------------------------------------------------------------------------------------------

MeasureBar::~MeasureBar()
{
}


// ----------------------------------------------------------------------------------------------------------

int MeasureBar::getMeasureBarHeight()
{
    if (m_data->isExpandedMode()) return 40;
    else                          return 20;
}

// ----------------------------------------------------------------------------------------------------------

float MeasureBar::measureLengthInPixels(int measure)
{
    if (measure == -1) measure = 0; // no parameter passed, use measure 0 settings
    
    return (float)m_data->measureLengthInTicks(measure) * (float)m_gseq->getZoom();
}

// ----------------------------------------------------------------------------------------------------------

int MeasureBar::measureAtPixel(int pixel)
{
    const float x1 = 90 - m_gseq->getXScrollInPixels();
    pixel -= (int)x1;
    
    if (m_data->isMeasureLengthConstant())
    {
        if (pixel < 0) pixel = 0;
        // length of a measure
        const float xstep = measureLengthInPixels();
        
        return (int)( pixel/xstep );
    }
    else
    {
        const float zoom = m_gseq->getZoom();
        
        if (pixel < 0) return 0;
        
        const int amount = m_data->m_measure_info.size();
        for (int n=0; n<amount; n++)
        {
            if (n == amount - 1) return amount - 1; // we hit end, return the last
            
            const int pixel_of_n        = m_data->m_measure_info[n].tick * zoom;
            const int pixel_of_n_plus_1 = m_data->m_measure_info[n+1].tick * zoom;

            if (pixel_of_n <= pixel and pixel_of_n_plus_1 > pixel)
            {
                return n;
            }
        }
        
        return 0;
    }
}

// ----------------------------------------------------------------------------------------------------------

void MeasureBar::render(int m_measure_bar_y_arg)
{
    m_measure_bar_y = m_measure_bar_y_arg;

    // if measure amount changed and MeasureBar is out of sync with its current number of measures, fix it
    if ((int)m_data->m_measure_info.size() != m_data->m_measure_amount)
    {
        m_data->updateVector(m_data->m_measure_amount);
    }
    
    const int height = (m_data->isExpandedMode() ? EXPANDED_MEASURE_BAR_H : MEASURE_BAR_H);
    AriaRender::primitives();
    AriaRender::color(1, 1, 0.9);
    AriaRender::rect(0, m_measure_bar_y, Display::getWidth(), m_measure_bar_y + height);

    // black line at the top and bottom
    AriaRender::color(0, 0, 0);

    AriaRender::line(0, m_measure_bar_y, Display::getWidth(), m_measure_bar_y);
    AriaRender::line(0,                   m_measure_bar_y + MEASURE_BAR_H,
                     Display::getWidth(), m_measure_bar_y + MEASURE_BAR_H);

    if (m_data->isExpandedMode()) AriaRender::line(0,                   m_measure_bar_y + EXPANDED_MEASURE_BAR_H,
                                                   Display::getWidth(), m_measure_bar_y + EXPANDED_MEASURE_BAR_H);


    // vertical lines and mesure ID
    int measureID = 0;
    AriaRender::color(0,0,0);

    const bool measureLengthConstant = m_data->isMeasureLengthConstant();

    const float x_initial = Editor::getEditorXStart() - m_gseq->getXScrollInPixels();
    const float x_step = measureLengthInPixels();

    const int width = Display::getWidth();
    const float zoom = m_gseq->getZoom();
    
    for (float n=x_initial; n<width;
        (measureLengthConstant ? n+=x_step : n += measureLengthInPixels(measureID-1)) )
    {
        measureID++;
        if (measureID > m_data->m_measure_amount) break;

        // if measure is selected, draw in blue
        if (m_data->m_measure_info[measureID-1].selected)
        {

            AriaRender::color(0.71, 0.84, 1);

            if (measureLengthConstant)
            {
                AriaRender::rect(n, m_measure_bar_y + 1, n + x_step, m_measure_bar_y + MEASURE_BAR_H - 1);
            }
            else
            {
                const int mw = m_data->m_measure_info[measureID-1].widthInTicks * zoom;
                AriaRender::rect(n, m_measure_bar_y + 1, n + mw, m_measure_bar_y + MEASURE_BAR_H - 1);
            }
            
            AriaRender::color(0,0,0);
        }

        // vertical line
        AriaRender::line(n, m_measure_bar_y, n, m_measure_bar_y + MEASURE_BAR_H);

        // measure ID
        AriaRender::images();
        AriaRender::renderNumber(measureID, n + 5, m_measure_bar_y + MEASURE_BAR_H - 2);

        AriaRender::primitives();
        if (m_data->isExpandedMode())
        {
            const int amount = m_data->m_time_sig_changes.size();
            for (int i=0; i<amount; i++)
            {

                if (m_data->m_time_sig_changes[i].getMeasure() == measureID-1)
                {
                    const bool selected = (m_data->m_selected_time_sig == i);
                    if (selected)
                    {
                        AriaRender::pointSize(11);
                        AriaRender::color(1,0,0);
                    }
                    else
                    {
                        AriaRender::pointSize(7);
                        AriaRender::color(0.5,0,0);
                    }
                    AriaRender::point(n, m_measure_bar_y + 30);

                    if (selected) AriaRender::color(0.5,0,0);
                    else          AriaRender::color(0,0,0);

                    AriaRender::pointSize(1);
                    AriaRender::images();

                    AriaRender::renderNumber(m_data->m_time_sig_changes[i].getDenom(), n + 18, m_measure_bar_y + 42 );
                    AriaRender::renderNumber(m_data->m_time_sig_changes[i].getNum(),   n + 10, m_measure_bar_y + 33 );
                    AriaRender::primitives();

                    AriaRender::color(0,0,0);
                    break;
                }
            } // next time sig change
        } // end if expanded mode
    } // next

}

// ----------------------------------------------------------------------------------------------------------

int MeasureBar::measureDivisionAt(int pixel)
{
    ASSERT_E(m_data->m_measure_amount, ==, (int)m_data->m_measure_info.size());
    const float x1 = 90 - m_gseq->getXScrollInPixels();
    
    if (m_data->isMeasureLengthConstant())
    {
        const float xstep = measureLengthInPixels();
        
        return (int)( ( pixel - x1 + xstep/2)/xstep );
    }
    else
    {
        const float zoom = m_gseq->getZoom();
        
        pixel -= (int)x1;
        const int measureAmount = m_data->m_measure_info.size();
        for (int n=0; n<measureAmount; n++)
        {
            const int pixel_of_n     = m_data->m_measure_info[n].tick * zoom;
            const int width_of_n     = m_data->m_measure_info[n].widthInTicks * zoom;
            const int end_pixel_of_n = pixel_of_n + width_of_n;

            if (pixel >= pixel_of_n    - width_of_n/2 and
                pixel < end_pixel_of_n - width_of_n/2)
            {
                return n;
            }
        }
        return measureAmount-1;
    }
    
}

// ----------------------------------------------------------------------------------------------------------

int MeasureBar::firstPixelInMeasure(int id)
{
    ASSERT_E(m_data->m_measure_amount, ==, (int)m_data->m_measure_info.size());
    if (m_data->isMeasureLengthConstant())
    {
        return (int)(
                     id * m_data->measureLengthInTicks() * m_gseq->getZoom() -
                     m_gseq->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)m_data->m_measure_info.size());
        return m_data->m_measure_info[id].tick*m_gseq->getZoom() - m_gseq->getXScrollInPixels() + 90;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureBar::lastPixelInMeasure(int id)
{
    ASSERT_E(m_data->m_measure_amount, ==, (int)m_data->m_measure_info.size());
    
    if (m_data->isMeasureLengthConstant())
    {
        return (int)(
                     (id+1) * m_data->measureLengthInTicks() * m_gseq->getZoom() -
                     m_gseq->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)m_data->m_measure_info.size());
        return m_data->m_measure_info[id].endTick*m_gseq->getZoom() - m_gseq->getXScrollInPixels() + 90;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureBar::getTotalPixelAmount()
{
    if (m_data->isMeasureLengthConstant())
    {
        return (int)( m_data->m_measure_amount * m_data->measureLengthInTicks() * m_gseq->getZoom() );
    }
    else
    {
        return m_data->totalNeededLengthInTicks * m_gseq->getZoom();
    }
}

// ----------------------------------------------------------------------------------------------------------

// right now, it's just the first measure that is considered "default". This may need to be reviewed.
// (i'm not sure if this is used at all or very much)
float MeasureBar::defaultMeasureLengthInPixels()
{
    return (float)m_data->measureLengthInTicks(0) * (float)m_gseq->getZoom();
}

// ----------------------------------------------------------------------------------------------------------

void MeasureBar::unselect()
{
    if (not m_data->m_something_selected) return;
    m_data->m_something_selected = false;
    
    const int measureAmount = m_data->m_measure_info.size();
    for (int n=0; n<measureAmount; n++)
    {
        m_data->m_measure_info[n].selected = false;
    }
    
    m_last_measure_in_drag = -1;
}

// ----------------------------------------------------------------------------------------------------------

/**
  * @brief When mouse button is pushed on Measure Bar
  */
void MeasureBar::mouseDown(int x, int y)
{
    // FIXME: stop directly accessing the model's internal fields, call a method and let the model handle itself
    
    // if click is in time sig change areas
    if (y > 20)
    {
        const int measureDivAt_x = measureDivisionAt(x);

        // we use the 'add' method, however if on event already exists at this location
        // it will be selected and none will be added
        const int id = m_data->addTimeSigChange(measureDivAt_x, -1, -1);

        selectTimeSig(id);
        
        // m_selected_time_sig = n + 1;
        // FIXME(DESIGN): we should not *tell* the main frame to update the change shown time sig, the listeners should do it
        getMainFrame()->changeShownTimeSig(m_data->m_time_sig_changes[id].getNum(),
                                           m_data->m_time_sig_changes[id].getDenom() );
        
        if (not m_gseq->getModel()->importing)
        {
            wxPoint pt = wxGetMousePosition();
            showTimeSigPicker(m_gseq, pt.x, pt.y,
                              m_data->m_time_sig_changes[id].getNum(),
                              m_data->m_time_sig_changes[id].getDenom() );
            m_data->updateMeasureInfo();
        }
        
        return;
    }

    const int measure_amount = m_data->m_measure_info.size();
    for (int n=0; n<measure_amount; n++)
    {
        m_data->m_measure_info[n].selected = false;
    }

    const int measure_vectorID = measureAtPixel( x );
    m_data->m_measure_info[measure_vectorID].selected = true;

    m_data->m_something_selected = true;
    m_last_measure_in_drag = measure_vectorID;
}

// ----------------------------------------------------------------------------------------------------------

void MeasureBar::selectTimeSig(const int id)
{
    // FIXME(DESIGN): instead use an observer pattern to know when the selected time sig has changed
    m_data->m_selected_time_sig = id;
    getMainFrame()->changeShownTimeSig(m_data->m_time_sig_changes[id].getNum(),
                                       m_data->m_time_sig_changes[id].getDenom() );
}


// ----------------------------------------------------------------------------------------------------------

/**
  * @brief When mouse is held down and dragged on Measure Bar
  */
void MeasureBar::mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial)
{
    const int measure_vectorID = measureAtPixel(mousex_current);

    if (m_last_measure_in_drag == -1) return; //invalid

    m_data->m_measure_info[measure_vectorID].selected = true;
    m_data->m_something_selected = true;

    // previous measure seelcted and this one and contiguous - select all other measures inbetween
    if ( abs(m_last_measure_in_drag-measure_vectorID) > 1 )
    {

        for (int n=measure_vectorID; n != m_last_measure_in_drag;
             n += (m_last_measure_in_drag - measure_vectorID) / abs(m_last_measure_in_drag - measure_vectorID))
        {
            m_data->m_measure_info[n].selected = true;
        }
    }

    m_last_measure_in_drag = measure_vectorID;
}

// ----------------------------------------------------------------------------------------------------------

/**
  * @brief Selection is done, select notes contained in the selected measures
  */
void MeasureBar::mouseUp(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial)
{
    if (m_last_measure_in_drag == -1) return; //invalid

    Sequence* sequence = m_gseq->getModel();

    const int measureAmount = m_data->m_measure_info.size();

    // determine selection range in midi ticks
    int minimal_tick = -1, maximal_tick = -1;
    for (int n=0; n<measureAmount; n++)
    {
        // iterate through measures to find the first selected one
        if (m_data->m_measure_info[n].selected)
        {
            // we found a first selected measure, remember it as minimal tick
            minimal_tick = m_data->firstTickInMeasure(n);
            do{ n++; } while (m_data->m_measure_info[n].selected); // skip all uneslected measures
            maximal_tick = m_data->firstTickInMeasure(n);
            break;
        }
    }

    // iterate through notes and select those that are within the selection range just found
    const int trackAmount = sequence->getTrackAmount();
    for (int track=0; track<trackAmount; track++)
    {
        const int noteAmount = sequence->getTrack(track)->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            const int note_tick = sequence->getTrack(track)->getNoteStartInMidiTicks(n);
            
            // note is within selection range? if so, select it, else unselect it.
            if ( note_tick >= minimal_tick and note_tick < maximal_tick )
            {
                sequence->getTrack(track)->graphics->selectNote(n, true, true);
            }
            else
            {
                sequence->getTrack(track)->graphics->selectNote(n, false, true);
            }
        }
    }

}

// ----------------------------------------------------------------------------------------------------------

void MeasureBar::rightClick(int x, int y)
{

    if (y > 20)
    {
        // find between which measures the user clicked
        int measure = measureDivisionAt(x);

        // check if click is on time sig event
        const int amount = m_data->m_time_sig_changes.size();
        for (int n=0; n<amount; n++)
        {
            if (m_data->m_time_sig_changes[n].getMeasure() == measure)
            {
                std::cout << "trying to delete measure " << n << std::endl;
                m_unselected_menu->enable_deleteTimeSig_item(true, n);
                Display::popupMenu( (wxMenu*)m_unselected_menu, x, y+20);
                return;
            }
        }

    }

    const int measure_vectorID = measureAtPixel( x );

    // is the clicked measure selected?
    if (m_data->m_measure_info[measure_vectorID].selected)
    {
        const int measureAmount = m_data->m_measure_info.size();
        for (int n=0; n<measureAmount; n++)
        {
            // iterate through measures to find which measures are selected
            if (m_data->m_measure_info[n].selected)
            {
                // we found a first selected measure, remember it
                remove_from = n;
                do{ n++; } while (m_data->m_measure_info[n].selected); // skip all selected measures
                remove_to = n;
                break;
            }
        }

        // unselect
        //for (int n=remove_from; n<remove_to+1; n++)
        //{
        //    data->m_measure_info[n].selected = false;
        //}
        Display::popupMenu( (wxMenu*)m_selected_menu, x, y+20);
    }
    else
    {
        // find between which measures the user clicked
        insert_at_measure = measureDivisionAt(x) - 1;

        m_unselected_menu->enable_deleteTimeSig_item(false);
        Display::popupMenu( (wxMenu*)m_unselected_menu, x, y+20);
    }

}
