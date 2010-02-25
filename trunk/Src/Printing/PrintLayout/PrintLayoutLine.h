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
    
    /** Contains the absolute coords part of a LineTrackRef */
    struct TrackCoords
    {
        /** Coordinates of that track, in print units. Will not be set initially. */
        int x0, y0, x1, y1;
    };
    
    /**
     * Represents the reference to a track within a single 'LayoutLine' obejct.
     */
    class LineTrackRef
    {
        LayoutLine* parent;
        int trackID;
    public:
        
        /**
         * Editor-specific data (each editor can override this class and set their data through it in each
         * 'LineTrackRef'.
         */
        class EditorData
        {
        public:
            virtual ~EditorData() {}
        };
    
        /** editors can put data of their own there. */
        OwnerPtr<EditorData> editor_data;
        
        /** Coordinates of that track, in print units. Will not be set initially.
          * Will be set by the PrintLayoutNumeric when layout is finalized. */
        OwnerPtr<TrackCoords> m_track_coords;
        
        bool show_measure_number;

        const Track* m_track;
        
        LineTrackRef(LayoutLine* parent, int trackID, const Track* track) : m_track(track)
        {
            this->parent = parent;
            this->trackID = trackID;
        }
        int getLastNote() const;
        int getFirstNote() const;
        
        int getFirstNoteInElement(const int layoutElementID);
        int getLastNoteInElement(const int layoutElementID);
        int getFirstNoteInElement(LayoutElement* layoutElement);
        int getLastNoteInElement(LayoutElement* layoutElement);
    };
    
    /**
      * Contains absolute coords of where to print a line, with absolute coords
      */
    struct LineCoords
    {
        int x0, y0, x1, y1;
        int margin_below, margin_above;
    };
    
    /**
      * A line on a notation to print. Can contain more than one track.
      * Essentially holds some 'LayoutElement' objects (the ones that fit
      * on this line)
      */
    class LayoutLine
    {
        friend class AriaPrintable;
        
        ptr_vector<LineTrackRef> m_track_render_info;
        ptr_vector<PrintLayoutMeasure, REF> m_measures;
        
        PrintableSequence* m_printable;
        
        std::vector<LayoutElement> m_layout_elements;

    public:
        /** used to store what percentage of this line's height this track should take.
          * e.g. a score with F+G clefs will need more space than a 4-string bass tab
          * so vertical space must not be divided equally
          */
        std::vector<short int> m_height_percent;
        
        void addLayoutElement( const LayoutElement& newElem )
        {
            m_layout_elements.push_back( newElem );
        }
        
        /** Initially NULL; will be set when PrintLayoutNumeric actually calculates the coords
          * of this track
          */
        OwnerPtr<LineCoords> m_line_coords;
        
        LayoutLine(PrintableSequence* parent, ptr_vector<PrintLayoutMeasure, REF>& measures);
        
        int m_level_height;
        
        bool m_last_of_page;
        
        int getTrackAmount() const;
        LineTrackRef& getLineTrackRef(const int id);
        
        int getLayoutElementCount() const { return m_layout_elements.size(); }
        LayoutElement& getLayoutElement(const int id) { return m_layout_elements[id]; }
        
        int getFirstNoteInElement(const int trackID, const int layoutElementID) const;
        int getLastNoteInElement (const int trackID, const int layoutElementID) const;
        int getFirstNoteInElement(const int trackID, const LayoutElement* layoutElement) const;
        int getLastNoteInElement (const int trackID, const LayoutElement* layoutElement) const;
        
        int calculateHeight();
        
        const PrintLayoutMeasure& getMeasureForElement(const int layoutElementID) const;
        const PrintLayoutMeasure& getMeasureForElement(const LayoutElement* layoutElement) const;
        
        int getLastMeasure()  const;
        int getFirstMeasure() const;
    };
    
}

#endif