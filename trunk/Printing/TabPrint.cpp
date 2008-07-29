
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

namespace AriaMaestosa
{
    
    
TablaturePrintable::TablaturePrintable(Track* track) : EditorPrintable()
{
    // FIXME  - will that work if printing e.g. a bass track + a guitar track,
    // both with different string counts?
    string_amount = track->graphics->guitarEditor->tuning.size();
}

TablaturePrintable::~TablaturePrintable()
{
}

void TablaturePrintable::drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1, bool show_measure_number)
{
    Track* track = line.getTrack();
    
    // draw tab background (guitar strings)
    dc.SetPen(  wxPen( wxColour(125,125,125), 1 ) );
    
    const float stringHeight = (float)(y1 - y0) / (float)(string_amount-1);
    
    for(int s=0; s<string_amount; s++)
    {
        const int y = (int)round(y0 + stringHeight*s);
        dc.DrawLine(x0, y, x1, y);
    }
    
    beginLine(&dc, &line, x0, y0, x1, y1, show_measure_number);
    
    // iterate through layout elements
    LayoutElement* currentElement;
    while((currentElement = getNextElement()) and (currentElement != NULL))
    {
        if(currentElement->type != SINGLE_MEASURE) continue;
        
        // for layout elements containing notes, render them
        const int firstNote = line.getFirstNoteInElement(currentElement);
        const int lastNote = line.getLastNoteInElement(currentElement);

        for(int i=firstNote; i<lastNote; i++)
        {
            const int string = track->getNoteString(i);
            const int fret = track->getNoteFret(i);
            
            if(fret < 0)  dc.SetTextForeground( wxColour(255,0,0) );
            
            // substract from width to leave some space on the right (coordinate is from the left of the text string so we need extra space on the right)
            // if fret number is greater than 9, the string will have two characters so we need to recenter it a bit more
            const int drawX = getNotePrintX(i) + (fret > 9 ? widthOfAChar/4 : widthOfAChar/2);
            const int drawY = y0 + stringHeight*string - getCurrentPrintable()->text_height_half*0.8;
            wxString label = to_wxString(fret);
            
            dc.DrawText( label, drawX, drawY );
            
            if(fret < 0)  dc.SetTextForeground( wxColour(0,0,0) );
        }
        
    }//next element
}


}
