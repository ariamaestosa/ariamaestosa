
#include "RelativePlacementManager.h"

#include "AriaCore.h"
#include "unit_test.h"
#include <iostream>
#include <cmath>
#include <cassert>

#if 0
#pragma mark Private
#endif

using namespace AriaMaestosa;

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
    test.addSymbol(1 /* from */, 3 /* to */, 1.0f, 0 /* track ID */);
    test.addSymbol(1 /* from */, 5 /* to */, 1.0f, 1 /* track ID */);
    
    // add 3
    test.getInterestingTick( 3, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(3 /* from */, 5 /* to */, 1.0f, 0 /* track ID */);
    test.addSymbol(3 /* from */, 5 /* to */, 1.0f, 0 /* track ID */);

    // add 5
    test.getInterestingTick( 5, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(5 /* from */, 7 /* to */, 1.0f, 0 /* track ID */);
    test.addSymbol(5 /* from */, 9 /* to */, 1.0f, 1 /* track ID */);

    // add 7
    test.getInterestingTick( 7, 0, test.m_all_interesting_ticks.size()-1 );
    test.addSymbol(7 /* from */, 9 /* to */, 1.0f, 0 /* track ID */);

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
            if (currSym.endTick - currTick.tick > shortest or shortest == -1)
            {
                shortest = currSym.endTick - currTick.tick;
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


void RelativePlacementManager::addSymbol(int tickFrom, int tickTo, float symbolProportion, int trackID)
{
    assert (tickFrom < m_end_of_measure_tick);
    //assert (tickTo <= m_end_of_measure_tick);
    
    const int interestingTickID = getInterestingTick( tickFrom, 0, m_all_interesting_ticks.size()-1 );
    assert( interestingTickID >= 0 );
    assert( interestingTickID < (int)m_all_interesting_ticks.size() );
    assert( m_all_interesting_ticks[interestingTickID].tick == tickFrom );
    
    m_all_interesting_ticks[interestingTickID].all_symbols_on_that_tick.push_back(
                        Symbol(tickTo, symbolProportion, trackID) );
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
    
    const int shortestSymbolLength = findShortestSymbolLength();
    assert(shortestSymbolLength != -1);
    
    const int tickAmount = m_all_interesting_ticks.size();
    float totalRelativePosition = 0.0f;

    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        
        float global_proportion_for_tick = 0.0f;
        
        const int symbolAmount = currTick.all_symbols_on_that_tick.size();
        assertExpr(symbolAmount, >, 0);
        for (int sym=0; sym<symbolAmount; sym++)
        {
            Symbol& currSym = currTick.all_symbols_on_that_tick[sym];
            
            currSym.fromUnit = n;
            
            const int nextTickInTrack = getNextTickInTrack(currTick.tick, currSym.trackID);
            const int nextTick = getNextTick(currTick.tick);

            // 2 cases : either the needed space for this symbol is implicitely granted by symbols
            // on other lines (see above), either it's not
            if (currSym.endTick <= nextTickInTrack and currSym.endTick > nextTick)
            {
                currSym.neededAdditionalProportion = 0.0f;
            }
            else
            {
                // space is not implicitely granted, we need to give more space manually
                const int whats_missing = std::max(0, currSym.endTick - nextTick);
                
                float ratioToShortest = (float)whats_missing / (float)shortestSymbolLength;
                if (ratioToShortest > 0)
                {
                    currSym.neededAdditionalProportion = std::log( ratioToShortest ) / std::log( 2 );
                }
                else
                {
                    currSym.neededAdditionalProportion = 0.0f;
                }
            }
            
            std::cout << "    currSym.proportion + currSym.neededAdditionalProportion = " << currSym.proportion << " + " << currSym.neededAdditionalProportion << std::endl; 
            
            // determine the largest needed proportion for each tick
            global_proportion_for_tick = std::max( global_proportion_for_tick,
                                                   currSym.proportion + currSym.neededAdditionalProportion );
        } // end for each symbol
        
        currTick.proportion = global_proportion_for_tick;
        
        // set start position of current unit (without caring yet for value to be inr ange [0, 1])
        currTick.position = totalRelativePosition;
        
        // set end position of previous unit (without caring yet for value to be inr ange [0, 1])
        if (n>0) m_all_interesting_ticks[n-1].endPosition = totalRelativePosition;
        
        assertExpr( global_proportion_for_tick, >, 0 );
        
        // increment current position
        totalRelativePosition += global_proportion_for_tick;
    }
    
    // for last
    m_all_interesting_ticks[m_all_interesting_ticks.size()-1].endPosition = totalRelativePosition;
    
    totalRelativePosition += 1.0f; // leave 1 space worth of empty space at the end
    
    // "normalize" positions and sizes (divide them to be in range [0, 1]
    for (int n=0; n<tickAmount; n++)
    {
        InterestingTick& currTick = m_all_interesting_ticks[n];
        

        currTick.position    = currTick.position/ totalRelativePosition;
        currTick.endPosition = currTick.endPosition / totalRelativePosition;
        
        std::cout << "m_all_interesting_ticks[" << n << "].position = " << currTick.position << " to " << currTick.endPosition << std::endl;
        
        assertExpr(currTick.position, >=, 0.0f);
        assertExpr(currTick.position, <=, 1.0f);
        
        assertExpr(currTick.endPosition, >=, 0.0f);
        assertExpr(currTick.endPosition, <=, 1.0f);
    }
    
}

// ----------------------------------------------------------------------------------------------------------------

Range<float> RelativePlacementManager::getSymbolRelativeArea(int tick)
{    
    int id = getInterestingTick( tick, 0, m_all_interesting_ticks.size()-1 );
    assert(id >= 0);
    assert(id < (int)m_all_interesting_ticks.size());
    
    InterestingTick& currTick = m_all_interesting_ticks[id];
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

int RelativePlacementManager::getUnitCount() const
{
    return m_all_interesting_ticks.size();
}


