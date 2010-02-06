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

#ifndef __PrintLayoutNumeric_H__
#define __PrintLayoutNumeric_H__

#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

namespace AriaMaestosa
{
    
    /**
     * This is the "numeric" counterpart for PrintLayoutAbstract.
     * Where PrintLayoutAbstract layed out things in abstract ways, without taking care
     * of actual measurements in units of things, this class takes the output of
     * of PrintLayoutAbstract and turns them into hard printing coordinates.
     */
    class PrintLayoutNumeric
    {
        /** Reference to the parent sequence */
        PrintableSequence* m_sequence;
        
        /** Internal method called by 'divideLineAmongTracks'. Takes care of setting the coords
          * in the passed 'LineTrackRef', then sets the coords of the layout elements. */
        void placeTrackAndElementsWithinCoords(const int trackID, LayoutLine& line, LineTrackRef& track,
                                               int x0, const int y0, const int x1, const int y1, bool show_measure_number);
        
        /**
         * Internal method called by 'placeLinesInPage'.
         * A single line of measures might contain more than one track, stacked in a vertical fashion.
         * This method receives the space that was allocated for the full line as argument;
         * it first sets the coord of this line. Then it splits the available vertical space between
         * the various tracks that form it, and set the coords of the LineTrackRef referring to each
         * track in this line.
         */
        void divideLineAmongTracks(LayoutLine& line, const int x0, const int y0, const int x1,
                                   const int y1, int margin_below, int margin_above);
        
    public:
        
        PrintLayoutNumeric(PrintableSequence* sequence);
        
        
        /**
         * Main method to use this class. Given a LayoutPage, calculates the actual print coordinates of the
         * lines and elements within this page. The output is achieved by setting previously not set values
         * in the layout classes (FIXME: that's ugly!!)
         *
         * @param text_height       Height of the title header (for page 1),
         *                          height of the bottom page # text (for other pages) [FIXME: ugly to have 2 meanings!]
         * @param level_y_amount    Height of the track in levels [FIXME: which track?? this works on a page!!]
         * @param track_area_height Height of the track in print units [FIXME: which track?? this works on a page!!]
         */
        void placeLinesInPage(LayoutPage& page, const int text_height, const float track_area_height,
                              const int level_y_amount, const int pageHeight,
                              const int x0, const int y0, const int x1);
    };
    
}
#endif
