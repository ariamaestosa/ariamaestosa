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

#ifndef _PRINT_LAYOUT_LINE_H_
#define _PRINT_LAYOUT_LINE_H_

#include "Config.h"

namespace AriaMaestosa
{
    class Track;
    class LayoutElement;
    class LayoutLine;
    class PrintLayoutMeasure;
    class PrintableSequence;
    
    /**
     * Editor-specific data (each editor can override this class and set their data through it in each
     * 'LineTrackRef'.
     */
    class EditorData
    {
    public:
        virtual ~EditorData() {}
    };
    
    
    /**
     * represents the reference to a track within a single 'LayoutLine' obejct.
     */
    class LineTrackRef
    {
        LayoutLine* parent;
        int trackID;
    public:
        int x0, y0, x1, y1;
        bool show_measure_number;
        //float pixel_width_of_an_unit;
        int layoutElementsAmount;
        
        // editors can put data of their own there.
        OwnerPtr<EditorData> editor_data;
        
        Track* track;
        
        LineTrackRef(LayoutLine* parent, int trackID) { this->parent = parent; this->trackID = trackID; }
        int getLastNote() const;
        int getFirstNote() const;
        
        int getFirstNoteInElement(const int layoutElementID);
        int getLastNoteInElement(const int layoutElementID);
        int getFirstNoteInElement(LayoutElement* layoutElement);
        int getLastNoteInElement(LayoutElement* layoutElement);
    };
    
    /*
     A line on a notation to print. Can contain more than one track.
     Essentially holds some 'LayoutElement' objects (the ones that fit
     on this line)
     */
    class LayoutLine
    {
        friend class AriaPrintable;
        
        ptr_vector<LineTrackRef> trackRenderInfo;
        ptr_vector<PrintLayoutMeasure, REF> measures;
        
        PrintableSequence* printable;
        
    public:
        /** used to store what percentage of this line's height this track should take.
         e.g. a score with F+G clefs will need more space than a 4-string bass tab
         so vertical space must not be divided equally */
        std::vector<short int> height_percent;
        
        /**
         Misc info about the coords of this track
         */
        int x0, y0, x1, y1;
        int margin_below, margin_above;
        
        LayoutLine(PrintableSequence* parent, ptr_vector<PrintLayoutMeasure, REF>& measures);
        
        // FIXME : is this really dependent on trackID?? I thought layout elements were the same for everyone
        const int getElementCount(const int trackID) const   { return trackRenderInfo[trackID].layoutElementsAmount; }
        
        //int width_in_units;
        int level_height;
        
        bool last_of_page;
        
        int getTrackAmount() const;
        LineTrackRef& getTrackRenderInfo(const int id);
        
        int getFirstNoteInElement(const int trackID, const int layoutElementID);
        int getLastNoteInElement(const int trackID, const int layoutElementID);
        int getFirstNoteInElement(const int trackID, LayoutElement* layoutElement);
        int getLastNoteInElement(const int trackID, LayoutElement* layoutElement);
        
        int calculateHeight();
        
        PrintLayoutMeasure& getMeasureForElement(const int layoutElementID) const;
        PrintLayoutMeasure& getMeasureForElement(LayoutElement* layoutElement);
        
        int getLastMeasure() const;
        int getFirstMeasure() const;
        
        std::vector<LayoutElement> layoutElements;
    };
    
}

#endif