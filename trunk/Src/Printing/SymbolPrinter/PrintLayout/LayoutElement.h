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

#ifndef __LAYOUT_ELEMENT_H__
#define __LAYOUT_ELEMENT_H__


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
        //SINGLE_REPEATED_MEASURE,
        EMPTY_MEASURE,
        GATHERED_REST,
        //REPEATED_RIFF,
        //PLAY_MANY_TIMES,
        LINE_HEADER, // like the clef and key (on a score) or the word TAB and the tuning (for tabs)
        TIME_SIGNATURE_EL
    };
    
    class LayoutElement
    {
        LayoutElementType m_type;
        
        /** value is -1 if no tempo change occurs on this element, contains the new tempo otherwise */
        int m_tempo_change;
        
        int m_x, m_x2;
    public:
        LayoutElement(LayoutElementType type_arg, int measure_arg = -1);

        LayoutElementType getType() const { return m_type; }
        
        int m_measure;
         /** 
           * @brief whether to draw a vertical divider line at the start of this element 
           * @note  most Layout Elements don't draw their end vertical line, since the next element
           *        generally draws a line at its beginning. So that's for special cases.
           */
        bool m_render_end_bar;
        
        /**
          * @brief whether to draw a vertical divider line at the start of this element 
          * @note for most elements, this is true by defaut
          */
        bool m_render_start_bar;
        
        int width_in_print_units;
        
        /** used for time sig elements */
        int num, denom;
        
        /** used in many-measure repetitions : the measures that repeat */
        int firstMeasure, lastMeasure;
        
        /** used in many-measure repetitions : the measures being repeated */
        int firstMeasureToRepeat, lastMeasureToRepeat;
        
        /** used for 'play many times' events, or gathered rests */
        int amountOfTimes;
        
        
        
        void setXFrom(const int x) { m_x = x; };
        void setXTo  (const int x) { m_x2 = x; };
        const int getXFrom() const { return m_x; }
        const int getXTo()   const { return m_x2; }
        
        /** @return       whether a tempo change occurs at this element */
        bool  hasTempoChange() const { return m_tempo_change != -1; }
        
        /** 
          * @return the new tempo at this element
          * @pre    'hasTempoChange' must have returned 'true'
          */
        int   getTempo      () const { return m_tempo_change;       }
        
        /** 
          * @brief           records a tempo change at this layout element
          * @param newTempo  the new value of the tempo, in bpm, at this layout element
          */
        void  setTempoChange(const int newTempo) { m_tempo_change = newTempo; }
    };
    
      
}

#endif
