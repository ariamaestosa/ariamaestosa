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
#include <wx/filename.h>
#include <wx/dc.h>

#include "GUI/GraphicalTrack.h"
#include "Midi/Track.h"
#include "PreferencesData.h"
#include "Printing/AriaPrintable.h"
#include "Printing/RenderRoutines.h"
#include "Printing/SymbolPrinter/EditorPrintable.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutLine.h"

using namespace AriaMaestosa;
    
// -------------------------------------------------------------------------------------------------------------
    
EditorPrintable::EditorPrintable()
{
}

// -------------------------------------------------------------------------------------------------------------

EditorPrintable::~EditorPrintable()
{
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::setCurrentDC(wxDC* dc)
{
    m_dc = dc;
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawVerticalDivider(LayoutElement* el, const int y0, const int y1, const bool atEnd)
{    
    if (not el->m_render_start_bar and not atEnd) return;
    if (not el->m_render_end_bar and atEnd)       return;

    const int elem_x_start = (atEnd ? el->getXTo() : el->getXFrom());
    
    // draw vertical line that starts measure
    m_dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    m_dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawVerticalDivider(const int x, const int y0, const int y1)
{
    // draw vertical line that starts measure
    m_dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    m_dc->DrawLine( x, y0, x, y1);
}

// -------------------------------------------------------------------------------------------------------------
    
void EditorPrintable::renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1)
{
    wxString num   = wxString::Format( wxT("%i"), el->num   );
    wxString denom = wxString::Format( wxT("%i"), el->denom );
    
    wxFont oldfont = m_dc->GetFont();

    m_dc->SetFont( getTimeSigPrintFont() );
    m_dc->SetTextForeground( wxColour(0,0,0) );
    
    wxSize text_size = m_dc->GetTextExtent(denom);
    const int text_x = el->getXTo() - text_size.GetWidth() - 20;
    const int text_h = text_size.GetHeight();

    const int numY   = y0 + (y1 - y0)/4;
    const int denomY = y0 + (y1 - y0)*3/4;

    m_dc->DrawText(num,   text_x, numY - text_h/2);
    m_dc->DrawText(denom, text_x, denomY - text_h/2);
    
    m_dc->SetFont(oldfont);    
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawElementBase(LayoutElement& currElem, const LayoutLine& layoutLine,
                                      const bool drawMeasureNumbers, const int y0, const int y1,
                                      const int barYFrom, const int barYTo)
{
    const int elem_x_start = currElem.getXFrom();
    
    // ****** repetitions
    /*
    if (currElem.getType() == SINGLE_REPEATED_MEASURE or currElem.getType() == REPEATED_RIFF)
    {
        wxString message;
        if (currElem.getType() == SINGLE_REPEATED_MEASURE)
        {
            message = to_wxString(layoutLine.getMeasureForElement(&currElem).firstSimilarMeasure+1);
        }
        else if (currElem.getType() == REPEATED_RIFF)
        {
            message = to_wxString(currElem.firstMeasureToRepeat+1) +
            wxT(" - ") +
            to_wxString(currElem.lastMeasureToRepeat+1);
        }
        
        m_dc->SetTextForeground( wxColour(0,0,255) );
        m_dc->DrawText(message, elem_x_start,
                       (barYFrom + barYTo)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
    }
    // ****** gathered rest
    else */
    if (currElem.getType() == GATHERED_REST)
    {
        const int elem_x_end = currElem.getXTo();
        const int y          = (barYFrom + barYTo)/2;
        
        wxString label;
        label << currElem.amountOfTimes;

        m_dc->SetTextForeground( wxColour(0,0,0) );
        m_dc->SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );
        
        const int lx = (elem_x_start + elem_x_end)/2 -
                       AriaPrintable::getCurrentPrintable()->getCharacterWidth()*label.size()/2;
        const int ly = y - AriaPrintable::getCurrentPrintable()->getCharacterHeight()*1.5;
        
        m_dc->DrawText( label, lx, ly );
        
        m_dc->SetPen( *wxBLACK_PEN );
        m_dc->SetBrush( *wxBLACK_BRUSH );
        
        const int rect_x_from = elem_x_start + 45;
        const int rect_x_to   = elem_x_end   - 45;

        m_dc->DrawRectangle( rect_x_from, y - 15, (rect_x_to - rect_x_from), 30 );
        
        m_dc->SetPen( wxPen(wxColor(0,0,0), 12) );
        m_dc->DrawLine( rect_x_from, y - 25, rect_x_from, y + 25 );
        m_dc->DrawLine( rect_x_to  , y - 25, rect_x_to  , y + 25 );
    }
    // ****** play again
    /*
    else if (currElem.getType() == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << currElem.amountOfTimes;
        
        m_dc->SetTextForeground( wxColour(0,0,255) );
        m_dc->DrawText(label, elem_x_start,
                       (barYFrom + barYTo)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2);
    }
     */
    // ****** normal measure
    else if (currElem.getType() == SINGLE_MEASURE)
    {
        //std::cout << "---- element is normal\n";
        
        m_dc->SetTextForeground( wxColour(0,0,255) );
        m_dc->SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );

        // draw measure ID
        if (drawMeasureNumbers)
        {
            const int meas_id = layoutLine.getMeasureForElement(&currElem).getMeasureID() + 1;
            
            wxString measureLabel;
            measureLabel << meas_id;
            
            m_dc->DrawText(measureLabel,
                           elem_x_start,
                           y0 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()*1.4);
        }
        m_dc->SetTextForeground( wxColour(0,0,0) );
    }
    
    if (currElem.hasTempoChange() and drawMeasureNumbers)
    {
        const int tempo_y = y0 - HEAD_RADIUS - 50;
        const int tempo_x = elem_x_start +
                            // leave space for measure number
                            AriaPrintable::getCurrentPrintable()->getCharacterWidth()*2 +
                            HEAD_RADIUS + 50;
        wxPoint tempoHeadLocation(tempo_x, tempo_y);
        
        m_dc->SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        m_dc->SetBrush( *wxBLACK_BRUSH );
        RenderRoutines::drawNoteHead(*m_dc, tempoHeadLocation, false /* not hollow head */);
        
        const int tempo = currElem.getTempo();
        m_dc->DrawText(wxString::Format(wxT("= %i"), tempo),
                       tempo_x + HEAD_RADIUS*2,
                       tempo_y + HEAD_RADIUS-AriaPrintable::getCurrentPrintable()->getCharacterHeight() );
        
        //FIXME: don't hardcode values
        m_dc->DrawLine( tempo_x + HEAD_RADIUS - 8, tempo_y - 10, tempo_x + HEAD_RADIUS - 8, tempo_y-150 );
        
    }
    
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawTrackName(wxDC& dc, const LineTrackRef& currentTrack, int x, int y0, int y1)
{
    if (AriaPrintable::getCurrentPrintable()->showTrackNames())
    {
        dc.SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );

        const GraphicalTrack* gtrack = currentTrack.getTrack();
        wxString label = gtrack->getTrack()->getName();
        wxSize textSize = dc.GetTextExtent(label);
        dc.DrawRotatedText(label, x - textSize.GetHeight() - 20,
                           (y0 + y1)/2 + textSize.GetWidth()/2,
                           90 /* degrees */ );
    }
}

// -------------------------------------------------------------------------------------------------------------

Range<int> EditorPrintable::getNoteSymbolX(const int trackID, LayoutLine& line, int noteID)
{
    const GraphicalTrack* gtrack = line.getLineTrackRef(trackID).getTrack();
    return tickToX(trackID, line, gtrack->getTrack()->getNoteStartInMidiTicks(noteID));
}

// -------------------------------------------------------------------------------------------------------------
    
Range<int> EditorPrintable::tickToX(const int trackID, LayoutLine& line, const int tick)
{
    // find in which measure this tick belongs
    const int count = line.getLayoutElementCount();
    for (int n=0; n<count; n++)
    {
        const LayoutElement& el = line.getLayoutElement(n);
        
        const PrintLayoutMeasure& meas = line.getMeasureForElement(n);
        const int firstTickInMeasure = meas.getFirstTick();
        const int lastTickInMeasure  = meas.getLastTick();
        if (el.getType() == GATHERED_REST and tick >= firstTickInMeasure and tick < lastTickInMeasure)
        {
            fprintf(stderr, "[EditorPrintable::tickToX] WARNING: tick is inside gathered rest!\n");
            return Range<int>(-1, -1);
        }
        
        if (el.getType() == GATHERED_REST) continue;
        if (meas == NULL_MEASURE) continue;

        if (tick >= firstTickInMeasure and tick < lastTickInMeasure)
        {
            const int elem_x_start = line.getLayoutElement(n).getXFrom();
            const int elem_x_end = line.getLayoutElement(n).getXTo();
            const int elem_w = elem_x_end - elem_x_start;
                        
            const Range<float> relative_pos = meas.getTicksPlacementManager().getSymbolRelativeArea( tick );
            
            ASSERT_E(elem_w, >, 0);
            
            const int from = (int)round(relative_pos.from * elem_w + elem_x_start);
            int to = (int)round(relative_pos.to * elem_w + elem_x_start);
            
            // FIXME: arbitrary max length for now
            // TODO: ideally, when one symbol is given too much space, the max size reached detection should
            //       be detected earlier in order to allow other symbols on the line to use the available space
            if ((to - from) > 175) to = from + 175;
            
            ASSERT_E(to, >=, from);
            ASSERT_E(from, >=, 0);
            ASSERT_E(to, >=, 0);
            
            return Range<int>(from, to);
        }
        // We went beyond that tick and didn't find anything yet
        else if (tick < firstTickInMeasure)
        {
            std::cerr << "[EditorPrintable::tickToX] Cannot find X coordinate for tick " << tick
                      << " (I'm now at tick " << firstTickInMeasure << " and still didn't find it.)\n";
            
            return Range<int>(-1, -1);
        }
        /* the tick we were given is not on the current line, but on the next.
         * this probably means there is a tie from a note on one line to a note
         * on another line. Return a X at the very right of the page.
         * FIXME - it's not necessarly a tie
         * FIXME - ties and line warping need better handling
         */
        else if (n == line.getLayoutElementCount() - 1 and tick >= lastTickInMeasure)
        {
            //std::cout << "tickToX Returning -" <<  (currentLine->layoutElements[n].getXTo() + 10) << " B\n";

            return Range<int>(line.getLayoutElement(n).getXTo() + 10, line.getLayoutElement(n).getXTo() + 10);
        }
    }
    
    std::cerr << "[EditorPrintable::tickToX] Cannot find X coordinate for tick " << tick << "\n";
    
    return Range<int>(-1, -1);
}
    
// -------------------------------------------------------------------------------------------------------------

