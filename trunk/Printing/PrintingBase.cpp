#include "AriaCore.h"

#include "Printing/PrintingBase.h"
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

#if 0
#pragma mark -
#endif

AriaPrintable* currentPrintable = NULL;

AriaPrintable* getCurrentPrintable()
{
    assert(currentPrintable != NULL);
    return currentPrintable;
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
        
        for(int x=x0; x<x1; x += 250)
        {
            dc.DrawLine(x, y0, x, y1);
            dc.DrawText( wxString::Format(wxT("%i"), x), x, y0 - 75 );
        }
        for(int y=y0; y<y1; y += 250)
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
    
    
    /*
     the equivalent of 3 times "text_height" will not be printed with notation.
     --> space for title at the top, and some space under it
     If it's the first page, leave more space because the title there is bigger. FIXME - compute proper size
     */
    const float track_area_height = (float)h - (float)text_height*3.0f + (pageNum == 1 ? 100 : 0);

    // ---- Give each track an area on the page
    this->placeLinesInPage(page, text_height, track_area_height, level_y_amount, h, x0, y0, x1);
    
    // ---- Draw the tracks
    const wxFont regularFont = wxFont(75, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    for(int l=0; l<lineAmount; l++)
    {
        dc.SetFont( regularFont );
        printLine(page.getLine(l), dc);
    }


    

}
// -----------------------------------------------------------------------------------------------------------------------
void AriaPrintable::printLine(LayoutLine& line, wxDC& dc)
{    
    assert( MAGIC_NUMBER_OK() );
    
    const int trackAmount = line.getTrackAmount();
    
    // ---- Draw vertical line to show these lines belong toghether
    const int my0 = line.y0 + line.margin_above;
    const int my1 = line.y0 + (line.y1 - line.y0) - line.margin_below;
    
    if (trackAmount>1)
    {
        dc.SetPen(  wxPen( wxColour(150,150,150), 25 ) );
        dc.DrawLine( line.x0-3, my0, line.x0-3, my1); // vertical line
        dc.DrawLine( line.x0-3, my0, line.x0-3+30, my0-50); // top thingy
        dc.DrawLine( line.x0-3, my1, line.x0-3+30, my1+50); // bottom thingy
        
        dc.DrawLine( line.x1-3, my0, line.x1-3, my1); // right-side line
    }
    
    std::cout << "\n======== Printing Line (contains " << line.layoutElements.size() << " layout elements) from y=" <<
                 my0 << " to " << my1 << " ========" << std::endl;
    
    // ---- Debug guides
    if (PRINT_LAYOUT_HINTS)
    {
        dc.SetPen( wxPen(*wxGREEN, 15) );
        dc.DrawLine(line.x0, my0, line.x1, my0);
        dc.DrawLine(line.x0, my1, line.x1, my1);
        dc.DrawLine(line.x0, my0, line.x0, my1);
        dc.DrawLine(line.x1, my0, line.x1, my1);
    }
    
    // ---- Do the actual track drawing
    for(int n=0; n<trackAmount; n++)
    {
        // skip empty tracks
        if (line.height_percent[n] == 0) continue;
        
        std::cout << "==== Printing track " << n << " ====" << std::endl;
        
        LineTrackRef& sizing = line.getTrackRenderInfo(n);
        std::cout << "Coords : " << sizing.x0 << ", " << sizing.y0 << " to " << sizing.x1 << ", " << sizing.y1 << std::endl;
        
        EditorPrintable* editorPrintable = seq->getEditorPrintable(n);
        editorPrintable->drawLine(n, sizing, line, dc);
    }
}
    

    /**
     * \param text_height       Height of the title header (for page 1), height of the bottom page # text (for other pages)
     * \param level_y_amount    Height of the track in levels
     * \param track_area_height Height of the track in print units
     */
    void AriaPrintable::placeLinesInPage(LayoutPage& page, const int text_height, const float track_area_height, const int level_y_amount,
                                              const int pageHeight, const int x0, const int y0, const int x1)
    {
        assert( MAGIC_NUMBER_OK() );
        std::cout << "\n========\nplaceTracksInPage\n========\n";
        
        // ---- Lay out tracks
        float y_from = y0 + text_height*3;
        
        const int lineAmount = page.getLineCount();
        for(int l=0; l<lineAmount; l++)
        {
            std::cout << "\n====\nLine " << l << "\n====\n";
            
            
            //std::cout << "layoutLines[l].level_height = " << layoutLines[l].level_height << " track_area_height=" << track_area_height
            //          << " total_height=" << total_height << std::endl;
            
            // give a height proportional to its part of the total height
            float height = (track_area_height/level_y_amount) * page.getLine(l).level_height;
            
            float used_height = height;
            
            
            LayoutLine& line = page.getLine(l);
            
            // line too high, will look weird... shrink a bit
            while (used_height/(float)line.level_height > 100)
            {
                used_height *= 0.95;
            }
            
            // shrink total height when track is way too large (if page contains only a few tracks)
            if (height > pageHeight/5 && height > used_height*1.3) height = used_height*1.3;  
            
            
            float used_y_from = y_from;
            
            // center vertically in available space  if more space than needed
            if (used_height < height) used_y_from += (height - used_height)/2;
            
            //std::cout << "```` used_y_from=" << used_y_from << std::endl;
            
            // split margin above and below depending on position within page
            const int line_amount = page.getLineCount();
            const float position = line_amount == 0 ? 0 : float(l) / line_amount;
            int margin_above = 250*position;
            int margin_below = 250*(1-position);
            
            std::cout << "height=" << height << " used_height=" << used_height << " used_y_from=" << used_y_from << " margin_above=" << margin_above << " margin_below=" << margin_below << std::endl;
            
            this->divideLineAmongTracks(line, x0, used_y_from, x1, used_y_from+used_height, margin_below, margin_above);
            
            y_from += height;
            //std::cout << "yfrom is now " << y_from << std::endl;
        }
        
    }
    

    void AriaPrintable::divideLineAmongTracks(LayoutLine& line, const int x0, const int y0, const int x1, const int y1,
                                                   int margin_below, int margin_above)
    {
        assert( MAGIC_NUMBER_OK() );
        
        const int trackAmount = line.getTrackAmount();
        
        std::cout << "Line given coords " << x0 << ", " << y0 << " to " << x1 << ", " << y1 << std::endl;
        std::cout << "==divideLineAmongTracks==\n";
        
        line.x0 = x0;
        line.y0 = y0;
        line.x1 = x1;
        line.y1 = y1;
        line.margin_below = margin_below;
        line.margin_above = margin_above;
        
        // ---- empty space around whole line
        const float height = (float)(y1 - y0);// - ( trackAmount>1 and not last_of_page ? 100 : 0 );
        
        if (height < 0.0001) return; // empty line. TODO : todo - draw empty bars to show there's something?
        
        // make sure margins are within acceptable bounds
        if (margin_below > height/2) margin_below = height/5;
        if (margin_above > height/2) margin_above = height/5;
        
        
        const int my0 = y0 + margin_above;
        
        // ---- Determine tracks positions and sizes
        
        int nonEmptyTrackAmount = 0; // empty tracks must not be counted
        for(int n=0; n<trackAmount; n++)
        {        
            if (line.height_percent[n] > 0) nonEmptyTrackAmount++;
        }
        
        // FIXME : this is layout, should go in PrintLayout.cpp ?
        // space between individual tracks
        const int space_between_tracks = nonEmptyTrackAmount > 1 ? 150 : 0;
        
        float current_y = my0;
        int nonEmptyID = 0;
        for(int n=0; n<trackAmount; n++)
        {        
            EditorPrintable* editorPrintable = seq->getEditorPrintable(n);
            
            // skip empty tracks
            if (line.height_percent[n] == 0) continue;
            
            // determine how much vertical space is allocated for this track
            const float track_height = (height - margin_below - margin_above) * line.height_percent[n]/100.0f;
            std::cout << "track_height=" << track_height << " (margin_below=" << margin_below << " margin_above=" << margin_above <<
            "space_between_tracks=" << space_between_tracks << ")\n";
            
            // margin space above and below each track are given by simple formula 'space_between_tracks*position' where
            // position ranges from 0 to 1. However, this formula doesn't make the space between 2 tracks equal to
            // 'space_between_tracks' in all cases (especially depending on track amount), so the following correction
            // needs to be applied (FIXME : can it be harder to understand what i'm doing here...)
            const float adjustMarginRatio = (float)(nonEmptyTrackAmount-1) / (float)(nonEmptyTrackAmount);
            
            const float position =
            nonEmptyTrackAmount-1 == 0 ? 0.0f : // avoid division by zero
            (float)nonEmptyID / (float)(nonEmptyTrackAmount-1);
            const float space_above_line = space_between_tracks*position*adjustMarginRatio;
            const float space_below_line = space_between_tracks*(1.0-position)*adjustMarginRatio;
            
            editorPrintable->placeTrackAndElementsWithinCoords(n, line, line.getTrackRenderInfo(n),
                                                               x0, current_y + space_above_line,
                                                               x1, current_y + track_height - space_below_line,
                                                               n==0);
            
            std::cout << "%%%% setting track coords " << n  << " : " << x0 << ", " << (current_y + space_above_line) << " to " <<
            x1 << ", "<< (current_y + track_height - space_below_line) << " ( space_above_line=" << space_above_line <<
            " space_below_line=" << space_below_line << " track_height=" << track_height << ")" <<  std::endl;
            
            
            current_y += track_height;
            nonEmptyID++;
        }
        
        
    }
    
}
