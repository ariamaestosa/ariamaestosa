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
#include "wx/wx.h"

#include "Midi/Track.h"
#include "Printing/EditorPrintable.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

using namespace AriaMaestosa;
    
// -------------------------------------------------------------------------------------------
    
EditorPrintable::EditorPrintable()
{
}

// -------------------------------------------------------------------------------------------

EditorPrintable::~EditorPrintable()
{
}

// -------------------------------------------------------------------------------------------
    
void EditorPrintable::setCurrentDC(wxDC* dc)
{
    EditorPrintable::dc = dc;
}

// -------------------------------------------------------------------------------------------
    
void EditorPrintable::drawVerticalDivider(LayoutElement* el, const int y0, const int y1, const bool atEnd)
{
    if (el->getType() == TIME_SIGNATURE_EL) return;
    
    const int elem_x_start = (atEnd ? el->getXTo() : el->getXFrom());
    
    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
}

// -------------------------------------------------------------------------------------------

void EditorPrintable::drawVerticalDivider(const int x, const int y0, const int y1)
{
    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( x, y0, x, y1);
}

// -------------------------------------------------------------------------------------------
    
void EditorPrintable::renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1)
{
    wxString num   = wxString::Format( wxT("%i"), el->num   );
    wxString denom = wxString::Format( wxT("%i"), el->denom );
    
    wxFont oldfont = dc->GetFont();
    dc->SetFont( wxFont(150,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD) );
    dc->SetTextForeground( wxColour(0,0,0) );
    
    wxSize text_size = dc->GetTextExtent(denom);
    const int text_x = el->getXTo() - text_size.GetWidth() - 20;
    
    dc->DrawText(num,   text_x, y0 + 10);
    dc->DrawText(denom, text_x, y0 + (y1 - y0)/2 + 10  );
    
    dc->SetFont(oldfont);    
}

// -------------------------------------------------------------------------------------------
    
// FIXME : unclean to pass trackID and LayoutLine as argument!
LayoutElement* EditorPrintable::continueWithNextElement(const int trackID, LayoutLine& layoutLine, const int currentLayoutElement)
{
    const LineTrackRef& lineTrackRef = layoutLine.getLineTrackRef(trackID);
    const TrackCoords* trackCoords = lineTrackRef.m_track_coords.raw_ptr;
    assert(trackCoords != NULL);
    
    if (!(currentLayoutElement < layoutLine.getLayoutElementCount()))
    {
        //std::cout << "---- returning NULL because we have a total of " << layoutElementsAmount << " elements\n";
        return NULL;
    }
    
    //std::vector<LayoutElement>& layoutElements = layoutLine.layoutElements;
    
    LayoutElement& currElem = layoutLine.getLayoutElement(currentLayoutElement);
    const int elem_x_start = currElem.getXFrom();
    
    dc->SetTextForeground( wxColour(0,0,255) );
    
    // ****** empty measure
    if (currElem.getType() == EMPTY_MEASURE)
    {
    }
    // ****** time signature change
    else if (currElem.getType() == TIME_SIGNATURE_EL)
    {
    }
    // ****** repetitions
    else if (currElem.getType() == SINGLE_REPEATED_MEASURE or currElem.getType() == REPEATED_RIFF)
    {
        // FIXME - why do I cut apart the measure and not the layout element?
        /*
         if (measures[layoutElements[n].measure].cutApart)
         {
         // TODO...
         //dc.SetPen(  wxPen( wxColour(0,0,0), 4 ) );
         //dc.DrawLine( elem_x_start, y0, elem_x_start, y1);
         }
         */
        
        wxString message;
        if (currElem.getType() == SINGLE_REPEATED_MEASURE)
        {
            message = to_wxString(layoutLine.getMeasureForElement(currentLayoutElement).firstSimilarMeasure+1);
        }
        else if (currElem.getType() == REPEATED_RIFF)
        {
            message = to_wxString(currElem.firstMeasureToRepeat+1) +
            wxT(" - ") +
            to_wxString(currElem.lastMeasureToRepeat+1);
        }
        
        dc->DrawText( message, elem_x_start,
                     (trackCoords->y0 + trackCoords->y1)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
    }
    // ****** play again
    else if (currElem.getType() == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << currElem.amountOfTimes;
        dc->DrawText( label, elem_x_start,
                     (trackCoords->y0 + trackCoords->y1)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
    }
    // ****** normal measure
    else if (currElem.getType() == SINGLE_MEASURE)
    {
        //std::cout << "---- element is normal\n";
        
        // draw measure ID
        if (lineTrackRef.showMeasureNumber())
        {
            const int meas_id = layoutLine.getMeasureForElement(currentLayoutElement).getMeasureID() + 1;
            
            wxString measureLabel;
            measureLabel << meas_id;
            
            dc->DrawText( measureLabel,
                          elem_x_start - ( meas_id > 9 ?
                                           AriaPrintable::getCurrentPrintable()->getCharacterWidth() :
                                           AriaPrintable::getCurrentPrintable()->getCharacterWidth()/2 ),
                          trackCoords->y0 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()*1.4 );
        }
        dc->SetTextForeground( wxColour(0,0,0) );
    }
    
    //FIXME: this can't go here, because this subclass doesn't have the information about
    //       which parts of the allocated area need the line (depending on editor type,
    //       it's not necessarily the full vertical range)
    if (currElem.render_end_bar)
    {
        drawVerticalDivider(&currElem, trackCoords->y0, trackCoords->y1, true /* at end */);
    }
    
    
    // close line with vertical bar
    //drawVerticalDivider(trackCoords->x1, trackCoords->y0, trackCoords->y1);
    
    return &currElem;
}

// -------------------------------------------------------------------------------------------
    
Range<int> EditorPrintable::getNotePrintX(const int trackID, LayoutLine& line, int noteID)
{
    return tickToX( trackID, line, line.getLineTrackRef(trackID).m_track->getNoteStartInMidiTicks(noteID) );
}
    
// -------------------------------------------------------------------------------------------
    
Range<int> EditorPrintable::tickToX(const int trackID, LayoutLine& line, const int tick)
{
    //LineTrackRef& renderInfo = line.getLineTrackRef(trackID);
    
    // find in which measure this tick belongs
    for (int n=0; n<line.getLayoutElementCount(); n++)
    {
        const PrintLayoutMeasure& meas = line.getMeasureForElement(n);
        if (meas == NULL_MEASURE) continue;
        const int firstTickInMeasure = meas.getFirstTick();
        const int lastTickInMeasure  = meas.getLastTick();
        
        if (tick >= firstTickInMeasure and tick < lastTickInMeasure)
        {
            //std::cout << tick << " is within bounds " << firstTick << " - " << lastTick << std::endl;
            const int elem_x_start = line.getLayoutElement(n).getXFrom();
            const int elem_x_end = line.getLayoutElement(n).getXTo();
            const int elem_w = elem_x_end - elem_x_start;
            
            //std::cout << "tickToX found tick " << tick << std::endl;
            
            Range<float> relative_pos = meas.ticks_placement_manager.getSymbolRelativeArea( tick );
            
            //if (meas.ticks_relative_position.find(tick) == meas.ticks_relative_position.end())
            //{
            //    std::cout << "\n/!\\ tickToX didn't find X for tick " << tick << " in measure " << (meas.id+1) << "\n\n";
            //}
            
            assertExpr(elem_w, >, 0);
            
            const int from = (int)round(relative_pos.from * elem_w + elem_x_start);
            int to = (int)round(relative_pos.to   * elem_w + elem_x_start);
            
            // FIXME: arbitrary max length for now
            // FIXME: ideally, when one symbol is given too much space, the max size reached detection should
            //        be detected earlier in order to allow other symbols on the line to use the available space
            if ((to - from) > 175) to = from + 175;
            
            return Range<int>(from, to);
        }
        else 
        // given tick is before the current line
        if (tick < firstTickInMeasure) 
        {
            //std::cout << "tickToX Returning -1 A\n";
            return Range<int>(-1, -1);
        }
        /* the tick we were given is not on the current line, but on the next.
         * this probably means there is a tie from a note on one line to a note
         * on another line. Return a X at the very right of the page.
         * FIXME - it's not necessarly a tie
         * FIXME - ties and line warping need better handling
         */
        else if (n==line.getLayoutElementCount()-1 and tick >= lastTickInMeasure)
        {
            //std::cout << "tickToX Returning -" <<  (currentLine->layoutElements[n].getXTo() + 10) << " B\n";

            return Range<int>(line.getLayoutElement(n).getXTo() + 10, line.getLayoutElement(n).getXTo() + 10);
        }
    }
    
    return Range<int>(-1, -1);
}
    
// -------------------------------------------------------------------------------------------

void EditorPrintable::addSilencesFromVector(const std::vector< SilenceAnalyser::SilenceInfo >& m_silences_ticks,
                                            RelativePlacementManager& ticks_relative_position, const int trackID,
                                            const int firstTickInMeasure, const int lastTickInMeasure)
{
    // ---- silences
    const int silenceAmount = m_silences_ticks.size();
    for (int n=0; n<silenceAmount; n++)
    {
        if (m_silences_ticks[n].m_tick_range.from < firstTickInMeasure or
            m_silences_ticks[n].m_tick_range.from >= lastTickInMeasure)
        {
            continue;
        }
        
#if VERBOSE
        std::cout << "    Adding [silence] tick " << m_silences_ticks[n] << " to list" << std::endl;
#endif
        
        int neededSize = 0;
        
        switch (m_silences_ticks[n].m_type)
        {
            case 1:
            case 2:
                neededSize = RECTANGULAR_SILENCE_SIZE + RECTANGULAR_SILENCE_LEFT_MARGIN;
                break;
            case 4:
            case 8:
            case 16:
                neededSize = 110; // FIXME: when using vector silence symbols, fix this to not use a hardcoded value
                break;
            default:
                std::cerr << "WARNING, unknown silence type : " << m_silences_ticks[n].m_type << std::endl;
                neededSize = 20;
        }
        
        if (m_silences_ticks[n].m_dotted) neededSize += 40; // a bit approximative for now
        
        ticks_relative_position.addSymbol( m_silences_ticks[n].m_tick_range.from,
                                           m_silences_ticks[n].m_tick_range.to,
                                           neededSize, trackID );
    }    
}
