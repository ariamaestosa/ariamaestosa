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

#include "Editors/ScoreEditor.h"
#include "Editors/ScoreAnalyser.h"
#include "GUI/MeasureBar.h"
#include "main.h"
#include <cmath>
#include <math.h>

namespace AriaMaestosa
{
    
class BeamGroup
{
    int first_id, last_id;
    int min_level, mid_level, max_level;
    
public:
    BeamGroup(const int first_id, const int last_id)
    {
         BeamGroup::first_id = first_id;
         BeamGroup::last_id = last_id;
    }
    void calculateLevel(std::vector<NoteRenderInfo>& gatheredNoteInfo, ScoreMidiConverter* converter)
    {
        min_level = 999;
        max_level = -999;
        
        for(int i=first_id;i<=last_id; i++)
        {
            if(gatheredNoteInfo[i].chord)
            {
                if(gatheredNoteInfo[i].min_chord_level < min_level) min_level = gatheredNoteInfo[i].min_chord_level;
                if(gatheredNoteInfo[i].max_chord_level > max_level) max_level = gatheredNoteInfo[i].max_chord_level;
            }
            else
            {
                const int level = converter->noteToLevel(gatheredNoteInfo[i].pitch);   
                if(level < min_level) min_level = level;
                if(level > max_level) max_level = level;
            }
        }
        
        // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
        if(min_level == 999)  min_level = converter->noteToLevel(gatheredNoteInfo[first_id].pitch);
        if(max_level == -999) max_level = converter->noteToLevel(gatheredNoteInfo[first_id].pitch);
        
        mid_level = (int)round( (min_level + max_level)/2.0 );
    }
    
    void doBeam(std::vector<NoteRenderInfo>& gatheredNoteInfo, ScoreEditor* editor)
    {
        if(last_id == first_id) return; // note alone, no beaming to perform
        
        ScoreMidiConverter* converter = editor->getScoreMidiConverter();
        const int y_step = editor->getYStep();
        
        // check for number of "beamable" notes and split if current amount is not acceptable with the current time sig
        // FIXME - not always right
        const int num = getMeasureBar()->getTimeSigNumerator();
        const int denom = getMeasureBar()->getTimeSigDenominator();
        const int subtail_amount = gatheredNoteInfo[first_id].subtail_amount;
        
        int max_amount_of_notes_beamed_toghether = 4;
        
        if(num == 3 and denom == 4) max_amount_of_notes_beamed_toghether = 2 * (int)(std::pow(2.0,subtail_amount-1));
        else if((num == 6 and denom == 4) or (num == 6 and denom == 8)) max_amount_of_notes_beamed_toghether = 3 * (int)(std::pow(2.0,subtail_amount-1));
        else max_amount_of_notes_beamed_toghether = num * (int)(std::pow(2.0,subtail_amount-1));
        if(gatheredNoteInfo[first_id].triplet) max_amount_of_notes_beamed_toghether=3;
        
        const int beamable_note_amount = last_id - first_id + 1;
        
        if(beamable_note_amount > max_amount_of_notes_beamed_toghether)
        {
            // amount is not acceptable, split
            BeamGroup first_half(first_id, first_id + max_amount_of_notes_beamed_toghether - 1);
            BeamGroup second_half(first_id + max_amount_of_notes_beamed_toghether, last_id);
            
            first_half.doBeam(gatheredNoteInfo, editor);
            second_half.doBeam(gatheredNoteInfo, editor);
            return;
        }
        
        calculateLevel(gatheredNoteInfo, converter);

        gatheredNoteInfo[first_id].beam_show_above = (mid_level < converter->getMiddleCLevel()-5 ? false : true);
        gatheredNoteInfo[first_id].beam = true;
        
        for(int j=first_id; j<=last_id; j++)
        {
            // give correct tail type (up or down)
            gatheredNoteInfo[j].tail_type = ( gatheredNoteInfo[first_id].beam_show_above ?  TAIL_UP : TAIL_DOWN );
        }
        
        const int last_tail_y_end = gatheredNoteInfo[last_id].getTailYTo();
        gatheredNoteInfo[first_id].beam_to_x = gatheredNoteInfo[last_id].getTailX();
        
        if(gatheredNoteInfo[first_id].beam_show_above)
        {
            const int min_level_y = min_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels() - 2;
            
            // choose y coords so that all notes are correctly in
            gatheredNoteInfo[first_id].beam_to_y = std::min(min_level_y - 10, last_tail_y_end); // Y to
            if(gatheredNoteInfo[first_id].getTailYFrom() > min_level_y - 25) gatheredNoteInfo[first_id].tail_y = min_level_y - 25; // Y from
        }
        else
        {
            const int max_level_y = max_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels();
            
            // choose y coords so that all notes are correctly in
            gatheredNoteInfo[first_id].beam_to_y = std::max(max_level_y + 10, last_tail_y_end); // Y to
            if(gatheredNoteInfo[first_id].getTailYFrom() < max_level_y + 25) gatheredNoteInfo[first_id].tail_y = max_level_y + 25; // Y from
        }
        
        // fix all note tails so they point in the right direction and have the correct height
        const int from_x = gatheredNoteInfo[first_id].getTailX();
        const int from_y = gatheredNoteInfo[first_id].getTailYTo();
        const int to_x = gatheredNoteInfo[first_id].beam_to_x;
        const int to_y = gatheredNoteInfo[first_id].beam_to_y;
        
        for(int j=first_id; j<=last_id; j++)
        {
            // give correct tail height (so it doesn't end above or below beam line)
            // rel_pos will be 0 for first note of a beamed serie, and 1 for the last one
            const float rel_pos = (float)(gatheredNoteInfo[j].getTailX() - from_x) / (float)(to_x - from_x);
            gatheredNoteInfo[j].tail_y = from_y + (int)round( (to_y - from_y) * rel_pos );
            
            if(j != first_id) gatheredNoteInfo[j].subtail_amount = 0;
        }
    }
};

NoteRenderInfo::NoteRenderInfo(int tick, int x, int level, int tick_length, int sign, const bool selected, int pitch)
{
    // what we know before render pass 1
    NoteRenderInfo::selected = selected;
    NoteRenderInfo::tick = tick;
    NoteRenderInfo::tick_length = tick_length;
    NoteRenderInfo::x = x;
    NoteRenderInfo::sign = sign;
    NoteRenderInfo::level = level;
    NoteRenderInfo::pitch = pitch;
    
    // what we will know after render pass 1
    //unknown_duration = false;
    instant_hit=false;
    triplet = false;
    dotted = false;
    subtail_amount = 0;
    y = -1;
    tied_with_x = -1;
    tie_up = false;
    tail_type = TAIL_NONE;
    
    draw_tail = true;
    
    triplet_show_above = false;
    triplet_x1 = -1;
    triplet_x2 = -1;
    drag_triplet_sign = false;
    
    beam_show_above = false;
    beam_to_x = -1;
    beam_to_y = -1;
    beam = false;
    
    chord = false;
    max_chord_y = -1;
    min_chord_y = -1;
    tail_y = -1;
    min_chord_level=-1;
    max_chord_level=-1;
}
void NoteRenderInfo::tieWith(NoteRenderInfo& renderInfo)
{
    tied_with_x = renderInfo.x;
    if(tail_type == TAIL_NONE) tie_up = renderInfo.tail_type;
    else tie_up = tail_type;
}
void NoteRenderInfo::triplet_arc(int pixel1, int pixel2)
{
    triplet_x1 = pixel1;
    triplet_x2 = pixel2;
}
void NoteRenderInfo::setTriplet()
{
    triplet = true;
    drag_triplet_sign = true;
}

int NoteRenderInfo::getTailX()
{
    if(tail_type == TAIL_UP) return (x + 9);
    else if(tail_type == TAIL_DOWN) return (x + 1);
    else return -1;
}
int NoteRenderInfo::getTailYFrom()
{
    const int tail_y_base = getYBase();
    if(tail_type == TAIL_UP) return (tail_y_base + 3);
    else if(tail_type == TAIL_DOWN) return (tail_y_base + 6);
    else return -1;
}
int NoteRenderInfo::getTailYTo()
{
   if(tail_type == TAIL_UP) return (tail_y == -1 ?y - 24 : tail_y);
   else if(tail_type == TAIL_DOWN) return (tail_y == -1 ? y + 33 : tail_y);
   else return -1;
}
int NoteRenderInfo::getYBase()
{
    if(chord) return (tail_type == TAIL_UP ? max_chord_y : min_chord_y);
    else return y;
}

/*
 * This function takes a vector containing information about visible notes.
 * Its job is to analyse them and fill missing data in the contained objects
 * so that they can be rendered correctly on a score.
 */

void analyseNoteInfo( std::vector<NoteRenderInfo>& gatheredNoteInfo, ScoreEditor* editor )
{
    ScoreMidiConverter* converter = editor->getScoreMidiConverter();
    
    const int halfh = editor->getHalfNoteHeight();
    const int y_step = editor->getYStep();
    // analyse collected data
    
    // ------------------- order notes in time order -------------------
    {
        const int visibleNoteAmount = gatheredNoteInfo.size();
        for(int i=1; i<visibleNoteAmount; i++)
        {
            if(gatheredNoteInfo[i].tick < gatheredNoteInfo[i-1].tick)
            {
                NoteRenderInfo tmp = gatheredNoteInfo[i-1];
                gatheredNoteInfo[i-1] = gatheredNoteInfo[i];
                gatheredNoteInfo[i] = tmp;
                i -= 2; if(i<0) i=0;
            }
        }
    }
    
    // FIXME - messy...
    
    /*
     * start by merging notes playing at the same time (chords)
     * The for loop iterates through all notes. When we find notes that play at the same time,
     * the while loop starts and iterates until we reached the end of the chord.
     * when we're done with a chord, we "summarize" it in a single NoteRenderInfo object.
     * (at this point, the "main round" of the note has been drawn and what's left to do
     * is draw tails, triplet signs, etc. so at this point a chord of note behaves just
     * like a single note).
     */
    for(int i=0; i<(int)gatheredNoteInfo.size(); i++)
    {
        int start_tick_of_next_note = -1;
        int first_note_of_chord = -1;
        int min_level = 999, max_level = -999;
        int maxid = i, minid = i; // id of the notes with highest and lowest Y
        int smallest_duration = 999;
        bool last_of_a_serie = false;
        bool triplet = false;
        int subtail_amount = 0;
        
        // when we have found a note chord, this variable will contain the ID of the
        // first note of that bunch (the ID of the last will be "i" when we get to it)
        first_note_of_chord = i;
        
        while(true) // FIXME - it should be checked whether there is a chord BEFORE entering the while loop. same for others below
        {
            if(i+1<(int)gatheredNoteInfo.size())
            {
                start_tick_of_next_note = gatheredNoteInfo[i+1].tick;
            }else start_tick_of_next_note=-1;
            
            if(!(i<(int)gatheredNoteInfo.size())) break;
            
            // check if we're in a chord (i.e. many notes that play at the same time). also check they have tails :
            // for instance wholes have no tails and thus there is no special processing to do on them.
            if(start_tick_of_next_note != -1 and aboutEqual_tick(start_tick_of_next_note, gatheredNoteInfo[i].tick) and gatheredNoteInfo[i].tail_type != TAIL_NONE);
            else
            {
                //after this one, it stops. mark this as the last so it will finalize stuff.
                last_of_a_serie = true;
                if(first_note_of_chord == i){ break; } //not a bunch of concurrent notes, just a note alone
            }
            
            // gather info on notes of the chord, for instance their y location (level) abd their duration
            const int level = converter->noteToLevel(gatheredNoteInfo[i].pitch);
            if(level < min_level){ min_level = level; minid = i; }
            if(level > max_level){ max_level = level; maxid = i; }
            
            const int len = gatheredNoteInfo[i].tick_length;
            if(len < smallest_duration)
            {
                smallest_duration = len;
            }
            
            if(gatheredNoteInfo[i].subtail_amount > subtail_amount)
            {
                subtail_amount = gatheredNoteInfo[i].subtail_amount;
            }
            
            if(gatheredNoteInfo[i].triplet) triplet = true;
            
            // remove this note's tail. we only need on tail per chord.
            // the right tail will be set on last iteration of the chord (see below)
            gatheredNoteInfo[i].draw_tail = false;
            
            // the note of this iteration is the end of a chord, so it's time to complete chord information
            if(last_of_a_serie)
            {
                if(maxid == minid) break;
                if(first_note_of_chord == i) break;
                
                // determine average note level to know if we put tails above or below
                // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
                if(min_level == 999)  min_level = converter->noteToLevel(gatheredNoteInfo[first_note_of_chord].pitch);
                if(max_level == -999) max_level = converter->noteToLevel(gatheredNoteInfo[first_note_of_chord].pitch);
                const int mid_level = (int)round( (min_level + max_level)/2.0 );
                
                const bool tailup = mid_level>=converter->getMiddleCLevel()-3;
                
                const int maxy = editor->getEditorYStart() + y_step*max_level - halfh - editor->getYScrollInPixels() + 2;
                const int miny = editor->getEditorYStart() + y_step*min_level - halfh - editor->getYScrollInPixels() + 2;
                
                // decide the one note to keep that will "summarize" all others.
                // it will be the highest or the lowest, depending on if tail is up or down.
                // feed this NoteRenderInfo object with the info Aria believes will best summarize
                // the chord. results may vary if you make very different notes play at the same time.
                NoteRenderInfo summary = gatheredNoteInfo[ tailup ? minid : maxid ];
                summary.chord = true;
                summary.max_chord_y = maxy;
                summary.min_chord_y = miny;
                summary.min_chord_level = min_level;
                summary.max_chord_level = max_level;
                summary.subtail_amount = subtail_amount;
                summary.triplet = triplet;
                summary.draw_tail = true;
                summary.tail_type = (tailup ? TAIL_UP : TAIL_DOWN);
                
                gatheredNoteInfo[i] = summary;
                
                // now that we summarised concurrent notes into a single one, we can erase the other notes of the chord
                assertExpr(i,<,(int)gatheredNoteInfo.size());
                gatheredNoteInfo.erase( gatheredNoteInfo.begin()+first_note_of_chord, gatheredNoteInfo.begin()+i );
                i = first_note_of_chord-2;
                if(i<0) i=0;
                
                break;
            }//end if
            i++;
        }//wend
        
    }//next
    
    /*
     *  Now that chords have been "summarized" into a single one, we can check for operations
     *  that affect series of notes, for instance triplet signs and note beaming.
     */
    
    // triplets
    const int visibleNoteAmount = gatheredNoteInfo.size();
    for(int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;
        
        bool is_triplet = gatheredNoteInfo[i].triplet;
        int first_triplet = (is_triplet ? i : -1);
        int min_level = 999;
        int max_level = -999;
        bool last_of_a_serie = false;
        
        int measure = getMeasureBar()->measureAtTick( gatheredNoteInfo[i].tick );
        int previous_measure = measure;
        
        // check for consecutive notes
        while(true)
        {
            if(i+1<visibleNoteAmount)
            {
                start_tick_of_next_note = gatheredNoteInfo[i+1].tick;
            }
            
            if(!(i<visibleNoteAmount)) break;
            
            // if notes are consecutive
            if(start_tick_of_next_note != -1 and aboutEqual_tick(start_tick_of_next_note, gatheredNoteInfo[i].tick+gatheredNoteInfo[i].tick_length));
            else
            {
                //notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
            }
            if(!gatheredNoteInfo[i+1].triplet or i-first_triplet>=2) last_of_a_serie = true;
            
            // do not cross measures
            if(i+1<visibleNoteAmount)
            {
                measure = getMeasureBar()->measureAtTick( gatheredNoteInfo[i+1].tick );
                if(measure != previous_measure) last_of_a_serie = true;
                previous_measure = measure;
            }
            
            if(gatheredNoteInfo[i].chord)
            {
                if(gatheredNoteInfo[i].min_chord_level < min_level) min_level = gatheredNoteInfo[i].min_chord_level;
                if(gatheredNoteInfo[i].max_chord_level > max_level) max_level = gatheredNoteInfo[i].max_chord_level;
            }
            else
            {
                const int level = converter->noteToLevel(gatheredNoteInfo[i].pitch);   
                if(level < min_level) min_level = level;
                if(level > max_level) max_level = level;
            }
            
            // --------- triplet ---------
            is_triplet = gatheredNoteInfo[i].triplet;
            if(is_triplet and first_triplet==-1) first_triplet = i;
            
            // this note is a triplet, but not the next, so time to do display the triplets sign
            // also triggered if we've had 3 triplet notes in a row, because triplets come by groups of 3...
            if( last_of_a_serie )
            {
                if(is_triplet)
                {
                    // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
                    if(min_level == 999)  min_level = converter->noteToLevel(gatheredNoteInfo[first_triplet].pitch);
                    if(max_level == -999) max_level = converter->noteToLevel(gatheredNoteInfo[first_triplet].pitch);
                    
                    int mid_level = (int)round( (min_level + max_level)/2.0 );
                        
                    gatheredNoteInfo[first_triplet].triplet_show_above = (mid_level < converter->getMiddleCLevel()-5);
                    
                    if(i != first_triplet) // if not a triplet note alone, but a 'chain' of triplets
                    {
                        // fix all note tails so they point in the right direction
                        for(int j=first_triplet; j<=i; j++)
                        {
                            gatheredNoteInfo[j].tail_type = ( gatheredNoteInfo[first_triplet].triplet_show_above ? TAIL_DOWN : TAIL_UP );
                            gatheredNoteInfo[j].drag_triplet_sign = false;
                        }
                    }
                    else
                    {
                        // this is either a triplet alone or a chord... just use the orientation that it already has
                        gatheredNoteInfo[first_triplet].triplet_show_above = (gatheredNoteInfo[first_triplet].tail_type == TAIL_DOWN);
                    }
                    
                    if(gatheredNoteInfo[first_triplet].triplet_show_above)
                    {
                        gatheredNoteInfo[first_triplet].triplet_y = min_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels() - 2;
                    }
                    else
                    {
                        gatheredNoteInfo[first_triplet].triplet_y = max_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels();
                    }
                    
                    gatheredNoteInfo[first_triplet].drag_triplet_sign = true;
                   // gatheredNoteInfo[first_triplet].triplet = true;
                    gatheredNoteInfo[first_triplet].triplet_x2 = gatheredNoteInfo[i].x + 8;
                }
                
                // reset search for triplets
                first_triplet = -1;
                min_level = 999;
                max_level = -999;
            }
            
            if(last_of_a_serie) break;
            else i++;
            
        }//wend
        
    }//next
    
    // beaming
    // all beam information is stored in the first note of the serie.
    // all others have their subtails removed
    for(int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;
        
        int subtail_amount = gatheredNoteInfo[i].subtail_amount;
        int first_of_serie = i;
        bool last_of_a_serie = false;
        
        int measure = getMeasureBar()->measureAtTick( gatheredNoteInfo[i].tick );
        int previous_measure = measure;
        
        // check for consecutive notes
        while(true)
        {
            if(i+1<visibleNoteAmount)
            {
                start_tick_of_next_note = gatheredNoteInfo[i+1].tick;
            }
            
            if(!(i<visibleNoteAmount)) break;
            
            // if notes are consecutive and of same length
            if(start_tick_of_next_note != -1 and
               aboutEqual_tick(start_tick_of_next_note, gatheredNoteInfo[i].tick+gatheredNoteInfo[i].tick_length) and
               gatheredNoteInfo[i+1].subtail_amount == subtail_amount and subtail_amount > 0 and
               gatheredNoteInfo[i+1].triplet == gatheredNoteInfo[i].triplet);
            else
            {
                //notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
            }
            
            // do not cross measures
            if(i+1<visibleNoteAmount)
            {
                measure = getMeasureBar()->measureAtTick( gatheredNoteInfo[i+1].tick );
                if(measure != previous_measure) last_of_a_serie = true;
                previous_measure = measure;
            }
            
            // it's the last of a serie, perform actions
            if( last_of_a_serie)
            {
                if(i>first_of_serie)
                {
                    BeamGroup beam(first_of_serie, i);
                    beam.doBeam(gatheredNoteInfo, editor);
                }
                
                // reset
                first_of_serie = -1;
                break;
            }
            
            i++;
            
        }//wend
        
    }//next
    
}// end analyseNotes function


bool aboutEqual(const float float1, const float float2)
{
	float diff = float1 - float2;
	if(diff < 0) diff = -diff;
	if(diff < 1.0/64.0) return true;
	else return false;
}
bool aboutEqual_tick(const int int1, const int int2)
{
	return std::abs(int1 - int2) < getMeasureBar()->beatLengthInTicks()/16;
}


}
