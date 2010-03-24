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

#ifndef _layout_tree_h_
#define _layout_tree_h_


namespace AriaMaestosa
{
    class Track;
    class AriaPrintable;
        
    /**
      * used to determine the order of what appears in the file.
      * the order is found first before writing anything because that allows more flexibility
      */
    enum LayoutElementType
    {
        SINGLE_MEASURE,
        SINGLE_REPEATED_MEASURE,
        EMPTY_MEASURE,
        REPEATED_RIFF,
        PLAY_MANY_TIMES,
        LINE_HEADER, // like the clef and key (on a score) or the word TAB and the tuning (for tabs)
        TIME_SIGNATURE_EL
    };
    
    class LayoutElement
    {
        LayoutElementType type;
        
        int x, x2;
    public:
        LayoutElement(LayoutElementType type_arg, int measure_arg = -1);

        LayoutElementType getType() const { return type; }
        
        int measure;
        
        /** Most Layout Elements don't draw their end vertical line, since the next element generally
          * draws a line at its beginning. But for special cases you can use this */
        bool m_render_end_bar;
        
        bool m_render_start_bar;
        
        int width_in_print_units;
        
        /** used for time sig elements */
        int num, denom;
        
        // used in many-measure repetitions. the first 2 ones are the measures that repeat, the last 2 ones the measures being repeated
        int firstMeasure, lastMeasure, firstMeasureToRepeat, lastMeasureToRepeat;
        
        int amountOfTimes; // used for 'play many times' events
        
        
        
        void setXFrom(const int x) { this->x = x; };
        void setXTo  (const int x) { this->x2 = x; };
        const int getXFrom() const { return x; }
        const int getXTo()   const { return x2; }
    };
    
      
}

#endif
