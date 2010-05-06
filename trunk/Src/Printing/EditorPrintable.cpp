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
#include "wx/filename.h"

#include "Midi/Track.h"
#include "Printing/EditorPrintable.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

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
    EditorPrintable::dc = dc;
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawVerticalDivider(LayoutElement* el, const int y0, const int y1, const bool atEnd)
{
    //if (el->getType() == TIME_SIGNATURE_EL) return;
    
    if (not el->m_render_start_bar and not atEnd) return;
    if (not el->m_render_end_bar and atEnd)       return;

    const int elem_x_start = (atEnd ? el->getXTo() : el->getXFrom());
    
    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawVerticalDivider(const int x, const int y0, const int y1)
{
    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( x, y0, x, y1);
}

// -------------------------------------------------------------------------------------------------------------
    
void EditorPrintable::renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1)
{
    wxString num   = wxString::Format( wxT("%i"), el->num   );
    wxString denom = wxString::Format( wxT("%i"), el->denom );
    
    wxFont oldfont = dc->GetFont();

    //FIXME: don't hardcode fonts here
    //FIXME: find why font sizes are so different between OS X and Linux
#ifdef __WXMAC__
    dc->SetFont( wxFont(150,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD) );
#else
    dc->SetFont( wxFont(100,wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD) );
#endif
    dc->SetTextForeground( wxColour(0,0,0) );
    
    wxSize text_size = dc->GetTextExtent(denom);
    const int text_x = el->getXTo() - text_size.GetWidth() - 20;
    const int text_h = text_size.GetHeight();

    const int numY   = y0 + (y1 - y0)/4;
    const int denomY = y0 + (y1 - y0)*3/4;

    dc->DrawText(num,   text_x, numY - text_h/2);
    dc->DrawText(denom, text_x, denomY - text_h/2);
    
    dc->SetFont(oldfont);    
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawElementBase(LayoutElement& currElem, const LayoutLine& layoutLine,
                                      const bool drawMeasureNumbers, const int y0, const int y1,
                                      const int barYFrom, const int barYTo)
{
    const int elem_x_start = currElem.getXFrom();
    
    // ****** repetitions
    if (currElem.getType() == SINGLE_REPEATED_MEASURE or currElem.getType() == REPEATED_RIFF)
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
            message = to_wxString(layoutLine.getMeasureForElement(&currElem).firstSimilarMeasure+1);
        }
        else if (currElem.getType() == REPEATED_RIFF)
        {
            message = to_wxString(currElem.firstMeasureToRepeat+1) +
            wxT(" - ") +
            to_wxString(currElem.lastMeasureToRepeat+1);
        }
        
        dc->SetTextForeground( wxColour(0,0,255) );
        dc->DrawText( message, elem_x_start,
                     (barYFrom + barYTo)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
    }
    // ****** gathered rest
    else if (currElem.getType() == GATHERED_REST)
    {
        const int elem_x_end = currElem.getXTo();
        const int y          = (barYFrom + barYTo)/2;
        
        wxString label;
        label << currElem.amountOfTimes;

        dc->SetTextForeground( wxColour(0,0,0) );
        dc->SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );
        const int lx = (elem_x_start + elem_x_end)/2 -
                       AriaPrintable::getCurrentPrintable()->getCharacterWidth()*label.size()/2;
        const int ly = y - AriaPrintable::getCurrentPrintable()->getCharacterHeight()*1.5;
        
        dc->DrawText( label, lx, ly );
        
        dc->SetPen( *wxBLACK_PEN );
        dc->SetBrush( *wxBLACK_BRUSH );
        
        const int rect_x_from = elem_x_start + 45;
        const int rect_x_to   = elem_x_end   - 45;

        dc->DrawRectangle( rect_x_from, y - 15, (rect_x_to - rect_x_from), 30 );
        
        dc->SetPen( wxPen(wxColor(0,0,0), 12) );
        dc->DrawLine( rect_x_from, y - 25, rect_x_from, y + 25 );
        dc->DrawLine( rect_x_to  , y - 25, rect_x_to  , y + 25 );
    }
    // ****** play again
    else if (currElem.getType() == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << currElem.amountOfTimes;
        
        dc->SetTextForeground( wxColour(0,0,255) );
        dc->DrawText( label, elem_x_start,
                     (barYFrom + barYTo)/2 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
    }
    // ****** normal measure
    else if (currElem.getType() == SINGLE_MEASURE)
    {
        //std::cout << "---- element is normal\n";
        
        dc->SetTextForeground( wxColour(0,0,255) );
        dc->SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );

        // draw measure ID
        if (drawMeasureNumbers)
        {
            const int meas_id = layoutLine.getMeasureForElement(&currElem).getMeasureID() + 1;
            
            wxString measureLabel;
            measureLabel << meas_id;
            
            dc->DrawText( measureLabel,
                         elem_x_start,
                         y0 - AriaPrintable::getCurrentPrintable()->getCharacterHeight()*1.4 );
        }
        dc->SetTextForeground( wxColour(0,0,0) );
    }
    
    if (currElem.hasTempoChange() and drawMeasureNumbers)
    {
        const int tempo_y = y0 - HEAD_RADIUS - 50;
        const int tempo_x = elem_x_start +
                            // leave space for measure number
                            AriaPrintable::getCurrentPrintable()->getCharacterWidth()*2 +
                            HEAD_RADIUS + 50;
        wxPoint tempoHeadLocation(tempo_x, tempo_y);
        
        dc->SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        dc->SetBrush( *wxBLACK_BRUSH );
        EditorPrintable::drawNoteHead(*dc, tempoHeadLocation, false /* not hollow head */);
        
        const int tempo = currElem.getTempo();
        dc->DrawText( wxString::Format(wxT("= %i"), tempo),
                      tempo_x+HEAD_RADIUS*2,
                      tempo_y+HEAD_RADIUS-AriaPrintable::getCurrentPrintable()->getCharacterHeight() );
        
        //FIXME: don't hardcode values
        dc->DrawLine( tempo_x + HEAD_RADIUS - 8, tempo_y - 10, tempo_x + HEAD_RADIUS - 8, tempo_y-150 );
        
    }
    
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawTrackName(wxDC& dc, const LineTrackRef& currentTrack, int x, int y0, int y1)
{
    if (AriaPrintable::getCurrentPrintable()->showTrackNames())
    {
        dc.SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );

        wxString label = currentTrack.getTrack()->getName();
        wxSize textSize = dc.GetTextExtent(label);
        dc.DrawRotatedText( label, x - textSize.GetHeight() - 20,
                           (y0 + y1)/2 + textSize.GetWidth()/2,
                           90 /* degrees */ );
    }
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawNoteHead(wxDC& dc, const wxPoint headCenter, const bool hollowHead)
{
    const int cx = headCenter.x + (hollowHead ? -2 : 0); // FIXME: the -2 is a hack for the head to blend in the stem
    const int cy = headCenter.y;
    wxPoint points[25];
    for (int n=0; n<25; n++)
    {
        // FIXME - instead of always substracting to radius, just make it smaller...
        const int px = cx + (HEAD_RADIUS-5)*cos(angle);
        const int py = cy + (HEAD_RADIUS - 14)*sin(angle) - HEAD_RADIUS*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f;
        
        const float angle = n/25.0*6.283185f /* 2*PI */;
        points[n] = wxPoint( px, py );
    }
    
    if (hollowHead) dc.DrawSpline(25, points);
    else            dc.DrawPolygon(25, points, -3);
}

// -------------------------------------------------------------------------------------------------------------

Range<int> EditorPrintable::getNoteSymbolX(const int trackID, LayoutLine& line, int noteID)
{
    return tickToX(trackID, line,
                   line.getLineTrackRef(trackID).getTrack()->getNoteStartInMidiTicks(noteID));
}

// -------------------------------------------------------------------------------------------------------------
    
Range<int> EditorPrintable::tickToX(const int trackID, LayoutLine& line, const int tick)
{
    //LineTrackRef& renderInfo = line.getLineTrackRef(trackID);
    
    // find in which measure this tick belongs
    for (int n=0; n<line.getLayoutElementCount(); n++)
    {
        const LayoutElement& el = line.getLayoutElement(n);
        if (el.getType() == GATHERED_REST or el.getType() == REPEATED_RIFF or
            el.getType() == SINGLE_REPEATED_MEASURE or el.getType() == PLAY_MANY_TIMES)
        {
            continue;
        }
        
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
            //    std::cout << "\n/!\\ tickToX didn't find X for tick " << tick << " in measure "
            //              << (meas.id+1) << "\n\n";
            //}
            
            ASSERT_E(elem_w, >, 0);
            
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
    
// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::renderArc(wxDC& dc, const int center_x, const int center_y,
                                const int radius_x, const int radius_y)
{
    wxPoint points[] =
    {
        wxPoint(center_x + radius_x*cos(0.1), center_y + radius_y*sin(0.1)),
        wxPoint(center_x + radius_x*cos(0.3), center_y + radius_y*sin(0.3)),
        wxPoint(center_x + radius_x*cos(0.6), center_y + radius_y*sin(0.6)),
        wxPoint(center_x + radius_x*cos(0.9), center_y + radius_y*sin(0.9)),
        wxPoint(center_x + radius_x*cos(1.2), center_y + radius_y*sin(1.2)),
        wxPoint(center_x + radius_x*cos(1.5), center_y + radius_y*sin(1.5)),
        wxPoint(center_x + radius_x*cos(1.8), center_y + radius_y*sin(1.8)),
        wxPoint(center_x + radius_x*cos(2.1), center_y + radius_y*sin(2.1)),
        wxPoint(center_x + radius_x*cos(2.4), center_y + radius_y*sin(2.4)),
        wxPoint(center_x +  radius_x*cos(2.7), center_y + radius_y*sin(2.7)),
        wxPoint(center_x + radius_x*cos(3.0), center_y + radius_y*sin(3.0)),
    };
    dc.DrawSpline(11, points);
    
#ifdef DEBUG_TIES
    dc.SetPen(*wxRED_PEN);
    dc.DrawRectangle( points[0].x - 20, points[0].y - 20, 40, 40);
    dc.SetPen(*wxGREEN_PEN);
    dc.DrawRectangle( points[10].x - 20, points[10].y - 20, 40, 40);
#endif
}

// -------------------------------------------------------------------------------------------------------------

void EditorPrintable::drawSilence(wxDC* dc, const Range<int> x, const int y, const int levelHeight,
                                  const int type, const bool triplet, const bool dotted)
{
    const int x_center = (x.from + x.to)/2;
    
    // debug draw
    //static int silenceShift = 0;
    //silenceShift += 5;
    //global_dc->DrawLine(x, silences_y + silenceShift % 25, x_to, silences_y + silenceShift % 25);
    
    //{ TODO : use again when repetition is properly back in
    //    LayoutElement* temp = g_printable->getElementForMeasure(measure);
    //    if (temp != NULL and (temp->getType() == REPEATED_RIFF or temp->getType() == SINGLE_REPEATED_MEASURE))
    //        return; //don't render silences in repetions measure!
    //}
    
    dc->SetBrush( *wxBLACK_BRUSH );
    
    int silence_center = -1;
    int silence_radius = -1;
    
    if ( type == 1 )
    {
        dc->SetPen(  *wxTRANSPARENT_PEN  );
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        
        dc->DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                                 /* y */ y,
                                 /* w */ RECTANGULAR_SILENCE_SIZE,
                                 /* h */ (int)round(levelHeight/2.2f));
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 2 )
    {
        silence_radius = RECTANGULAR_SILENCE_SIZE/2;
        dc->SetPen(  *wxTRANSPARENT_PEN  );
        
        const int h = (int)round(levelHeight/2.2f);
        
        dc->DrawRectangle(/* x */ x.from + RECTANGULAR_SILENCE_LEFT_MARGIN,
                                 /* y */ y + levelHeight - h ,
                                 /* w */ RECTANGULAR_SILENCE_SIZE,
                                 /* h */ h);
        silence_center = x.from + RECTANGULAR_SILENCE_LEFT_MARGIN + silence_radius;
    }
    else if ( type == 4 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence4.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        // take the average of 'center-aligned' and 'right-aligned'
        
        // for dotted silences, place them much closer to the left area, to leave room at the right for the dot
        if (dotted) silence_center = (x.from + silence_radius*2);
        else        silence_center = (x_center + (x.to - silence_radius))/2;
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y );
        
        // <debug>
        //global_dc->SetPen(  wxPen( wxColour(255,0,0), 8 ) );
        //global_dc->DrawLine( x.from, silences_y, x.to, silences_y);
        // </debug>
        
    }
    else if ( type == 8 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence8.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        
        if (dotted) silence_center = (x.to - silence_radius - 30);
        else        silence_center = (x.to - silence_radius);
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y + 20);
        
        // <debug>
        //global_dc->SetPen(  wxPen( wxColour(255,0,0), 8 ) );
        //global_dc->DrawLine( x.from, silences_y, x.to, silences_y);
        // </debug>
    }
    else if ( type == 16 )
    {
        static wxBitmap silenceBigger = getScaledBitmap(wxT("silence8.png"), 6.5f);
        
        silence_radius = silenceBigger.GetWidth()/2;
        
        if (dotted) silence_center = (x.to - silence_radius - 30);
        else        silence_center = (x.to - silence_radius);
        
        dc->DrawBitmap( silenceBigger, silence_center - silence_radius, y + 20);
        dc->DrawBitmap( silenceBigger, silence_center - 10, y - 40);

        
        /*
        // TODO : use x_center
        dc->SetPen(  wxPen( wxColour(0,0,0), 8 ) );
        silence_radius = 25;
        const int mx = x.to - silence_radius*2;
        const int sy = y + 80;
        
        ASSERT_E(mx, >, -5000);
        ASSERT_E(sy, >, -5000);

        std::cout << "1/16th : x=" << mx << ", y=" << sy << "\n";
        
        wxPoint points[] =
        {
            wxPoint(mx,     sy+50),
            wxPoint(mx+25,  sy),
            wxPoint(mx,     sy),
        };
        dc->DrawSpline(3, points);
        wxPoint points2[] =
        {
            wxPoint(mx+20,  sy+5),
            wxPoint(mx+50,  sy-50),
            wxPoint(mx+25,  sy-50),
        };
        dc->DrawSpline(3, points2);
        
        dc->DrawCircle(mx,    sy, 6);
        dc->DrawCircle(mx+25, sy-50, 6);
        
        silence_center = mx + 50/2;
         */
    }
    
    // dotted
    if (dotted)
    {
        dc->SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        dc->SetBrush( *wxBLACK_BRUSH );
        wxPoint headLocation( silence_center + silence_radius + DOT_SIZE*2, y+30 );
        dc->DrawEllipse( headLocation, wxSize(DOT_SIZE, DOT_SIZE) );
    }
    
    // triplet
    if (triplet)
    {
        wxPen tiePen( wxColour(0,0,0), 10 ) ;
        dc->SetPen( tiePen );
        dc->SetBrush( *wxTRANSPARENT_BRUSH );
        
        const int radius_x = 50;
        
        const int base_y = y + 150;
        
        static wxSize triplet_3_size = dc->GetTextExtent(wxT("3"));
        
        renderArc(*dc, silence_center - 9, base_y, radius_x, 80);
        dc->SetTextForeground( wxColour(0,0,0) );
        dc->DrawText( wxT("3"), silence_center - triplet_3_size.GetWidth()/3 - 11, base_y-20 );
    }
}
    
// -------------------------------------------------------------------------------------------------------------

wxBitmap EditorPrintable::getScaledBitmap(const wxString& fileName, float scale)
{
    wxImage tempImage(getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + fileName,
                      wxBITMAP_TYPE_PNG);
    wxImage image = getPrintableImage(tempImage);
    return wxBitmap(image.Scale(image.GetWidth()*scale, image.GetHeight()*scale),wxIMAGE_QUALITY_HIGH);
}

// -------------------------------------------------------------------------------------------------------------

wxImage EditorPrintable::getPrintableImage(const wxImage& image)
{
#ifdef __WXMSW__
    wxImage printimage = image;
    if (printimage.HasAlpha())
    {
        printimage.ConvertAlphaToMask();
    }
    if (printimage.HasMask())
    {
        const wxColour mask(printimage.GetMaskRed(), printimage.GetMaskGreen(), printimage.GetMaskBlue());
        const wxColour back = *wxWHITE;
        
        printimage.Replace(mask.Red(), mask.Green(), mask.Blue(),
                           back.Red(), back.Green(), back.Blue());
    }
    return printimage;
#else
    return image;
#endif
}

// -------------------------------------------------------------------------------------------------------------

