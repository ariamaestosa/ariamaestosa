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

#include "AriaCore.h"
#include "PrintLayoutMeasure.h"
#include "Midi/MeasureData.h"
#include "Midi/Track.h"

#include "Printing/PrintLayout/PrintLayoutAbstract.h"

namespace AriaMaestosa
{
    const PrintLayoutMeasure NULL_MEASURE(-1);
}

#define PLM_CHATTY 1

using namespace AriaMaestosa;
// -------------------------------------------------------------------------------------------
    
PrintLayoutMeasure::PrintLayoutMeasure(const int measID) :
    ticks_placement_manager(measID == -1 ? 0 : getMeasureData()->lastTickInMeasure( measID ))
{
    m_shortest_duration  = -1;
    firstSimilarMeasure  = -1;
    cutApart             = false;
    m_measure_id         = measID;
    m_contains_something = false;
    
    if (measID != -1)
    {
        m_first_tick = getMeasureData()->firstTickInMeasure( measID );
        m_last_tick  = getMeasureData()->lastTickInMeasure ( measID );
    }
}

// -------------------------------------------------------------------------------------------
    
bool PrintLayoutMeasure::calculateIfMeasureIsSameAs(PrintLayoutMeasure& checkMeasure)
{
    
    const int trackRefAmount = m_track_refs.size();
    int total_note_amount = 0;
    
    for (int tref=0; tref<trackRefAmount; tref++)
    {
        const int my_first_note  = m_track_refs[tref].getFirstNote();
        const int my_last_note   = m_track_refs[tref].getLastNote();
        const int his_first_note = checkMeasure.m_track_refs[tref].getFirstNote();
        const int his_last_note  = checkMeasure.m_track_refs[tref].getLastNote();
        
        ASSERT( m_track_refs.size() == checkMeasure.m_track_refs.size() );
        ASSERT_E( tref,<,(int)m_track_refs.size() );
        ASSERT( m_track_refs[tref].getTrack() == checkMeasure.m_track_refs[tref].getTrack() );
        Track* track = m_track_refs[tref].getTrack();
        
        // if these 2 measures don't even have the same number of notes, they're definitely not the same
        if ( (his_last_note - his_first_note + 1) != (my_last_note - my_first_note + 1) )
        {
            return false;
        }
        
        
        const int noteAmount = (his_last_note - his_first_note);
        total_note_amount += noteAmount;
        
        // don't count empty measures as repetitions 
        if (noteAmount<1)
        {
            // when comparing a multiple-track line, don't stop on empty measures, for other tracks may not be empty
            // for multi-track lines, variable 'total_note_amount' will be checked at the end to verify measure is not empty
            if (trackRefAmount>1) continue;
            return false;
        }
        
        /*
         if we get till there, the 2 measures have the same amount of notes.
         to know whether they are truly identitcal, we need to compare note by note
         we will match each notes from the first measure to the identical one in the second.
         If ever one note failes to be matched, then the 2 measures are different.
         */
        int noteMatched_this[noteAmount];
        int noteMatched_other[noteAmount];
        for (int n=0; n<noteAmount; n++)
        {
            noteMatched_this[n] = false;
            noteMatched_other[n] = false;
        }
        
        for (int checkNote_this=0; checkNote_this<=noteAmount; checkNote_this++)
        {
            for (int checkNote_other=0; checkNote_other<=noteAmount; checkNote_other++)
            {
                if (noteMatched_other[checkNote_other]) continue; // this note was already matched
                
                // check start tick matches
                if (track->getNoteStartInMidiTicks(his_first_note + checkNote_other) - checkMeasure.getFirstTick() !=
                   track->getNoteStartInMidiTicks(my_first_note + checkNote_this) - getFirstTick())
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check end tick matches
                if (track->getNoteEndInMidiTicks(his_first_note + checkNote_other) - checkMeasure.getFirstTick() !=
                   track->getNoteEndInMidiTicks(my_first_note + checkNote_this) - getFirstTick())
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check pitch matches
                if (track->getNotePitchID(his_first_note + checkNote_other) !=
                   track->getNotePitchID(my_first_note + checkNote_this))
                {
                    // they dont match, check the next one
                    continue;
                }
                
                noteMatched_this[checkNote_this] = true;
                noteMatched_other[checkNote_other] = true;
                
                
            }//next note
            
            // we couldn't find a note in the other measure that matches this one
            if (noteMatched_this[checkNote_this] == false)
            {
                // std::cout << "  not the same cause couldn't find a note in the other measure that matches this one\n";
                return false;
            }
            
        }//next note
        
    } // next track reference
    
    if (total_note_amount == 0) return false; // don't count empty measures as repeitions
    return true;
}
    
// -------------------------------------------------------------------------------------------
    
int PrintLayoutMeasure::addTrackReference(const int firstNote, Track* track)
{
#if PLM_CHATTY
    std::cout << "PrintLayoutMeasure::addTrackReference '" << track->getName().mb_str()
              << "' in measure " << (m_measure_id + 1) << "\n";
#endif
    
    const int noteAmount = track->getNoteAmount();
    
    //MeasureTrackReference* newTrackRef = new MeasureTrackReference();
    //newTrackRef->track = track;
    
    // if firstNote is -1, it means all notes were processed. just add the track ref without searching for notes
    if (firstNote == -1)
    {
        // but first check if some note starts in a previous measure and ends in the current one :
        // the layout code will need to know the measure is not empty.
        for (int note=0; note<noteAmount; note++)
        {
            const int start_tick = track->getNoteStartInMidiTicks(note);
            const int end_tick   = track->getNoteEndInMidiTicks(note);
            
            if (start_tick < m_first_tick and end_tick > m_first_tick)
            {
                m_contains_something = true;
                break;
            }
        }

        m_track_refs.push_back( new MeasureTrackReference(track, -1, -1) );
#if PLM_CHATTY
        std::cout << "    --> Received input -1, assuming empty\n";
#endif
        return -1;
    }
    
    // FIXME: why is this necessary?
    int effectiveFirstNote = firstNote;
    if (effectiveFirstNote >= noteAmount) effectiveFirstNote = noteAmount-1;
    
    // find what the first, last and shortest note in current measure
    int lastNote = effectiveFirstNote;
    int last_note_end = -1, last_note_start = -1;
    
    bool measure_empty_in_this_track = true;
    
    // check if some note starts in a previous measure and ends in the current one
    for (int note=effectiveFirstNote-1; note>=0; note--)
    {
        const int startTick = track->getNoteStartInMidiTicks(note);
        const int endTick = track->getNoteEndInMidiTicks(note);

        if (startTick < m_first_tick and endTick > m_first_tick)
        {
            //const int startMeas = getMeasureData()->measureAtTick(track->getNoteStartInMidiTicks(note));
            //std::cout << "---> found a note that starts in measure " << startMeas << " but ends measure "
            //          << getMeasureData()->measureAtTick(end_tick) << "\n";
            measure_empty_in_this_track = false;
            break;
        }
    }
    
    // check for notes that start in this measure
    for (int note = effectiveFirstNote; note<noteAmount; note++)
    {
        const int start_tick          = track->getNoteStartInMidiTicks(note);
        const int end_tick            = track->getNoteEndInMidiTicks(note);
        const int currentNoteDuration = end_tick - start_tick;
        
        if (currentNoteDuration <= 0)  continue; // skip malformed notes if any
        
        // stop when we're at next measure
        if (start_tick >= m_last_tick) break;
                
        // find last note (if many notes end at the same time, keep the one that started last)
        if (start_tick > last_note_start || end_tick > last_note_start ||
            (end_tick == last_note_end && start_tick >= last_note_start))
        {
            lastNote        = note;
            last_note_end   = end_tick;
            last_note_start = start_tick;
            measure_empty_in_this_track   = false;
        }
        
        // store duration if it's the shortest yet (but ignore dead/instant-hit notes)
        const float relativeLength = (end_tick - start_tick) / (float)(getMeasureData()->beatLengthInTicks()*4);
        if (relativeLength < 1.0/32.0) continue;
        
        if (currentNoteDuration < m_shortest_duration or m_shortest_duration == -1)
        {
            m_shortest_duration = currentNoteDuration;
        }
    }
    ASSERT_E(lastNote,>,-1);
    ASSERT_E(lastNote,<,noteAmount);
    
    if (measure_empty_in_this_track)
    {
#if PLM_CHATTY
        std::cout << "    --> empty\n";
#endif
        m_track_refs.push_back( new MeasureTrackReference(track, -1, -1) );
    }
    else
    {
#if PLM_CHATTY
        std::cout << "    --> non-empty measure, m_shortest_duration = " << m_shortest_duration << "\n";
#endif
        m_track_refs.push_back( new MeasureTrackReference(track, effectiveFirstNote, lastNote) );
    }
    
    m_contains_something = m_contains_something or not measure_empty_in_this_track;
    
    // check if all notes were used (but check 'measure_empty' first, firstNote and lastNote can
    // exist but be out of the bounds of this measure
    if (not measure_empty_in_this_track and lastNote == noteAmount-1)
    {
#if PLM_CHATTY
        std::cout << "    --> returning -1 because all notes were covered\n";
#endif
        return -1;
    }
    
    // if this measure is empty, return the same note as the one given in input (i.e. it was not used)
    // if this measure is not empty, add 1 so next measure will start from the next
#if PLM_CHATTY
    std::cout << "    -> returning " << lastNote << " + " << (measure_empty_in_this_track ? 0 : 1) << "\n";
#endif
    return lastNote + (measure_empty_in_this_track ? 0 : 1);
}
    
// -------------------------------------------------------------------------------------------

bool PrintLayoutMeasure::findConsecutiveRepetition(ptr_vector<PrintLayoutMeasure>& measures, const int measureAmount,
                                                   int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                                   int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/)
{
    
    // check if it works with first measure occurence of similar measures
    if (m_measure_id+1<measureAmount and
        measures[m_measure_id+1].firstSimilarMeasure == measures[m_measure_id].firstSimilarMeasure+1 )
    {
        int amount = 0;
        
        for (int iter=1; iter<measureAmount; iter++)
        {
            if (m_measure_id+iter<measureAmount and
                measures[m_measure_id+iter].firstSimilarMeasure == measures[m_measure_id].firstSimilarMeasure+iter )
            {
                amount++;
            }
            else
            {
                break;
            }
        }//next
        
        firstMeasureThatRepeats = m_measure_id;
        lastMeasureThatRepeats  = m_measure_id + amount;
        firstMeasureRepeated    = measures[m_measure_id].firstSimilarMeasure;
        lastMeasureRepeated     = measures[m_measure_id].firstSimilarMeasure + amount;
        return true;
    }
    // check if it works with a later occurence of a similar measure
    else
    {
        const int first_measure =  measures[m_measure_id].firstSimilarMeasure;
        const int amount = measures[ first_measure ].similarMeasuresFoundLater.size();
        for(int laterOccurence=0; laterOccurence<amount; laterOccurence++)
        {
            const int checkFromMeasure = measures[ first_measure ].similarMeasuresFoundLater[laterOccurence];
            //std::cout << "        < lvl 2, testing measure " << checkFromMeasure << std::endl;
            //if (checkFromMeasure+1<id and measures[checkFromMeasure+1].firstSimilarMeasure ==
            //   measures[checkFromMeasure].firstSimilarMeasure+1 )
            //{
            int amount = 0;
            
            // check if there is a consecutive repetition with measures from this area
            
            for(int iter=0; iter</*id-checkFromMeasure*/measureAmount; iter++)
            {
                if (not(checkFromMeasure+iter < m_measure_id and
                        checkFromMeasure+iter < measureAmount and m_measure_id+iter < measureAmount)) continue;
                
                // check if they are identical
                
                if ( // they are identical because they both are repetitions of the same one
                    (measures[checkFromMeasure+iter].firstSimilarMeasure == measures[m_measure_id+iter].firstSimilarMeasure and
                     measures[checkFromMeasure+iter].firstSimilarMeasure != -1)
                    or
                    // they are identical because the second is a repetition of the first
                    (checkFromMeasure+iter == measures[m_measure_id+iter].firstSimilarMeasure)
                    )
                {
                    //std::cout << "            //" << (checkFromMeasure+iter+1) << " is same as " << (id+iter+1) << std::endl;
                    amount++;
                }
                else
                {
                    //std::cout << "            //but " << (checkFromMeasure+iter+1) << " is NOT same as " << (id+iter+1) << " (" << measures[checkFromMeasure+iter].firstSimilarMeasure+1 << " != " << measures[id+iter].firstSimilarMeasure+1 << ")" << std::endl;
                    break;
                }
            }//next
             //std::cout << "        > amount=" << amount << std::endl;
            
            if (amount < getRepetitionMinimalLength()) continue;
            //std::cout << "measure " << id+1  << " is a level 2 repetition" << std::endl;
            
            firstMeasureThatRepeats = m_measure_id;
            lastMeasureThatRepeats  = m_measure_id + amount-1;
            firstMeasureRepeated    = checkFromMeasure;
            lastMeasureRepeated     = checkFromMeasure + amount-1;
            return true;
            
            //}
        }//next
        
        // if we get there, it never works
        return false;
    }
}
    
