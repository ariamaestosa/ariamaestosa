/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "PreferencesData.h"
#include "Printing/AriaPrintable.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutNumeric.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutLine.h"
#include "Printing/SymbolPrinter/TabPrint.h"
#include "Printing/SymbolPrinter/ScorePrint.h"
#include "Printing/SymbolPrinter/SymbolPrintableSequence.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"

#include <wx/dc.h>

using namespace AriaMaestosa;

#define DEBUG_DRAW 0

SymbolPrintableSequence::SymbolPrintableSequence(Sequence* parent) : AbstractPrintableSequence(parent)
{
    m_is_guitar_editor_used = false;
    m_is_score_editor_used = false;
    m_max_signs_in_keysig = 0;
    m_layout_calculated = false;
    m_numeric_layout_manager = NULL;
}

// -----------------------------------------------------------------------------------------------------------------

bool SymbolPrintableSequence::addTrack(GraphicalTrack* gtrack, NotationType mode)
{
    Track* track = gtrack->getTrack();
    ASSERT(track->getSequence() == m_sequence);
    ASSERT(not m_layout_calculated);
    
    if (mode == GUITAR)
    {
        m_editor_printables.push_back(new TablaturePrintable(gtrack));
        m_is_guitar_editor_used = true;
    }
    else if (mode == SCORE)
    {
        m_editor_printables.push_back(new ScorePrintable(gtrack->getTrack()));
        m_is_score_editor_used = true;
        
        m_max_signs_in_keysig = std::max( m_max_signs_in_keysig,
                                       std::max(track->getKeySharpsAmount(),
                                                track->getKeyFlatsAmount()) );
    }
    else
    {
        std::cerr << "[SymbolPrintableSequence] addTrack : mode " << mode << " not supported for printing" << std::endl;
        return false;
    }
    
    return AbstractPrintableSequence::addTrack(gtrack, mode);
}

// -----------------------------------------------------------------------------------------------------------------

void SymbolPrintableSequence::calculateLayout()
{
    ASSERT( MAGIC_NUMBER_OK_FOR(m_tracks) );
    
    m_abstract_layout_manager = new PrintLayoutAbstract(this);
    m_abstract_layout_manager->addLayoutInformation(m_tracks, layoutPages /* out */);
    
    // prepare it for when we're ready to print
    m_numeric_layout_manager = new PrintLayoutNumeric();
    
    AbstractPrintableSequence::calculateLayout();
}

// -----------------------------------------------------------------------------------------------------------------

EditorPrintable* SymbolPrintableSequence::getEditorPrintable(const int trackID)
{
    return m_editor_printables.get(trackID);
}

// -----------------------------------------------------------------------------------------------------------------

EditorPrintable* SymbolPrintableSequence::getEditorPrintableFor(Track* track)
{
    for (int n=0; n<m_editor_printables.size(); n++)
    {
        if (m_editor_printables[n].getTrack() == track) return m_editor_printables.get(n);
    }
    
    ASSERT(false);
    return m_editor_printables.get(0);
}

// ------------------------------------------------------------------------------------------------------------------

void SymbolPrintableSequence::printLinesInArea(wxDC& dc, wxGraphicsContext* grctx, const int pageID, 
                                               const float notation_area_y0, const float notation_area_h,
                                               const int pageHeight, const int x0, const int x1)
{
    ASSERT(m_layout_calculated);
    ASSERT(m_numeric_layout_manager != NULL);
    
    ASSERT(notation_area_h > 0);
    ASSERT(pageHeight > 0);
    
    //ASSERT_E(notation_area_y0 + notation_area_h, <=, pageHeight);

    LayoutPage& page = getPage(pageID);
    
    // ---- Give each track an area on the page
    m_numeric_layout_manager->placeLinesInPage(page, notation_area_y0, notation_area_h,
                                               pageHeight, x0, x1);
    
    // ---- Draw the tracks
    const wxFont regularFont = getPrintFont();
    
    const int lineAmount = page.getLineCount();
    for (int l=0; l<lineAmount; l++)
    {
        dc.SetFont( regularFont );
        this->printLine(page.getLine(l), dc, grctx);
    }
}

// -----------------------------------------------------------------------------------------------------------------

void SymbolPrintableSequence::printLine(LayoutLine& line, wxDC& dc, wxGraphicsContext* grctx)
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
    
    /*
     int getFirstLineY(const LineTrackRef& currentTrack, const int y0, const int y1);

     */
    
    if (trackAmount > 1)
    {
        const LineTrackRef& currentTrack = line.getLineTrackRef(0);
        const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        EditorPrintable* editorPrintable = this->getEditorPrintable(0);
        const int y0 = editorPrintable->getFirstLineY(currentTrack);
        
        dc.SetPen(  wxPen( wxColour(150,150,150), 25 ) );
        dc.DrawLine( lineCoords->x0-3, y0, lineCoords->x0-3, my1); // vertical line
        dc.DrawLine( lineCoords->x0-3, y0, lineCoords->x0-3+30, y0-50); // top thingy
        dc.DrawLine( lineCoords->x0-3, my1, lineCoords->x0-3+30, my1+50); // bottom thingy
        
        dc.DrawLine( lineCoords->x1-3, y0, lineCoords->x1-3, my1); // right-side line
    }
    
    std::cout << "[SymbolPrintableSequence] ==== Printing Line (contains " << line.getLayoutElementCount()
              << " layout elements), measures " << (line.getFirstMeasure()+1) << " to " << (line.getLastMeasure()+1)
              << " (tick " << line.getMeasureForElement(0).getFirstTick() << " to "
              << line.getMeasureForElement(line.getLayoutElementCount() - 1).getLastTick() << ")"
              << "; from y=" << my0 << " to " << my1 << " ====" << std::endl;
    
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
        
        std::cout << "[SymbolPrintableSequence] --- Printing track " << n << " ---\n";
        
        const LineTrackRef& sizing = line.getLineTrackRef(n);
        const TrackCoords* trackCoords = sizing.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        
        std::cout << "[SymbolPrintableSequence] Coords : " << trackCoords->x0 << ", " << trackCoords->y0
                  << " to " << trackCoords->x1 << ", " << trackCoords->y1 << std::endl;
        
#if DEBUG_DRAW
        dc.SetPen(  wxPen( wxColour(0,255,0), 12 ) );
        dc.DrawLine( trackCoords->x0, trackCoords->y0, trackCoords->x0, trackCoords->y1);
        dc.DrawLine( trackCoords->x1, trackCoords->y0, trackCoords->x1, trackCoords->y1);
        dc.DrawLine( trackCoords->x0, trackCoords->y0, trackCoords->x1, trackCoords->y0);
        dc.DrawLine( trackCoords->x0, trackCoords->y1, trackCoords->x1, trackCoords->y1);
#endif
        
        EditorPrintable* editorPrintable = this->getEditorPrintable(n);
        editorPrintable->drawTrackBackground(n, sizing, line, dc, grctx, first);
    }
    
    first = true;
    for (int n=0; n<trackAmount; n++)
    {
        const LineTrackRef& sizing = line.getLineTrackRef(n);

        EditorPrintable* editorPrintable = this->getEditorPrintable(n);
        editorPrintable->drawTrack(n, sizing, line, dc, grctx, first);
        first = false;
    }
}
    
// -----------------------------------------------------------------------------------------------------------------

int SymbolPrintableSequence::getPageAmount() const
{
    return layoutPages.size();
}

// -----------------------------------------------------------------------------------------------------------------

LayoutPage& SymbolPrintableSequence::getPage(const int id)
{
    ASSERT_E(id, >=, 0);
    ASSERT_E(id, <, (int)layoutPages.size());
    
    LayoutPage& out = layoutPages[id];
    
    ASSERT( MAGIC_NUMBER_OK_FOR(out) );
    
    return out;
}
    
