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
#include "LeakCheck.h"

#include "GUI/MeasureBar.h"
#include "GUI/MainFrame.h"
#include "GUI/RenderUtils.h"

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
	
	DECLARE_LEAK_CHECK();
	
public:
	
SelectedMenu() : wxMenu()
{
		INIT_LEAK_CHECK();
		
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
	getMeasureBar()->selectTimeSig(0);
}

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(SelectedMenu, wxMenu)
EVT_MENU(1,SelectedMenu::removeSelected)
END_EVENT_TABLE()

// ****************************************************************************************************************

/*
 * The pop-up menu that is shown if you right-click on an unselected measure in the measure bar
 */

class UnselectedMenu : public wxMenu
{
	
	DECLARE_LEAK_CHECK();
	wxMenuItem* deleteTimeSig;
	int remove_timeSigID;
	
public:
	
	UnselectedMenu() : wxMenu()
	{
		INIT_LEAK_CHECK();
		
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
		getMeasureBar()->eraseTimeSig(remove_timeSigID);
		getMeasureBar()->updateMeasureInfo();
		Display::render();
	}
	
	DECLARE_EVENT_TABLE();
};

int MeasureBar::getTimeSigAmount()
{
	return timeSigChanges.size();
}
TimeSigChange& MeasureBar::getTimeSig(int id)
{
	return timeSigChanges[id];
}

BEGIN_EVENT_TABLE(UnselectedMenu, wxMenu)
EVT_MENU(2,UnselectedMenu::insert)
EVT_MENU(3,UnselectedMenu::removeTimeSig)
END_EVENT_TABLE()

// ****************************************************************************************************************
	
	
MeasureBar::MeasureBar()
{
	INIT_LEAK_CHECK();
	
	mainFrame = getMainFrame();
	somethingSelected = false;
	
	selectedMenu = new SelectedMenu();
	unselectedMenu = new UnselectedMenu();
	
	lastMeasureInDrag = -1;
	
	expandedMode = false;
	
	measureAmount=DEFAULT_SONG_LENGTH;
	timeSigChanges.push_back( new TimeSigChange(0,4,4) );
	timeSigChanges[0].tick = 0;
	timeSigChanges[0].measure = 0;
	
	selectedTimeSig = 0;
	firstMeasure=0;
	
	updateVector(measureAmount);
}

void MeasureBar::setMeasureAmount(int measureAmount)
{
    MeasureBar::measureAmount=measureAmount;
    Display::render();
	getMeasureBar()->updateVector(measureAmount);
}

int MeasureBar::getMeasureAmount()
{
    return measureAmount;
}


MeasureBar::~MeasureBar()
{
	delete selectedMenu;
	delete unselectedMenu;
}


void MeasureBar::selectTimeSig(const int id)
{
	selectedTimeSig = id;
	getMainFrame()->changeShownTimeSig( timeSigChanges[id].num, timeSigChanges[id].denom );
}

int MeasureBar::getFirstMeasure()
{
	return firstMeasure;
}

void MeasureBar::setFirstMeasure(int firstMeasureID)
{
	firstMeasure = firstMeasureID;
}

int MeasureBar::measureAtPixel(int pixel)
{
	const float x1 = 90 - getCurrentSequence()->getXScrollInPixels();
	pixel -= (int)x1;
    
	if(isMeasureLengthConstant())
	{
        if(pixel < 0) pixel = 0;
		// length of a measure
		const float xstep = measureLengthInPixels();
		
		return (int)( pixel/xstep );
	}
	else
	{

        if(pixel < 0) return 0;
        
		const int amount = measureInfo.size();
		for(int n=0; n<amount; n++)
		{
			if(n==amount-1) return amount-1; // we hit end, return the last
			if( measureInfo[n].pixel <= pixel and measureInfo[n+1].pixel > pixel ) return n;
		}
		
		return 0;
	}
}

int MeasureBar::getTimeSigNumerator(int measure)
{
	if(measure != -1)
	{
		measure+=1;
		const int timeSigChangeAmount = timeSigChanges.size();
		for(int n=0; n<timeSigChangeAmount; n++)
		{
			if( timeSigChanges[n].measure >= measure ) return timeSigChanges[n-1].num;
		}
		return timeSigChanges[timeSigChangeAmount-1].num;
	}
	else return timeSigChanges[selectedTimeSig].num;
}
int MeasureBar::getTimeSigDenominator(int measure)
{
	
	if(measure != -1)
	{
		measure+=1;
		
		const int timeSigChangeAmount = timeSigChanges.size();
		for(int n=0; n<timeSigChangeAmount; n++)
		{
			if( timeSigChanges[n].measure >= measure ) return timeSigChanges[n-1].denom;
		}
		return timeSigChanges[timeSigChangeAmount-1].denom;
	}
	else return timeSigChanges[selectedTimeSig].denom;
}

/*
 * Called either when user changes the numbers on the top bar, either when importing a song
 */

void MeasureBar::setTimeSig(int top, int bottom)
{
	//std::cout << "****** set time sig " << top << " " << bottom << std::endl;
	if(top > 0)
	{
		float denom = (float)log(bottom)/(float)log(2);
		if( (int)denom != (float)denom )
		{
			wxBell();
			wxMessageBox(  _("Denominator must be a power of 2") );
			return;
		}
		
		timeSigChanges[selectedTimeSig].num = top;
	}
	
	if(bottom > 0)
	{
		timeSigChanges[selectedTimeSig].denom = bottom;
    }
	updateMeasureInfo();
	

	getCurrentSequence()->setZoom( getCurrentSequence()->getZoomInPercent() ); // update zoom to new measure size
	
    wxSpinEvent unused;
    getMainFrame()->songLengthChanged(unused);
	
    Display::render();
}

int MeasureBar::measureAtTick(int tick)
{
	if(isMeasureLengthConstant())
	{
		const float step = measureLengthInTicks();
		
		return (int)( tick / step );
		
	}
	else
	{
        if(tick <0) tick = 0;

		// iterate through measures till we find the one at the given tick
		const int amount = measureInfo.size();
		for(int n=0; n<amount-1; n++)
		{
			if( measureInfo[n].tick <= tick and measureInfo[n+1].tick > tick ) return n;
		}

		// didnt find any... current song length is not long enough
		const int last_id = timeSigChanges.size()-1;
		tick -= timeSigChanges[ last_id ].tick;
		
		return (int)(
					 timeSigChanges[ last_id ].measure + 
			tick / ( getCurrentSequence()->ticksPerBeat() *
					 timeSigChanges[ last_id ].num *
					 (4.0/ timeSigChanges[ last_id ].denom ) )
					 );
		
	}
	
}

int MeasureBar::firstPixelInMeasure(int id)
{ 
	if(isMeasureLengthConstant())
	{
		return (int)(
					 id * measureLengthInTicks() * getCurrentSequence()->getZoom() -
					 getCurrentSequence()->getXScrollInPixels() + 90
					 );
	}
	else
	{
        assertExpr(id,<,measureInfo.size());
		return measureInfo[id].pixel -  getCurrentSequence()->getXScrollInPixels() + 90;
	}
}

int MeasureBar::lastPixelInMeasure(int id)
{ 
	if(isMeasureLengthConstant())
	{
		return (int)(
					 (id+1) * measureLengthInTicks() * getCurrentSequence()->getZoom() -
					 getCurrentSequence()->getXScrollInPixels() + 90
					 );
	}
	else
	{
        assertExpr(id,<,measureInfo.size());
		return measureInfo[id].endPixel -  getCurrentSequence()->getXScrollInPixels() + 90;
	}
}


int MeasureBar::firstTickInMeasure(int id)
{
	if(isMeasureLengthConstant())
	{
		return id * measureLengthInTicks();	
	}
	else
	{
        assertExpr(id,<,measureInfo.size());
		return measureInfo[id].tick;
	}
}
int MeasureBar::lastTickInMeasure(int id)
{
	if(isMeasureLengthConstant())
	{
		return (id+1) * measureLengthInTicks();	
	}
	else
	{
        assertExpr(id,<,measureInfo.size());
		return measureInfo[id].endTick;
	}
}

// used when right-cliking an object - tells on which line between measures pixel is
int MeasureBar::measureDivisionAt(int pixel)
{
	const float x1 = 90 - getCurrentSequence()->getXScrollInPixels();

	
	if(isMeasureLengthConstant())
	{
		const float xstep = measureLengthInPixels();
		
		return (int)( ( pixel - x1 + xstep/2)/xstep );
	}
	else
	{
		pixel -= (int)x1;
		const int measureAmount = measureInfo.size();
		for(int n=0; n<measureAmount; n++)
		{
			//std::cout << "checking measur " << n << " : is " << pixel << " between " << measureInfo[n].pixel-measureInfo[n].widthInPixels/2 << " and " << measureInfo[n].endPixel-measureInfo[n].widthInPixels/2 << std::endl;
			if(pixel >= measureInfo[n].pixel-measureInfo[n].widthInPixels/2 and pixel<measureInfo[n].endPixel-measureInfo[n].widthInPixels/2)
			{
				return n;
			}
		}
		return measureAmount-1;
	}

}

/*
 * Change the number of items in the selected vector sothat it contains the same amount of elements as the number of measures.
 */

void MeasureBar::updateVector(int newSize)
{
	while((int)measureInfo.size() < newSize) measureInfo.push_back( MeasureInfo() );
	while((int)measureInfo.size() > newSize) measureInfo.erase( measureInfo.begin()+measureInfo.size()-1 );
	
	if(!isMeasureLengthConstant() or expandedMode) updateMeasureInfo();
}

int MeasureBar::getMeasureBarHeight()
{
	if(expandedMode) return 40;
	else return 20;
}

void MeasureBar::render(int measureBarY_arg)
{
	measureBarY = measureBarY_arg;
	
	// if measure amount changed and MeasureBar is out of sync with its current number of measures, fix it
	if((int)measureInfo.size() != measureAmount)
	{
		updateVector(measureAmount);
	}
	
	int height = (expandedMode?40:20);
    AriaRender::primitives();
    AriaRender::color(1, 1, 0.9);
    AriaRender::rect(0, measureBarY, Display::getWidth(), measureBarY+ height);
    
    // black line at the top and bottom
    AriaRender::color(0, 0, 0);
    
    AriaRender::line(0, measureBarY+1, Display::getWidth(), measureBarY+1);
    AriaRender::line(0, measureBarY+20, Display::getWidth(), measureBarY+20);

	if(expandedMode) AriaRender::line(0, measureBarY+40, Display::getWidth(), measureBarY+40);
	

    // vertical lines and mesure ID
    int measureID=0;
    AriaRender::color(0,0,0);
	
	const bool measureLengthConstant = isMeasureLengthConstant();
	
	const float x_initial = getEditorXStart() - getCurrentSequence()->getXScrollInPixels();
	const float x_step = measureLengthInPixels();
	
	for(float n=x_initial; n<Display::getWidth(); (measureLengthConstant ? n+=x_step : n+=measureLengthInPixels(measureID-1) ) )
	{
		measureID++;
		if(measureID > measureAmount) break;
		
		// if measure is selected, draw in blue
		if( measureInfo[measureID-1].selected )
		{
			
            AriaRender::color(0.71, 0.84, 1);
			
			if(measureLengthConstant)
                AriaRender::rect(n, measureBarY+1, n+x_step, measureBarY+19);
			else
                AriaRender::rect(n, measureBarY+1, n+measureInfo[measureID-1].widthInPixels, measureBarY+19);
                                 
            AriaRender::color(0,0,0);
		}
		
		// vertical line
        AriaRender::line(n, measureBarY, n, measureBarY+20);
		
		// measure ID
		char buffer[4];
		sprintf (buffer, "%d", measureID);
        AriaRender::small_text(buffer, n+5, measureBarY+15);
		
		if(expandedMode)
		{

			const int amount = timeSigChanges.size();
			for(int i=0; i<amount; i++)
			{
				
				if(timeSigChanges[i].measure == measureID-1)
				{
					const bool selected = (selectedTimeSig == i);
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
					
					char denom_name[3];
					sprintf (denom_name, "%d", timeSigChanges[i].denom);
                    AriaRender::small_text(denom_name, n + 18, measureBarY + 38);
                    
					char num_name[3];
					sprintf (num_name, "%d", timeSigChanges[i].num);
                    AriaRender::small_text(num_name, n + 10, measureBarY + 29);

                    AriaRender::color(0,0,0);
					break;
				}
			}
		}
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
		
		const int measureDivAt_x = measureDivisionAt(x);
		
        // we use the 'add' method, however if on event already exists at this location
        // it will be selected and none will be added
		addTimeSigChange(measureDivAt_x, -1, -1);
		
		return;
	}
	
	const int measure_amount = measureInfo.size();
	for(int n=0; n<measure_amount; n++)
	{
		measureInfo[n].selected = false;
	}
	
	const int measure_vectorID = measureAtPixel( x );
	
	measureInfo[measure_vectorID].selected = true;
	
	somethingSelected = true;
	lastMeasureInDrag = measure_vectorID;

	std::cout << "clicked on measure " << lastMeasureInDrag << std::endl;
	

}

void MeasureBar::setExpandedMode(bool expanded)
{
	expandedMode = expanded;
	updateMeasureInfo();
	Display::render();
}

bool MeasureBar::isExpandedMode()
{
	return expandedMode;
}

/*
 * When mouse is held down and dragged on Measure Bar
 */

void MeasureBar::mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial)
{

	const int measure_vectorID = measureAtPixel(mousex_current);
	
	if(lastMeasureInDrag == -1) return; //invalid
	
	measureInfo[measure_vectorID].selected = true;
	somethingSelected = true;
	
	// previous measure seelcted and this one and contiguous - select all other measures inbetween
	if( abs(lastMeasureInDrag-measure_vectorID) > 1 )
	{
		
		for(int n=measure_vectorID; n != lastMeasureInDrag; n += 
			(lastMeasureInDrag-measure_vectorID)/abs(lastMeasureInDrag-measure_vectorID))
		{
			measureInfo[n].selected = true;
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
	
	const int measureAmount = measureInfo.size();
	
	// determine selection range in midi ticks
	int minimal_tick = -1, maximal_tick = -1;
	for(int n=0; n<measureAmount; n++)
	{
		// iterate through measures to find the first selected one
		if(measureInfo[n].selected)
		{
			// we found a first selected measure, remember it as minimal tick
			minimal_tick = firstTickInMeasure(n);
			do{ n++; } while(measureInfo[n].selected); // skip all uneslected measures
			maximal_tick = firstTickInMeasure(n);
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

// -----------------------

int MeasureBar::getTotalTickAmount()
{
	if(isMeasureLengthConstant())
	{
		return (int)( measureAmount * measureLengthInTicks() );
	}
	else
	{
		return totalNeededLengthInTicks;
	}
}

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

bool MeasureBar::isMeasureLengthConstant()
{
	return (!expandedMode and timeSigChanges.size() == 1);
}

float MeasureBar::measureLengthInPixels(int measure)
{
	if(measure==-1) measure=0; // no parameter passed, use measure 0 settings
	return (float)measureLengthInTicks(measure) * (float)getCurrentSequence()->getZoom();
}

int MeasureBar::measureLengthInTicks(int measure)
{
	if(measure==-1) measure=0; // no parameter passed, use measure 0 settings
	Sequence* sequence = getCurrentSequence();
	
	const int num = getTimeSigNumerator(measure), denom=getTimeSigDenominator(measure);
	
	return (int)(
				 sequence->ticksPerBeat() * num * (4.0/ denom)
				 );
}

// right now, it's just the first measure that is considered "default". in the future, it should be made the most used
// im not sure if this is used at all or very much
int MeasureBar::defaultMeasureLengthInTicks()
{
	Sequence* sequence = getCurrentSequence();
	
	return (int)( sequence->ticksPerBeat() * getTimeSigNumerator(0) * (4.0/getTimeSigDenominator(0)) );
}

// right now, it's just the first measure that is considered "default". in the future, it should be made the most used
// im not sure if this is used at all or very much
float MeasureBar::defaultMeasureLengthInPixels()
{
	Sequence* sequence = getCurrentSequence();
	return (float)measureLengthInTicks(0) * (float)sequence->getZoom();
}

float MeasureBar::beatLengthInPixels()
{
	Sequence* sequence = getCurrentSequence();
	
	return getCurrentSequence()->ticksPerBeat() * sequence->getZoom();
}

int MeasureBar::beatLengthInTicks()
{
	return getCurrentSequence()->ticksPerBeat();
}

// -----------------------

void MeasureBar::rightClick(int x, int y)
{

	if(y>20)
	{
		// find between which measures the user clicked
		int measure = measureDivisionAt(x);
		
		// check if click is on time sig event
		const int amount = timeSigChanges.size();
		for(int n=0; n<amount; n++)
		{
			if(timeSigChanges[n].measure == measure)
			{
				std::cout << "trying to delete " << n << std::endl;
				unselectedMenu->enable_deleteTimeSig_item(true, n);
                Display::popupMenu( (wxMenu*)unselectedMenu, x, y+20);
				return;
			}
		}

	}
	
	const int measure_vectorID = measureAtPixel( x );
	
	// is the clicked measure selected?
	if(measureInfo[measure_vectorID].selected)
	{
		const int measureAmount = measureInfo.size();
		for(int n=0; n<measureAmount; n++)
		{
			// iterate through measures to find which measures are selected
			if(measureInfo[n].selected)
			{
				// we found a first selected measure, remember it
				remove_from = n;
				do{ n++; } while(measureInfo[n].selected); // skip all selected measures
				remove_to = n;
				break;
			}
		}
		
		// unselect
		for(int n=remove_from; n<remove_to+1; n++)
		{
			measureInfo[n].selected=false;
		}
        Display::popupMenu( (wxMenu*)selectedMenu, x, y+20);
	}
	else
	{
		// find between which measures the user clicked
		insert_at_measure = measureDivisionAt(x)-1;

		unselectedMenu->enable_deleteTimeSig_item(false);
        Display::popupMenu( (wxMenu*)unselectedMenu, x, y+20);
	}
	
}

void MeasureBar::unselect()
{
	if(!somethingSelected) return;
	somethingSelected = false;
	
	const int measureAmount = measureInfo.size();
	for(int n=0; n<measureAmount; n++)
	{
		measureInfo[n].selected = false;
	}
	lastMeasureInDrag = -1;
}

/*
 * Time Signatures have changed, update and recalculate information about location of measures and events.
 */

void MeasureBar::updateMeasureInfo()
{
	const int amount = measureInfo.size();
	const float zoom = getCurrentSequence()->getZoom();
	Sequence* sequence = getCurrentSequence();
	const int ticksPerBeat = sequence->ticksPerBeat();
	float tick = 0;
	int timg_sig_event = 0;
	
	//std::cout << "udpate with " << timeSigChanges.size() << " key sig events" << std::endl;
	
	
	assertExpr(timg_sig_event,<,timeSigChanges.size());
	timeSigChanges[timg_sig_event].tick = 0;
	timeSigChanges[timg_sig_event].pixel = 0;
	
	for(int n=0; n<amount; n++)
	{
		// check if time sig changes on this measure
		if(timg_sig_event != (int)timeSigChanges.size()-1 and timeSigChanges[timg_sig_event+1].measure == n)
		{
			timg_sig_event++;
			timeSigChanges[timg_sig_event].tick = (int)round( tick );
			timeSigChanges[timg_sig_event].pixel = (int)round( tick * zoom );
		}
		
		// set end location of previous measure
		if(n>0)
		{
			measureInfo[n-1].endTick = (int)round( tick );
			measureInfo[n-1].endPixel = (int)round( tick * zoom );
			measureInfo[n-1].widthInTicks = measureInfo[n-1].endTick - measureInfo[n-1].tick;
			measureInfo[n-1].widthInPixels = (int)( measureInfo[n-1].widthInTicks * zoom );
		}
		
		// set the location of measure in both ticks and pixels so that it can be used later in calculations and drawing
		measureInfo[n].tick = (int)round( tick );
		measureInfo[n].pixel = (int)round( tick * zoom );
		tick += ticksPerBeat * timeSigChanges[timg_sig_event].num * ( 4.0 / timeSigChanges[timg_sig_event].denom );
	}
	
	// fill length and end of last measure
	measureInfo[amount-1].endTick = (int)tick;
	measureInfo[amount-1].endPixel = (int)( tick * zoom );
	measureInfo[amount-1].widthInTicks = measureInfo[amount-1].endTick - measureInfo[amount-1].tick;
	measureInfo[amount-1].widthInPixels = (int)( measureInfo[amount-1].widthInTicks * zoom );
	
	totalNeededLengthInTicks = (int)tick;
	totalNeededLengthInPixels = (int)( tick * zoom );
	
    DisplayFrame::updateHorizontalScrollbar();
	
}

void MeasureBar::eraseTimeSig(int id)
{
	timeSigChanges.erase( id );
	if(selectedTimeSig == id)
	{
		selectedTimeSig = 0;
		getMainFrame()->changeShownTimeSig( timeSigChanges[0].num, timeSigChanges[0].denom );
	}
}

TimeSigChange* MeasureBar::removeTimeSig(int id)
{
	TimeSigChange* return_ptr = timeSigChanges.get(id);
	
	timeSigChanges.remove( id );
	if(selectedTimeSig == id)
	{
		selectedTimeSig = 0;
		getMainFrame()->changeShownTimeSig( timeSigChanges[0].num, timeSigChanges[0].denom );
	}
	
	return return_ptr;
}

// when we import time sig changes, their time is given in ticks. however Aria needs them in measure ID. this variable is used to convert
// ticks to measures (we can't yet use measureAt method and friends because measure information is still incomplete at this point ).
int last_event_tick;
int measuresPassed;

void MeasureBar::beforeImporting()
{
	timeSigChanges.clearAndDeleteAll();
	timeSigChanges.push_back(new TimeSigChange(0,4,4) );
	timeSigChanges[0].tick = 0;
	timeSigChanges[0].measure = 0;
	last_event_tick = 0;
	measuresPassed = 0;
}

void MeasureBar::addTimeSigChange(int measure, int num, int denom) // -1 means "same as previous event"
{
	const int timeSig_amount_minus_one = timeSigChanges.size()-1;
	
	// if there are no events, just add it. otherwise, add in time order.
	if(timeSigChanges.size() == 0)
	{
		timeSigChanges.push_back( new TimeSigChange(measure, num, denom) );
	}
	else
	{
		
		for(int n=0; n<(int)timeSigChanges.size(); n++) // iterate through time sig events, until an iteration does something with the event
		{
			if( timeSigChanges[n].measure == measure)
			{
				// a time sig event already exists at this location
				// if we're not importing, select it
				if(!getCurrentSequence()->importing)
				{
					selectedTimeSig = n;
                    getMainFrame()->changeShownTimeSig( timeSigChanges[selectedTimeSig].num, timeSigChanges[selectedTimeSig].denom );
					break;
				}
				// if we're importing, replace it with new value
				else
				{
					timeSigChanges[selectedTimeSig].num = num;
					timeSigChanges[selectedTimeSig].denom = denom;
					break;
				}
			}	
			// we checked enough events so that we are past the point where the click occured.
			// we know there is no event already existing at the clicked measure.
			if( timeSigChanges[n].measure > measure )
			{

				if(num==-1 or denom==-1)
					timeSigChanges.add( new TimeSigChange(measure, timeSigChanges[n].num, timeSigChanges[n].denom), n );
				else
					timeSigChanges.add( new TimeSigChange(measure, num, denom), n );
                
				selectedTimeSig = n;
				
				getMainFrame()->changeShownTimeSig( timeSigChanges[n].num, timeSigChanges[n].denom );
				
				if(!getCurrentSequence()->importing) updateMeasureInfo();
				break;
			}
            else if( n==timeSig_amount_minus_one )
			{
                
				if(num==-1 or denom==-1)
					timeSigChanges.add( new TimeSigChange(measure, timeSigChanges[n].num, timeSigChanges[n].denom), n+1 );
				else
					timeSigChanges.add( new TimeSigChange(measure, num, denom), n+1 );
                
				selectedTimeSig = n+1;
				
				getMainFrame()->changeShownTimeSig( timeSigChanges[n+1].num, timeSigChanges[n+1].denom );
				
				if(!getCurrentSequence()->importing) updateMeasureInfo();
				break;
			}
		}//next
        
        /*
        // check order
        // FIXME- debug, remove
        std::cout << "-----" << std::endl;
        for(int n=0; n<(int)timeSigChanges.size(); n++)
		{
            std::cout << "  " << timeSigChanges[n].measure << std::endl;
        }
         */
	}
	
}

/*
 * Used when importing midi file
 */
void MeasureBar::addTimeSigChangeAtTick(int tick, int num, int denom)
{
	int measure;
	
	if(tick == 0)
	{
		measure = 0;
		
		if(timeSigChanges.size() == 0)
		{
			timeSigChanges.push_back( new TimeSigChange(0,4,4) );
			timeSigChanges[0].tick = 0;
			timeSigChanges[0].measure = 0;
		}
		
		// when an event already exists at the beginning, don't add another one, just modify it
		if(timeSigChanges[0].tick == 0)
		{
			timeSigChanges[0].num = num;
			timeSigChanges[0].denom = denom;
			timeSigChanges[0].tick = 0;
			timeSigChanges[0].measure = 0;
			last_event_tick = 0;
			return;
		}
		else
		{
			std::cout << "Unexpected!! tick is " << timeSigChanges[0].tick << std::endl;
		}
	}
	else
	{
		const int last_id = timeSigChanges.size() - 1;
		measure = (int)(
						measuresPassed + (tick - last_event_tick)  /
						( getCurrentSequence()->ticksPerBeat() *
						  timeSigChanges[last_id].num *
						  (4.0/ timeSigChanges[last_id].denom )
						  )
						);
	}
	
	timeSigChanges.push_back( new TimeSigChange(measure, num, denom) );
	
	measuresPassed = measure;
	
	last_event_tick = tick;
	timeSigChanges[timeSigChanges.size()-1].tick = tick;
}
void MeasureBar::afterImporting()
{
	if(timeSigChanges.size()>1)
	{
		expandedMode = true;
	}
	else
	{
		expandedMode = false;
	}
	if(!isMeasureLengthConstant()) updateMeasureInfo();
}

bool MeasureBar::readFromFile(irr::io::IrrXMLReader* xml)
{
	
	// ---------- measure ------
	if (!strcmp("measure", xml->getNodeName()))
	{
		
		const char* firstMeasure_c = xml->getAttributeValue("firstMeasure");
		if( firstMeasure_c != NULL ) setFirstMeasure( atoi( firstMeasure_c ) );
		else
		{
			std::cerr << "Missing info from file: first measure" << std::endl;
			setFirstMeasure( 0 );
		}
		
		if(getFirstMeasure() < 0)
		{
			std::cerr << "Wrong first measure: " << getFirstMeasure() << std::endl;
			setFirstMeasure( 0 );
		}
		
		const char* denom = xml->getAttributeValue("denom");
		if( denom != NULL )
		{
			if(timeSigChanges.size()==0)
			{
				timeSigChanges.push_back(new TimeSigChange(0,4,4) ); // add an event if one is not already there
				timeSigChanges[0].tick = 0;
				timeSigChanges[0].measure = 0;
			}
			timeSigChanges[0].denom = atoi( denom );
			
			if(timeSigChanges[0].denom < 0 or timeSigChanges[0].denom>64)
			{
				std::cerr << "Wrong measureBar->getTimeSigDenominator(): " << timeSigChanges[0].denom << std::endl;
				timeSigChanges[0].denom = 4;
			}
		}
		
		const char* num = xml->getAttributeValue("num");
		if( num != NULL ) 
		{
			if(timeSigChanges.size()==0)
			{
				timeSigChanges.push_back(new TimeSigChange(0,4,4)); // add an event if one is not already there
				timeSigChanges[0].tick = 0;
				timeSigChanges[0].measure = 0;
			}
			timeSigChanges[0].num = atoi( num );
			
			if(timeSigChanges[0].num < 0 or timeSigChanges[0].num>64)
			{
				std::cerr << "Wrong timeSigChanges[0].num: " << timeSigChanges[0].num << std::endl;
				timeSigChanges[0].num = 4;
			}
		}

		
	}
	else if (!strcmp("timesig", xml->getNodeName()))
	{
		std::cout << "importing event" << std::endl;
		int num=-1, denom=-1, meas=-1;
		
		const char* num_c = xml->getAttributeValue("num");
		if( num_c != NULL ) num = atoi( num_c );
		else
		{
			std::cerr << "Missing info from file: measure numerator" << std::endl;
			return true; // dont halt program but just ignore event
		}
		const char* denom_c = xml->getAttributeValue("denom");
		if( denom_c != NULL ) denom = atoi( denom_c );
		else
		{
			std::cerr << "Missing info from file: measure denominator" << std::endl;
			return true;  // dont halt program but just ignore event
		}
		const char* meas_c = xml->getAttributeValue("measure");
		if( meas_c != NULL ) meas = atoi( meas_c );
		else
		{
			std::cerr << "Missing info from file: time sig location" << std::endl;
			return true;  // dont halt program but just ignore event
		}
		
		addTimeSigChange(meas, num, denom);
		if(timeSigChanges.size()>1)
		{
			expandedMode = true;
		}
	}
	
	return true;
	
}
void MeasureBar::saveToFile(wxFileOutputStream& fileout)
{
	writeData(wxT("<measure ") +
			  wxString( wxT(" firstMeasure=\"") ) + to_wxString(getMeasureBar()->getFirstMeasure()),
			  fileout);
	
	if(isMeasureLengthConstant())
	{
		writeData(wxT("\" denom=\"") + to_wxString(getTimeSigDenominator()) +
				  wxT("\" num=\"") + to_wxString(getTimeSigNumerator()) +
				  wxT("\"/>\n\n"),
				  fileout );
	}
	else
	{
		writeData(wxT("\">\n"), fileout );
		const int timeSigAmount = timeSigChanges.size();
		for(int n=0; n<timeSigAmount; n++)
		{
			writeData(wxT("<timesig num=\"") + to_wxString(timeSigChanges[n].num) +
					  wxT("\" denom=\"") + to_wxString(timeSigChanges[n].denom) +
					  wxT("\" measure=\"") + to_wxString(timeSigChanges[n].measure) + wxT("\"/>\n"),
					  fileout );
		}//next
		writeData(wxT("</measure>\n\n"), fileout );
	}
}
}
