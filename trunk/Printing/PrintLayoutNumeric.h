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

#include "Printing/PrintLayoutAbstract.h"
#include "Printing/PrintLayoutLine.h"

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
        PrintableSequence* m_sequence;
        
    public:
        
        PrintLayoutNumeric(PrintableSequence* sequence);
        
        void placeTrackAndElementsWithinCoords(const int trackID, LayoutLine& line, LineTrackRef& track,
                                               int x0, const int y0, const int x1, const int y1, bool show_measure_number);
        
        void divideLineAmongTracks(LayoutLine& line, const int x0, const int y0, const int x1,
                                   const int y1, int margin_below, int margin_above);
        
        /**
         * @param text_height       Height of the title header (for page 1), height of the bottom page # text (for other pages)
         * @param level_y_amount    Height of the track in levels
         * @param track_area_height Height of the track in print units
         */
        void placeLinesInPage(LayoutPage& page, const int text_height, const float track_area_height,
                              const int level_y_amount, const int pageHeight,
                              const int x0, const int y0, const int x1);
    };
    
}
#endif
