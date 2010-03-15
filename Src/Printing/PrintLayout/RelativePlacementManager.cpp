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
#include "unit_test.h"
#include <iostream>
#include <cmath>
#include <cassert>

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
        const int pivot_tick = m_all_interesting_ticks[pivot_id].tick;
        
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
        const int pivot_tick = m_all_interesting_ticks[pivot_id].tick;
        
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

UNIT_TEST( RelativePlacementManager_TestAddingAndFindingInterestingTicks )
{
    //throw std::logic_error("C'est normal que ça échoue!!");
    RelativePlacementManager test(20);
    
    require( test.m_all_interesting_ticks.size() == 0, "RelativePlacementManager is initially empty" );
    
    // ---- add 15
    int id_added = test.getInterestingTick( 15, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 1, "An element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 0, "The returned ID is correct for the first added item" );
    require( test.m_all_interesting_ticks[0].tick == 15, "First added item was correctly added" );
    
    int id_obtained = test.getInterestingTick( 15, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 1, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 0, "The returned ID is correct for searching a single item" );
    
    // ---- add 17
    id_added = test.getInterestingTick( 17, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 2, "An second element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 1, "The returned ID is correct for the second added item" );
    require( test.m_all_interesting_ticks[1].tick == 17, "Second added item was correctly added" );
    
    id_obtained = test.getInterestingTick( 15, 0, test.m_all_interesting_ticks.size()-1 );
    require( test.m_all_interesting_ticks.size() == 2, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 0, "The returned ID is correct for searching a single item" );
    
    id_obtained = test.getInterestingTick( 17, 0, test.m_all_interesting_ticks.size()-1 );
    require( test.m_all_interesting_ticks.size() == 2, "No element was added when we wanted to obtained one that already exists" );
    require( id_obtained == 1, "The returned ID is correct for searching a single item" );
    
    // ---- add 13
    id_added = test.getInterestingTick( 13, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 3, "A third element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 0, "The returned ID is correct for the third added item" );
    require( test.m_all_interesting_ticks[0].tick == 13, "third added item was correctly added" );
    
    // ---- add 14
    id_added = test.getInterestingTick( 14, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 4, "A fourth element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 1, "The returned ID is correct for the fourth added item" );
    require( test.m_all_interesting_ticks[0].tick == 13, "fourth added item was correctly added" );
    require( test.m_all_interesting_ticks[1].tick == 14, "fourth added item was correctly added" );
    require( test.m_all_interesting_ticks[2].tick == 15, "fourth added item was correctly added" );
    require( test.m_all_interesting_ticks[3].tick == 17, "fourth added item was correctly added" );

    // ---- add 16
    id_added = test.getInterestingTick( 16, 0, test.m_all_interesting_ticks.size()-1 );
    
    // __print__
    //for (int n=0; n<test.m_all_interesting_ticks.size(); n++) std::cout << test.m_all_interesting_ticks[n].tick << "  ";
    //std::cout << "\n";
    // _________
    
    require( test.m_all_interesting_ticks.size() == 5, "A fifth element was added when we wanted to obtained it and it wasn't already there" );
    require( id_added == 3, "The returned ID is correct for the fifth added item" );
    require( test.m_all_interesting_ticks[0].tick == 13, "fifth added item was correctly added" );
    require( test.m_all_interesting_ticks[1].tick == 14, "fifth added item was correctly added" );
    require( test.m_all_interesting_ticks[2].tick == 15, "fifth added item was correctly added" );
    require( test.m_all_interesting_ticks[3].tick == 16, "fifth added item was correctly added" );
    require( test.m_all_interesting_ticks[4].tick == 17, "fifth added item was correctly added" );

}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getNextTick(const int from_tick)
{
    const int tickAmount = m_all_interesting_ticks.size();
    for (int n=0; n<tickAmount; n++)
    {
        // Assert that it's sorted
        assert(n == 0 or m_all_interesting_ticks[n].tick >= m_all_interesting_ticks[n-1].tick);
        
        if (m_all_interesting_ticks[n].tick > from_tick)
        {
            return m_all_interesting_ticks[n].tick;
        }
    }
    
    // No other symbol found in this measure, in this track. Return the end
    return m_end_of_measure_tick;
}

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getNextTickInTrack(const int from_tick, const int trackID)
{
    const int tickAmount = m_all_interesting_ticks.size();
    for (int n=0; n<tickAmount; n++)
    {
        // Assert that it's sorted
        assert(n == 0 or m_all_interesting_ticks[n].tick >= m_all_interesting_ticks[n-1].tick);
        
        if (m_all_interesting_ticks[n].tick > from_tick and m_all_interesting_ticks[n].hasSymbolInTrack(trackID))
        {
            return m_all_interesting_ticks[n].tick;
        }
    }
    
    // No other symbol found in this measure, in this track. Return the end
    return m_end_of_measure_tick;
}

UNIT_TEST( RelativePlacementManager_TestFindingNextTick )
{
    const int last_tick_in_measure = 11;
    RelativePlacementManager test(last_tick_in_measure);
        
    //    1     3     5     7     9
    //  /       ø=====      ø======  \    Track 0
    //  \ ø=====ø=====ø=====         /
    //
    //  / ø===========               \    Track 1
    //  \             ø============  /
     
    // add 1
    test.getInterestingTick( 1, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(1 /* from */, 3 /* to */, 1 /* symbol size */, 0 /* track ID */);
    test.addSymbol(1 /* from */, 5 /* to */, 1 /* symbol size */, 1 /* track ID */);
    
    // add 3
    test.getInterestingTick( 3, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(3 /* from */, 5 /* to */, 1 /* symbol size */, 0 /* track ID */);
    test.addSymbol(3 /* from */, 5 /* to */, 1 /* symbol size */, 0 /* track ID */);

    // add 5
    test.getInterestingTick( 5, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(5 /* from */, 7 /* to */, 1 /* symbol size */, 0 /* track ID */);
    test.addSymbol(5 /* from */, 9 /* to */, 1 /* symbol size */, 1 /* track ID */);

    // add 7
    test.getInterestingTick( 7, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(7 /* from */, 9 /* to */, 1 /* symbol size */, 0 /* track ID */);

    // add 9
    test.getInterestingTick( 9, 0, test.m_all_interesting_ticks.size()-1 );
    
    // ---- perform tests in track 0
    int next_tick = test.getNextTickInTrack(1 /* from */, 0 /* track ID */);
    require( next_tick == 3, "Next tick is OK in track 0" );
    
    next_tick = test.getNextTickInTrack(3 /* from */, 0 /* track ID */);
    require( next_tick == 5, "Next tick is OK in track 0" );
    
    next_tick = test.getNextTickInTrack(5 /* from */, 0 /* track ID */);
    require( next_tick == 7, "Next tick is OK in track 0" );
    
    next_tick = test.getNextTickInTrack(7 /* from */, 0 /* track ID */);
    require( next_tick == last_tick_in_measure, "Next tick is OK in track 0" );
    
    // ---- perform tests in track 1
    next_tick = test.getNextTickInTrack(1 /* from */, 1 /* track ID */);
    require( next_tick == 5, "Next tick is OK in track 1" );
    
    next_tick = test.getNextTickInTrack(5 /* from */, 1 /* track ID */);
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
        
        const int symbolAmount = currTick.all_symbols_on_that_tick.size();
        for (int sym=0; sym<symbolAmount; sym++)
        {
            const Symbol& currSym = currTick.all_symbols_on_that_tick[sym];
            const int newAttempt = currSym.endTick - currTick.tick;
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
    
    assert (tickFrom < m_end_of_measure_tick);
    //assert (tickTo <= m_end_of_measure_tick);
    
    const int interestingTickID = getInterestingTick( tickFrom, 0, m_all_interesting_ticks.size()-1 );
    assert( interestingTickID >= 0 );
    assert( interestingTickID < (int)m_all_interesting_ticks.size() );
    assert( m_all_interesting_ticks[interestingTickID].tick == tickFrom );
    
    m_all_interesting_ticks[interestingTickID].all_symbols_on_that_tick.push_back(
                        Symbol(tickTo, symbolWidth, trackID) );
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
    assert(shortestSymbolLength > 0);
    
    int totalAbsolutePosition = 0;

    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        
        int global_width_for_tick = 0;
        
        const int symbolAmount = currTick.all_symbols_on_that_tick.size();
        assertExpr(symbolAmount, >, 0);
        
#if RPM_CHATTY
        std::cout << "    {\n";
#endif
        for (int sym=0; sym<symbolAmount; sym++)
        {
            Symbol& currSym = currTick.all_symbols_on_that_tick[sym];
            
            currSym.fromUnit = n;
            
            //const int nextTickInTrack = getNextTickInTrack(currTick.tick, currSym.trackID);
            const int nextTickInAnyTrack = getNextTick(currTick.tick);

            // 2 cases : either the needed space for this symbol is implicitely granted by symbols
            // on other lines (see above), either it's not
            if (/*currSym.endTick <= nextTickInTrack and*/ currSym.endTick > nextTickInAnyTrack)
            {
                currSym.neededAdditionalProportion = 0.0f;
            }
            else
            {
                // space is not implicitely granted, we need to give more space manually
                //const int whats_missing = std::max(0, currSym.endTick - nextTickInAnyTrack);
                //float ratioToShortest = (float)whats_missing / (float)shortestSymbolLength;
                
                assertExpr(shortestSymbolLength, >=, 0);
                
                const int length = currSym.endTick - currTick.tick;
                float ratioToShortest = (float)length / (float)shortestSymbolLength;
                
                // if the ratio is smaller than 1, then we don't have the right "shortest"...
                assertExpr(ratioToShortest, >=, 1.0);
                
                if (ratioToShortest >= 1)
                {
                    // gros logaritmically (the 0.6 factor is empirical - FIXME)
                    currSym.neededAdditionalProportion = (float)std::log( ratioToShortest ) / (float)std::log( 2 )*0.6f;
                    //std::cout << "ratioToShortest=" << ratioToShortest << ", neededAdditionalProportion=" << currSym.neededAdditionalProportion << std::endl;
                }
                else // no additionnal needed space
                {
                    currSym.neededAdditionalProportion = 0.0f;
                }
#if RPM_CHATTY
                std::cout << "ratioToShortest=" << ratioToShortest << ", neededAdditionalProportion=" << currSym.neededAdditionalProportion << std::endl;
#endif
            }

            const int symbolWidth = (int)round(currSym.widthInPrintUnits * (1.0f + currSym.neededAdditionalProportion));
                                    
            // determine the largest needed proportion for each tick
            global_width_for_tick = std::max( global_width_for_tick, symbolWidth);
            
#if RPM_CHATTY
            std::cout << "      symbolWidth=" << symbolWidth << "; global_width_for_tick=" << global_width_for_tick << std::endl;
#endif
            
        } // end for each symbol
        
#if RPM_CHATTY
        std::cout << "    }\n";
#endif
        
        currTick.size = global_width_for_tick;
        
        // set start position of current unit (without caring yet for value to be in range [0, 1])
        currTick.position = totalAbsolutePosition;
        
        // set end position of previous unit (without caring yet for value to be in range [0, 1])
        if (n>0) m_all_interesting_ticks[n-1].endPosition = totalAbsolutePosition;
        
        assertExpr( global_width_for_tick, >, 0 );
        
        // increment current position
        totalAbsolutePosition += global_width_for_tick;
    }
    
    // for last
    m_all_interesting_ticks[m_all_interesting_ticks.size()-1].endPosition = totalAbsolutePosition;

    totalAbsolutePosition += SIDE_MARGIN_WIDTH; // leave empty at the end
    
    // "normalize" positions (divide to be in range [0, 1])
    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        
        currTick.position    = currTick.position    / totalAbsolutePosition;
        currTick.endPosition = currTick.endPosition / totalAbsolutePosition;
        
#if RPM_CHATTY
        std::cout << "m_all_interesting_ticks[" << n << "].position = " << currTick.position << " to " << currTick.endPosition << std::endl;
#endif
        
        assertExpr(currTick.position, >=, 0.0f);
        assertExpr(currTick.position, <=, 1.0f);
        
        assertExpr(currTick.endPosition, >=, 0.0f);
        assertExpr(currTick.endPosition, <=, 1.0f);
    }
    
}

// ----------------------------------------------------------------------------------------------------------------

Range<float> RelativePlacementManager::getSymbolRelativeArea(int tick) const
{    
    int id = getInterestingTickNoAdd( tick, 0, m_all_interesting_ticks.size()-1 );
    assert(id >= 0);
    assert(id < (int)m_all_interesting_ticks.size());
    
    const InterestingTick& currTick = m_all_interesting_ticks[id];
    return Range<float>(currTick.position, currTick.endPosition);
    
    /*
    const int symbolAmount = currTick.all_symbols_on_that_tick.size();
    for (int sym=0; sym<symbolAmount; sym++)
    {
        Symbol& currSym = currTick.all_symbols_on_that_tick[sym];

        if (currSym.trackID == trackID and currSym.endTick == endTick)
        {
            
        }
    }*/
}

// ----------------------------------------------------------------------------------------------------------------

/*
int RelativePlacementManager::getUnitCount() const
{
    return m_all_interesting_ticks.size();
}*/

// ----------------------------------------------------------------------------------------------------------------

int RelativePlacementManager::getWidth() const
{
    int totalSize = 0;
    
#if RPM_CHATTY
    std::cout << "RelativePlacementManager::getWidth()\n{\n";
#endif
    
    const int amount = m_all_interesting_ticks.size();
    for (int n=0; n<amount; n++)
    {
#if RPM_CHATTY
        std::cout << "    " << m_all_interesting_ticks[n].size << std::endl;
#endif
        totalSize += m_all_interesting_ticks[n].size;
    }
    
#if RPM_CHATTY
    std::cout << "}\n";
#endif
    
    return totalSize;
}


