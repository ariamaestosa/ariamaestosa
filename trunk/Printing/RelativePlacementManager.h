#ifndef __RELATIVE_PLACEMENT_MANAGER__
#define __RELATIVE_PLACEMENT_MANAGER__

#include <vector>
#include "Range.h"

// For unit tests
class RelativePlacementManager_TestAddingAndFindingInterestingTicks;
class RelativePlacementManager_TestFindingNextTick;

namespace AriaMaestosa
{

/**
 * Calculates the relative segment of symbols, which can be spawn across several tracks,
 * within a single measure.
 */
class RelativePlacementManager
{
    // For unit tests
    friend class ::RelativePlacementManager_TestAddingAndFindingInterestingTicks;
    friend class ::RelativePlacementManager_TestFindingNextTick;
    
    /** Last tick of the measure (A RelativePlacementManager object represents a single measure) */
    int m_end_of_measure_tick;
    
    struct Symbol
    {
        float proportion;
        int trackID;
        int endTick;
        
        /** to be filled later; the first interesting tick would be unit 0,
         * the second will be unit 1, etc. */
        int fromUnit;
        float neededAdditionalProportion;
        
        Symbol (const int tick_to, const float proportion, const int trackID)
        {
            Symbol::endTick = tick_to;
            Symbol::proportion = proportion;
            Symbol::trackID = trackID;
            
            // To be set later
            fromUnit = -1;
            neededAdditionalProportion = -1;
        }
    };
    
    struct InterestingTick
    {
        int tick;
        std::vector<Symbol> all_symbols_on_that_tick;
        
        /** unset intially, is set later during calculations.
          * represents the total proportion this unit needs to receive */
        float proportion;
        
        /** unset intially, is set later during calculations.
         * represents the position of this tick, from 0 to 1 */
        float position;
        
        float endPosition;
        
        InterestingTick (const int tick)
        {
            InterestingTick::tick = tick;
        }
        
        /** Returns whether this object contains a symbol on the given track */
        bool hasSymbolInTrack(const int trackID)
        {
            const int sym_amount = all_symbols_on_that_tick.size();
            for (int n=0; n<sym_amount; n++)
            {
                if (all_symbols_on_that_tick[n].trackID == trackID) return true;
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
      * @return         The id of the associated interesting tick within the 
      */
    int getInterestingTick(const int tick, const int id_from, const int id_to);
    
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
    }
    
    /**
     * @param tickFrom         The tick where the added symbol's "area of relevance" starts
     *                         (by "area of relevance", I mean the ticks that this note/silence
     *                          covers, independently of the size of the symbol itself)
     * @param tickTo           The tick where the added symbol's "area of relevance" end
     * @param symbolProportion How big this symbol is, relative to other (here we are
     *                         only talking of the size of the graphical symbol, not the
     *                         duration of the note it represents)
     * @param trackID          ID of the track in which this symbol is
     */
    void addSymbol(int tickFrom, int tickTo, float symbolProportion, int trackID);
    
    /**
     * Calculates the relative placement of all added symbols.
     *
     * @precondition           To be called after all symbols have been added
     */
    void calculateRelativePlacement();

    /**
     * @precondition           To be called after all symbols have been added
     * @return                 The number of units (number of distinct ticks on which symbols are place)
     */
    int getUnitCount() const;
    
    /**
     * Getter to get the results of the calculation
     *
     * @param tick             The tick at which the symbol we want to get info on starts
     * @precondition           calculateRelativePlacement must have been called first.
     *
     * @return                 The calculated area (in proportion, 0 being leftmost and 1 rightmost)
     *                         of a tick within a specific track.
     */
    Range<float> getSymbolRelativeArea(int tick);
};

}
#endif
