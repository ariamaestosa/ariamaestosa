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

#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/file.h"
#include "wx/filename.h"

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "Midi/MeasureData.h"
#include "Editors/ScoreEditor.h"
#include "Editors/ScoreAnalyser.h"
#include "IO/IOUtils.h"
#include "Printing/ScorePrint.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

namespace AriaMaestosa
{
    /** Size of a note's head */
    const int HEAD_RADIUS = 36;

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
    
#if 0
#pragma mark -
#pragma mark ScoreData
#endif
    
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
    
    class ScoreData : public LineTrackRef::EditorData
    {
    public:            
        LEAK_CHECK();
        
        virtual ~ScoreData() {}
        
        //int middle_c_level, from_note, to_note;
        //bool g_clef, f_clef;
        int extra_lines_above_g_score;
        int extra_lines_under_g_score;
        int extra_lines_above_f_score;
        int extra_lines_under_f_score;
        float first_clef_proportion ;
        float second_clef_proportion;
        
        //OwnerPtr<ScoreAnalyser> g_clef_analyser;
        //OwnerPtr<ScoreAnalyser> f_clef_analyser;
    };
    
    // FIXME : find cleaner way to keep info per-track
    std::map< int /* trackID*5000 measure ID */, PerMeasureInfo > perMeasureInfo;

#if 0
#pragma mark -
#pragma mark PrintXConverter
#endif
    
    /**
     * An instance of this will be given to the score analyser. Having a separate x-coord-converter
     * allows it to do conversions between units without becoming context-dependent.
     */
    class PrintXConverter
    {
        ScorePrintable* parent;
        LayoutLine* line;
        int trackID;
    public:
        LEAK_CHECK();
        
        PrintXConverter(ScorePrintable* parent, LayoutLine* line, const int trackID)
        {
            PrintXConverter::parent = parent;
            PrintXConverter::line = line;
            PrintXConverter::trackID = trackID;
        }
        ~PrintXConverter(){}
        
        Range<int> tickToX(const int tick)
        {
            return parent->tickToX(trackID, *line, tick);
        }
        /*
         int getXTo(const int tick)
         {
         const int out = parent->tickToXLimit(trackID, *line, tick);
         std::cout << "out : " << tick << " --> " << out << std::endl;
         assert(out != -1);
         return out;
         }
         int getClosestXFromTick(const int tick)
         {
         return tickToX(parent->getClosestTickFrom(trackID, *line, tick));
         }
         */
    };
    
    // global (FIXME)
    PrintXConverter* x_converter;
    
#if 0
#pragma mark -
#pragma mark Render Routines
#endif
    
    /*
     * A few drawing routines
     */
    // -------------------------------------------------------------------------------------------
    /** Renders a 'sharp' symbol at the given coordinates */
    void renderSharp(wxDC& dc, const int x, const int y)
    {
        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        // horizontal lines
        dc.DrawLine( x-50/2, y,      x+50/2, y-20/2 );
        dc.DrawLine( x-50/2, y+40/2, x+50/2, y+20/2 );
        
        // vertical lines
        dc.DrawLine( x-20/2, y-30/2, x-20/2, y+60/2 );
        dc.DrawLine( x+20/2, y-40/2, x+20/2, y+50/2 );
    }
    
    // -------------------------------------------------------------------------------------------
    /** Renders a 'flat' symbol at the given coordinates */
    void renderFlat(wxDC& dc, const int x, const int y)
    {
        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        wxPoint points[] =
        {
            wxPoint(x,    y-15-40),
            wxPoint(x,    y+60-40),
            wxPoint(x+5,  y+60-40),
            wxPoint(x+25, y+30-40),
            wxPoint(x+15, y+20-40),
            wxPoint(x,    y+30-40)
        };
        dc.DrawSpline(6, points);
    }
    
    // -------------------------------------------------------------------------------------------
    /** Renders a 'natural' symbol at the given coordinates */
    void renderNatural(wxDC& dc, const int x, const int y)
    {
        dc.SetPen(  wxPen( wxColour(0,0,0), 6 ) );
        
        // horizontal lines
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y,      x+NATURAL_SIGN_WIDTH/2, y-20/2 );
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y+40/2, x+NATURAL_SIGN_WIDTH/2, y+20/2 );
        
        // vertical lines
        dc.DrawLine( x-NATURAL_SIGN_WIDTH/2, y+40/2, x-NATURAL_SIGN_WIDTH/2, y-60/2 );
        dc.DrawLine( x+NATURAL_SIGN_WIDTH/2, y-20/2, x+NATURAL_SIGN_WIDTH/2, y+80/2 );
    }
    
    // -------------------------------------------------------------------------------------------
    /** Renders a 'G clef' sign at the the given coordinates */
    void renderGClef(wxDC& dc, const int x, const float score_bottom, const float b_line_y)
    {
        static wxBitmap gclef( getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + wxT("keyG.png"), wxBITMAP_TYPE_PNG );
        
        const int b_on_image = 30;
        const int bottom_on_image = 49;
        const float scale = (score_bottom - b_line_y) / (float)(bottom_on_image - b_on_image);
        const int y = score_bottom - bottom_on_image*scale;
        
        wxBitmap scaled = wxBitmap(gclef.ConvertToImage().Scale(gclef.GetWidth()*scale, gclef.GetHeight()*scale));
        
        
        dc.DrawBitmap(scaled, x, y, true);
        
        /*
         dc.SetPen(  wxPen( wxColour(0,0,0), 7 ) );
         
         const float scale = abs(score_bottom - b_line_y) / 6.729;
         const int y = score_bottom - 3.776*scale;
         
         #define POINT(MX,MY) wxPoint( (int)round(x + MX*scale), (int)round(y - MY*scale) )
         
         wxPoint points[] =
         {
         POINT(4.531, -8.744), // bottom tail
         POINT(3.181, -9.748),
         POINT(4.946, -11.236),
         POINT(7.191, -10.123),
         POINT(7.577, -6.909),
         POINT(6.642, -1.336),
         POINT(4.941, 4.612),
         POINT(3.852, 10.668),
         POINT(4.527, 14.740),
         POINT(6.063, 16.144), // 10 - top
         POINT(7.227, 15.416),
         POINT(7.485, 11.511),
         POINT(5.365, 8.513),
         POINT(0.796, 3.155),
         POINT(1.062, -1.551), // 15
         POINT(5.739, -3.776), // main circle bottom
         POINT(9.401, 0.390),
         POINT(6.059, 2.953), // main circle top
         POINT(3.358, 1.260),
         POINT(3.908, -1.258), // 20
         POINT(4.503, -1.487) // G end
         };
         
         #undef POINT
         
         dc.DrawSpline(21, points);
         */
    }
    
    // -------------------------------------------------------------------------------------------
    /** Renders a 'F clef' sign at the the given coordinates */
    void renderFClef(wxDC& dc, const int x, const float score_top, const float e_line_y)
    {
        static wxBitmap fclef( getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + wxT("Fkey.png"), wxBITMAP_TYPE_PNG );
        
        const int e_on_image = 15;
        const float scale = (float)(e_line_y - score_top) / (float)e_on_image;
        
        wxBitmap scaled = wxBitmap(fclef.ConvertToImage().Scale(fclef.GetWidth()*scale, fclef.GetHeight()*scale));
        
        dc.DrawBitmap(scaled, x, score_top, true);
    }
    
    // -------------------------------------------------------------------------------------------

    // leave a pointer to the dc for the callback
    // FIXME find cleaner way
    wxDC* global_dc = NULL;
    ScorePrintable* g_printable = NULL;
    
    // leave height of lines for the renderSilence callback
    // FIXME find cleaner way
    int g_line_height=5;
    
    // -------------------------------------------------------------------------------------------
    
    void renderSilenceCallback(const int duration, const int tick, const int type, const int silences_y,
                               const bool triplet, const bool dotted, const int dot_delta_x,
                               const int dot_delta_y)
    {
        assert( global_dc != NULL);
        
        const Range<int> x = x_converter->tickToX(tick);
        
        //FIXME: don't rely on from_tick being negative!! what's that crap
        if (x.from < 0) return; // this part of score is not printed (e.g. is in a repetition)
        assert(x.to != -1);
        
        g_printable->drawSilence(global_dc, x, silences_y, g_line_height, type, triplet, dotted);
    }
    
#if 0
#pragma mark -
#pragma mark ScorePrintable (EditorPrintable common interface)
#endif
    
    // -------------------------------------------------------------------------------------------
    
    ScorePrintable::ScorePrintable() : EditorPrintable()
    {
        // std::cout << " *** setting global g_printable" << std::endl;
        g_printable = this;
    }
    
    // -------------------------------------------------------------------------------------------
    
    ScorePrintable::~ScorePrintable()
    {
    }
    
    // -------------------------------------------------------------------------------------------
      
    int ScorePrintable::calculateHeight(const int trackID, LineTrackRef& lineTrack, LayoutLine& line)
    {
        gatherVerticalSizingInfo(trackID, lineTrack, line);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(lineTrack.editor_data.raw_ptr);
        assert(scoreData != NULL);
        
        const int from_note = lineTrack.getFirstNote();
        const int to_note   = lineTrack.getLastNote();
        
        // check if empty
        // FIXME : if a note starts in the previous line and ends in this one, it won't be detected
        if (from_note == -1 || to_note == -1) return 0;
                
        std::cout <<
            PRINT_VAR(scoreData->extra_lines_under_g_score) <<
            PRINT_VAR(scoreData->extra_lines_above_g_score) <<
            PRINT_VAR(scoreData->extra_lines_under_f_score) <<
            PRINT_VAR(scoreData->extra_lines_above_f_score) << std::endl;
        
        int total = (g_clef ? 5 : 0) + (f_clef ? 5 : 0) + 
                abs(scoreData->extra_lines_under_g_score) +
                abs(scoreData->extra_lines_above_g_score) +
                abs(scoreData->extra_lines_under_f_score) +
                abs(scoreData->extra_lines_above_f_score);
        
        // if we have both scores, add the margin between them to the required space.
        if (g_clef and f_clef)
        {
            //FIXME: it's not too clear whether needed additional space should be returned
            //       as a function of levels??
            total = total + MARGIN_PROPORTION_BETWEEN_CLEFS*total;
        }
        
        return total;

    }
    
    // -------------------------------------------------------------------------------------------
#define VERBOSE 0
    
    void ScorePrintable::addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                      const MeasureTrackReference& trackRef,
                                      RelativePlacementManager& ticks_relative_position)
    {
        const int measureFromTick = measure.getFirstTick();
        const int measureToTick   = measure.getLastTick();
        
#if VERBOSE
        std::cout << "\naddingTicks(measure " << (measure.id+1) << ", from " << measureFromTick << ", to " << measureToTick << "\n{\n";
#endif
        
        const Track* track = trackRef.getConstTrack();
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        // ---- notes
        for (int clef=0; clef<2; clef++)
        {
            ScoreAnalyser* current_analyser;
            if (clef == 0)
            {
                if (g_clef) current_analyser = g_clef_analyser;
                else        continue;
            }
            else if (clef == 1)
            {
                if (f_clef) current_analyser = f_clef_analyser;
                else        continue;
            }
            else
            {
                assert(false);
                break;
            }
            assert( current_analyser != NULL );
            
            const int noteAmount = current_analyser->noteRenderInfo.size();
            
            // find shortest note
            int shortest = -1;
            for(int n=0; n<noteAmount; n++)
            {
                const int tick = current_analyser->noteRenderInfo[n].tick;
                if (tick < measureFromTick or tick >= measureToTick) continue;
                
                if (current_analyser->noteRenderInfo[n].tick_length < shortest or shortest == -1)
                {
                    shortest = current_analyser->noteRenderInfo[n].tick_length;
                }
            }
            
            for (int n=0; n<noteAmount; n++)
            {
                const int tick = current_analyser->noteRenderInfo[n].tick;
                if (tick < measureFromTick or tick >= measureToTick) continue;
                
                const int tickTo = tick + current_analyser->noteRenderInfo[n].tick_length;

#if VERBOSE
                std::cout << "    Adding tick " << tick << " to list" << std::endl;
#endif
                
                if (current_analyser->noteRenderInfo[n].sign != PITCH_SIGN_NONE)
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
#endif
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::drawTrack(const int trackID, const LineTrackRef& currentTrack,
                                   LayoutLine& currentLine, wxDC& dc)
    {
        const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
        assert(trackCoords != NULL);
        
        assertExpr(trackCoords->y0,>,0);
        assertExpr(trackCoords->y1,>,0);
        assertExpr(trackCoords->y0,<,50000);
        assertExpr(trackCoords->y1,<,50000);
        setCurrentDC(&dc);
        
        
        std::cout << "ScorePrintable size : " << trackCoords->x0 << ", " << trackCoords->y0 << " to "
                  << trackCoords->x1 << ", " << trackCoords->y1 << std::endl;
        
        x_converter = new PrintXConverter(this, &currentLine, trackID);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(currentTrack.editor_data.raw_ptr);
        
        // if we have only one clef, give it the full space.
        // if we have two, split the space between both
        int g_clef_y_from = -1, g_clef_y_to = -1;
        int f_clef_y_from = -1, f_clef_y_to = -1;
        
        if (g_clef and not f_clef)
        {
            g_clef_y_from = trackCoords->y0;
            g_clef_y_to   = trackCoords->y1;
        }
        else if (f_clef and not g_clef)
        {
            f_clef_y_from = trackCoords->y0;
            f_clef_y_to   = trackCoords->y1;
        }
        else if (f_clef and g_clef)
        {
            g_clef_y_from = trackCoords->y0;
            g_clef_y_to   = trackCoords->y0 +
                            (int)round((trackCoords->y1 - trackCoords->y0)*scoreData->first_clef_proportion);
            f_clef_y_from = trackCoords->y0 +
                            (int)round((trackCoords->y1 - trackCoords->y0)*(1-scoreData->second_clef_proportion));
            f_clef_y_to   = trackCoords->y1;
        }
        else
        {
            assert(false);
        }
        
        
        
        // iterate through layout elements (this also inits them)
        // FIXME : not too clean to init them at this point
        LayoutElement* currentElement;
        std::cout << "\nLayout elements X coords :\n";
        
        const int elementAmount = currentLine.getLayoutElementCount();
        for (int el=0; el<elementAmount; el++)
        {
            currentElement = continueWithNextElement(trackID, currentLine, el);
            std::cout << "    Layout element from x=" << currentElement->getXFrom() << " to x=" << currentElement->getXTo() << std::endl;
        }//next element
        std::cout << std::endl;
        
        g_printable = this;
        
        if (g_clef)
        {
            analyseAndDrawScore(false /*G*/, *g_clef_analyser, currentLine, currentTrack.m_track,
                                dc, abs(scoreData->extra_lines_above_g_score),
                                abs(scoreData->extra_lines_under_g_score),
                                trackCoords->x0, g_clef_y_from, trackCoords->x1, g_clef_y_to,
                                currentTrack.showMeasureNumber());
        }
        
        if (f_clef)
        {
            analyseAndDrawScore(true /*F*/, *f_clef_analyser, currentLine, currentTrack.m_track,
                                dc, abs(scoreData->extra_lines_above_f_score),
                                abs(scoreData->extra_lines_under_f_score),
                                trackCoords->x0, f_clef_y_from, trackCoords->x1, f_clef_y_to,
                                (g_clef ? false : currentTrack.showMeasureNumber()) /* if we have both keys don't show twice */);
        }
        
        delete x_converter;
        x_converter = NULL;
        
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
        
        const Track* track = lineTrack.m_track;
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        // ---- Determine the y level of the highest and the lowest note
        const int fromMeasure = line.getFirstMeasure();
        const int lastMeasure = line.getLastMeasure();

        int biggest_level = -1, smallest_level = -1;
        
        for (int m=fromMeasure; m<=lastMeasure; m++)
        {
            //  FIXME : see other fixme above, this * 500 + m trick is stupid and ugly
            PerMeasureInfo& info = perMeasureInfo[trackID*5000+m];
                        
            if (info.biggest_level > biggest_level or biggest_level == -1)
            {
                biggest_level = info.biggest_level;
            }
            if (info.smallest_level < smallest_level or smallest_level == -1)
            {
                smallest_level = info.smallest_level;
            }
        }

        // ---- get some values useful for later
        middle_c_level = converter->getScoreCenterCLevel(); //converter->getMiddleCLevel();
        
        const int g_clef_from_level = middle_c_level-10;
        const int g_clef_to_level   = middle_c_level-2;
        const int f_clef_from_level = middle_c_level+2;
        const int f_clef_to_level   = middle_c_level+10;
        
        g_clef = scoreEditor->isGClefEnabled();
        f_clef = scoreEditor->isFClefEnabled();
        
        const int fromTick = getMeasureData()->firstTickInMeasure( line.getFirstMeasure() );
        const int toTick   = getMeasureData()->lastTickInMeasure ( line.getLastMeasure() );

        // ---- check if some signs (stems, triplet signs, etc.) go out of bounds
        for (int n=0; n<2; n++) // 0 is G clef, 1 is F clef
        {
            if (n == 0 and not g_clef) continue;
            if (n == 1 and not f_clef) continue;
            
            // analyse notes. this analysis will be used to determine is some things go out of the track
            // verticall, and will be thrown away after [FIXME] (it will be analysed again when it's time to render)
            ScoreAnalyser* analyser = NULL;
            if      (n == 0) analyser = g_clef_analyser;
            else if (n == 1) analyser = f_clef_analyser;
            else             assert(false);
            
            assert(analyser != NULL);

            OwnerPtr<ScoreAnalyser> lineAnalyser;
            lineAnalyser = analyser->getSubset(fromTick, toTick);
            lineAnalyser->analyseNoteInfo();
            
            const int noteAmount = lineAnalyser->noteRenderInfo.size();
            for (int i=0; i<noteAmount; i++)
            {
                NoteRenderInfo& noteRenderInfo = lineAnalyser->noteRenderInfo[i];
                
                // --- stem
                if (noteRenderInfo.draw_stem and noteRenderInfo.stem_type != STEM_NONE)
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
                if (noteRenderInfo.draw_triplet_sign)
                {
                    //FIXME: remove these magic constants
                    const int triplet_level = noteRenderInfo.triplet_arc_level +
                                              (noteRenderInfo.triplet_show_above ? -6 : 4);
                    
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
                if (noteRenderInfo.beam)
                {
                    const float to_level = noteRenderInfo.beam_to_level;
                    
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
        
        if (g_clef and not f_clef)
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
        else if (f_clef and not g_clef)
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
        else if (f_clef and g_clef)
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
        
        std::cout << PRINT_VAR(scoreData->extra_lines_above_g_score) <<
                     PRINT_VAR(scoreData->extra_lines_under_g_score) <<
                     PRINT_VAR(scoreData->extra_lines_above_f_score) <<
                     PRINT_VAR(scoreData->extra_lines_under_f_score) << std::endl;
        
        // Split space between both scores (one may need more than the other)
        scoreData->first_clef_proportion  = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) / 2.0f;
        scoreData->second_clef_proportion = (1.0f - MARGIN_PROPORTION_BETWEEN_CLEFS) / 2.0f;
        
        if (g_clef and f_clef and
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
        int stem_up_x_offset;
        float stem_up_y_offset;
        int stem_down_x_offset;
        float stem_down_y_offset;
        int note_x_shift = 0;
        
        const int getStemX(const int tick, const PitchSign sign, const STEM stem_type)
        {
            const Range<int> noteX = x_converter->tickToX(tick);
            
            //const int accidentalShift = sign == PITCH_SIGN_NONE ? 0 : headRadius*1.85;
            
            if      (stem_type == STEM_UP)   return (noteX.to + stem_up_x_offset  );
            else if (stem_type == STEM_DOWN) return (noteX.to + stem_down_x_offset);
            else return -1;
        }
        const int getStemX(const NoteRenderInfo& noteRenderInfo)
        {
            return getStemX(noteRenderInfo.tick, noteRenderInfo.sign, noteRenderInfo.stem_type);
        }
    }
    using namespace PrintStemParams;
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::earlySetup(const int trackID, Track* track)
    {
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        MeasureData* measures = getMeasureData();
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
                if (pitch < highest_pitch || highest_pitch == -1)
                {
                    highest_pitch = pitch;
                    smallest_level = level;
                }
                if (pitch > lowest_pitch  ||  lowest_pitch == -1)
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
        
        
        g_clef = scoreEditor->isGClefEnabled();
        f_clef = scoreEditor->isFClefEnabled();
        
        // ---- Build score analyzers
        if (g_clef)
        {
            g_clef_analyser = new ScoreAnalyser(scoreEditor, middle_c_level-5);
            g_clef_analyser->setStemPivot(middle_c_level-5);
        }
        if (f_clef)
        {
            f_clef_analyser = new ScoreAnalyser(scoreEditor, middle_c_level-5);
            f_clef_analyser->setStemPivot(middle_c_level+6);
        }
        
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        // --- collect notes in the vector
        // by iterating through measures so ScoreAnalyser can prepare the score
        
        std::cout << " == gathering note list ==\n";
        for (int m=0; m<measureAmount; m++)
        {
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
                
                NoteRenderInfo currentNote(tick, noteLevel, noteLength, note_sign,
                                           track->isNoteSelected(n), track->getNotePitchID(n));
                
                // add note to either G clef score or F clef score
                if (g_clef and not f_clef)
                {
                    //std::cout << "   G clef : Adding note at beat " << tick/960 << "\n";
                    g_clef_analyser->addToVector(currentNote);
                }
                else if (f_clef and not g_clef)
                {
                    //std::cout << "   F clef : Adding note at beat " << tick/960 << "\n";
                    f_clef_analyser->addToVector(currentNote);
                }
                else if (f_clef and g_clef)
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
        
        // ---- Silences
        std::cout << " == gathering silences ==\n";
        if (f_clef)
        {
            m_silences_ticks = SilenceAnalyser::findSilences( f_clef_analyser, 0, measureAmount-1,
                                                             -1 /* y not important at this point */ );
        }
        if (g_clef)
        {
            m_silences_ticks = SilenceAnalyser::findSilences( g_clef_analyser, 0, measureAmount-1,
                                                            -1 /* y not important at this point */ );
        }
        
        
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::analyseAndDrawScore(bool f_clef, ScoreAnalyser& analyser, LayoutLine& line,
                                             const Track* track, wxDC& dc,
                                             const int extra_lines_above, const int extra_lines_under,
                                             const int x0, const int y0, const int x1, const int y1,
                                             bool show_measure_number)
    {
        std::cout << "==========================\n    analyseAndDrawScore " << (f_clef ? "F" : "G")
                  << "\n==========================\n\n";
        
        analyser.putInTimeOrder();
        
        /*
        {
            std::cout << "analyser contents {\n";
            const int noteAmount = analyser.noteRenderInfo.size();
            for (int i=0; i<noteAmount; i++)
            {
                NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
                std::cout << "    Note at beat " << noteRenderInfo.tick / 960 << "\n";
            }
            std::cout << "}\n";
        }
        */
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        const int middle_c_level = converter->getScoreCenterCLevel();
        const int first_score_level = middle_c_level + (f_clef? 2 : -10);
        const int last_score_level = first_score_level + 8;
        const int min_level =  first_score_level - extra_lines_above*2;

        note_x_shift = HEAD_RADIUS;// shift to LEFT by a 'headRadius', since note will be drawn from the right of its area
                                   // and its center is the origin of the drawing
        						   // e.g. Drawing area of a note :
          						   // |     |
                                   // |     |  <-- stem at the right
                                   // |  ( )|
        						   //     ^ origin of the note here, in its center. so substract a radius from the right
        
        stem_up_x_offset = -10; // since note is right-aligned, keep the stem at the right. go 10 towards the note to "blend" in it.
        stem_up_y_offset = 0;
        stem_down_x_offset = -HEAD_RADIUS*2 + 3; // since note is right-aligned. go 4 towards the note to "blend" in it.
        stem_down_y_offset = 0;
                
#define LEVEL_TO_Y( lvl ) y0 + 1 + lineHeight*0.5*(lvl - min_level)
                
        
        // ------------ draw score background (horizontal lines) ------------
        std::cout << " == rendering score background ==\n";
        dc.SetPen(  wxPen( wxColour(125,125,125), 7 ) );
        const int lineAmount = 5 + extra_lines_above + extra_lines_under;
        const float lineHeight = (float)(y1 - y0) / (float)(lineAmount-1);
        
        for (int lvl=first_score_level; lvl<=last_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
            
            // DEBUG
            // dc.DrawText( wxString::Format(wxT("%i"), lvl), x0 - 120, y - 35 );
        }
        
        /*
        //DEBUG
        dc.SetPen(  wxPen( wxColour(255,0,0), 7 ) );
        for (int lvl=first_score_level-extra_lines_above*2; lvl<first_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
        }
        for (int lvl=last_score_level+extra_lines_under*2; lvl>last_score_level; lvl-=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
        }
        dc.SetPen(  wxPen( wxColour(125,125,125), 7 ) );
        */
        
        // ------------ render vertical dividers and time signature changes ---------
        std::cout << " == rendering vertical dividers & time sig changes ==\n";
        
        const int measure_dividers_from_y = LEVEL_TO_Y(first_score_level);
        const int measure_dividers_to_y   = LEVEL_TO_Y(last_score_level);
        
        // draw end of line vertical line
        drawVerticalDivider(x1, measure_dividers_from_y, measure_dividers_to_y);


        const int elamount = line.getLayoutElementCount();
        for (int n=0; n<elamount; n++)
        {
            drawVerticalDivider(&line.getLayoutElement(n), measure_dividers_from_y, measure_dividers_to_y);
            
            if (line.getLayoutElement(n).getType() == TIME_SIGNATURE_EL)
            {
                EditorPrintable::renderTimeSignatureChange(&line.getLayoutElement(n),
                                                           LEVEL_TO_Y(first_score_level),
                                                           LEVEL_TO_Y(last_score_level));
            }
        }
        
        // ------------ line header if any ------------
        if (line.getLayoutElement(0).getType() == LINE_HEADER)
        {
            LayoutElement& headElement = line.getLayoutElement(0);
            
            std::cout << " == rendering line header ==\n";
            if (!f_clef)
            {
                renderGClef(dc, headElement.getXFrom(),
                            LEVEL_TO_Y(last_score_level)+10,
                            LEVEL_TO_Y(last_score_level-4)-5);
            }
            else
            {
                renderFClef(dc, headElement.getXFrom(),
                            LEVEL_TO_Y(first_score_level),
                            LEVEL_TO_Y(first_score_level+3));
            }
            
            // render key signature next to staff
            // on which level to put the signs (0 is the level of the highest sign when all are shown)
            const unsigned short int sharp_sign_lvl[] = { 1, 4, 0, 3, 6, 2, 5 };
            const unsigned short int flat_sign_lvl[]  = { 3, 0, 4, 1, 5, 2, 6 };
            
            const int sharps = track->getKeySharpsAmount();
            const int flats  = track->getKeyFlatsAmount();
            assertExpr(line.getLayoutElementCount(),>=,0);
            
            if (sharps > 0 or flats > 0)
            {
                // FIXME: remove hardcoded values
                const int SPACE_FOR_CLEF = 300;
                const int MAX_ACCIDENTAL_SIZE = 50;
                
                int x_space_per_symbol = (headElement.getXTo() - headElement.getXFrom() -
                                          SPACE_FOR_CLEF - 50 /* some additional space */) / std::max(sharps, flats);
                if (x_space_per_symbol > MAX_ACCIDENTAL_SIZE) x_space_per_symbol = MAX_ACCIDENTAL_SIZE;
                
                for (int n=0; n<sharps; n++)
                {
                    const int level = first_score_level + (f_clef ? 1 : -1) + sharp_sign_lvl[n];
                    renderSharp( dc,
                                 headElement.getXFrom() + SPACE_FOR_CLEF + n*x_space_per_symbol + MAX_ACCIDENTAL_SIZE/2,
                                 LEVEL_TO_Y(level) );
                }
                for (int n=0; n<flats; n++)
                {
                    const int level = first_score_level + (f_clef ? 3 : 1) + flat_sign_lvl[n];
                    renderFlat( dc,
                                headElement.getXFrom() + SPACE_FOR_CLEF + n*x_space_per_symbol + MAX_ACCIDENTAL_SIZE/2,
                                LEVEL_TO_Y(level) );
                }
            }
            
            // also show ottava bassa/alta if relevant
            const int octave_shift = track->graphics->scoreEditor->getScoreMidiConverter()->getOctaveShift(); // FIXME - woot
            if (octave_shift > 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(first_score_level - 3);
                dc.DrawText(wxT("8va"), line.getLayoutElement(0).getXFrom()+200, y);
            }
            else if (octave_shift < 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(last_score_level);
                dc.DrawText(wxT("8vb"), line.getLayoutElement(0).getXFrom()+200, y);
            }
        }
        
        // ------------------ first part : basic unintelligent drawing -----------------
        // for all parts of score notes that can be rendered without using the ScoreAnalyser.
        
        const int fromTick = getMeasureData()->firstTickInMeasure( line.getFirstMeasure() );
        const int toTick   = getMeasureData()->lastTickInMeasure ( line.getLastMeasure () );
        
        {
            const int noteAmount = analyser.noteRenderInfo.size();
                
            // ---- Draw small lines above/below score
            std::cout << " == rendering lines for notes out of score ==\n";
            for (int i=0; i<noteAmount; i++)
            {
                if (analyser.noteRenderInfo[i].tick < fromTick) continue;
                if (analyser.noteRenderInfo[i].tick >= toTick) break;
                
                NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
                
                // draw small lines above score if needed
                if (noteRenderInfo.level < first_score_level-1)
                {
                    const Range<int> noteX = x_converter->tickToX(noteRenderInfo.tick);
                    dc.SetPen(  wxPen( wxColour(125,125,125), 8 ) );
                    
                    for(int lvl=first_score_level-2; lvl>noteRenderInfo.level+noteRenderInfo.level%2-2; lvl -= 2)
                    {
                        const int y = LEVEL_TO_Y(lvl);
                        dc.DrawLine(noteX.from+HEAD_RADIUS, y, noteX.to+HEAD_RADIUS, y);
                    }
                }
                
                // draw small lines below score if needed
                if (noteRenderInfo.level > last_score_level+1)
                {
                    const Range<int> noteX = x_converter->tickToX(noteRenderInfo.tick);
                    dc.SetPen(  wxPen( wxColour(125,125,125), 8 ) );
                    
                    for(int lvl=last_score_level+2; lvl<noteRenderInfo.level-noteRenderInfo.level%2+2; lvl += 2)
                    {
                        const int y = LEVEL_TO_Y(lvl);
                        dc.DrawLine(noteX.from+HEAD_RADIUS, y, noteX.to+HEAD_RADIUS, y);
                    }
                }
            } // end scope
            
            // ---- draw notes heads
            
            dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
            dc.SetBrush( *wxBLACK_BRUSH );
            
            std::cout << " == rendering note heads ==\n";
            for (int i=0; i<noteAmount; i++)
            {
                /*
                std::cout << "Checking note at " << analyser.noteRenderInfo[i].tick
                          << " - beat " <<  analyser.noteRenderInfo[i].tick/960
                          << " in measure " << analyser.noteRenderInfo[i].measureBegin+1 << std::endl;
                */
                if (analyser.noteRenderInfo[i].tick < fromTick) continue;
                if (analyser.noteRenderInfo[i].tick >= toTick)  break;

                /*
                std::cout << "    Drawing note at " << analyser.noteRenderInfo[i].tick
                          << " - beat " <<  analyser.noteRenderInfo[i].tick/960
                          << " in measure " << analyser.noteRenderInfo[i].measureBegin+1 << std::endl; 
                */
                NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];

                const Range<int> noteX = x_converter->tickToX(noteRenderInfo.tick);

                // Amount by which to shift the accidental sign, from 'noteX'
                // FIXME: what is that??
                const int accidentalShift = (noteRenderInfo.sign == PITCH_SIGN_NONE ? 0 : HEAD_RADIUS*1.85);
    
                // draw head
                const int notey = LEVEL_TO_Y(noteRenderInfo.getBaseLevel());
                
                /** This coord is the CENTER of the note's head */
                wxPoint headLocation( noteX.to - note_x_shift, // this is the center of the note
                                      notey-(HEAD_RADIUS-5)/2.0);
                
                if (noteRenderInfo.instant_hit)
                {
                    //FIXME: don't hardcode 36
                    dc.DrawText(wxT("X"), headLocation.x - 36, headLocation.y);
                }
                else
                {
                    const int cx = headLocation.x + (noteRenderInfo.hollow_head ? -2 : 0); // FIXME: the -2 is a hack for the head to blend in the stem
                    const int cy = headLocation.y;
                    wxPoint points[25];
                    for (int n=0; n<25; n++)
                    {
                        // FIXME - instead of always substracting to radius, just make it smaller...
                        const float angle = n/25.0*6.283185f /* 2*PI */;
                        points[n] = wxPoint( cx + (HEAD_RADIUS-5)*cos(angle),
                                             cy + HEAD_RADIUS/2 + (HEAD_RADIUS - 14)*sin(angle) - HEAD_RADIUS*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f );
                    }

                    if (noteRenderInfo.hollow_head) dc.DrawSpline(25, points);
                    else                            dc.DrawPolygon(25, points, -3);
                    
                }
                noteRenderInfo.setY(notey+HEAD_RADIUS/2.0);
                
                // draw dot if note is dotted
                if (noteRenderInfo.dotted)
                {
                    wxPoint headLocation( headLocation.x + HEAD_RADIUS, notey+10 );
                    dc.DrawEllipse( headLocation /* top left corner */, wxSize(DOT_SIZE, DOT_SIZE) );
                }
                
                // draw sharpness sign if relevant
                if      (noteRenderInfo.sign == SHARP)   renderSharp  ( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 15  );
                else if (noteRenderInfo.sign == FLAT)    renderFlat   ( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 15  );
                else if (noteRenderInfo.sign == NATURAL) renderNatural( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 20  );
                
                // set pen/brush back, accidental routines might have changed them
                dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                
            } // next note
        } // end scope

        // ------------------ second part : intelligent drawing of the rest -----------------
        std::cout << " == analyzing score ==\n";
        
        // analyse notes to know how to build the score
        OwnerPtr<ScoreAnalyser> lineAnalyser;
        lineAnalyser = analyser.getSubset(fromTick, toTick);
        lineAnalyser->analyseNoteInfo();
        
        std::cout << " == rendering note ornaments ==\n";
        // now that score was analysed, draw the remaining note bits
        const int noteAmount = lineAnalyser->noteRenderInfo.size();
        for (int i=0; i<noteAmount; i++)
        {
            NoteRenderInfo& noteRenderInfo = lineAnalyser->noteRenderInfo[i];
            
            dc.SetPen(  wxPen( wxColour(0,0,0), 15 ) );
            
            // draw stem
            if (noteRenderInfo.stem_type != STEM_NONE)
            {                
                const int stem_x = getStemX(noteRenderInfo);
                dc.DrawLine( stem_x, LEVEL_TO_Y(noteRenderInfo.getStemOriginLevel()),
                             stem_x, LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo)) + 10    );
            }
            
            // draw flags
            if (not noteRenderInfo.instant_hit and noteRenderInfo.flag_amount>0 and not noteRenderInfo.beam)
            {
                const int stem_end = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo)) + 10;
                const int flag_x_origin = getStemX(noteRenderInfo);
                const int flag_step = (noteRenderInfo.stem_type==STEM_UP ? 7 : -7 );
                
                for(int n=0; n<noteRenderInfo.flag_amount; n++)
                {
                    const int flag_y = stem_end + n*flag_step*5;
                    const int orient = (noteRenderInfo.stem_type==STEM_UP ? 1 : -1 );
                    
                    wxPoint points[] =
                    {
                        wxPoint(flag_x_origin, flag_y),
                        wxPoint(flag_x_origin + 30/2,  flag_y + orient*60/2),
                        wxPoint(flag_x_origin + 110/2, flag_y + orient*110/2),
                        wxPoint(flag_x_origin + 90/2,  flag_y + orient*150/2)
                    };
                    dc.DrawSpline(4, points);
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
                
                const Range<int> noteLoc = x_converter->tickToX(noteRenderInfo.tick);
                const int tiedXStart = (noteLoc.from + noteLoc.to)/2;
                
                //std::cout << "tied to tick " << noteRenderInfo.getTiedToTick() << " from " << noteRenderInfo.tick << std::endl;
                const Range<int> tiedToSymbolLocation = x_converter->tickToX(noteRenderInfo.getTiedToTick());

                const int tiedToPixel = tiedToSymbolLocation.to;
                //std::cout << "tied to pixel " << tiedToPixel << " from " << getStemX(noteRenderInfo) << std::endl;

                const int center_x = (tiedToPixel + tiedXStart)/2;
                const int radius_x = abs(tiedToPixel - tiedXStart)/2;
                renderArc(dc, center_x, base_y, radius_x, show_above ? -50 : 50);
            }
            
            // beam
            if (noteRenderInfo.beam)
            {
                dc.SetPen(  wxPen( wxColour(0,0,0), 10 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                
                const int x1 = getStemX(noteRenderInfo) - 2;
                int y1       = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
                int y2       = LEVEL_TO_Y(noteRenderInfo.beam_to_level);
                
                const int y_diff = (noteRenderInfo.stem_type == STEM_UP ? 55 : -55);
                                
                const int beam_to_x = getStemX(noteRenderInfo.beam_to_tick, noteRenderInfo.beam_to_sign, noteRenderInfo.stem_type) + 2;
                
                for(int n=0; n<noteRenderInfo.flag_amount; n++)
                {
                    
                    
                    wxPoint points[] =
                    {
                        wxPoint(x1, y1),
                        wxPoint(beam_to_x, y2),
                        wxPoint(beam_to_x, y2+20),
                        wxPoint(x1, y1+20)
                    };
                    dc.DrawPolygon(4, points);
                    
                    y1 += y_diff;
                    y2 += y_diff;
                }
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
            }
            
            // triplet
            if (noteRenderInfo.draw_triplet_sign and noteRenderInfo.triplet_arc_tick_start != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                int triplet_arc_x_start = x_converter->tickToX(noteRenderInfo.triplet_arc_tick_start).to - HEAD_RADIUS;
                int triplet_arc_x_end = -1;
                
                if (noteRenderInfo.triplet_arc_tick_end != -1)
                {
                    const Range<int> arcEndSymbolLocation = x_converter->tickToX(noteRenderInfo.triplet_arc_tick_end);
                    
                    triplet_arc_x_end = arcEndSymbolLocation.to - HEAD_RADIUS;
                }
                
                const int center_x = (triplet_arc_x_end == -1 ?
                                      triplet_arc_x_start :
                                      (triplet_arc_x_start + triplet_arc_x_end)/2);
                const int radius_x = (triplet_arc_x_end == -1 or  triplet_arc_x_end == triplet_arc_x_start ?
                                      100 : (triplet_arc_x_end - triplet_arc_x_start)/2);
                
                const int base_y = LEVEL_TO_Y(noteRenderInfo.triplet_arc_level) + (noteRenderInfo.triplet_show_above ? -80 : 90);
                
                renderArc(dc, center_x, base_y, radius_x, noteRenderInfo.triplet_show_above ? -80 : 80);
                dc.SetTextForeground( wxColour(0,0,0) );
                // FIXME: use font size instead of hardcoded constant
                dc.DrawText( wxT("3"), center_x - 25, base_y + (noteRenderInfo.triplet_show_above ? -75 : -20) );
            }
            
            
        } // next note
        
        g_printable = this;
        
        // ---- render silences
        std::cout << " == rendering silences ==\n";
        
        const int first_measure = line.getFirstMeasure();
        const int last_measure  = line.getLastMeasure();
        
        global_dc = &dc;
        
        //FIXME: we already have collected all silence info in a vector... don't call the SilenceAnalyser again!
        if (f_clef)
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level + 4);
            g_line_height = lineHeight;
            SilenceAnalyser::findSilences( &renderSilenceCallback, lineAnalyser, first_measure, last_measure, silences_y );
        }
        else
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level - 8);
            g_line_height = lineHeight;
            SilenceAnalyser::findSilences( &renderSilenceCallback, lineAnalyser, first_measure, last_measure, silences_y );
        }
        
    }
    // -------------------------------------------------------------------------------------------
}
