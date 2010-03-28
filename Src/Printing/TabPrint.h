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


#ifndef _tablature_out_
#define _tablature_out_

#include "Utils.h"
#include "Analysers/SilenceAnalyser.h"
#include "Printing/EditorPrintable.h"
#include "Printing/PrintLayout/RelativePlacementManager.h"

namespace AriaMaestosa
{
    class GuitarEditor;
    class ScoreAnalyser;
    
    /** Specialization to print tablatures */
    class TablaturePrintable : public EditorPrintable
    {
        int string_amount;
        GuitarEditor* editor;
        
        std::vector<SilenceAnalyser::SilenceInfo> m_silences;
        
        OwnerPtr<ScoreAnalyser> m_analyser;
        
    public:
        TablaturePrintable(Track* track_arg);
        virtual ~TablaturePrintable();
        
        /** Implement method from EditorPrintable */
        virtual void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                                  const MeasureTrackReference& trackRef, RelativePlacementManager& ticks);
        
        /** Implement method from EditorPrintable */
        virtual void drawTrack(const int trackID, const LineTrackRef& track, LayoutLine& line,
                               wxDC& dc, const bool drawMeasureNumbers);
        
        /** Implement method from EditorPrintable */
        virtual int calculateHeight(const int trackID, LineTrackRef& renderInfo, LayoutLine& line, bool* empty);
        
        /** Implement method from EditorPrintable */
        virtual void earlySetup(const int trackID, Track* track);
    };
    
}

#endif
