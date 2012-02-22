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

class wxDC;
class wxPoint;
class wxImage;
class wxBitmap;
class wxGraphicsContext;

namespace AriaMaestosa
{
    class GraphicalTrack;
    class LayoutLine;
    class LayoutElement;
    class LineTrackRef;
    class PrintLayoutMeasure;
    class MeasureTrackReference;
    class RelativePlacementManager;
    class Track;
    
    /**
      * @brief   base class for the various types of notations that can be printed
      *
      * To add the ability to print a new notation type, simply derive from this class.
      * This base class provides many common features, like rendering of common features and
      * coordinate management of notes.
      *
      * @ingroup printing
      */
    class EditorPrintable
    {
    protected:
        wxDC* m_dc;

        Track* m_track;
        
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
        
        /** Utility function : renders the track name at the given coordinates */
        static void drawTrackName(wxDC& dc, const LineTrackRef& currentTrack, int x, int y0, int y1);



    public:
        EditorPrintable(Track* track);
        virtual ~EditorPrintable();
        
        /** 
          * @brief Called by the print code when it's time to render one track.
          *
          * This will be handled in the appropriate subclass.
         */
        virtual void drawTrack(const int trackID, const LineTrackRef& track, LayoutLine& line,
                               wxDC& dc, wxGraphicsContext* gc, const bool drawMeasureNumbers) = 0;
        
        virtual void drawTrackBackground(const int trackID, const LineTrackRef& currentTrack,
                                         LayoutLine& currentLine, wxDC& dc, wxGraphicsContext* grctx,
                                         const bool drawMeasureNumbers) = 0;
        
        /**
          * @brief Called by the layout code to know the relative height of a track within a line
          *        for this particular editor
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
          * @brief call early in the callback to draw a track.
          *
          * All base class methods doing rendering
          * will use the passed DC to perform the rendering (FIXME(DESIGN): that's quite ugly)
          */
        void setCurrentDC(wxDC* dc);

        /** @return the print area reserved for a specific note's symbol (by note ID) */
        Range<int> getNoteSymbolX(const int trackID, LayoutLine& line, int noteID);

        /** @return the print area reserved for a specific note/silence/symbol (by tick).
          * @pre The tick must be known already to the RelativePlacementManager;
          * otherwise (-1,-1) may be returned. */
        Range<int> tickToX(const int trackID, LayoutLine& line, const int tick);

        /** 
          * @brief Render utility
          *
          * renders time signature numbers in the coords of the passed layout element,
          * at the given y coords
          */
        void renderTimeSignatureChange(LayoutElement* el, const int y0, const int y1);
        
        /** 
          * @brief This method is called for each track of this editor very early in the layout process.
          *
          * If this class needs to do some early setup to prepare the data for following calls,
          * this is the place to do so.
          */
        virtual void earlySetup(const int trackID, GraphicalTrack* track) {}
        
        /**
          * Classes deriving from EditorPrintable must implemented this method. It will be
          * called by the layout routines for each routine and each track handled by this editor.
          * The implemention must add information about where it needs to display symbols to
          * the RelativePlacementManager.
          */ 
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                  MeasureTrackReference& trackRef, RelativePlacementManager& ticks) = 0;

        
        /** @brief draw some basic stuff common to all editors for the given layout element */
        void drawElementBase(LayoutElement& currElem, const LayoutLine& layoutLine,
                             const bool drawMeasureNumbers, const int y0, const int y1,
                             const int barYFrom, const int barYTo);

        Track* getTrack() { return m_track; }
        
        /**
         * Get the Y coordinate where the first line of the score is.
         */
        virtual int getFirstLineY(const LineTrackRef& currentTrack) = 0;
        
    };
}

#endif
