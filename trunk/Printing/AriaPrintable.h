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


#ifndef _print_base_h_
#define _print_base_h_

#include <vector>
#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "wx/wx.h"

const bool PRINT_LAYOUT_HINTS = false;

namespace AriaMaestosa
{
    class PrintableSequence;    
    
    class AriaPrintable
    {
        DECLARE_MAGIC_NUMBER();
        
        friend class AriaMaestosa::LayoutLine;
        
        PrintableSequence* seq;

    public:
        // ---------------------------------------
        // global info for printables, read-only
        // FIXME - find cleaner way
        int text_height;
        int text_height_half;
        int character_width;
        // ---------------------------------------
        
        /**
         * 'seq' remains owned by the caller, AriaPrintable will not delete it. Caller must not delete 'seq' before
         * AriaPrintable is deleted too.
         */
        AriaPrintable(PrintableSequence* seq);
        
        virtual ~AriaPrintable();
        
        /**
          * Called when it is time to print a page.
          *
          * @param pageNum      ID of the page we want to print
          * @param dc           The wxDC onto which stuff to print is to be rendered
          * @param x0           x origin coordinate from which drawing can occur
          * @param y0           y origin coordinate from which drawing can occur
          * @param w            Width of the printable area
          * @param h            Height of the printable area
          */
        void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int w, const int h);
        
        int print();
    };
    
    AriaPrintable* getCurrentPrintable();
    
    
}

#endif
