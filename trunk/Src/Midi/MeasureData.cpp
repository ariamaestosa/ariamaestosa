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

#include "wx/wx.h"
#include "wx/numdlg.h"

#include "Actions/EditAction.h"
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/RemoveMeasures.h"

#include "AriaCore.h"

#include "GUI/MeasureBar.h"
#include "GUI/MainFrame.h"

#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/TimeSigChange.h"
#include "Midi/Players/PlatformMidiManager.h"

#include "Editors/Editor.h"

#include "Pickers/TimeSigPicker.h"

#include <iostream>
#include "irrXML/irrXML.h"
#include "wx/spinctrl.h"

using namespace AriaMaestosa;

// ---------------------------------------------------------------------------------------------------------

MeasureData::MeasureData(int measureAmount)
{
    somethingSelected = false;
    selectedTimeSig = 0;

    m_measure_amount = measureAmount;
    timeSigChanges.push_back( new TimeSigChange(0,4,4) );
    timeSigChanges[0].tick = 0;
    timeSigChanges[0].measure = 0;

    firstMeasure=0;

    graphics = new MeasureBar(this);
    expandedMode = false;
    updateVector(measureAmount);
}

// ---------------------------------------------------------------------------------------------------------

MeasureData::~MeasureData()
{
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

void MeasureData::setExpandedMode(bool arg_expanded)
{
    //when turning it off, ask for a confirmation because all events will be lost
    if (this->expandedMode and not arg_expanded)
    {
        const int answer = wxMessageBox(_("Are you sure you want to go back to having a single time signature? Any time sig events you may have added will be lost. This cannot be undone."),
                                        _("Confirm"), wxYES_NO);
        if (answer == wxNO){ getMainFrame()->updateMenuBarToSequence(); return; }

        // remove all added events
        selectedTimeSig = 0;
        timeSigChanges.clearAndDeleteAll();
        timeSigChanges.push_back( new TimeSigChange(0,4,4) );
        timeSigChanges[0].tick = 0;
        timeSigChanges[0].measure = 0;
    }

    expandedMode = arg_expanded;
    updateMeasureInfo();
    getMainFrame()->updateMenuBarToSequence();
    Display::render();
}


// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Measures
#endif

void MeasureData::setMeasureAmount(int measureAmount)
{
    m_measure_amount = measureAmount;
    updateVector(measureAmount);
    
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    Display::render();
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::setFirstMeasure(int firstMeasureID)
{
    firstMeasure = firstMeasureID;
}

// ---------------------------------------------------------------------------------------------------------

bool MeasureData::isMeasureLengthConstant()
{
    return (!expandedMode and timeSigChanges.size() == 1);
}

// ---------------------------------------------------------------------------------------------------------

float MeasureData::measureLengthInPixels(int measure)
{
    if (measure==-1) measure=0; // no parameter passed, use measure 0 settings
    return (float)measureLengthInTicks(measure) * (float)getCurrentSequence()->getZoom();
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::measureLengthInTicks(int measure)
{
    if (measure==-1) measure=0; // no parameter passed, use measure 0 settings
    Sequence* sequence = getCurrentSequence();

    const int num = getTimeSigNumerator(measure), denom=getTimeSigDenominator(measure);

    return (int)round(
                 sequence->ticksPerBeat() * num * (4.0/(float)denom)
                 );
}

// ---------------------------------------------------------------------------------------------------------

// right now, it's just the first measure that is considered "default". in the future, it should be made the most used
// im not sure if this is used at all or very much
int MeasureData::defaultMeasureLengthInTicks()
{
    Sequence* sequence = getCurrentSequence();

    return (int)( sequence->ticksPerBeat() * getTimeSigNumerator(0) * (4.0/getTimeSigDenominator(0)) );
}

// ---------------------------------------------------------------------------------------------------------

// right now, it's just the first measure that is considered "default". in the future, it should be made the most used
// im not sure if this is used at all or very much
float MeasureData::defaultMeasureLengthInPixels()
{
    Sequence* sequence = getCurrentSequence();
    return (float)measureLengthInTicks(0) * (float)sequence->getZoom();
}

// ---------------------------------------------------------------------------------------------------------

// FIXME- unsure where that belongs
void MeasureData::unselect()
{
    if (!somethingSelected) return;
    somethingSelected = false;

    const int measureAmount = measureInfo.size();
    for(int n=0; n<measureAmount; n++)
    {
        measureInfo[n].selected = false;
    }
    graphics->lastMeasureInDrag = -1;
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Find Measure From Location
#endif

int MeasureData::measureAtPixel(int pixel)
{
    const float x1 = 90 - getCurrentSequence()->getXScrollInPixels();
    pixel -= (int)x1;

    if (isMeasureLengthConstant())
    {
        if (pixel < 0) pixel = 0;
        // length of a measure
        const float xstep = measureLengthInPixels();

        return (int)( pixel/xstep );
    }
    else
    {

        if (pixel < 0) return 0;

        const int amount = measureInfo.size();
        for(int n=0; n<amount; n++)
        {
            if (n==amount-1) return amount-1; // we hit end, return the last
            if ( measureInfo[n].pixel <= pixel and measureInfo[n+1].pixel > pixel ) return n;
        }

        return 0;
    }
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::measureAtTick(int tick)
{
    if (isMeasureLengthConstant())
    {
        const float step = measureLengthInTicks();

        const int answer = (int)( tick / step );
        
        ASSERT_E(answer, <=, m_measure_amount);
        
        return answer;

    }
    else
    {
        if (tick <0) tick = 0;
        ASSERT_E(tick, <=, lastTickInMeasure(m_measure_amount-1));
        
        // iterate through measures till we find the one at the given tick
        const int amount = measureInfo.size();
        for (int n=0; n<amount-1; n++)
        {
            if ( measureInfo[n].tick <= tick and measureInfo[n+1].tick > tick ) return n;
        }

        // didnt find any... current song length is not long enough
        return m_measure_amount-1;
        /*
        const int last_id = timeSigChanges.size()-1;
        tick -= timeSigChanges[ last_id ].tick;

        const int answer =  (int)(
                     timeSigChanges[ last_id ].measure +
                     tick / ( getCurrentSequence()->ticksPerBeat() *
                              timeSigChanges[ last_id ].num *
                              (4.0/ timeSigChanges[ last_id ].denom ) )
                     );
        ASSERT_E(answer, <=, m_measure_amount);
        return answer;
         */
    }

}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::measureDivisionAt(int pixel)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    const float x1 = 90 - getCurrentSequence()->getXScrollInPixels();

    if (isMeasureLengthConstant())
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
            if (pixel >= measureInfo[n].pixel-measureInfo[n].widthInPixels/2 and pixel<measureInfo[n].endPixel-measureInfo[n].widthInPixels/2)
            {
                return n;
            }
        }
        return measureAmount-1;
    }

}

// ---------------------------------------------------------------------------------------------------------

/*
int MeasureData::measureAtPixel(int pixel)
{
    const float x1 = 90 - getCurrentSequence()->getXScrollInPixels();
    pixel -= (int)x1;

    if (isMeasureLengthConstant())
    {
        if (pixel < 0) pixel = 0;
        // length of a measure
        const float xstep = measureLengthInPixels();

        return (int)( pixel/xstep );
    }
    else
    {

        if (pixel < 0) return 0;

        const int amount = measureInfo.size();
        for(int n=0; n<amount; n++)
        {
            if (n==amount-1) return amount-1; // we hit end, return the last
            if ( measureInfo[n].pixel <= pixel and measureInfo[n+1].pixel > pixel ) return n;
        }

        return 0;
    }
}
*/

// ---------------------------------------------------------------------------------------------------------

int MeasureData::firstPixelInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    if (isMeasureLengthConstant())
    {
        return (int)(
                     id * measureLengthInTicks() * getCurrentSequence()->getZoom() -
                     getCurrentSequence()->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)measureInfo.size());
        return measureInfo[id].pixel -  getCurrentSequence()->getXScrollInPixels() + 90;
    }
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::lastPixelInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    
    if (isMeasureLengthConstant())
    {
        return (int)(
                     (id+1) * measureLengthInTicks() * getCurrentSequence()->getZoom() -
                     getCurrentSequence()->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)measureInfo.size());
        return measureInfo[id].endPixel -  getCurrentSequence()->getXScrollInPixels() + 90;
    }
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::firstTickInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    ASSERT_E(id, <, m_measure_amount);
    
    if (isMeasureLengthConstant())
    {
        return id * measureLengthInTicks();
    }
    else
    {
        // allow going a little past 'official' song end
        if (id >= (int)measureInfo.size())
        {
            if (id > (int)measureInfo.size()+50){ ASSERT(false); } // but not too far, to detect corrupt values
            
            return measureInfo[measureInfo.size()-1].endTick;
        }
        
        
        return measureInfo[id].tick;
    }
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::lastTickInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    
    if (isMeasureLengthConstant())
    {
        return (id+1) * measureLengthInTicks();
    }
    else
    {
        // allow going a little past 'official' song end
        if (id >= (int)measureInfo.size())
        {
            if (id > (int)measureInfo.size()+50){ ASSERT(false); } // but not too far, to detect corrupt values
            
            return measureInfo[measureInfo.size()-1].endTick;
        }
        
        ASSERT_E(id,<,(int)measureInfo.size());
        return measureInfo[id].endTick;
    }
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Time Signature Management
#endif

void MeasureData::selectTimeSig(const int id)
{
    // FIXE... not really what you'd expect considering the method's name
    selectedTimeSig = id;
    getMainFrame()->changeShownTimeSig( timeSigChanges[id].num, timeSigChanges[id].denom );
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::getTimeSigNumerator(int measure) const
{
    if (measure != -1)
    {
        measure+=1;
        const int timeSigChangeAmount = timeSigChanges.size();
        for(int n=0; n<timeSigChangeAmount; n++)
        {
            if ( timeSigChanges[n].measure >= measure ) return timeSigChanges[n-1].num;
        }
        return timeSigChanges[timeSigChangeAmount-1].num;
    }
    else return timeSigChanges[selectedTimeSig].num;
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::getTimeSigDenominator(int measure) const
{

    if (measure != -1)
    {
        measure+=1;

        const int timeSigChangeAmount = timeSigChanges.size();
        for(int n=0; n<timeSigChangeAmount; n++)
        {
            if ( timeSigChanges[n].measure >= measure ) return timeSigChanges[n-1].denom;
        }
        return timeSigChanges[timeSigChangeAmount-1].denom;
    }
    else return timeSigChanges[selectedTimeSig].denom;
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::setTimeSig(int top, int bottom)
{
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    
    timeSigChanges[selectedTimeSig].num = top;
    timeSigChanges[selectedTimeSig].denom = bottom;
    updateMeasureInfo();


    getCurrentSequence()->setZoom( getCurrentSequence()->getZoomInPercent() ); // update zoom to new measure size

    wxSpinEvent unused;
    getMainFrame()->songLengthChanged(unused);

    Display::render();
}


// ---------------------------------------------------------------------------------------------------------

TimeSigChange& MeasureData::getTimeSig(int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,timeSigChanges.size());
    return timeSigChanges[id];
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::eraseTimeSig(int id)
{
    timeSigChanges.erase( id );
    if (selectedTimeSig == id)
    {
        selectedTimeSig = 0;
        getMainFrame()->changeShownTimeSig( timeSigChanges[0].num, timeSigChanges[0].denom );
    }
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::addTimeSigChange(int measure, int num, int denom) // -1 means "same as previous event"
{
    const int timeSig_amount_minus_one = timeSigChanges.size()-1;

    // if there are no events, just add it. otherwise, add in time order.
    if (timeSigChanges.size() == 0)
    {
        timeSigChanges.push_back( new TimeSigChange(measure, num, denom) );
    }
    else
    {

        for(int n=0; n<(int)timeSigChanges.size(); n++) // iterate through time sig events, until an iteration does something with the event
        {
            if ( timeSigChanges[n].measure == measure)
            {
                // a time sig event already exists at this location
                // if we're not importing, select it
                if (!getCurrentSequence()->importing)
                {
                    selectedTimeSig = n;
                    getMainFrame()->changeShownTimeSig( timeSigChanges[selectedTimeSig].num, timeSigChanges[selectedTimeSig].denom );

                    wxPoint pt = wxGetMousePosition();
                    showTimeSigPicker( pt.x, pt.y, timeSigChanges[n].num, timeSigChanges[n].denom );
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
            if ( timeSigChanges[n].measure > measure )
            {
                if (num==-1 or denom==-1)
                    timeSigChanges.add( new TimeSigChange(measure, timeSigChanges[n].num, timeSigChanges[n].denom), n );
                else
                    timeSigChanges.add( new TimeSigChange(measure, num, denom), n );

                selectedTimeSig = n;

                getMainFrame()->changeShownTimeSig( timeSigChanges[n].num, timeSigChanges[n].denom );

                if (!getCurrentSequence()->importing)
                {
                    wxPoint pt = wxGetMousePosition();
                    showTimeSigPicker( pt.x, pt.y, timeSigChanges[n].num, timeSigChanges[n].denom );
                    if (!getCurrentSequence()->importing) updateMeasureInfo();
                }
                
                break;
            }
            else if ( n==timeSig_amount_minus_one )
            {

                if (num==-1 or denom==-1)
                    timeSigChanges.add( new TimeSigChange(measure, timeSigChanges[n].num, timeSigChanges[n].denom), n+1 );
                else
                    timeSigChanges.add( new TimeSigChange(measure, num, denom), n+1 );

                selectedTimeSig = n+1;

                getMainFrame()->changeShownTimeSig( timeSigChanges[n+1].num, timeSigChanges[n+1].denom );

                if (!getCurrentSequence()->importing)
                {
                    wxPoint pt = wxGetMousePosition();
                    showTimeSigPicker( pt.x, pt.y, timeSigChanges[n].num, timeSigChanges[n].denom );
                    if (!getCurrentSequence()->importing) updateMeasureInfo();
                }
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

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Refresh Info
#endif

void MeasureData::updateVector(int newSize)
{
    while ((int)measureInfo.size() < newSize) measureInfo.push_back( MeasureInfo() );
    while ((int)measureInfo.size() > newSize) measureInfo.erase( measureInfo.begin()+measureInfo.size()-1 );

    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
    
    if (not isMeasureLengthConstant() or expandedMode) updateMeasureInfo();
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::updateMeasureInfo()
{
    const int amount = measureInfo.size();
    const float zoom = getCurrentSequence()->getZoom();
    Sequence* sequence = getCurrentSequence();
    const int ticksPerBeat = sequence->ticksPerBeat();
    float tick = 0;
    int timg_sig_event = 0;

    ASSERT_E(timg_sig_event,<,timeSigChanges.size());
    timeSigChanges[timg_sig_event].tick = 0;
    timeSigChanges[timg_sig_event].pixel = 0;

    for(int n=0; n<amount; n++)
    {
        // check if time sig changes on this measure
        if (timg_sig_event != (int)timeSigChanges.size()-1 and timeSigChanges[timg_sig_event+1].measure == n)
        {
            timg_sig_event++;
            timeSigChanges[timg_sig_event].tick = (int)round( tick );
            timeSigChanges[timg_sig_event].pixel = (int)round( tick * zoom );
        }

        // set end location of previous measure
        if (n>0)
        {
            measureInfo[n-1].endTick = (int)round( tick );
            measureInfo[n-1].endPixel = (int)round( tick * zoom );
            measureInfo[n-1].widthInTicks = measureInfo[n-1].endTick - measureInfo[n-1].tick;
            measureInfo[n-1].widthInPixels = (int)( measureInfo[n-1].widthInTicks * zoom );
        }

        // set the location of measure in both ticks and pixels so that it can be used later in calculations and drawing
        measureInfo[n].tick = (int)round( tick );
        measureInfo[n].pixel = (int)round( tick * zoom );
        //std::cout << "beats : " << timeSigChanges[timg_sig_event].num * ( 4.0 /(float)timeSigChanges[timg_sig_event].denom ) << std::endl;
        tick += ticksPerBeat * timeSigChanges[timg_sig_event].num * ( 4.0 /(float)timeSigChanges[timg_sig_event].denom );
    }

    // fill length and end of last measure
    measureInfo[amount-1].endTick = (int)tick;
    measureInfo[amount-1].endPixel = (int)( tick * zoom );
    measureInfo[amount-1].widthInTicks = measureInfo[amount-1].endTick - measureInfo[amount-1].tick;
    measureInfo[amount-1].widthInPixels = (int)( measureInfo[amount-1].widthInTicks * zoom );

    totalNeededLengthInTicks = (int)tick;
    totalNeededLengthInPixels = (int)( tick * zoom );

    DisplayFrame::updateHorizontalScrollbar();
    ASSERT_E(m_measure_amount, ==, (int)measureInfo.size());
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark I/O
#endif

//FIXME: remove globals
// when we import time sig changes, their time is given in ticks. however Aria needs them in measure ID. this variable is used to convert
// ticks to measures (we can't yet use measureAt method and friends because measure information is still incomplete at this point ).
int last_event_tick;
int measuresPassed;

// ---------------------------------------------------------------------------------------------------------

void MeasureData::beforeImporting()
{
    timeSigChanges.clearAndDeleteAll();
    timeSigChanges.push_back(new TimeSigChange(0,4,4) );
    timeSigChanges[0].tick = 0;
    timeSigChanges[0].measure = 0;
    last_event_tick = 0;
    measuresPassed = 0;
}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::addTimeSigChange_import(int tick, int num, int denom)
{
    int measure;

    if (tick == 0)
    {
        measure = 0;

        if (timeSigChanges.size() == 0)
        {
            timeSigChanges.push_back( new TimeSigChange(0,4,4) );
            timeSigChanges[0].tick = 0;
            timeSigChanges[0].measure = 0;
        }

        // when an event already exists at the beginning, don't add another one, just modify it
        if (timeSigChanges[0].tick == 0)
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

// ---------------------------------------------------------------------------------------------------------

void MeasureData::afterImporting()
{
    if (timeSigChanges.size()>1)
    {
        expandedMode = true;
    }
    else
    {
        expandedMode = false;
    }
    if (!isMeasureLengthConstant()) updateMeasureInfo();
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

int MeasureData::getTotalTickAmount()
{
    if (isMeasureLengthConstant())
    {
        return (int)( m_measure_amount * measureLengthInTicks() );
    }
    else
    {
        return totalNeededLengthInTicks;
    }
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::getTotalPixelAmount()
{
    if (isMeasureLengthConstant())
    {
        return (int)( m_measure_amount * measureLengthInTicks() * getCurrentSequence()->getZoom() );
    }
    else
    {
        return totalNeededLengthInPixels;
    }
}

// ---------------------------------------------------------------------------------------------------------

float MeasureData::beatLengthInPixels()
{
    Sequence* sequence = getCurrentSequence();

    return sequence->ticksPerBeat() * sequence->getZoom();
}

// ---------------------------------------------------------------------------------------------------------

int MeasureData::beatLengthInTicks()
{
    return getCurrentSequence()->ticksPerBeat();
}

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Serialisation
#endif

bool MeasureData::readFromFile(irr::io::IrrXMLReader* xml)
{

    // ---------- measure ------
    if (!strcmp("measure", xml->getNodeName()))
    {

        const char* firstMeasure_c = xml->getAttributeValue("firstMeasure");
        if ( firstMeasure_c != NULL ) setFirstMeasure( atoi( firstMeasure_c ) );
        else
        {
            std::cerr << "Missing info from file: first measure" << std::endl;
            setFirstMeasure( 0 );
        }

        if (getFirstMeasure() < 0)
        {
            std::cerr << "Wrong first measure: " << getFirstMeasure() << std::endl;
            setFirstMeasure( 0 );
        }

        const char* denom = xml->getAttributeValue("denom");
        if ( denom != NULL )
        {
            if (timeSigChanges.size()==0)
            {
                timeSigChanges.push_back(new TimeSigChange(0,4,4) ); // add an event if one is not already there
                timeSigChanges[0].tick = 0;
                timeSigChanges[0].measure = 0;
            }
            timeSigChanges[0].denom = atoi( denom );

            if (timeSigChanges[0].denom < 0 or timeSigChanges[0].denom>64)
            {
                std::cerr << "Wrong measureBar->getTimeSigDenominator(): " << timeSigChanges[0].denom << std::endl;
                timeSigChanges[0].denom = 4;
            }
        }

        const char* num = xml->getAttributeValue("num");
        if ( num != NULL )
        {
            if (timeSigChanges.size()==0)
            {
                timeSigChanges.push_back(new TimeSigChange(0,4,4)); // add an event if one is not already there
                timeSigChanges[0].tick = 0;
                timeSigChanges[0].measure = 0;
            }
            timeSigChanges[0].num = atoi( num );

            if (timeSigChanges[0].num < 0 or timeSigChanges[0].num>64)
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
        if ( num_c != NULL ) num = atoi( num_c );
        else
        {
            std::cerr << "Missing info from file: measure numerator" << std::endl;
            return true; // dont halt program but just ignore event
        }
        const char* denom_c = xml->getAttributeValue("denom");
        if ( denom_c != NULL ) denom = atoi( denom_c );
        else
        {
            std::cerr << "Missing info from file: measure denominator" << std::endl;
            return true;  // dont halt program but just ignore event
        }
        const char* meas_c = xml->getAttributeValue("measure");
        if ( meas_c != NULL ) meas = atoi( meas_c );
        else
        {
            std::cerr << "Missing info from file: time sig location" << std::endl;
            return true;  // dont halt program but just ignore event
        }

        addTimeSigChange(meas, num, denom);
        if (timeSigChanges.size()>1)
        {
            expandedMode = true;
        }
    }

    return true;

}

// ---------------------------------------------------------------------------------------------------------

void MeasureData::saveToFile(wxFileOutputStream& fileout)
{
    writeData(wxT("<measure ") +
              wxString( wxT(" firstMeasure=\"") ) + to_wxString(getMeasureData()->getFirstMeasure()),
              fileout);

    if (isMeasureLengthConstant())
    {
        writeData(wxT("\" denom=\"") + to_wxString(getTimeSigDenominator()) +
                  wxT("\" num=\"")   + to_wxString(getTimeSigNumerator()) +
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

// ---------------------------------------------------------------------------------------------------------
