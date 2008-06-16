#include "AriaCore.h"
#include "Printing/PrintingBase.h"
#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

#include "wx/wx.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include <iostream>

namespace AriaMaestosa
{


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
#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(printable->getTitle());
#endif
    
    QuickPrint myprint( printable );
    wxPrinter printer;
    
    if(!myprint.preparePrint()) return false;
    const bool success = printer.Print(NULL, &myprint, true /* show dialog */);

#ifdef __WXMAC__
    getMainFrame()->SetTitle(wxT("Aria Maestosa"));
#endif
    
    if(!success) return false;
    
    return true;
}

#pragma mark -

// methods to be overriden if needed
AriaPrintable::AriaPrintable(Track* track, bool checkRepetitions_bool)
{
    AriaPrintable::parent = track;
    
    ptr_vector<Track> tracks;
    tracks.push_back(track);
    calculateLayoutElements(tracks, checkRepetitions_bool, layoutPages, measures);
    
    tracks.clearWithoutDeleting();
    
    std::cout << "\nContains " << layoutPages.size() << " pages\n" << std::endl;
}

AriaPrintable::~AriaPrintable()
{
}
wxString AriaPrintable::getTitle()
{
    wxString song_title = parent->sequence->suggestTitle();
    wxString track_title = parent->getName();
    
    wxString final_title;
    
    // give song title
    if(song_title.IsSameAs(_("Untitled")))
        final_title = _("Aria Maestosa song");
    else
        final_title = song_title;
    
    // give track name, if any
    if(!track_title.IsSameAs(_("Untitled"))) final_title += (wxT(", ") + track_title);
    
    std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;    
}

int AriaPrintable::getPageAmount()
{
    return layoutPages.size();
}
bool AriaPrintable::portraitOrientation()
{
    // FIXME - any use??
    return true;
}

// FIXME - make it support multiple tracks
void AriaPrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    dc.SetFont( wxFont(11,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    
    wxString label( getTitle() + wxT(", page ") );
    label << pageNum;
    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, x0, y0 );
    
    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);
    
    // -------------------- generate the tablature  -------------------- 
    assertExpr(pageNum-1,<,(int)layoutPages.size());
    const int lineAmount = layoutPages[pageNum-1].layoutLines.size();
    
    /*
     the equivalent of 4 times "text_height" will not be printed with notation.
     first : space for title at the top
     second and third : space below title at top
     fourth : space below the last line
     */
    
    const float line_height = ((float)h - (float)text_height*4) / (float)maxLinesInPage;
    
    const int notation_area_origin_y = y0 + text_height*3;
    for(int l=0; l<lineAmount; l++)
    { 
        const float line_y_from = notation_area_origin_y + line_height*l;
        const float line_y_to = notation_area_origin_y + line_height*(l+0.6f);
        
        drawLine(layoutPages[pageNum-1].layoutLines[l], dc, x0, (int)round(line_y_from), x1, (int)round(line_y_to));
    }
    
}

void AriaPrintable::drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1)
{
    // should not be called, override in children
}

}