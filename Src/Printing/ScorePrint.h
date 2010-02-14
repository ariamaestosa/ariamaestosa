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


#ifndef _score_out_
#define _score_out_

#include <wx/file.h>
#include "Config.h"
#include "Editors/ScoreEditor.h"
#include "Printing/AriaPrintable.h"
#include "Printing/EditorPrintable.h"
#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/RelativePlacementManager.h"

namespace AriaMaestosa
{
    class ScoreAnalyser;

    /** For internal use by ScorePrintable . Describes a silence to drawn on the score. */
    class SilenceInfo
    {
    public:
        Range<int> m_tick_range;
        int m_type;
        bool m_dotted;
        int m_dot_delta_x;
        
        SilenceInfo(int tickFrom, int tickTo, int type, bool dotted, int dot_delta_x) :
        m_tick_range(tickFrom, tickTo), m_type(type), m_dotted(dotted), m_dot_delta_x(dot_delta_x)
        {
        }
    };

    /** Specialization to print scores */
    class ScorePrintable : public EditorPrintable
    {
        void gatherVerticalSizingInfo(const int trackID, LineTrackRef& track, LayoutLine& line);
        
        void analyseAndDrawScore(bool f_clef, ScoreAnalyser& analyser, LayoutLine& line, Track* track, wxDC& dc,
                                 const int extra_lines_above, const int extra_lines_under,
                                 const int x0, const int y0, const int x1, const int y1, bool show_measure_number);
        
        bool g_clef, f_clef;
        int middle_c_level;
        OwnerPtr<ScoreAnalyser> g_clef_analyser;
        OwnerPtr<ScoreAnalyser> f_clef_analyser;
        
        std::vector< SilenceInfo > silences_ticks;
        
    public:
        ScorePrintable();
        virtual ~ScorePrintable();
        
        /** Implement method from EditorPrintable */
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID, const MeasureTrackReference& trackRef, RelativePlacementManager& ticks);
        
        /** Implement method from EditorPrintable */
        virtual void earlySetup(const int trackID, Track* track);
        
        /** Implement method from EditorPrintable */
        virtual void drawTrack(const int trackID, LineTrackRef& track, LayoutLine& line, wxDC& dc);
        
        /** Implement method from EditorPrintable */
        virtual int calculateHeight(const int trackID, LineTrackRef& renderInfo, LayoutLine& line);
    };
    
}

#endif
