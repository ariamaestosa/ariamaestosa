
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "Midi/MeasureData.h"
#include "Editors/ScoreEditor.h"
#include "Editors/ScoreAnalyser.h"
#include "IO/IOUtils.h"
#include "Printing/ScorePrint.h"
#include "Printing/PrintingBase.h"
#include "Printing/PrintLayout.h"

namespace AriaMaestosa
{
#if 0
#pragma mark -
#pragma mark ScoreData
#endif
    
    class ScoreData : public EditorData
        {
        public:
            LEAK_CHECK();
            
            virtual ~ScoreData() {}
            
            int middle_c_level, from_note, to_note;
            bool g_clef, f_clef;
            int extra_lines_above_g_score;
            int extra_lines_under_g_score;
            int extra_lines_above_f_score;
            int extra_lines_under_f_score;
            float first_clef_proportion ;
            float second_clef_proportion;
            
            OwnerPtr<ScoreAnalyser> g_clef_analyser;
            OwnerPtr<ScoreAnalyser> f_clef_analyser;
        };

#if 0
#pragma mark -
#pragma mark PrintXConverter
#endif
    
    /*
     * An instance of this will be given to the score analyser. Having a separate x-coord-converter
     * allows it to do conversions between units without becoming context-dependent.
     */
    class PrintXConverter : public TickToXConverter
        {
            ScorePrintable* parent;
        public:
            LEAK_CHECK();
            
            PrintXConverter(ScorePrintable* parent)
            {
                PrintXConverter::parent = parent;
            }
            ~PrintXConverter(){}
            int tickToX(const int tick)
            {
                return parent->tickToX(tick);
            }
        };
    
    // global (FIXME)
    PrintXConverter* x_converter = NULL;
    
#if 0
#pragma mark -
#pragma mark Render Routines
#endif
    
    /*
     * A few drawing routines
     */
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
    void renderGClef(wxDC& dc, const int x, const float score_bottom, const float b_line_y)
    {
        static wxBitmap gclef( getResourcePrefix() + wxT("/score/keyG.png"), wxBITMAP_TYPE_PNG );
        
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
    
    void renderFClef(wxDC& dc, const int x, const float score_top, const float e_line_y)
    {
        static wxBitmap fclef( getResourcePrefix() + wxT("/score/Fkey.png"), wxBITMAP_TYPE_PNG );
        
        const int e_on_image = 15;
        const float scale = (float)(e_line_y - score_top) / (float)e_on_image;
        
        wxBitmap scaled = wxBitmap(fclef.ConvertToImage().Scale(fclef.GetWidth()*scale, fclef.GetHeight()*scale));
        
        dc.DrawBitmap(scaled, x, score_top, true);
    }
    
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
    
    // leave a pointer to the dc for the callback
    // FIXME find cleaner way
    wxDC* global_dc = NULL;
    ScorePrintable* g_printable = NULL;
    
    // leave height of lines for the renderSilence callback
    // FIXME find cleaner way
    int global_line_height=5;
    
    void renderSilenceCallback(const int tick, const int type, const int silences_y, const bool triplet, const bool dotted, const int dot_delta_x, const int dot_delta_y)
    {
        assert( global_dc != NULL);
        
        const int x = x_converter->tickToX(tick);
        if(x < 0) return; // this part of score is not printed (e.g. is in a repetition)
    
        //{ TODO : use again
        //    LayoutElement* temp = g_printable->getElementForMeasure(measure);
        //    if(temp != NULL and (temp->getType() == REPEATED_RIFF or temp->getType() == SINGLE_REPEATED_MEASURE))
        //        return; //don't render silences in repetions measure!
        //}
        
        global_dc->SetBrush( *wxBLACK_BRUSH );
        
        int silence_center = -1;
        int silence_radius = -1;
        
        if( type == 1 )
        {
            global_dc->SetPen(  *wxTRANSPARENT_PEN  );
            // FIXME - remove hardcoded values
            silence_radius = 60;
            global_dc->DrawRectangle(x+40, silences_y, silence_radius*2, (int)round(global_line_height/2));
            silence_center = x+40+silence_radius;
        }
        else if( type == 2 )
        {
            silence_radius = 60;
            global_dc->SetPen(  *wxTRANSPARENT_PEN  );
            // FIXME - hardcoded values
            global_dc->DrawRectangle(x+40, (int)round(silences_y+global_line_height/2), silence_radius*2, (int)round(global_line_height/2.0));
            silence_center = x+40+silence_radius;
        }
        else if( type == 4 )
        {
            static wxBitmap silence( getResourcePrefix() + wxT("/score/silence4.png"), wxBITMAP_TYPE_PNG );
            const float scale = 6.5f;
            static wxBitmap silenceBigger = wxBitmap(silence.ConvertToImage().Scale(silence.GetWidth()*scale, silence.GetHeight()*scale));
            
            global_dc->DrawBitmap( silenceBigger, x+25, silences_y-15 );
            
            silence_radius = silenceBigger.GetWidth()/2;
            silence_center = x + 25 + silence_radius;
        }
        else if( type == 8 )
        {
            static wxBitmap silence( getResourcePrefix() + wxT("/score/silence8.png"), wxBITMAP_TYPE_PNG );
            const float scale = 6.5f;
            static wxBitmap silenceBigger = wxBitmap(silence.ConvertToImage().Scale(silence.GetWidth()*scale, silence.GetHeight()*scale));
            
            global_dc->DrawBitmap( silenceBigger, x+50, silences_y-15 );
            
            silence_radius = silenceBigger.GetWidth()/2;
            silence_center = x + 50 + silence_radius;
        }
        else if( type == 16 )
        {
            global_dc->SetPen(  wxPen( wxColour(0,0,0), 8 ) );
            const int mx = x + 50;
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
        if(dotted)
        {
            wxPoint headLocation( silence_center + silence_radius + 20, silences_y+30 );
            global_dc->DrawEllipse( headLocation, wxSize(15,15) );
        }
        
        // triplet
        if(triplet)
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
    
    ScorePrintable::ScorePrintable(Track* track) : EditorPrintable()
    {
        // std::cout << " *** setting global g_printable" << std::endl;
        g_printable = this;
    }
    ScorePrintable::~ScorePrintable() { }
    
      
    int ScorePrintable::calculateHeight(LayoutLine& line)
    {
        gatherScoreInfo(line);
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(line.editor_data.raw_ptr);
        assert(scoreData != NULL);
        
        return (scoreData->g_clef ? 5 : 0) + (scoreData->f_clef ? 5 : 0) + 
                abs(scoreData->extra_lines_above_g_score) +
                abs(scoreData->extra_lines_under_f_score);

    }
    
    void ScorePrintable::addUsedTicks(const MeasureTrackReference& trackRef,
                                      std::map< int /* tick */, float /* position */ >& ticks_relative_position)
    {
        const int first_note = trackRef.firstNote;
        const int last_note = trackRef.lastNote;
        
        Track* track = trackRef.track;
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        PitchSign note_sign;
        
        if(first_note == -1 or last_note == -1) return; // empty measure
        
        for(int n=first_note; n<=last_note; n++)
        {
            const int tick = track->getNoteStartInMidiTicks(n);

            converter->noteToLevel(track->getNote(n), &note_sign);
            if (note_sign != NONE)
            {
                //std::cout << "note " << n << " will need more space\n";
                ticks_relative_position[ tick-1 ] = -1; // FIXME : ugly hack to leave more space before note
            }
            
            ticks_relative_position[ tick ] = -1; // will be set later
        }
    }
    
    void ScorePrintable::drawLine(LayoutLine& line, wxDC& dc)
    {
        TrackRenderInfo& renderInfo = line.getTrackRenderInfo();
        
        assertExpr(renderInfo.y0,>,0);
        assertExpr(renderInfo.y1,>,0);
        assertExpr(renderInfo.y0,<,50000);
        assertExpr(renderInfo.y1,<,50000);
        setCurrentDC(&dc);
        
        x_converter = new PrintXConverter(this);
        
        // gather score info
        gatherNotesAndBasicSetup(line);
        ScoreData* scoreData = dynamic_cast<ScoreData*>(line.editor_data.raw_ptr);
        
        // since height is used to determine where to put repetitions/notes/etc.
        // only pass the height of the first score if there's 2, so stuff don't appear between both scores
        setLineYCoords(renderInfo.y0,
                       ( scoreData->f_clef and scoreData->g_clef ?
                        renderInfo.y0 + (renderInfo.y1 - renderInfo.y0)*scoreData->first_clef_proportion :
                        renderInfo.y1 ));
        
        // if we have only one clef, give it the full space.
        // if we have two, split the space between both
        int g_clef_y_from=-1, g_clef_y_to=-1;
        int f_clef_y_from=-1, f_clef_y_to=-1;
        
        if(scoreData->g_clef and not scoreData->f_clef)
        {
            g_clef_y_from = renderInfo.y0;
            g_clef_y_to = renderInfo.y1;
        }
        else if(scoreData->f_clef and not scoreData->g_clef)
        {
            f_clef_y_from = renderInfo.y0;
            f_clef_y_to = renderInfo.y1;
        }
        else if(scoreData->f_clef and scoreData->g_clef)
        {
            g_clef_y_from = renderInfo.y0;
            g_clef_y_to = renderInfo.y0 + (int)round((renderInfo.y1 - renderInfo.y0)*scoreData->first_clef_proportion);
            f_clef_y_from = renderInfo.y0 + (int)round((renderInfo.y1 - renderInfo.y0)*(1-scoreData->second_clef_proportion));
            f_clef_y_to = renderInfo.y1;
        }
        else { assert(false); }
        
        
        
        // iterate through layout elements... FIXME : needed?
        LayoutElement* currentElement;
        while((currentElement = continueWithNextElement()) and (currentElement != NULL))
        {
        }//next element
        
        g_printable = this;
        
        if(scoreData->g_clef)
        {
            analyseAndDrawScore(false /*G*/, *(scoreData->g_clef_analyser), line, dc,
                      abs(scoreData->extra_lines_above_g_score), abs(scoreData->extra_lines_under_g_score),
                      renderInfo.x0, g_clef_y_from, renderInfo.x1, g_clef_y_to,
                      renderInfo.show_measure_number);
        }
        
        if(scoreData->f_clef)
        {
            analyseAndDrawScore(true /*F*/, *(scoreData->f_clef_analyser), line, dc,
                      abs(scoreData->extra_lines_above_f_score), abs(scoreData->extra_lines_under_f_score),
                      renderInfo.x0, f_clef_y_from, renderInfo.x1, f_clef_y_to,
                      (scoreData->g_clef ? false : renderInfo.show_measure_number) /* if we have both keys don't show twice */);
        }
        
        delete x_converter;
        x_converter = NULL;
    }

    
#if 0
#pragma mark -
#pragma mark ScorePrintable (private utils)
#endif
    
    void ScorePrintable::gatherScoreInfo(LayoutLine& line)
    {
        if(line.editor_data != NULL) return; // already set
        
        LayoutLine* previousLine = currentLine;
        currentLine = &line;
        
        ScoreData* scoreData = new ScoreData();
        line.editor_data = scoreData;
        
        Track* track = line.getTrack();
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        
        scoreData->from_note = line.getFirstNote();
        scoreData->to_note   = line.getLastNote();
        
        // find highest and lowest note we need to render
        int highest_pitch = -1, lowest_pitch = -1;
        int biggest_level = -1, smallest_level = -1;
        std::cout << "line : reading notes " << scoreData->from_note << " to " << scoreData->to_note << std::endl;
        for(int n=scoreData->from_note; n<=scoreData->to_note; n++)
        {
            if(n == -1) break; // will happen if line is empty
            const int pitch = track->getNotePitchID(n);
            const int level = converter->noteToLevel(track->getNote(n));
            if(pitch < highest_pitch || highest_pitch == -1)
            {
                highest_pitch = pitch;
                smallest_level = level;
            }
            if(pitch > lowest_pitch  ||  lowest_pitch == -1)
            {
                lowest_pitch  = pitch;
                biggest_level  = level;
            }
        }
        
        scoreData->middle_c_level = converter->getScoreCenterCLevel(); //converter->getMiddleCLevel();
        
        const int g_clef_from = scoreData->middle_c_level-10;
        const int g_clef_to   = scoreData->middle_c_level-2;
        const int f_clef_from = scoreData->middle_c_level+2;
        const int f_clef_to   = scoreData->middle_c_level+10;
        
        scoreData->g_clef = scoreEditor->isGClefEnabled();
        scoreData->f_clef = scoreEditor->isFClefEnabled();
        
        
        scoreData->extra_lines_above_g_score = 0;
        scoreData->extra_lines_under_g_score = 0;
        scoreData->extra_lines_above_f_score = 0;
        scoreData->extra_lines_under_f_score = 0;
        if(scoreData->g_clef and not scoreData->f_clef)
        {
            if(smallest_level!=-1 and smallest_level < g_clef_from)  scoreData->extra_lines_above_g_score = (g_clef_from - smallest_level)/2;
            if(biggest_level!=-1 and biggest_level > g_clef_to) scoreData->extra_lines_under_g_score = (g_clef_to - biggest_level)/2;
        }
        else if(scoreData->f_clef and not scoreData->g_clef)
        {
            if(smallest_level!=-1 and smallest_level < f_clef_from) scoreData->extra_lines_above_f_score = (f_clef_from - smallest_level)/2;
            if(biggest_level!=-1 and biggest_level > f_clef_to) scoreData->extra_lines_under_f_score = (f_clef_to - biggest_level)/2;
        }
        else if(scoreData->f_clef and scoreData->g_clef)
        {
            if(smallest_level!=-1 and smallest_level < g_clef_from) scoreData->extra_lines_above_g_score = (g_clef_from - smallest_level)/2;
            if(biggest_level!=-1 and biggest_level > f_clef_to) scoreData->extra_lines_under_f_score = (f_clef_to - biggest_level)/2;
        }
        
        std::cout << "extra_lines_above_g_score = " << scoreData->extra_lines_above_g_score <<
            " extra_lines_under_g_score = " << scoreData->extra_lines_under_g_score <<
            " extra_lines_above_f_score = " << scoreData->extra_lines_above_f_score <<
            " extra_lines_under_f_score = " << scoreData->extra_lines_under_f_score << std::endl;
        
        // Split space between both scores (one may need more than the other)
        // I use a total of 0.8 to leave a 0.2 free space between both scores.
        scoreData->first_clef_proportion = 0.4;
        scoreData->second_clef_proportion = 0.4;
        
        if(scoreData->g_clef and scoreData->f_clef and
           scoreData->extra_lines_above_g_score + scoreData->extra_lines_under_f_score != 0 /* unnecessary if nothing under/over scores*/)
        {
            /*  where 0.8 is used to leave a 0.2 margin between both scores. 5 is the amount of lines needed for the regular score.
             10 is the amount of lines needed for both regular scores */
            const float total_height = abs(scoreData->extra_lines_above_g_score) + abs(scoreData->extra_lines_under_f_score) + 10.0;
            scoreData->first_clef_proportion = 0.8 * (abs(scoreData->extra_lines_above_g_score)+5.0) / total_height;
            scoreData->second_clef_proportion = 0.8 * (abs(scoreData->extra_lines_under_f_score)+5.0) / total_height;
        }

        
        // restore previous current lien (FIXME not too pretty)
        currentLine = previousLine;

    }
    
    namespace PrintStemParams
    {
        int stem_up_x_offset;
        float stem_up_y_offset;
        int stem_down_x_offset;
        float stem_down_y_offset;
        int note_x_shift = 0;
        
        const int getStemX(const int tick, const STEM stem_type)
        {
            const int noteX = x_converter->tickToX(tick);
            
            if     (stem_type == STEM_UP)   return (noteX + stem_up_x_offset + note_x_shift);
            else if(stem_type == STEM_DOWN) return (noteX + stem_down_x_offset + note_x_shift);
            else return -1;
        }
        const int getStemX(const NoteRenderInfo& noteRenderInfo)
        {
            return getStemX(noteRenderInfo.tick, noteRenderInfo.stem_type);
        }
    }
    using namespace PrintStemParams;
    
    
    /**
     * Goes a bit further that 'gatherScoreInfo' : builds analyzers objects and gathers notes. Does
     * not perform any actual analysis with the ScoreAnalyser(s)
     */
    void ScorePrintable::gatherNotesAndBasicSetup(LayoutLine& line)
    {
        LayoutLine* previousLine = currentLine;
        currentLine = &line;
        
        ScoreData* scoreData = dynamic_cast<ScoreData*>(line.editor_data.raw_ptr);
        assert(scoreData != NULL);
        
        Track* track = line.getTrack();
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        

        // ---- Build score analyzers
        if(scoreData->g_clef)
        {
            scoreData->g_clef_analyser = new ScoreAnalyser(scoreEditor, new PrintXConverter(this), scoreData->middle_c_level-5);
            scoreData->g_clef_analyser->setStemPivot(scoreData->middle_c_level-5);
        }
        if(scoreData->f_clef)
        {
            scoreData->f_clef_analyser = new ScoreAnalyser(scoreEditor, new PrintXConverter(this), scoreData->middle_c_level-5);
            scoreData->f_clef_analyser->setStemPivot(scoreData->middle_c_level+6);
        }

        converter->updateConversionData();
        converter->resetAccidentalsForNewRender();
        
        // iterate through layout elements to collect notes in the vector
        // so ScoreAnalyser can prepare the score
        LayoutElement* currentElement;
        std::vector<LayoutElement>& layoutElements = line.layoutElements;
        const int amount = layoutElements.size();
        for(int el=0; el<amount; el++)
        {
            currentElement = &layoutElements[el];
            
            // we're collecting notes here... types other than regular measures
            // don't contain notes and thus don't interest us
            if(currentElement->getType() != SINGLE_MEASURE) continue;
            
            const int firstNote = line.getFirstNoteInElement(currentElement);
            const int lastNote = line.getLastNoteInElement(currentElement);
            
            if(firstNote == -1 or lastNote == -1) continue; // empty measure
            
            for(int n=firstNote; n<=lastNote; n++)
            {
                PitchSign note_sign;
                const int noteLevel = converter->noteToLevel(track->getNote(n), &note_sign);
                
                if(noteLevel == -1) continue;
                const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
                const int tick = track->getNoteStartInMidiTicks(n);
                
                const int note_x = tickToX(tick);
                
                NoteRenderInfo currentNote(tick, note_x, noteLevel, noteLength, note_sign,
                                           track->isNoteSelected(n), track->getNotePitchID(n));
                
                // add note to either G clef score or F clef score
                if(scoreData->g_clef and not scoreData->f_clef)
                {
                    scoreData->g_clef_analyser->addToVector(currentNote, false);
                }
                else if(scoreData->f_clef and not scoreData->g_clef)
                {
                    scoreData->f_clef_analyser->addToVector(currentNote, false);
                }
                else if(scoreData->f_clef and scoreData->g_clef)
                {
                    if(noteLevel < scoreData->middle_c_level)
                        scoreData->g_clef_analyser->addToVector(currentNote, false);
                    else
                        scoreData->f_clef_analyser->addToVector(currentNote, false);
                }
            }
            
        }//next element

        // FIXME : ugly mechanism to retain the same currentLine
        currentLine = previousLine;
    }
    
    
    void ScorePrintable::analyseAndDrawScore(bool f_clef, ScoreAnalyser& analyser, LayoutLine& line, wxDC& dc,
                                   const int extra_lines_above, const int extra_lines_under,
                                   const int x0, const int y0, const int x1, const int y1,
                                   bool show_measure_number)
    {
        Track* track = line.getTrack();
        ScoreEditor* scoreEditor = track->graphics->scoreEditor;
        ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
        const int middle_c_level = converter->getScoreCenterCLevel();
        const int first_score_level = middle_c_level + (f_clef? 2 : -10);
        const int last_score_level = first_score_level + 8;
        const int min_level =  first_score_level - extra_lines_above*2;
        const int headRadius = 36; //(int)round(lineHeight*0.8);

        note_x_shift = headRadius + 36; // shift by a 'headRadius', since note will be drawn centered on its X and not left-aligned
        
        stem_up_x_offset = headRadius - 10;
        stem_up_y_offset = 0;
        stem_down_x_offset = -headRadius + 4;
        stem_down_y_offset = 0;
        
        analyser.setStemDrawInfo( stem_up_x_offset, stem_up_y_offset, stem_down_x_offset, stem_down_y_offset );
        
#define LEVEL_TO_Y( lvl ) y0 + 1 + lineHeight*0.5*(lvl - min_level)
                
        
        // ------------------ preliminary part : backgrounds -----------------

        // ---- draw score background (horizontal lines)
        dc.SetPen(  wxPen( wxColour(125,125,125), 7 ) );
        const int lineAmount = 5 + extra_lines_above + extra_lines_under;
        const float lineHeight = (float)(y1 - y0) / (float)(lineAmount-1);
        
        for(int lvl=first_score_level; lvl<=last_score_level; lvl+=2)
        {
            const int y = LEVEL_TO_Y(lvl);
            dc.DrawLine(x0, y, x1, y);
        }
        
        // ---- render vertical dividers and time signature changes
        const int measure_dividers_from_y = LEVEL_TO_Y(first_score_level);
        const int measure_dividers_to_y = LEVEL_TO_Y(last_score_level);
        
        const int elamount = line.layoutElements.size();
        for(int n=0; n<elamount; n++)
        {
            drawVerticalDivider(&line.layoutElements[n], measure_dividers_from_y, measure_dividers_to_y);
            
            if(line.layoutElements[n].getType() == TIME_SIGNATURE)
            {
                EditorPrintable::renderTimeSignatureChange(&line.layoutElements[n], LEVEL_TO_Y(first_score_level), LEVEL_TO_Y(last_score_level));
            }
        }
        
        // ---- line header if any
        if(line.layoutElements[0].getType() == LINE_HEADER)
        {
            if(!f_clef) renderGClef(dc, line.layoutElements[0].x,
                                    LEVEL_TO_Y(last_score_level)+10,
                                    LEVEL_TO_Y(last_score_level-4)-5);
            else renderFClef(dc, line.layoutElements[0].x,
                             LEVEL_TO_Y(first_score_level),
                             LEVEL_TO_Y(first_score_level+3));
            
            // key
            // on which level to put the signs (0 is the level of the highest sign when all are shown)
            const unsigned short int sharp_sign_lvl[] = { 1, 4, 0, 3, 6, 2, 5 };
            const unsigned short int flat_sign_lvl[] = { 3, 0, 4, 1, 5, 2, 6 };
            
            const int sharps = track->graphics->getCurrentEditor()->getKeySharpsAmount();
            const int flats = track->graphics->getCurrentEditor()->getKeyFlatsAmount();
            assertExpr(line.layoutElements.size(),>=,0);
            
            if(sharps > 0 or flats > 0)
            {
                int x_space_per_symbol = (line.layoutElements[0].x2 - line.layoutElements[0].x
                                          - 300 - 50 /* some additional space */) / std::max(sharps, flats);
                if(x_space_per_symbol > 50) x_space_per_symbol = 50;
                
                for(int n=0; n<sharps; n++)
                {
                    const int level = first_score_level + (f_clef ? 1 : -1) + sharp_sign_lvl[n];
                    renderSharp( dc, line.layoutElements[0].x+300+n*x_space_per_symbol, LEVEL_TO_Y(level) );
                }
                for(int n=0; n<flats; n++)
                {
                    const int level = first_score_level + (f_clef ? 3 : 1) + flat_sign_lvl[n];
                    renderFlat( dc, line.layoutElements[0].x+300+n*x_space_per_symbol, LEVEL_TO_Y(level) );
                }
            }
            
            // also show ottava bassa/alta if relevant
            const int octave_shift = track->graphics->scoreEditor->getScoreMidiConverter()->getOctaveShift(); // FIXME - woot
            if(octave_shift > 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(first_score_level - 3);
                dc.DrawText(wxT("8va"), line.layoutElements[0].x+200, y);
            }
            else if(octave_shift < 0)
            {
                dc.SetTextForeground( wxColour(0,0,0) );
                const int y = LEVEL_TO_Y(last_score_level);
                dc.DrawText(wxT("8vb"), line.layoutElements[0].x+200, y);
            }
        }
        
        // ------------------ first part : basic unintelligent drawing -----------------
        // for all parts of score notes that can be rendered without using the ScoreAnalyser.
        
        // ---- draw notes heads
        { // we scope this because info like 'noteAmount' are bound to change just after
            const int noteAmount = analyser.noteRenderInfo.size();
            for(int i=0; i<noteAmount; i++)
            {
                NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
                                
                dc.SetPen(  wxPen( wxColour(125,125,125), 8 ) );
                // draw small lines above score if needed
                if(noteRenderInfo.level < first_score_level-1)
                {
                    for(int lvl=first_score_level-2; lvl>noteRenderInfo.level+noteRenderInfo.level%2-2; lvl -= 2)
                    {
                        const int y = LEVEL_TO_Y(lvl);
                        dc.DrawLine(noteRenderInfo.x, y, noteRenderInfo.x+200, y);
                    }
                }
                
                // draw small lines below score if needed
                if(noteRenderInfo.level > last_score_level+1)
                {
                    for(int lvl=last_score_level+2; lvl<noteRenderInfo.level-noteRenderInfo.level%2+2; lvl += 2)
                    {
                        const int y = LEVEL_TO_Y(lvl);
                        dc.DrawLine(noteRenderInfo.x, y, noteRenderInfo.x+200, y);
                    }
                }
                
                // draw head
                const int notey = LEVEL_TO_Y(noteRenderInfo.getBaseLevel());
                wxPoint headLocation( noteRenderInfo.x + note_x_shift,
                                     notey-(headRadius-5)/2.0); // FIXME....
                
                if(noteRenderInfo.instant_hit)
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
                    for(int n=0; n<25; n++)
                    {
                        // FIXME - instead of always substracting to radius, just make it smaller...
                        const float angle = n/25.0*6.283185f /* 2*PI */;
                        points[n] = wxPoint( cx + (headRadius-5)*cos(angle),
                                             cy + headRadius/2 + (headRadius - 14)*sin(angle) - headRadius*(-0.5f + fabsf( (n-12.5f)/12.5f ))/2.0f );
                    }

                    if(noteRenderInfo.hollow_head) dc.DrawSpline(25, points);
                    else dc.DrawPolygon(25, points, -3);
                    
                }
                noteRenderInfo.setY(notey+headRadius/2.0);
                
                // draw dot if note is dotted
                if(noteRenderInfo.dotted)
                {
                    // FIXME - what's that?? 4 head radiuses to get it visible?? coords are really screwed up
                    wxPoint headLocation( noteRenderInfo.x + headRadius*3 + 20, notey+10 );
                    dc.DrawEllipse( headLocation, wxSize(10,10) );
                }
                
                // draw sharpness sign if relevant
                if(noteRenderInfo.sign == SHARP)        renderSharp  ( dc, noteRenderInfo.x, noteRenderInfo.getY() - 40  );
                else if(noteRenderInfo.sign == FLAT)    renderFlat   ( dc, noteRenderInfo.x, noteRenderInfo.getY() - 40  );
                else if(noteRenderInfo.sign == NATURAL) renderNatural( dc, noteRenderInfo.x, noteRenderInfo.getY() - 50  );
                
            } // next note
        }// end scope
        
        // ------------------ second part : intelligent drawing of the rest -----------------
        // analyse notes to know how to build the score
        analyser.analyseNoteInfo();
        
        // now that score was analysed, draw the remaining note bits
        const int noteAmount = analyser.noteRenderInfo.size();
        for(int i=0; i<noteAmount; i++)
        {
            NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
            
            dc.SetPen(  wxPen( wxColour(0,0,0), 15 ) );
            
            // draw stem
            if(noteRenderInfo.stem_type != STEM_NONE)
            {                
                dc.DrawLine( getStemX(noteRenderInfo), LEVEL_TO_Y(analyser.getStemFrom(noteRenderInfo)),
                             getStemX(noteRenderInfo), LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo))    );
            }
            
            // draw flags
            if(not noteRenderInfo.instant_hit and noteRenderInfo.flag_amount>0 and not noteRenderInfo.beam)
            {
                const int stem_end = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
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
            if(noteRenderInfo.getTiedToPixel() != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                const bool show_above = noteRenderInfo.isTieUp();
                const int base_y = LEVEL_TO_Y( noteRenderInfo.getStemOriginLevel() ) + (show_above ? - 30 : 60);
                
                const int center_x = (noteRenderInfo.getTiedToPixel() + noteRenderInfo.x)/2 + headRadius*2;
                const int radius_x = abs(noteRenderInfo.getTiedToPixel() - noteRenderInfo.x)/2;
                renderArc(dc, center_x, base_y, radius_x, show_above ? -50 : 50);
            }
            
            // beam
            if(noteRenderInfo.beam)
            {
                dc.SetPen(  wxPen( wxColour(0,0,0), 10 ) );
                dc.SetBrush( *wxBLACK_BRUSH );
                
                const int x1 = getStemX(noteRenderInfo);
                int y1       = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
                int y2       = LEVEL_TO_Y(noteRenderInfo.beam_to_level);
                
                const int y_diff = (noteRenderInfo.stem_type == STEM_UP ? 55 : -55);
                
                const int beam_to_x = getStemX(noteRenderInfo.beam_to_tick, noteRenderInfo.stem_type);
                
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
            if (noteRenderInfo.drag_triplet_sign and noteRenderInfo.triplet_arc_x_start != -1)
            {
                wxPen tiePen( wxColour(0,0,0), 10 ) ;
                dc.SetPen( tiePen );
                dc.SetBrush( *wxTRANSPARENT_BRUSH );
                
                const int center_x = (noteRenderInfo.triplet_arc_x_end == -1 ?
                                      noteRenderInfo.triplet_arc_x_start :
                                      (noteRenderInfo.triplet_arc_x_start + noteRenderInfo.triplet_arc_x_end)/2) + headRadius;
                const int radius_x = (noteRenderInfo.triplet_arc_x_end == -1 or  noteRenderInfo.triplet_arc_x_end == noteRenderInfo.triplet_arc_x_start ?
                                      100 : (noteRenderInfo.triplet_arc_x_end - noteRenderInfo.triplet_arc_x_start)/2);
                
                const int base_y = LEVEL_TO_Y(noteRenderInfo.triplet_arc_level) + (noteRenderInfo.triplet_show_above ? -80 : 90);
                
                renderArc(dc, center_x + headRadius*1.2, base_y, radius_x, noteRenderInfo.triplet_show_above ? -80 : 80);
                dc.SetTextForeground( wxColour(0,0,0) );
                dc.DrawText( wxT("3"), center_x + headRadius/2, base_y + (noteRenderInfo.triplet_show_above ? -75 : -20) );
            }
            
            
        } // next note
        
        g_printable = this;
        
        // render silences
        const int first_measure = line.getFirstMeasure();
        const int last_measure  = line.getLastMeasure();
        
        global_dc = &dc;
        
        if(f_clef)
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level + 4);
            global_line_height = lineHeight;
            analyser.renderSilences( &renderSilenceCallback, first_measure, last_measure, silences_y );
        }
        else
        {
            const int silences_y = LEVEL_TO_Y(middle_c_level - 8);
            global_line_height = lineHeight;
            analyser.renderSilences( &renderSilenceCallback, first_measure, last_measure, silences_y );
        }
        
    }
    
}
