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
#include "wx/print.h"

const bool PRINT_LAYOUT_HINTS = false;

namespace AriaMaestosa
{
    class PrintableSequence;    
    class QuickPrint;
    
    class AriaPrintable
    {
        DECLARE_MAGIC_NUMBER();
        
        friend class AriaMaestosa::LayoutLine;
        friend class AriaMaestosa::QuickPrint;

        PrintableSequence* seq;

        /** This is set by QuickPrint during its setup */
        int m_unit_width, m_unit_height;
        
        /**
         * Called (by QuickPrint) when it is time to print a page.
         *
         * @param pageNum      ID of the page we want to print
         * @param dc           The wxDC onto which stuff to print is to be rendered
         * @param x0           x origin coordinate from which drawing can occur
         * @param y0           y origin coordinate from which drawing can occur
         * @param w            Width of the printable area
         * @param h            Height of the printable area
         */
        void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int w, const int h);
        
        /** There can only be one instance at a time. Holds the current instance, or NULL if there is none */
        static AriaPrintable* m_current_printable;
        
        QuickPrint* m_printer_manager;
        
    public:
        // ---------------------------------------
        // global info for printables, read-only
        // FIXME - find cleaner way
        int text_height;
        int text_height_half;
        int character_width;
        // ---------------------------------------
        
        /**
         * Construct this object BEFORE calling 'calculateLayout' in the prntable sequence, since the printable
         * sequence may need some info from the Ariaprintable (FIXME: confusing design)
         *
         * @param seq the sequence to print. Remains owned by the caller, AriaPrintable will not delete it.
         *            The caller must not delete the passed sequence before AriaPrintable is deleted too.
         * @param[out] success Whether setting up the printing subsystem was successful.
         */
        AriaPrintable(PrintableSequence* seq, bool* success);
        
        virtual ~AriaPrintable();
        
        /** 
          * @brief Initiate the actual printing of the sequence
          * @precondition  the 'calculateLayout' method of the printable sequence has been called
          * @return whether an error occurred (wxPRINTER_ERROR), whether printing was cancelled
          *         (wxPRINTER_CANCELLED), or whether all is well (wxPRINTER_NO_ERROR)
          */ 
        wxPrinterError print();
        
        /** 
          * @return the number of units used horizontally in the coordinate system set-up for
          * the kind of paper that is selected.
          */
        int getUnitWidth () const { assert(m_unit_width != -1); return m_unit_width;  }
        
        /** 
          * @return the number of units used vertically in the coordinate system set-up for
          * the kind of paper that is selected.
          */
        int getUnitHeight() const { assert(m_unit_height != -1); return m_unit_height; }
        
        /** 
          * There can only be one object of type AriaPrintable at any given time.
          * @return the current AriaPrintable object, or NULL if there is none currently.
          */
        static AriaPrintable* getCurrentPrintable();

    };
        
    
}

#endif
