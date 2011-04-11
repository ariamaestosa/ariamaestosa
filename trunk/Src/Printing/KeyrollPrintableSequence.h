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

#ifndef __KEYROLL_PRINTABLE_SEQUENCE_H__
#define __KEYROLL_PRINTABLE_SEQUENCE_H__

#include "Printing/AbstractPrintableSequence.h"
#include <vector>

namespace AriaMaestosa
{
    class Sequence;
    
    class KeyrollPrintableSequence : public AbstractPrintableSequence
    {
        int m_min_pitch, m_max_pitch;

        struct Page
        {
            int m_first_tick;
            int m_last_tick;
            int m_first_print_unit;
            int m_last_print_unit;
        };
        std::vector<Page> m_pages;
        
        bool m_compact;
        int m_compact_pitch_count;
        std::vector<int> m_compact_info;
        
        std::vector<wxColor> m_colors;
        
        float m_units_per_tick;
        
        /** Space to leave vertically between notes (in millimeters) */
        float m_vertical_margin;
        
    public:
        
        KeyrollPrintableSequence(Sequence* parent, float cmPerBeat, float verticalMargin,
                                 bool compact, std::vector<wxColor> colors);
        
        /**
         * @return the number of pages that was determined to be necessary in order to print this sequence.
         */
        virtual int getPageAmount() const { return m_pages.size(); }

        /**
          * @brief implement callback from parent
          */
        virtual void printLinesInArea(wxDC& dc, wxGraphicsContext* grctx, const int page,
                                      const float notation_area_y0, const float notation_area_h,
                                      const int pageHeight, const int x0, const int x1);
        
        /**
         * @brief implement callback from parent
         */
        virtual void calculateLayout();

    };
}


#endif