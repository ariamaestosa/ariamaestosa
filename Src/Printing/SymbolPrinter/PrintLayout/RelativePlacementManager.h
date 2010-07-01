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

#ifndef __RELATIVE_PLACEMENT_MANAGER__
#define __RELATIVE_PLACEMENT_MANAGER__

#include <vector>
#include "unit_test.h"
#include "Range.h"
#include "Analysers/SilenceAnalyser.h"

// For unit tests
test TestAddingAndFindingInterestingTicks;
test TestFindingNextTick;

namespace AriaMaestosa
{

/**
 * Calculates the relative segment of symbols, which can be spawn across several tracks,
 * within a single measure.
 */
class RelativePlacementManager
{
    // For unit tests
    friend test ::TestAddingAndFindingInterestingTicks;
    friend test ::TestFindingNextTick;
    
    /** Last tick of the measure (A RelativePlacementManager object represents a single measure) */
    int m_end_of_measure_tick;
    
    int m_total_needed_size;
    
    struct Symbol
    {
        int m_width_in_print_units;
        int m_track_ID;
        int m_end_tick;
        
        /** to be filled later; the first interesting tick would be unit 0,
         * the second will be unit 1, etc. */
        int fromUnit;
        float neededAdditionalProportion;
        
        Symbol (const int tick_to, const int widthInPrintUnits, const int trackID)
        {
            m_end_tick = tick_to;
            m_width_in_print_units = widthInPrintUnits;
            m_track_ID = trackID;
            
            // To be set later
            fromUnit = -1;
            neededAdditionalProportion = -1;
        }
    };
    
    struct InterestingTick
    {
        int m_tick;
        std::vector<Symbol> m_all_symbols_on_that_tick;
        
        /** unset intially, is set later during calculations.
          * represents the total proportion this unit needs to receive */
        int m_size;
        
        /** unset intially, is set later during calculations.
         * represents the position of this tick, from 0 to 1 */
        float m_position;
        
        float m_end_position;
        
        InterestingTick (const int tick)
        {
            m_tick = tick;
        }
        
        /** Returns whether this object contains a symbol on the given track */
        bool hasSymbolInTrack(const int trackID)
        {
            const int sym_amount = m_all_symbols_on_that_tick.size();
            for (int n=0; n<sym_amount; n++)
            {
                if (m_all_symbols_on_that_tick[n].m_track_ID == trackID) return true;
            }
            return false;
        }
    };
    
    std::vector<InterestingTick> m_all_interesting_ticks;
    
    /**
      * @param tick     The midi tick for which we want to find the associated 'InterestingTick' object
      * @param id_from  ID of the element of the 'm_all_interesting_ticks' vector at which to start searching
      * @param id_to    ID of the element of the 'm_all_interesting_ticks' vector at which to search ends
      *
      * @postcondition  If no 'InterestingTick' object exists for that tick, one is created. The vector remains ordered.
      *
      * @return         The id of the associated interesting tick
      */
    int getInterestingTick(const int tick, const int id_from, const int id_to);
    
    /**
     * @param tick     The midi tick for which we want to find the associated 'InterestingTick' object
     * @param id_from  ID of the element of the 'm_all_interesting_ticks' vector at which to start searching
     * @param id_to    ID of the element of the 'm_all_interesting_ticks' vector at which to search ends
     *
     * precondition    All interesting ticks must have been created prior to call this, as this variant will
     *                 NOT insert a new tick of none exists yet.
     *
     * @return         The id of the associated interesting tick, or -1 if none
     */
    int getInterestingTickNoAdd(const int tick, const int id_from, const int id_to) const;

    /**
      * @return          Length (in ticks) of the shortest symbol in this measure
      */
    int findShortestSymbolLength() const;
    
    /**
     * Finds the start tick of the next symbol, in any track, starting from a base tick
     *
     * @return  The tick of this symbol, if found, the last tick of the measure otherwise
     */
    int getNextTick(const int from_tick);
    
    /**
      * Finds the start tick of the next symbol in a given track, starting from a base tick
      *
      * @return  The tick of this symbol, if found, the last tick of the measure otherwise
      */
    int getNextTickInTrack(const int from_tick, const int trackID);
    
public:
    
    /**
      * Constructor.
      *
      * @param end_of_measure_tick  Last tick in the measure that this object will represent
      */
    RelativePlacementManager(const int end_of_measure_tick)
    {
        m_end_of_measure_tick = end_of_measure_tick;
        m_total_needed_size   = 0;
    }
    
    /**
     * @param tickFrom         The tick where the added symbol's "area of relevance" starts
     *                         (by "area of relevance", I mean the ticks that this note/silence
     *                          covers, independently of the size of the symbol itself)
     * @param tickTo           The tick where the added symbol's "area of relevance" end
     * @param symbolWidth      How big this symbol (is in print units)
     * @param trackID          ID of the track in which this symbol is
     */
    void addSymbol(int tickFrom, int tickTo, int symbolWidth, int trackID);
    
    /**
      * Utility that lets you add many symbols at once for silences
      */
    void addSilenceSymbols(const std::vector< SilenceAnalyser::SilenceInfo >& silences_ticks,
                           const int trackID, const int firstTickInMeasure, const int lastTickInMeasure);
    
    /**
     * Calculates the relative placement of all added symbols.
     *
     * @pre           To be called after all symbols have been added
     */
    void calculateRelativePlacement();

    /**
     * @pre           To be called after all symbols have been added
     * @return                 The number of units (number of distinct ticks on which symbols are place)
     */
    //int getUnitCount() const;
    
    /**
     * @pre           To be called after all symbols have been added
     * @return                 The number of print units this measure needs to be allocated
     */
    int getWidth() const
    {
        return m_total_needed_size;
    }
    
    /**
     * Getter to get the results of the calculation
     *
     * @param tick             The tick at which the symbol we want to get info on starts
     * @pre           calculateRelativePlacement must have been called first.
     *
     * @return                 The calculated area (in proportion, 0 being leftmost and 1 rightmost)
     *                         of a tick within a specific track.
     */
    Range<float> getSymbolRelativeArea(int tick) const;
};

}
#endif
