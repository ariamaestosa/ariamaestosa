
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "Midi/MeasureData.h"
#include "Editors/GuitarEditor.h"
#include "IO/IOUtils.h"
#include "Printing/TabPrint.h"
#include "Printing/PrintingBase.h"
#include "Printing/PrintLayout.h"

namespace AriaMaestosa
{
    
    
TablaturePrintable::TablaturePrintable(Track* track, bool checkRepetitions_bool)
{

    parent = track;
    calculateLayoutElements(track, checkRepetitions_bool, layoutPages, measures);
    
    std::cout << "\nContains " << layoutPages.size() << " pages\n" << std::endl;
}

TablaturePrintable::~TablaturePrintable()
{
}

wxString TablaturePrintable::getTitle()
{
    wxString song_title = parent->sequence->suggestTitle();
    wxString track_title = parent->getName();
    
    wxString final_title;
    
    // give song title
    if(song_title.IsSameAs(_("Untitled")))
        final_title = _("Aria Maestosa song");
    else
        final_title = song_title;
    
    // give track name, if any
    if(!track_title.IsSameAs(_("Untitled"))) final_title += (wxT(", ") + track_title);
    
    std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;
}
int TablaturePrintable::getPageAmount()
{
    return layoutPages.size();
}
void TablaturePrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    dc.SetFont( wxFont(11,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    
    wxString label( getTitle() + wxT(", page ") );
    label << pageNum;
    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, x0, y0 );
    
    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);
    
    // -------------------- generate the tablature  -------------------- 
    assertExpr(pageNum-1,<,(int)layoutPages.size());
    const int lineAmount = layoutPages[pageNum-1].layoutLines.size();
    
    /*
     the equivalent of 4 times "text_height" will not be printed with notation.
     first : space for title at the top
     second and third : space below title at top
     fourth : space below the last line
     */
     
    const float line_height = ((float)h - (float)text_height*4) / (float)maxLinesInPage;

    const int notation_area_origin_y = y0 + text_height*3;
    for(int l=0; l<lineAmount; l++)
    { 
        const float line_y_from = notation_area_origin_y + line_height*l;
        const float line_y_to = notation_area_origin_y + line_height*(l+0.6f);
        
        drawLine(layoutPages[pageNum-1].layoutLines[l], dc, x0, (int)round(line_y_from), x1, (int)round(line_y_to));
    }
}

void TablaturePrintable::drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1)
{
    static const int line_width = 1;
    
    Track* track = line.parent;
    
    // draw tab background
    dc.SetPen(  wxPen( wxColour(125,125,125), line_width ) );
    
    const int stringAmount = line.string_amount;
    const float stringHeight = (float)(y1 - y0) / (float)(stringAmount-1);
    
    for(int s=0; s<stringAmount; s++)
    {
        const int y = (int)round(y0 + stringHeight*s);
        dc.DrawLine(x0, y, x1, y);
    }
    
    std::vector<LayoutElement>& layoutElements = line.layoutElements;
    
    const int layoutElementsAmount = layoutElements.size();
    
    int xloc = 2;
    
    // 2 spaces allocated for left area of the tab
    float widthOfAChar = (float)(x1 - x0) / (float)(line.charWidth+2);
    
    assertExpr(line.charWidth,>,0);
    assertExpr(widthOfAChar,>,0);
    
    for(int n=0; n<layoutElementsAmount; n++)
    {
        const int meas_x_start = x0 + (int)round(xloc*widthOfAChar) - widthOfAChar;
        const int meas_x_end = x0 + (int)round((xloc+layoutElements[n].charWidth)*widthOfAChar);
        const int mesa_w = meas_x_end - meas_x_start;
        
        // draw vertical line that starts measure
        dc.SetPen(  wxPen( wxColour(0,0,0), line_width*2 ) );
        dc.DrawLine( meas_x_start, y0, meas_x_start, y1);
        
        dc.SetTextForeground( wxColour(0,0,255) );

        // ****** empty measure
        if(layoutElements[n].type == EMPTY_MEASURE)
        {
            
        } 
        // ****** repetitions
        else if(layoutElements[n].type == SINGLE_REPEATED_MEASURE or layoutElements[n].type == REPEATED_RIFF)
        {
            // FIXME - why do I cut apart the measure and not the layout element?
            if(measures[layoutElements[n].measure].cutApart)
			{
                // TODO...
                //dc.SetPen(  wxPen( wxColour(0,0,0), line_width*4 ) );
                //dc.DrawLine( meas_x_start, y0, meas_x_start, y1);
			}
			
			wxString message;
			if(layoutElements[n].type == SINGLE_REPEATED_MEASURE)
			{
				message = to_wxString(measures[layoutElements[n].measure].firstSimilarMeasure+1);
			}
			else if(layoutElements[n].type == REPEATED_RIFF)
			{
				message =	to_wxString(layoutElements[n].firstMeasureToRepeat+1) +
				wxT(" - ") + 
				to_wxString(layoutElements[n].lastMeasureToRepeat+1);
			}
            
            dc.DrawText( message, meas_x_start + widthOfAChar/2, (y0+y1)/2-text_height_half );
        }
        // ****** play again
        else if(layoutElements[n].type == PLAY_MANY_TIMES)
        {
            wxString label(wxT("X"));
            label << layoutElements[n].amountOfTimes;
            dc.DrawText( label, meas_x_start + widthOfAChar/2, (y0+y1)/2-text_height_half );
        }
        // ****** normal measure
        else if(layoutElements[n].type == SINGLE_MEASURE)
        {  
            // draw measure ID
            wxString measureLabel;
            measureLabel << (measures[layoutElements[n].measure].id+1);
            dc.DrawText( measureLabel, meas_x_start - widthOfAChar/4, y0 - text_height*1.4 );
            
            dc.SetTextForeground( wxColour(0,0,0) );
            

            const int firstNote = measures[layoutElements[n].measure].firstNote;
            const int firstTick = measures[layoutElements[n].measure].firstTick;
            const int lastTick = measures[layoutElements[n].measure].lastTick;
            const int lastNote = measures[layoutElements[n].measure].lastNote;
            
            for(int i=firstNote; i<lastNote; i++)
            {
                const int tick = track->getNoteStartInMidiTicks(i);
                const int string = track->getNoteString(i);
                const int fret = track->getNoteFret(i);
                
                const float nratio = ((float)(tick - firstTick) / (float)(lastTick - firstTick));
                
                if(fret < 0)  dc.SetTextForeground( wxColour(255,0,0) );
                
                // substract from width to leave some space on the right (coordinate is from the left of the text string so we need extra space on the right)
                // if fret number is greater than 9, the string will have two characters so we need to recenter it a bit more
                const int drawX = nratio * (mesa_w-widthOfAChar*1.5) + meas_x_start + (fret > 9 ? widthOfAChar/4 : widthOfAChar/2);
                const int drawY = y0 + stringHeight*string - text_height_half*0.8;
                wxString label = to_wxString(fret);
                
                dc.DrawText( label, drawX, drawY );
                
                if(fret < 0)  dc.SetTextForeground( wxColour(0,0,0) );
            }
        }
        
        xloc += layoutElements[n].charWidth;
        
    }//next element
}


}
