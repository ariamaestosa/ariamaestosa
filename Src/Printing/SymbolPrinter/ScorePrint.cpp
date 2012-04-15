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

//#include <wx/sizer.h>
//#include <wx/file.h>
#include <wx/filename.h>
#include <wx/dc.h>
#include <wx/image.h>

#include "AriaCore.h"
#include "Analysers/ScoreAnalyser.h"
#include "Editors/ScoreEditor.h"
#include "IO/IOUtils.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Printing/AriaPrintable.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutLine.h"
#include "Printing/SymbolPrinter/PrintLayout/RelativePlacementManager.h"
#include "Printing/SymbolPrinter/ScorePrint.h"
#include "Printing/RenderRoutines.h"

#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
#include <wx/graphics.h>
#endif

const bool LOGGING = false;
#define BE_VERBOSE 0

namespace AriaMaestosa
{
    /** Width of the "natural" accidental symbol */
    const int NATURAL_SIGN_WIDTH = 30;
    
    /** Used to determine how much space to leave for accidentals (FIXME: use finer support) */
    const int MAX_ACCIDENTAL_SIZE = 80;
    
    /** how much extra is allocated at the left and right of note heads */
    const int NOTE_HEAD_MARGIN = 44;
     
    const int LINES_IN_A_SCORE = 5;
    
    /** Out of 1.0 (100%), how much of the height is reserved for the space between G and F clefs
      * (when both are present) FIXME: this should probably be an absolute margin, not a relative proportion?
      */
    const float MARGIN_PROPORTION_BETWEEN_CLEFS = 0.2f;
    
    /** how many units after the note head to render the dot (for dotted notes) */
    const int DOT_SHIFT_AFTER_NOTE_HEAD = 20;
    
#if 0
#pragma mark -
#pragma mark ScoreData
#endif
    
    /**
      * TODO: document
      * @ingroup printing
      */
    class PerMeasureInfo
    {
    public:
        int highest_pitch;
        int lowest_pitch;
        int smallest_level;
        int biggest_level;
        int first_note;
        int last_note;
        
        PerMeasureInfo()
        {
            highest_pitch = -1;
            lowest_pitch = -1;
            smallest_level = -1;
            biggest_level = -1;
            first_note = -1;
            last_note = -1;
        }
    };
    
    /**
      * TODO: document
      * @ingroup printing
      */
    class ScoreData : public LineTrackRef::EditorData
    {
    public:            
        LEAK_CHECK();
        
        virtual ~ScoreData() {}
        
        int extra_lines_above_g_score;
        int extra_lines_under_g_score;
        int extra_lines_above_f_score;
        int extra_lines_under_f_score;
        float first_clef_proportion ;
        float second_clef_proportion;
    };
    
    // FIXME : find cleaner way to keep info per-track
    std::map< int /* trackID*5000 measure ID */, PerMeasureInfo > perMeasureInfo;



#if 0
#pragma mark -
#pragma mark PrintXConverter
#endif
    
    /**
      * @brief A x-coord converter that can be given to the score analyser and handles printing contextes.
      *
      * An instance of this will be given to the score analyser. Having a separate x-coord-converter
      * allows it to do conversions between units without becoming context-dependent.
      */
    class PrintXConverter
    {
        ScorePrintable* m_parent;
        LayoutLine* m_line;
        int m_track_id;
        
    public:
        LEAK_CHECK();
        
        PrintXConverter(ScorePrintable* parent, LayoutLine* line, const int trackID)
        {
            m_parent   = parent;
            m_line     = line;
            m_track_id = trackID;
        }
        ~PrintXConverter()
        {
        }
        
        Range<int> tickToX(const int tick)
        {
            return m_parent->tickToX(m_track_id, *m_line, tick);
        }
    };
    
#if 0
#pragma mark -
#pragma mark Render Routines
#endif
    
    /*
     * A few drawing routines
     */


    const int SHARP_WIDTH = 74;
    
    // -------------------------------------------------------------------------------------------
    /** @brief Renders a 'sharp' symbol at the given coordinates
      * @param center  If true, the center of the symbol is drawn at the x coordinate.
      *                If false, the right side of the symbol is drawn the x coordinate.
      */
    void renderSharp(wxDC& dc, int x, const int y, const bool center=true)
    {        
        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        //const int WIDTH = 50;
        //const int INNER_WIDTH = 20;
        //const int HEIGHT = 40;
        //const int INNER_HEIGHT = 20;
        //const int SLANT = 10;

        const int WIDTH = SHARP_WIDTH;
        const int INNER_WIDTH = 30;
        const int HEIGHT = 60;
        const int INNER_HEIGHT = 30;
        const int SLANT = 15;
        
        if (not center) x -= WIDTH;

        // horizontal lines
        dc.DrawLine( x-WIDTH/2, y - INNER_HEIGHT/2 + SLANT/2, x+WIDTH/2, y-INNER_HEIGHT/2 - SLANT/2 );
        dc.DrawLine( x-WIDTH/2, y + INNER_HEIGHT/2 + SLANT/2, x+WIDTH/2, y+INNER_HEIGHT/2 - SLANT/2 );
        
        // vertical lines
        dc.DrawLine( x-INNER_WIDTH/2, y - HEIGHT/2 + SLANT/4, x-INNER_WIDTH/2, y + HEIGHT/2 + SLANT/4 );
        dc.DrawLine( x+INNER_WIDTH/2, y - HEIGHT/2 - SLANT/4, x+INNER_WIDTH/2, y + HEIGHT/2 - SLANT/4 );
    }
    
    // -------------------------------------------------------------------------------------------
    
    const int FLAT_WIDTH = 30;
    
    /** @brief Renders a 'flat' symbol at the given coordinates
      * @param center  If true, the center of the symbol is drawn at the x coordinate.
      *                If false, the right side of the symbol is drawn the x coordinate.
      */
    void renderFlat(wxDC& dc, int x, const int y, const bool center=true)
    {
        if (not center) x -= 30;

        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        wxPoint points[] =
        {
            wxPoint(x,    y-18-43),
            wxPoint(x,    y+72-43),
            wxPoint(x+6,  y+72-43),
            wxPoint(x+30, y+36-43),
            wxPoint(x+18, y+24-43),
            wxPoint(x,    y+36-43)
        };
        dc.DrawSpline(6, points);
    }
    
    // -------------------------------------------------------------------------------------------
    /** @brief Renders a 'natural' symbol at the given coordinates
      * @param center  If true, the center of the symbol is drawn at the x coordinate.
      *                If false, the right side of the symbol is drawn the x coordinate.
      */
    void renderNatural(wxDC& dc, int x, const int y, const bool center=true)
    {
        if (not center) x -= NATURAL_SIGN_WIDTH;

        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        // horizontal lines
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y,      x+NATURAL_SIGN_WIDTH/2, y-20/2 );
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y+40/2, x+NATURAL_SIGN_WIDTH/2, y+20/2 );
        
        // vertical lines
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y+40/2, x-NATURAL_SIGN_WIDTH/2, y-60/2 );
        dc.DrawLine( x+NATURAL_SIGN_WIDTH/2, y-20/2, x+NATURAL_SIGN_WIDTH/2, y+80/2 );
    }
    
    // -------------------------------------------------------------------------------------------------------
    /** Renders a 'G clef' sign at the the given coordinates */
    void renderGClef(wxDC& dc, wxGraphicsContext* gc, const int x, const float score_bottom,
                     const float b_line_y)
    {
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
        RenderRoutines::paintTreble(*gc, x + 20, b_line_y, score_bottom);
        
        //dc.SetBrush(*wxRED_BRUSH);
        //dc.SetPen(*wxRED_PEN);
        //dc.DrawRectangle( x + 10, b_line_y - 10, 20, 20 );
#else
        const int b_on_image = 30;
        const int bottom_on_image = 49;
        const float scale = (score_bottom - b_line_y) / (float)(bottom_on_image - b_on_image);
        const int y = score_bottom - bottom_on_image*scale;
        static wxString path = getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() +
                               wxT("keyG.png");
        
        /*
        static wxBitmap gclef(path, wxBITMAP_TYPE_PNG);
        wxBitmap scaled = wxBitmap(gclef.ConvertToImage().Scale(gclef.GetWidth()*scale,
                                   gclef.GetHeight()*scale));
        */
        
        wxImage inputGClef(path, wxBITMAP_TYPE_PNG);
        wxImage gclef = RenderRoutines::getPrintableImage(inputGClef);
        wxBitmap scaled = wxBitmap(gclef.Scale(gclef.GetWidth()*scale, gclef.GetHeight()*scale),
                                   wxIMAGE_QUALITY_HIGH);
        

        dc.DrawBitmap(scaled, x, y, true);
#endif
    }
    
    // -------------------------------------------------------------------------------------------------------
    /** Renders a 'F clef' sign at the the given coordinates */
    void renderFClef(wxDC& dc, wxGraphicsContext* gc, const int x, const float score_top,
                     const float e_line_y)
    {
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
        RenderRoutines::paintBass(*gc, x + 20, score_top, e_line_y);
#else
        const int e_on_image = 15;
        const float scale = (float)(e_line_y - score_top) / (float)e_on_image;
        wxString path =  getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() +
                         wxT("FKey.png");
        
        wxImage inputFClef(path, wxBITMAP_TYPE_PNG);
        wxImage fclef = RenderRoutines::getPrintableImage(inputFClef);
        wxBitmap scaled = wxBitmap(fclef.Scale(fclef.GetWidth()*scale, fclef.GetHeight()*scale),
                                   wxIMAGE_QUALITY_HIGH);
        /*
        static wxBitmap fclef( , wxBITMAP_TYPE_PNG );
        wxBitmap scaled = wxBitmap(fclef.ConvertToImage().Scale(fclef.GetWidth()*scale,
                                   fclef.GetHeight()*scale));
        */
        
        dc.DrawBitmap(scaled, x, score_top, true);
#endif
    }
    
    // -------------------------------------------------------------------------------------------------------

    // leave a pointer to the dc for the callback
    // FIXME: find cleaner way than globals
    wxDC* global_dc = NULL;
    
#if wxCHECK_VERSION(2,9,1)
    wxGraphicsContext* gc = NULL;
#endif
    
    ScorePrintable* g_printable = NULL;
    
    // leave height of lines for the renderSilence callback
    // FIXME: find cleaner way than globals
    int g_line_height=5;
    
    // -------------------------------------------------------------------------------------------------------
    
    void renderSilenceCallback(const Sequence*, const int duration, const int tick, const int type,
                               const int silences_y, const bool triplet, const bool dotted,
                               const int dot_delta_x, const int dot_delta_y, void* userdata)
    {
        ASSERT( global_dc != NULL);
        
        PrintXConverter* converter = (PrintXConverter*)userdata;
        
        const Range<int> x = converter->tickToX(tick);
        
        // silences in gathered rests, for instance, will not be found by tickToX
        if (x.from < 0 or x.to < 0) return;
        
        //global_dc->SetPen(*wxRED_PEN);
        //global_dc->SetBrush(*wxTRANSPARENT_BRUSH);
        //global_dc->DrawRectangle(x.from, silences_y-20, x.to - x.from, 420);
        
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
        RenderRoutines::drawSilence(*gc, x, silences_y, g_line_height, type, triplet, dotted);
#else
        RenderRoutines::drawSilence(global_dc, x, silences_y, g_line_height, type, triplet, dotted);
#endif
    }
    
#if 0
#pragma mark -
#pragma mark ScorePrintable (EditorPrintable common interface)
#endif
    
    // -------------------------------------------------------------------------------------------------------
    
    ScorePrintable::ScorePrintable(Track* track) : EditorPrintable(track)
    {
        // std::cout << " *** setting global g_printable" << std::endl;
        g_printable = this;
        
        m_g_clef_y_from        = -1;
        m_g_clef_y_to          = -1;
        m_f_clef_y_from        = -1;
        m_f_clef_y_to          = -1;
        m_grand_staff_center_y = -1;
        m_line_height          = -1;
        m_min_level            = -1;
        m_first_score_level    = -1;
        m_last_score_level     = -1;
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    ScorePrintable::~ScorePrintable()
    {
    }
    
    // -------------------------------------------------------------------------------------------------------
      
    int ScorePrintable::calculateHeight(const int trackID, LineTrackRef& lineTrack, LayoutLine& line,
                                        bool* empty)
    {
        gatherVerticalSizingInfo(trackID, lineTrack, line);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(lineTrack.editor_data.raw_ptr);
        ASSERT(scoreData != NULL);
        
        const int from_note = lineTrack.getFirstNote();
        const int to_note   = lineTrack.getLastNote();
                
#if BE_VERBOSE
        std::cout <<
            PRINT_VAR(scoreData->extra_lines_under_g_score) <<
            PRINT_VAR(scoreData->extra_lines_above_g_score) <<
            PRINT_VAR(scoreData->extra_lines_under_f_score) <<
            PRINT_VAR(scoreData->extra_lines_above_f_score) << std::endl;
#endif
        
        int total = (m_g_clef ? 5 : 0) + (m_f_clef ? 5 : 0) + 
                abs(scoreData->extra_lines_under_g_score) +
                abs(scoreData->extra_lines_above_g_score) +
                abs(scoreData->extra_lines_under_f_score) +
                abs(scoreData->extra_lines_above_f_score);
        
        // if we have both scores, add the margin between them to the required space.
        if (m_g_clef and m_f_clef)
        {
            //FIXME: it's not too clear whether needed additional space should be returned
            //       as a function of levels??
            total = total + MARGIN_PROPORTION_BETWEEN_CLEFS*total;
        }
        
        // check if empty
        // FIXME : if a note starts in the previous line and ends in this one, it won't be detected
        if (from_note == -1 or to_note == -1) *empty = true;
        else                                  *empty = false;
        
        return total;

    }
    
    // -------------------------------------------------------------------------------------------
#define VERBOSE 0
    
    void ScorePrintable::addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                      MeasureTrackReference& trackRef,
                                      RelativePlacementManager& ticks_relative_position)
    {
        const int measureFromTick = measure.getFirstTick();
        const int measureToTick   = measure.getLastTick();
        
#if VERBOSE
        std::cout << "[ScorePrintable] addingTicks(measure " << (measure.id+1) << ", from "
                  << measureFromTick << ", to " << measureToTick << "\n{\n";
#endif
        
        GraphicalTrack* gtrack = trackRef.getTrack();
        ScoreEditor* scoreEditor = gtrack->getScoreEditor();
        
        ASSERT(gtrack->getTrack() == m_track);
        
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        // ---- notes
        for (int clef=0; clef<2; clef++)
        {
            ScoreAnalyser* current_analyser;
            if (clef == 0)
            {
                if (m_g_clef) current_analyser = g_clef_analyser;
                else          continue;
            }
            else if (clef == 1)
            {
                if (m_f_clef) current_analyser = f_clef_analyser;
                else          continue;
            }
            else
            {
                ASSERT(false);
                break;
            }
            ASSERT( current_analyser != NULL );
            
            const int noteAmount = current_analyser->m_note_render_info.size();
                        
            // find shortest note
            int shortest = -1;
            for (int n=0; n<noteAmount; n++)
            {
                const int tick = current_analyser->m_note_render_info[n].getTick();
                if (tick < measureFromTick or tick >= measureToTick) continue;
                
                if (current_analyser->m_note_render_info[n].getTickLength() < shortest or shortest == -1)
                {
                    shortest = current_analyser->m_note_render_info[n].getTickLength();
                }
            }
            
            for (int n=0; n<noteAmount; n++)
            {
                const int tick = current_analyser->m_note_render_info[n].getTick();
                if (tick < measureFromTick or tick >= measureToTick) continue;
                
                const int tickTo = tick + current_analyser->m_note_render_info[n].getTickLength();
                ASSERT_E(tick, <=, tickTo);
                
#if VERBOSE
                std::cout << "    Adding tick " << tick << " to list" << std::endl;
#endif
                
                if (current_analyser->m_note_render_info[n].m_sign != PITCH_SIGN_NONE)
                {
                    // if there's an accidental sign to show, allocate a bigger space for this note
                    ticks_relative_position.addSymbol( tick, tickTo,
                                                       HEAD_RADIUS*2 + MAX_ACCIDENTAL_SIZE + NOTE_HEAD_MARGIN,
                                                       trackID );
                }
                else
                {
                    // these proportion numbers have been determined experimentally
                    ticks_relative_position.addSymbol( tick, tickTo, HEAD_RADIUS*2 + NOTE_HEAD_MARGIN, trackID );
                }
            }
        } // end for each clef

        // ---- silences
        ticks_relative_position.addSilenceSymbols(m_silences_ticks, trackID,
                                                  measureFromTick, measureToTick);
                
#if VERBOSE
        std::cout << "}\n";
        std::cout << "[ScorePrintable] addUsedTicks : silences\n{\n";
        for (int n=0; n< m_silences_ticks.size(); n++)
        {
            std::cout << "    tick " << m_silences_ticks[n].m_tick_range.from << " (beat "
                      << m_silences_ticks[n].m_tick_range.from/float(getMeasureData()->beatLengthInTicks())
                      << ")\n";
        }
        std::cout << "}\n";
#endif
    }
    
    // ------------------------------------------------------------------------------------------------------
    
    void ScorePrintable::calculateVerticalMeasurements(const TrackCoords* trackCoords, ScoreData* scoreData)
    {
        // if we have only one clef, give it the full space.
        // if we have two, split the space between both
        m_g_clef_y_from = -1;
        m_g_clef_y_to   = -1;
        m_f_clef_y_from = -1;
        m_f_clef_y_to   = -1;
        
        if (m_g_clef and not m_f_clef)
        {
            m_g_clef_y_from = trackCoords->y0;
            m_g_clef_y_to   = trackCoords->y1;
        }
        else if (m_f_clef and not m_g_clef)
        {
            m_f_clef_y_from = trackCoords->y0;
            m_f_clef_y_to   = trackCoords->y1;
        }
        else if (m_f_clef and m_g_clef)
        {
            m_g_clef_y_from = trackCoords->y0;
            m_g_clef_y_to   = trackCoords->y0 +
            (int)round((trackCoords->y1 - trackCoords->y0)*scoreData->first_clef_proportion);
            m_f_clef_y_from = trackCoords->y0 +
            (int)round((trackCoords->y1 - trackCoords->y0)*(1 - scoreData->second_clef_proportion));
            m_f_clef_y_to   = trackCoords->y1;
        }
        else
        {
            ASSERT(false);
        }
        
    }
    
    // ------------------------------------------------------------------------------------------------------

    void ScorePrintable::calculateClefVerticalMeasurements(ClefRenderType clefType,
                                                           const GraphicalTrack* gtrack,
                                                           int extra_lines_above)
    {
        const bool f_clef = (clefType == F_CLEF_ALONE or clefType == F_CLEF_FROM_GRAND_STAFF);

        const ScoreEditor* scoreEditor = gtrack->getScoreEditor();
        const ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        const int middle_c_level = converter->getScoreCenterCLevel();
        m_first_score_level = middle_c_level + (f_clef? 2 : -10);
        m_last_score_level  = m_first_score_level + 8;
        m_min_level         = m_first_score_level - extra_lines_above*2;    
    }
    
    // ------------------------------------------------------------------------------------------------------
    
#define LEVEL_TO_Y( lvl ) (y0 + 1 + m_line_height*0.5*((lvl) - m_min_level))

    int ScorePrintable::getFirstLineY(const LineTrackRef& currentTrack)
    {
        const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        
        ASSERT_E(trackCoords->y0,>,0);
        ASSERT_E(trackCoords->y1,>,0);
        ASSERT_E(trackCoords->y0,<,50000);
        ASSERT_E(trackCoords->y1,<,50000);
                
        ScoreData* scoreData = dynamic_cast<ScoreData*>(currentTrack.editor_data.raw_ptr);
        
        calculateVerticalMeasurements(trackCoords, scoreData);
        

        if (m_g_clef)
        {
            float y0 = m_g_clef_y_from;
            float y1 = m_g_clef_y_to;

            const int lineAmount = abs(scoreData->extra_lines_above_g_score) +
                                   abs(scoreData->extra_lines_under_g_score) + 5;
            m_line_height = (float)(y1 - y0) / (float)(lineAmount-1);
            
            m_grand_staff_center_y = (m_g_clef and m_f_clef ? (m_g_clef_y_to + m_f_clef_y_from)/2 : -1);
        
            
            ClefRenderType clef = (m_f_clef ? G_CLEF_FROM_GRAND_STAFF : G_CLEF_ALONE);
            
            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_g_score));
            return LEVEL_TO_Y(m_first_score_level);
        }
        else
        {
            float y0 = m_f_clef_y_from;
            float y1 = m_f_clef_y_to;
            
            const int lineAmount = abs(scoreData->extra_lines_above_f_score) +
                                   abs(scoreData->extra_lines_under_f_score) + 5;
            m_line_height = (float)(y1 - y0) / (float)(lineAmount-1);
            
            m_grand_staff_center_y = (m_g_clef and m_f_clef ? (m_g_clef_y_to + m_f_clef_y_from)/2 : -1);
            
            
            ClefRenderType clef = (m_g_clef ? F_CLEF_FROM_GRAND_STAFF : F_CLEF_ALONE);
            
            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_f_score));
            return LEVEL_TO_Y(m_first_score_level);
        }
    }
    
    // ------------------------------------------------------------------------------------------------------

    void ScorePrintable::drawTrackBackground(const int trackID, const LineTrackRef& currentTrack,
                                             LayoutLine& currentLine, wxDC& dc, wxGraphicsContext* grctx,
                                             const bool drawMeasureNumbers)
    {
        const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        
        ASSERT_E(trackCoords->y0,>,0);
        ASSERT_E(trackCoords->y1,>,0);
        ASSERT_E(trackCoords->y0,<,50000);
        ASSERT_E(trackCoords->y1,<,50000);
        setCurrentDC(&dc);
        
        if (LOGGING)
        {
            std::cout << "[ScorePrintable] ScorePrintable size : " << trackCoords->x0 << ", " << trackCoords->y0
                      << " to " << trackCoords->x1 << ", " << trackCoords->y1 << std::endl;
        }
        
        m_x_converter = new PrintXConverter(this, &currentLine, trackID);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(currentTrack.editor_data.raw_ptr);
        
        calculateVerticalMeasurements(trackCoords, scoreData);
        g_printable = this;
        
        // draw track name
        int score_area_from_y = (m_g_clef ? m_g_clef_y_from : m_f_clef_y_from);
        int score_area_to_y   = (m_f_clef ? m_f_clef_y_to   : m_g_clef_y_to);
        drawTrackName(dc, currentTrack, trackCoords->x0, score_area_from_y, score_area_to_y);
        
        m_grand_staff_center_y = (m_g_clef and m_f_clef ? (m_g_clef_y_to + m_f_clef_y_from)/2 : -1);
        
        if (m_g_clef)
        {
            ClefRenderType clef = (m_f_clef ? G_CLEF_FROM_GRAND_STAFF : G_CLEF_ALONE);
            
            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_g_score));
            
            backgroundDrawing(clef, *g_clef_analyser, currentLine, currentTrack.getTrack(),
                              dc, grctx, abs(scoreData->extra_lines_above_g_score),
                              abs(scoreData->extra_lines_under_g_score),
                              trackCoords->x0, m_g_clef_y_from, trackCoords->x1, m_g_clef_y_to,
                              drawMeasureNumbers, m_grand_staff_center_y);
        }
        
        if (m_f_clef)
        {
            ClefRenderType clef = (m_g_clef ? F_CLEF_FROM_GRAND_STAFF : F_CLEF_ALONE);
            
            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_f_score));
            
            backgroundDrawing(clef, *f_clef_analyser, currentLine, currentTrack.getTrack(),
                              dc, grctx, abs(scoreData->extra_lines_above_f_score),
                              abs(scoreData->extra_lines_under_f_score),
                              trackCoords->x0, m_f_clef_y_from, trackCoords->x1, m_f_clef_y_to,
                              (m_g_clef ? false : drawMeasureNumbers) /* if we have both keys don't show twice */,
                              m_grand_staff_center_y);
        }
    }
    
    // ------------------------------------------------------------------------------------------------------

    void ScorePrintable::drawTrack(const int trackID, const LineTrackRef& currentTrack,
                                   LayoutLine& currentLine, wxDC& dc, wxGraphicsContext* grctx,
                                   const bool drawMeasureNumbers)
    {
        if (m_g_clef)
        {
            ASSERT(m_g_clef_y_from        != -1);
            ASSERT(m_g_clef_y_to          != -1);
        }
        if (m_f_clef)
        {
            ASSERT(m_f_clef_y_from        != -1);
            ASSERT(m_f_clef_y_to          != -1);
        }
        if (m_g_clef and m_f_clef)
        {
            ASSERT(m_grand_staff_center_y != -1);
        }
        ASSERT(m_line_height          != -1);
        ASSERT(m_min_level            != -1);
        ASSERT(m_first_score_level    != -1);
        ASSERT(m_last_score_level     != -1);
        
        const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
        ASSERT(trackCoords != NULL);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(currentTrack.editor_data.raw_ptr);

        // ---- draw scores
        if (m_g_clef)
        {
            ClefRenderType clef = (m_f_clef ? G_CLEF_FROM_GRAND_STAFF : G_CLEF_ALONE);
            
            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_g_score));
            
            analyseAndDrawScore(clef, *g_clef_analyser, currentLine, currentTrack.getTrack(),
                                dc, grctx, abs(scoreData->extra_lines_above_g_score),
                                abs(scoreData->extra_lines_under_g_score),
                                trackCoords->x0, m_g_clef_y_from, trackCoords->x1, m_g_clef_y_to,
                                drawMeasureNumbers, m_grand_staff_center_y);
        }
        
        if (m_f_clef)
        {
            ClefRenderType clef = (m_g_clef ? F_CLEF_FROM_GRAND_STAFF : F_CLEF_ALONE);

            calculateClefVerticalMeasurements(clef,
                                              currentTrack.getTrack(),
                                              abs(scoreData->extra_lines_above_f_score));
            
            analyseAndDrawScore(clef, *f_clef_analyser, currentLine, currentTrack.getTrack(),
                                dc, grctx, abs(scoreData->extra_lines_above_f_score),
                                abs(scoreData->extra_lines_under_f_score),
                                trackCoords->x0, m_f_clef_y_from, trackCoords->x1, m_f_clef_y_to,
                                (m_g_clef ? false : drawMeasureNumbers) /* if we have both keys don't show twice */,
                                m_grand_staff_center_y);
        }
        
        delete m_x_converter;
        m_x_converter = NULL;
        
        // ---- Debug guides
        if (PRINT_LAYOUT_HINTS)
        {
            dc.SetPen( wxPen(*wxBLUE, 7) );
            dc.DrawLine(trackCoords->x0, trackCoords->y0, trackCoords->x1, trackCoords->y0);
            dc.DrawLine(trackCoords->x0, trackCoords->y1, trackCoords->x1, trackCoords->y1);
            dc.DrawLine(trackCoords->x0, trackCoords->y0, trackCoords->x0, trackCoords->y1);
            dc.DrawLine(trackCoords->x1, trackCoords->y0, trackCoords->x1, trackCoords->y1);
        }
    }

    // -------------------------------------------------------------------------------------------

    void ScorePrintable::gatherVerticalSizingInfo(const int trackID, LineTrackRef& lineTrack, LayoutLine& line)
    {
        ScoreData* scoreData = new ScoreData();
        lineTrack.editor_data = scoreData;
        
        const GraphicalTrack* gtrack = lineTrack.getTrack();
        const ScoreEditor* scoreEditor = gtrack->getScoreEditor();
        const ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        ASSERT(gtrack->getTrack() == m_track);
        
        // ---- Determine the y level of the highest and the lowest note
        const int fromMeasure = line.getFirstMeasure();
        const int lastMeasure = line.getLastMeasure();

        int biggest_level = -1, smallest_level = -1;
        
        for (int m=fromMeasure; m<=lastMeasure; m++)
        {
            //  FIXME : see other fixme above, this * 500 + m trick is stupid and ugly
            PerMeasureInfo& info = perMeasureInfo[trackID*5000+m];
            
            if ((info.biggest_level > biggest_level or biggest_level == -1) and info.biggest_level != -1)
            {
                biggest_level = info.biggest_level;
            }
            if ((info.smallest_level < smallest_level or smallest_level == -1) and info.smallest_level != -1)
            {
                smallest_level = info.smallest_level;
            }
        }
        
        // ---- get some values useful for later
        m_middle_c_level = converter->getScoreCenterCLevel(); //converter->getMiddleCLevel();
        
        const int g_clef_from_level = m_middle_c_level - 10;
        const int g_clef_to_level   = m_middle_c_level - 2;
        const int f_clef_from_level = m_middle_c_level + 2;
        const int f_clef_to_level   = m_middle_c_level + 10;
        
        m_g_clef = scoreEditor->isGClefEnabled();
        m_f_clef = scoreEditor->isFClefEnabled();
        
        const Sequence* seq = gtrack->getSequence()->getModel();
        const MeasureData* md = seq->getMeasureData();
        
        const int fromTick = md->firstTickInMeasure( line.getFirstMeasure() );
        const int toTick   = md->lastTickInMeasure ( line.getLastMeasure() );

        // ---- check if some signs (stems, triplet signs, etc.) go out of bounds
        for (int n=0; n<2; n++) // 0 is G clef, 1 is F clef
        {
            if (n == 0 and not m_g_clef) continue;
            if (n == 1 and not m_f_clef) continue;
            
            // analyse notes. this analysis will be used to determine is some things go out of the track
            // verticall, and will be thrown away after [FIXME] (it will be analysed again when it's time to render)
            ScoreAnalyser* analyser = NULL;
            if      (n == 0) analyser = g_clef_analyser;
            else if (n == 1) analyser = f_clef_analyser;
            else             ASSERT(false);
            
            ASSERT(analyser != NULL);

            OwnerPtr<ScoreAnalyser> lineAnalyser;
            lineAnalyser = analyser->getSubset(fromTick, toTick);
            lineAnalyser->analyseNoteInfo();
            
            const int noteAmount = lineAnalyser->m_note_render_info.size();
            for (int i=0; i<noteAmount; i++)
            {
                NoteRenderInfo& noteRenderInfo = lineAnalyser->m_note_render_info[i];
                
                // --- stem
                if (noteRenderInfo.m_draw_stem and noteRenderInfo.m_stem_type != STEM_NONE)
                {
                    const float stem_y_level = analyser->getStemTo(noteRenderInfo);
                    
                    if (stem_y_level < smallest_level or smallest_level == -1)
                    {
                        smallest_level = stem_y_level;
                    }
                    if (stem_y_level > biggest_level)
                    {
                        biggest_level  = stem_y_level;
                    }
                }
                
                // ---- triplet
                if (noteRenderInfo.m_draw_triplet_sign)
                {
                    //FIXME: remove these magic constants
                    const int triplet_level = noteRenderInfo.m_triplet_arc_level +
                                              (noteRenderInfo.m_triplet_show_above ? -6 : 4);
                    
                    if (triplet_level < smallest_level or smallest_level == -1)
                    {
                        smallest_level = triplet_level;
                    }
                    if (triplet_level > biggest_level)
                    {
                        biggest_level  = triplet_level;
                    }
                }
                
                // ---- beams
                if (noteRenderInfo.m_beam)
                {
                    const float to_level = noteRenderInfo.m_beam_to_level;
                    
                    if (to_level < smallest_level or smallest_level == -1)
                    {
                        smallest_level = to_level;
                    }
                    if (to_level > biggest_level)
                    {
                        biggest_level  = to_level;
                    }
                }
                
                //TODO: also handle overlapping clefs (e.g. 2 G clefs)
                
            }
        }
        
        // ---- Assign the values
        scoreData->extra_lines_above_g_score = 0;
        scoreData->extra_lines_under_g_score = 0;
        scoreData->extra_lines_above_f_score = 0;
        scoreData->extra_lines_under_f_score = 0;
        
        if (m_g_clef and not m_f_clef)
        {
            //std::cout << "G: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level)
            //                   << PRINT_VAR(g_clef_from_level) << PRINT_VAR(g_clef_to_level) << std::endl;

            if (smallest_level!=-1 and smallest_level < g_clef_from_level)
            {
                scoreData->extra_lines_above_g_score = (g_clef_from_level - smallest_level)/2;
            }
            if (biggest_level!=-1 and biggest_level > g_clef_to_level)
            {
                scoreData->extra_lines_under_g_score = (g_clef_to_level - biggest_level)/2;
            }
        }
        else if (m_f_clef and not m_g_clef)
        {
            //std::cout << "F: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level)
            //                   << PRINT_VAR(f_clef_from_level) << PRINT_VAR(f_clef_to_level) << std::endl;
            
            if (smallest_level!=-1 and smallest_level < f_clef_from_level)
            {
                scoreData->extra_lines_above_f_score = (f_clef_from_level - smallest_level)/2;
            }
            if (biggest_level!=-1 and biggest_level > f_clef_to_level)
            {
                scoreData->extra_lines_under_f_score = (f_clef_to_level - biggest_level)/2;
            }
        }
        else if (m_f_clef and m_g_clef)
        {
            //std::cout << "F: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level)
            //                   << PRINT_VAR(f_clef_from_level) << PRINT_VAR(f_clef_to_level) << std::endl;
            //std::cout << "G: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level)
            //                   << PRINT_VAR(g_clef_from_level) << PRINT_VAR(g_clef_to_level) << std::endl;
            
            if (smallest_level!=-1 and smallest_level < g_clef_from_level)
            {
                scoreData->extra_lines_above_g_score = (g_clef_from_level - smallest_level)/2;
            }
            if (biggest_level!=-1 and biggest_level > f_clef_to_level)
            {
                scoreData->extra_lines_under_f_score = (f_clef_to_level - biggest_level)/2;
            }
        }
        
        //std::cout << PRINT_VAR(scoreData->extra_lines_above_g_score) <<
        //             PRINT_VAR(scoreData->extra_lines_under_g_score) <<
        //             PRINT_VAR(scoreData->extra_lines_above_f_score) <<
        //             PRINT_VAR(scoreData->extra_lines_under_f_score) << std::endl;
        
        // Split space between both scores (one may need more than the other)
        scoreData->first_clef_proportion  = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) / 2.0f;
        scoreData->second_clef_proportion = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) / 2.0f;
        
        if (m_g_clef and m_f_clef and
           scoreData->extra_lines_above_g_score + scoreData->extra_lines_under_f_score != 0 /* unnecessary if nothing under/over scores*/)
        {
            const int total_G_level_count = abs(scoreData->extra_lines_above_g_score) + LINES_IN_A_SCORE;
            const int total_F_level_count = abs(scoreData->extra_lines_under_f_score) + LINES_IN_A_SCORE;

            const float total_height = total_G_level_count + total_F_level_count;
            
            // the forumla used below is :
            // given prop. = available space (minus margin) x fraction of the total lines that are in this clef
            scoreData->first_clef_proportion  = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) *
                    (float)(total_G_level_count) / (float)total_height;
            scoreData->second_clef_proportion = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) *
                    (float)(total_F_level_count) / (float)total_height;
        }

    }
    
    // -------------------------------------------------------------------------------------------
    
    namespace PrintStemParams
    {
        int g_stem_up_x_offset;
        //float stem_up_y_offset;
        int g_stem_down_x_offset;
        //float stem_down_y_offset;
        int g_note_x_shift = 0;
        
        int getStemX(const int tick, const PitchSign sign, const STEM stem_type,
                     PrintXConverter* converter)
        {
            const Range<int> noteX = converter->tickToX(tick);
            
            //const int accidentalShift = sign == PITCH_SIGN_NONE ? 0 : headRadius*1.85;
            
            if      (stem_type == STEM_UP)   return (noteX.to + g_stem_up_x_offset  );
            else if (stem_type == STEM_DOWN) return (noteX.to + g_stem_down_x_offset);
            else return -1;
        }
        
        int getStemX(const NoteRenderInfo& noteRenderInfo, PrintXConverter* converter)
        {
            return getStemX(noteRenderInfo.getTick(), noteRenderInfo.m_sign,
                            noteRenderInfo.m_stem_type, converter);
        }
    }
    using namespace PrintStemParams;
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::earlySetup(const int trackID, GraphicalTrack* gtrack)
    {
        Track* track = gtrack->getTrack();
        
        m_track = track;
        
        ScoreEditor* scoreEditor = gtrack->getScoreEditor();
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        MeasureData* measures = track->getSequence()->getMeasureData();
        const int measureAmount = measures->getMeasureAmount();
        
        // ---- find highest and lowest note we need to render in each measure
        for (int m=0; m<measureAmount; m++)
        {
            int highest_pitch = -1, lowest_pitch = -1;
            int biggest_level = -1, smallest_level = -1;
            
            const int from_tick = measures->firstTickInMeasure(m);
            const int to_tick = measures->lastTickInMeasure(m);
            
            const int firstNote = track->findFirstNoteInRange( from_tick, to_tick );
            const int lastNote  = track->findLastNoteInRange ( from_tick, to_tick );
            
            //std::cout << "Measure " << m << " : reading notes " << firstNote << " to " << lastNote << std::endl;
            
            for (int n=firstNote; n<=lastNote; n++)
            {
                if (n == -1) break; // will happen if line is empty
                
                const int pitch = track->getNotePitchID(n);
                const int level = converter->noteToLevel(track->getNote(n));
                if (pitch < highest_pitch or highest_pitch == -1)
                {
                    highest_pitch = pitch;
                    smallest_level = level;
                }
                if (pitch > lowest_pitch or lowest_pitch == -1)
                {
                    lowest_pitch  = pitch;
                    biggest_level  = level;
                }
            }
            
            PerMeasureInfo info;
            info.highest_pitch = highest_pitch;
            info.lowest_pitch = lowest_pitch;
            info.biggest_level = biggest_level;
            info.smallest_level = smallest_level;
            info.first_note = firstNote;
            info.last_note = lastNote;
            perMeasureInfo[trackID*5000+m] = info;
        } // end for all measures
        
        const int middle_c_level = converter->getScoreCenterCLevel(); //converter->getMiddleCLevel();
        
        
        m_g_clef = scoreEditor->isGClefEnabled();
        m_f_clef = scoreEditor->isFClefEnabled();
        
        // ---- Build score analyzers
        if (m_g_clef)
        {
            g_clef_analyser = new ScoreAnalyser(scoreEditor, middle_c_level-5);
            g_clef_analyser->setStemPivot(middle_c_level-5);
        }
        if (m_f_clef)
        {
            f_clef_analyser = new ScoreAnalyser(scoreEditor, middle_c_level-5);
            f_clef_analyser->setStemPivot(middle_c_level+6);
        }
        
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        // --- collect notes in the vector
        // by iterating through measures so ScoreAnalyser can prepare the score
        
        MeasureData* md = track->getSequence()->getMeasureData();
        
        if (LOGGING) printf("[ScorePrintable] earlySetup (%s) : gathering note list\n",
                            (const char*)track->getName().utf8_str());
        for (int m=0; m<measureAmount; m++)
        {
            ASSERT(perMeasureInfo.find(trackID*5000+m) != perMeasureInfo.end());
            PerMeasureInfo& measInfo = perMeasureInfo[trackID*5000+m];
            const int firstNote = measInfo.first_note;
            const int lastNote = measInfo.last_note;
            
            if (firstNote == -1 or lastNote == -1) continue; // empty measure
            
            for (int n=firstNote; n<=lastNote; n++)
            {
                PitchSign note_sign;
                const int noteLevel = converter->noteToLevel(track->getNote(n), &note_sign);
                
                if (noteLevel == -1) continue;
                const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
                const int tick = track->getNoteStartInMidiTicks(n);
                
                NoteRenderInfo currentNote = NoteRenderInfo::factory(tick, noteLevel, noteLength,
                                                                     note_sign, track->isNoteSelected(n),
                                                                     track->getNotePitchID(n), md);
                
                // add note to either G clef score or F clef score
                if (m_g_clef and not m_f_clef)
                {
                    //std::cout << "   G clef : Adding note at beat " << tick/960 << "\n";
                    g_clef_analyser->addToVector(currentNote);
                }
                else if (m_f_clef and not m_g_clef)
                {
                    //std::cout << "   F clef : Adding note at beat " << tick/960 << "\n";
                    f_clef_analyser->addToVector(currentNote);
                }
                else if (m_f_clef and m_g_clef)
                {
                    if (noteLevel < middle_c_level)
                    {
                        //std::cout << "   G clef : Adding note at beat " << tick/960 << "\n";
                        g_clef_analyser->addToVector(currentNote);
                    }
                    else
                    {
                        //std::cout << "   F clef : Adding note at beat " << tick/960 << "\n";
                        f_clef_analyser->addToVector(currentNote);
                    }
                }
            }
            
        }//next element
        
        if (m_g_clef)
        {
            g_clef_analyser->doneAdding();
        }
        if (m_f_clef)
        {
            f_clef_analyser->doneAdding();
        }
        
        // ---- Silences
        if (LOGGING) std::cout << "[ScorePrintable] early setup : gathering silences\n";
        if (m_f_clef)
        {
            m_silences_ticks = SilenceAnalyser::findSilences(track->getSequence(),
                                                             f_clef_analyser, 0, measureAmount-1,
                                                             -1 /* y not important at this point */ );
        }
        if (m_g_clef)
        {
            std::vector< SilenceAnalyser::SilenceInfo > g_clef_silences =
                    SilenceAnalyser::findSilences(track->getSequence(),
                                                  g_clef_analyser, 0, measureAmount-1,
                                                  -1 /* y not important at this point */ );
            
            // append the new items to the existing F clef items if any
            m_silences_ticks.insert(m_silences_ticks.end(), g_clef_silences.begin(), g_clef_silences.end());
        }
        
        /*
        printf("    %i notes in G clef (analyzer %x), %i in F clef (analyzer %x), this = %x\n",
               (int)g_clef_analyser->m_note_render_info.size(), (unsigned int)(void*)g_clef_analyser,
               (int)f_clef_analyser->m_note_render_info.size(), (unsigned int)(void*)f_clef_analyser,
               (unsigned int)(void*)this);
        */
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::backgroundDrawing(ClefRenderType clefType, ScoreAnalyser& analyser, LayoutLine& line,
                                           const GraphicalTrack* gtrack, wxDC& dc, wxGraphicsContext* grctx,
                                           const int extra_lines_above, const int extra_lines_under,
                                           const int x0, const int y0, const int x1, const int y1,
                                           bool show_measure_number, const int grandStaffCenterY)
    {
        // const Track* track = gtrack->getTrack();
        
        ASSERT(m_track == gtrack->getTrack());
                
        if (LOGGING) std::cout << "[ScorePrintable] ==== backgroundDrawing ==== \n";
        
        analyser.putInTimeOrder();
        
        /*
         {
         std::cout << "analyser contents {\n";
         const int noteAmount = analyser.m_note_render_info.size();
         for (int i=0; i<noteAmount; i++)
         {
         NoteRenderInfo& noteRenderInfo = analyser.m_note_render_info[i];
         std::cout << "    Note at beat " << noteRenderInfo.tick / 960 << "\n";
         }
         std::cout << "}\n";
         }
         */
        
        g_note_x_shift = HEAD_RADIUS;// shift to LEFT by a 'headRadius', since note will be drawn from the right of its area
                                     // and its center is the origin of the drawing
        						     // e.g. Drawing area of a note :
          						     // |     |
                                     // |     |  <-- stem at the right
                                     // |  ( )|
        						     //     ^ origin of the note here, in its center. so substract a radius from the right
        
        // since note is right-aligned, keep the stem at the right. go 10 towards the note to "blend" in it.
        g_stem_up_x_offset = -10;
        
        // since note is right-aligned. go towards the note to "blend" in it.
        g_stem_down_x_offset = -HEAD_RADIUS*2 + 3;
        
        /*
        const ScoreEditor* scoreEditor = gtrack->getScoreEditor();
        const ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        const int middle_c_level = converter->getScoreCenterCLevel();
        m_first_score_level = middle_c_level + (f_clef? 2 : -10);
        m_last_score_level  = m_first_score_level + 8;
        m_min_level         = m_first_score_level - extra_lines_above*2;        
        */
        
        // ------------ draw score background (horizontal lines) ------------
        //std::cout << "[ScorePrintable] rendering score background ==\n";
        dc.SetPen(  wxPen( wxColour(125,125,125), 7 ) );
        const int lineAmount = 5 + extra_lines_above + extra_lines_under;
        m_line_height = (float)(y1 - y0) / (float)(lineAmount-1);
        
        for (int lvl=m_first_score_level; lvl<=m_last_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
            
            //std::cout << "LEVEL_TO_Y(" << lvl << ") = " << y << std::endl;
            
            
            // DEBUG
            // dc.DrawText( wxString::Format(wxT("%i"), lvl), x0 - 120, y - 35 );
            // dc.DrawText( wxString::Format(wxT("%i"), y), x0 - 120, y - 35 );
        }
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::analyseAndDrawScore(ClefRenderType clefType, ScoreAnalyser& analyser, LayoutLine& line,
                                             const GraphicalTrack* gtrack, wxDC& dc, wxGraphicsContext* grctx,
                                             const int extra_lines_above, const int extra_lines_under,
                                             const int x0, const int y0, const int x1, const int y1,
                                             bool show_measure_number, const int grandStaffCenterY)
    {
        const Track* track = gtrack->getTrack();
        const MeasureData* md = track->getSequence()->getMeasureData();

        const int fromTick = md->firstTickInMeasure( line.getFirstMeasure() );
        const int toTick   = md->lastTickInMeasure ( line.getLastMeasure () );
        
        const bool f_clef = (clefType == F_CLEF_ALONE or clefType == F_CLEF_FROM_GRAND_STAFF);
        if (LOGGING) std::cout << "[ScorePrintable] ==== analyseAndDrawScore " << (f_clef ? "F" : "G") << " ==== \n";
        
        const int lineAmount = abs(extra_lines_above) + abs(extra_lines_under) + 5;
        m_line_height = (float)(y1 - y0) / (float)(lineAmount-1);
        
        
        /*
        //DEBUG
         
        // lines on score
        dc.SetPen(  wxPen( wxColour(255,0,0), 1 ) );
        for (int lvl=first_score_level; lvl<=last_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
            
            std::cout << "(1) Level " << lvl << " has Y " << y << "\n";
        }
        
        // lines above score
        dc.SetPen(  wxPen( wxColour(255,0,0), 7 ) );
        for (int lvl=first_score_level-extra_lines_above*2; lvl<first_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
        }
         
         // lines blow score
        for (int lvl=last_score_level+extra_lines_under*2; lvl>last_score_level; lvl-=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
        }
        dc.SetPen(  wxPen( wxColour(125,125,125), 7 ) );
        */
        
        // ------------ render vertical dividers and time signature changes ---------
        //std::cout << "[ScorePrintable] rendering vertical dividers & time sig changes ==\n";
        
        // const bool fclef = (clefType == F_CLEF_ALONE or clefType == F_CLEF_FROM_GRAND_STAFF);
        
        /*
        //DEBUG
        {
            const int noteAmount = analyser.m_note_render_info.size();
            
            for (int i=0; i<noteAmount; i++)
            {
                if (analyser.m_note_render_info[i].getTick() < fromTick) continue;
                if (analyser.m_note_render_info[i].getTick() >= toTick) break;
                
                NoteRenderInfo& noteRenderInfo = analyser.m_note_render_info[i];
                
                // DEBUG
                const Range<int> noteX = x_converter->tickToX(noteRenderInfo.getTick());
                dc.SetPen(  wxPen( wxColour(0,0,255), 2 ) );
                
                int lvl = round(LEVEL_TO_Y(noteRenderInfo.getBaseLevel()-1));
                std::cout << "(2) Level " << noteRenderInfo.getBaseLevel()-1 << " has Y " << lvl << "\n";

                dc.DrawLine( noteX.from, lvl, noteX.to, lvl );
                
                int lvl2 = round(LEVEL_TO_Y(noteRenderInfo.getBaseLevel()+1));
                std::cout << "(2) Level " << noteRenderInfo.getBaseLevel()+1 << " has Y " << lvl2 << "\n";

                dc.DrawLine( noteX.from, lvl2, noteX.to, lvl2 );

                dc.DrawLine( noteX.from, lvl, noteX.from, lvl2 );
                dc.DrawLine( noteX.to, lvl, noteX.to, lvl2 );

                
                dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                // END DEBUG
            }
        }
        //END DEBUG
        */
        
        int measure_dividers_from_y = LEVEL_TO_Y(m_first_score_level);
        int measure_dividers_to_y   = LEVEL_TO_Y(m_last_score_level);
        
        if (clefType == F_CLEF_FROM_GRAND_STAFF)
        {
            measure_dividers_from_y = grandStaffCenterY;
        }
        else if (clefType == G_CLEF_FROM_GRAND_STAFF)
        {
            measure_dividers_to_y = grandStaffCenterY;
        }
        
        // ---- draw end of line vertical line
        drawVerticalDivider(x1, measure_dividers_from_y, measure_dividers_to_y);


        const int elamount = line.getLayoutElementCount();
        for (int n=0; n<elamount; n++)
        {
            LayoutElement& currElem = line.getLayoutElement(n);
            
            drawVerticalDivider(&line.getLayoutElement(n), measure_dividers_from_y, measure_dividers_to_y);
            
            if (currElem.getType() == TIME_SIGNATURE_EL)
            {
                EditorPrintable::renderTimeSignatureChange(&currElem,
                                                           LEVEL_TO_Y(m_first_score_level),
                                                           LEVEL_TO_Y(m_last_score_level));
            }
            
            if (currElem.m_render_end_bar)
            {
                drawVerticalDivider(&currElem, measure_dividers_from_y, measure_dividers_to_y,
                                    true /* at end */);
            }
        }
        
        grctx->PushState();
        
        // ------------ line header if any ------------
        if (line.getLayoutElement(0).getType() == LINE_HEADER)
        {
            LayoutElement& headElement = line.getLayoutElement(0);
            
            //std::cout << "[ScorePrintable] rendering line header\n";
            if (not f_clef)
            {
                renderGClef(dc, grctx, headElement.getXFrom(),
                            LEVEL_TO_Y(m_last_score_level)+10,
                            LEVEL_TO_Y(m_last_score_level-4)-5);
            }
            else
            {
                renderFClef(dc, grctx, headElement.getXFrom(),
                            LEVEL_TO_Y(m_first_score_level),
                            LEVEL_TO_Y(m_first_score_level+3));
            }
            
            // render key signature next to staff
            // on which level to put the signs (0 is the level of the highest sign when all are shown)
            const unsigned short int sharp_sign_lvl[] = { 1, 4, 0, 3, 6, 2, 5 };
            const unsigned short int flat_sign_lvl[]  = { 3, 0, 4, 1, 5, 2, 6 };
            
            const int sharps = track->getKeySharpsAmount();
            const int flats  = track->getKeyFlatsAmount();
            ASSERT_E(line.getLayoutElementCount(),>=,0);
            
            if (sharps > 0 or flats > 0)
            {
                // FIXME: remove hardcoded values
                const int SPACE_FOR_CLEF = 300;
                const int KEY_MAX_ACCIDENTAL_SIZE = 50;
                
                int x_space_per_symbol = (headElement.getXTo() - headElement.getXFrom() -
                                          SPACE_FOR_CLEF - 50 /* some additional space */) / std::max(sharps, flats);
                if (x_space_per_symbol > KEY_MAX_ACCIDENTAL_SIZE) x_space_per_symbol = KEY_MAX_ACCIDENTAL_SIZE;
                
                for (int n=0; n<sharps; n++)
                {
                    const int level = m_first_score_level + (f_clef ? 1 : -1) + sharp_sign_lvl[n];
                    renderSharp( dc,
                                 headElement.getXFrom() + SPACE_FOR_CLEF + n*x_space_per_symbol +
                                    KEY_MAX_ACCIDENTAL_SIZE/2,
                                 LEVEL_TO_Y(level) );
                }
                for (int n=0; n<flats; n++)
                {
                    const int level = m_first_score_level + (f_clef ? 3 : 1) + flat_sign_lvl[n];
                    renderFlat( dc,
                                headElement.getXFrom() + SPACE_FOR_CLEF + n*x_space_per_symbol +
                                    KEY_MAX_ACCIDENTAL_SIZE/2,
                                LEVEL_TO_Y(level) );
                }
            }
            
            // also show ottava bassa/alta if relevant
            const int octave_shift = gtrack->getScoreEditor()->getScoreMidiConverter()->getOctaveShift();
            if (octave_shift > 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(m_first_score_level - 3);
                dc.DrawText(wxT("8va"), line.getLayoutElement(0).getXFrom()+200, y);
            }
            else if (octave_shift < 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(m_last_score_level);
                dc.DrawText(wxT("8vb"), line.getLayoutElement(0).getXFrom()+200, y);
            }
        }
        
        
        grctx->PopState();        
        
        // ---- draw base  
        for (int el=0; el<elamount; el++)
        {
            drawElementBase(line.getLayoutElement(el), line,
                            show_measure_number, y0, y1, measure_dividers_from_y, measure_dividers_to_y);
        }
        
        // ------------------ first part : basic unintelligent drawing -----------------
        // for all parts of score notes that can be rendered without using the ScoreAnalyser.
        
        
        {
            const int noteAmount = analyser.m_note_render_info.size();
                
            // ---- Draw small lines above/below score (ledger lines)
            //std::cout << "[ScorePrintable] rendering lines for notes out of score\n";
            for (int i=0; i<noteAmount; i++)
            {
                if (analyser.m_note_render_info[i].getTick() < fromTick) continue;
                if (analyser.m_note_render_info[i].getTick() >= toTick) break;
                
                NoteRenderInfo& noteRenderInfo = analyser.m_note_render_info[i];
                
                // draw small lines above score if needed
                if (noteRenderInfo.getLevel() < m_first_score_level-1)
                {
                    const Range<int> noteX = m_x_converter->tickToX(noteRenderInfo.getTick());
                    dc.SetPen(  wxPen( wxColour(125,125,125), 8 ) );
                    
                    for (int lvl=m_first_score_level-1; lvl>=noteRenderInfo.getLevel(); lvl --)
                    {
                        if ((m_first_score_level - lvl) % 2 == 0)
                        {
                            const int y = LEVEL_TO_Y(lvl);
                            //FIXME: not sure why I need to add HEAD_RADIUS/2 to make it look good
                            dc.DrawLine(noteX.from+HEAD_RADIUS/2, y, noteX.to+HEAD_RADIUS, y);
                        }
                    }
                }
                
                // draw small lines below score if needed
                if (noteRenderInfo.getLevel() > m_last_score_level+1)
                {
                    const Range<int> noteX = m_x_converter->tickToX(noteRenderInfo.getTick());
                    dc.SetPen(  wxPen( wxColour(125,125,125), 8 ) );
                    
                    for (int lvl=m_last_score_level+1; lvl<=noteRenderInfo.getLevel(); lvl++)
                    {
                        if ((lvl - m_last_score_level) % 2 == 0)
                        {
                            const int y = LEVEL_TO_Y(lvl);
                            dc.DrawLine(noteX.from + HEAD_RADIUS, y, noteX.to+HEAD_RADIUS, y);
                        }
                    }
                }
            } // end scope
            
            // ---- draw notes heads
            
            dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
            dc.SetBrush( *wxBLACK_BRUSH );
            
            if (LOGGING) std::cout << "[ScorePrintable] rendering note heads\n";
            for (int i=0; i<noteAmount; i++)
            {
                /*
                std::cout << "Checking note at " << analyser.m_note_render_info[i].tick
                          << " - beat " <<  analyser.m_note_render_info[i].tick/960
                          << " in measure " << analyser.m_note_render_info[i].measureBegin+1 << std::endl;
                */
                if (analyser.m_note_render_info[i].getTick() < fromTick) continue;
                if (analyser.m_note_render_info[i].getTick() >= toTick)  break;

                /*
                std::cout << "    Drawing note at " << analyser.m_note_render_info[i].tick
                          << " - beat " <<  analyser.m_note_render_info[i].tick/960
                          << " in measure " << analyser.m_note_render_info[i].measureBegin+1 << std::endl; 
                */
                NoteRenderInfo& noteRenderInfo = analyser.m_note_render_info[i];

                const Range<int> noteX = m_x_converter->tickToX(noteRenderInfo.getTick());
                
                // make sure we were given the requested size (TODO: fix and uncomment)
                //ASSERT_E(noteX.to - noteX.from, >=, HEAD_RADIUS*2 +
                //           (noteRenderInfo.sign == PITCH_SIGN_NONE ? 0 : MAX_ACCIDENTAL_SIZE) +
                //            NOTE_HEAD_MARGIN);
                                
                //ASSERT_E(noteRenderInfo.getBaseLevel(), >=, min_level);
                
                // draw head
                const int notey = LEVEL_TO_Y(noteRenderInfo.getBaseLevel());
                
                /** This coord is the CENTER of the note's head */
                wxPoint headLocation( noteX.to - g_note_x_shift,
                                      notey                    );
                
                if (noteRenderInfo.m_instant_hit)
                {
                    //FIXME: don't hardcode 36
                    dc.DrawText(wxT("X"), headLocation.x - 36, headLocation.y);
                }
                else
                {
                    RenderRoutines::drawNoteHead(dc, headLocation, noteRenderInfo.m_hollow_head);
                }
                noteRenderInfo.setY(notey + HEAD_RADIUS/2.0); // FIXME: why +HEAD_RADIUS/2.0 ?
                
                // draw dot if note is dotted
                if (noteRenderInfo.m_dotted)
                {     
                    wxPoint dotLocation( headLocation.x + HEAD_RADIUS + DOT_SHIFT_AFTER_NOTE_HEAD, notey+10 );
                    dc.DrawEllipse( dotLocation /* top left corner */, wxSize(DOT_SIZE, DOT_SIZE) );
                }
                
                /*
                // ---- DEBUG
                // ---- {
                dc.SetPen(  wxPen( wxColour(255,0,0), 2 ) );
                dc.DrawLine( headLocation.x - 10, headLocation.y, headLocation.x + 10, headLocation.y );
                const int lvlAbove = round(LEVEL_TO_Y(noteRenderInfo.getBaseLevel()-1));
                //const int lvlAt    = round(LEVEL_TO_Y(noteRenderInfo.getBaseLevel()));
                const int lvlBelow = round(LEVEL_TO_Y(noteRenderInfo.getBaseLevel()+1));
                
                std::cout << "(2) LEVEL_TO_Y(" << (noteRenderInfo.getBaseLevel()-1) << ") = " << lvlAbove << std::endl;
                std::cout << "(2) LEVEL_TO_Y(" << (noteRenderInfo.getBaseLevel()+1) << ") = " << lvlBelow << std::endl;

                
                dc.SetPen(  wxPen( wxColour(0,255,0), 2 ) );
                dc.DrawLine( headLocation.x - 10, lvlAbove, headLocation.x + 10, lvlAbove );
                //dc.DrawLine( headLocation.x - 10, lvlAt, headLocation.x + 10, lvlAt );
                dc.DrawLine( headLocation.x - 10, lvlBelow, headLocation.x + 10, lvlBelow );

                wxFont saved = dc.GetFont();
                dc.SetFont( wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC ,wxFONTWEIGHT_NORMAL) );
                dc.DrawText( wxString::Format(wxT("%i"), lvlAbove), headLocation.x - 50, lvlAbove );
                dc.DrawText( wxString::Format(wxT("%i"), lvlBelow), headLocation.x - 50, lvlBelow );
                dc.SetFont(saved);
                
                // ---- }
                */
                
                // draw sharpness sign if relevant
                if      (noteRenderInfo.m_sign == SHARP)
                {
                    renderSharp  ( dc, headLocation.x - HEAD_RADIUS - SHARP_WIDTH/2 - 10, noteRenderInfo.getY() - 10, true );
                }
                else if (noteRenderInfo.m_sign == FLAT)
                {
                    renderFlat   ( dc, headLocation.x - HEAD_RADIUS - FLAT_WIDTH - 10, noteRenderInfo.getY() - 10, true );
                }
                else if (noteRenderInfo.m_sign == NATURAL)
                {
                    renderNatural( dc, headLocation.x - HEAD_RADIUS, noteRenderInfo.getY() - 20, false );
                }
                
                // set pen/brush back, accidental routines might have changed them
                dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                
            } // next note
        } // end scope

        
        
        OwnerPtr<ScoreAnalyser> lineAnalyser;
        lineAnalyser = analyser.getSubset(fromTick, toTick);
        
        
        g_printable = this;
        
        // ---- render silences
        if (LOGGING) std::cout << "[ScorePrintable] rendering silences\n";
        
        const int first_measure = line.getFirstMeasure();
        const int last_measure  = line.getLastMeasure();
        
        global_dc = &dc;
        
#if wxCHECK_VERSION(2,9,1)
        gc = grctx;
#endif
        
        grctx->PushState();
        
        //FIXME: we already have collected all silence info in a vector... don't call the SilenceAnalyser again!
        if (f_clef)
        {
            const int silences_y = LEVEL_TO_Y(m_middle_c_level + 4);
            g_line_height = m_line_height;
            SilenceAnalyser::findSilences(track->getSequence(), &renderSilenceCallback, lineAnalyser,
                                          first_measure, last_measure, silences_y, m_x_converter);
        }
        else
        {
            const int silences_y = LEVEL_TO_Y(m_middle_c_level - 8);
            g_line_height = m_line_height;
            SilenceAnalyser::findSilences(track->getSequence(), &renderSilenceCallback, lineAnalyser,
                                          first_measure, last_measure, silences_y, m_x_converter);
        }
        
        grctx->PopState();
        
        
        // ------------------ second part : intelligent drawing of the rest -----------------
        if (LOGGING) std::cout << "[ScorePrintable] analyzing score\n";
        
        // analyse notes to know how to build the score
        lineAnalyser->analyseNoteInfo();
        
        if (LOGGING) std::cout << "[ScorePrintable] rendering note ornaments\n";
        // now that score was analysed, draw the remaining note bits
        const int noteAmount = lineAnalyser->m_note_render_info.size();
        for (int i=0; i<noteAmount; i++)
        {
            grctx->PushState();
            NoteRenderInfo& noteRenderInfo = lineAnalyser->m_note_render_info[i];
            
            dc.SetPen(  wxPen( wxColour(0,0,0), 15 ) );
            
            // draw stem
            if (noteRenderInfo.m_stem_type != STEM_NONE)
            {                
                const int stem_x = getStemX(noteRenderInfo, m_x_converter);
                dc.DrawLine( stem_x, LEVEL_TO_Y(noteRenderInfo.getStemOriginLevel()),
                             stem_x, LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo)) + 10    );
            }
            
            // draw flags
            if (not noteRenderInfo.m_instant_hit and noteRenderInfo.m_flag_amount > 0 and
                not noteRenderInfo.m_beam)
            {
                const int stem_end = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo)) + 10;
                const int flag_x_origin = getStemX(noteRenderInfo, m_x_converter);
                const int flag_step = (noteRenderInfo.m_stem_type==STEM_UP ? 7 : -7 );
                
                for (int n=0; n<noteRenderInfo.m_flag_amount; n++)
                {
                    const int flag_y = stem_end + n*flag_step*5;
                    const int orient = (noteRenderInfo.m_stem_type == STEM_UP ? 1 : -1 );
                    
                    RenderRoutines::drawFlag(&dc, grctx, flag_x_origin, flag_y, orient);
                    dc.SetPen(*wxBLACK_PEN);
                }
            }
            
            // ties
            if (noteRenderInfo.getTiedToTick() != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                const bool show_above = noteRenderInfo.isTieUp();
                const int base_y = LEVEL_TO_Y( noteRenderInfo.getStemOriginLevel() ) + (show_above ? - 30 : 60);
                
                const Range<int> noteLoc = m_x_converter->tickToX(noteRenderInfo.getTick());
                const int tiedXStart = (noteLoc.from + noteLoc.to)/2;
                
                //std::cout << "tied to tick " << noteRenderInfo.getTiedToTick() << " from " << noteRenderInfo.tick << std::endl;
                const Range<int> tiedToSymbolLocation = m_x_converter->tickToX(noteRenderInfo.getTiedToTick());

                const int tiedToPixel = tiedToSymbolLocation.to;
                //std::cout << "tied to pixel " << tiedToPixel << " from " << getStemX(noteRenderInfo) << std::endl;

                const int center_x = (tiedToPixel + tiedXStart)/2;
                const int radius_x = abs(tiedToPixel - tiedXStart)/2;
                RenderRoutines::renderArc(dc, center_x, base_y, radius_x, show_above ? -50 : 50);
            }
            
            // beam
            if (noteRenderInfo.m_beam)
            {
                dc.SetPen(  wxPen( wxColour(0,0,0), 10 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                
                const int beam_x1 = getStemX(noteRenderInfo, m_x_converter) - 2;
                int beam_y1       = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
                int beam_y2       = LEVEL_TO_Y(noteRenderInfo.m_beam_to_level);
                
                const int y_diff = (noteRenderInfo.m_stem_type == STEM_UP ? 55 : -55);
                                
                const int beam_x2 = getStemX(noteRenderInfo.m_beam_to_tick,
                                             noteRenderInfo.m_beam_to_sign,
                                             noteRenderInfo.m_stem_type,
                                             m_x_converter) + 2;
                
                for (int n=0; n<noteRenderInfo.m_flag_amount; n++)
                {
                    wxPoint points[] =
                    {
                        wxPoint(beam_x1, beam_y1),
                        wxPoint(beam_x2, beam_y2),
                        wxPoint(beam_x2, beam_y2 + 20),
                        wxPoint(beam_x1, beam_y1 + 20)
                    };
                    dc.DrawPolygon(4, points);
                    
                    beam_y1 += y_diff;
                    beam_y2 += y_diff;
                }
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
            }
            
            // triplet
            if (noteRenderInfo.m_draw_triplet_sign and noteRenderInfo.m_triplet_arc_tick_start != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                int triplet_arc_x_start = m_x_converter->tickToX(noteRenderInfo.m_triplet_arc_tick_start).to -
                                          HEAD_RADIUS;
                int triplet_arc_x_end = -1;
                
                if (noteRenderInfo.m_triplet_arc_tick_end != -1)
                {
                    const Range<int> arcEndSymbolLocation = m_x_converter->tickToX(noteRenderInfo.m_triplet_arc_tick_end);
                    
                    triplet_arc_x_end = arcEndSymbolLocation.to - HEAD_RADIUS;
                }
                
                const int center_x = (triplet_arc_x_end == -1 ?
                                      triplet_arc_x_start :
                                      (triplet_arc_x_start + triplet_arc_x_end)/2);
                const int radius_x = (triplet_arc_x_end == -1 or  triplet_arc_x_end == triplet_arc_x_start ?
                                      100 : (triplet_arc_x_end - triplet_arc_x_start)/2);
                
                const int base_y = LEVEL_TO_Y(noteRenderInfo.m_triplet_arc_level) +
                                  (noteRenderInfo.m_triplet_show_above ? -80 : 90);
                
                RenderRoutines::renderArc(dc, center_x, base_y, radius_x,
                                          noteRenderInfo.m_triplet_show_above ? -80 : 80);
                dc.SetTextForeground( wxColour(0,0,0) );
                
                // FIXME: use font size instead of hardcoded constant
                dc.DrawText(wxT("3"), center_x - 25,
                            base_y + (noteRenderInfo.m_triplet_show_above ? -75 : -20) );
            }
            
            grctx->PopState();
        } // next note
    }

    // -------------------------------------------------------------------------------------------
}
