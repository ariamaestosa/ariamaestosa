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
#include "Midi/MeasureData.h"
#include "AriaCore.h"
#include <cmath>
#include <math.h>

namespace AriaMaestosa
{

int up_down_pivot_level = 0;
void setUpDownPivotLevel(const int level)
{
    up_down_pivot_level = level;
}
    
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
        const int num = getMeasureData()->getTimeSigNumerator();
        const int denom = getMeasureData()->getTimeSigDenominator();
        const int flag_amount = noteRenderInfo[first_id].flag_amount;

        int max_amount_of_notes_beamed_toghether = 4;

        if(num == 3 and denom == 4) max_amount_of_notes_beamed_toghether = 2 * (int)(std::pow(2.0,flag_amount-1));
        else if((num == 6 and denom == 4) or (num == 6 and denom == 8)) max_amount_of_notes_beamed_toghether = 3 * (int)(std::pow(2.0,flag_amount-1));
        else max_amount_of_notes_beamed_toghether = num * (int)(std::pow(2.0,flag_amount-1));
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
            const int first_tick_in_measure = getMeasureData()->firstTickInMeasure( getMeasureData()->measureAtTick(noteRenderInfo[first_id].tick) );

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

        noteRenderInfo[first_id].beam_show_above = (mid_level < up_down_pivot_level ? false : true);
        noteRenderInfo[first_id].beam = true;

        for(int j=first_id; j<=last_id; j++)
        {
            // give correct stem orientation (up or down)
            noteRenderInfo[j].stem_type = ( noteRenderInfo[first_id].beam_show_above ?  STEM_UP : STEM_DOWN );
        }

        const int last_stem_y_end = noteRenderInfo[last_id].getStemYTo();
        noteRenderInfo[first_id].beam_to_x = noteRenderInfo[last_id].getStemX();

        if(noteRenderInfo[first_id].beam_show_above)
        {
            const int min_level_y = min_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels() - 2;

            // choose y coords so that all notes are correctly in
            noteRenderInfo[first_id].beam_to_y = std::min(min_level_y - 10, last_stem_y_end); // Y to
            if(noteRenderInfo[first_id].getStemYFrom() > min_level_y - 25) noteRenderInfo[first_id].stem_y = min_level_y - 25; // Y from
        }
        else
        {
            const int max_level_y = max_level*y_step + editor->getEditorYStart() - editor->getYScrollInPixels();

            // choose y coords so that all notes are correctly in
            noteRenderInfo[first_id].beam_to_y = std::max(max_level_y + 10, last_stem_y_end); // Y to
            if(noteRenderInfo[first_id].getStemYFrom() < max_level_y + 25) noteRenderInfo[first_id].stem_y = max_level_y + 25; // Y from
        }

        // fix all note stems so they all point in the same direction and have the correct height
        const int from_x = noteRenderInfo[first_id].getStemX();
        const int from_y = noteRenderInfo[first_id].getStemYTo();
        const int to_x = noteRenderInfo[first_id].beam_to_x;
        const int to_y = noteRenderInfo[first_id].beam_to_y;

        for(int j=first_id; j<=last_id; j++)
        {
            // give correct stem height (so it doesn't end above or below beam line)
            // rel_pos will be 0 for first note of a beamed serie, and 1 for the last one
            const float rel_pos = (float)(noteRenderInfo[j].getStemX() - from_x) / (float)(to_x - from_x);
            noteRenderInfo[j].stem_y = from_y + (int)round( (to_y - from_y) * rel_pos );

            if(j != first_id) noteRenderInfo[j].flag_amount = 0;
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
    flag_amount = 0;
    y = -1;
    tied_with_tick = -1;
    tie_up = false;
    stem_type = STEM_NONE;

    draw_stem = true;

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
    stem_y = -1;
    min_chord_level=-1;
    max_chord_level=-1;

    // measure where the note begins and ends
    measureBegin = getMeasureData()->measureAtTick(tick);
	measureEnd = getMeasureData()->measureAtTick(tick + tick_length - 1);
}
void NoteRenderInfo::tieWith(NoteRenderInfo& renderInfo)
{    
    tied_with_tick = renderInfo.tick;
    
    if(stem_type == STEM_NONE) tie_up = renderInfo.stem_type;
    else tie_up = stem_type;
}
void NoteRenderInfo::setTiedToTick(const int tick)
{
    tied_with_tick = tick;
}
int NoteRenderInfo::getTiedToPixel()
{
    if(tied_with_tick == -1) return -1; // no tie
    
    RelativeXCoord tied_with(tied_with_tick, MIDI);
    return tied_with.getRelativeTo(WINDOW);
}
int NoteRenderInfo::getTiedToTick()
{
    return tied_with_tick;
}
void NoteRenderInfo::setTieUp(const bool up)
{
    tie_up = up;
}
bool NoteRenderInfo::isTieUp()
{
    return (stem_type == STEM_NONE ? tie_up : stem_type != STEM_UP);
}

void NoteRenderInfo::triplet_arc(int pixel1, int pixel2)
{
    assertExpr(pixel1,>=,0);
    assertExpr(pixel2,>=,0);
    // FIXME - this method seems not even used
    triplet_arc_x_start = pixel1;
    triplet_arc_x_end = pixel2;
}
void NoteRenderInfo::setTriplet()
{
    triplet = true;
    drag_triplet_sign = true;
}

int NoteRenderInfo::getStemX()
{
    if(stem_type == STEM_UP) return (x + 9);
    else if(stem_type == STEM_DOWN) return (x + 1);
    else return -1;
}
int NoteRenderInfo::getStemYFrom()
{
    const int stem_y_base = getYBase();
    if(stem_type == STEM_UP) return (stem_y_base + 3);
    else if(stem_type == STEM_DOWN) return (stem_y_base + 6);
    else return -1;
}
int NoteRenderInfo::getStemYTo()
{
   if(stem_type == STEM_UP) return (stem_y == -1 ? y - 24 : stem_y);
   else if(stem_type == STEM_DOWN) return (stem_y == -1 ? y + 33 : stem_y);
   else return -1;
}
int NoteRenderInfo::getYBase()
{
    if(chord) return (stem_type == STEM_UP ? max_chord_y : min_chord_y);
    else return y;
}
int NoteRenderInfo::getBaseLevel()
{
    if(chord) return (stem_type == STEM_UP ? max_chord_level : min_chord_level);
    else return level;
}
inline const int NoteRenderInfo::getY() const{ return y; }

// too be called by renderer where location is computer from level
void NoteRenderInfo::setY(const int newY)
{
    y = newY;
}

#pragma mark -

void putInTimeOrder(std::vector<NoteRenderInfo>& noteRenderInfo, ScoreEditor* editor)
{
    // put notes in time order.
    // notes that have no stems go last so that they don't disturb note grouping in chords
    const int visibleNoteAmount = noteRenderInfo.size();
#ifdef _MORE_DEBUG_CHECKS
    int iteration = 0;
#endif
    for(int i=1; i<visibleNoteAmount; i++)
    {
#ifdef _MORE_DEBUG_CHECKS
        iteration++;
        assertExpr(iteration,<,100000);
#endif
        
        // put in time order
        // making sure notes without stem come before notes with a stem
        if( noteRenderInfo[i].tick < noteRenderInfo[i-1].tick or
           ( noteRenderInfo[i].tick == noteRenderInfo[i-1].tick and noteRenderInfo[i-1].stem_type != STEM_NONE and noteRenderInfo[i].stem_type == STEM_NONE)
           )
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

    /*
     * start by merging notes playing at the same time (chords)
     * The for loop iterates through all notes. When we find notes that play at the same time,
     * the while loop starts and iterates until we reached the end of the chord.
     * when we're done with a chord, we "summarize" it in a single NoteRenderInfo object.
     * (at this point, the head of the note has been drawn and what's left to do
     * is draw stems, triplet signs, etc. so at this point a chord of note behaves just
     * like a single note).
     */
    for(int i=0; i<(int)noteRenderInfo.size(); i++)
    {
        int start_tick_of_next_note = -1;
        int first_note_of_chord = -1;
        int min_level = 999, max_level = -999;
        int maxid = i, minid = i; // id of the notes with highest and lowest Y
        int smallest_duration = 99999;
        bool last_of_a_serie = false;
        bool triplet = false;
        int flag_amount = 0;

        // when we have found a note chord, this variable will contain the ID of the
        // first note of that bunch (the ID of the last will be "i" when we get to it)
        first_note_of_chord = i;

        if(noteRenderInfo[i].stem_type == STEM_NONE) continue;

        while(true) // FIXME - it should be checked whether there is a chord BEFORE entering the while loop. same for others
        {
            if(i+1<(int)noteRenderInfo.size())
            {
                start_tick_of_next_note = noteRenderInfo[i+1].tick;
            }else start_tick_of_next_note=-1;

            if(!(i<(int)noteRenderInfo.size())) break;

            // check if we're in a chord (i.e. many notes that play at the same time). also check they have stems :
            // for instance wholes have no stems and thus there is no special processing to do on them.
            if(start_tick_of_next_note != -1 and aboutEqual_tick(start_tick_of_next_note, noteRenderInfo[i].tick) and noteRenderInfo[i+1].stem_type != STEM_NONE);
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
            if(len < smallest_duration or smallest_duration==99999)
                smallest_duration = len;

            if(noteRenderInfo[i].flag_amount > flag_amount)
                flag_amount = noteRenderInfo[i].flag_amount;

            if(noteRenderInfo[i].triplet) triplet = true;

            // remove this note's stem. we only need on stem per chord.
            // the right stem will be set on last iterated note of the chord (see below)
            noteRenderInfo[i].draw_stem = false;

            // the note of this iteration is the end of a chord, so it's time to complete chord information
            if(last_of_a_serie)
            {
                if(maxid == minid) break;
                if(first_note_of_chord == i) break;

                // determine average note level to know if we put stems above or below
                // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
                if(min_level == 999)  min_level = noteRenderInfo[first_note_of_chord].level;
                if(max_level == -999) max_level = noteRenderInfo[first_note_of_chord].level;
                const int mid_level = (int)round( (min_level + max_level)/2.0 );

                const bool stem_up = mid_level >= up_down_pivot_level+2;

                const int maxy = editor->getEditorYStart() + y_step*max_level - halfh - editor->getYScrollInPixels() + 2;
                const int miny = editor->getEditorYStart() + y_step*min_level - halfh - editor->getYScrollInPixels() + 2;

                // decide the one note to keep that will "summarize" all others.
                // it will be the highest or the lowest, depending on if stem is up or down.
                // feed this NoteRenderInfo object with the info Aria believes will best summarize
                // the chord. results may vary if you make very different notes play at the same time.
                NoteRenderInfo summary = noteRenderInfo[ stem_up ? minid : maxid ];
                summary.chord = true;
                summary.max_chord_y = maxy;
                summary.min_chord_y = miny;
                summary.min_chord_level = min_level;
                summary.max_chord_level = max_level;
                summary.flag_amount = flag_amount;
                summary.triplet = triplet;
                summary.draw_stem = true;
                summary.stem_type = (stem_up ? STEM_UP : STEM_DOWN);
                summary.tick_length = smallest_duration;

                summary.setTiedToTick(noteRenderInfo[ !stem_up ? minid : maxid ].getTiedToTick());
                summary.setTieUp(noteRenderInfo[ !stem_up ? minid : maxid ].isTieUp());

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

                    noteRenderInfo[first_triplet].triplet_show_above = (mid_level < up_down_pivot_level);

                    if(i != first_triplet) // if not a triplet note alone, but a 'serie' of triplets
                    {
                        // fix all note stems so they all point in the same direction
                        for(int j=first_triplet; j<=i; j++)
                        {
                            noteRenderInfo[j].stem_type = ( noteRenderInfo[first_triplet].triplet_show_above ? STEM_DOWN : STEM_UP );
                            noteRenderInfo[j].drag_triplet_sign = false;
                        }
                    }
                    else
                    {
                        // this is either a triplet alone or a chord... just use the orientation that it already has
                        noteRenderInfo[first_triplet].triplet_show_above = (noteRenderInfo[first_triplet].stem_type == STEM_DOWN);
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
    // all others have their flags removed
    // BeamGroup objects are used to ease beaming

    for(int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;

        int flag_amount = noteRenderInfo[i].flag_amount;
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
               noteRenderInfo[i+1].flag_amount == flag_amount and flag_amount > 0 and
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

void addToVector( NoteRenderInfo& renderInfo, std::vector<NoteRenderInfo>& vector, const bool recursion )
{
    // check if note lasts more than one measure. If so we need to divide it in 2.
	if(renderInfo.measureEnd > renderInfo.measureBegin) // note in longer than mesaure, need to divide it in 2
	{
		const int firstEnd = getMeasureData()->lastTickInMeasure(renderInfo.measureBegin);
		const int firstLength = firstEnd - renderInfo.tick;
		const int secondLength = renderInfo.tick_length - firstLength;
		
		RelativeXCoord firstEndRel(firstEnd, MIDI);
		
		// split the note in two, and collect resulting notes in a vector.
		// then we can iterate through that vector and tie all notes together
		// (remember, note may be split in more than 2 if one of the 2 initial halves has a rare length)
		
		
		int initial_id = -1;
		
		if(!recursion)
		{
			initial_id = vector.size();
		}
		
		NoteRenderInfo part1(renderInfo.tick, renderInfo.x, renderInfo.level, firstLength, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		addToVector(part1, vector, true);
		NoteRenderInfo part2(getMeasureData()->firstTickInMeasure(renderInfo.measureBegin+1), firstEndRel.getRelativeTo(WINDOW),
                             renderInfo.level, secondLength, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		addToVector(part2, vector, true);
		
		if(!recursion)
		{
            // done splitting, now iterate through all notes that
            // were added in this recusrion and tie them
			const int amount = vector.size();
			for(int i=initial_id+1; i<amount; i++)
			{
				vector[i].tieWith(vector[i-1]);
			}
		}
		
		return;
	}
	
    // find how to draw notes. how many flags, dotted, triplet, etc.
    // if note duration is unknown it will be split
	const float relativeLength = renderInfo.tick_length / (float)(getMeasureData()->beatLengthInTicks()*4);
    
	renderInfo.stem_type = (renderInfo.level >= up_down_pivot_level ? STEM_UP : STEM_DOWN);
	if(relativeLength>=1) renderInfo.stem_type=STEM_NONE; // whole notes have no stem
	renderInfo.hollow_head = false;
	
    const int beat = getMeasureData()->beatLengthInTicks();
    const int tick_in_measure_start = renderInfo.tick - getMeasureData()->firstTickInMeasure( renderInfo.measureBegin );
    const int remaining = beat - (tick_in_measure_start % beat);
    const bool starts_on_beat = aboutEqual(remaining,0) or aboutEqual(remaining,beat);
    
	if( aboutEqual(relativeLength, 1.0) ){ renderInfo.hollow_head = true; renderInfo.stem_type=STEM_NONE; }
	else if( aboutEqual(relativeLength, 1.0/2.0) ){ renderInfo.hollow_head = true; } // 1/2
	else if( aboutEqual(relativeLength, 1.0/3.0) ){ renderInfo.setTriplet(); renderInfo.hollow_head = true; } // triplet 1/2
	else if( aboutEqual(relativeLength, 1.0/4.0) ); // 1/4
	else if( aboutEqual(relativeLength, 1.0/6.0) ){ renderInfo.setTriplet(); } // triplet 1/4
	else if( aboutEqual(relativeLength, 1.0/8.0) ) renderInfo.flag_amount = 1; // 1/8
	else if( aboutEqual(relativeLength, 1.0/12.0) ){ renderInfo.setTriplet(); renderInfo.flag_amount = 1; } // triplet 1/8
	else if( aboutEqual(relativeLength, 1.0/16.0) ) renderInfo.flag_amount = 2; // 1/16
	else if( aboutEqual(relativeLength, 1.0/24.0) ) { renderInfo.setTriplet(); renderInfo.flag_amount = 2; } // triplet 1/16
	else if( aboutEqual(relativeLength, 1.0/32.0) ) renderInfo.flag_amount = 3; // 1/32
	else if( aboutEqual(relativeLength, 3.0/4.0) and starts_on_beat){ renderInfo.dotted = true; renderInfo.hollow_head=true; } // dotted 1/2
	else if( aboutEqual(relativeLength, 3.0/8.0) and starts_on_beat ) renderInfo.dotted = true; // dotted 1/4
	else if( aboutEqual(relativeLength, 3.0/2.0) and starts_on_beat ){ renderInfo.dotted = true; renderInfo.hollow_head=true; } // dotted whole
	else if( relativeLength < 1.0/32.0 )
	{
		renderInfo.instant_hit = true;
	}
	else
	{ // note is of unknown duration. split it in a serie of tied notes.
        
        
        // how long is the first note after the split?
        int firstLength_tick;
        
        // start by reaching the next beat if not already done
		if(!starts_on_beat and !aboutEqual(remaining, renderInfo.tick_length))
		{
            firstLength_tick = remaining;
		}
        else
        {
            // use division to split note
            float closestShorterDuration = 1;
            while(closestShorterDuration >= relativeLength) closestShorterDuration /= 2.0;
            
            firstLength_tick = closestShorterDuration*(float)(getMeasureData()->beatLengthInTicks()*4);
		}
        
        const int secondBeginning_tick = renderInfo.tick + firstLength_tick;
        RelativeXCoord secondBeginningRel(secondBeginning_tick, MIDI);
        
		int initial_id = -1;
		
		if(!recursion)
		{
			initial_id = vector.size();
		}
		
		NoteRenderInfo part1(renderInfo.tick, renderInfo.x, renderInfo.level, firstLength_tick, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		addToVector(part1, vector, true);
		NoteRenderInfo part2(secondBeginning_tick, secondBeginningRel.getRelativeTo(WINDOW), renderInfo.level,
                             renderInfo.tick_length-firstLength_tick, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		addToVector(part2, vector, true);
		
		if(!recursion)
		{
            // done splitting, now iterate through all notes that
            // were added in this recusrion and tie them
			const int amount = vector.size();
			for(int i=initial_id+1; i<amount; i++)
			{
				vector[i].tieWith(vector[i-1]);
			}
		}
		
		return;
	}
	
    if(renderInfo.triplet)
    {
        renderInfo.triplet_arc_x_start = renderInfo.x + 8;
        renderInfo.triplet_arc_y = renderInfo.getY();
    }
	
    assertExpr(renderInfo.level,>,-1);    
    
    vector.push_back(renderInfo);
}

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
	return std::abs(int1 - int2) < getMeasureData()->beatLengthInTicks()/16;
}


}
