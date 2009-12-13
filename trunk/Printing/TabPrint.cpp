
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "Midi/MeasureData.h"
#include "Editors/GuitarEditor.h"
#include "IO/IOUtils.h"
#include "Printing/TabPrint.h"
#include "Printing/PrintingBase.h"
#include "Printing/PrintLayout.h"
#include "Printing/PrintLayoutMeasure.h"
#include "Printing/PrintLayoutLine.h"

namespace AriaMaestosa
{
    
    
    TablaturePrintable::TablaturePrintable(Track* track) : EditorPrintable()
    {
        // FIXME  - will that work if printing e.g. a bass track + a guitar track,
        // both with different string counts?
        string_amount = track->graphics->guitarEditor->tuning.size();
        editor = track->graphics->guitarEditor;
    }
    
    TablaturePrintable::~TablaturePrintable()
    {
    }
    
    void TablaturePrintable::drawLine(const int trackID, LineTrackRef& renderInfo, LayoutLine& line, wxDC& dc)
    {
        assertExpr(renderInfo.y0,>,0);
        assertExpr(renderInfo.y1,>,0);
        assertExpr(renderInfo.y0,<,50000);
        assertExpr(renderInfo.y1,<,50000);
        
        wxFont oldfont = dc.GetFont();
                
        std::cout << "Tablature : starting new line\n";
        
        
        // draw tab background (guitar strings)
        dc.SetPen(  wxPen( wxColour(125,125,125), 5 ) );
        
        const float stringHeight = (float)(renderInfo.y1 - renderInfo.y0) / (float)(string_amount-1);
        
        for(int s=0; s<string_amount; s++)
        {
            const int y = (int)round(renderInfo.y0 + stringHeight*s);
            dc.DrawLine(renderInfo.x0, y, renderInfo.x1, y);
        }
        
        setCurrentDC(&dc);
        
        // iterate through layout elements
        LayoutElement* currentElement;
        
        const int elementAmount = line.getElementCount(trackID);
        for(int el=0; el<elementAmount; el++)
        {
            currentElement = continueWithNextElement(trackID, line, el);
            std::cout << "Tablature : starting new layout element. Type = " << currentElement->getType() << "\n";
            
            drawVerticalDivider(currentElement, renderInfo.y0, renderInfo.y1);
            
            if (currentElement->getType() == LINE_HEADER)
            {
                std::cout << "Tablature : it's a header\n";
                
                dc.SetFont( wxFont(100,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD) );
                dc.SetTextForeground( wxColour(0,0,0) );
                
                // wxSize textSize = dc.GetTextExtent( wxT("T") );
                const int h4 = (renderInfo.y1 - renderInfo.y0)/3 - 2;    
                const int textY = renderInfo.y0;
                
                dc.DrawText( wxT("T") , currentElement->getXFrom()+20, textY);
                dc.DrawText( wxT("A") , currentElement->getXFrom()+20, textY + h4  );
                dc.DrawText( wxT("B") , currentElement->getXFrom()+20, textY + h4*2 );
                
                //dc.SetFont( wxFont(50, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
                dc.SetFont(oldfont);
                wxSize textSize2 = dc.GetTextExtent( wxT("T") );
                
                // draw tuning
                const int tuning_x = currentElement->getXFrom()+140;
                for(int n=0; n<string_amount; n++)
                {
                    const int note   = editor->tuning[n]%12;
                    wxString label;
                    switch(note)
                    {
                        case 0:  label = wxT("B");  break;
                        case 1:  label = wxT("A#"); break;
                        case 2:  label = wxT("A");  break;
                        case 3:  label = wxT("G#"); break;
                        case 4:  label = wxT("G");  break;
                        case 5:  label = wxT("F#"); break;
                        case 6:  label = wxT("F");  break;
                        case 7:  label = wxT("E");  break;
                        case 8:  label = wxT("D#"); break;
                        case 9:  label = wxT("D");  break;
                        case 10: label = wxT("C#"); break;
                        case 11: label = wxT("C");  break;
                    } // end switch
                    dc.DrawText( label, tuning_x, renderInfo.y0 + n*stringHeight - textSize2.y/2 );
                }//next
                
                continue;
            }
            if (currentElement->getType() == TIME_SIGNATURE)
            {
                std::cout << "Tablature : it's a time sig\n";
                
                EditorPrintable::renderTimeSignatureChange(currentElement, renderInfo.y0, renderInfo.y1);
                continue;
            }
            
            if (currentElement->getType() != SINGLE_MEASURE)
            {
                std::cout << "Tablature : it's something else we won't draw : ";
                if (currentElement->getType() == SINGLE_REPEATED_MEASURE) std::cout << "SINGLE_REPEATED_MEASURE\n";
                if (currentElement->getType() == EMPTY_MEASURE) std::cout << "EMPTY_MEASURE\n";
                if (currentElement->getType() == REPEATED_RIFF) std::cout << "REPEATED_RIFF\n";
                if (currentElement->getType() == PLAY_MANY_TIMES) std::cout << "PLAY_MANY_TIMES\n";
                continue;
            }
            
            // for layout elements containing notes, render them
            const int firstNote = line.getFirstNoteInElement(trackID, currentElement);
            const int lastNote = line.getLastNoteInElement(trackID, currentElement);
            
            if (firstNote == -1 || lastNote == -1)
            {
                std::cout << "Tablature : it's an empty measure\n";
                continue; // empty measure
            }
            
            dc.SetFont( wxFont(75, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD) );
            wxSize textSize3 = dc.GetTextExtent( wxT("X") );
            
            std::cout << "Tablature : drawing notes " << firstNote << " to " << lastNote << std::endl;
            
            for (int i=firstNote; i<=lastNote; i++)
            {
                const int string = renderInfo.track->getNoteString(i);
                const int fret = renderInfo.track->getNoteFret(i);
                
                if (fret < 0)  dc.SetTextForeground( wxColour(255,0,0) );
                
                // substract from width to leave some space on the right (coordinate is from the left of the text string so we need extra space on the right)
                // if fret number is greater than 9, the string will have two characters so we need to recenter it a bit more
                const int drawX = getNotePrintX(trackID, line, i).from + (fret > 9 ? renderInfo.pixel_width_of_an_unit/4 : renderInfo.pixel_width_of_an_unit/2);
                const int drawY = renderInfo.y0 + stringHeight*string - textSize3.y/2;
                wxString label = to_wxString(fret);
                
                dc.DrawText( label, drawX, drawY );
                
                if (fret < 0)  dc.SetTextForeground( wxColour(0,0,0) );
            }
            
            dc.SetFont(oldfont);
        }//next element 
                
        // ---- Debug guides
        if (PRINT_LAYOUT_HINTS)
        {
            dc.SetPen( wxPen(*wxBLUE, 7) );
            dc.DrawLine(renderInfo.x0, renderInfo.y0, renderInfo.x1, renderInfo.y0);
            dc.DrawLine(renderInfo.x0, renderInfo.y1, renderInfo.x1, renderInfo.y1);
            dc.DrawLine(renderInfo.x0, renderInfo.y0, renderInfo.x0, renderInfo.y1);
            dc.DrawLine(renderInfo.x1, renderInfo.y0, renderInfo.x1, renderInfo.y1);
        }
    }
    
    void TablaturePrintable::addUsedTicks(const PrintLayoutMeasure& measure, const MeasureTrackReference& trackRef,
                                          std::map<int /* tick */,TickPosInfo>& ticks_relative_position)
    {
        const int first_note = trackRef.firstNote;
        const int last_note = trackRef.lastNote;
        
        Track* track = trackRef.track;
        
        if (first_note == -1 or last_note == -1) return; // empty measure
        
        // find shortest note
        int shortest = -1;
        
        for (int n=first_note; n<=last_note; n++)
        {
            int noteLen = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);

            if (noteLen < shortest or shortest == -1) shortest = noteLen;
        }
        

        for (int n=first_note; n<=last_note; n++)
        {
            const int tick = track->getNoteStartInMidiTicks(n);
            
            int noteLen = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
            
            // wider notes should be given a bit more space.
            float ratioToShortest = (float)noteLen / (float)shortest;
            float additionalWidth = log( ratioToShortest ) / log( 2 );
            
            ticks_relative_position[ tick ].setProportion(2 + additionalWidth);
        }
    }
    
    int TablaturePrintable::calculateHeight(const int trackID, LineTrackRef& lineTrack, LayoutLine& line)
    {
        const int from_note = lineTrack.getFirstNote();
        const int to_note   = lineTrack.getLastNote();
        
        // check if empty
        if (from_note == -1 || to_note == -1)
            return 0;

        return string_amount;
    }  
    
}
