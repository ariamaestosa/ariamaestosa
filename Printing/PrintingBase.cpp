#include "Printing/PrintingBase.h"
#include "wx/wx.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include <iostream>

namespace AriaMaestosa
{

/*
 * Shows a basic example of how to print stuff in wx.
 * Use the 'preview' feature here, actually printing this will take much of your ink ^^
 */


class QuickPrint : public wxPrintout
{
    wxPageSetupDialogData page_setup;
    int orient;
    int max_x, max_y;
    int pageAmount;
    
    static const int brush_size = 15;
    
    AriaPrintable* printable;
    
public:
    QuickPrint(AriaPrintable* printable) : wxPrintout( printable->getTitle() )
    {
            QuickPrint::printable = printable;
            pageAmount =  printable->getPageAmount();
            orient = printable->portraitOrientation() ? wxPORTRAIT : wxLANDSCAPE;
    }
    
    bool OnPrintPage(int pageNum)
    {
        std::cout << "printing page " << pageNum << std::endl;
        
        wxDC* ptr = GetDC();
        if(ptr==NULL or !ptr->IsOk()){ std::cerr << "DC is not Ok, interrupting printing" << std::endl; return false; }
        wxDC& dc = *ptr;
        
        
        wxRect bounds = GetLogicalPageRect();
        
        const int x0 = bounds.x;
        const int y0 = bounds.y;
        const int width = bounds.width;
        const int height = bounds.height;
        const int x1 = x0 + width;
        const int y1 = y0 + height;
        
        std::cout << "printable area : (" << x0 << ", " << y0 << ") to (" << x1 << ", " << y1 << ")" << std::endl;
        printable->printPage(pageNum, dc, x0, y0, x1, y1, width, height);
        
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
        
        if(showPageSetupDialog)
        {
            // let user change default values if he wishes to
            wxPageSetupDialog dialog( NULL,  &page_setup );
            if(dialog.ShowModal()==wxID_OK)
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
        
        // here i'm using arbitrary an size, use whatever you wish
        if(orient == wxPORTRAIT)
        {
            max_x = 680;
            max_y = 880;
        }
        else 
        {
            max_x = 880;
            max_y = 680;
        }
        
        FitThisSizeToPageMargins(wxSize(max_x, max_y), page_setup);
    }
    
    bool OnBeginDocument(int startPage, int endPage)
    {
        std::cout << "beginning to print document, from page " << startPage << " to " << endPage << std::endl;
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
        if(pageNum >= 1 and pageNum <= pageAmount)
            return true;
        else
            return false;
    }
    
    void OnEndPrinting()
    {
    }
};

bool printResult(AriaPrintable* printable)
{
    
    QuickPrint myprint( printable );
    wxPrinter printer;
    
    if(!myprint.preparePrint()) return false;
    const bool success = printer.Print(NULL, &myprint, true /* show dialog */);
    
    if(!success) return false;
    
    return true;
}

#pragma mark -

// methods to be overriden if needed
AriaPrintable::AriaPrintable()
{
}

AriaPrintable::~AriaPrintable()
{
}
wxString AriaPrintable::getTitle()
{
    return wxT("Aria Maestosa printed score");
}

int AriaPrintable::getPageAmount()
{
    return 1;
}
bool AriaPrintable::portraitOrientation()
{
    return true;
}
void AriaPrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
}


}