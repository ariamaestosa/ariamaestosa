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

namespace AriaMaestosa
{
    class Sequence;
    
    class KeyrollPrintableSequence : public AbstractPrintableSequence
    {
        int m_page_amount;
        
    public:
        
        KeyrollPrintableSequence(Sequence* parent);
        
        /**
         * @return the number of pages that was determined to be necessary in order to print this sequence.
         */
        virtual int getPageAmount() const { return m_page_amount; }

        /**
          * @brief implement callback from parent
          */
        virtual void printLinesInArea(wxDC& dc, const int page, const float notation_area_y0,
                                      const float notation_area_h, const int pageHeight,
                                      const int x0, const int x1);
        
        /**
         * @brief implement callback from parent
         */
        virtual void calculateLayout(bool checkRepetitions);

    };
}


#endif