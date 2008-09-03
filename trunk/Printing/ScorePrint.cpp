
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
    
    
ScorePrintable::ScorePrintable(Track* track) : EditorPrintable()
{

}

ScorePrintable::~ScorePrintable()
{
}

class PrintXConverter : public TickToXConverter
{
    ScorePrintable* parent;
public:
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

// FIXME - F key, notes above/below score, etc... --> variable score height needed
void ScorePrintable::drawLine(LayoutLine& line, wxDC& dc,
                              const int x0, const int y0,
                              const int x1, const int y1,
                              bool show_measure_number)
{
    Track* track = line.getTrack();
    ScoreEditor* scoreEditor = track->graphics->scoreEditor;
    ScoreMidiConverter* converter = scoreEditor->getScoreMidiConverter();
    
    const int middleC = converter->getMiddleCLevel();
    const int lineAmount = 5;
    
#define LEVEL_TO_Y( lvl ) y0 + 1 + lineHeight*0.5*(lvl - middleC + 10)
    
    // draw score background (lines)
    dc.SetPen(  wxPen( wxColour(125,125,125), 1 ) );
    
    const float lineHeight = (float)(y1 - y0) / (float)(lineAmount-1);
    const int headRadius = 9;//(int)round((float)lineHeight*0.72);
    
    for(int s=0; s<lineAmount; s++)
    {
        const int y = (int)round(y0 + lineHeight*s);
        dc.DrawLine(x0, y, x1, y);
    }
    
    beginLine(&dc, &line, x0, y0, x1, y1, show_measure_number);
    
    ScoreAnalyser analyser(scoreEditor, new PrintXConverter(this), middleC-5);
    analyser.setStemSize( 19, 0, 9, 0 );
    
    // iterate through layout elements to collect notes in the vector
    // so ScoreAnalyser can prepare the score
    
    // FIXME - handle both clefs + variable height
    
    /*
     if(g_clef)
     {
         setUpDownPivotLevel(converter->getScoreCenterCLevel()-5);
         const int silences_y = getEditorYStart() + y_step*(converter->getScoreCenterCLevel()-8) - getYScrollInPixels() + 1;
         renderScore(noteRenderInfo_GClef, silences_y);
     }
     
     if(f_clef)
     {
         setUpDownPivotLevel(converter->getScoreCenterCLevel()+6);
         const int silences_y = getEditorYStart() + y_step*(converter->getScoreCenterCLevel()+4) - getYScrollInPixels() + 1;
         renderScore(noteRenderInfo_FClef, silences_y);
     }
     */    
    
    LayoutElement* currentElement;
    while((currentElement = getNextElement()) and (currentElement != NULL))
    {
        if(currentElement->type != SINGLE_MEASURE) continue;
        
        const int firstNote = line.getFirstNoteInElement(currentElement);
        const int lastNote = line.getLastNoteInElement(currentElement);
        
        // for layout elements containing notes, render them
        for(int n=firstNote; n<lastNote; n++)
        {
            int note_sign;
            const int noteLevel = converter->noteToLevel(track->getNote(n), &note_sign);
            
            if(noteLevel == -1) continue;
            const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
            const int tick = track->getNoteStartInMidiTicks(n);
            
            const int note_x = tickToX(tick);
            
			NoteRenderInfo currentNote(tick, note_x, noteLevel, noteLength, note_sign,
                                       track->isNoteSelected(n), track->getNotePitchID(n));
			analyser.addToVector(currentNote, false);   
        }
        
    }//next element
    
    dc.SetPen(  wxPen( wxColour(0,0,0), 2 ) );
    
    // draw note head
    {
    const int noteAmount = analyser.noteRenderInfo.size();
    for(int i=0; i<noteAmount; i++)
    {
        NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
        
        if(noteRenderInfo.hollow_head) dc.SetBrush( *wxTRANSPARENT_BRUSH );
        else dc.SetBrush( *wxBLACK_BRUSH );
        
        const int notey = LEVEL_TO_Y(noteRenderInfo.getBaseLevel());
        wxPoint headLocation( noteRenderInfo.x + headRadius, notey-headRadius/2.0 );
        dc.DrawEllipse( headLocation, wxSize(headRadius+1, headRadius) );
        noteRenderInfo.setY(notey+headRadius/2.0);
    }
    }
    
    // analyse notes to know how to build the score
    analyser.analyseNoteInfo();
    
    const int noteAmount = analyser.noteRenderInfo.size();
    for(int i=0; i<noteAmount; i++)
    {
        NoteRenderInfo& noteRenderInfo = analyser.noteRenderInfo[i];
        
        dc.SetPen(  wxPen( wxColour(0,0,0), 2 ) );
        
        // draw stem
        if(noteRenderInfo.stem_type != STEM_NONE)
        {
            dc.DrawLine( analyser.getStemX(noteRenderInfo), LEVEL_TO_Y(analyser.getStemFrom(noteRenderInfo)),
                         analyser.getStemX(noteRenderInfo), LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo))    );
        }
        
        // drag flags
        if(noteRenderInfo.flag_amount>0 and not noteRenderInfo.beam)
        {
            const int stem_end = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
            const int flag_x_origin = analyser.getStemX(noteRenderInfo);
            const int flag_step = (noteRenderInfo.stem_type==STEM_UP ? 7 : -7 );
            
            for(int n=0; n<noteRenderInfo.flag_amount; n++)
            {
                const int flag_y = stem_end + n*flag_step;
                const int orient = (noteRenderInfo.stem_type==STEM_UP ? 1 : -1 );
                
                wxPoint points[] = 
                {
                    wxPoint(flag_x_origin, flag_y),
                    wxPoint(flag_x_origin + 3, flag_y + orient*6),
                    wxPoint(flag_x_origin + 11, flag_y + orient*11),
                    wxPoint(flag_x_origin + 9, flag_y + orient*15)
                };
                dc.DrawSpline(4, points);
            }
        }
        
        // ties
        if(noteRenderInfo.getTiedToPixel() != -1)
        {
            wxPen tiePen( wxColour(0,0,0), 1 ) ;
            //tiePen.SetJoin(wxJOIN_BEVEL);
            dc.SetPen( tiePen );
            dc.SetBrush( *wxTRANSPARENT_BRUSH );
            
            const bool show_above = noteRenderInfo.isTieUp();
            const int base_y = LEVEL_TO_Y( noteRenderInfo.getBaseLevel() ) + (show_above ? - 9 : 9); 
            
            // FIXME - when a note is split in two, it seems like some of the
            // new created ones receive the score's X coord and not the printerDC's
            dc.DrawEllipticArc(noteRenderInfo.x + headRadius*1.8,
                               base_y - 8,
                               noteRenderInfo.getTiedToPixel() - noteRenderInfo.x,
                               16,
                               (show_above ? 0   : 180),
                               (show_above ? 180 : 360));
        }

/*
            
            int center_x = noteRenderInfo.x + headRadius*1.8 + (noteRenderInfo.getTiedToPixel() - noteRenderInfo.x)/2;
            int center_y = base_y;
            int radius_x = (noteRenderInfo.getTiedToPixel() - noteRenderInfo.x)/2;
            int radius_y = (show_above ? -8 : 8);
            
            center_x *= 10;
            center_y *= 10;
            radius_x *= 10;
            radius_y *= 10;
            
            dc.SetUserScale(0.1, 0.1);
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
            dc.SetUserScale(1.0, 1.0);
 */
        // beam
        if(noteRenderInfo.beam)
        {
            const int x1 = analyser.getStemX(noteRenderInfo);
            int y1       = LEVEL_TO_Y(analyser.getStemTo(noteRenderInfo));
            int y2       = LEVEL_TO_Y(noteRenderInfo.beam_to_level);
            
            const int y_diff = (noteRenderInfo.stem_type == STEM_UP ? 5 : -5);
            
            for(int n=0; n<noteRenderInfo.flag_amount; n++)
            {
                dc.DrawLine(x1, y1, noteRenderInfo.beam_to_x, y2);
                y1 += y_diff;
                y2 += y_diff;
            }
        }
        
        /*    
                 
        // triplet
        if (renderInfo.drag_triplet_sign and renderInfo.triplet_arc_x_start != -1)
        {
            const int center_x = (renderInfo.triplet_arc_x_end == -1 ? renderInfo.triplet_arc_x_start : (renderInfo.triplet_arc_x_start + renderInfo.triplet_arc_x_end)/2);
            const int radius_x = (renderInfo.triplet_arc_x_end == -1 or  renderInfo.triplet_arc_x_end == renderInfo.triplet_arc_x_start ?
                                  10 : (renderInfo.triplet_arc_x_end - renderInfo.triplet_arc_x_start)/2);
            AriaRender::arc(center_x, LEVEL_TO_Y(renderInfo.triplet_arc_level) + (renderInfo.triplet_show_above ? 0 : 10), radius_x, 10, renderInfo.triplet_show_above);
            
            AriaRender::color(0,0,0);
            AriaRender::small_character('3', center_x-2, LEVEL_TO_Y(renderInfo.triplet_arc_level) + ( renderInfo.triplet_show_above? 0 : 18));
        }
        
        // tie
        if(renderInfo.getTiedToPixel() != -1)
        {
            const float center_x = (renderInfo.getTiedToPixel() + renderInfo.x)/2.0 + 6;
            const float radius_x = (renderInfo.getTiedToPixel() - renderInfo.x)/2.0;
            const bool show_above = renderInfo.isTieUp();
            
            const int base_y = renderInfo.getY() + head_radius; 
            AriaRender::arc(center_x, base_y + (show_above ? -5 : 5), radius_x, 6, show_above);
        }
        
        // beam
        if(renderInfo.beam)
        {
            AriaRender::color(0,0,0);
            AriaRender::lineWidth(2);
            
            const int x1 = analyser->getStemX(renderInfo);
            int y1       = LEVEL_TO_Y(analyser->getStemTo(renderInfo));
            int y2       = LEVEL_TO_Y(renderInfo.beam_to_level);
            
            const int y_diff = (renderInfo.stem_type == STEM_UP ? 5 : -5);
            
            AriaRender::lineSmooth(true);
            for(int n=0; n<renderInfo.flag_amount; n++)
            {
                AriaRender::line(x1, y1, renderInfo.beam_to_x, y2);
                y1 += y_diff;
                y2 += y_diff;
            }
            AriaRender::lineSmooth(false);
        }
*/        
        
    }
    
}


}
