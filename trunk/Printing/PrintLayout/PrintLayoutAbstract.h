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

#include "Printing/PrintLayout/LayoutElement.h"
#include "Printing/PrintLayout/LayoutPage.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"


namespace AriaMaestosa
{
    const int MARGIN_AT_MEASURE_BEGINNING = 50;
    
    class Track;
    class PrintableSequence;
    class PrintLayoutMeasure;
    
    int  getRepetitionMinimalLength();
    void setRepetitionMinimalLength(const int newvalue);
        
    
    /**
      * This class manages the abstract part of layout, i.e. it places stuff in lines and pages, but
      * does not actually determine the absolute coords of elements.
      */
    class PrintLayoutAbstract
    {
        PrintableSequence* sequence;
        
        // referencing vectors from PrintableSequence (FIXME: no two objects should reference these vectors)
        ptr_vector<LayoutPage>& layoutPages;
        
        ptr_vector<PrintLayoutMeasure> measures; 
        
        void layInLinesAndPages(std::vector<LayoutElement>& layoutElements);
        
        void calculateRelativeLengths(std::vector<LayoutElement>& layoutElements);
        
        void createLayoutElements(std::vector<LayoutElement>& layoutElements, bool checkRepetitions_bool);
        
        void findSimilarMeasures();
        
        
    public:
        PrintLayoutAbstract(PrintableSequence* parent,
                           ptr_vector<LayoutPage>& layoutPages  /* out */);
        
        void generateMeasures(ptr_vector<Track, REF>& tracks);
        
        
        void calculateLayoutElements(ptr_vector<Track, REF>& track, const bool checkRepetitions_bool);
    };
    
}

#endif
