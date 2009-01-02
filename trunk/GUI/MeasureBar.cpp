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

#include "Config.h"

#include "wx/wx.h"
#include "wx/numdlg.h"

#include "Actions/EditAction.h"
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/RemoveMeasures.h"

#include "AriaCore.h"

#include "GUI/MeasureBar.h"
#include "Renderers/RenderAPI.h"

#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/TimeSigChange.h"
#include "Midi/Players/PlatformMidiManager.h"

#include "Editors/Editor.h"

#include <iostream>


namespace AriaMaestosa
{

  int insert_at_measure = -1;
  int remove_from = -1;
  int remove_to = -1;

// ****************************************************************************************************************

/*
 * The pop-up menu that is shown if you right-click on a selected measure in the measure bar
 */

class SelectedMenu : public wxMenu
{

public:
    LEAK_CHECK(SelectedMenu);
SelectedMenu() : wxMenu()
{


        Append(1, _("Remove selected measures"));

}

/*
 * Removes selected measures
 */

void removeSelected(wxCommandEvent& event)
{
    // ask confirmation
    int answer = wxMessageBox(  _("Do you really want to remove selected measures?"), wxT(""), wxYES_NO);
    if(answer == wxNO or answer == wxCANCEL) return;
    if(answer != wxYES and answer != wxOK)
    {
        std::cout << "Unknown answer from wxMessageBox in SelectedMenu::removeSelected() : " << answer << std::endl;
    }

    getCurrentSequence()->action( new Action::RemoveMeasures(remove_from, remove_to) );
    getMeasureData()->selectTimeSig(0);
}

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(SelectedMenu, wxMenu)
EVT_MENU(1,SelectedMenu::removeSelected)
END_EVENT_TABLE()

// ****************************************************************************************************************
#if 0
#pragma mark -
#endif
/*
 * The pop-up menu that is shown if you right-click on an unselected measure in the measure bar
 */

class UnselectedMenu : public wxMenu
{
    wxMenuItem* deleteTimeSig;
    int remove_timeSigID;

public:
    LEAK_CHECK(UnselectedMenu);

    UnselectedMenu() : wxMenu()
    {


        Append(2, _("Insert measures"));
        deleteTimeSig = Append(3, _("Remove time sig change"));

    }

    void insert(wxCommandEvent& event)
    {
        int number = wxGetNumberFromUser(  _("Insert empty measures between measures ") + to_wxString(insert_at_measure+1) +
                                           _(" and ") + to_wxString(insert_at_measure+2),
                                           _("Amount: "), wxT(""), 4 /*default*/, 1 /*min*/);
        if(number==-1) return;

        Sequence* seq = getCurrentSequence();

        // --------- move notes in all tracks -----------

        seq->action( new Action::InsertEmptyMeasures(insert_at_measure+1, number) );

    }

    void enable_deleteTimeSig_item(bool enabled, int timeSigID=-1)
    {
        deleteTimeSig->Enable(enabled);
        if(enabled) remove_timeSigID = timeSigID;
    }

    void removeTimeSig(wxCommandEvent& event)
    {
        int answer = wxMessageBox(  _("Do you really want to remove this time sig change?"), wxT(""), wxYES_NO);
        if(answer == wxNO or answer == wxCANCEL) return;

        if(remove_timeSigID==0)
        {
            wxMessageBox( _("You can't remove the first time signature change."));
            return;
        }

        //std::cout << "really removing " << remove_timeSigID << std::endl;
        getMeasureData()->eraseTimeSig(remove_timeSigID);
        getMeasureData()->updateMeasureInfo();
        Display::render();
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(UnselectedMenu, wxMenu)
EVT_MENU(2,UnselectedMenu::insert)
EVT_MENU(3,UnselectedMenu::removeTimeSig)
END_EVENT_TABLE()

// ****************************************************************************************************************
#if 0
#pragma mark -
#endif

MeasureBar::MeasureBar(MeasureData* parent)
{
    selectedMenu  = new SelectedMenu();
    unselectedMenu = new UnselectedMenu();

    lastMeasureInDrag = -1;

    data = parent;
}

MeasureBar::~MeasureBar()
{
}


int MeasureBar::getMeasureBarHeight()
{
    if(data->expandedMode) return 40;
    else return 20;
}

void MeasureBar::render(int measureBarY_arg)
{
    measureBarY = measureBarY_arg;

    // if measure amount changed and MeasureBar is out of sync with its current number of measures, fix it
    if((int)data->measureInfo.size() != data->measureAmount)
        data->updateVector(data->measureAmount);

    const int height = (data->expandedMode ? 40 : 20);
    AriaRender::primitives();
    AriaRender::color(1, 1, 0.9);
    AriaRender::rect(0, measureBarY, Display::getWidth(), measureBarY+ height);

    // black line at the top and bottom
    AriaRender::color(0, 0, 0);

    AriaRender::line(0, measureBarY+1, Display::getWidth(), measureBarY+1);
    AriaRender::line(0, measureBarY+20, Display::getWidth(), measureBarY+20);

    if(data->expandedMode) AriaRender::line(0, measureBarY+40, Display::getWidth(), measureBarY+40);


    // vertical lines and mesure ID
    int measureID=0;
    AriaRender::color(0,0,0);

    const bool measureLengthConstant = data->isMeasureLengthConstant();

    const float x_initial = getEditorsXStart() - getCurrentSequence()->getXScrollInPixels();
    const float x_step = data->measureLengthInPixels();

    for(float n=x_initial; n<Display::getWidth();
        (measureLengthConstant ? n+=x_step : n += data->measureLengthInPixels(measureID-1) )
        )
    {
        measureID++;
        if(measureID > data->measureAmount) break;

        // if measure is selected, draw in blue
        if( data->measureInfo[measureID-1].selected )
        {

            AriaRender::color(0.71, 0.84, 1);

            if(measureLengthConstant)
                AriaRender::rect(n, measureBarY+1, n+x_step, measureBarY+19);
            else
                AriaRender::rect(n, measureBarY+1, n+data->measureInfo[measureID-1].widthInPixels, measureBarY+19);

            AriaRender::color(0,0,0);
        }

        // vertical line
        AriaRender::line(n, measureBarY, n, measureBarY+20);

        // measure ID
        wxString buffer = to_wxString(measureID);
        AriaRender::small_text(buffer.mb_str(), n+5, measureBarY+15);

        if(data->expandedMode)
        {

            const int amount = data->timeSigChanges.size();
            for(int i=0; i<amount; i++)
            {

                if(data->timeSigChanges[i].measure == measureID-1)
                {
                    const bool selected = (data->selectedTimeSig == i);
                    if(selected)
                    {
                        AriaRender::pointSize(11);
                        AriaRender::color(1,0,0);
                    }
                    else
                    {
                        AriaRender::pointSize(7);
                        AriaRender::color(0.5,0,0);
                    }
                    AriaRender::point(n, measureBarY + 30);

                    if(selected) AriaRender::color(0.5,0,0);
                    else AriaRender::color(0,0,0);

                    AriaRender::pointSize(1);

                    wxString denom_name = to_wxString(data->timeSigChanges[i].denom);
                    AriaRender::small_text(denom_name.mb_str(), n + 18, measureBarY + 38);

                    wxString num_name = to_wxString(data->timeSigChanges[i].num);
                    AriaRender::small_text(num_name.mb_str(), n + 10, measureBarY + 29);

                    AriaRender::color(0,0,0);
                    break;
                }
            } // next time sig change
        } // end if expanded mode
    } // next

}

/*
 * When mouse button is pushed on Measure Bar
 */

void MeasureBar::mouseDown(int x, int y)
{

    // if click is in time sig change areas
    if(y>20)
    {

        const int measureDivAt_x = data->measureDivisionAt(x);

        // we use the 'add' method, however if on event already exists at this location
        // it will be selected and none will be added
        data->addTimeSigChange(measureDivAt_x, -1, -1);

        return;
    }

    const int measure_amount = data->measureInfo.size();
    for(int n=0; n<measure_amount; n++)
    {
        data->measureInfo[n].selected = false;
    }

    const int measure_vectorID = data->measureAtPixel( x );
    data->measureInfo[measure_vectorID].selected = true;

    data->somethingSelected = true;
    lastMeasureInDrag = measure_vectorID;

    std::cout << "clicked on measure " << lastMeasureInDrag << std::endl;


}

/*
 * When mouse is held down and dragged on Measure Bar
 */

void MeasureBar::mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial)
{

    const int measure_vectorID = data->measureAtPixel(mousex_current);

    if(lastMeasureInDrag == -1) return; //invalid

    data->measureInfo[measure_vectorID].selected = true;
    data->somethingSelected = true;

    // previous measure seelcted and this one and contiguous - select all other measures inbetween
    if( abs(lastMeasureInDrag-measure_vectorID) > 1 )
    {

        for(int n=measure_vectorID; n != lastMeasureInDrag; n +=
            (lastMeasureInDrag-measure_vectorID)/abs(lastMeasureInDrag-measure_vectorID))
        {
            data->measureInfo[n].selected = true;
        }
    }

    lastMeasureInDrag = measure_vectorID;
}

/*
 * Selection is done, select notes contained in the selected measures
 */

void MeasureBar::mouseUp(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial)
{

    if(lastMeasureInDrag == -1) return; //invalid

    Sequence* sequence = getCurrentSequence();

    const int measureAmount = data->measureInfo.size();

    // determine selection range in midi ticks
    int minimal_tick = -1, maximal_tick = -1;
    for(int n=0; n<measureAmount; n++)
    {
        // iterate through measures to find the first selected one
        if(data->measureInfo[n].selected)
        {
            // we found a first selected measure, remember it as minimal tick
            minimal_tick = data->firstTickInMeasure(n);
            do{ n++; } while(data->measureInfo[n].selected); // skip all uneslected measures
            maximal_tick = data->firstTickInMeasure(n);
            break;
        }
    }

    // iterate through notes and select those that are within the selection range just found
    const int trackAmount = sequence->getTrackAmount();
    for(int track=0; track<trackAmount; track++)
    {
        const int noteAmount = sequence->getTrack(track)->getNoteAmount();
        for(int n=0; n<noteAmount; n++)
        {
            const int note_tick = sequence->getTrack(track)->getNoteStartInMidiTicks(n);
            // note is within selection range? if so, select it, else unselect it.
            if( note_tick >= minimal_tick and note_tick < maximal_tick ) sequence->getTrack(track)->selectNote(n, true, true);
            else sequence->getTrack(track)->selectNote(n, false, true);
        }
    }

}

/*
int MeasureBar::getTotalPixelAmount()
{
    if(isMeasureLengthConstant())
    {
        return (int)( measureAmount * measureLengthInTicks() * getCurrentSequence()->getZoom() );
    }
    else
    {
        return totalNeededLengthInPixels;
    }
}
*/

void MeasureBar::rightClick(int x, int y)
{

    if(y>20)
    {
        // find between which measures the user clicked
        int measure = data->measureDivisionAt(x);

        // check if click is on time sig event
        const int amount = data->timeSigChanges.size();
        for(int n=0; n<amount; n++)
        {
            if(data->timeSigChanges[n].measure == measure)
            {
                std::cout << "trying to delete measure " << n << std::endl;
                unselectedMenu->enable_deleteTimeSig_item(true, n);
                Display::popupMenu( (wxMenu*)unselectedMenu, x, y+20);
                return;
            }
        }

    }

    const int measure_vectorID = data->measureAtPixel( x );

    // is the clicked measure selected?
    if(data->measureInfo[measure_vectorID].selected)
    {
        const int measureAmount = data->measureInfo.size();
        for(int n=0; n<measureAmount; n++)
        {
            // iterate through measures to find which measures are selected
            if(data->measureInfo[n].selected)
            {
                // we found a first selected measure, remember it
                remove_from = n;
                do{ n++; } while(data->measureInfo[n].selected); // skip all selected measures
                remove_to = n;
                break;
            }
        }

        // unselect
        for(int n=remove_from; n<remove_to+1; n++)
        {
            data->measureInfo[n].selected=false;
        }
        Display::popupMenu( (wxMenu*)selectedMenu, x, y+20);
    }
    else
    {
        // find between which measures the user clicked
        insert_at_measure = data->measureDivisionAt(x)-1;

        unselectedMenu->enable_deleteTimeSig_item(false);
        Display::popupMenu( (wxMenu*)unselectedMenu, x, y+20);
    }

}

}
