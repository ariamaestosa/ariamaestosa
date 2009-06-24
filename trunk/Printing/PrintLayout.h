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


#ifndef _print_layout_h_
#define _print_layout_h_

#include <vector>
#include <map>
#include "ptr_vector.h"
#include "wx/wx.h"
#include "Printing/LayoutTree.h"

namespace AriaMaestosa
{
    class Track;
    class AriaPrintable;

    static const int max_line_width_in_units = 70;
    static const int maxLinesInPage = 10;

int getRepetitionMinimalLength();
void setRepetitionMinimalLength(const int newvalue);


class PrintLayoutManager
{
    AriaPrintable* parent;
    
    // referencing vectors from AriaPrintable
    std::vector<LayoutLine>& layoutLines;
    std::vector<LayoutPage>& layoutPages;
    ptr_vector<MeasureToExport>& measures;
    
    std::vector<LayoutElement> layoutElements;
    
    void calculateLineLayout();
    
    void calculateRelativeLengths();
    
    void generateOutputOrder(bool checkRepetitions_bool);
    void generateMeasures(ptr_vector<Track, REF>& tracks);
    
    void findSimilarMeasures();
public:
    PrintLayoutManager(AriaPrintable* parent,
                       std::vector<LayoutLine>& layoutLines  /* out */,
                       std::vector<LayoutPage>& layoutPages  /* out */,
                       ptr_vector<MeasureToExport>& mesaures /* out */);
    
    void calculateLayoutElements(ptr_vector<Track, REF>& track, const bool checkRepetitions_bool);
};
    
}

#endif
