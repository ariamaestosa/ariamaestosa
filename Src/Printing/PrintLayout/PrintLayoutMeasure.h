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



#ifndef _PRINT_LAYOUT_MEASURE_H_
#define _PRINT_LAYOUT_MEASURE_H_

#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/RelativePlacementManager.h"
#include <map>

namespace AriaMaestosa
{
    class PrintLayoutMeasure;
    class Track;

    extern const PrintLayoutMeasure nullMeasure;


    /**
      * A description of a measure to print. If we print more than one track at once,
      * each measure will hold multiple 'MeasureTrackReference' instances.
      * A 'MeasureTrackReference' "ties" a PrintLayoutMeasure to a Track object,
      * by keeping a pointer to it and holding the range of notes in that track
      * that belong to this measure.
      */
    class MeasureTrackReference
    {
    public:
        Track* track;
        int firstNote, lastNote;
    };

    class PrintLayoutMeasure
    {
    public:
        PrintLayoutMeasure(const int measID);
        
        RelativePlacementManager ticks_placement_manager;
        
        /** Finds the notes correcsponding to this measure
          * in the given track and keep the reference.
          * Returns the ID of the last note in this measure
          */
        int addTrackReference(const int firstNote, Track* track);
        
        /** used when we print more than one track each track we print will have one entry here
          * for each printed measure
          */
        ptr_vector<MeasureTrackReference> trackRef;
        
        /** first and last tick in this measure */
        int firstTick, lastTick;

        /** shortest note in the measure (will be used to determine "zoom" on measure. e.g. a measure with very
          * short notes takes more room).
          */
        int shortestDuration;
        
        /** ID of the measure */
        int id;
        
        // -------- Experimental : automatic repetition detection --------      
        /** if this measure is later repeated and is not a repetition of a previous measure,
         * contains ID of all later measures similar to this one
         */
        std::vector<int> similarMeasuresFoundLater;
        
        /** if this measure is a repetition of a previous measure, contains the ID of which one */
        int firstSimilarMeasure;
        
        
        /** true if measure needs to be apart from others
          * mostly used with repetitions (e.g. x4) to tell where the repetition starts
          */
        bool cutApart; // FIXME - doesn't really belong here, should be a layout element
        
        bool calculateIfMeasureIsSameAs(PrintLayoutMeasure& checkMeasure);
        
        /** if a repetition is found, it is stored in the variables and returns true,
          * otherwise returns false
          */
        bool findConsecutiveRepetition(ptr_vector<PrintLayoutMeasure>& measures, const int measureAmount,
                                       int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                       int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/);
    };

}

#endif
