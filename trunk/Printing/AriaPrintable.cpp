#include "AriaCore.h"

#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayoutLine.h"
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
        wxPageSetupDialogData page_setup;
        int orient;
        int pageAmount;
        
        static const int brush_size = 15;
        
        AriaPrintable* printCallBack;
        
    public:
        QuickPrint(PrintableSequence* printableSequence, AriaPrintable* printCallBack) : wxPrintout( printableSequence->getTitle() )
        {
            QuickPrint::printCallBack = printCallBack;
            pageAmount =  printableSequence->getPageAmount();
            orient = wxPORTRAIT;
        }
        
        bool OnPrintPage(int pageNum)
        {
            std::cout << "\n============\nprinting page " << pageNum << "\n==============" << std::endl;
            
            wxDC* ptr = GetDC();
            if (ptr==NULL or !ptr->IsOk()){ std::cerr << "DC is not Ok, interrupting printing" << std::endl; return false; }
            wxDC& dc = *ptr;
            
            
            wxRect bounds = GetLogicalPageRect();
            
            const int x0 = bounds.x;
            const int y0 = bounds.y;
            const int width = bounds.width;
            const int height = bounds.height;
            const int x1 = x0 + width;
            const int y1 = y0 + height;
            
            std::cout << "printable area : (" << x0 << ", " << y0 << ") to (" << x1 << ", " << y1 << ")" << std::endl;
            printCallBack->printPage(pageNum, dc, x0, y0, x1, y1, width, height);
            
            return true;
        }
        
        bool preparePrint(const bool showPageSetupDialog=false)
        {
            wxPrintData printdata;
            printdata.SetPrintMode( wxPRINT_MODE_PRINTER );
            printdata.SetOrientation( orient ); // wxPORTRAIT, wxLANDSCAPE
            printdata.SetNoCopies(1);
            
            page_setup = wxPageSetupDialogData(printdata);
            //page_setup.SetMarginTopLeft(wxPoint(16, 16));
            //page_setup.SetMarginBottomRight(wxPoint(16, 16));
            
            if (showPageSetupDialog)
            {
                // let user change default values if he wishes to
                wxPageSetupDialog dialog( NULL,  &page_setup );
                if (dialog.ShowModal()==wxID_OK)
                {
                    page_setup = dialog.GetPageSetupData();
                    orient = page_setup.GetPrintData().GetOrientation();
                }
                else
                {
                    std::cout << "user canceled at page setup dialog" << std::endl;
                    return false;
                }
            }
            return true;
            
        }
        
        void OnBeginPrinting()
        {
            // set-up coordinate system however we want
            // we'll use it when drawing
            
            int max_x, max_y;
            
            // here i'm using arbitrary an size, use whatever you wish
            if (orient == wxPORTRAIT)
            {
                max_x = 6800;
                max_y = 8800;
            }
            else
            {
                max_x = 8800;
                max_y = 6800;
            }
            
            FitThisSizeToPageMargins(wxSize(max_x, max_y), page_setup);
        }
        
        bool OnBeginDocument(int startPage, int endPage)
        {
            std::cout << "\n\n=============================\n";
            std::cout << "beginning to print document, from page " << startPage << " to " << endPage << std::endl;
            std::cout << "=============================\n\n";
            
            return wxPrintout::OnBeginDocument(startPage, endPage);
        }
        
        void GetPageInfo(int *minPage, int *maxPage, int *pageSelFrom, int *pageSelTo)
        {
            *minPage = 1;
            *maxPage = pageAmount;
            
            *pageSelFrom = 1;
            *pageSelTo = pageAmount;
        }
        bool HasPage(int pageNum)
        {
            if (pageNum >= 1 and pageNum <= pageAmount)
                return true;
            else
                return false;
        }
        
        void OnEndPrinting()
        {
        }
    };
    
}

// -----------------------------------------------------------------------------------------------------------------------

int AriaPrintable::print()
{
    assert( MAGIC_NUMBER_OK() );
    
#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(seq->getTitle());
#endif

    QuickPrint myprint( seq, this );
    wxPrinter printer;

    if (!myprint.preparePrint()) return false;
    //const bool success =
    printer.Print(NULL, &myprint, true /* show dialog */);

#ifdef __WXMAC__
    getMainFrame()->SetTitle(wxT("Aria Maestosa"));
#endif

    return wxPrinter::GetLastError();

    //if (!success) return false;
    //return true;
}

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

namespace AriaMaestosa
{
    AriaPrintable* currentPrintable = NULL;
    
    AriaPrintable* getCurrentPrintable()
    {
        assert(currentPrintable != NULL);
        return currentPrintable;
    }
}

// -----------------------------------------------------------------------------------------------------------------------

AriaPrintable::AriaPrintable(PrintableSequence* seq)
{
    currentPrintable = this;
    AriaPrintable::seq = seq;
    INIT_MAGIC_NUMBER();
}

AriaPrintable::~AriaPrintable()
{
    currentPrintable = NULL;
}

// -----------------------------------------------------------------------------------------------------------------------

void AriaPrintable::printPage(const int pageNum, wxDC& dc,
                              const int x0, const int y0,
                              const int x1, const int y1,
                              const int w, const int h)
{    
    assert( MAGIC_NUMBER_OK() );
    LayoutPage& page = seq->getPage(pageNum-1);

    const int lineAmount = page.getLineCount();
    // const int lineAmount = page.last_line - page.first_line + 1;

    std::cout << "page has " << lineAmount << " lines" << std::endl;
    
    int level_y_amount = 4;
    for(int n=0; n < lineAmount; n++)
    {
        level_y_amount += page.getLine(n).level_height;
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

    /*
     the equivalent of 3 times "text_height" will not be printed with notation.
     --> space for title at the top, and some space under it
     If it's the first page, leave more space because the title there is bigger. FIXME - compute proper size
     */
    const float track_area_height = (float)h - (float)text_height*3.0f + (pageNum == 1 ? 100 : 0);

    seq->printLinesInArea(dc, page, text_height, track_area_height, level_y_amount, h, x0, y0, x1);
    
}
    
// -----------------------------------------------------------------------------------------------------------------------

 
