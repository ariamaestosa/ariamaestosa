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

#include "Printing/RenderRoutines.h"

#include "IO/IOUtils.h"
#include "Range.h"
#include <wx/dc.h>
#include <wx/filename.h>
#include <wx/image.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::RenderRoutines;

void AriaMaestosa::RenderRoutines::drawSilence(wxDC* dc, const Range<int> x, const int y,
                                               const int levelHeight, const int type, const bool triplet,
                                               const bool dotted)
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

// -------------------------------------------------------------------------------------------------------

void AriaMaestosa::RenderRoutines::drawNoteHead(wxDC& dc, const wxPoint headCenter, const bool hollowHead)
{
    const int cx = headCenter.x + (hollowHead ? -2 : 0); // FIXME: the -2 is a hack for the head to blend in the stem
    const int cy = headCenter.y;
    wxPoint points[25];
    for (int n=0; n<25; n++)
    {
        const float angle = n/25.0*6.283185f /* 2*PI */;
        
        // FIXME - instead of always substracting to radius, just make it smaller...
        const int px = cx + (HEAD_RADIUS-5)*cos(angle);
        const int py = cy + (HEAD_RADIUS - 14)*sin(angle) - HEAD_RADIUS*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f;
        
        points[n] = wxPoint( px, py );
    }
    
    if (hollowHead) dc.DrawSpline(25, points);
    else            dc.DrawPolygon(25, points, -3);
}

// -------------------------------------------------------------------------------------------------------

void AriaMaestosa::RenderRoutines::renderArc(wxDC& dc, const int center_x, const int center_y,
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

// -------------------------------------------------------------------------------------------------------

wxBitmap AriaMaestosa::RenderRoutines::getScaledBitmap(const wxString& fileName, float scale)
{
    wxImage tempImage(getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + fileName,
                      wxBITMAP_TYPE_PNG);
    wxImage image = getPrintableImage(tempImage);
    return wxBitmap(image.Scale(image.GetWidth()*scale, image.GetHeight()*scale),wxIMAGE_QUALITY_HIGH);
}

// -------------------------------------------------------------------------------------------------------

wxImage AriaMaestosa::RenderRoutines::getPrintableImage(const wxImage& image)
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
