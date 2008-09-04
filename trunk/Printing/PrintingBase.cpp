#include "AriaCore.h"

#include "Printing/PrintingBase.h"
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

AriaPrintable* currentPrintable = NULL;

AriaPrintable* getCurrentPrintable()
{
    assert(currentPrintable != NULL);
    return currentPrintable;
}

AriaPrintable::AriaPrintable(Sequence* parent)
{
    sequence = parent;
    currentPrintable = this;
}
AriaPrintable::~AriaPrintable()
{
    currentPrintable = NULL;
}

void AriaPrintable::addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */)
{
    if(mode == GUITAR)
    {
        editorPrintables.push_back(new TablaturePrintable(track));   
    }
    else if(mode == SCORE)
    {
        editorPrintables.push_back(new ScorePrintable(track));   
    }
    else
    {
        std::cerr << "AriaPrintable::addTrack : mode " << mode << " not supported for printing" << std::endl;
        return;
    }
    tracks.push_back(track);
}
void AriaPrintable::calculateLayout(bool checkRepetitions_bool)
{
    calculateLayoutElements(tracks, checkRepetitions_bool, layoutPages, measures);
}
wxString AriaPrintable::getTitle()
{
    wxString song_title = sequence->suggestTitle();
    wxString track_title;
    if(tracks.size()==1) tracks[0].getName();
    
    wxString final_title;
    
    // give song title
    if(song_title.IsSameAs(_("Untitled")))
        final_title = _("Aria Maestosa song");
    else
        final_title = song_title;
    
    // give track name, if any
    if(!track_title.IsSameAs(_("Untitled")) and track_title.Length()>0) final_title += (wxT(", ") + track_title);
    
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

void AriaPrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
    assertExpr(pageNum-1,<,(int)layoutPages.size());
    LayoutPage& page = layoutPages[pageNum-1];

    const int lineAmount = page.layoutLines.size();
    
    int totalTrackAmount = 0;
    for(int l=0; l<lineAmount; l++)
    {
        totalTrackAmount += page.layoutLines[l].getTrackAmount();
    }

    
    /*
     the equivalent of 4 times "text_height" will not be printed with notation.
     first : space for title at the top
     second and third : space below title at top
     fourth : space below the last line
     */
    
    const float track_height = ( (float)h - (float)text_height*4 ) / (float) std::max(totalTrackAmount, maxLinesInPage);
    
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    // draw title
    wxString label = getTitle();
    
    int title_x = x0;
    
    if(pageNum == 1)
    {
        // on page one make title big and centered
        dc.SetFont( wxFont(13,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );
        wxCoord txw, txh, descent, externalLeading;
        dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
        title_x = (x0+x1)/2 - txw/2;
    }
    else
    {
        // on other pages, repeat the title in small font, not centered, with page number
        label += wxT(", page ");
        label << pageNum;
        dc.SetFont( wxFont(9,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    }

    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, title_x, y0 );
    
    // set font we will use and get info about it
    dc.SetFont( wxFont(9,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );

    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);
    
    // ask all EditorPrintables to render their part
    float y_from = y0 + text_height*3;
    for(int l=0; l<lineAmount; l++)
    { 
        float y_to = y_from + page.layoutLines[l].getTrackAmount()*track_height;
        page.layoutLines[l].printYourself(dc, x0, (int)round(y_from), x1, (int)round(y_to));
        y_from = y_to;
    }
    
}


#pragma mark -

// do not call, override in children
EditorPrintable::EditorPrintable(){}
EditorPrintable::~EditorPrintable(){}
void EditorPrintable::drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const bool draw_m_num){}

void EditorPrintable::beginLine(wxDC* dc, LayoutLine* line,  int x0, const int y0, const int x1, const int y1, bool show_measure_number)
{
    EditorPrintable::x0 = x0;
    EditorPrintable::y0 = y0;
    EditorPrintable::x1 = x1;
    EditorPrintable::y1 = y1;
    EditorPrintable::show_measure_number = show_measure_number;
    EditorPrintable::currentLine = line;
    EditorPrintable::dc = dc;
    
    currentLayoutElement = -1;
    layoutElementsAmount = currentLine->layoutElements.size();
    
    // 2 spaces allocated for left area of the tab
    widthOfAChar = (float)(x1 - x0) / (float)(line->charWidth+2);
    
    assertExpr(line->charWidth,>,0);
    assertExpr(widthOfAChar,>,0);
    
}
int EditorPrintable::getCurrentElementXStart()
{
    return x0 + (int)round(xloc*widthOfAChar) - widthOfAChar;
}
int EditorPrintable::getCurrentElementXEnd()
{
    return x0 + (int)round((xloc+currentLine->layoutElements[currentLayoutElement].charWidth)*widthOfAChar);
}
LayoutElement* EditorPrintable::getNextElement()
{
    currentLayoutElement ++;
    
    if(!(currentLayoutElement<layoutElementsAmount)) return NULL;
    
    std::vector<LayoutElement>& layoutElements = currentLine->layoutElements;
    
    if(currentLayoutElement == 0) xloc = 2;
    else if(currentLayoutElement > 0) xloc += layoutElements[currentLayoutElement-1].charWidth;
    
    const int elem_x_start = getCurrentElementXStart();
    currentLine->layoutElements[currentLayoutElement].x = elem_x_start;
    currentLine->layoutElements[currentLayoutElement].x2 =  getCurrentElementXEnd();

    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 2 ) );
    dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
    
    dc->SetTextForeground( wxColour(0,0,255) );
    
    // ****** empty measure
    if(layoutElements[currentLayoutElement].type == EMPTY_MEASURE)
    {
    } 
    // ****** repetitions
    else if(layoutElements[currentLayoutElement].type == SINGLE_REPEATED_MEASURE or layoutElements[currentLayoutElement].type == REPEATED_RIFF)
    {
        // FIXME - why do I cut apart the measure and not the layout element?
        /*
         if(measures[layoutElements[n].measure].cutApart)
         {
             // TODO...
             //dc.SetPen(  wxPen( wxColour(0,0,0), 4 ) );
             //dc.DrawLine( elem_x_start, y0, elem_x_start, y1);
         }
         */
        
        wxString message;
        if(layoutElements[currentLayoutElement].type == SINGLE_REPEATED_MEASURE)
        {
            message = to_wxString(currentLine->getMeasureForElement(currentLayoutElement).firstSimilarMeasure+1);
        }
        else if(layoutElements[currentLayoutElement].type == REPEATED_RIFF)
        {
            message =	to_wxString(layoutElements[currentLayoutElement].firstMeasureToRepeat+1) +
            wxT(" - ") + 
            to_wxString(layoutElements[currentLayoutElement].lastMeasureToRepeat+1);
        }
        
        dc->DrawText( message, elem_x_start + widthOfAChar/2, (y0+y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** play again
    else if(layoutElements[currentLayoutElement].type == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << layoutElements[currentLayoutElement].amountOfTimes;
        dc->DrawText( label, elem_x_start + widthOfAChar/2, (y0+y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** normal measure
    else if(layoutElements[currentLayoutElement].type == SINGLE_MEASURE)
    {  
        // draw measure ID
        if(show_measure_number)
        {
            wxString measureLabel;
            measureLabel << (currentLine->getMeasureForElement(currentLayoutElement).id+1);
            dc->DrawText( measureLabel, elem_x_start - widthOfAChar/4, y0 - getCurrentPrintable()->text_height*1.4 );
        }
        
        dc->SetTextForeground( wxColour(0,0,0) );
    }
    
    return &layoutElements[currentLayoutElement];
}


int EditorPrintable::getNotePrintX(int noteID)
{
    return tickToX( currentLine->getTrack()->getNoteStartInMidiTicks(noteID) );
    /*
    const int elem_x_start = getCurrentElementXStart();
    const int elem_x_end = getCurrentElementXEnd();
    const int elem_w = elem_x_end - elem_x_start;
    
    const int firstTick = currentLine->getMeasureForElement(currentLayoutElement).firstTick;
    const int lastTick = currentLine->getMeasureForElement(currentLayoutElement).lastTick;
    const int tick = currentLine->getTrack()->getNoteStartInMidiTicks(noteID);
    
    const float nratio = ((float)(tick - firstTick) / (float)(lastTick - firstTick));
    
    return (int)round(nratio * (elem_w-widthOfAChar*1.5) + elem_x_start);
     */
}
int EditorPrintable::tickToX(const int tick)
{
    
    for(int n=0; n<layoutElementsAmount; n++)
    {
        const int firstTick = currentLine->getMeasureForElement(n).firstTick;
        const int lastTick = currentLine->getMeasureForElement(n).lastTick;
        
        if(tick >= firstTick and tick < lastTick)
        {
            
            /*
             * note position ranges from 0 (at the very beginning of the layout element)
             * to 1 (at the very end of the layout element)
             */
            const int elem_x_start = currentLine->layoutElements[n].x;
            const int elem_x_end = currentLine->layoutElements[n].x2;
            //std::cout << elem_x_start << ", " << elem_x_end << std::endl;
            const int elem_w = elem_x_end - elem_x_start;
            const float nratio = ((float)(tick - firstTick) / (float)(lastTick - firstTick));
            
            std::cout << "nratio = " << nratio << std::endl;
            return (int)round(nratio * (elem_w-widthOfAChar*1.5) + elem_x_start);
        }
    }
    assert(false);
    return 0;
}

}