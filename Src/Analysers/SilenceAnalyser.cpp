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

#include "Analysers/SilenceAnalyser.h"

#include "AriaCore.h"

#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"

#include <algorithm>

using namespace AriaMaestosa;
using namespace AriaMaestosa::SilenceAnalyser;

#ifdef _MORE_DEBUG_CHECKS
int recursionDepth = 0;
class IncRecDepth
{
public:
    IncRecDepth()  { recursionDepth++; }
    ~IncRecDepth() { recursionDepth--; }
};
#endif


/**
  * @internal
  */
void recursivelyAnalyzeSilence(const Sequence* seq, RenderSilenceCallback renderSilenceCallback,
                               const int tick, const int tick_length, const int silences_y, void* userdata)
{
#ifdef _MORE_DEBUG_CHECKS
    ASSERT(recursionDepth < 150)
    IncRecDepth scopeObject;
#endif
    
    if (tick_length < 2) return;
    
    const MeasureData* md = seq->getMeasureData();
    const int beatLen = seq->ticksPerQuarterNote();
    
    const int measure     = md->measureAtTick(tick);
    const int end_measure = md->measureAtTick(tick + tick_length - 1);
    
    if (tick_length < 2) return;
    
    // check if silence spawns over more than one measure
    if (measure != end_measure)
    {
        // we need to plit it in two
        const int split_tick = md->firstTickInMeasure(end_measure);
        
        // Check split is valid before attempting.
        if (split_tick - tick > 0 and tick_length - (split_tick - tick) > 0)
        {
            recursivelyAnalyzeSilence(seq, renderSilenceCallback, tick, split_tick - tick, silences_y, userdata);
            recursivelyAnalyzeSilence(seq, renderSilenceCallback, split_tick,
                                      tick_length - (split_tick - tick), silences_y, userdata);
            return;
        }
    }
    
    if (tick < 0) return; // FIXME - find why it happens
                          // ASSERT_E(tick,>,-1);
    
    bool dotted = false, triplet = false;
    int type = -1;
    
    int dot_delta_x = 0, dot_delta_y = 0;
    
    const float relativeLength = tick_length / (float)(beatLen*4);
    
    const int  tick_from_measure_start = tick - md->firstTickInMeasure( md->measureAtTick(tick) );
    
    /** How many ticks remain before the first beat this note plays on */
    const int  remaining      = beatLen - (tick_from_measure_start % beatLen);
    const bool starts_on_beat = aboutEqual(remaining, 0) or aboutEqual(remaining, beatLen);
    
    int measureId = md->measureAtTick(tick);
    bool isFour = md->getTimeSigDenominator(measureId) == 4;
    
    if (aboutEqual(relativeLength, 1.0))
    {
        type = 1;
    }
    else if (aboutEqual(relativeLength, 3.0/2.0) and starts_on_beat)
    {
        type = 1; dotted = true; dot_delta_x = 5; dot_delta_y = 2;
    }
    else if (aboutEqual(relativeLength, 1.0/2.0) and
             // TODO: a similar check (to prevent a silence in the middle of a measure)
             //       should be added for other time signatures than */4
             (not isFour or (tick_from_measure_start % (beatLen*2)) < beatLen/10))
    {
        type = 2;
    }
    else if (aboutEqual(relativeLength, 3.0/4.0) and starts_on_beat)
    {
        type = 2; dotted = true; dot_delta_x = 5; dot_delta_y = 2;
    }
    else if (aboutEqual(relativeLength, 1.0/4.0))
    {
        type = 4;
    }
    else if (aboutEqual(relativeLength, 1.0/3.0))
    {
        type = 2; triplet = true;
    }
    else if (aboutEqual(relativeLength, 3.0/8.0) and starts_on_beat)
    {
        type = 4; dotted = true; dot_delta_x = -3; dot_delta_y = 10;
    }
    else if (relativeLength > 0.159f and relativeLength < 0.175f) // triplet 1/4 (1/6)
    {
        type = 4; triplet = true;
    }
    else if (relativeLength > 0.110f and relativeLength < 0.140f) // 1/8
    {
        type = 8;
    }
    else if (relativeLength > 0.076f and relativeLength < 0.090f) // triplet 1/8 (1/12)
    {
        triplet = true; type = 8;
    }
    else if (relativeLength > 0.055f and relativeLength < 0.070f) // 1/16
    {
        type = 16;
    }
    else if (relativeLength > 0.040f and relativeLength < 0.044f) // 1/16 (1/24)
    {
        type = 16; triplet = true;
    }
    else if (relativeLength > 0.026f and relativeLength < 0.034f) // 1/32
    {
        type = 32;
    }
    else if (aboutEqual(relativeLength, 3.0/16.0) and starts_on_beat)
    {
        type = 8; dotted = true;
    }
    else if (relativeLength < 1.0/16.0)
    {
        return;
    }
    else
    {
        // This silence is of unknown duration. split it in a series of silences.
        
        // start by reaching the next beat if not already done
        if (not starts_on_beat and not aboutEqual(remaining, tick_length) and remaining <= tick_length)
        {
            ASSERT_E(remaining, <=, tick_length);
            recursivelyAnalyzeSilence(seq, renderSilenceCallback, tick, remaining, silences_y, userdata);
            recursivelyAnalyzeSilence(seq, renderSilenceCallback, tick+remaining, tick_length - remaining, silences_y, userdata);
            return;
        }
        
        // split in two smaller halves. render using a simple recursion.
        float closestShorterDuration = 1;
        while (closestShorterDuration >= relativeLength) closestShorterDuration /= 2.0;
        
        const int firstLength = closestShorterDuration*(float)(beatLen*4);
        
        recursivelyAnalyzeSilence(seq, renderSilenceCallback, tick, firstLength, silences_y, userdata);
        recursivelyAnalyzeSilence(seq, renderSilenceCallback, tick + firstLength, tick_length - firstLength, silences_y, userdata);
        return;
    }
    
    renderSilenceCallback(seq, tick_length, tick, type, silences_y, triplet, dotted, dot_delta_x, dot_delta_y, userdata);
}
        
// ----------------------------------------------------------------------------------------------------------

void AriaMaestosa::SilenceAnalyser::findSilences(const Sequence* seq, RenderSilenceCallback renderSilenceCallback,
                                                 INoteSource* noteSource, const int first_visible_measure,
                                                 const int last_visible_measure, const int silences_y, void* userdata)
{
    const int visible_measure_amount = last_visible_measure-first_visible_measure+1;
    bool measure_empty[visible_measure_amount+1];
    for (int i=0; i<=visible_measure_amount; i++) measure_empty[i] = true;
    
    const MeasureData* md = seq->getMeasureData();
    
    int previous_tick = -1;
    
    const int visibleNoteAmount = noteSource->getNoteCount();
    
    if (visibleNoteAmount>0)
    {
        // by comparing the ending of the previous note to the beginning of the current note,
        // we can know if there is a silence. If multiple notes play at the same time,
        // 'previous_note_end' will contain the end position of the longest note.
        // At this point all notes will already have been split so that they do not overlap on
        // 2 measures so we don't need to care about that.
        int previous_note_end = -1;
        
        // last_note_end is similar to previous_note_end, but contains the end tick of the last note that ended
        // while previous_note_end contains the end tick of the last note that started
        int last_note_end = -1;
        
        int last_measure = -1;
        
        for (int i=0; i<visibleNoteAmount; i++)
        {       
            const int measure = noteSource->getBeginMeasure(i);
            ASSERT_E(measure,>=,0);                    
            ASSERT_E(last_measure,>=,-1);
            
            const int startTick = noteSource->getStartTick(i);
            ASSERT_E(startTick, >=, previous_tick);
            
            previous_tick = startTick;
            
            // we switched to another measure
            if (measure > last_measure)
            {
                // if the last note of previous measure does not finish at the end of the measure,
                // we need to add a silence at the end of it
                if (last_measure != -1 and
                    not aboutEqual(last_note_end, md->firstTickInMeasure(measure) ))
                {
                    const int last_measure_id = md->measureAtTick(last_note_end-1);
                    const int silence_length  = md->lastTickInMeasure(last_measure_id) - 
                                                last_note_end;
                    if (silence_length < 0) continue;
                    
                    recursivelyAnalyzeSilence(seq, renderSilenceCallback, last_note_end,
                                              silence_length, silences_y, userdata);
                    
                }
                
                // if note is not at the very beginning of the new measure, and it's the first note of
                // the measure, we need to add a silence before it
                if (not aboutEqual(startTick, md->firstTickInMeasure(measure) ))
                {
                    const int silence_length = startTick -
                                               md->firstTickInMeasure(measure);
                    
                    recursivelyAnalyzeSilence(seq, renderSilenceCallback,
                                              md->firstTickInMeasure(measure),
                                              silence_length, silences_y, userdata);
                }
                
                if (last_measure != -1)
                {
                    previous_note_end = -1; // we switched to another measure, reset and start again
                    last_note_end     = -1;
                }
            }
            
            last_measure = measure;
            
            // remember that this measure was not empty (only if it fits somewhere in the 'measure_empty' array)
            if ((int)(measure-first_visible_measure) >= 0 and
                (int)(measure-first_visible_measure) < (int)(visible_measure_amount+1))
            {
                if ((int)(measure-first_visible_measure) >= (int)visible_measure_amount)
                {
                    break; // we're too far
                }
                measure_empty[measure-first_visible_measure] = false;
            }
            
            // silences between two notes
            const int current_begin_tick = noteSource->getStartTick(i);
            if ( previous_note_end != -1 and !aboutEqual(previous_note_end, current_begin_tick) and
                (current_begin_tick-previous_note_end)>0 /*and previous_note_end >= last_note_end*/)
            {
                recursivelyAnalyzeSilence(seq, renderSilenceCallback, previous_note_end, current_begin_tick-previous_note_end,
                                          silences_y, userdata);
            }
            
            previous_note_end = noteSource->getEndTick(i);
            
            // if there's multiple notes playing at the same time
            while (i+1<visibleNoteAmount and noteSource->getStartTick(i) == noteSource->getStartTick(i+1))
            {
                i++;
                previous_note_end = std::max(previous_note_end, noteSource->getEndTick(i));
            }
            
            if (previous_note_end > last_note_end) last_note_end = previous_note_end;
        }//next visible note
        
        // check for silence after last note
        const int lastNoteMeasure           = md->measureAtTick(noteSource->getStartTick(visibleNoteAmount-1));
        const unsigned int last_measure_end = md->lastTickInMeasure(lastNoteMeasure);
        
        if (not aboutEqual(last_note_end, last_measure_end ) and last_note_end > -1)
        {
            const int silence_length = last_measure_end-last_note_end;
            recursivelyAnalyzeSilence(seq, renderSilenceCallback, last_note_end, silence_length, silences_y, userdata);
        }
        
        
    }// end if there are visible notes
    
    // silences in empty measures
    for (int i=0; i<visible_measure_amount; i++)
    {
        if (measure_empty[i])
        {
            recursivelyAnalyzeSilence(seq, renderSilenceCallback,
                                      md->firstTickInMeasure(first_visible_measure+i),
                                      md->measureLengthInTicks(first_visible_measure+i), silences_y, userdata);
        }
    }
} // end function
        
// ----------------------------------------------------------------------------------------------------------

std::vector<SilenceInfo> g_silences_ticks;

/**
  * @internal
  */
void gatherSilenceCallback(const Sequence* seq, const int duration, const int tick, const int type,
                           const int silences_y, const bool triplet,  const bool dotted,
                           const int dot_delta_x, const int dot_delta_y, void* userdata)
{
    g_silences_ticks.push_back( SilenceInfo(tick, tick + duration, type, silences_y,
                                            triplet, dotted) );
}

// ----------------------------------------------------------------------------------------------------------

std::vector<SilenceInfo> AriaMaestosa::SilenceAnalyser::findSilences(const Sequence* seq, INoteSource* noteSource,
                                                                     const int first_visible_measure, const int last_visible_measure,
                                                                     const int silences_y)
{
    g_silences_ticks.clear();
    findSilences(seq, &gatherSilenceCallback, noteSource, first_visible_measure,
                 last_visible_measure, silences_y, NULL);
    return g_silences_ticks;
}


