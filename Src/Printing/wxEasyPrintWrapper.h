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


#ifndef __WX_EASY_PRINT_WRAPPER_H__
#define __WX_EASY_PRINT_WRAPPER_H__

#include "Utils.h"
#include <wx/print.h>

namespace AriaMaestosa
{
    const int MARGIN_UNDER_PAGE_HEADER = 200;
    
    /**
      * @brief abstract interface used by wxEasyPrintWrapper to communicate with its caller
      * @ingroup printing
      */
    class IPrintCallback
    {
    public:
        
        virtual ~IPrintCallback() {}
        
        /**
         * Called (by wxEasyPrintWrapper) when it is time to print a page.
         *
         * @param pageNum      ID of the page we want to print
         * @param dc           The wxDC onto which stuff to print is to be rendered
         * @param x0           x origin coordinate from which drawing can occur
         * @param y0           y origin coordinate from which drawing can occur
         */
        virtual void printPage(const int pageNum, wxDC& dc,
                               const int x0, const int y0,
                               const int x1, const int y1) = 0;
        
    };
    
    /**
      * @brief utility class to wrap the wxWidgets print framework into a higher-level API
      * @ingroup printing
      */
    class wxEasyPrintWrapper : public wxPrintout
    {
        IPrintCallback*       m_print_callback;
        wxPageSetupDialogData m_page_setup;
        wxPaperSize           m_paper_id;
        
#if wxCHECK_VERSION(2,9,1)
        wxPrintOrientation    m_orient;
#else
        int                   m_orient;
#endif
        
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
        
        void updateCoordinateSystem();

        void updateMarginMembers( bool savePreferences );
        
    public:
        
        LEAK_CHECK();
        

        /** 
          * basic constructor. call early, as this constructor does not do anything beyond
          * some basic initialisation.
          * @param units_per_cm  wanted resolution (number of DC units per centimeter)
          */
        wxEasyPrintWrapper(wxString title, IPrintCallback* printCallBack, float units_per_cm);
        
        /** 
         * @return the number of units horizontally on the printable area of the paper
         * @pre  'performPageSetup' must have been called at least once prior to calling this
         */
        int getUnitWidth() const
        {
            ASSERT_E(m_unit_width, >, 0);
            return m_unit_width;
        }
        
        float getUnitsPerCm() const
        {
            return m_units_per_cm;
        }
        
        /** 
          * @return the number of units vertically on the printable area of the paper
          * @pre  'performPageSetup' must have been called at least once prior to calling this
          */
        int getUnitHeight() const
        {
            ASSERT_E(m_unit_height, >, 0);
            return m_unit_height;
        }

        /** 
         * Perform print setup (paper size, orientation, etc...), with our without dialog.
         * @postcondition sets m_unit_width and m_unit_height in AriaPrintable, as well
         *                as usable_area_y*
         */
        bool performPageSetup(const bool showPageSetupDialog=false);

#ifdef __WXMAC__
        /**
          * Because the native page setup dialog on mac does not allow editing margins
          */
        void macEditMargins(wxFrame* parentFrame);
#endif
        
        wxPrintData getPrintData();  
        
        /**
         * @return a string that summarises the current page setup information
         */
        wxString getPageSetupSummary() const;
        
        /** 
          * Call before trying to actually print this wxPrintout.
          */
        void setPageCount(const int pageCount);
                
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
