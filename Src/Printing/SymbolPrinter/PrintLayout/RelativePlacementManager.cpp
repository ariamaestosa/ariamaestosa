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

#include "RelativePlacementManager.h"

#include "AriaCore.h"
#include "UnitTest.h"
#include <iostream>
#include <cmath>
#include <cassert>

#include "Midi/MeasureData.h"
#include "Printing/RenderRoutines.h"
#include "Printing/SymbolPrinter/EditorPrintable.h"

#if 0
#pragma mark Private
#endif


#define RPM_CHATTY 0

using namespace AriaMaestosa;
const int SIDE_MARGIN_WIDTH = 60;

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getInterestingTick(const int tick, const int id_from, const int id_to)
{
    if (id_to < id_from)
    {
        m_all_interesting_ticks.insert( m_all_interesting_ticks.begin()+id_from, InterestingTick(tick) );
        return id_from;
    }
    else
    {
        // perform binary search to find the interesting tick (or to find where to insert it, if it doesn't exist)
        const int pivot_id = (id_from + id_to)/2;
        const int pivot_tick = m_all_interesting_ticks[pivot_id].m_tick;
        
        if (tick == pivot_tick)
        {
            return pivot_id;
        }
        else if (tick < pivot_tick)
        {
            return getInterestingTick(tick, id_from, pivot_id - 1);
        }
        else //if (tick > pivot_tick) // [implicit]
        {
            return getInterestingTick(tick, pivot_id + 1, id_to);
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getInterestingTickNoAdd(const int tick, const int id_from, const int id_to) const
{
    if (id_to < id_from)
    {
        return -1;
    }
    else
    {
        // perform binary search to find the interesting tick
        const int pivot_id = (id_from + id_to)/2;
        const int pivot_tick = m_all_interesting_ticks[pivot_id].m_tick;
        
        if (tick == pivot_tick)
        {
            return pivot_id;
        }
        else if (tick < pivot_tick)
        {
            return getInterestingTickNoAdd(tick, id_from, pivot_id - 1);
        }
        else //if (tick > pivot_tick) // [implicit]
        {
            return getInterestingTickNoAdd(tick, pivot_id + 1, id_to);
        }
    }
}

UNIT_TEST( TestAddingAndFindingInterestingTicks )
{
    //throw std::logic_error("C'est normal que ça échoue!!");
    RelativePlacementManager testObj(20);
    
    require( testObj.m_all_interesting_ticks.size() == 0, "RelativePlacementManager is initially empty" );
    
    // ---- add 15
    int id_added = testObj.getInterestingTick( 15, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 1, "An element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 0, "The returned ID is correct for the first added item" );
    require( testObj.m_all_interesting_ticks[0].m_tick == 15, "First added item was correctly added" );
    
    int id_obtained = testObj.getInterestingTick( 15, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 1, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 0, "The returned ID is correct for searching a single item" );
    
    // ---- add 17
    id_added = testObj.getInterestingTick( 17, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 2, "An second element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 1, "The returned ID is correct for the second added item" );
    require( testObj.m_all_interesting_ticks[1].m_tick == 17, "Second added item was correctly added" );
    
    id_obtained = testObj.getInterestingTick( 15, 0, testObj.m_all_interesting_ticks.size()-1 );
    require( testObj.m_all_interesting_ticks.size() == 2, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 0, "The returned ID is correct for searching a single item" );
    
    id_obtained = testObj.getInterestingTick( 17, 0, testObj.m_all_interesting_ticks.size()-1 );
    require( testObj.m_all_interesting_ticks.size() == 2, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 1, "The returned ID is correct for searching a single item" );
    
    // ---- add 13
    id_added = testObj.getInterestingTick( 13, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 3, "A third element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 0, "The returned ID is correct for the third added item" );
    require( testObj.m_all_interesting_ticks[0].m_tick == 13, "third added item was correctly added" );
    
    // ---- add 14
    id_added = testObj.getInterestingTick( 14, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 4, "A fourth element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 1, "The returned ID is correct for the fourth added item" );
    require( testObj.m_all_interesting_ticks[0].m_tick == 13, "fourth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[1].m_tick == 14, "fourth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[2].m_tick == 15, "fourth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[3].m_tick == 17, "fourth added item was correctly added" );

    // ---- add 16
    id_added = testObj.getInterestingTick( 16, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( testObj.m_all_interesting_ticks.size() == 5, "A fifth element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 3, "The returned ID is correct for the fifth added item" );
    require( testObj.m_all_interesting_ticks[0].m_tick == 13, "fifth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[1].m_tick == 14, "fifth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[2].m_tick == 15, "fifth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[3].m_tick == 16, "fifth added item was correctly added" );
    require( testObj.m_all_interesting_ticks[4].m_tick == 17, "fifth added item was correctly added" );

}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getNextTick(const int fromTick)
{
    const int tickAmount = m_all_interesting_ticks.size();
    for (int n=0; n<tickAmount; n++)
    {
        // Assert that it's sorted
        ASSERT(n == 0 or m_all_interesting_ticks[n].m_tick >= m_all_interesting_ticks[n-1].m_tick);
        
        if (m_all_interesting_ticks[n].m_tick > fromTick)
        {
            return m_all_interesting_ticks[n].m_tick;
        }
    }
    
    // No other symbol found in this measure, in this track. Return the end
    return m_end_of_measure_tick;
}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getNextTickInTrack(const int fromTick, const int trackID)
{
    const int tickAmount = m_all_interesting_ticks.size();
    for (int n=0; n<tickAmount; n++)
    {
        // Assert that it's sorted
        ASSERT(n == 0 or m_all_interesting_ticks[n].m_tick >= m_all_interesting_ticks[n-1].m_tick);
        
        if (m_all_interesting_ticks[n].m_tick > fromTick and m_all_interesting_ticks[n].hasSymbolInTrack(trackID))
        {
            return m_all_interesting_ticks[n].m_tick;
        }
    }
    
    // No other symbol found in this measure, in this track. Return the end
    return m_end_of_measure_tick;
}

UNIT_TEST( TestFindingNextTick )
{
    const int last_tick_in_measure = 11;
    RelativePlacementManager testObj(last_tick_in_measure);
        
    //    1     3     5     7     9
    //  /       ø=====      ø======  \    Track 0
    //  \ ø=====ø=====ø=====         /
    //
    //  / ø===========               \    Track 1
    //  \             ø============  /
     
    // add 1
    testObj.getInterestingTick( 1, 0, testObj.m_all_interesting_ticks.size()-1 );
    testObj.addSymbol(1 /* from */, 3 /* to */, 1 /* symbol size */, 0 /* track ID */);
    testObj.addSymbol(1 /* from */, 5 /* to */, 1 /* symbol size */, 1 /* track ID */);
    
    // add 3
    testObj.getInterestingTick( 3, 0, testObj.m_all_interesting_ticks.size()-1 );
    testObj.addSymbol(3 /* from */, 5 /* to */, 1 /* symbol size */, 0 /* track ID */);
    testObj.addSymbol(3 /* from */, 5 /* to */, 1 /* symbol size */, 0 /* track ID */);

    // add 5
    testObj.getInterestingTick( 5, 0, testObj.m_all_interesting_ticks.size()-1 );
    testObj.addSymbol(5 /* from */, 7 /* to */, 1 /* symbol size */, 0 /* track ID */);
    testObj.addSymbol(5 /* from */, 9 /* to */, 1 /* symbol size */, 1 /* track ID */);

    // add 7
    testObj.getInterestingTick( 7, 0, testObj.m_all_interesting_ticks.size()-1 );
    testObj.addSymbol(7 /* from */, 9 /* to */, 1 /* symbol size */, 0 /* track ID */);

    // add 9
    testObj.getInterestingTick( 9, 0, testObj.m_all_interesting_ticks.size()-1 );
    
    // ---- perform tests in track 0
    int next_tick = testObj.getNextTickInTrack(1 /* from */, 0 /* track ID */);
    require( next_tick == 3, "Next tick is OK in track 0" );
    
    next_tick = testObj.getNextTickInTrack(3 /* from */, 0 /* track ID */);
    require( next_tick == 5, "Next tick is OK in track 0" );
    
    next_tick = testObj.getNextTickInTrack(5 /* from */, 0 /* track ID */);
    require( next_tick == 7, "Next tick is OK in track 0" );
    
    next_tick = testObj.getNextTickInTrack(7 /* from */, 0 /* track ID */);
    require( next_tick == last_tick_in_measure, "Next tick is OK in track 0" );
    
    // ---- perform tests in track 1
    next_tick = testObj.getNextTickInTrack(1 /* from */, 1 /* track ID */);
    require( next_tick == 5, "Next tick is OK in track 1" );
    
    next_tick = testObj.getNextTickInTrack(5 /* from */, 1 /* track ID */);
    require( next_tick == last_tick_in_measure, "Next tick is OK in track 1" );
}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::findShortestSymbolLength() const
{
    int shortest = -1;
    
    const int tickAmount = m_all_interesting_ticks.size();

    for (int n=0; n<tickAmount; n++)
    {
        const InterestingTick& currTick = m_all_interesting_ticks[n];
        
        const int symbolAmount = currTick.m_all_symbols_on_that_tick.size();
        for (int sym=0; sym<symbolAmount; sym++)
        {
            const Symbol& currSym = currTick.m_all_symbols_on_that_tick[sym];
            const int newAttempt = currSym.m_end_tick - currTick.m_tick;
            
            if (newAttempt <= 0)
            {
                std::cerr << "Warning, RelativePlacementManager::findShortestSymbolLength() found a note of length " << newAttempt << ", that's kinda weird!\n"; 
                continue;
            }
            
            if (newAttempt < shortest or shortest == -1)
            {
                shortest = newAttempt;
            }
        }
    }
    
    return shortest;
}

// ----------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Public
#endif


void RelativePlacementManager::addSymbol(int tickFrom, int tickTo, int symbolWidth, int trackID)
{
#if RPM_CHATTY
    std::cout << "++++ addSymbol " << symbolWidth << "; tick " << tickFrom << " track " << trackID << std::endl;
#endif
    
    ASSERT_E(tickFrom, <, m_end_of_measure_tick);
    ASSERT_E(tickFrom, <=, tickTo);
    //ASSERT (tickTo <= m_end_of_measure_tick);
    
    const int interestingTickID = getInterestingTick( tickFrom, 0, m_all_interesting_ticks.size()-1 );
    ASSERT_E( interestingTickID, >=, 0 );
    ASSERT_E( interestingTickID, <, (int)m_all_interesting_ticks.size() );
    ASSERT_E( m_all_interesting_ticks[interestingTickID].m_tick, ==, tickFrom );
    
    m_all_interesting_ticks[interestingTickID].m_all_symbols_on_that_tick.push_back(
                        Symbol(tickTo, symbolWidth, trackID) );
}

// ----------------------------------------------------------------------------------------------------------------

void RelativePlacementManager::addSilenceSymbols(const std::vector< SilenceAnalyser::SilenceInfo >& silences_ticks,
                                             const int trackID, const int firstTickInMeasure, const int lastTickInMeasure)
{
    // ---- silences
    const int silenceAmount = silences_ticks.size();
    for (int n=0; n<silenceAmount; n++)
    {
        if (silences_ticks[n].m_tick_range.from < firstTickInMeasure or
            silences_ticks[n].m_tick_range.from >= lastTickInMeasure)
        {
            continue;
        }
        
#if VERBOSE
        std::cout << "    Adding [silence] tick " << silences_ticks[n] << " to list" << std::endl;
#endif
        
        int neededSize = 0;
        
        switch (silences_ticks[n].m_type)
        {
            case 1:
            case 2:
                neededSize = RECTANGULAR_SILENCE_SIZE + RECTANGULAR_SILENCE_LEFT_MARGIN;
                break;
            case 4:
            case 8:
            case 16:
                neededSize = 90; // FIXME: when using vector silence symbols, fix this to not use a hardcoded value
                break;
            default:
                std::cerr << "WARNING, unknown silence type : " << silences_ticks[n].m_type << std::endl;
                neededSize = 90;
        }
        
        if (silences_ticks[n].m_dotted) neededSize += 40; // a bit approximative for now
        
        addSymbol( silences_ticks[n].m_tick_range.from,
                   silences_ticks[n].m_tick_range.to,
                   neededSize, trackID );
    }    
}

// ----------------------------------------------------------------------------------------------------------------

void RelativePlacementManager::calculateRelativePlacement()
{
    // 1. Build the list of units
    // 2. Calculate the needed size of each unit by considering :
    //     2.1. The proportion of the largest symbol on that tick
    //     2.2. The "area of influence" of each symbol : larger areas of influence need
    //          to receive more space to make the printed notation easier to read
    //          (e.g. a whole figure should receive more room than a 1/8th)
    //          This step must consider how notes are distributed across tracks; if a whole
    //          is alone in its track, and the other track is filled with 1/8ths, the whole
    //          will automatically have enough space in its own track without need to allocate
    //          more. e.g. :
    //
    //  Track 1 |ø===|ø===|ø===|ø===|   But  |ø=======|ø===|ø===|  (first note must be larger, since its extra
    //  Track 2 |ø===|====|====|====|        |ø=======|ø========|   space is not granted by the other track)
    
    const int tickAmount = m_all_interesting_ticks.size();

    if (tickAmount == 0) return;

    const int shortestSymbolLength = findShortestSymbolLength();
    ASSERT(shortestSymbolLength > 0);
    
    m_total_needed_size = 0;

    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        
        // --------------------------------------------------------------------------------
        // Find how much space we need on that tick to fit all symbols
        // (i.e. find the biggest symbol)
        
        int spaceNeededToFitAllSymbols = 0;
        int spaceNeededAfterSymbolForProportions = 0;
        
        const int symbolAmount = currTick.m_all_symbols_on_that_tick.size();
        ASSERT_E(symbolAmount, >, 0);
        
#if RPM_CHATTY
        std::cout << "    {\n";
#endif
        for (int sym=0; sym<symbolAmount; sym++)
        {
            Symbol& currSym = currTick.m_all_symbols_on_that_tick[sym];
            
            currSym.fromUnit = n;
            
            //const int nextTickInTrack = getNextTickInTrack(currTick.tick, currSym.trackID);
            const int nextTickInAnyTrack = getNextTick(currTick.m_tick);

            // 2 cases : either the needed space for this symbol is implicitely granted by symbols
            // on other lines (see above), either it's not
            if (/*currSym.endTick <= nextTickInTrack and*/ currSym.m_end_tick > nextTickInAnyTrack)
            {
                currSym.neededAdditionalProportion = 0.0f;
            }
            else
            {
                // space is not implicitely granted, we need to give more space manually
                //const int whats_missing = std::max(0, currSym.endTick - nextTickInAnyTrack);
                //float ratioToShortest = (float)whats_missing / (float)shortestSymbolLength;
                
                ASSERT_E(shortestSymbolLength, >=, 0);
                
                const int length = currSym.m_end_tick - currTick.m_tick;
                if (length <= 0)
                {
                    std::cerr << "Warning, RelativePlacementManager::calculateRelativePlacement() found a "
                              << " note of length " << (currSym.m_end_tick - currTick.m_tick)
                              << ", which is kinda weird!\n";
                    currSym.neededAdditionalProportion = 0.0f;
                    continue;
                }
                
                float ratioToShortest = (float)length / (float)shortestSymbolLength;
                
                // if the ratio is smaller than 1, then we don't have the right "shortest"...
                ASSERT_E(ratioToShortest, >=, 1.0);
                
                if (ratioToShortest >= 1)
                {
                    // grow logaritmically (the factor is empirical - FIXME)
                    currSym.neededAdditionalProportion = (float)std::log( ratioToShortest ) /
                                                            (float)std::log( 2 ) * 90.0f;
                    ASSERT_E( currSym.neededAdditionalProportion, >=, 0.0f );
                    //std::cout << "currSym.neededAdditionalProportion = " << currSym.neededAdditionalProportion << "\n";
                }
                else // no additionnal needed space
                {
                    currSym.neededAdditionalProportion = 0.0f;
                }
#if RPM_CHATTY
                std::cout << "ratioToShortest=" << ratioToShortest
                          << ", neededAdditionalProportion="
                          << currSym.neededAdditionalProportion << std::endl;
#endif
            }
            
            // determine the largest needed proportion for each tick
            spaceNeededToFitAllSymbols = std::max( spaceNeededToFitAllSymbols, currSym.m_width_in_print_units );
            
            spaceNeededAfterSymbolForProportions = std::max( spaceNeededAfterSymbolForProportions,
                                                             (int)round(currSym.neededAdditionalProportion) );
            
#if RPM_CHATTY
            std::cout << "      symbolWidth=" << symbolWidth
                      << "; spaceNeededToFitAllSymbols=" << spaceNeededToFitAllSymbols 
                      << "; spaceNeededAfterSymbolForProportions=" << spaceNeededAfterSymbolForProportions <<std::endl;
#endif
            
        } // end for each symbol
        
#if RPM_CHATTY
        std::cout << "    }\n";
#endif
        
        currTick.m_size = spaceNeededToFitAllSymbols;
        
        // --------------------------------------------------------------------------------
        // Set coordinatess
        
        // set start position of current unit (without caring yet for value to be in range [0, 1])
        currTick.m_position     = m_total_needed_size;
        currTick.m_end_position = m_total_needed_size + spaceNeededToFitAllSymbols;

        // set end position of previous unit (without caring yet for value to be in range [0, 1])
        //if (n>0) m_all_interesting_ticks[n-1].endPosition = totalAbsolutePosition;
        
        ASSERT_E( spaceNeededToFitAllSymbols, >, 0 );
        
        // increment current position
        m_total_needed_size += spaceNeededToFitAllSymbols + spaceNeededAfterSymbolForProportions;
    }
    
    // for last
    //m_all_interesting_ticks[m_all_interesting_ticks.size()-1].endPosition = totalAbsolutePosition;

    m_total_needed_size += SIDE_MARGIN_WIDTH; // leave empty at the end
    
    // ------------------------------------------------------------------------------------
    // "normalize" positions (divide to be in range [0, 1])
    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        
        currTick.m_position     = currTick.m_position     / m_total_needed_size;
        currTick.m_end_position = currTick.m_end_position / m_total_needed_size;
        
#if RPM_CHATTY
        std::cout << "m_all_interesting_ticks[" << n << "].position = "
                  << currTick.position << " to " << currTick.endPosition << std::endl;
#endif
        
        ASSERT_E(currTick.m_position, >=, 0.0f);
        ASSERT_E(currTick.m_position, <=, 1.0f);
        
        ASSERT_E(currTick.m_end_position, >=, 0.0f);
        ASSERT_E(currTick.m_end_position, <=, 1.0f);
    }
    
}

// ----------------------------------------------------------------------------------------------------------------

Range<float> RelativePlacementManager::getSymbolRelativeArea(int tick) const
{    
    int id = getInterestingTickNoAdd( tick, 0, m_all_interesting_ticks.size()-1 );
    
    if (id == -1)
    {
        std::cerr << "WARNING: RelativePlacementManager::getSymbolRelativeArea could not find tick " << tick 
                  << " (beat " << tick/float(getMeasureData()->beatLengthInTicks()) << ")\n";
        //ASSERT(false);
        return Range<float>(0, 0);
    }
    
    ASSERT(id < (int)m_all_interesting_ticks.size());
    
    const InterestingTick& currTick = m_all_interesting_ticks[id];
    return Range<float>(currTick.m_position, currTick.m_end_position);
}

// ----------------------------------------------------------------------------------------------------------------

