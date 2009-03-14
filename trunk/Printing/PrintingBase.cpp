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
        orient = wxPORTRAIT;
    }

    bool OnPrintPage(int pageNum)
    {
        std::cout << "\n\n*****\nprinting page " << pageNum << std::endl;

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

int printResult(AriaPrintable* printable)
{
#ifdef __WXMAC__
    // change window title so any generated PDF is given the right name
    getMainFrame()->SetTitle(printable->getTitle());
#endif

    QuickPrint myprint( printable );
    wxPrinter printer;

    if(!myprint.preparePrint()) return false;
    //const bool success =
    printer.Print(NULL, &myprint, true /* show dialog */);

#ifdef __WXMAC__
    getMainFrame()->SetTitle(wxT("Aria Maestosa"));
#endif

    return wxPrinter::GetLastError();

    //if(!success) return false;
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

AriaPrintable::AriaPrintable(Sequence* parent)
{
    sequence = parent;
    currentPrintable = this;
    is_guitar_editor_used = false;
    is_score_editor_used = false;
    track_amount = 0;
    max_signs_in_keysig = 0;
}
AriaPrintable::~AriaPrintable()
{
    currentPrintable = NULL;
}

bool AriaPrintable::addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */)
{
    if(mode == GUITAR)
    {
        editorPrintables.push_back(new TablaturePrintable(track));
        is_guitar_editor_used = true;
    }
    else if(mode == SCORE)
    {
        editorPrintables.push_back(new ScorePrintable(track));
        is_score_editor_used = true;
        
        max_signs_in_keysig = std::max( max_signs_in_keysig,
                                       std::max(track->graphics->getCurrentEditor()->getKeySharpsAmount(),
                                                track->graphics->getCurrentEditor()->getKeyFlatsAmount()) );
    }
    else
    {
        std::cerr << "AriaPrintable::addTrack : mode " << mode << " not supported for printing" << std::endl;
        return false;
    }
    tracks.push_back(track);
    track_amount = tracks.size();
    return true;
}
void AriaPrintable::calculateLayout(bool checkRepetitions_bool)
{
    PrintLayoutManager layout(this, layoutLines /* out */, layoutPages /* out */, measures /* out */);
    layout.calculateLayoutElements(tracks, checkRepetitions_bool);
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

void AriaPrintable::printPage(const int pageNum, wxDC& dc,
                              const int x0, const int y0,
                              const int x1, const int y1,
                              const int w, const int h)
{
    assertExpr(pageNum-1,<,(int)layoutPages.size());
    LayoutPage& page = layoutPages[pageNum-1];

    const int lineAmount = page.last_line - page.first_line + 1;

    std::cout << "printing page " << pageNum << ", which has " << lineAmount << " lines" << std::endl;
    /*
    int totalTrackAmount = 0;
    for(int l=0; l<lineAmount; l++)
    {
        totalTrackAmount += page.layoutLines[l].getTrackAmount();
    }
*/
    int total_height = 4;
    for(int n=page.first_line; n <= page.last_line; n++)
    {
        total_height += layoutLines[n].level_height;
    }

    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    // draw title
    wxString label = getTitle();

    int title_x = x0;

    if(pageNum == 1)
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

    // set font we will use and get info about it
    dc.SetFont( wxFont(60,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );

    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);

    /*
     the equivalent of 4 times "text_height" will not be printed with notation.
     first : space for title at the top
     second and third : space below title at top
     fourth : space below the last line
     */
    //const float track_height = ( (float)h - (float)text_height*4 ) / (float) std::max(totalTrackAmount, maxLinesInPage);

    const float track_area_height = (float)h - (float)text_height*4.0f;

    std::cout << "printing lines from " << page.first_line << " to " << page.last_line << std::endl;

    // ask all EditorPrintables to render their part
    float y_from = y0 + text_height*3;
    for(int l=page.first_line; l<=page.last_line; l++)
    {
        // give a height proportional to its part of the total height
        float height = layoutLines[l].level_height*track_area_height/total_height;

        // track too high, will look weird... shrink a bit
        while(height/(float)layoutLines[l].level_height > 100) height *= 0.95;

        float y_to = y_from + height;
        layoutLines[l].printYourself(dc, x0, (int)round(y_from), x1, (int)round(y_to));
        y_from = y_to;
    }

}


#if 0
#pragma mark -
#endif

// do not call, override in children
EditorPrintable::EditorPrintable(){}
EditorPrintable::~EditorPrintable(){}

void EditorPrintable::beginLine(wxDC* dc, LayoutLine* line,  int x0, const int y0, const int x1, const int y1, bool show_measure_number)
{
    EditorPrintable::x0 = x0;
    EditorPrintable::y0 = y0;
    EditorPrintable::x1 = x1;
    EditorPrintable::y1 = y1;
    EditorPrintable::show_measure_number = show_measure_number;
    EditorPrintable::currentLine = line;
    EditorPrintable::dc = dc;
    
    // 2 spaces allocated for left area of the line
    pixel_width_of_an_unit = (float)(x1 - x0) / (float)(line->width_in_units+2);

    layoutElementsAmount = currentLine->layoutElements.size();

    // init layout elements' locations
    for(currentLayoutElement=0; currentLayoutElement<layoutElementsAmount; currentLayoutElement++)
    {
        //if(currentLayoutElement == 0) xloc = 2;
        //else if(currentLayoutElement > 0) xloc += currentLine->layoutElements[currentLayoutElement-1].width_in_units;

        if(currentLayoutElement == 0) xloc = 1;
        else if(currentLayoutElement > 0) xloc += currentLine->layoutElements[currentLayoutElement-1].width_in_units;
        
        currentLine->layoutElements[currentLayoutElement].x  = getCurrentElementXStart();
        if(currentLayoutElement > 0)
            currentLine->layoutElements[currentLayoutElement-1].x2 =  currentLine->layoutElements[currentLayoutElement].x;
        //std::cout << "layout element " << currentLayoutElement <<  " from " <<
       //     currentLine->layoutElements[currentLayoutElement].x << " to " << currentLine->layoutElements[currentLayoutElement].x2 << std::endl;
    }
    // for last
    currentLine->layoutElements[currentLine->layoutElements.size()-1].x2 = x1; // FIXME - fix naming conventions... in track it's x1, in element it's x2
    
    xloc = -1;
    currentLayoutElement = -1;

    assertExpr(line->width_in_units,>,0);
    assertExpr(pixel_width_of_an_unit,>,0);

}
int EditorPrintable::getCurrentElementXStart()
{
    return x0 + (int)round(xloc*pixel_width_of_an_unit) - pixel_width_of_an_unit;
}

LayoutElement* EditorPrintable::getElementForMeasure(const int measureID)
{
    assert(currentLine != NULL);
    std::vector<LayoutElement>& layoutElements = currentLine->layoutElements;
    const int amount = layoutElements.size();
    for(int n=0; n<amount; n++)
    {
        if(layoutElements[n].measure == measureID) return &layoutElements[n];
    }
    return NULL;
}
    
void EditorPrintable::drawVerticalDivider(LayoutElement* el, const int y0, const int y1)
{
    if(el->type == TIME_SIGNATURE) return;
    
    const int elem_x_start = el->x; // currentLine->layoutElements[elemenentID].x

    // draw vertical line that starts measure
    dc->SetPen(  wxPen( wxColour(0,0,0), 10 ) );
    dc->DrawLine( elem_x_start, y0, elem_x_start, y1);
}
void EditorPrintable::renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1)
{
    wxString num   = wxString::Format( wxT("%i"), el->num   );
    wxString denom = wxString::Format( wxT("%i"), el->denom );
    
    wxFont oldfont = dc->GetFont();
    dc->SetFont( wxFont(100,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );
    dc->SetTextForeground( wxColour(0,0,0) );
    
    wxSize text_size = dc->GetTextExtent(denom);
    const int text_x = el->x2 - text_size.GetWidth() - 20;
    
    dc->DrawText(num,   text_x, y0 + 10);
    dc->DrawText(denom, text_x, y0 + (y1 - y0)/2 + 10  );
    
    dc->SetFont(oldfont);    
}
LayoutElement* EditorPrintable::continueWithNextElement()
{
    currentLayoutElement ++;

    if(!(currentLayoutElement<layoutElementsAmount)) return NULL;

    std::vector<LayoutElement>& layoutElements = currentLine->layoutElements;

    const int elem_x_start = currentLine->layoutElements[currentLayoutElement].x;

    dc->SetTextForeground( wxColour(0,0,255) );

    // ****** empty measure
    if(layoutElements[currentLayoutElement].type == EMPTY_MEASURE)
    {
    }
    // ****** time signature change
    else if(layoutElements[currentLayoutElement].type == TIME_SIGNATURE)
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
            message =    to_wxString(layoutElements[currentLayoutElement].firstMeasureToRepeat+1) +
            wxT(" - ") +
            to_wxString(layoutElements[currentLayoutElement].lastMeasureToRepeat+1);
        }

        dc->DrawText( message, elem_x_start + pixel_width_of_an_unit/2, (y0+y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** play again
    else if(layoutElements[currentLayoutElement].type == PLAY_MANY_TIMES)
    {
        wxString label(wxT("X"));
        label << layoutElements[currentLayoutElement].amountOfTimes;
        dc->DrawText( label, elem_x_start + pixel_width_of_an_unit/2, (y0+y1)/2 - getCurrentPrintable()->text_height_half );
    }
    // ****** normal measure
    else if(layoutElements[currentLayoutElement].type == SINGLE_MEASURE)
    {
        // draw measure ID
        if(show_measure_number)
        {
            const int meas_id = currentLine->getMeasureForElement(currentLayoutElement).id+1;

            wxString measureLabel;
            measureLabel << meas_id;

            dc->DrawText( measureLabel,
                          elem_x_start - ( meas_id > 9 ? pixel_width_of_an_unit/4 : pixel_width_of_an_unit/5 ),
                          y0 - getCurrentPrintable()->text_height*1.4 );
        }

        dc->SetTextForeground( wxColour(0,0,0) );
    }

    return &layoutElements[currentLayoutElement];
}


int EditorPrintable::getNotePrintX(int noteID)
{
    return tickToX( currentLine->getTrack()->getNoteStartInMidiTicks(noteID) );
}
int EditorPrintable::tickToX(const int tick)
{
    for(int n=0; n<layoutElementsAmount; n++)
    {
        MeasureToExport& meas = currentLine->getMeasureForElement(n);
        if(meas.id == -1) continue; // nullMeasure, ignore
        const int firstTick = meas.firstTick;
        const int lastTick  = meas.lastTick;

        if(tick >= firstTick and tick < lastTick)
        {
            /*
             * note position ranges from 0 (at the very beginning of the layout element)
             * to 1 (at the very end of the layout element)
             */
            const int elem_x_start = currentLine->layoutElements[n].x;
            const int elem_x_end = currentLine->layoutElements[n].x2;
            const int elem_w = elem_x_end - elem_x_start;
            const float nratio = ((float)(tick - firstTick) / (float)(lastTick - firstTick));

            return (int)round(nratio * (elem_w-pixel_width_of_an_unit*1.5) + elem_x_start);
        }

        // given tick is not in a visible measure
        if(tick < firstTick) return -1;
        
        /* the tick we were given is not on the current line, but on the next.
         * this probably means there is a tie from a note on one line to a note
         * on another line. Return a X at the very right of the page.
         * FIXME - it's not necessarly a tie
         * FIXME - ties aand line warping need better handling
         */
        if(n==layoutElementsAmount-1 and tick >= lastTick)
        {
            return currentLine->layoutElements[n].x2 + 10;
        }
    }
    return -1;
    //return currentLine->layoutElements[layoutElementsAmount-1].x2 + 10;
}

}
