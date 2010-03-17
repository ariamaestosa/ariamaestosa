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
    const int INTER_LINE_MARGIN_LEVELS = 6;

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
        /** Reference to the parent sequence */
        PrintableSequence* m_sequence;
        
        /** holds all measure objects from this sequence (there are not multiple copies of these
          * objects for each measure number even if multiple tracks are being printed) 
          */
        ptr_vector<PrintLayoutMeasure> m_measures; 
        

        /**
          * @brief Builds the Page/Line layout tree from the full list of layout elements
          * Creates LayoutPage and LayoutLine objects, add the created layout elements to their
          * corresponding line
          */
        void layInLinesAndPages(std::vector<LayoutElement>& layoutElements, ptr_vector<LayoutPage>& layoutPages);
        
        /** The main goal of this method is to set the 'width_in_print_units' member of each LayoutElement */
        void calculateRelativeLengths(std::vector<LayoutElement>& layoutElements);
        
        /**
          * Populates the 'layoutElements' vector with elements that represent the current sequence.
          *
          * @param[out] layoutElements   The vector that will be filled with LayoutElement objects
          * @param checkRepetitions_bool Whether to attempt to automatically detect repeated song parts
          */
        void createLayoutElements(std::vector<LayoutElement>& layoutElements, bool checkRepetitions_bool);
        
        /** fills fields containing info about similar measures withing the PrintLayoutMeasure objects */
        void findSimilarMeasures();
        
        /** utility method invoked by 'layInLinesAndPages' when a line is complete */
        void terminateLine(LayoutLine* line, ptr_vector<LayoutPage>& layoutPages, const int maxLevelHeight,
                           int& current_height, int& current_page);

        /** generates measures. needs to be called before "calculateLayoutElements" */
        void generateMeasures(ptr_vector<Track, REF>& tracks);
        
        /**
          * @precondition measures must have been generated.
          *
          * @param[out] layoutPages
          */
        void calculateLayoutElements(ptr_vector<Track, REF>& tracks,
                                     ptr_vector<LayoutPage>& layoutPages,
                                     const bool checkRepetitions_bool);
        
    public:
        /**
          * Constructs a PrintLayoutAbstract object that can lay a given sequence within pages
          * @param sequence   which sequence this layout manager will analyse and fit
          */
        PrintLayoutAbstract(PrintableSequence* sequence);
        
        /**
          * @brief                  main function called from other classes
          * @precondition           measures must have been generated.
          * @param tracks           a list of all tracks to be printed.
          * @param[out] layoutPages the vector of pages that is filled by this call
          * @param findReps         whether to use the feature to automatically attempt to detect
          *                         repeated song sections
          */
        void addLayoutInformation(ptr_vector<Track, REF>& tracks, ptr_vector<LayoutPage>& layoutPages,
                                  const bool findReps);
    };
    
}

#endif
