
#include "Printing/PrintableSequence.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "Printing/TabPrint.h"
#include "Printing/ScorePrint.h"

#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa
{

PrintableSequence::PrintableSequence(Sequence* parent)
{
    sequence = parent;
    track_amount = 0;
    is_guitar_editor_used = false;
    is_score_editor_used = false;
    max_signs_in_keysig = 0;
}

// -----------------------------------------------------------------------------------------------------------------------
bool PrintableSequence::addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */)
{
    if (mode == GUITAR)
    {
        editorPrintables.push_back(new TablaturePrintable(track));
        is_guitar_editor_used = true;
    }
    else if (mode == SCORE)
    {
        editorPrintables.push_back(new ScorePrintable());
        is_score_editor_used = true;
        
        max_signs_in_keysig = std::max( max_signs_in_keysig,
                                       std::max(track->graphics->getCurrentEditor()->getKeySharpsAmount(),
                                                track->graphics->getCurrentEditor()->getKeyFlatsAmount()) );
    }
    else
    {
        std::cerr << "PrintableSequence::addTrack : mode " << mode << " not supported for printing" << std::endl;
        return false;
    }
    tracks.push_back(track);
    track_amount = tracks.size();
    return true;
}

// -----------------------------------------------------------------------------------------------------------------------
void PrintableSequence::calculateLayout(bool checkRepetitions)
{
    m_abstract_layout_manager = new PrintLayoutAbstract(this, layoutPages /* out */);
    m_abstract_layout_manager->generateMeasures(tracks);
    m_abstract_layout_manager->calculateLayoutElements(tracks, checkRepetitions);
    
    // prepare it for when we're ready to print
    m_numeric_layout_manager = new PrintLayoutNumeric(this);
}

// -----------------------------------------------------------------------------------------------------------------------

EditorPrintable* PrintableSequence::getEditorPrintable(const int trackID)
{
    return editorPrintables.get(trackID);
}

// -----------------------------------------------------------------------------------------------------------------------

void PrintableSequence::printLinesInArea(wxDC& dc, LayoutPage& page, 
                                         const float notation_area_y0, const float notation_area_h,
                                         const int level_y_amount, const int pageHeight,
                                         const int x0, const int x1)
{
    // ---- Give each track an area on the page
    m_numeric_layout_manager->placeLinesInPage(page, notation_area_y0, notation_area_h,
                                               level_y_amount, pageHeight, x0, x1);
    
    // ---- Draw the tracks
    const wxFont regularFont = wxFont(75, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    
    const int lineAmount = page.getLineCount();
    for (int l=0; l<lineAmount; l++)
    {
        dc.SetFont( regularFont );
        this->printLine(page.getLine(l), dc);
    }
}

// -----------------------------------------------------------------------------------------------------------------------

void PrintableSequence::printLine(LayoutLine& line, wxDC& dc)
{        
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
    
    std::cout << "\n======== Printing Line (contains " << line.getLayoutElementCount() << " layout elements) from y=" <<
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
    for (int n=0; n<trackAmount; n++)
    {
        // skip empty tracks
        if (line.height_percent[n] == 0) continue;
        
        std::cout << "==== Printing track " << n << " ====" << std::endl;
        
        LineTrackRef& sizing = line.getLineTrackRef(n);
        std::cout << "Coords : " << sizing.x0 << ", " << sizing.y0 << " to " << sizing.x1 << ", " << sizing.y1 << std::endl;
        
        EditorPrintable* editorPrintable = this->getEditorPrintable(n);
        editorPrintable->drawLine(n, sizing, line, dc);
    }
}
    
// -----------------------------------------------------------------------------------------------------------------------

wxString PrintableSequence::getTitle()
{
    wxString song_title = sequence->suggestTitle();
    wxString track_title;
    if (tracks.size()==1) tracks[0].getName();
    
    wxString final_title;
    
    // give song title
    if (song_title.IsSameAs(_("Untitled")))
        final_title = _("Aria Maestosa song");
    else
        final_title = song_title;
    
    // give track name, if any
    if (!track_title.IsSameAs(_("Untitled")) and track_title.Length()>0) final_title += (wxT(", ") + track_title);
    
    std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;
}

// -----------------------------------------------------------------------------------------------------------------------
int PrintableSequence::getPageAmount()
{
    return layoutPages.size();
}
// -----------------------------------------------------------------------------------------------------------------------
LayoutPage& PrintableSequence::getPage(const int id)
{
    assertExpr(id, >=, 0);
    assertExpr(id, <, (int)layoutPages.size());
    
    LayoutPage& out = layoutPages[id];
    
    assert( MAGIC_NUMBER_OK_FOR(&out) );
    
    return out;
}
    
}
