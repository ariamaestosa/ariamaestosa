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
#include "Printing/PrintingBase.h"
#include "Printing/LayoutTree.h"

namespace AriaMaestosa
{

EditorPrintable::EditorPrintable(Track* track)
{
    this->track = track;
}

EditorPrintable::~EditorPrintable(){}

void EditorPrintable::setCurrentDC(wxDC* dc)
{
    EditorPrintable::dc = dc;
}

void EditorPrintable::setCurrentTrack(LayoutLine* line)
{
    EditorPrintable::currentLine = line;
}

void EditorPrintable::placeTrackAndElementsWithinCoords(LayoutLine& line, TrackRenderInfo& track,  int x0, const int y0, const int x1, const int y1, bool show_measure_number)
{
    std::cout << "= placeTrackAndElementsWithinCoords =\n";

    assertExpr(x0, >=, 0);
    assertExpr(x1, >=, 0);
    assertExpr(y0, >=, 0);
    assertExpr(y1, >=, 0);
    
    track.x0 = x0;
    track.x1 = x1;
    track.y0 = y0;
    track.y1 = y1;
    
    track.show_measure_number = show_measure_number;
    
    if(&line.getTrackRenderInfo() != &track) std::cerr << "TrackRenderInfo is not the right one!!!!!!!!!\n";
    std::cout << "coords for track " << line.getCurrentTrack() << " : " << x0 << ", " << y0 << ", " << x1 << ", " << y1 << std::endl;
    
    // 2 spaces allocated for left area of the line
    track.pixel_width_of_an_unit = (float)(x1 - x0) / (float)(line.width_in_units+2);
    std::cout << "    Line has " << line.width_in_units << " units. pixel_width_of_an_unit=" << track.pixel_width_of_an_unit << "\n";
    
    track.layoutElementsAmount = line.layoutElements.size();
    
    int xloc = 0;
        
    // init coords of each layout element
    for(int currentLayoutElement=0; currentLayoutElement<track.layoutElementsAmount; currentLayoutElement++)
    {
        if(currentLayoutElement == 0) xloc = 1;
        else if(currentLayoutElement > 0) xloc += line.layoutElements[currentLayoutElement-1].width_in_units;
        
        std::cout << "    - Setting coords of element " << currentLayoutElement << " of current line. xfrom = " <<
        x0 + (int)round(xloc*track.pixel_width_of_an_unit) - track.pixel_width_of_an_unit << "\n";
        line.layoutElements[currentLayoutElement].setXFrom(x0 + (int)round(xloc*track.pixel_width_of_an_unit) - track.pixel_width_of_an_unit);
        
        if(currentLayoutElement > 0)
        {
            line.layoutElements[currentLayoutElement-1].setXTo( line.layoutElements[currentLayoutElement].getXFrom() );
        }
    }
    // for last
    line.layoutElements[line.layoutElements.size()-1].setXTo( x1 );
    
    assertExpr(line.width_in_units,>,0);
    assertExpr(track.pixel_width_of_an_unit,>,0);
}


/*
 int EditorPrintable::getCurrentElementXStart()
 {
 return currentLine->x0 + (int)round(currentLine->xloc*pixel_width_of_an_unit) - pixel_width_of_an_unit;
 }*/

LayoutElement* EditorPrintable::getElementForMeasure(const int measureID)
{
    assert(currentLine != NULL);
    std::vector<LayoutElement>& layoutElements = currentLine->layoutElements;
    const int amount = layoutElements.size();
    for(int n=0; n<amount; n++)
    {
        if(layoutElements[n].measure == measureID) return &layoutElements[n];
    }
    return NULL;
}

void EditorPrintable::drawVerticalDivider(LayoutElement* el, const int y0, const int y1)
{
    if(el->getType() == TIME_SIGNATURE) return;
    
    const int elem_x_start = el->getXFrom();
    
    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
}

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

const int EditorPrintable::getElementCount() const
{
    return currentLine->getTrackRenderInfo().layoutElementsAmount;
}

LayoutElement* EditorPrintable::continueWithNextElement(const int currentLayoutElement)
{
    TrackRenderInfo& renderInfo = currentLine->getTrackRenderInfo();
    
    if(!(currentLayoutElement < renderInfo.layoutElementsAmount))
    {
        //std::cout << "---- returning NULL because we have a total of " << layoutElementsAmount << " elements\n";
        return NULL;
    }
    
    std::vector<LayoutElement>& layoutElements = currentLine->layoutElements;
    
    const int elem_x_start = currentLine->layoutElements[currentLayoutElement].getXFrom();
    
    dc->SetTextForeground( wxColour(0,0,255) );
    
    // ****** empty measure
    if(layoutElements[currentLayoutElement].getType() == EMPTY_MEASURE)
    {
    }
    // ****** time signature change
    else if(layoutElements[currentLayoutElement].getType() == TIME_SIGNATURE)
    {
    }
    // ****** repetitions
    else if(layoutElements[currentLayoutElement].getType() == SINGLE_REPEATED_MEASURE or
            layoutElements[currentLayoutElement].getType() == REPEATED_RIFF)
    {
        // FIXME - why do I cut apart the measure and not the layout element?
        /*
         if(measures[layoutElements[n].measure].cutApart)
         {
         // TODO...
         //dc.SetPen(  wxPen( wxColour(0,0,0), 4 ) );
         //dc.DrawLine( elem_x_start, y0, elem_x_start, y1);
         }
         */
        
        wxString message;
        if(layoutElements[currentLayoutElement].getType() == SINGLE_REPEATED_MEASURE)
        {
            message = to_wxString(currentLine->getMeasureForElement(currentLayoutElement).firstSimilarMeasure+1);
        }
        else if(layoutElements[currentLayoutElement].getType() == REPEATED_RIFF)
        {
            message =    to_wxString(layoutElements[currentLayoutElement].firstMeasureToRepeat+1) +
            wxT(" - ") +
            to_wxString(layoutElements[currentLayoutElement].lastMeasureToRepeat+1);
        }
        
        dc->DrawText( message, elem_x_start + renderInfo.pixel_width_of_an_unit/2,
                     (renderInfo.y0 + renderInfo.y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** play again
    else if(layoutElements[currentLayoutElement].getType() == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << layoutElements[currentLayoutElement].amountOfTimes;
        dc->DrawText( label, elem_x_start + renderInfo.pixel_width_of_an_unit/2,
                     (renderInfo.y0 + renderInfo.y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** normal measure
    else if(layoutElements[currentLayoutElement].getType() == SINGLE_MEASURE)
    {
        //std::cout << "---- element is normal\n";
        
        // draw measure ID
        if(renderInfo.show_measure_number)
        {
            const int meas_id = currentLine->getMeasureForElement(currentLayoutElement).id+1;
            
            wxString measureLabel;
            measureLabel << meas_id;
            
            dc->DrawText( measureLabel,
                         elem_x_start - ( meas_id > 9 ? renderInfo.pixel_width_of_an_unit/4 : renderInfo.pixel_width_of_an_unit/5 ),
                         renderInfo.y0 - getCurrentPrintable()->text_height*1.4 );
        }
        
        dc->SetTextForeground( wxColour(0,0,0) );
    }
    
    //std::cout << "---- Returning element " << currentLayoutElement << " which is " << &layoutElements[currentLayoutElement] << std::endl;
    return &layoutElements[currentLayoutElement];
}


int EditorPrintable::getNotePrintX(int noteID)
{
    return tickToX( currentLine->getTrack()->getNoteStartInMidiTicks(noteID) );
}
int EditorPrintable::tickToX(const int tick)
{
    TrackRenderInfo& renderInfo = currentLine->getTrackRenderInfo();
    
    // find in which measure this tick belongs
    for(int n=0; n<renderInfo.layoutElementsAmount; n++)
    {
        MeasureToExport& meas = currentLine->getMeasureForElement(n);
        if(meas.id == -1) continue; // nullMeasure, ignore
        const int firstTick = meas.firstTick;
        const int lastTick  = meas.lastTick;
        
        if(tick >= firstTick and tick < lastTick)
        {
            //std::cout << tick << " is within bounds " << firstTick << " - " << lastTick << std::endl;
            const int elem_x_start = currentLine->layoutElements[n].getXFrom();
            const int elem_x_end = currentLine->layoutElements[n].getXTo();
            const int elem_w = elem_x_end - elem_x_start;
            
            //std::cout << "tickToX found tick " << tick << std::endl;
            
            float nratio = meas.ticks_relative_position[tick].relativePosition;
            
            assertExpr(elem_w, >, 0);
            
            return (int)round(nratio * elem_w + elem_x_start);
        }
        else 
        // given tick is before the current line
        if(tick < firstTick) 
        {
            //std::cout << "tickToX Returning -1 A\n";
            return -1;
        }
        else
        /* the tick we were given is not on the current line, but on the next.
         * this probably means there is a tie from a note on one line to a note
         * on another line. Return a X at the very right of the page.
         * FIXME - it's not necessarly a tie
         * FIXME - ties aand line warping need better handling
         */
        if(n==renderInfo.layoutElementsAmount-1 and tick >= lastTick)
        {
            //std::cout << "tickToX Returning -" <<  (currentLine->layoutElements[n].getXTo() + 10) << " B\n";

            return currentLine->layoutElements[n].getXTo() + 10;
        }
    }
    
    return -1;
}

/**
  * tickToX returns the beginning of the area allocated to a tick; this method returns its end.
  */
int EditorPrintable::tickToXLimit(const int tick)
{
    int closest = getClosestTickFrom(tick+1);
    if (closest == -1) return -1;
    
    return tickToX(closest);
}
    
/**
  * This method exists because in multi-track prints, one track may request more ticks (and thus more space) than
  * the other. When rendering, the other can thus call this to know the extent of its free size and center things
  * instead of leaving holes (but for this they must have silence information). returns -1 if nothing was found.
  */
int EditorPrintable::getClosestTickFrom(const int tick)
{
    //std::cout << "getClosestXFromTick " << tick << std::endl;
    TrackRenderInfo& renderInfo = currentLine->getTrackRenderInfo();
    
    // find in which measure this tick belongs
    for(int n=0; n<renderInfo.layoutElementsAmount; n++)
    {
        MeasureToExport& meas = currentLine->getMeasureForElement(n);
        if(meas.id == -1) continue; // nullMeasure, ignore
        const int firstTick = meas.firstTick;
        const int lastTick  = meas.lastTick;
        //std::cout << "checkingWithinMeasure {" << firstTick << ", " << lastTick << "}\n";
        
        int closestTick = -1;
        
        if(tick >= firstTick and tick < lastTick)
        {
            
            std::map<int, TickPosInfo>::iterator iter;
            for( iter = meas.ticks_relative_position.begin(); iter != meas.ticks_relative_position.end(); ++iter )
            {
                const int thisTick = iter->first;
                //std::cout << "    checking tick " << thisTick << std::endl;
                if (thisTick >= tick and (thisTick < closestTick or closestTick == -1))
                {
                    closestTick = thisTick;
                    //std::cout << "    closestTick = " << closestTick << std::endl;
                }
            } // end for
            if (closestTick == -1) closestTick = lastTick;
            
            //std::cout << "FINAL closestTick = " << closestTick << std::endl;
            return closestTick;
        }
    } // end for
    
    //std::cerr << "Couldn't find any measure containing this tick\n";
    
    return -1;
}

}