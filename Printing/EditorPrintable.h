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

#ifndef _editor_printable_h_
#define _editor_printable_h_

#include <map>
#include "Printing/PrintLayout.h"

class wxDC;

namespace AriaMaestosa
{

class LayoutLine;
class LayoutElement;
class LineTrackRef;
class PrintLayoutMeasure;
class MeasureTrackReference;
class Track;
    
    template<typename T>
    class Range
    {
    public:
        T from;
        T to;
        
        Range(T from, T to)
        {
            Range::from = from;
            Range::to = to;
        }
    };
    
    class EditorPrintable
    {
    protected:
        wxDC* dc;
        int getClosestTickFrom(const int trackID, LayoutLine& line, const int tick);

    public:
        EditorPrintable();
        virtual ~EditorPrintable();
        
        /** Called by the print code when it's time to render a line. This will be handled in the appropriate subclass.
         */
        virtual void drawLine(const int trackID, LineTrackRef& track, LayoutLine& line, wxDC& dc) = 0;
        
        /** Called by the layout code to know the relative height of this line
         */
        virtual int calculateHeight(const int trackID, LineTrackRef& renderInfo, LayoutLine& line) = 0;
                
        void setCurrentTrack(LayoutLine* line);
        
        void setCurrentDC(wxDC* dc);
        
        void placeTrackAndElementsWithinCoords(const int trackID, LayoutLine& line, LineTrackRef& track,
                                               int x0, const int y0, const int x1, const int y1, bool show_measure_number);
        
        // int getCurrentElementXStart();
        LayoutElement* continueWithNextElement(const int trackID, LayoutLine& layoutLine, const int currentLayoutElement);
        //LayoutElement* getElementForMeasure(const int measureID);
        Range<int> getNotePrintX(const int trackID, LayoutLine& line, int noteID);
        Range<int> tickToX(const int trackID, LayoutLine& line, const int tick);
        //int tickToXLimit(const int trackID, LayoutLine& line, const int tick);

        void drawVerticalDivider(LayoutElement* el, const int y0, const int y1);
        void renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1);
        
        virtual void earlySetup(const int trackID, Track* track) {}
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID, const MeasureTrackReference& trackRef, std::map< int /* tick */, TickPosInfo >&) { }
    };
}

#endif