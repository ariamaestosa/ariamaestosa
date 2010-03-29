
#include "Printing/PrintableSequence.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/PrintLayoutNumeric.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "Printing/TabPrint.h"
#include "Printing/ScorePrint.h"

#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

using namespace AriaMaestosa;

#define DEBUG_DRAW 0

PrintableSequence::PrintableSequence(Sequence* parent)
{
    m_sequence = parent;
    //track_amount = 0;
    m_is_guitar_editor_used = false;
    m_is_score_editor_used = false;
    m_max_signs_in_keysig = 0;
    m_layout_calculated = false;
    m_numeric_layout_manager = NULL;
}

// -----------------------------------------------------------------------------------------------------------------

bool PrintableSequence::addTrack(Track* track, EditorType mode)
{
    ASSERT(track->sequence == m_sequence);
    ASSERT(not m_layout_calculated);
    
    if (mode == GUITAR)
    {
        m_editor_printables.push_back(new TablaturePrintable(track));
        m_is_guitar_editor_used = true;
    }
    else if (mode == SCORE)
    {
        m_editor_printables.push_back(new ScorePrintable());
        m_is_score_editor_used = true;
        
        m_max_signs_in_keysig = std::max( m_max_signs_in_keysig,
                                       std::max(track->getKeySharpsAmount(),
                                                track->getKeyFlatsAmount()) );
    }
    else
    {
        std::cerr << "PrintableSequence::addTrack : mode " << mode << " not supported for printing" << std::endl;
        return false;
    }
    m_tracks.push_back(track);
    return true;
}

// -----------------------------------------------------------------------------------------------------------------

void PrintableSequence::calculateLayout(bool checkRepetitions)
{
    ASSERT( MAGIC_NUMBER_OK_FOR(&m_tracks) );
    
    m_abstract_layout_manager = new PrintLayoutAbstract(this);
    m_abstract_layout_manager->addLayoutInformation(m_tracks, layoutPages /* out */, checkRepetitions);
    
    // prepare it for when we're ready to print
    m_numeric_layout_manager = new PrintLayoutNumeric();
    
    m_layout_calculated = true;
}

// -----------------------------------------------------------------------------------------------------------------

EditorPrintable* PrintableSequence::getEditorPrintable(const int trackID)
{
    return m_editor_printables.get(trackID);
}

// ------------------------------------------------------------------------------------------------------------------

void PrintableSequence::printLinesInArea(wxDC& dc, LayoutPage& page, 
                                         const float notation_area_y0, const float notation_area_h,
                                         const int pageHeight, const int x0, const int x1)
{
    ASSERT(m_layout_calculated);
    ASSERT(m_numeric_layout_manager != NULL);
    
    ASSERT(notation_area_h > 0);
    ASSERT(pageHeight > 0);
    //ASSERT_E(notation_area_y0 + notation_area_h, <=, pageHeight);

    // ---- Give each track an area on the page
    m_numeric_layout_manager->placeLinesInPage(page, notation_area_y0, notation_area_h,
                                               pageHeight, x0, x1);
    
    // ---- Draw the tracks
    const wxFont regularFont = wxFont(75, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    
    const int lineAmount = page.getLineCount();
    for (int l=0; l<lineAmount; l++)
    {
        dc.SetFont( regularFont );
        this->printLine(page.getLine(l), dc);
    }
}

// -----------------------------------------------------------------------------------------------------------------

void PrintableSequence::printLine(LayoutLine& line, wxDC& dc)
{        
    const int trackAmount = line.getTrackAmount();
    
    const LineCoords* lineCoords = line.m_line_coords;
    ASSERT(lineCoords != NULL);
    
#if DEBUG_DRAW
    dc.SetPen(  wxPen( wxColour(255,0,0), 25 ) );
    dc.DrawLine( lineCoords->x0, lineCoords->y0, lineCoords->x0, lineCoords->y1);
    dc.DrawLine( lineCoords->x1, lineCoords->y0, lineCoords->x1, lineCoords->y1);
    dc.DrawLine( lineCoords->x0, lineCoords->y0, lineCoords->x1, lineCoords->y0);
    dc.DrawLine( lineCoords->x0, lineCoords->y1, lineCoords->x1, lineCoords->y1);
#endif
    
    // ---- Draw vertical line to show these lines belong toghether
    const int my0 = lineCoords->y0 + lineCoords->margin_above;
    const int my1 = lineCoords->y0 + (lineCoords->y1 - lineCoords->y0) - lineCoords->margin_below;
    
    if (trackAmount>1)
    {
        dc.SetPen(  wxPen( wxColour(150,150,150), 25 ) );
        dc.DrawLine( lineCoords->x0-3, my0, lineCoords->x0-3, my1); // vertical line
        dc.DrawLine( lineCoords->x0-3, my0, lineCoords->x0-3+30, my0-50); // top thingy
        dc.DrawLine( lineCoords->x0-3, my1, lineCoords->x0-3+30, my1+50); // bottom thingy
        
        dc.DrawLine( lineCoords->x1-3, my0, lineCoords->x1-3, my1); // right-side line
    }
    
    std::cout << "\n======== Printing Line (contains " << line.getLayoutElementCount()
              << " layout elements) from y=" << my0 << " to " << my1 << " ========" << std::endl;
    
    // ---- Debug guides
    if (PRINT_LAYOUT_HINTS)
    {
        dc.SetPen( wxPen(*wxGREEN, 15) );
        dc.DrawLine(lineCoords->x0, my0, lineCoords->x1, my0);
        dc.DrawLine(lineCoords->x0, my1, lineCoords->x1, my1);
        dc.DrawLine(lineCoords->x0, my0, lineCoords->x0, my1);
        dc.DrawLine(lineCoords->x1, my0, lineCoords->x1, my1);
    }
    
    // ---- Do the actual track drawing
    bool first = true;
    for (int n=0; n<trackAmount; n++)
    {
        // skip empty tracks
        if (line.getLineTrackRef(n).empty()) continue;
        
        std::cout << "==== Printing track " << n << " ====" << std::endl;
        
        const LineTrackRef& sizing = line.getLineTrackRef(n);
        const TrackCoords* trackCoords = sizing.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        
        std::cout << "Coords : " << trackCoords->x0 << ", " << trackCoords->y0 << " to "
                  << trackCoords->x1 << ", " << trackCoords->y1 << std::endl;
        
#if DEBUG_DRAW
        dc.SetPen(  wxPen( wxColour(0,255,0), 12 ) );
        dc.DrawLine( trackCoords->x0, trackCoords->y0, trackCoords->x0, trackCoords->y1);
        dc.DrawLine( trackCoords->x1, trackCoords->y0, trackCoords->x1, trackCoords->y1);
        dc.DrawLine( trackCoords->x0, trackCoords->y0, trackCoords->x1, trackCoords->y0);
        dc.DrawLine( trackCoords->x0, trackCoords->y1, trackCoords->x1, trackCoords->y1);
#endif
        
        EditorPrintable* editorPrintable = this->getEditorPrintable(n);
        editorPrintable->drawTrack(n, sizing, line, dc, first);
        first = false;
    }
}
    
// -----------------------------------------------------------------------------------------------------------------

wxString PrintableSequence::getTitle() const
{
    wxString song_title = m_sequence->suggestTitle();
    wxString track_title;
    
    //FIXME: what's that line for??
    if (m_tracks.size()==1) m_tracks[0].getName();
    
    wxString final_title;
    
    // give song title
    if (song_title.IsSameAs(_("Untitled"))) final_title = _("Aria Maestosa song");
    else                                    final_title = song_title;
    
    // give track name, if any
    if (!track_title.IsSameAs(_("Untitled")) and track_title.Length()>0)
    {
        final_title += (wxT(", ") + track_title);
    }
    
    std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;
}

// -----------------------------------------------------------------------------------------------------------------

int PrintableSequence::getPageAmount() const
{
    return layoutPages.size();
}

// -----------------------------------------------------------------------------------------------------------------

LayoutPage& PrintableSequence::getPage(const int id)
{
    ASSERT_E(id, >=, 0);
    ASSERT_E(id, <, (int)layoutPages.size());
    
    LayoutPage& out = layoutPages[id];
    
    ASSERT( MAGIC_NUMBER_OK_FOR(&out) );
    
    return out;
}
    
