
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
#include "Printing/PrintingBase.h"
#include "Printing/PrintLayout.h"
#include "Printing/PrintLayoutMeasure.h"
#include "Printing/PrintLayoutLine.h"

namespace AriaMaestosa
{
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
    
    class ScoreData : public EditorData
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
    
    /*
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
        dc.DrawLine( x-30/2, y,      x+30/2, y-20/2 );
        dc.DrawLine( x-30/2, y+40/2, x+30/2, y+20/2 );
        
        // vertical lines
        dc.DrawLine( x-30/2, y+40/2, x-30/2, y-60/2 );
        dc.DrawLine( x+30/2, y-20/2, x+30/2, y+80/2 );
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
    /** Renders an arc (half an ellipse) at the given coordinates */
    void renderArc(wxDC& dc, const int center_x, const int center_y,
                   const int radius_x, const int radius_y)
    {
        wxPoint points[] =
        {
            wxPoint(center_x + radius_x*cos(0.1), center_y + radius_y*sin(0.1)),
            wxPoint(center_x + radius_x*cos(0.3), center_y + radius_y*sin(0.3)),
            wxPoint(center_x + radius_x*cos(0.6), center_y + radius_y*sin(0.6)),
            wxPoint(center_x + radius_x*cos(0.9), center_y + radius_y*sin(0.9)),
            wxPoint(center_x + radius_x*cos(1.2), center_y + radius_y*sin(1.2)),
            wxPoint(center_x + radius_x*cos(1.5), center_y + radius_y*sin(1.5)),
            wxPoint(center_x + radius_x*cos(1.8), center_y + radius_y*sin(1.8)),
            wxPoint(center_x + radius_x*cos(2.1), center_y + radius_y*sin(2.1)),
            wxPoint(center_x + radius_x*cos(2.4), center_y + radius_y*sin(2.4)),
            wxPoint(center_x + radius_x*cos(2.7), center_y + radius_y*sin(2.7)),
            wxPoint(center_x + radius_x*cos(3.0), center_y + radius_y*sin(3.0)),
        };
        dc.DrawSpline(11, points);
    }
    
    // -------------------------------------------------------------------------------------------

    // leave a pointer to the dc for the callback
    // FIXME find cleaner way
    wxDC* global_dc = NULL;
    ScorePrintable* g_printable = NULL;
    
    // leave height of lines for the renderSilence callback
    // FIXME find cleaner way
    int global_line_height=5;
    
    std::vector<int> g_silences_ticks;
    
    // -------------------------------------------------------------------------------------------
    
    void gatherSilenceCallback(const int duration, const int tick, const int type, const int silences_y, const bool triplet, const bool dotted, const int dot_delta_x, const int dot_delta_y)
    {
        g_silences_ticks.push_back(tick);
        std::cout << "gatherSilenceCallback : silence at " << tick << " (beat " << (tick/960.0f) << ")\n";
    }
    
    // -------------------------------------------------------------------------------------------
    
    void renderSilenceCallback(const int duration, const int tick, const int type, const int silences_y, const bool triplet,
                               const bool dotted, const int dot_delta_x, const int dot_delta_y)
    {
        assert( global_dc != NULL);
        
        const Range<int> x = x_converter->tickToX(tick);
        if (x.from < 0) return; // this part of score is not printed (e.g. is in a repetition)
        assert(x.to != -1);
        
        const int x_center = (x.from + x.to)/2;
        
        // debug draw
        //static int silenceShift = 0;
        //silenceShift += 5;
        //global_dc->DrawLine(x, silences_y + silenceShift % 25, x_to, silences_y + silenceShift % 25);
                
        //{ TODO : use again when repetition is properly back in
        //    LayoutElement* temp = g_printable->getElementForMeasure(measure);
        //    if (temp != NULL and (temp->getType() == REPEATED_RIFF or temp->getType() == SINGLE_REPEATED_MEASURE))
        //        return; //don't render silences in repetions measure!
        //}
        
        global_dc->SetBrush( *wxBLACK_BRUSH );
        
        int silence_center = -1;
        int silence_radius = -1;
        
        if ( type == 1 )
        {
            global_dc->SetPen(  *wxTRANSPARENT_PEN  );
            silence_radius = 40;
            
            // FIXME - remove hardcoded values
            global_dc->DrawRectangle(x.from + 40, silences_y, silence_radius*2, (int)round(global_line_height/2));
            silence_center = x.from + 40 + silence_radius;
        }
        else if ( type == 2 )
        {
            silence_radius = 40;
            global_dc->SetPen(  *wxTRANSPARENT_PEN  );
            
            // FIXME - hardcoded values
            global_dc->DrawRectangle(x.from + 40, (int)round(silences_y+global_line_height/2),
                                     silence_radius*2, (int)round(global_line_height/2.0));
            silence_center = x.from + 40 + silence_radius;
        }
        else if ( type == 4 )
        {
            static wxBitmap silence( getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + wxT("silence4.png"), wxBITMAP_TYPE_PNG );
            const float scale = 6.5f;
            static wxBitmap silenceBigger = wxBitmap(silence.ConvertToImage().Scale(silence.GetWidth()*scale, silence.GetHeight()*scale));
            
            silence_radius = silenceBigger.GetWidth()/2;
            global_dc->DrawBitmap( silenceBigger, x_center - silence_radius, silences_y );
            
            silence_center = x_center;
        }
        else if ( type == 8 )
        {
            static wxBitmap silence( getResourcePrefix() + wxT("score") + wxFileName::GetPathSeparator() + wxT("silence8.png"), wxBITMAP_TYPE_PNG );
            const float scale = 6.5f;
            static wxBitmap silenceBigger = wxBitmap(silence.ConvertToImage().Scale(silence.GetWidth()*scale, silence.GetHeight()*scale));
            
            silence_radius = silenceBigger.GetWidth()/2;
            global_dc->DrawBitmap( silenceBigger, x_center - silence_radius, silences_y + 20);
            
            silence_center = x_center;
        }
        else if ( type == 16 )
        {
            // TODO : use x_center
            global_dc->SetPen(  wxPen( wxColour(0,0,0), 8 ) );
            const int mx = x.from + 50;
            const int y = silences_y + 80;
            wxPoint points[] =
            {
                wxPoint(mx,     y+50),
                wxPoint(mx+25,  y),
                wxPoint(mx,     y),
            };
            global_dc->DrawSpline(3, points);
            wxPoint points2[] =
            {
                wxPoint(mx+20,  y+5),
                wxPoint(mx+50,  y-50),
                wxPoint(mx+25,  y-50),
            };
            global_dc->DrawSpline(3, points2);
            
            global_dc->DrawCircle(mx, y, 6);
            global_dc->DrawCircle(mx+25, y-50, 6);
            
            silence_center = mx + 50/2;
            silence_radius = 25;
        }
        
        // dotted
        if (dotted)
        {
            wxPoint headLocation( silence_center + silence_radius + 20, silences_y+30 );
            global_dc->DrawEllipse( headLocation, wxSize(15,15) );
        }
        
        // triplet
        if (triplet)
        {
            wxPen tiePen( wxColour(0,0,0), 10 ) ;
            global_dc->SetPen( tiePen );
            global_dc->SetBrush( *wxTRANSPARENT_BRUSH );
            
            const int radius_x = 50;
            
            const int base_y = silences_y + 100;
            
            static wxSize triplet_3_size = global_dc->GetTextExtent(wxT("3"));
            
            renderArc(*global_dc, silence_center - 9, base_y, radius_x, 80);
            global_dc->SetTextForeground( wxColour(0,0,0) );
            global_dc->DrawText( wxT("3"), silence_center - triplet_3_size.GetWidth()/3 - 11, base_y-20 );
        }
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
        if (from_note == -1 || to_note == -1)
            return 0;
                
        std::cout <<
            PRINT_VAR(scoreData->extra_lines_under_g_score) <<
            PRINT_VAR(scoreData->extra_lines_above_g_score) <<
            PRINT_VAR(scoreData->extra_lines_under_f_score) <<
            PRINT_VAR(scoreData->extra_lines_above_f_score) << std::endl;
        
        return (g_clef ? 5 : 0) + (f_clef ? 5 : 0) + 
                abs(scoreData->extra_lines_under_g_score) +
                abs(scoreData->extra_lines_above_g_score) +
                abs(scoreData->extra_lines_under_f_score) +
                abs(scoreData->extra_lines_above_f_score);

    }
    
    // -------------------------------------------------------------------------------------------
#define VERBOSE 0
    
    void ScorePrintable::addUsedTicks(const PrintLayoutMeasure& measure, const MeasureTrackReference& trackRef,
                                      std::map<int /* tick */,TickPosInfo>& ticks_relative_position)
    {
        const int fromTick = measure.firstTick;
        const int toTick = measure.lastTick;
        
#if VERBOSE
        std::cout << "\naddingTicks(measure " << (measure.id+1) << ", from " << fromTick << ", to " << toTick << "\n{\n";
#endif
        
        Track* track = trackRef.track;
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();

        if (f_clef)
        {
            const int noteAmount = f_clef_analyser->noteRenderInfo.size();
            
            // find shortest note
            int shortest = -1;
            for(int n=0; n<noteAmount; n++)
            {
                const int tick = f_clef_analyser->noteRenderInfo[n].tick;
                if (tick < fromTick or tick >= toTick) continue;
                
                if (f_clef_analyser->noteRenderInfo[n].tick_length < shortest or shortest == -1)
                {
                    shortest = f_clef_analyser->noteRenderInfo[n].tick_length;
                }
            }
            
            for(int n=0; n<noteAmount; n++)
            {
                const int tick = f_clef_analyser->noteRenderInfo[n].tick;
                if (tick < fromTick or tick >= toTick) continue;

#if VERBOSE
                std::cout << "    Adding tick " << tick << " to list" << std::endl;
#endif
                
                // wider notes should be given a bit more space.
                float ratioToShortest = (float)f_clef_analyser->noteRenderInfo[n].tick_length / (float)shortest;
                float additionalWidth = log( ratioToShortest ) / log( 2 );

                if (f_clef_analyser->noteRenderInfo[n].sign != PITCH_SIGN_NONE)
                {
                    // if there's an accidental sign to show, allocate a bigger space for this note
                    ticks_relative_position[ tick ].setProportion(2 + additionalWidth);
                }
                else
                {
                    ticks_relative_position[ tick ].setProportion(1 + additionalWidth);
                }
            }
        }
        if (g_clef)
        {
            const int noteAmount = g_clef_analyser->noteRenderInfo.size();
            
            // find shortest note
            int shortest = -1;
            for(int n=0; n<noteAmount; n++)
            {
                const int tick = g_clef_analyser->noteRenderInfo[n].tick;
                if (tick < fromTick or tick >= toTick) continue;
                
                if (g_clef_analyser->noteRenderInfo[n].tick_length < shortest or shortest == -1)
                {
                    shortest = g_clef_analyser->noteRenderInfo[n].tick_length;
                }
            }
            
            for (int n=0; n<noteAmount; n++)
            {
                const int tick = g_clef_analyser->noteRenderInfo[n].tick;
                if (tick < fromTick or tick >= toTick) continue;
                
#if VERBOSE
                std::cout << "    Adding tick " << tick << " to list" << std::endl;
#endif
                
                // wider notes should be given a bit more space.
                float ratioToShortest = (float)g_clef_analyser->noteRenderInfo[n].tick_length / (float)shortest;
                float additionalWidth = log( ratioToShortest ) / log( 2 );
                
                if (g_clef_analyser->noteRenderInfo[n].sign != PITCH_SIGN_NONE)
                {
                    // if there's an accidental sign to show, allocate a bigger space for this note
                    ticks_relative_position[ tick ].setProportion(2 + additionalWidth);
                }
                else
                {
                    ticks_relative_position[ tick ].setProportion(1 + additionalWidth);
                }
            }
        }

        

        const int silenceAmount = silences_ticks.size();
        for (int n=0; n<silenceAmount; n++)
        {
            if (silences_ticks[n] < fromTick or silences_ticks[n] >= toTick) continue;
            
#if VERBOSE
            std::cout << "    Adding [silence] tick " << silences_ticks[n] << " to list" << std::endl;
#endif
            
            ticks_relative_position[ silences_ticks[n]].setProportion(1);
        }
        
#if VERBOSE
        std::cout << "}\n";
#endif
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::drawLine(const int trackID, LineTrackRef& lineTrack, LayoutLine& line, wxDC& dc)
    {
        assertExpr(lineTrack.y0,>,0);
        assertExpr(lineTrack.y1,>,0);
        assertExpr(lineTrack.y0,<,50000);
        assertExpr(lineTrack.y1,<,50000);
        setCurrentDC(&dc);
        
        
        std::cout << "ScorePrintable size : " << lineTrack.x0 << ", " << lineTrack.y0 << " to " << lineTrack.x1 << ", " << lineTrack.y1 << std::endl;
        
        x_converter = new PrintXConverter(this, &line, trackID);
        
        // gather score info
        //gatherNotesAndBasicSetup(line);
        ScoreData* scoreData = dynamic_cast<ScoreData*>(lineTrack.editor_data.raw_ptr);
        
        // since height is used to determine where to put repetitions/notes/etc.
        // only pass the height of the first score if there's 2, so stuff don't appear between both scores
        //setLineYCoords(lineTrack.y0,
        //               ( f_clef and g_clef ?
        //                lineTrack.y0 + (lineTrack.y1 - lineTrack.y0)*scoreData->first_clef_proportion :
        //                lineTrack.y1 ));
        
        // if we have only one clef, give it the full space.
        // if we have two, split the space between both
        int g_clef_y_from=-1, g_clef_y_to=-1;
        int f_clef_y_from=-1, f_clef_y_to=-1;
        
        if (g_clef and not f_clef)
        {
            g_clef_y_from = lineTrack.y0;
            g_clef_y_to = lineTrack.y1;
        }
        else if (f_clef and not g_clef)
        {
            f_clef_y_from = lineTrack.y0;
            f_clef_y_to = lineTrack.y1;
        }
        else if (f_clef and g_clef)
        {
            g_clef_y_from = lineTrack.y0;
            g_clef_y_to = lineTrack.y0 + (int)round((lineTrack.y1 - lineTrack.y0)*scoreData->first_clef_proportion);
            f_clef_y_from = lineTrack.y0 + (int)round((lineTrack.y1 - lineTrack.y0)*(1-scoreData->second_clef_proportion));
            f_clef_y_to = lineTrack.y1;
        }
        else { assert(false); }
        
        
        
        // iterate through layout elements (this also inits them)
        // FIXME : not too clean to init them at this point
        LayoutElement* currentElement;
        std::cout << "\nLayout elements X coords :\n";
        
        const int elementAmount = line.getElementCount(trackID);
        for(int el=0; el<elementAmount; el++)
        {
            currentElement = continueWithNextElement(trackID, line, el);
            std::cout << "    Layout element from x=" << currentElement->getXFrom() << " to x=" << currentElement->getXTo() << std::endl;
        }//next element
        std::cout << std::endl;
        
        g_printable = this;
        
        if (g_clef)
        {
            analyseAndDrawScore(false /*G*/, *g_clef_analyser, line, lineTrack.track, dc,
                      abs(scoreData->extra_lines_above_g_score), abs(scoreData->extra_lines_under_g_score),
                      lineTrack.x0, g_clef_y_from, lineTrack.x1, g_clef_y_to,
                      lineTrack.show_measure_number);
        }
        
        if (f_clef)
        {
            analyseAndDrawScore(true /*F*/, *f_clef_analyser, line, lineTrack.track, dc,
                      abs(scoreData->extra_lines_above_f_score), abs(scoreData->extra_lines_under_f_score),
                      lineTrack.x0, f_clef_y_from, lineTrack.x1, f_clef_y_to,
                      (g_clef ? false : lineTrack.show_measure_number) /* if we have both keys don't show twice */);
        }
        
        delete x_converter;
        x_converter = NULL;
        
        // ---- Debug guides
        if (PRINT_LAYOUT_HINTS)
        {
            dc.SetPen( wxPen(*wxBLUE, 7) );
            dc.DrawLine(lineTrack.x0, lineTrack.y0, lineTrack.x1, lineTrack.y0);
            dc.DrawLine(lineTrack.x0, lineTrack.y1, lineTrack.x1, lineTrack.y1);
            dc.DrawLine(lineTrack.x0, lineTrack.y0, lineTrack.x0, lineTrack.y1);
            dc.DrawLine(lineTrack.x1, lineTrack.y0, lineTrack.x1, lineTrack.y1);
        }
    }

    // -------------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------------
    
#if 0
#pragma mark -
#pragma mark ScorePrintable (private utils)
#endif
    

    void ScorePrintable::gatherVerticalSizingInfo(const int trackID, LineTrackRef& lineTrack, LayoutLine& line)
    {
        ScoreData* scoreData = new ScoreData();
        lineTrack.editor_data = scoreData;
        
        Track* track = lineTrack.track;
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        const int fromMeasure = line.getFirstMeasure();
        const int lastMeasure = line.getLastMeasure();

        //scoreData->from_note = line.getFirstNote();
        //scoreData->to_note   = line.getLastNote();
        
        int highest_pitch = -1, lowest_pitch = -1;
        int biggest_level = -1, smallest_level = -1;
        
        for (int m=fromMeasure; m<=lastMeasure; m++)
        {
            //  FIXME : this also needs to be per-track
            PerMeasureInfo& info = perMeasureInfo[trackID*5000+m];
                        
            if (info.highest_pitch < highest_pitch or highest_pitch == -1)
            {
                highest_pitch = info.highest_pitch;
            }
            if (info.lowest_pitch > lowest_pitch or lowest_pitch == -1)
            {
                lowest_pitch = info.lowest_pitch;
            } 
            if (info.biggest_level > biggest_level or biggest_level == -1)
            {
                biggest_level = info.biggest_level;
            }
            if (info.smallest_level < smallest_level or smallest_level == -1)
            {
                smallest_level = info.smallest_level;
            }
        }

        middle_c_level = converter->getScoreCenterCLevel(); //converter->getMiddleCLevel();
        
        const int g_clef_from_level = middle_c_level-10;
        const int g_clef_to_level   = middle_c_level-2;
        const int f_clef_from_level = middle_c_level+2;
        const int f_clef_to_level   = middle_c_level+10;
        
        g_clef = scoreEditor->isGClefEnabled();
        f_clef = scoreEditor->isFClefEnabled();
        
        scoreData->extra_lines_above_g_score = 0;
        scoreData->extra_lines_under_g_score = 0;
        scoreData->extra_lines_above_f_score = 0;
        scoreData->extra_lines_under_f_score = 0;
        if (g_clef and not f_clef)
        {
            std::cout << "G: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level) << PRINT_VAR(g_clef_from_level) << PRINT_VAR(g_clef_to_level) << std::endl;

            if (smallest_level!=-1 and smallest_level < g_clef_from_level)  scoreData->extra_lines_above_g_score = (g_clef_from_level - smallest_level)/2;
            if (biggest_level!=-1 and biggest_level > g_clef_to_level) scoreData->extra_lines_under_g_score = (g_clef_to_level - biggest_level)/2;
        }
        else if (f_clef and not g_clef)
        {
            std::cout << "F: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level) << PRINT_VAR(f_clef_from_level) << PRINT_VAR(f_clef_to_level) << std::endl;
            if (smallest_level!=-1 and smallest_level < f_clef_from_level) scoreData->extra_lines_above_f_score = (f_clef_from_level - smallest_level)/2;
            if (biggest_level!=-1 and biggest_level > f_clef_to_level) scoreData->extra_lines_under_f_score = (f_clef_to_level - biggest_level)/2;
        }
        else if (f_clef and g_clef)
        {
            std::cout << "F: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level) << PRINT_VAR(f_clef_from_level) << PRINT_VAR(f_clef_to_level) << std::endl;
            std::cout << "G: " << PRINT_VAR(smallest_level) << PRINT_VAR(biggest_level) << PRINT_VAR(g_clef_from_level) << PRINT_VAR(g_clef_to_level) << std::endl;

            if (smallest_level!=-1 and smallest_level < g_clef_from_level) scoreData->extra_lines_above_g_score = (g_clef_from_level - smallest_level)/2;
            if (biggest_level!=-1 and biggest_level > f_clef_to_level) scoreData->extra_lines_under_f_score = (f_clef_to_level - biggest_level)/2;
        }
        
        std::cout << PRINT_VAR(scoreData->extra_lines_above_g_score) <<
                     PRINT_VAR(scoreData->extra_lines_under_g_score) <<
                     PRINT_VAR(scoreData->extra_lines_above_f_score) <<
                     PRINT_VAR(scoreData->extra_lines_under_f_score) << std::endl;
        
        // Split space between both scores (one may need more than the other)
        // I use a total of 0.8 to leave a 0.2 free space between both scores.
        scoreData->first_clef_proportion = 0.4;
        scoreData->second_clef_proportion = 0.4;
        
        if (g_clef and f_clef and
           scoreData->extra_lines_above_g_score + scoreData->extra_lines_under_f_score != 0 /* unnecessary if nothing under/over scores*/)
        {
            /*  where 0.8 is used to leave a 0.2 margin between both scores. 5 is the amount of lines needed for the regular score.
             10 is the amount of lines needed for both regular scores */
            const float total_height = abs(scoreData->extra_lines_above_g_score) + abs(scoreData->extra_lines_under_f_score) + 10.0;
            scoreData->first_clef_proportion = 0.8 * (abs(scoreData->extra_lines_above_g_score)+5.0) / total_height;
            scoreData->second_clef_proportion = 0.8 * (abs(scoreData->extra_lines_under_f_score)+5.0) / total_height;
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
        
        for (int m=0; m<measureAmount; m++)
        {
            // find highest and lowest note we need to render in each measure
            int highest_pitch = -1, lowest_pitch = -1;
            int biggest_level = -1, smallest_level = -1;
            
            const int from_tick = measures->firstTickInMeasure(m);
            const int to_tick = measures->lastTickInMeasure(m);
            
            const int firstNote = track->findFirstNoteInRange( from_tick, to_tick );
            const int lastNote = track->findLastNoteInRange( from_tick, to_tick );
            
            //std::cout << "Measure " << m << " : reading notes " << firstNote << " to " << lastNote << std::endl;
            
            for(int n=firstNote; n<=lastNote; n++)
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
        
        // iterate through measures to collect notes in the vector
        // so ScoreAnalyser can prepare the score
        std::cout << " == gathering note list ==\n";
        for(int m=0; m<measureAmount; m++)
        {
            PerMeasureInfo& measInfo = perMeasureInfo[trackID*5000+m];
            const int firstNote = measInfo.first_note;
            const int lastNote = measInfo.last_note;
            
            if (firstNote == -1 or lastNote == -1) continue; // empty measure
            
            for(int n=firstNote; n<=lastNote; n++)
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
                    g_clef_analyser->addToVector(currentNote, false);
                }
                else if (f_clef and not g_clef)
                {
                    f_clef_analyser->addToVector(currentNote, false);
                }
                else if (f_clef and g_clef)
                {
                    if (noteLevel < middle_c_level)
                        g_clef_analyser->addToVector(currentNote, false);
                    else
                        f_clef_analyser->addToVector(currentNote, false);
                }
            }
            
        }//next element
        
        /*
         if (f_clef)
         {
         const int amount = f_clef_analyser->noteRenderInfo.size();
         int tick = -1;
         for(int n=0; n<amount; n++)
         {
         const int newTick = f_clef_analyser->noteRenderInfo[n].tick;
         
         if (newTick < tick)
         {
         std::cout << "[F noteRenderInfo] ticks not in order!! " << tick << " then " << newTick << "\n";
         }
         f_clef_analyser->noteRenderInfo[n].tick = tick;
         
         //std::cout << "[F noteRenderInfo] " << f_clef_analyser->noteRenderInfo[n].tick << std::endl;
         }
         }
         if (g_clef)
         {
         const int amount = g_clef_analyser->noteRenderInfo.size();
         for(int n=0; n<amount; n++)
         {
         std::cout << "[G noteRenderInfo] " << g_clef_analyser->noteRenderInfo[n].tick << std::endl;
         }
         }
         */
        
        // ---- Silences
        std::cout << " == gathering silence list ==\n";
        g_silences_ticks.clear();
        if (f_clef)
        {
            f_clef_analyser->renderSilences( &gatherSilenceCallback, 0, measureAmount-1, -1 /* y not important at this point */ );
        }
        if (g_clef)
        {
            g_clef_analyser->renderSilences( &gatherSilenceCallback, 0, measureAmount-1, -1 /* y not important at this point */ );
        }
        this->silences_ticks = g_silences_ticks;
        g_silences_ticks.clear();
        
        
    }
    
    // -------------------------------------------------------------------------------------------
    
    void ScorePrintable::analyseAndDrawScore(bool f_clef, ScoreAnalyser& analyser, LayoutLine& line, Track* track,
                                   wxDC& dc, const int extra_lines_above, const int extra_lines_under,
                                   const int x0, const int y0, const int x1, const int y1,
                                   bool show_measure_number)
    {
        std::cout << "\n    analyseAndDrawScore " << (f_clef ? "F" : "G") << "\n\n";
        
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        const int middle_c_level = converter->getScoreCenterCLevel();
        const int first_score_level = middle_c_level + (f_clef? 2 : -10);
        const int last_score_level = first_score_level + 8;
        const int min_level =  first_score_level - extra_lines_above*2;

        note_x_shift = headRadius; // shift to LEFT by a 'headRadius', since note will be drawn from the right of its area
                                   // and its center is the origin of the drawing
        						   // e.g. Drawing area of a note :
          						   // |     |
                                   // |     |  <-- stem at the right
                                   // |  ( )|
        						   //     ^ origin of the note here, in its center. so substract a radius from the right
        
        stem_up_x_offset = -10; // since note is right-aligned, keep the stem at the right. go 10 towards the note to "blend" in it.
        stem_up_y_offset = 0;
        stem_down_x_offset = -headRadius*2 + 4; // since note is right-aligned. go 4 towards the note to "blend" in it.
        stem_down_y_offset = 0;
                
#define LEVEL_TO_Y( lvl ) y0 + 1 + lineHeight*0.5*(lvl - min_level)
                
        
        // ------------------ preliminary part : backgrounds -----------------

        // ---- draw score background (horizontal lines)
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
        
        // ---- render vertical dividers and time signature changes
        std::cout << " == rendering vertical dividers & time sig changes ==\n";
        
        const int measure_dividers_from_y = LEVEL_TO_Y(first_score_level);
        const int measure_dividers_to_y = LEVEL_TO_Y(last_score_level);
        
        const int elamount = line.layoutElements.size();
        for (int n=0; n<elamount; n++)
        {
            drawVerticalDivider(&line.layoutElements[n], measure_dividers_from_y, measure_dividers_to_y);
            
            if (line.layoutElements[n].getType() == TIME_SIGNATURE)
            {
                EditorPrintable::renderTimeSignatureChange(&line.layoutElements[n], LEVEL_TO_Y(first_score_level), LEVEL_TO_Y(last_score_level));
            }
        }
        
        // ---- line header if any
        if (line.layoutElements[0].getType() == LINE_HEADER)
        {
            std::cout << " == rendering line header ==\n";
            if (!f_clef) renderGClef(dc, line.layoutElements[0].getXFrom(),
                                    LEVEL_TO_Y(last_score_level)+10,
                                    LEVEL_TO_Y(last_score_level-4)-5);
            else renderFClef(dc, line.layoutElements[0].getXFrom(),
                             LEVEL_TO_Y(first_score_level),
                             LEVEL_TO_Y(first_score_level+3));
            
            // render key signature next to staff
            // on which level to put the signs (0 is the level of the highest sign when all are shown)
            const unsigned short int sharp_sign_lvl[] = { 1, 4, 0, 3, 6, 2, 5 };
            const unsigned short int flat_sign_lvl[] = { 3, 0, 4, 1, 5, 2, 6 };
            
            const int sharps = track->graphics->getCurrentEditor()->getKeySharpsAmount();
            const int flats = track->graphics->getCurrentEditor()->getKeyFlatsAmount();
            assertExpr(line.layoutElements.size(),>=,0);
            
            if (sharps > 0 or flats > 0)
            {
                int x_space_per_symbol = (line.layoutElements[0].getXTo() - line.layoutElements[0].getXFrom()
                                          - 300 - 50 /* some additional space */) / std::max(sharps, flats);
                if (x_space_per_symbol > 50) x_space_per_symbol = 50;
                
                for (int n=0; n<sharps; n++)
                {
                    const int level = first_score_level + (f_clef ? 1 : -1) + sharp_sign_lvl[n];
                    renderSharp( dc, line.layoutElements[0].getXFrom()+300+n*x_space_per_symbol, LEVEL_TO_Y(level) );
                }
                for (int n=0; n<flats; n++)
                {
                    const int level = first_score_level + (f_clef ? 3 : 1) + flat_sign_lvl[n];
                    renderFlat( dc, line.layoutElements[0].getXFrom()+300+n*x_space_per_symbol, LEVEL_TO_Y(level) );
                }
            }
            
            // also show ottava bassa/alta if relevant
            const int octave_shift = track->graphics->scoreEditor->getScoreMidiConverter()->getOctaveShift(); // FIXME - woot
            if (octave_shift > 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(first_score_level - 3);
                dc.DrawText(wxT("8va"), line.layoutElements[0].getXFrom()+200, y);
            }
            else if (octave_shift < 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(last_score_level);
                dc.DrawText(wxT("8vb"), line.layoutElements[0].getXFrom()+200, y);
            }
        }
        
        // ------------------ first part : basic unintelligent drawing -----------------
        // for all parts of score notes that can be rendered without using the ScoreAnalyser.
        
        const int fromTick = getMeasureData()->firstTickInMeasure( line.getFirstMeasure() );
        const int toTick = getMeasureData()->lastTickInMeasure( line.getLastMeasure() );
        
        {
            const int noteAmount = analyser.noteRenderInfo.size();
                
            // ---- Draw small lines above/below score
            std::cout << " == rendering lines for notes out of score ==\n";
            for(int i=0; i<noteAmount; i++)
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
                        dc.DrawLine(noteX.from, y, noteX.from+135, y);
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
                        dc.DrawLine(noteX.from, y, noteX.from+135, y);
                    }
                }
            } // end scope
            
            // ---- draw notes heads
            std::cout << " == rendering note heads ==\n";
            for (int i=0; i<noteAmount; i++)
            {
                if (analyser.noteRenderInfo[i].tick < fromTick) continue;
                if (analyser.noteRenderInfo[i].tick >= toTick) break;

                //std::cout << "Drawing note at " << analyser.noteRenderInfo[i].tick << " - beat " <<  analyser.noteRenderInfo[i].tick/960 << std::endl;         
                NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];

                const Range<int> noteX = x_converter->tickToX(noteRenderInfo.tick);

                // Amount by which to shift the executable sign, from 'noteX'
                const int accidentalShift = noteRenderInfo.sign == PITCH_SIGN_NONE ? 0 : headRadius*1.85;
    
                // draw head
                const int notey = LEVEL_TO_Y(noteRenderInfo.getBaseLevel());
                wxPoint headLocation( noteX.to - note_x_shift, // this is the center of the note
                                      notey-(headRadius-5)/2.0);
                
                if (noteRenderInfo.instant_hit)
                {
                    dc.DrawText(wxT("X"), headLocation.x - 36, headLocation.y);
                }
                else
                {
                    dc.SetPen(  wxPen( wxColour(0,0,0), 12 ) );
                    dc.SetBrush( *wxBLACK_BRUSH );
                    const int cx = headLocation.x;
                    const int cy = headLocation.y;
                    wxPoint points[25];
                    for (int n=0; n<25; n++)
                    {
                        // FIXME - instead of always substracting to radius, just make it smaller...
                        const float angle = n/25.0*6.283185f /* 2*PI */;
                        points[n] = wxPoint( cx + (headRadius-5)*cos(angle),
                                             cy + headRadius/2 + (headRadius - 14)*sin(angle) - headRadius*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f );
                    }

                    if (noteRenderInfo.hollow_head) dc.DrawSpline(25, points);
                    else dc.DrawPolygon(25, points, -3);
                    
                }
                noteRenderInfo.setY(notey+headRadius/2.0);
                
                // draw dot if note is dotted
                if (noteRenderInfo.dotted)
                {
                    wxPoint headLocation( headLocation.x + headRadius*2, notey+10 );
                    dc.DrawEllipse( headLocation, wxSize(10,10) );
                }
                
                // draw sharpness sign if relevant
                if (noteRenderInfo.sign == SHARP)        renderSharp  ( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 15  );
                else if (noteRenderInfo.sign == FLAT)    renderFlat   ( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 15  );
                else if (noteRenderInfo.sign == NATURAL) renderNatural( dc, noteX.from + accidentalShift, noteRenderInfo.getY() - 20  );
                
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
                
                const Range<int> noteX = x_converter->tickToX(noteRenderInfo.tick);
                std::cout << "tied to tick " << noteRenderInfo.getTiedToTick() << " from " << noteRenderInfo.tick << std::endl;
                const int tiedToPixel = x_converter->tickToX(noteRenderInfo.getTiedToTick()).from;
                std::cout << "tied to pixel " << tiedToPixel << " from " << getStemX(noteRenderInfo) << std::endl;

                const int center_x = (tiedToPixel + noteX.from)/2 + headRadius*2;
                const int radius_x = abs(tiedToPixel - noteX.from)/2;
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
            if (noteRenderInfo.drag_triplet_sign and noteRenderInfo.triplet_arc_tick_start != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                int triplet_arc_x_start = x_converter->tickToX(noteRenderInfo.triplet_arc_tick_start).from;
                int triplet_arc_x_end = -1;
                
                if (noteRenderInfo.triplet_arc_tick_end != -1)
                {
                    triplet_arc_x_end = x_converter->tickToX(noteRenderInfo.triplet_arc_tick_end).from;
                }
                
                const int center_x = (triplet_arc_x_end == -1 ?
                                      triplet_arc_x_start :
                                      (triplet_arc_x_start + triplet_arc_x_end)/2) + headRadius;
                const int radius_x = (triplet_arc_x_end == -1 or  triplet_arc_x_end == triplet_arc_x_start ?
                                      100 : (triplet_arc_x_end - triplet_arc_x_start)/2);
                
                const int base_y = LEVEL_TO_Y(noteRenderInfo.triplet_arc_level) + (noteRenderInfo.triplet_show_above ? -80 : 90);
                
                renderArc(dc, center_x + headRadius*1.2, base_y, radius_x, noteRenderInfo.triplet_show_above ? -80 : 80);
                dc.SetTextForeground( wxColour(0,0,0) );
                dc.DrawText( wxT("3"), center_x + headRadius/2, base_y + (noteRenderInfo.triplet_show_above ? -75 : -20) );
            }
            
            
        } // next note
        
        g_printable = this;
        
        // ---- render silences
        std::cout << " == rendering silences ==\n";
        
        const int first_measure = line.getFirstMeasure();
        const int last_measure  = line.getLastMeasure();
        
        global_dc = &dc;
        
        if (f_clef)
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level + 4);
            global_line_height = lineHeight;
            lineAnalyser->renderSilences( &renderSilenceCallback, first_measure, last_measure, silences_y );
        }
        else
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level - 8);
            global_line_height = lineHeight;
            lineAnalyser->renderSilences( &renderSilenceCallback, first_measure, last_measure, silences_y );
        }
        
    }
    // -------------------------------------------------------------------------------------------
}
