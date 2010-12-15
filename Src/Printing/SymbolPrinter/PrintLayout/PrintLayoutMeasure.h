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

#include "Printing/SymbolPrinter/PrintLayout/RelativePlacementManager.h"
#include "ptr_vector.h"

namespace AriaMaestosa
{
    class PrintLayoutMeasure;
    class Track;

    extern const PrintLayoutMeasure NULL_MEASURE;


    /**
      * A description of a measure to print. If we print more than one track at once,
      * each measure will hold multiple 'MeasureTrackReference' instances.
      * A 'MeasureTrackReference' "ties" a PrintLayoutMeasure to a Track object,
      * by keeping a pointer to it and holding the range of notes in that track
      * that belong to this measure.
      */
    class MeasureTrackReference
    {
        Track* m_track;
        int m_first_note, m_last_note;
    public:
        
        MeasureTrackReference (Track* parent, const int firstNote, const int lastNote)
        {
            m_track      = parent;
            m_first_note = firstNote;
            m_last_note  = lastNote;
        }
                                                                        
        const Track* getConstTrack() const { return m_track;      }
        Track*       getTrack()            { return m_track;      }
        int          getFirstNote() const  { return m_first_note; }
        int          getLastNote()  const  { return m_last_note;  }

    };

    class PrintLayoutMeasure
    {
        /** first and last tick in this measure */
        int m_first_tick, m_last_tick;
        
        /** 
         * whether there is anything in this measure (a note that starts, or a note that starts in
         * previous measure and continues in this one, etc...) i.e. whether it's not empty
         */
        bool m_contains_something;
        
        /** ID of the measure */
        int m_measure_id;
        
        /** used when we print more than one track each track we print will have one entry here
         *  for each printed measure
         */
        ptr_vector<MeasureTrackReference> m_track_refs;
        
        RelativePlacementManager m_ticks_placement_manager;
        
    public:
        
        PrintLayoutMeasure(const int measID);

        /** 
          * Finds the notes correcsponding to this measure
          * in the given track and keep the reference.
          *
          * @return the ID of the next note to process (i.e. the first after this measure)
          *         or -1 if none
          */
        int  addTrackReference(const int firstNote, Track* track);
        
        int  getFirstTick     () const { return m_first_tick;             }
        int  getLastTick      () const { return m_last_tick;              }

        const RelativePlacementManager& getTicksPlacementManager() const { return m_ticks_placement_manager; }

        // FIXME: should not return a writable object probably?
        RelativePlacementManager& getTicksPlacementManager() { return m_ticks_placement_manager; }
        
        /** 
          * @return whether there is anything in this measure (a note that starts, or a note that starts in
          *         previous measure and continues in this one, etc...)
          */
        bool isEmpty          () const { return not m_contains_something; }
        
        int  getTrackRefAmount() const { return m_track_refs.size();      }
        
        const MeasureTrackReference& getTrackRef(const int id) const { return m_track_refs[id]; }
        
        /**
          * @return  The ID of the measure, from 0 to the amount of measures in the sequence - 1
          */
        int  getMeasureID() const { return m_measure_id;              }
        
        bool operator==  (const PrintLayoutMeasure& meas) const { return meas.m_measure_id == m_measure_id; }
        
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
