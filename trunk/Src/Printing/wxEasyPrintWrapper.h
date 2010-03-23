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


#ifndef _WX_EASY_PRINT_WRAPPER_
#define _WX_EASY_PRINT_WRAPPER_


#include "wx/wx.h"
#include "wx/print.h"

namespace AriaMaestosa
{
    const int MARGIN_UNDER_PAGE_HEADER = 200;
            
    class wxEasyPrintWrapper : public wxPrintout
    {
        AriaPrintable*        m_print_callback;
        wxPageSetupDialogData m_page_setup;
        wxPaperSize           m_paper_id;
        int                   m_orient;
        int                   m_page_amount;
        
        /** in millimeters */
        int                   m_left_margin;
        
        /** in millimeters */
        int                   m_top_margin;
        
        /** in millimeters */
        int                   m_right_margin;
        
        /** in millimeters */
        int                   m_bottom_margin;
                
        float                 m_unit_width;
        float                 m_unit_height;
        
        /** Resolution (number of DC units per centimeter) */
        float                 m_units_per_cm;
        
    public:
        
        LEAK_CHECK();
        

        /** 
          * basic constructor. call early, as this constructor does not do anything beyond
          * some basic initialisation.
          * @param units_per_cm  wanted resolution (number of DC units per centimeter)
          */
        wxEasyPrintWrapper(wxString title, AriaPrintable* printCallBack, float units_per_cm);
        
        /** 
         * @return the number of units horizontally on the printable area of the paper
         * @precondition  'performPageSetup' must have been called at least once prior to calling this
         */
        int getUnitWidth() const
        {
            assertExpr(m_unit_width, >, 0);
            return m_unit_width;
        }
        
        /** 
          * @return the number of units vertically on the printable area of the paper
          * @precondition  'performPageSetup' must have been called at least once prior to calling this
          */
        int getUnitHeight() const
        {
            assertExpr(m_unit_height, >, 0);
            return m_unit_height;
        }

        /** 
         * Perform print setup (paper size, orientation, etc...), with our without dialog.
         * @postcondition sets m_unit_width and m_unit_height in AriaPrintable, as well
         *                as usable_area_y* (FIXME: ugly design)
         */
        bool performPageSetup(const bool showPageSetupDialog=false);

        wxPrintData getPrintData();  
        
        /**
         * @return a string that summarises the current page setup information
         */
        wxString getPageSetupSummary() const;
        
        /** 
          * Call AFTER the PrintableSequence had its layout calculated.
          * Call BEFORE trying to actually print this wxPrintout.
          */
        void setPrintableSequence(PrintableSequence* printableSequence);
        
        // ---- callbacks from wxPrintout
        virtual void OnBeginPrinting();
        virtual bool OnBeginDocument(int startPage, int endPage);
        virtual void GetPageInfo(int *minPage, int *maxPage, int *pageSelFrom, int *pageSelTo);
        virtual bool HasPage(int pageNum);
        virtual bool OnPrintPage(int pageNum);
        virtual void OnEndPrinting();
    };
    
} // end namespace


#endif
