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


#ifndef __SCORE_PRINTABLE_H__
#define __SCORE_PRINTABLE_H__

#include "Utils.h"
#include "Analysers/SilenceAnalyser.h"
#include "Printing/SymbolPrinter/EditorPrintable.h"

namespace AriaMaestosa
{
    class ScoreAnalyser;
    class RelativePlacementManager;

    /**
      * @brief Implementation of EditorPrintable that can print scores
      * @ingroup printing
      */
    class ScorePrintable : public EditorPrintable
    {
        void gatherVerticalSizingInfo(const int trackID, LineTrackRef& track, LayoutLine& line);
        
        enum ClefRenderType
        {
            G_CLEF_ALONE,
            F_CLEF_ALONE,
            G_CLEF_FROM_GRAND_STAFF,
            F_CLEF_FROM_GRAND_STAFF
        };
        
        void analyseAndDrawScore(ClefRenderType clefType, ScoreAnalyser& analyser, LayoutLine& line, const Track* track,
                                 wxDC& dc, const int extra_lines_above, const int extra_lines_under,
                                 const int x0, const int y0, const int x1, const int y1,
                                 bool show_measure_number, const int grandStaffCenterY);
        
        bool m_g_clef, m_f_clef;
        int m_middle_c_level;
        OwnerPtr<ScoreAnalyser> g_clef_analyser;
        OwnerPtr<ScoreAnalyser> f_clef_analyser;
        
        std::vector< SilenceAnalyser::SilenceInfo > m_silences_ticks;
        
    public:
        ScorePrintable();
        virtual ~ScorePrintable();
        
        /** Implement method from EditorPrintable */
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                  MeasureTrackReference& trackRef, RelativePlacementManager& ticks);
        
        /** Implement method from EditorPrintable */
        virtual void earlySetup(const int trackID, Track* track);
        
        /** Implement method from EditorPrintable */
        virtual void drawTrack(const int trackID, const LineTrackRef& track, LayoutLine& line,
                               wxDC& dc, const bool drawMeasureNumbers);
        
        /** Implement method from EditorPrintable */
        virtual int calculateHeight(const int trackID, LineTrackRef& renderInfo, LayoutLine& line, bool* empty);
    };
    
}

#endif
