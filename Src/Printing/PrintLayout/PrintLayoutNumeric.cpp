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

#include "Printing/PrintLayout/PrintLayoutNumeric.h"

#include "Printing/AriaPrintable.h"
#include "Printing/PrintableSequence.h"

using namespace AriaMaestosa;

/**
  * The maximum height (in print units) a level can have. Having a maximum prevents vertical
  * stretching when there are few items in a page, because it would look ugly.
  */
const int MAX_LEVEL_HEIGHT = 72;

/**
  * Elements may be horizontally enlarged (zoomed in) when there is more room than strictly
  * required. Setting a maximum to this prevents excessive horizontal stretching when lots
  * of space is available, because it would look ugly.
  */
const float ELEMENT_MAX_ZOOM = 1.5f;

/**
  * Vertical empty space left above/below the various tracks of a single line.
  * (this space is split between above and below each track)
  */
const int SPACE_BETWEEN_TRACKS = 150;

/**
 * Vertical empty space left above/below the various lines of a page.
 * (this space is split between above and below each line)
 */
const int MARGIN_AROUND_LINE = 250;

/**
  * A page is split between its various lines, proportionnally to how much
  * vertical space they need. However, if there are few tracks in the page,
  * each track will be given a way too large area, which can result in the lines
  * being "widely spread" through the page, i.e. one line at the top, another
  * at the middle of the page (notice the difference with MAX_LEVEL_HEIGHT;
  * MAX_LEVEL_HEIGHT will determine the maximum _used_ space within the space
  * allocated for a line; this variable will _also_ reduce the allocated
  * space so the various lines are not vertically too far from each other).
  * This constant means that the total allocated heigt might not be more than
  * e.g. 1.3 as high as the actually used part of the allocaed height.
  */
const float MAX_HEIGHT_COMPARED_TO_USED_HEIGHT = 1.3f;

// -----------------------------------------------------------------------------------------------------------------

PrintLayoutNumeric::PrintLayoutNumeric()
{
}

// -----------------------------------------------------------------------------------------------------------------

void PrintLayoutNumeric::placeTrackWithinCoords(const int trackID, LayoutLine& line,
                                                int x0, const int y0, const int x1, const int y1)
{
    std::cout << "= placeTrackWithinCoords =\n";
    
    assertExpr(x0, >=, 0);
    assertExpr(x1, >=, 0);
    assertExpr(y0, >=, 0);
    assertExpr(y1, >=, 0);
    assertExpr(y1, >, y0);
    
    TrackCoords* trackCoords = new TrackCoords();
    trackCoords->x0 = x0;
    trackCoords->x1 = x1;
    trackCoords->y0 = y0;
    trackCoords->y1 = y1;
    line.setTrackCoords(trackID, trackCoords);
}

// -----------------------------------------------------------------------------------------------------------------

void PrintLayoutNumeric::placeElementsWithinCoords(LayoutLine& line, int x0, const int x1)
{
    // ---- find total amount of units
    // find total needed width (just in case we have more, then we can spread things a bit!)
    int totalNeededWidth = 0;
    for (int currentLayoutElement=0; currentLayoutElement<line.getLayoutElementCount(); currentLayoutElement++)
    {
        totalNeededWidth += line.getLayoutElement(currentLayoutElement).width_in_print_units;
        totalNeededWidth += MARGIN_AT_MEASURE_BEGINNING;
    }
    
    const int availableWidth = (x1 - x0);
    assertExpr(totalNeededWidth, <=, availableWidth);
    
    float zoom = (float)availableWidth / (float)totalNeededWidth;
    if (zoom > ELEMENT_MAX_ZOOM) zoom = ELEMENT_MAX_ZOOM; // prevent zooming too much, will look weird
    
    // ---- init coords of each layout element
    int xloc = 0;
    
    for (int currentLayoutElement=0; currentLayoutElement<line.getLayoutElementCount(); currentLayoutElement++)
    {
        if (currentLayoutElement  > 0)
        {
            // The margin at the end is provided by the RelativePlacementManager IIRC (FIXME: ugly)
            xloc += line.getLayoutElement(currentLayoutElement-1).width_in_print_units*zoom + MARGIN_AT_MEASURE_BEGINNING;
        }
        
        std::cout << "    - Setting coords of element " << currentLayoutElement
                  << " of current line. xfrom = " << x0 + xloc << "\n";
        
        line.getLayoutElement(currentLayoutElement).setXFrom( x0 + xloc );
        
        if (currentLayoutElement > 0)
        {
            line.getLayoutElement(currentLayoutElement-1).setXTo( line.getLayoutElement(currentLayoutElement).getXFrom() );
        }
    }
    // for last
    xloc += line.getLayoutElement(line.getLayoutElementCount()-1).width_in_print_units*zoom + MARGIN_AT_MEASURE_BEGINNING;
    line.getLayoutElement(line.getLayoutElementCount()-1).setXTo( x0 + xloc );
    
    // check if there is space left between the last element and the end of the line.
    if (x0 + xloc < x1 - 300 )
    {
        line.getLayoutElement(line.getLayoutElementCount()-1).render_end_bar = true;
    }
    
    //assertExpr(line.width_in_units,>,0);
}

// -----------------------------------------------------------------------------------------------------------------

void PrintLayoutNumeric::setLineCoordsAndDivideItsSpace(LayoutLine& line, const int x0, const int y0,
                                                        const int x1, const int y1)
{
    const int trackAmount = line.getTrackAmount();
    
    std::cout << "Line given coords " << x0 << ", " << y0 << " to " << x1 << ", " << y1 << std::endl;
    std::cout << "==setLineCoordsAndDivideItsSpace==\n";
    
    line.m_line_coords = new LineCoords();
    line.m_line_coords->x0 = x0;
    line.m_line_coords->y0 = y0;
    line.m_line_coords->x1 = x1;
    line.m_line_coords->y1 = y1;
        
    // ---- empty space around whole line
    const float heightAvailableForThisLine = (float)(y1 - y0);
    
    if (heightAvailableForThisLine < 0.0001) return; // empty line. TODO: draw empty bars to show there's something?
    
    const int levelHeight = std::min(int(heightAvailableForThisLine/line.getCalculatedHeight()), MAX_LEVEL_HEIGHT);
    //std::cout << "Level height within line : " << levelHeight << "\n";
    
    //FIXME; don't hardcode 70 here, have a formal minimal value
    //assertExpr((heightAvailableForThisLine - margin_below - margin_above)/line.calculateHeight(), >=, 70);
    
    // ---- Determine tracks positions and sizes
    
    //int nonEmptyTrackAmount = 0; // empty tracks must not be counted
    //for (int n=0; n<trackAmount; n++)
    //{        
    //    if (not line.getLineTrackRef(n).empty()) nonEmptyTrackAmount++;
    //}
        
    // space between individual tracks
    //const int space_between_tracks = (nonEmptyTrackAmount > 1 ? SPACE_BETWEEN_TRACKS : 0);
    
    for (int n=0; n<trackAmount; n++)
    {               
        // skip empty tracks
        if (line.getLineTrackRef(n).empty()) continue;
        
        // determine how much vertical space is allocated for this track
        //const float track_height = heightAvailableForThisLine * line.m_height_percent[n]/100.0f;
        //const int trackLevelHeight = line.getLineTrackRef(n).getLevelHeight();
        
        const int levelFrom = line.getLineTrackRef(n).getLevelFrom();
        const int levelTo   = line.getLineTrackRef(n).getLevelTo();

        placeTrackWithinCoords(n, line,
                               x0, y0 + levelFrom*levelHeight,
                               x1, y0 + levelTo*levelHeight);
        
        std::cout << "Track " << n << " in y [" << y0 + line.getLineTrackRef(n).getLevelFrom()*levelHeight
                  << " .. " << y0 + line.getLineTrackRef(n).getLevelTo()*levelHeight << "]\n";
    }
    
    placeElementsWithinCoords(line, x0, x1);
    
}

// -----------------------------------------------------------------------------------------------------------------

void PrintLayoutNumeric::placeLinesInPage(LayoutPage& page, float notation_area_y_from,
                                          const float notation_area_h,
                                          const int pageHeight, const int x0, const int x1)
{
    assert(notation_area_y_from >= 0);
    assert(notation_area_h > 0);
    assert(pageHeight > 0);
    
    const int lineAmount = page.getLineCount();

    const int level_y_amount = page.getLine(lineAmount-1).getLevelTo();
    
    std::cout << "\n========\nplaceTracksInPage\n========\n";
    std::cout << level_y_amount << " levels within " << notation_area_h << " units (vertically)\n";
    
    const float levelHeight = std::min(notation_area_h/(float)level_y_amount, (float)MAX_LEVEL_HEIGHT);
    std::cout << "level height = " << levelHeight << "\n";
    
    // ---- Lay out lines
    for (int l=0; l<lineAmount; l++)
    {
        std::cout << "\n====\nLine " << l << "\n====\n";
        
        LayoutLine& line = page.getLine(l);
        
        const int y_from = notation_area_y_from + line.getLevelFrom()*levelHeight;
        const int y_to   = notation_area_y_from + line.getLevelTo()*levelHeight;
        
        assertExpr(line.getLevelTo(), <=, level_y_amount);
        assertExpr(y_from, >, 0);
        assertExpr(y_from, <, pageHeight);
        
        //FIXME: handle empty lines
        assertExpr(y_to,   >=, y_from);
        //TODO: uncomment
        //assertExpr(y_to,   <, pageHeight);
        
        this->setLineCoordsAndDivideItsSpace(line, x0, y_from,
                                                   x1, y_to);
    }
    
}

// -----------------------------------------------------------------------------------------------------------------
