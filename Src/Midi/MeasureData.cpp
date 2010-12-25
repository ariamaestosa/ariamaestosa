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

//#include "Actions/EditAction.h"
//#include "Actions/InsertEmptyMeasures.h"
//#include "Actions/RemoveMeasures.h"

//#include "AriaCore.h"
//#include "Editors/Editor.h"
#include "GUI/MeasureBar.h"
#include "GUI/MainFrame.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/TimeSigChange.h"
//#include "Midi/Players/PlatformMidiManager.h"
#include "Pickers/TimeSigPicker.h"

#include <iostream>
#include "irrXML/irrXML.h"
#include <wx/spinbutt.h>
#include <wx/msgdlg.h>

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

MeasureData::MeasureData(int measureAmount)
{
    m_something_selected = false;
    m_selected_time_sig  = 0;
    m_measure_amount     = measureAmount;
    m_first_measure      = 0;
    m_expanded_mode      = false;
    
    m_time_sig_changes.push_back( new TimeSigChange(0,4,4) );
    m_time_sig_changes[0].setTick(0);
    updateVector(measureAmount);
}

// ----------------------------------------------------------------------------------------------------------

MeasureData::~MeasureData()
{
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

void MeasureData::setExpandedMode(bool arg_expanded)
{
    //when turning it off, ask for a confirmation because all events will be lost
    if (this->m_expanded_mode and not arg_expanded)
    {
        // FIXME: this is GUI code and should not go in this model class
        const int answer = wxMessageBox(_("Are you sure you want to go back to having a single time signature? Any time sig events you may have added will be lost. This cannot be undone."),
                                        _("Confirm"), wxYES_NO);
        if (answer == wxNO){ getMainFrame()->updateMenuBarToSequence(); return; }

        // remove all added events
        m_selected_time_sig = 0;
        m_time_sig_changes.clearAndDeleteAll();
        m_time_sig_changes.push_back( new TimeSigChange(0, 4, 4) );
        m_time_sig_changes[0].setTick(0);
    }

    m_expanded_mode = arg_expanded;
    updateMeasureInfo();
    getMainFrame()->updateMenuBarToSequence();
    Display::render();
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Measures
#endif

void MeasureData::setMeasureAmount(int measureAmount)
{
    m_measure_amount = measureAmount;
    updateVector(measureAmount);
    
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::setFirstMeasure(int firstMeasureID)
{
    m_first_measure = firstMeasureID;
}

// ----------------------------------------------------------------------------------------------------------

float MeasureData::measureLengthInPixels(int measure)
{
    if (measure==-1) measure=0; // no parameter passed, use measure 0 settings
    return (float)measureLengthInTicks(measure) * (float)getCurrentGraphicalSequence()->getZoom();
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::measureLengthInTicks(int measure)
{
    if (measure==-1) measure=0; // no parameter passed, use measure 0 settings
    Sequence* sequence = getCurrentSequence();

    const int num = getTimeSigNumerator(measure), denom=getTimeSigDenominator(measure);

    return (int)round(
                 sequence->ticksPerBeat() * num * (4.0/(float)denom)
                 );
}

// ----------------------------------------------------------------------------------------------------------

// right now, it's just the first measure that is considered "default". This may need to be reviewed
// (i'm not sure if this is used at all or very much)
int MeasureData::defaultMeasureLengthInTicks()
{
    Sequence* sequence = getCurrentSequence();

    return (int)( sequence->ticksPerBeat() * getTimeSigNumerator(0) * (4.0/getTimeSigDenominator(0)) );
}

// ----------------------------------------------------------------------------------------------------------

// right now, it's just the first measure that is considered "default". This may need to be reviewed.
// (i'm not sure if this is used at all or very much)
float MeasureData::defaultMeasureLengthInPixels()
{
    GraphicalSequence* sequence = getCurrentGraphicalSequence();
    return (float)measureLengthInTicks(0) * (float)sequence->getZoom();
}

// ----------------------------------------------------------------------------------------------------------

// FIXME- large bits of this belong to the GUI
void MeasureData::unselect()
{
    if (not m_something_selected) return;
    m_something_selected = false;

    const int measureAmount = m_measure_info.size();
    for (int n=0; n<measureAmount; n++)
    {
        m_measure_info[n].selected = false;
    }
    
    getCurrentGraphicalSequence()->getMeasureBar()->lastMeasureInDrag = -1;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Find Measure From Location
#endif

int MeasureData::measureAtPixel(int pixel)
{
    const float x1 = 90 - getCurrentGraphicalSequence()->getXScrollInPixels();
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

        const int amount = m_measure_info.size();
        for (int n=0; n<amount; n++)
        {
            if (n==amount-1) return amount-1; // we hit end, return the last
            if ( m_measure_info[n].pixel <= pixel and m_measure_info[n+1].pixel > pixel ) return n;
        }

        return 0;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::measureAtTick(int tick)
{
    if (isMeasureLengthConstant())
    {
        const float step = measureLengthInTicks();

        const int answer = (int)( tick / step );
        
        if (not getCurrentSequence()->importing)
        {
            // verify that we're within song bounds. except if importing, since the song length
            // might not have been set yet.
            ASSERT_E(answer, <=, m_measure_amount);
        }
        
        return answer;

    }
    else
    {
        if (tick < 0) tick = 0;
        if (not getCurrentSequence()->importing)
        {
            // verify that we're within song bounds. except if importing, since the song length
            // might not have been set yet.
            ASSERT_E(tick, <=, lastTickInMeasure(m_measure_amount-1));
        }
        
        // iterate through measures till we find the one at the given tick
        const int amount = m_measure_info.size();
        for (int n=0; n<amount-1; n++)
        {
            if ( m_measure_info[n].tick <= tick and m_measure_info[n+1].tick > tick ) return n;
        }

        // did not find this tick in our current measure set
        if (getCurrentSequence()->importing)
        {
            // if we're currently importing, extrapolate beyond the current song end since we
            // might be trying to determine needed song length (FIXME: this should not read the
            // importing bool from the sequence, whether to extrapolate should be a parameter)
            const int last_id = m_time_sig_changes.size()-1;
            tick -= m_time_sig_changes[ last_id ].getTick();
            
            const int answer =  (int)(
                                      m_time_sig_changes[ last_id ].getMeasure() +
                                      tick / ( getCurrentSequence()->ticksPerBeat() *
                                              m_time_sig_changes[ last_id ].getNum() *
                                              (4.0/ m_time_sig_changes[ last_id ].getDenom() ) )
                                      );
            return answer;
        }
        else
        {
            // didnt find any... current song length is not long enough
            return m_measure_amount-1;
        }
    }

}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::measureDivisionAt(int pixel)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    const float x1 = 90 - getCurrentGraphicalSequence()->getXScrollInPixels();

    if (isMeasureLengthConstant())
    {
        const float xstep = measureLengthInPixels();

        return (int)( ( pixel - x1 + xstep/2)/xstep );
    }
    else
    {
        pixel -= (int)x1;
        const int measureAmount = m_measure_info.size();
        for (int n=0; n<measureAmount; n++)
        {
            //std::cout << "checking measur " << n << " : is " << pixel << " between "
            //          << m_measure_info[n].pixel-m_measure_info[n].widthInPixels/2 << " and "
            //          << m_measure_info[n].endPixel-m_measure_info[n].widthInPixels/2 << std::endl;
            
            if (pixel >= m_measure_info[n].pixel-m_measure_info[n].widthInPixels/2 and
                pixel<m_measure_info[n].endPixel-m_measure_info[n].widthInPixels/2)
            {
                return n;
            }
        }
        return measureAmount-1;
    }

}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::firstPixelInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    if (isMeasureLengthConstant())
    {
        return (int)(
                     id * measureLengthInTicks() * getCurrentGraphicalSequence()->getZoom() -
                     getCurrentGraphicalSequence()->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)m_measure_info.size());
        return m_measure_info[id].pixel -  getCurrentGraphicalSequence()->getXScrollInPixels() + 90;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::lastPixelInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    
    if (isMeasureLengthConstant())
    {
        return (int)(
                     (id+1) * measureLengthInTicks() * getCurrentGraphicalSequence()->getZoom() -
                     getCurrentGraphicalSequence()->getXScrollInPixels() + 90
                     );
    }
    else
    {
        ASSERT_E(id,<,(int)m_measure_info.size());
        return m_measure_info[id].endPixel -  getCurrentGraphicalSequence()->getXScrollInPixels() + 90;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::firstTickInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    
    if (isMeasureLengthConstant())
    {
        if (id >= m_measure_amount)
        {
            // allow going a little past 'official' song end to account for rounding errors, etc. (FIXME?)
            ASSERT(id < m_measure_amount + 50); // but not too far, to detect corrupt values
            return m_measure_amount * measureLengthInTicks();
        }
        return id * measureLengthInTicks();
    }
    else
    {
        // allow going a little past 'official' song end to account for rounding errors, etc. (FIXME?)
        if (id >= (int)m_measure_info.size())
        {
            if (id > (int)m_measure_info.size()+50){ ASSERT(false); } // but not too far, to detect corrupt values
            
            return m_measure_info[m_measure_info.size()-1].endTick;
        }
        
        
        return m_measure_info[id].tick;
    }
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::lastTickInMeasure(int id)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    
    if (isMeasureLengthConstant())
    {
        return (id+1) * measureLengthInTicks();
    }
    else
    {
        // allow going a little past 'official' song end
        if (id >= (int)m_measure_info.size())
        {
            if (id > (int)m_measure_info.size()+50){ ASSERT(false); } // but not too far, to detect corrupt values
            
            return m_measure_info[m_measure_info.size()-1].endTick;
        }
        
        ASSERT_E(id,<,(int)m_measure_info.size());
        return m_measure_info[id].endTick;
    }
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Time Signature Management
#endif

void MeasureData::selectTimeSig(const int id)
{
    // FIXE... not really what you'd expect considering the method's name
    m_selected_time_sig = id;
    getMainFrame()->changeShownTimeSig( m_time_sig_changes[id].getNum(), m_time_sig_changes[id].getDenom() );
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::getTimeSigNumerator(int measure) const
{
    if (measure != -1)
    {
        measure+=1;
        const int timeSigChangeAmount = m_time_sig_changes.size();
        for (int n=0; n<timeSigChangeAmount; n++)
        {
            if (m_time_sig_changes[n].getMeasure() >= measure) return m_time_sig_changes[n-1].getNum();
        }
        return m_time_sig_changes[timeSigChangeAmount-1].getNum();
    }
    else return m_time_sig_changes[m_selected_time_sig].getNum();
}

// ----------------------------------------------------------------------------------------------------------

int MeasureData::getTimeSigDenominator(int measure) const
{

    if (measure != -1)
    {
        measure+=1;

        const int timeSigChangeAmount = m_time_sig_changes.size();
        for (int n=0; n<timeSigChangeAmount; n++)
        {
            if (m_time_sig_changes[n].getMeasure() >= measure) return m_time_sig_changes[n-1].getDenom();
        }
        return m_time_sig_changes[timeSigChangeAmount-1].getDenom();
    }
    else
    {
        return m_time_sig_changes[m_selected_time_sig].getDenom();
    }
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::setTimeSig(int top, int bottom)
{
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    
    m_time_sig_changes[m_selected_time_sig].setNum( top );
    m_time_sig_changes[m_selected_time_sig].setDenom( bottom );
    updateMeasureInfo();

    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    // FIXME: confusing line, maybe rename 'setZoom' so it's clearer what it does...
    gseq->setZoom( gseq->getZoomInPercent() ); // update zoom to new measure size

    wxSpinEvent unused;
    getMainFrame()->songLengthChanged(unused);

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::eraseTimeSig(int id)
{
    m_time_sig_changes.erase( id );
    if (m_selected_time_sig == id)
    {
        m_selected_time_sig = 0;
        getMainFrame()->changeShownTimeSig( m_time_sig_changes[0].getNum(), m_time_sig_changes[0].getDenom() );
    }
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::addTimeSigChange(int measure, int num, int denom) // -1 means "same as previous event"
{
    const int timeSig_amount_minus_one = m_time_sig_changes.size()-1;

    // if there are no events, just add it. otherwise, add in time order.
    if (m_time_sig_changes.size() == 0)
    {
        m_time_sig_changes.push_back( new TimeSigChange(measure, num, denom) );
    }
    else
    {
        // iterate through time sig events, until an iteration does something with the event
        for (int n=0; n<(int)m_time_sig_changes.size(); n++) 
        {
            if (m_time_sig_changes[n].getMeasure() == measure)
            {
                // a time sig event already exists at this location
                // if we're not importing, select it
                if (not getCurrentSequence()->importing)
                {
                    m_selected_time_sig = n;
                    getMainFrame()->changeShownTimeSig(m_time_sig_changes[m_selected_time_sig].getNum(),
                                                       m_time_sig_changes[m_selected_time_sig].getDenom() );

                    wxPoint pt = wxGetMousePosition();
                    
                    // FIXME: this is GUI code and should not appear in this model
                    showTimeSigPicker( pt.x, pt.y, m_time_sig_changes[n].getNum(), m_time_sig_changes[n].getDenom() );
                    break;
                }
                // if we're importing, replace it with new value
                else
                {
                    m_time_sig_changes[m_selected_time_sig].setNum(num);
                    m_time_sig_changes[m_selected_time_sig].setDenom(denom);
                    break;
                }
            }
            // we checked enough events so that we are past the point where the click occured.
            // we know there is no event already existing at the clicked measure.
            if (m_time_sig_changes[n].getMeasure() > measure)
            {
                if (num == -1 or denom == -1)
                {
                    m_time_sig_changes.add( new TimeSigChange(measure, m_time_sig_changes[n].getNum(),
                                                          m_time_sig_changes[n].getDenom()), n );
                }
                else
                {
                    m_time_sig_changes.add( new TimeSigChange(measure, num, denom), n );
                }

                m_selected_time_sig = n;

                getMainFrame()->changeShownTimeSig( m_time_sig_changes[n].getNum(), m_time_sig_changes[n].getDenom() );

                if (not getCurrentSequence()->importing)
                {
                    wxPoint pt = wxGetMousePosition();
                    showTimeSigPicker(pt.x, pt.y, m_time_sig_changes[n].getNum(), m_time_sig_changes[n].getDenom());
                    if (not getCurrentSequence()->importing) updateMeasureInfo();
                }
                
                break;
            }
            else if (n == timeSig_amount_minus_one)
            {

                if (num == -1 or denom == -1)
                {
                    m_time_sig_changes.add( new TimeSigChange(measure, m_time_sig_changes[n].getNum(),
                                                          m_time_sig_changes[n].getDenom()), n+1 );
                }
                else
                {
                    m_time_sig_changes.add( new TimeSigChange(measure, num, denom), n+1 );
                }
                
                selectTimeSig(n+1);
                
                // m_selected_time_sig = n + 1;
                // getMainFrame()->changeShownTimeSig( m_time_sig_changes[n+1].getNum(), m_time_sig_changes[n+1].getDenom() );

                if (not getCurrentSequence()->importing)
                {
                    wxPoint pt = wxGetMousePosition();
                    showTimeSigPicker( pt.x, pt.y, m_time_sig_changes[n].getNum(), m_time_sig_changes[n].getDenom() );
                    if (not getCurrentSequence()->importing) updateMeasureInfo();
                }
                break;
            }
        }//next

        /*
         // check order
         // FIXME- debug, remove
         std::cout << "-----" << std::endl;
         for(int n=0; n<(int)m_time_sig_changes.size(); n++)
         {
             std::cout << "  " << m_time_sig_changes[n].getMeasure() << std::endl;
         }
         */
    }

}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::setTimesigMeasure(const int id, const int newMeasure)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_time_sig_changes.size());
    m_time_sig_changes[id].setMeasure(newMeasure);
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Refresh Info
#endif

void MeasureData::updateVector(int newSize)
{
    while ((int)m_measure_info.size() < newSize) m_measure_info.push_back( MeasureInfo() );
    while ((int)m_measure_info.size() > newSize) m_measure_info.erase( m_measure_info.begin()+m_measure_info.size()-1 );

    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
    
    if (not isMeasureLengthConstant() or m_expanded_mode) updateMeasureInfo();
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::updateMeasureInfo()
{
    const int amount = m_measure_info.size();
    const float zoom = getCurrentGraphicalSequence()->getZoom();
    GraphicalSequence* sequence = getCurrentGraphicalSequence();
    const int ticksPerBeat = sequence->getModel()->ticksPerBeat();
    float tick = 0;
    int timg_sig_event = 0;

    ASSERT_E(timg_sig_event,<,m_time_sig_changes.size());
    m_time_sig_changes[timg_sig_event].setTick(0);
    //m_time_sig_changes[timg_sig_event].pixel = 0;

    for (int n=0; n<amount; n++)
    {
        // check if time sig changes on this measure
        if (timg_sig_event != (int)m_time_sig_changes.size()-1 and
            m_time_sig_changes[timg_sig_event+1].getMeasure() == n)
        {
            timg_sig_event++;
            m_time_sig_changes[timg_sig_event].setTick((int)round( tick ));
            //m_time_sig_changes[timg_sig_event].pixel = (int)round( tick * zoom );
        }

        // set end location of previous measure
        if (n>0)
        {
            m_measure_info[n-1].endTick = (int)round( tick );
            m_measure_info[n-1].endPixel = (int)round( tick * zoom );
            m_measure_info[n-1].widthInTicks = m_measure_info[n-1].endTick - m_measure_info[n-1].tick;
            m_measure_info[n-1].widthInPixels = (int)( m_measure_info[n-1].widthInTicks * zoom );
        }

        // set the location of measure in both ticks and pixels so that it can be used later in
        // calculations and drawing
        m_measure_info[n].tick = (int)round( tick );
        m_measure_info[n].pixel = (int)round( tick * zoom );
        tick += ticksPerBeat * m_time_sig_changes[timg_sig_event].getNum() *
                (4.0 /(float)m_time_sig_changes[timg_sig_event].getDenom());
    }

    // fill length and end of last measure
    m_measure_info[amount-1].endTick = (int)tick;
    m_measure_info[amount-1].endPixel = (int)( tick * zoom );
    m_measure_info[amount-1].widthInTicks = m_measure_info[amount-1].endTick - m_measure_info[amount-1].tick;
    m_measure_info[amount-1].widthInPixels = (int)( m_measure_info[amount-1].widthInTicks * zoom );

    totalNeededLengthInTicks = (int)tick;
    totalNeededLengthInPixels = (int)( tick * zoom );

    DisplayFrame::updateHorizontalScrollbar();
    ASSERT_E(m_measure_amount, ==, (int)m_measure_info.size());
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark I/O
#endif

//FIXME: remove globals
// when we import time sig changes, their time is given in ticks. however Aria needs them in measure ID.
// this variable is used to convert ticks to measures (we can't yet use measureAt method and friends because
// measure information is still incomplete at this point ).
int last_event_tick;
int measuresPassed;

// ----------------------------------------------------------------------------------------------------------

void MeasureData::beforeImporting()
{
    m_time_sig_changes.clearAndDeleteAll();
    m_time_sig_changes.push_back(new TimeSigChange(0,4,4) );
    m_time_sig_changes[0].setTick(0);

    last_event_tick = 0;
    measuresPassed = 0;
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::addTimeSigChange_import(int tick, int num, int denom)
{
    int measure;

    if (tick == 0)
    {
        measure = 0;

        if (m_time_sig_changes.size() == 0)
        {
            m_time_sig_changes.push_back( new TimeSigChange(0, 4, 4) );
            m_time_sig_changes[0].setTick(0);
        }

        // when an event already exists at the beginning, don't add another one, just modify it
        if (m_time_sig_changes[0].getTick() == 0)
        {
            m_time_sig_changes[0].setNum(num);
            m_time_sig_changes[0].setDenom(denom);
            m_time_sig_changes[0].setTick(0);
            m_time_sig_changes[0].setMeasure(0);
            last_event_tick = 0;
            return;
        }
        else
        {
            std::cerr << "Unexpected!! tick is " << m_time_sig_changes[0].getTick() << std::endl;
        }
    }
    else
    {
        const int last_id = m_time_sig_changes.size() - 1;
        measure = (int)(
                        measuresPassed + (tick - last_event_tick)  /
                        ( getCurrentSequence()->ticksPerBeat() *
                          m_time_sig_changes[last_id].getNum() *
                          (4.0/ m_time_sig_changes[last_id].getDenom() )
                          )
                        );
    }

    m_time_sig_changes.push_back( new TimeSigChange(measure, num, denom) );

    measuresPassed = measure;

    last_event_tick = tick;
    m_time_sig_changes[m_time_sig_changes.size()-1].setTick(tick);
}

// ----------------------------------------------------------------------------------------------------------

void MeasureData::afterImporting()
{
    if (m_time_sig_changes.size()>1)
    {
        m_expanded_mode = true;
    }
    else
    {
        m_expanded_mode = false;
    }
    if (not isMeasureLengthConstant()) updateMeasureInfo();
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------------------------------------

int MeasureData::getTotalPixelAmount()
{
    if (isMeasureLengthConstant())
    {
        return (int)( m_measure_amount * measureLengthInTicks() * getCurrentGraphicalSequence()->getZoom() );
    }
    else
    {
        return totalNeededLengthInPixels;
    }
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Serialisation
#endif

bool MeasureData::readFromFile(irr::io::IrrXMLReader* xml)
{

    // ---------- measure ------
    if (strcmp("measure", xml->getNodeName()) == 0)
    {

        const char* firstMeasure_c = xml->getAttributeValue("firstMeasure");
        if ( firstMeasure_c != NULL )
        {
            setFirstMeasure( atoi( firstMeasure_c ) );
        }
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
        if (denom != NULL)
        {
            if (m_time_sig_changes.size()==0)
            {
                m_time_sig_changes.push_back(new TimeSigChange(0,4,4) ); // add an event if one is not already there
                m_time_sig_changes[0].setTick(0);
            }
            m_time_sig_changes[0].setDenom( atoi( denom ) );

            if (m_time_sig_changes[0].getDenom() < 0 or m_time_sig_changes[0].getDenom() > 64)
            {
                std::cerr << "Wrong measureBar->getTimeSigDenominator(): "
                          << m_time_sig_changes[0].getDenom() << std::endl;
                
                m_time_sig_changes[0].setDenom(4);
            }
        }

        const char* num = xml->getAttributeValue("num");
        if (num != NULL)
        {
            if (m_time_sig_changes.size()==0)
            {
                m_time_sig_changes.push_back(new TimeSigChange(0,4,4)); // add an event if one is not already there
                m_time_sig_changes[0].setTick(0);
            }
            m_time_sig_changes[0].setNum( atoi( num ) );

            if (m_time_sig_changes[0].getNum() < 0 or m_time_sig_changes[0].getNum() > 64)
            {
                std::cerr << "Wrong m_time_sig_changes[0].getNum(): " << m_time_sig_changes[0].getNum() << std::endl;
                m_time_sig_changes[0].setNum( 4 );
            }
        }


    }
    else if (strcmp("timesig", xml->getNodeName()) == 0)
    {
        std::cout << "importing event" << std::endl;
        int num=-1, denom=-1, meas=-1;

        const char* num_c = xml->getAttributeValue("num");
        if (num_c != NULL)
        {
            num = atoi( num_c );
        }
        else
        {
            std::cerr << "Missing info from file: measure numerator" << std::endl;
            return true; // dont halt program but just ignore event
        }
        const char* denom_c = xml->getAttributeValue("denom");
        if (denom_c != NULL)
        {
            denom = atoi( denom_c );
        }
        else
        {
            std::cerr << "Missing info from file: measure denominator" << std::endl;
            return true;  // dont halt program but just ignore event
        }
        const char* meas_c = xml->getAttributeValue("measure");
        if (meas_c != NULL)
        {
            meas = atoi( meas_c );
        }
        else
        {
            std::cerr << "Missing info from file: time sig location" << std::endl;
            return true;  // dont halt program but just ignore event
        }

        addTimeSigChange(meas, num, denom);
        if (m_time_sig_changes.size() > 1)
        {
            m_expanded_mode = true;
        }
    }

    return true;

}

// ----------------------------------------------------------------------------------------------------------

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
        const int timeSigAmount = m_time_sig_changes.size();
        for (int n=0; n<timeSigAmount; n++)
        {
            writeData(wxT("<timesig num=\"") + to_wxString(m_time_sig_changes[n].getNum()) +
                      wxT("\" denom=\"") + to_wxString(m_time_sig_changes[n].getDenom()) +
                      wxT("\" measure=\"") + to_wxString(m_time_sig_changes[n].getMeasure()) + wxT("\"/>\n"),
                      fileout );
        }//next
        writeData(wxT("</measure>\n\n"), fileout );
    }
}

// ----------------------------------------------------------------------------------------------------------
