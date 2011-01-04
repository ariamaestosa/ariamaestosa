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

#include "AriaCore.h"
#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "PreferencesData.h"
#include "Printing/AriaPrintable.h"
#include "Printing/AbstractPrintableSequence.h"
#include "Printing/RenderRoutines.h"

#include <iostream>
#include <wx/dcmemory.h>
#include <wx/print.h>
#include <wx/printdlg.h>

using namespace AriaMaestosa;


#if 0
#pragma mark -
#pragma mark AriaPrintable
#endif


AriaPrintable* AriaPrintable::m_current_printable = NULL;


AriaPrintable::AriaPrintable(wxString title, bool* success) :
    m_normal_font   ( getPrintFont() ),
    m_title_font    ( getPrintTitleFont()  ),
    m_subtitle_font ( getPrintSubtitleFont() )
{
    ASSERT(m_current_printable == NULL);
    
    m_current_printable = this;
    m_hide_empty_tracks = true;
    m_show_track_names  = true;
    
    // ---- Get fonts size
    m_font_height       = -1;
    m_character_width   = -1;
    
    // wx only allows getting font size with a DC or a window... so I need to create
    // a dummy one (FIXME)
    wxBitmap dummyBmp(5,5);
    wxMemoryDC dummy(dummyBmp);
    ASSERT(dummy.IsOk());
    dummy.SetFont( m_normal_font );
    wxCoord txw = -1, txh = -1, descent = -1, externalLeading = -1;
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_font_height      = txh;
    m_character_width  =  txw;
    ASSERT(m_font_height     > 0);
    ASSERT(m_character_width > 0);
    
    txw = -1; txh = -1; descent = -1; externalLeading = -1;
    dummy.SetFont( m_title_font );
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_title_font_height = txh;
    ASSERT(m_title_font_height > 0);

    txw = -1; txh = -1; descent = -1; externalLeading = -1;
    dummy.SetFont( m_subtitle_font );
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_subtitle_font_height= txh;
    ASSERT(m_subtitle_font_height > 0);
    
    // --------------------
    
    // those are not known yet
    m_usable_area_height_page_1 = -1;
    m_usable_area_height        = -1;
    
    m_seq  = NULL;
    
    m_printer_manager = new wxEasyPrintWrapper( title, this, UNITS_PER_CM );
    if (not m_printer_manager->performPageSetup())
    {
        std::cerr << "[AriaPrintable] ERROR: Default page setup failed!\n";
        *success = false;
    }
    else
    {
        findUsableHeight();
        *success = true;
    }
}

// -------------------------------------------------------------------------------------------------------------

AriaPrintable::~AriaPrintable()
{
    m_current_printable = NULL;
}

// -------------------------------------------------------------------------------------------------------------

void AriaPrintable::showPageSetupDialog()
{
    m_printer_manager->performPageSetup(true);
    findUsableHeight();
}

// -------------------------------------------------------------------------------------------------------------

#ifdef __WXMAC__

void AriaPrintable::macEditMargins(wxFrame* parentFrame)
{
    m_printer_manager->macEditMargins(parentFrame);
    findUsableHeight();
}

#endif

// -------------------------------------------------------------------------------------------------------------

void AriaPrintable::findUsableHeight()
{
    const int height = getUnitHeight();
    m_usable_area_height_page_1 = height - m_title_font_height - m_subtitle_font_height - MARGIN_UNDER_PAGE_HEADER;
    m_usable_area_height        = height - m_subtitle_font_height - MARGIN_UNDER_PAGE_HEADER;
    
    std::cout << "[AriaPrintable] Calculating 'm_usable_area_height' with height=" << height << "\n";
    
    ASSERT(m_usable_area_height_page_1 > 0);
    ASSERT(m_usable_area_height > 0);
}

// -------------------------------------------------------------------------------------------------------------

wxString AriaPrintable::getPageSetupSummary() const
{
    ASSERT(m_printer_manager.raw_ptr != NULL);
    return m_printer_manager->getPageSetupSummary();
}

// -------------------------------------------------------------------------------------------------------------

wxPrinterError AriaPrintable::print()
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    ASSERT(m_seq->isLayoutCalculated());

#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(AbstractPrintableSequence::getTitle(m_seq->getSequence()));
#endif

    ASSERT(m_printer_manager != NULL);
    m_printer_manager->setPageCount(m_seq->getPageAmount());
    
    wxPrintDialogData data(m_printer_manager->getPrintData());
    wxPrinter printer(&data);
    const bool success = printer.Print(NULL, m_printer_manager, true /* show dialog */);

#ifdef __WXMAC__
    getMainFrame()->SetTitle(wxT("Aria Maestosa"));
#endif

    wxPrinterError output = wxPrinter::GetLastError();
    
    // for some obscure reason, when cancelling, it doesn't return wxPRINTER_CANCELLED,
    // so work around this manually
    if (not success and output == wxPRINTER_NO_ERROR) output = wxPRINTER_CANCELLED;
    
    return output;
}

// -------------------------------------------------------------------------------------------------------------

AriaPrintable* AriaPrintable::getCurrentPrintable()
{
    ASSERT(m_current_printable != NULL);
    return m_current_printable;
}

// -------------------------------------------------------------------------------------------------------------

void AriaPrintable::printPage(const int pageNum, wxDC& dc,
                              const int x0, const int y0, const int x1, const int y1)
{    
    ASSERT( MAGIC_NUMBER_OK() );

    ASSERT( x1 - x0 > 0 );
    ASSERT( y1 - y0 > 0 );
    
    ASSERT( m_seq != NULL );
    
    const int h = y1 - y0;

    //LayoutPage& page = m_seq->getPage(pageNum-1);

    //const int lineAmount = page.getLineCount();
    // const int lineAmount = page.last_line - page.first_line + 1;

    //std::cout << "page has " << lineAmount << " lines" << std::endl;
    
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    // ---- Debug guides
    if (PRINT_LAYOUT_HINTS)
    {
        //dc.SetPen( wxPen(*wxRED, 25) );
        //dc.DrawLine(x0, y0, x1, y0);
        //dc.DrawLine(x0, y1, x1, y1);
        //dc.DrawLine(x0, y0, x0, y1);
        //dc.DrawLine(x1, y0, x1, y1);

        dc.SetPen( wxPen(*wxLIGHT_GREY, 7) );
        dc.SetFont( wxFont(45,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );
        dc.SetTextForeground( wxColour(175, 175, 175) );
        
        for (int x=x0; x<x1; x += 250)
        {
            dc.DrawLine(x, y0, x, y1);
            dc.DrawText( wxString::Format(wxT("%i"), x), x, y0 - 75 );
        }
        for (int y=y0; y<y1; y += 250)
        {
            dc.DrawLine(x0, y, x1, y);
            dc.DrawText( wxString::Format(wxT("%i"), y), x0 - 150, y );
        }
    }
    
    // ---- Draw title / page number
    wxString label = AbstractPrintableSequence::getTitle(m_seq->getSequence());

    int title_x = x0;

    if (pageNum == 1)
    {
        // on page one make title big and centered
        dc.SetFont( m_title_font );
        wxCoord txw, txh, descent, externalLeading;
        dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
        title_x = (x0+x1)/2 - txw/2;
    }
    else
    {
        // on other pages, repeat the title in small font, not centered, with page number
        label += wxT(", page ");
        label << pageNum;
        dc.SetFont( m_subtitle_font );
    }
    
    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, title_x, y0 );
    
    const int subtitle_y = y0 + m_title_font_height + MARGIN_UNDER_PAGE_HEADER/3;

    /*
     //DEBUG
    dc.SetPen(  wxPen( wxColour(255,0,0), 5 ) );
    dc.DrawLine( x0, subtitle_y, x1, subtitle_y );
    dc.DrawLine( x0, subtitle_y+m_subtitle_font_height, x1, subtitle_y+m_subtitle_font_height );
     */
    
    // ---- draw tempo
    if (pageNum == 1)
    {
        // center note head vertically
        const int tempo_y = subtitle_y + m_subtitle_font_height/2;
        
        dc.SetFont( m_subtitle_font );
        const int tempo_x = 100;
        wxPoint tempoHeadLocation(tempo_x, tempo_y);
        
        dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
        dc.SetBrush( *wxBLACK_BRUSH );
        RenderRoutines::drawNoteHead(dc, tempoHeadLocation, false /* not hollow head */);
        
        const Sequence* seq = m_seq->getSequence();
        const int tempo = seq->getTempo();
        dc.DrawText( wxString::Format(wxT(" = %i"), tempo),
                     tempo_x+HEAD_RADIUS*2,
                     subtitle_y );
        
        //FIXME: don't hardcode values
        dc.DrawLine( tempo_x + HEAD_RADIUS - 8, tempo_y - 10,
                     tempo_x + HEAD_RADIUS - 8, tempo_y - 150 );
    }
    
    // ---- draw track name at top if relevant
    if (pageNum == 1 and m_seq->getTrackAmount() == 1 and not showTrackNames())
    {
        wxString name = m_seq->getTrack(0)->getTrack()->getName();
        dc.DrawText( name,
                     x1 - dc.GetTextExtent(name).GetWidth(),
                     subtitle_y );
    }
    
    // ---- get usable area (some area at the top is reserved to the title) 
    const float notation_area_h  = getUsableAreaHeight(pageNum);
    const float notation_area_y0 = y0 + MARGIN_UNDER_PAGE_HEADER +
                                   (pageNum == 1 ?
                                    m_title_font_height + m_subtitle_font_height :
                                    m_subtitle_font_height);

    ASSERT(notation_area_h > 0);
    ASSERT(h > 0);
    
    ASSERT_E(notation_area_y0 + notation_area_h, <=, y1);

    dc.SetFont( m_normal_font );
    m_seq->printLinesInArea(dc, pageNum-1, notation_area_y0, notation_area_h, h, x0, x1);
    
}
    
// -------------------------------------------------------------------------------------------------------------
 
