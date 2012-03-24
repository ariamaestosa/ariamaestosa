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


#ifndef __ARIA_PRINTABLE_H__
#define __ARIA_PRINTABLE_H__

#include "Printing/wxEasyPrintWrapper.h"
#include <wx/print.h>

/** @defgroup printing */

const bool PRINT_LAYOUT_HINTS = false;

namespace AriaMaestosa
{
    const float UNITS_PER_CM = 315.0f;

    
    class AbstractPrintableSequence;   
    class LayoutLine;
    
    /**
      * @brief public interface to the low-level aspects of the printing module
      *
      * Receives a PrintableSequence and manages the page setup/print dialogs, handles print
      * margins, initiates the actual printing by delegating to wxEasyPrintWrapper.
      * @ingroup printing
      */
    class AriaPrintable : public IPrintCallback
    {
        DECLARE_MAGIC_NUMBER();
        
        friend class AriaMaestosa::LayoutLine;
        friend class AriaMaestosa::wxEasyPrintWrapper;

        AbstractPrintableSequence* m_seq;

        wxFont m_normal_font;
        wxFont m_title_font;
        wxFont m_subtitle_font;
        
        /** There can only be one instance at a time. Holds the current instance, or NULL if there is none */
        static AriaPrintable* m_current_printable;
        
        /** The object used to wrap some of the wx printing framework */
        OwnerPtr<wxEasyPrintWrapper> m_printer_manager;
        
        /** height of font used for measure numbers, tablature fret numbers, etc... */
        int m_font_height;
        
        /** "avreage" width of one character of font used for measure numbers,
          * tablature fret numbers, etc...
          */
        int m_character_width;
        
        /** height of font used for titles and captions (i.e. the bigger font) */
        int m_title_font_height;
        
        /** height of font used for second-level titles and captions (i.e. bigger than normal but
          * smaller than title font)
          */
        int m_subtitle_font_height;

        int m_usable_area_height_page_1;
        int m_usable_area_height;
        
        /** Called after page setup is first configured OR is later changed; calculates
          * 'm_usable_area_height' members, for use by 'getUsableAreaHeight'
          */
        void findUsableHeight();
        
        bool m_hide_empty_tracks;
        
        bool m_show_track_names;
        
    public:
        
        LEAK_CHECK();
        
        /**
         * Construct this object BEFORE calling 'calculateLayout' in the prntable sequence, since the printable
         * sequence may need some info from the Ariaprintable
         * FIXME(DESIGN): call sequence relies on implementation details
         *
         * @param seq the sequence to print. Remains owned by the caller, AriaPrintable will not delete it.
         *            The caller must not delete the passed sequence before AriaPrintable is deleted too.
         * @param[out] success Whether setting up the printing subsystem was successful.
         * @note it is mandatory to call 'setSequence' before actually printing
         */
        AriaPrintable(wxString title, bool* success);
        
        virtual ~AriaPrintable();
        
        /**
          * @param seq the sequence to print. Remains owned by the caller, AriaPrintable will not delete it.
          *            The caller must not delete the passed sequence before AriaPrintable is deleted too.
          */
        void setSequence(AbstractPrintableSequence* seq) { m_seq = seq; }
        
        /** @return whether to hide empty tracks */
        bool   hideEmptyTracks() const { return m_hide_empty_tracks; }
        
        void   hideEmptyTracks(const bool hide) { m_hide_empty_tracks = hide; }
        
        /** @return whether to print track names */
        bool   showTrackNames() const { return m_show_track_names; }
        
        void   showTrackNames(const bool show) { m_show_track_names = show; }
        
        /** 
          * @brief Initiate the actual printing of the sequence
          * @pre  the 'calculateLayout' method of the printable sequence has been called
          * @return whether an error occurred (wxPRINTER_ERROR), whether printing was cancelled
          *         (wxPRINTER_CANCELLED), or whether all is well (wxPRINTER_NO_ERROR)
          */ 
        wxPrinterError print();
        
        /** 
          * @return the number of units used horizontally in the coordinate system set-up for
          * the kind of paper that is selected.
          */
        int getUnitWidth () const { return m_printer_manager->getUnitWidth();  }
        
        /** 
          * @return the number of units used vertically in the coordinate system set-up for
          * the kind of paper that is selected.
          */
        int getUnitHeight() const { return m_printer_manager->getUnitHeight(); }
        
        /** 
          * There can only be one object of type AriaPrintable at any given time.
          * @return the current AriaPrintable object, or NULL if there is none currently.
          */
        static AriaPrintable* getCurrentPrintable();

        /** @return the height (in print units) of the font used for printing notation itelf */
        int getCharacterHeight    () const { return m_font_height;      }

        /** 
          * @return approximate width (in print units) of one character of the font used for
          *         printing notation itelf
          */
        int getCharacterWidth     () const { return m_character_width;  }
        

        /** @return a human-readable summary of the current page setup */
        PageSetupSummary getPageSetupSummary() const;
        
        /** show the page setup dialog, so that the user can edit the page setup settings */
        void showPageSetupDialog();
        
#ifdef __WXMAC__
        /**
         * Because the native page setup dialog on mac does not allow editing margins
         */
        void macEditMargins(wxFrame* parentFrame);
#endif
        
        /** 
          * @return the height of the area that is usable for actual notation printing
          *         (excluding margins, headers, etc...) in print units
          */
        int getUsableAreaHeight(const int pageNumber) const
        {
            ASSERT(pageNumber >= 1);
            ASSERT(m_usable_area_height_page_1 != -1 and m_usable_area_height != -1);
            return (pageNumber == 1 ? m_usable_area_height_page_1 : m_usable_area_height);
        }
        
        /**
         * Called (by wxEasyPrintWrapper) when it is time to print a page.
         *
         * @param pageNum      ID of the page we want to print
         * @param dc           The wxDC onto which stuff to print is to be rendered
         * @param x0           x origin coordinate from which drawing can occur
         * @param y0           y origin coordinate from which drawing can occur
         */
        virtual void printPage(const int pageNum, wxDC& dc,
                               wxGraphicsContext* gc,
                               const int x0, const int y0,
                               const int x1, const int y1);

        wxFont getNormalFont() { return m_normal_font; }
        
        float getUnitsPerCm() const
        {
            return m_printer_manager->getUnitsPerCm();
        }
        
    };
        
    
}

#endif
