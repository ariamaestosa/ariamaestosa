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
#include "wx/print.h"
#include "wx/printdlg.h"
#include <iostream>

using namespace AriaMaestosa;

namespace AriaMaestosa
{
    
    class QuickPrint : public wxPrintout
    {
        AriaPrintable*        m_print_callback;
        wxPageSetupDialogData m_page_setup;
        wxPaperSize           m_paper_id;
        int                   m_orient;
        int                   m_page_amount;
        
        static const int      m_brush_size = 15;
        
        
    public:
 
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
          * @postcondition sets m_unit_width and m_unit_height in AriaPrintable
          */
        bool performPageSetup(const bool showPageSetupDialog=false)
        {
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
                    m_orient     = m_page_setup.GetPrintData().GetOrientation();
                }
                else
                {
                    std::cout << "user canceled at page setup dialog" << std::endl;
                    return false;
                }
            }
            
            assert(m_page_setup.GetPaperId() != wxPAPER_NONE);
            
            // ---- set-up coordinate system however we want
            // we'll use it when drawing
            wxSize paperSize = m_page_setup.GetPaperSize();
            const int large_side = std::max(paperSize.GetWidth(), paperSize.GetHeight());
            const int small_side = std::min(paperSize.GetWidth(), paperSize.GetHeight());
            
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
            
            return true;
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
            const int width  = bounds.width;
            const int height = bounds.height;
            
            std::cout << "printable area : (" << x0 << ", " << y0 << ") to ("
            << (x0 + width) << ", " << (y0 + height) << ")" << std::endl;
            assert( width  > 0 );
            assert( height > 0 );
            
            m_print_callback->printPage(pageNum, dc, x0, y0, width, height);
            
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


AriaPrintable::AriaPrintable(PrintableSequence* seq, bool* success)
{
    INIT_MAGIC_NUMBER();
    assert(m_current_printable == NULL);
    
    m_current_printable = this;
    m_unit_width        = -1;
    m_unit_height       = -1;
    
    AriaPrintable::seq  = seq;
    assert(not seq->isLayoutCalculated());
    
    m_printer_manager = new QuickPrint( seq->getTitle(), this );
    if (not m_printer_manager->performPageSetup())
    {
        std::cerr << "Preparing the QUickPrint object failed!\n";
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
    delete m_printer_manager;
}

// -------------------------------------------------------------------------------------------------------------

wxPrinterError AriaPrintable::print()
{
    assert( MAGIC_NUMBER_OK() );
    
    assert(seq->isLayoutCalculated());

#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(seq->getTitle());
#endif

    assert(m_printer_manager != NULL);
    m_printer_manager->setPrintableSequence(seq);
    
    wxPrinter printer;
    if (not printer.Print(NULL, m_printer_manager, true /* show dialog */))
    {
        std::cerr << "The Print method returned false\n";
        return wxPRINTER_ERROR;
    }

#ifdef __WXMAC__
    getMainFrame()->SetTitle(wxT("Aria Maestosa"));
#endif

    return wxPrinter::GetLastError();
}

// -------------------------------------------------------------------------------------------------------------

AriaPrintable* AriaPrintable::getCurrentPrintable()
{
    assert(m_current_printable != NULL);
    return m_current_printable;
}

// -------------------------------------------------------------------------------------------------------------

void AriaPrintable::printPage(const int pageNum, wxDC& dc,
                              const int x0, const int y0,
                              const int w, const int h)
{    
    assert( MAGIC_NUMBER_OK() );
    assert( w > 0 );
    assert( h > 0 );
    
    const int x1 = x0 + w;
    const int y1 = y0 + h;
    
    LayoutPage& page = seq->getPage(pageNum-1);

    const int lineAmount = page.getLineCount();
    // const int lineAmount = page.last_line - page.first_line + 1;

    std::cout << "page has " << lineAmount << " lines" << std::endl;
    
    // FIXME: what is this 4 ?
    int level_y_amount = 4;
    for(int n=0; n < lineAmount; n++)
    {
        level_y_amount += page.getLine(n).m_level_height;
    }

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
    wxString label = seq->getTitle();

    int title_x = x0;

    if (pageNum == 1)
    {
        // on page one make title big and centered
        dc.SetFont( wxFont(130,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );
        wxCoord txw, txh, descent, externalLeading;
        dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
        title_x = (x0+x1)/2 - txw/2;
    }
    else
    {
        // on other pages, repeat the title in small font, not centered, with page number
        label += wxT(", page ");
        label << pageNum;
        dc.SetFont( wxFont(90,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    }
    
    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, title_x, y0 );
    
    // ---- Set font we will use and get info about it
    dc.SetFont( wxFont(75,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    
    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);
    
    character_width =  dc.GetTextExtent(wxT("X")).GetWidth();


    // the equivalent of 3 times "text_height" will not be printed with notation.
    // --> space for title at the top, and some space under it
    // FIXME: compute proper size so that big title room is not wasted on other pages.
    //        Also, for title, don't rely on * 3 the small font, calculate with  the actual font.
    const float notation_area_h  = (float)h - (float)text_height*3.0f;
    const float notation_area_y0 = y0 + (float)text_height*3.0f;
    
    assert(notation_area_h > 0);
    assert(h > 0);
    
    seq->printLinesInArea(dc, page, notation_area_y0, notation_area_h, level_y_amount, h, x0, x1);
    
}
    
// -------------------------------------------------------------------------------------------------------------
 
