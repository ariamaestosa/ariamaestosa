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

#ifndef __EDITOR_PRINTABLE_H__
#define __EDITOR_PRINTABLE_H__

#include "Range.h"
#include "Printing/PrintLayout/RelativePlacementManager.h"

class wxDC;
class wxPoint;

namespace AriaMaestosa
{

    class LayoutLine;
    class LayoutElement;
    class LineTrackRef;
    class PrintLayoutMeasure;
    class MeasureTrackReference;
    class Track;
    
    
    /** Width of 1/1 and 1/2 silences */
    const int RECTANGULAR_SILENCE_SIZE = 80;
    
    /** How much margin space to leave at the left of 1/1 and 1/2 silences*/
    const int RECTANGULAR_SILENCE_LEFT_MARGIN = 40;
    
    /** size of the dot, for dotted notes */
    const int DOT_SIZE = 14;
    
    /** Size of a note's head */
    const int HEAD_RADIUS = 36;
    
    
    class EditorPrintable
    {
    protected:
        wxDC* dc;
        //int getClosestTickFrom(const int trackID, LayoutLine& line, const int tick);

        /**
          * Draws a vertical divider line
          * @param el     The layout element that will be used to determine at which x to draw the line.
          * @param y0     The y at which the divider starts
          * @param y1     The y at which the divider ends
          * @param atEnd  By default the line is drawn at the beginning of the element; pass true here
          *               to draw the line at the end of the element instead.
          */
        void drawVerticalDivider(LayoutElement* el, const int y0, const int y1, const bool atEnd=false);

        /**
          * Draws a vertical divider line
          * @param x      The x coord where to draw the line.
          * @param y0     The y at which the divider starts
          * @param y1     The y at which the divider ends
          */
        void drawVerticalDivider(const int x, const int y0, const int y1);
        
        /** Utility function : renders an arc (half an ellipse) at the given coordinates */
        static void renderArc(wxDC& dc, const int center_x, const int center_y,
                              const int radius_x, const int radius_y);
        
        /** Utility function : renders the track name at the given coordinates */
        static void drawTrackName(wxDC& dc, const LineTrackRef& currentTrack, int x, int y0, int y1);

    public:
        EditorPrintable();
        virtual ~EditorPrintable();
        
        /** 
          * Called by the print code when it's time to render one track.
          * This will be handled in the appropriate subclass.
         */
        virtual void drawTrack(const int trackID, const LineTrackRef& track, LayoutLine& line,
                               wxDC& dc, const bool drawMeasureNumbers) = 0;
        
        /**
          * Called by the layout code to know the relative height of a track within a line
          * for this particular editor
          *
          * @param      trackID  id of the track within this line to calculate the height of
          * @param      track    the 'LineTrackRef' associated with this track
          * @param      line     the line this track is part of
          * @param[out] empty    this parameter shall be set to 'true' if this track, in this
          *                      line, is empty (contains no note or symbol whatsoever)
          * @return              the calculated height, in abstract vertical print units
          */
        virtual int calculateHeight(const int trackID, LineTrackRef& track, LayoutLine& line,
                                    bool* empty) = 0;
                        
        /**
          * call early in the callback to draw a track. All base class methods doing rendering
          * will use the passed DC to perform the rendering (FIXME: that's quite ugly)
          */
        void setCurrentDC(wxDC* dc);

        /** Returns the print area reserved for a specific note (by note ID) */
        //Range<int> getNoteAreaX(const int trackID, LayoutLine& line, int noteID);
        
        /** Returns the print area reserved for a specific note's symbol (by note ID) */
        Range<int> getNoteSymbolX(const int trackID, LayoutLine& line, int noteID);

        /** Returns the print area reserved for a specific note/silence/symbol (by tick).
          * @precondition The tick must be known already to the RelativePlacementManager;
          * otherwise (-1,-1) may be returned. */
        Range<int> tickToX(const int trackID, LayoutLine& line, const int tick);

        /** Render utility; renders time signature numbers in the coords of the passed layout element,
          * at the given y coords */
        void renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1);
        
        /** This method is called for each track of this editor very early in the layout process.
          * If this class needs to do some early setup to prepare the data for following calls,
          * this is the place to do so.
          */
        virtual void earlySetup(const int trackID, Track* track) {}
        
        /**
          * Classes deriving from EditorPrintable must implemented this method. It will be
          * called by the layout routines for each routine and each track handled by this editor.
          * The implemention must add information about where it needs to display symbols to
          * the RelativePlacementManager.
          */ 
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                  const MeasureTrackReference& trackRef, RelativePlacementManager& ticks) = 0;
        
        /** utility method subclasses can call to render a silence at a given location */
        void drawSilence(wxDC* dc, const Range<int> x, const int y, const int level_height,
                         const int type, const bool triplet, const bool dotted);
        
        /** draw some basic stuff common to all editors for the given layout element */
        void drawElementBase(LayoutElement& currElem, const LayoutLine& layoutLine,
                             const bool drawMeasureNumbers, const int y0, const int y1);
        
        /** utility method to render a note's head */
        static void drawNoteHead(wxDC& dc, const wxPoint headCenter, const bool hollowHead);

    };
}

#endif