#include "AriaCore.h"

#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "Printing/PrintableSequence.h"
#include "Printing/TabPrint.h"
#include "Printing/ScorePrint.h"

#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

#include "wx/wx.h"
#include "wx/paper.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include <iostream>

using namespace AriaMaestosa;

namespace AriaMaestosa
{
    
    const int MARGIN_UNDER_PAGE_HEADER = 200;

    class QuickPrint : public wxPrintout
    {
        AriaPrintable*        m_print_callback;
        wxPageSetupDialogData m_page_setup;
        wxPaperSize           m_paper_id;
        int                   m_orient;
        int                   m_page_amount;
        
        static const int      m_brush_size = 15;
        
        
    public:
 
        LEAK_CHECK();

#if 0
#pragma mark -
#pragma mark QuickPrint : public interface
#endif
        
        // -----------------------------------------------------------------------------------------------------
        
        /** basic constructor. call early, as this constructor does not do anything beyond
          * some basic initialisation.
          */
        QuickPrint(wxString title, AriaPrintable* printCallBack) : wxPrintout( title )
        {
            m_print_callback = printCallBack;
            m_page_amount    = -1; // unknown yet
            m_orient         = wxPORTRAIT;
            m_paper_id       = wxPAPER_LETTER; //TODO: remember user's favorite paper
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        /** 
          * Perform print setup (paper size, orientation, etc...), with our without dialog.
          * @postcondition sets m_unit_width and m_unit_height in AriaPrintable, as well
          *                as usable_area_y* (FIXME: ugly design)
          */
        bool performPageSetup(const bool showPageSetupDialog=false)
        {
            std::cout << "Showing PAGE SETUP dialog with paper " << wxThePrintPaperDatabase->ConvertIdToName( m_paper_id ).mb_str() << "\n";
            
            
            wxPrintData printdata;
            printdata.SetPrintMode( wxPRINT_MODE_PRINTER );
            printdata.SetOrientation( m_orient ); // wxPORTRAIT, wxLANDSCAPE
            printdata.SetPaperId( m_paper_id );
            printdata.SetNoCopies(1);
            
            m_page_setup = wxPageSetupDialogData(printdata);
            //page_setup.SetMarginTopLeft(wxPoint(16, 16));
            //page_setup.SetMarginBottomRight(wxPoint(16, 16));
            
            if (showPageSetupDialog)
            {
                // let user change default values if he wishes to
                wxPageSetupDialog dialog( NULL,  &m_page_setup );
                if (dialog.ShowModal()==wxID_OK)
                {
                    m_page_setup = dialog.GetPageSetupData();
                    m_paper_id   = m_page_setup.GetPaperId();
                    m_orient     = m_page_setup.GetPrintData().GetOrientation();
                }
                else
                {
                    std::cout << "user canceled at page setup dialog" << std::endl;
                    return false;
                }
            }
            
            assert(m_paper_id != wxPAPER_NONE);
            assert(m_page_setup.GetPaperId() != wxPAPER_NONE);
            
            wxPoint marginTopLeft     = m_page_setup.GetMarginTopLeft();
            wxPoint marginBottomRight = m_page_setup.GetMarginBottomRight();

            // ---- set-up coordinate system however we want
            // we'll use it when drawing
            wxSize paperSize = m_page_setup.GetPaperSize();
            int large_side = std::max(paperSize.GetWidth(), paperSize.GetHeight());
            int small_side = std::min(paperSize.GetWidth(), paperSize.GetHeight());
            
            assert(large_side > 0);
            assert(small_side > 0);
            
            // here, the 31.5 factor was determined empirically
            if (m_orient == wxPORTRAIT)
            {
                m_print_callback->m_unit_width  = (int)(small_side*31.5f);
                m_print_callback->m_unit_height = (int)(large_side*31.5f);
            }
            else
            {
                m_print_callback->m_unit_width  = (int)(large_side*31.5f);
                m_print_callback->m_unit_height = (int)(small_side*31.5f);
            }
            
            assert(m_print_callback->m_unit_width  > 0);
            assert(m_print_callback->m_unit_height > 0);
            
            
            const int height = m_print_callback->m_unit_height;
            
            //std::cout << PRINT_VAR(height) << " - " << PRINT_VAR(m_print_callback->m_title_font_height)
            //          << " - " <<  PRINT_VAR(MARGIN_UNDER_PAGE_HEADER) << std::endl;
            //std::cout << PRINT_VAR(height) << " - " << PRINT_VAR(m_print_callback->m_subtitle_font_height)
            //          << " - " <<  PRINT_VAR(MARGIN_UNDER_PAGE_HEADER) << std::endl;
            m_print_callback->m_usable_area_height_page_1 = height - m_print_callback->m_title_font_height - MARGIN_UNDER_PAGE_HEADER;
            m_print_callback->m_usable_area_height        = height - m_print_callback->m_subtitle_font_height - MARGIN_UNDER_PAGE_HEADER;
            
            std::cout << "Calculating 'm_usable_area_height' with height=" << height << "\n";
            
            assert(m_print_callback->m_usable_area_height_page_1 > 0);
            assert(m_print_callback->m_usable_area_height > 0);
            return true;
        }
        
        // -----------------------------------------------------------------------------------------------------

        wxPrintData getPrintData()
        {
            return m_page_setup.GetPrintData();
        }
        
        // -----------------------------------------------------------------------------------------------------

        /**
          * @return a string that summarises the current page setup information
          */
        wxString getPageSetupSummary() const
        {
            // printdata.SetOrientation( m_orient ); // wxPORTRAIT, wxLANDSCAPE
            // printdata.SetPaperId( m_paper_id );
            return wxThePrintPaperDatabase->ConvertIdToName( m_paper_id ) + wxT(", ") +
                   //I18N: for printing (page orientation)
                   (m_orient == wxPORTRAIT ? _("Portrait") : _("Landscape"));
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        /** Call AFTER the PrintableSequence had its layout calculated.
          * Call BEFORE trying to actually print this wxPrintout.
          */
        void setPrintableSequence(PrintableSequence* printableSequence)
        {
            assert(printableSequence->isLayoutCalculated());
            m_page_amount    = printableSequence->getPageAmount();
        }
        
        // -----------------------------------------------------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark QuickPrint : callbacks from wxPrintout
#endif
        
        bool OnPrintPage(int pageNum)
        {
            std::cout << "\n============\nprinting page " << pageNum << "\n==============" << std::endl;
            
            wxDC* ptr = GetDC();
            if (ptr == NULL or not ptr->IsOk())
            {
                std::cerr << "DC is not Ok, interrupting printing" << std::endl;
                return false;
            }
            wxDC& dc = *ptr;
            
            
            wxRect bounds = GetLogicalPageRect();
            
            const int x0     = bounds.x;
            const int y0     = bounds.y;
            //const int width  = bounds.width;
            //const int height = bounds.height;
            
            std::cout << "printable area : (" << x0 << ", " << y0 << ") to ("
                      << (x0 + bounds.width) << ", " << (y0 + bounds.height) << ")" << std::endl;
            
            assertExpr(bounds.width,  >, 0);
            assertExpr(bounds.height, >, 0);
            
            //FIXME: this is a workaround
            if (bounds.width  != m_print_callback->m_unit_width or
                bounds.height != m_print_callback->m_unit_height)
            {
                std::cerr << "WTF!!! Wx didn't give me the size I asked for!!\n";
                m_print_callback->m_unit_width  = bounds.width;
                m_print_callback->m_unit_height = bounds.height;
            }
            
            assertExpr(bounds.width,  ==, m_print_callback->m_unit_width  );
            assertExpr(bounds.height, ==, m_print_callback->m_unit_height );

            m_print_callback->printPage(pageNum, dc, x0, y0);
            
            return true;
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        void OnBeginPrinting()
        {
            std::cout << "---- ON BEGIN PRINTING ----\n" << m_print_callback->m_unit_width << "x"
                      << m_print_callback->m_unit_height << "\n";
            
            // setup coordinate system
            assert(m_print_callback->m_unit_width  > 0);
            assert(m_print_callback->m_unit_height > 0);
            
            FitThisSizeToPageMargins(wxSize(m_print_callback->m_unit_width, m_print_callback->m_unit_height),
                                     m_page_setup);
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        bool OnBeginDocument(int startPage, int endPage)
        {
            std::cout << "\n\n=============================\n";
            std::cout << "beginning to print document, from page " << startPage << " to "
                      << endPage << std::endl;
            std::cout << "=============================\n\n";
            
            return wxPrintout::OnBeginDocument(startPage, endPage);
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        void GetPageInfo(int *minPage, int *maxPage, int *pageSelFrom, int *pageSelTo)
        {
            assert(m_page_amount > 0);
            
            *minPage = 1;
            *maxPage = m_page_amount;
            
            *pageSelFrom = 1;
            *pageSelTo   = m_page_amount;
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        bool HasPage(int pageNum)
        {
            assert(m_page_amount > 0);
            
            if (pageNum >= 1 and pageNum <= m_page_amount) return true;
            else                                           return false;
        }
        
        // -----------------------------------------------------------------------------------------------------
        
        void OnEndPrinting()
        {
        }
    };
    
}

// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark AriaPrintable
#endif


AriaPrintable* AriaPrintable::m_current_printable = NULL;


AriaPrintable::AriaPrintable(PrintableSequence* seq, bool* success) :
    m_normal_font   (75,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL),
    m_title_font    (130, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD  ),
    m_subtitle_font (90,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)
{
    INIT_MAGIC_NUMBER();
    assert(m_current_printable == NULL);
    
    m_current_printable = this;
    m_unit_width        = -1;
    m_unit_height       = -1;
    
    // ---- Get fonts size
    m_font_height       = -1;
    m_character_width   = -1;
    
    // wx only allows getting font size with a DC or a window... so I need to create
    // a dummy one (FIXME)
    wxBitmap dummyBmp(5,5);
    wxMemoryDC dummy(dummyBmp);
    assert(dummy.IsOk());
    dummy.SetFont( m_normal_font );
    wxCoord txw = -1, txh = -1, descent = -1, externalLeading = -1;
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_font_height      = txh;
    m_character_width  =  txw;
    assert(m_font_height     > 0);
    assert(m_character_width > 0);
    
    txw = -1; txh = -1; descent = -1; externalLeading = -1;
    dummy.SetFont( m_title_font );
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_title_font_height = txh;
    assert(m_title_font_height > 0);

    txw = -1; txh = -1; descent = -1; externalLeading = -1;
    dummy.SetFont( m_subtitle_font );
    dummy.GetTextExtent(wxT("X"), &txw, &txh, &descent, &externalLeading);
    
    m_subtitle_font_height= txh;
    assert(m_subtitle_font_height > 0);
    
    // --------------------
    
    // those are not known yet
    m_usable_area_height_page_1 = -1;
    m_usable_area_height        = -1;
    
    m_seq  = seq;
    assert(not m_seq->isLayoutCalculated());
    
    m_printer_manager = new QuickPrint( m_seq->getTitle(), this );
    if (not m_printer_manager->performPageSetup())
    {
        std::cerr << "Default page setup failed!\n";
        *success = false;
    }
    else
    {
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
}

// -------------------------------------------------------------------------------------------------------------

wxString AriaPrintable::getPageSetupSummary() const
{
    assert(m_printer_manager.raw_ptr != NULL);
    return m_printer_manager->getPageSetupSummary();
}

// -------------------------------------------------------------------------------------------------------------

wxPrinterError AriaPrintable::print()
{
    assert( MAGIC_NUMBER_OK() );
    
    assert(m_seq->isLayoutCalculated());

#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(m_seq->getTitle());
#endif

    assert(m_printer_manager != NULL);
    m_printer_manager->setPrintableSequence(m_seq);
    
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
    assert(m_current_printable != NULL);
    return m_current_printable;
}

// -------------------------------------------------------------------------------------------------------------

void AriaPrintable::printPage(const int pageNum, wxDC& dc,
                              const int x0, const int y0)
{    
    assert( MAGIC_NUMBER_OK() );

    const int w = m_unit_width;
    const int h = m_unit_height;
    
    assert( w > 0 );
    assert( h > 0 );
    
    const int x1 = x0 + w;
    const int y1 = y0 + h;
    
    LayoutPage& page = m_seq->getPage(pageNum-1);

    const int lineAmount = page.getLineCount();
    // const int lineAmount = page.last_line - page.first_line + 1;

    std::cout << "page has " << lineAmount << " lines" << std::endl;
    
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
    wxString label = m_seq->getTitle();

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
    
    dc.SetFont( m_normal_font );
    
    // get usable area (some area at the top is reserved to the title) 
    const float notation_area_h  = getUsableAreaHeight(pageNum);
    const float notation_area_y0 = y0 + MARGIN_UNDER_PAGE_HEADER +
                                   (pageNum == 1 ? m_title_font_height : m_subtitle_font_height);

    assert(notation_area_h > 0);
    assert(h > 0);
    
    assertExpr(notation_area_y0 + notation_area_h, <=, y1);

    m_seq->printLinesInArea(dc, page, notation_area_y0, notation_area_h, h, x0, x1);
    
}
    
// -------------------------------------------------------------------------------------------------------------
 
