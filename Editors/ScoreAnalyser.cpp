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
#include "AriaCore.h"
#include <cmath>
#include <math.h>

namespace AriaMaestosa
{

/*
 * This class receives a range of IDs of notes that are candidates for beaming. Its job is to decide
 * how to beam the notes in order to get maximal results, as well as changing the NoteRenderInfo objects
 * accordingly so that the render is correct.
 */
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
    void calculateLevel(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreMidiConverter* converter)
    {
        min_level = 999;
        max_level = -999;
        
        for(int i=first_id;i<=last_id; i++)
        {
            if(noteRenderInfo[i].chord)
            {
                if(noteRenderInfo[i].min_chord_level < min_level) min_level = noteRenderInfo[i].min_chord_level;
                if(noteRenderInfo[i].max_chord_level > max_level) max_level = noteRenderInfo[i].max_chord_level;
            }
            else
            {
                const int level = noteRenderInfo[i].level;   
                if(level < min_level) min_level = level;
                if(level > max_level) max_level = level;
            }
        }
        
        // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
        if(min_level == 999)  min_level = noteRenderInfo[first_id].level;
        if(max_level == -999) max_level = noteRenderInfo[first_id].level;
        
        mid_level = (int)round( (min_level + max_level)/2.0 );
    }
    
    void doBeam(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
    {
        if(last_id == first_id) return; // note alone, no beaming to perform
        
        ScoreMidiConverter* converter = editor->getScoreMidiConverter();
        const int y_step = editor->getYStep();
        
        // check for number of "beamable" notes and split if current amount is not acceptable with the current time sig
        // FIXME - not always right
        const int num = getMeasureBar()->getTimeSigNumerator();
        const int denom = getMeasureBar()->getTimeSigDenominator();
        const int subtail_amount = noteRenderInfo[first_id].subtail_amount;
        
        int max_amount_of_notes_beamed_toghether = 4;
        
        if(num == 3 and denom == 4) max_amount_of_notes_beamed_toghether = 2 * (int)(std::pow(2.0,subtail_amount-1));
        else if((num == 6 and denom == 4) or (num == 6 and denom == 8)) max_amount_of_notes_beamed_toghether = 3 * (int)(std::pow(2.0,subtail_amount-1));
        else max_amount_of_notes_beamed_toghether = num * (int)(std::pow(2.0,subtail_amount-1));
        if(noteRenderInfo[first_id].triplet) max_amount_of_notes_beamed_toghether=3;
        
        const int beamable_note_amount = last_id - first_id + 1;
                
        // if max_amount_of_notes_beamed_toghether is an even number, don't accept an odd number of grouped notes
        const int base_unit = (max_amount_of_notes_beamed_toghether % 2 == 0 ? 2 : 1);
        if(beamable_note_amount <= max_amount_of_notes_beamed_toghether and beamable_note_amount % base_unit != 0)
        {
            max_amount_of_notes_beamed_toghether = base_unit;
        }
        
        if(beamable_note_amount > max_amount_of_notes_beamed_toghether)
        {
            // amount is not acceptable, split
            
            // try to find where beamed groups of such notes usually start and end in the measure
            // this is where splitting should be performed
            const int group_len = noteRenderInfo[first_id].tick_length * max_amount_of_notes_beamed_toghether;
            // FIXME - i don't understans that measureAtTick call, it should use a tick, not a length??
            const int first_tick_in_measure = getMeasureBar()->firstTickInMeasure( getMeasureBar()->measureAtTick(noteRenderInfo[first_id].tick_length) );
            
            int split_at_id = -1;
            for(int n=first_id+1; n<=last_id; n++)
            {
                if( (noteRenderInfo[n].tick - first_tick_in_measure) % group_len == 0 )
                {
                    split_at_id = n;
                    break;
                }
            }
            
            if(split_at_id == -1)
            {
                // dumb split
                BeamGroup first_half(first_id, first_id + max_amount_of_notes_beamed_toghether - 1);
                BeamGroup second_half(first_id + max_amount_of_notes_beamed_toghether, last_id);
                first_half.doBeam(noteRenderInfo, editor);
                second_half.doBeam(noteRenderInfo, editor);
            }
            else
            {
                BeamGroup first_half(first_id, split_at_id - 1);
                BeamGroup second_half(split_at_id, last_id);
                first_half.doBeam(noteRenderInfo, editor);
                second_half.doBeam(noteRenderInfo, editor);
            }
            
            return;
        }
        
        calculateLevel(noteRenderInfo, converter);

        noteRenderInfo[first_id].beam_show_above = (mid_level < converter->getMiddleCLevel()-5 ? false : true);
        noteRenderInfo[first_id].beam = true;
        
        for(int j=first_id; j<=last_id; j++)
        {
            // give correct tail type (up or down)
            noteRenderInfo[j].tail_type = ( noteRenderInfo[first_id].beam_show_above ?  TAIL_UP : TAIL_DOWN );
        }
        
        const int last_tail_y_end = noteRenderInfo[last_id].getTailYTo();
        noteRenderInfo[first_id].beam_to_x = noteRenderInfo[last_id].getTailX();
        
        if(noteRenderInfo[first_id].beam_show_above)
        {
            const int min_level_y = min_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels() - 2;
            
            // choose y coords so that all notes are correctly in
            noteRenderInfo[first_id].beam_to_y = std::min(min_level_y - 10, last_tail_y_end); // Y to
            if(noteRenderInfo[first_id].getTailYFrom() > min_level_y - 25) noteRenderInfo[first_id].tail_y = min_level_y - 25; // Y from
        }
        else
        {
            const int max_level_y = max_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels();
            
            // choose y coords so that all notes are correctly in
            noteRenderInfo[first_id].beam_to_y = std::max(max_level_y + 10, last_tail_y_end); // Y to
            if(noteRenderInfo[first_id].getTailYFrom() < max_level_y + 25) noteRenderInfo[first_id].tail_y = max_level_y + 25; // Y from
        }
        
        // fix all note tails so they point in the right direction and have the correct height
        const int from_x = noteRenderInfo[first_id].getTailX();
        const int from_y = noteRenderInfo[first_id].getTailYTo();
        const int to_x = noteRenderInfo[first_id].beam_to_x;
        const int to_y = noteRenderInfo[first_id].beam_to_y;
        
        for(int j=first_id; j<=last_id; j++)
        {
            // give correct tail height (so it doesn't end above or below beam line)
            // rel_pos will be 0 for first note of a beamed serie, and 1 for the last one
            const float rel_pos = (float)(noteRenderInfo[j].getTailX() - from_x) / (float)(to_x - from_x);
            noteRenderInfo[j].tail_y = from_y + (int)round( (to_y - from_y) * rel_pos );
            
            if(j != first_id) noteRenderInfo[j].subtail_amount = 0;
        }
    }
};

#pragma mark -

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
    triplet_arc_x_start = -1;
    triplet_arc_x_end = -1;
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
    
    // measure where the note begins and ends
    measureBegin = getMeasureBar()->measureAtTick(tick);
	measureEnd = getMeasureBar()->measureAtTick(tick + tick_length - 1);
}
void NoteRenderInfo::tieWith(NoteRenderInfo& renderInfo)
{
    tied_with_x = renderInfo.x;
    if(tail_type == TAIL_NONE) tie_up = renderInfo.tail_type;
    else tie_up = tail_type;
}
void NoteRenderInfo::triplet_arc(int pixel1, int pixel2)
{
    triplet_arc_x_start = pixel1;
    triplet_arc_x_end = pixel2;
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

#pragma mark -

void putInTimeOrder(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
{
    const int visibleNoteAmount = noteRenderInfo.size();
    for(int i=1; i<visibleNoteAmount; i++)
    {
        if(noteRenderInfo[i].tick < noteRenderInfo[i-1].tick)
        {
            NoteRenderInfo tmp = noteRenderInfo[i-1];
            noteRenderInfo[i-1] = noteRenderInfo[i];
            noteRenderInfo[i] = tmp;
            i -= 2; if(i<0) i=0;
        }
    }
}

void findAndMergeChords(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
{
    const int halfh = editor->getHalfNoteHeight();
    const int y_step = editor->getYStep();
    ScoreMidiConverter* converter = editor->getScoreMidiConverter();
    
    /*
     * start by merging notes playing at the same time (chords)
     * The for loop iterates through all notes. When we find notes that play at the same time,
     * the while loop starts and iterates until we reached the end of the chord.
     * when we're done with a chord, we "summarize" it in a single NoteRenderInfo object.
     * (at this point, the "main round" of the note has been drawn and what's left to do
        * is draw tails, triplet signs, etc. so at this point a chord of note behaves just
        * like a single note).
     */
    for(int i=0; i<(int)noteRenderInfo.size(); i++)
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
            if(i+1<(int)noteRenderInfo.size())
            {
                start_tick_of_next_note = noteRenderInfo[i+1].tick;
            }else start_tick_of_next_note=-1;
            
            if(!(i<(int)noteRenderInfo.size())) break;
            
            // check if we're in a chord (i.e. many notes that play at the same time). also check they have tails :
            // for instance wholes have no tails and thus there is no special processing to do on them.
            if(start_tick_of_next_note != -1 and aboutEqual_tick(start_tick_of_next_note, noteRenderInfo[i].tick) and noteRenderInfo[i].tail_type != TAIL_NONE);
            else
            {
                //after this one, it stops. mark this as the last so it will finalize stuff.
                last_of_a_serie = true;
                if(first_note_of_chord == i){ break; } //not a bunch of concurrent notes, just a note alone
            }
            
            // gather info on notes of the chord, for instance their y location (level) and their duration
            const int level = noteRenderInfo[i].level;
            if(level < min_level){ min_level = level; minid = i; }
            if(level > max_level){ max_level = level; maxid = i; }
            
            const int len = noteRenderInfo[i].tick_length;
            if(len < smallest_duration)
            {
                smallest_duration = len;
            }
            
            if(noteRenderInfo[i].subtail_amount > subtail_amount)
            {
                subtail_amount = noteRenderInfo[i].subtail_amount;
            }
            
            if(noteRenderInfo[i].triplet) triplet = true;
            
            // remove this note's tail. we only need on tail per chord.
            // the right tail will be set on last iteration of the chord (see below)
            noteRenderInfo[i].draw_tail = false;
            
            // the note of this iteration is the end of a chord, so it's time to complete chord information
            if(last_of_a_serie)
            {
                if(maxid == minid) break;
                if(first_note_of_chord == i) break;
                
                // determine average note level to know if we put tails above or below
                // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
                if(min_level == 999)  min_level = noteRenderInfo[first_note_of_chord].level;
                if(max_level == -999) max_level = noteRenderInfo[first_note_of_chord].level;
                const int mid_level = (int)round( (min_level + max_level)/2.0 );
                
                const bool tailup = mid_level>=converter->getMiddleCLevel()-3;
                
                const int maxy = editor->getEditorYStart() + y_step*max_level - halfh - editor->getYScrollInPixels() + 2;
                const int miny = editor->getEditorYStart() + y_step*min_level - halfh - editor->getYScrollInPixels() + 2;
                
                // decide the one note to keep that will "summarize" all others.
                // it will be the highest or the lowest, depending on if tail is up or down.
                // feed this NoteRenderInfo object with the info Aria believes will best summarize
                // the chord. results may vary if you make very different notes play at the same time.
                NoteRenderInfo summary = noteRenderInfo[ tailup ? minid : maxid ];
                summary.chord = true;
                summary.max_chord_y = maxy;
                summary.min_chord_y = miny;
                summary.min_chord_level = min_level;
                summary.max_chord_level = max_level;
                summary.subtail_amount = subtail_amount;
                summary.triplet = triplet;
                summary.draw_tail = true;
                summary.tail_type = (tailup ? TAIL_UP : TAIL_DOWN);
                
                noteRenderInfo[i] = summary;
                
                // now that we summarised concurrent notes into a single one, we can erase the other notes of the chord
                assertExpr(i,<,(int)noteRenderInfo.size());
                noteRenderInfo.erase( noteRenderInfo.begin()+first_note_of_chord, noteRenderInfo.begin()+i );
                i = first_note_of_chord-2;
                if(i<0) i=0;
                
                break;
            }//end if
            i++;
        }//wend
        
    }//next
    
}

void processTriplets(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
{
    const int visibleNoteAmount = noteRenderInfo.size();
    const int y_step = editor->getYStep();
    ScoreMidiConverter* converter = editor->getScoreMidiConverter();
    
    for(int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;
        
        bool is_triplet = noteRenderInfo[i].triplet;
        int first_triplet = (is_triplet ? i : -1);
        int min_level = 999;
        int max_level = -999;
        bool last_of_a_serie = false;
        
        int measure = noteRenderInfo[i].measureBegin;
        int previous_measure = measure;
        
        // check for consecutive notes
        while(true)
        {
            if(i+1<visibleNoteAmount)
            {
                start_tick_of_next_note = noteRenderInfo[i+1].tick;
            }
            
            if(!(i<visibleNoteAmount)) break;
            
            // if notes are consecutive
            if(start_tick_of_next_note != -1 and aboutEqual_tick(start_tick_of_next_note, noteRenderInfo[i].tick+noteRenderInfo[i].tick_length));
            else
            {
                //notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
            }
            if(!noteRenderInfo[i+1].triplet or i-first_triplet>=2) last_of_a_serie = true;
            
            // do not cross measures
            if(i+1<visibleNoteAmount)
            {
                measure = noteRenderInfo[i+1].measureBegin;
                if(measure != previous_measure) last_of_a_serie = true;
                previous_measure = measure;
            }
            
            if(noteRenderInfo[i].chord)
            {
                if(noteRenderInfo[i].min_chord_level < min_level) min_level = noteRenderInfo[i].min_chord_level;
                if(noteRenderInfo[i].max_chord_level > max_level) max_level = noteRenderInfo[i].max_chord_level;
            }
            else
            {
                const int level = noteRenderInfo[i].level;   
                if(level < min_level) min_level = level;
                if(level > max_level) max_level = level;
            }
            
            // --------- triplet ---------
            is_triplet = noteRenderInfo[i].triplet;
            if(is_triplet and first_triplet==-1) first_triplet = i;
            
            // this note is a triplet, but not the next, so time to do display the triplets sign
            // also triggered if we've had 3 triplet notes in a row, because triplets come by groups of 3...
            if( last_of_a_serie )
            {
                if(is_triplet)
                {
                    // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
                    if(min_level == 999)  min_level = noteRenderInfo[first_triplet].level;
                    if(max_level == -999) max_level = noteRenderInfo[first_triplet].level;
                    
                    int mid_level = (int)round( (min_level + max_level)/2.0 );
                    
                    noteRenderInfo[first_triplet].triplet_show_above = (mid_level < converter->getMiddleCLevel()-5);
                    
                    if(i != first_triplet) // if not a triplet note alone, but a 'chain' of triplets
                    {
                        // fix all note tails so they point in the right direction
                        for(int j=first_triplet; j<=i; j++)
                        {
                            noteRenderInfo[j].tail_type = ( noteRenderInfo[first_triplet].triplet_show_above ? TAIL_DOWN : TAIL_UP );
                            noteRenderInfo[j].drag_triplet_sign = false;
                        }
                    }
                    else
                    {
                        // this is either a triplet alone or a chord... just use the orientation that it already has
                        noteRenderInfo[first_triplet].triplet_show_above = (noteRenderInfo[first_triplet].tail_type == TAIL_DOWN);
                    }
                    
                    if(noteRenderInfo[first_triplet].triplet_show_above)
                    {
                        noteRenderInfo[first_triplet].triplet_arc_y = min_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels() - 2;
                    }
                    else
                    {
                        noteRenderInfo[first_triplet].triplet_arc_y = max_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels();
                    }
                    
                    noteRenderInfo[first_triplet].drag_triplet_sign = true;
                    // noteRenderInfo[first_triplet].triplet = true;
                    noteRenderInfo[first_triplet].triplet_arc_x_end = noteRenderInfo[i].x + 8;
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
    
}


void processNoteBeam(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
{
    const int visibleNoteAmount = noteRenderInfo.size();
    
    // beaming
    // all beam information is stored in the first note of the serie.
    // all others have their subtails removed
    // BeamGroup objects are used to ease beaming
    
    for(int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;
        
        int subtail_amount = noteRenderInfo[i].subtail_amount;
        int first_of_serie = i;
        bool last_of_a_serie = false;
        
        int measure = noteRenderInfo[i].measureBegin;
        int previous_measure = measure;
        
        // check for consecutive notes
        while(true)
        {
            if(i+1<visibleNoteAmount)
            {
                start_tick_of_next_note = noteRenderInfo[i+1].tick;
            }
            
            if(!(i<visibleNoteAmount)) break;
            
            // if notes are consecutive and of same length
            if(start_tick_of_next_note != -1 and
               aboutEqual_tick(start_tick_of_next_note, noteRenderInfo[i].tick+noteRenderInfo[i].tick_length) and
               noteRenderInfo[i+1].subtail_amount == subtail_amount and subtail_amount > 0 and
               noteRenderInfo[i+1].triplet == noteRenderInfo[i].triplet);
            else
            {
                //notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
            }
            
            // do not cross measures
            if(i+1<visibleNoteAmount)
            {
                measure = noteRenderInfo[i+1].measureBegin;
                if(measure != previous_measure) last_of_a_serie = true;
                previous_measure = measure;
            }
            
            // it's the last of a serie, perform actions
            if( last_of_a_serie)
            {
                if(i>first_of_serie)
                {
                    BeamGroup beam(first_of_serie, i);
                    beam.doBeam(noteRenderInfo, editor);
                }
                
                // reset
                first_of_serie = -1;
                break;
            }
            
            i++;
            
        }//wend
        
    }//next
    
}

/*
 * This function takes a vector containing information about visible notes.
 * Its job is to analyse them and fill missing data in the contained objects
 * so that they can be rendered correctly on a score.
 */

void analyseNoteInfo( std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor )
{
    putInTimeOrder( noteRenderInfo, editor );
    findAndMergeChords( noteRenderInfo, editor );
    processTriplets( noteRenderInfo, editor );
    processNoteBeam( noteRenderInfo, editor );
     
}// end analyseNotes function

#pragma mark -

/*
 * Since various midi editors use various note length conventions, and since some durations, like triplets, are only approximative,
 * we can not check for absolute equality when comparing note durations/locations, this is why I use this instead.
 */

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
