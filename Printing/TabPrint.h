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

#include <wx/file.h>
#include "Config.h"
#include "Printing/EditorPrintable.h"
#include "Printing/PrintLayout.h"
#include "Printing/RelativePlacementManager.h"

namespace AriaMaestosa
{
    class GuitarEditor;
    
class TablaturePrintable : public EditorPrintable
{
    int string_amount;
    GuitarEditor* editor;
public:
    TablaturePrintable(Track* track_arg);
    virtual ~TablaturePrintable();

    void addUsedTicks(const PrintLayoutMeasure& measure, const int trackID,
                      const MeasureTrackReference& trackRef, RelativePlacementManager& ticks);

    void drawLine(const int trackID, LineTrackRef& track, LayoutLine& line, wxDC& dc);
    int calculateHeight(const int trackID, LineTrackRef& renderInfo, LayoutLine& line);
    
};

}

#endif
