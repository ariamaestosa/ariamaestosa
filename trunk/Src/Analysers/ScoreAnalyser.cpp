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

#include "Analysers/ScoreAnalyser.h"

#include "AriaCore.h"
#include "Editors/ScoreEditor.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"

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
    int m_first_id, m_last_id;
    int m_min_level, m_mid_level, m_max_level;
    ScoreAnalyser* m_analyser;

public:
    BeamGroup(ScoreAnalyser* analyser, const int first_id, const int last_id)
    {
        m_first_id  = first_id;
        m_last_id   = last_id;
        m_analyser  = analyser;
        m_min_level = 999;
        m_max_level = -999;
    }
    
    void calculateLevel(std::vector<NoteRenderInfo>& m_note_render_info, ScoreMidiConverter* converter)
    {
        for (int i=m_first_id;i<=m_last_id; i++)
        {
            if (m_note_render_info[i].m_chord)
            {
                if (m_note_render_info[i].m_min_chord_level < m_min_level)
                {
                    m_min_level = m_note_render_info[i].m_min_chord_level;
                }
                
                if (m_note_render_info[i].m_max_chord_level > m_max_level)
                {
                    m_max_level = m_note_render_info[i].m_max_chord_level;
                }
            }
            else
            {
                const int level = m_note_render_info[i].getLevel();
                if (level < m_min_level) m_min_level = level;
                if (level > m_max_level) m_max_level = level;
            }
        }

        // if nothing found (most likely meaning we only have one triplet note alone) use values from the first
        if (m_min_level == 999)  m_min_level = m_note_render_info[m_first_id].getLevel();
        if (m_max_level == -999) m_max_level = m_note_render_info[m_first_id].getLevel();

        m_mid_level = (int)round( (m_min_level + m_max_level)/2.0 );
    }

    void doBeam(std::vector<NoteRenderInfo>& m_note_render_info, ScoreEditor* editor)
    {
        ASSERT( editor != NULL );
        
        if (m_last_id == m_first_id) return; // note alone, no beaming to perform

        ScoreMidiConverter* converter = editor->getScoreMidiConverter();
        //const int y_step = editor->getYStep();

        Sequence* seq = editor->getSequence();
        MeasureData* md = seq->getMeasureData();
        
        // check for number of "beamable" notes and split if current amount is not acceptable with the current time sig
        // only considering note 0 to get the measure should be fine since Aria only beams within the same measure
        const int num   = md->getTimeSigNumerator  ( m_note_render_info[m_first_id].m_measure_begin );
        const int denom = md->getTimeSigDenominator( m_note_render_info[m_first_id].m_measure_begin );
        const int flag_amount = m_note_render_info[m_first_id].m_flag_amount;

        int max_amount_of_notes_beamed_toghether = 4;

        // FIXME - not always right
        if (num == 3 and denom == 4)
        {
            max_amount_of_notes_beamed_toghether = 2 * (int)(std::pow(2.0,flag_amount-1));
        }
        else if ((num == 6 and denom == 4) or (num == 6 and denom == 8))
        {
            max_amount_of_notes_beamed_toghether = 3 * (int)(std::pow(2.0,flag_amount-1));
        }
        else
        {
            max_amount_of_notes_beamed_toghether = num * (int)(std::pow(2.0,flag_amount-1));
        }
        
        if (m_note_render_info[m_first_id].m_triplet) max_amount_of_notes_beamed_toghether = 3;

        const int beamable_note_amount = m_last_id - m_first_id + 1;

        // if max_amount_of_notes_beamed_toghether is an even number, don't accept an odd number of grouped notes
        const int base_unit = (max_amount_of_notes_beamed_toghether % 2 == 0 ? 2 : 1);
        if (beamable_note_amount <= max_amount_of_notes_beamed_toghether and beamable_note_amount % base_unit != 0)
        {
            max_amount_of_notes_beamed_toghether = base_unit;
        }

        if (beamable_note_amount > max_amount_of_notes_beamed_toghether)
        {
            // amount is not acceptable, split

            // try to find where beamed groups of such notes usually start and end in the measure
            // this is where splitting should be performed
            const int group_len = m_note_render_info[m_first_id].getTickLength() * max_amount_of_notes_beamed_toghether;
            const int measId = md->measureAtTick(m_note_render_info[m_first_id].getTick());
            const int first_tick_in_measure = md->firstTickInMeasure( measId );

            int split_at_id = -1;
            for (int n=m_first_id+1; n<=m_last_id; n++)
            {
                if ((m_note_render_info[n].getTick() - first_tick_in_measure) % group_len == 0)
                {
                    split_at_id = n;
                    break;
                }
            }

            if (split_at_id == -1)
            {
                // dumb split
                BeamGroup first_half(m_analyser, m_first_id, m_first_id + max_amount_of_notes_beamed_toghether - 1);
                BeamGroup second_half(m_analyser, m_first_id + max_amount_of_notes_beamed_toghether, m_last_id);
                first_half.doBeam(m_note_render_info, editor);
                second_half.doBeam(m_note_render_info, editor);
            }
            else
            {
                BeamGroup first_half(m_analyser, m_first_id, split_at_id - 1);
                BeamGroup second_half(m_analyser, split_at_id, m_last_id);
                first_half.doBeam(m_note_render_info, editor);
                second_half.doBeam(m_note_render_info, editor);
            }

            return;
        }

        calculateLevel(m_note_render_info, converter);

        m_note_render_info[m_first_id].m_beam_show_above = m_analyser->stemUp(m_mid_level);
        m_note_render_info[m_first_id].m_beam = true;

        for (int j=m_first_id; j<=m_last_id; j++)
        {
            // give correct stem orientation (up or down)
            m_note_render_info[j].m_stem_type = (m_note_render_info[m_first_id].m_beam_show_above ?  STEM_UP : STEM_DOWN);

            // reset any already set stem location, since we'll need to totally redo them for the beam
            m_note_render_info[j].m_stem_y_level = -1;
        }

        // set initial beam info in note
        m_note_render_info[m_first_id].m_beam_to_tick  = m_note_render_info[m_last_id].getTick();
        m_note_render_info[m_first_id].m_beam_to_sign  = m_note_render_info[m_last_id].m_sign;
        m_note_render_info[m_first_id].m_beam_to_level = m_analyser->getStemTo(m_note_render_info[m_last_id]);
        m_note_render_info[m_first_id].m_stem_y_level  = m_analyser->getStemTo(m_note_render_info[m_first_id]);

        // check if the stem is too inclined, fix it if necessary
        const float height_diff = fabsf(m_note_render_info[m_first_id].m_beam_to_level -
                                        m_note_render_info[m_first_id].m_stem_y_level );

        if (height_diff > 3)
        {
            const float height_shift = height_diff - 3;
            const bool end_on_higher_level = (m_note_render_info[m_first_id].m_beam_to_level >
                                              m_note_render_info[m_first_id].m_stem_y_level );
            
            if (m_note_render_info[m_first_id].m_beam_show_above)
            {
                if (end_on_higher_level) m_note_render_info[m_first_id].m_beam_to_level -= height_shift;
                else                     m_note_render_info[m_first_id].m_stem_y_level  -= height_shift;
            }
            else
            {
                if (end_on_higher_level) m_note_render_info[m_first_id].m_stem_y_level  += height_shift;
                else                     m_note_render_info[m_first_id].m_beam_to_level += height_shift;
            }
        }

        // fix all note stems so they all point in the same direction and have the correct height
        while (true)
        {
            const int   from_tick  = m_note_render_info[m_first_id].getTick();
            const float from_level = m_analyser->getStemTo(m_note_render_info[m_first_id]);
            const int   to_tick    = m_note_render_info[m_first_id].m_beam_to_tick;
            const float to_level   = m_note_render_info[m_first_id].m_beam_to_level;

            bool need_to_start_again = false;
            for (int j=m_first_id; j<=m_last_id; j++)
            {
                // give correct stem height (so it doesn't end above or below beam line)
                // rel_pos will be 0 for first note of a beamed serie, and 1 for the last one
                const float rel_pos = (float)(m_note_render_info[j].getTick() - from_tick) / (float)(to_tick - from_tick);
                if (j != m_first_id)
                {
                    m_note_render_info[j].m_stem_y_level = (float)from_level +
                                                       (float)(to_level - from_level) * rel_pos;
                }

                // check if stem is long enough and on right side of beam
                // here the distinction between base level and stem origin is tricky but necessary
                // to preporly deal with chords. In a chord, when we check if the stem is long enough,
                // we don't want to check the entire stem, only the part coming out of the top/bottom note
                const float stemheight = fabsf(m_note_render_info[j].m_stem_y_level -
                                               m_note_render_info[j].getBaseLevel());
                const bool too_short = stemheight < m_analyser->min_stem_height;

                const float diff = m_note_render_info[j].m_stem_y_level - m_note_render_info[j].getBaseLevel();
                
                const bool on_wrong_side_of_beam =
                    (m_note_render_info[m_first_id].m_beam_show_above and diff > 0) or
                    ((not m_note_render_info[m_first_id].m_beam_show_above) and diff < 0);

                if (too_short or on_wrong_side_of_beam)
                {
                    // we've got a problem here. this stem is too short and will look weird
                    // we'll adjust the height of the beam and try again
                    // (the order of the tests here is important; if the beam is on the wrong
                    // side of the beam AND too short, only the first should should happen)
                    float beam_shift = 0;
                    if (on_wrong_side_of_beam) beam_shift = m_analyser->min_stem_height + fabsf(diff);
                    else if (too_short)        beam_shift = m_analyser->min_stem_height - stemheight;

                    if (m_note_render_info[m_first_id].m_beam_show_above)
                    {
                        m_note_render_info[m_first_id].m_beam_to_level -= beam_shift;
                        m_note_render_info[m_first_id].m_stem_y_level  -= beam_shift;
                    }
                    else
                    {
                        m_note_render_info[m_first_id].m_beam_to_level += beam_shift;
                        m_note_render_info[m_first_id].m_stem_y_level  += beam_shift;
                    }
                    need_to_start_again = true;
                    break;
                }

                if (j != m_first_id) m_note_render_info[j].m_flag_amount = 0;
            }
            if (not need_to_start_again) break; // we're done, no need to loop again
        }
    }
};

}

using namespace AriaMaestosa;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark NoteRenderInfo
#endif

NoteRenderInfo::NoteRenderInfo(int tick, int level, int tick_length, PitchSign sign, const bool selected,
                               int pitch, const int measureBegin, const int measureEnd)
{
    ASSERT_E(tick_length, >=, 0);
    ASSERT_E(tick, >=, 0);
    
    // what we know before render pass 1
    m_selected    = selected;
    m_tick        = tick;
    m_tick_length = tick_length;
    m_sign        = sign;
    m_level       = level;
    m_pitch       = pitch;
    
    // what we will know after render pass 1
    m_instant_hit    = false;
    m_triplet        = false;
    m_dotted         = false;
    m_flag_amount    = 0;
    m_y              = -1;
    m_tied_with_tick = -1;
    m_tie_up         = false;
    m_stem_type      = STEM_NONE;

    m_draw_stem      = true;

    m_triplet_show_above     = false;
    m_triplet_arc_tick_start = -1;
    m_triplet_arc_tick_end   = -1;
    m_draw_triplet_sign      = false;

    m_beam_show_above = false;
    m_beam_to_tick    = -1;
    m_beam_to_level   = -1;
    m_beam            = false;

    m_chord           = false;
    m_stem_y_level    = -1;
    m_min_chord_level = -1;
    m_max_chord_level = -1;

    // measure where the note begins and ends
    m_measure_begin = measureBegin;
    m_measure_end   = measureEnd;
}

NoteRenderInfo NoteRenderInfo::factory(int tick, int level, int tick_length, PitchSign sign, const bool selected,
                                       int pitch, MeasureData* md)
{
    return NoteRenderInfo(tick, level, tick_length, sign, selected, pitch,
                          md->measureAtTick(tick), md->measureAtTick(tick + tick_length - 1));
}
// -----------------------------------------------------------------------------------------------------------

void NoteRenderInfo::tieWith(NoteRenderInfo& renderInfo)
{
    m_tied_with_tick = renderInfo.m_tick;

    if (m_stem_type == STEM_NONE) m_tie_up = renderInfo.m_stem_type;
    else                          m_tie_up = m_stem_type;
}

// -----------------------------------------------------------------------------------------------------------

void NoteRenderInfo::tieWith(const int tick)
{
    m_tied_with_tick = tick;
}

// -----------------------------------------------------------------------------------------------------------

void NoteRenderInfo::setTieUp(const bool up)
{
    m_tie_up = up;
}

// -----------------------------------------------------------------------------------------------------------

void NoteRenderInfo::setTriplet()
{
    m_triplet = true;
    m_draw_triplet_sign = true;
}

// -----------------------------------------------------------------------------------------------------------

int NoteRenderInfo::getBaseLevel()
{
    if (m_chord) return (m_stem_type == STEM_UP ? m_min_chord_level : m_max_chord_level);
    else         return m_level;
}

// -----------------------------------------------------------------------------------------------------------

int NoteRenderInfo::getStemOriginLevel()
{
    if (m_chord) return (m_stem_type == STEM_UP ? m_max_chord_level : m_min_chord_level);
    else         return m_level;
}

// -----------------------------------------------------------------------------------------------------------

void NoteRenderInfo::setY(const int newY)
{
    m_y = newY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Score Analyser (public)
#endif

ScoreAnalyser::ScoreAnalyser(Editor* parent, int stemPivot)
{
    m_editor = parent;
    m_stem_pivot = stemPivot;

    stem_height = 5.2;
    min_stem_height = 4.5;
}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::addToVector( NoteRenderInfo& renderInfo )
{
    addToVector( renderInfo, false );
}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::addToVector( NoteRenderInfo& renderInfo, const bool recursion )
{
    Sequence* seq = m_editor->getSequence();
    MeasureData* md = seq->getMeasureData();

    // check if note lasts more than one measure. If so we need to divide it in 2.
    if (renderInfo.m_measure_end > renderInfo.m_measure_begin) 
    {
        const int firstEnd     = md->lastTickInMeasure(renderInfo.m_measure_begin);
        const int firstLength  = firstEnd - renderInfo.getTick();
        const int secondLength = renderInfo.getTickLength() - firstLength;
        
        // split the note in two, and collect resulting notes in a vector.
        // then we can iterate through that vector and tie all notes together
        // (remember, note may be split in more than 2 if one of the 2 initial halves has a rare length)
        
        int initial_id = -1;
        if (not recursion) initial_id = m_note_render_info.size();
        
        if (aboutEqual(firstLength, 0)) return;
        if (aboutEqual(secondLength, 0)) return;
        
        NoteRenderInfo part1 = NoteRenderInfo::factory(renderInfo.getTick(), renderInfo.getLevel(), firstLength,
                                                       renderInfo.m_sign, renderInfo.m_selected,
                                                       renderInfo.m_pitch, md);
        addToVector(part1, true);
        NoteRenderInfo part2 = NoteRenderInfo::factory(md->firstTickInMeasure(renderInfo.m_measure_begin+1),
                                                       renderInfo.getLevel(), secondLength, renderInfo.m_sign,
                                                       renderInfo.m_selected, renderInfo.m_pitch, md);
        addToVector(part2, true);
        
        if (not recursion)
        {
            // done splitting, now iterate through all notes that
            // were added in this recusrion and tie them
            const int amount = m_note_render_info.size();
            for (int i=initial_id+1; i<amount; i++)
            {
                m_note_render_info[i].tieWith(m_note_render_info[i-1]);
            }
        }
        
        return;
    }
    
    // find how to draw notes. how many flags, dotted, triplet, etc.
    // if note duration is unknown it will be split 
    const int beat = seq->ticksPerBeat();
    const float relativeLength = renderInfo.getTickLength() / (float)(beat*4);
    
    renderInfo.m_stem_type = (stemUp(renderInfo.getLevel()) ? STEM_UP : STEM_DOWN);
    if (relativeLength >= 1) renderInfo.m_stem_type = STEM_NONE; // whole notes have no stem
    renderInfo.m_hollow_head = false;
    
    const int tick_in_measure_start = renderInfo.getTick() -
                                      md->firstTickInMeasure( renderInfo.m_measure_begin );
    const int remaining = beat - (tick_in_measure_start % beat);
    const bool starts_on_beat = aboutEqual(remaining,0) or aboutEqual(remaining,beat);
    
    if ( aboutEqual(relativeLength, 1.0) ){ renderInfo.m_hollow_head = true; renderInfo.m_stem_type = STEM_NONE; }
    else if ( aboutEqual(relativeLength, 1.0/2.0) ){ renderInfo.m_hollow_head = true; } // 1/2
    else if ( aboutEqual(relativeLength, 1.0/3.0) ){ renderInfo.setTriplet(); renderInfo.m_hollow_head = true; } // triplet 1/2
    else if ( aboutEqual(relativeLength, 1.0/4.0) ); // 1/4
    else if ( relativeLength > 0.146f and relativeLength < 0.208f ) { renderInfo.setTriplet();      } // triplet 1/4 (1/6)
    else if ( relativeLength > 0.104f and relativeLength < 0.146f ) { renderInfo.m_flag_amount = 1; } // 1/8
    else if ( relativeLength > 0.073f and relativeLength < 0.104f ) { renderInfo.setTriplet(); renderInfo.m_flag_amount = 1; } // triplet 1/8 (1/12)
    else if ( relativeLength > 0.052f and relativeLength < 0.073f ) { renderInfo.m_flag_amount = 2; } // 1/16
    else if ( relativeLength > 0.037f and relativeLength < 0.052f ) { renderInfo.setTriplet(); renderInfo.m_flag_amount = 2; } // triplet 1/16 (1/24)
    else if ( relativeLength > 0.023f and relativeLength < 0.037f ) { renderInfo.m_flag_amount = 3; } // 1/32
    else if ( aboutEqual(relativeLength, 3.0/4.0) and starts_on_beat){ renderInfo.m_dotted = true; renderInfo.m_hollow_head=true; } // dotted 1/2
    else if ( aboutEqual(relativeLength, 3.0/8.0) and starts_on_beat ) renderInfo.m_dotted = true; // dotted 1/4
    else if ( aboutEqual(relativeLength, 3.0/2.0) and starts_on_beat ){ renderInfo.m_dotted = true; renderInfo.m_hollow_head=true; } // dotted whole
    else if ( relativeLength < 1.0/32.0 )
    {
        renderInfo.m_instant_hit = true;
    }
    else
    { // note is of unknown duration. split it in a serie of tied notes.
        
        
        // how long is the first note after the split?
        int firstLength_tick;
        
        // start by reaching the next beat if not already done
        if (not starts_on_beat and not aboutEqual(remaining, renderInfo.getTickLength()) and remaining <= renderInfo.getTickLength())
        {
            firstLength_tick = remaining;
        }
        else
        {
            // use division to split note
            float closestShorterDuration = 1;
            while (closestShorterDuration >= relativeLength) closestShorterDuration /= 2.0;
            
            firstLength_tick = closestShorterDuration*(float)(beat*4);
        }
        
        const int secondBeginning_tick = renderInfo.getTick() + firstLength_tick;
        
        int initial_id = -1;
        
        if (not recursion)
        {
            initial_id = m_note_render_info.size();
        }
        
        NoteRenderInfo part1 = NoteRenderInfo::factory(renderInfo.getTick(), renderInfo.getLevel(),
                                                       firstLength_tick, renderInfo.m_sign,
                                                       renderInfo.m_selected, renderInfo.m_pitch, md);
        addToVector(part1, true);
        NoteRenderInfo part2 = NoteRenderInfo::factory(secondBeginning_tick, renderInfo.getLevel(),
                                                       renderInfo.getTickLength() - firstLength_tick, renderInfo.m_sign,
                                                       renderInfo.m_selected, renderInfo.m_pitch, md);
        addToVector(part2, true);
        
        if (not recursion)
        {
            // done splitting, now iterate through all notes that
            // were added in this recusrion and tie them
            const int amount = m_note_render_info.size();
            for (int i=initial_id+1; i<amount; i++)
            {
                m_note_render_info[i].tieWith(m_note_render_info[i-1]);
            }
        }
        
        return;
    }
    
    if (renderInfo.m_triplet)
    {
        renderInfo.m_triplet_arc_tick_start = renderInfo.getTick();
        renderInfo.m_triplet_arc_level      = renderInfo.getLevel();
    }
    
    ASSERT_E(renderInfo.getLevel(),>,-1);
    m_note_render_info.push_back(renderInfo);
}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::setStemPivot(const int level)
{
    m_stem_pivot = level;
}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::clearAndPrepare()
{
    m_note_render_info.clear();
}

// -----------------------------------------------------------------------------------------------------------

float ScoreAnalyser::getStemTo(NoteRenderInfo& note)
{
    if      (note.m_stem_y_level != -1)     return note.m_stem_y_level;
    else if (note.m_stem_type == STEM_UP)   return note.getStemOriginLevel() - stem_height;
    else if (note.m_stem_type == STEM_DOWN) return note.getStemOriginLevel() + stem_height;
    else { ASSERT(false); return -1; }
}
    
// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::analyseNoteInfo()
{
    putInTimeOrder();
    findAndMergeChords();
    processTriplets();
    processNoteBeam();
    
}

// -----------------------------------------------------------------------------------------------------------

ScoreAnalyser* ScoreAnalyser::getSubset(const int fromTick, const int toTick) const
{
    ScoreAnalyser* out = new ScoreAnalyser();
    out->m_editor        = m_editor;
    out->m_stem_pivot    = m_stem_pivot;
    out->min_stem_height = min_stem_height;
    out->stem_height     = stem_height;
        
    // Copy only the note render infos that fit the specified tick range
    // TODO: are the note render infos sorted by tick? If so this could be made faster
    const int count = m_note_render_info.size();
    for (int n=0; n<count; n++)
    {
        if (m_note_render_info[n].getTick() >= fromTick and m_note_render_info[n].getTick() < toTick)
        {
            out->m_note_render_info.push_back( m_note_render_info[n] );
        }
    }
    
    return out;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Score Analyser (private)
#endif

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::putInTimeOrder()
{

    const int visibleNoteAmount = m_note_render_info.size();
#ifdef _MORE_DEBUG_CHECKS
    int iteration = 0;
#endif
    for (int i=1; i<visibleNoteAmount; i++)
    {
#ifdef _MORE_DEBUG_CHECKS
        iteration++;
        ASSERT_E(iteration,<,100000);
#endif

        // put in time order
        // making sure notes without stem come before notes with a stem
        if (m_note_render_info[i].getTick() < m_note_render_info[i-1].getTick() or
            (m_note_render_info[i].getTick() == m_note_render_info[i-1].getTick() and
             m_note_render_info[i-1].m_stem_type != STEM_NONE and m_note_render_info[i].m_stem_type == STEM_NONE)
           )
        {
            NoteRenderInfo tmp = m_note_render_info[i-1];
            m_note_render_info[i-1] = m_note_render_info[i];
            m_note_render_info[i] = tmp;
            i -= 2; if (i<0) i=0;
        }
    }

}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::findAndMergeChords()
{
    const int beatLen = m_editor->getSequence()->ticksPerBeat();
    
    /*
     * start by merging notes playing at the same time (chords)
     * The for loop iterates through all notes. When we find notes that play at the same time,
     * the while loop starts and iterates until we reached the end of the chord.
     * when we're done with a chord, we "summarize" it in a single NoteRenderInfo object.
     * (at this point, the head of the note has been drawn and what's left to do
     * is draw stems, triplet signs, etc. so at this point a chord of note behaves just
     * like a single note).
     */
    for (int i=0; i<(int)m_note_render_info.size(); i++)
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

        if (m_note_render_info[i].m_stem_type == STEM_NONE) continue;

        while (true) // FIXME - it should be checked whether there is a chord BEFORE entering the while loop. same for others
        {
            // find next note's tick if there's one
            if (i + 1 < (int)m_note_render_info.size())
            {
                start_tick_of_next_note = m_note_render_info[i+1].getTick();
            }
            else
            {
                start_tick_of_next_note = -1;
            }
            
            // we've processed all notes, exit the loop
            if (not (i<(int)m_note_render_info.size())) break;

            // check if we're in a chord (i.e. many notes that play at the same time). also check they have stems :
            // for instance wholes have no stems and thus there is no special processing to do on them.
            if (start_tick_of_next_note != -1 and
                aboutEqual_tick(start_tick_of_next_note, m_note_render_info[i].getTick(), beatLen) and
                m_note_render_info[i+1].m_stem_type != STEM_NONE)
            {
            }
            else
            {
                //after this one, it stops. mark this as the last so it will finalize stuff.
                last_of_a_serie = true;
                if (first_note_of_chord == i) break; //not a bunch of concurrent notes, just a note alone
            }
            
            // gather info on notes of the chord, for instance their y location (level) and their duration
            const int level = m_note_render_info[i].getLevel();
            
            if (level < min_level)
            {
                min_level = level;
                minid = i;
            }
            if (level > max_level)
            {
                max_level = level;
                maxid = i;
            }

            const int len = m_note_render_info[i].getTickLength();
            if (len < smallest_duration or smallest_duration == 99999)
            {
                smallest_duration = len;
            }

            if (m_note_render_info[i].m_flag_amount > flag_amount)
            {
                flag_amount = m_note_render_info[i].m_flag_amount;
            }

            if (m_note_render_info[i].m_triplet) triplet = true;

            // remove this note's stem. we only need on stem per chord.
            // the right stem will be set on last iterated note of the chord (see below)
            m_note_render_info[i].m_draw_stem = false;

            // the note of this iteration is the end of a chord, so it's time to complete chord information
            if (last_of_a_serie)
            {
                if (maxid == minid) break;
                if (first_note_of_chord == i) break;

                // determine average note level to know if we put stems above or below
                // if nothing found use values from the first
                if (min_level == 999)  min_level = m_note_render_info[first_note_of_chord].getLevel();
                if (max_level == -999) max_level = m_note_render_info[first_note_of_chord].getLevel();
                const int mid_level = (int)round( (min_level + max_level)/2.0 );

                const bool stem_up = stemUp(mid_level);

                /*
                 * decide the one note to keep that will "summarize" all others.
                 * it will be the highest or the lowest, depending on if stem is up or down.
                 * feed this NoteRenderInfo object with the info Aria believes will best summarize
                 * the chord. results may vary if you make very different notes play at the same time.
                 * Making a chord into a single note allows it to enter other steps of analysis,
                 * like beaming.
                 */
                NoteRenderInfo summary = m_note_render_info[ stem_up ? minid : maxid ];
                summary.m_chord = true;
                summary.m_min_chord_level = min_level;
                summary.m_max_chord_level = max_level;
                summary.m_stem_y_level = (stem_up ?
                                          m_note_render_info[minid].getStemOriginLevel() - stem_height :
                                          m_note_render_info[maxid].getStemOriginLevel() + stem_height);
                summary.m_flag_amount = flag_amount;
                summary.m_triplet = triplet;
                summary.m_draw_stem = true;
                summary.m_stem_type = (stem_up ? STEM_UP : STEM_DOWN);
                summary.setLength( smallest_duration );

                summary.tieWith( m_note_render_info[ !stem_up ? minid : maxid ].getTiedToTick() );
                summary.setTieUp( m_note_render_info[ !stem_up ? minid : maxid ].isTieUp() );
                
                m_note_render_info[i] = summary;

                // now that we summarised concurrent notes into a single one, we can erase the other notes of the chord
                ASSERT_E(i,<,(int)m_note_render_info.size());
                m_note_render_info.erase( m_note_render_info.begin()+first_note_of_chord, m_note_render_info.begin()+i );
                i = first_note_of_chord-2;
                if (i<0) i=0;

                break;
            }//end if
            i++;
        }//wend

    }//next

}

// -----------------------------------------------------------------------------------------------------------

const bool VERBOSE_ABOUT_TRIPLETS = false;

void ScoreAnalyser::processTriplets()
{
    const int visibleNoteAmount = m_note_render_info.size();
    const int beatLen = m_editor->getSequence()->ticksPerBeat();
    
    for (int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;

        bool is_triplet = m_note_render_info[i].m_triplet;
        int first_triplet = (is_triplet ? i : -1);
        int min_level = 999;
        int max_level = -999;
        bool last_of_a_serie = false;

        int measure = m_note_render_info[i].m_measure_begin;
        int previous_measure = measure;

        if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3) ---- Measure " << (measure+1) << " ----\n";
        
        // check for consecutive notes
        while (true)
        {
            if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3) Looking at note #" << i << std::endl;
            
            // ---- search for consecutive notes
            if (i + 1 < visibleNoteAmount)
            {
                ASSERT_E(i+1, <, (int)m_note_render_info.size());
                start_tick_of_next_note = m_note_render_info[i+1].getTick();
            }

            if (not (i<visibleNoteAmount)) break;

            // if notes are consecutive
            if (start_tick_of_next_note != -1 and
                aboutEqual_tick(start_tick_of_next_note,
                                m_note_render_info[i].getTick() + m_note_render_info[i].getTickLength(),
                                beatLen)
                )
            {
                if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3)    consecutive\n";
            }
            else
            {
                // notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
                if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3) } // serie ends here :  NOT (no more) consecutive\n";
            }
            
            if (first_triplet != -1)
            {
                if (i == (int)m_note_render_info.size()-1 or (not m_note_render_info[i+1].m_triplet) or
                    (i - first_triplet >= 2 and first_triplet != -1) )
                {
                    if (VERBOSE_ABOUT_TRIPLETS)
                    {
                        std::cout << "(3) } // serie ends here : "
                                  << (not m_note_render_info[i+1].m_triplet ? "next is no triplet " : "")
                                  << ((i-first_triplet>=2 and first_triplet != -1) ? "We've had 3 in a row. " : "")
                                  << "\n";
                    }
                    
                    last_of_a_serie = true;
                }
            }

            // do not cross measures
            if (i + 1 < visibleNoteAmount)
            {
                measure = m_note_render_info[i+1].m_measure_begin;
                if (measure != previous_measure)
                {
                    if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3) } // serie ends here : crossing a measure bar\n";

                    last_of_a_serie = true;
                }
                previous_measure = measure;
            }

            if (m_note_render_info[i].m_chord)
            {
                if (m_note_render_info[i].m_min_chord_level < min_level)
                {
                    min_level = m_note_render_info[i].m_min_chord_level;
                }
                if (m_note_render_info[i].m_max_chord_level > max_level)
                {
                    max_level = m_note_render_info[i].m_max_chord_level;
                }
            }
            else
            {
                const int level = m_note_render_info[i].getLevel();
                if (level < min_level) min_level = level;
                if (level > max_level) max_level = level;
            }

            // ---- ... and triplet notes
            is_triplet = m_note_render_info[i].m_triplet;
            if (is_triplet and first_triplet == -1)
            {
                if (VERBOSE_ABOUT_TRIPLETS) std::cout << "(3) { // starting triplet serie\n";
                first_triplet = i;
                
                // since it's the first note in this triplet series, it's both the min and max
                const int level = m_note_render_info[i].getLevel();
                min_level = level;
                max_level = level;
            }

            // this note is a triplet, but not the next, so time to do display the triplets sign
            // also triggered if we've had 3 triplet notes in a row, because triplets come by groups of 3...
            if (last_of_a_serie)
            {
                if (is_triplet)
                {
                    if (VERBOSE_ABOUT_TRIPLETS)
                    {
                        std::cout << "(3) == Binding triplet [" << first_triplet << " .. " << i << "] ==\n";
                    }
                    
                    // if nothing found (most likely meaning we only have one triplet note alone)
                    // use values from the first then.
                    if (min_level == 999)  min_level = m_note_render_info[first_triplet].getLevel();
                    if (max_level == -999) max_level = m_note_render_info[first_triplet].getLevel();

                    int mid_level = (int)round( (min_level + max_level)/2.0 );

                    m_note_render_info[first_triplet].m_triplet_show_above = (mid_level < m_stem_pivot);

                    if (i != first_triplet) // if not a triplet note alone, but a 'serie' of triplets
                    {
                        // fix all note stems so they all point in the same direction
                        for (int j=first_triplet; j<=i; j++)
                        {
                            //m_note_render_info[j].stem_type = ( m_note_render_info[first_triplet].triplet_show_above ? STEM_DOWN : STEM_UP );
                            m_note_render_info[j].m_draw_triplet_sign = false;
                        }
                    }
                    else
                    {
                        // this is either a triplet alone or a chord... just use the orientation that it already has
                        m_note_render_info[first_triplet].m_triplet_show_above =
                            (m_note_render_info[first_triplet].m_stem_type == STEM_DOWN);
                    }

                    if (m_note_render_info[first_triplet].m_triplet_show_above)
                    {
                        m_note_render_info[first_triplet].m_triplet_arc_level = min_level;
                    }
                    else
                    {
                        m_note_render_info[first_triplet].m_triplet_arc_level = max_level;
                    }

                    m_note_render_info[first_triplet].m_draw_triplet_sign = true;
                    m_note_render_info[first_triplet].m_triplet_arc_tick_end = m_note_render_info[i].getTick();
                }

                // reset search for triplets
                first_triplet = -1;
                min_level = 999;
                max_level = -999;
            }

            if (last_of_a_serie) break;
            else i++;

        }//wend

    }//next

}

// -----------------------------------------------------------------------------------------------------------

void ScoreAnalyser::processNoteBeam()
{
    const int visibleNoteAmount = m_note_render_info.size();
    const int beatLen = m_editor->getSequence()->ticksPerBeat();
    
    // beaming
    // all beam information is stored in the first note of the serie.
    // all others have their flags removed
    // BeamGroup objects are used to ease beaming

    for (int i=0; i<visibleNoteAmount; i++)
    {
        int start_tick_of_next_note = -1;

        int flag_amount = m_note_render_info[i].m_flag_amount;
        int first_of_serie = i;
        bool last_of_a_serie = false;

        int measure = m_note_render_info[i].m_measure_begin;
        int previous_measure = measure;

        // check for consecutive notes
        while (true)
        {
            if (i + 1 < visibleNoteAmount)
            {
                start_tick_of_next_note = m_note_render_info[i+1].getTick();
            }

            if (not (i < visibleNoteAmount)) break;

            // if notes are consecutive and of same length
            if (start_tick_of_next_note != -1 and
               aboutEqual_tick(start_tick_of_next_note, m_note_render_info[i].getTick() + m_note_render_info[i].getTickLength(), beatLen) and
               m_note_render_info[i+1].m_flag_amount == flag_amount and flag_amount > 0 and
               m_note_render_info[i+1].m_triplet == m_note_render_info[i].m_triplet);
            else
            {
                //notes are no more consecutive. it is likely a special action will be performed at the end of a serie
                last_of_a_serie = true;
            }

            // do not cross measures
            if (i + 1 < visibleNoteAmount)
            {
                measure = m_note_render_info[i+1].m_measure_begin;
                if (measure != previous_measure) last_of_a_serie = true;
                previous_measure = measure;
            }

            // it's the last of a serie, perform actions
            if (last_of_a_serie)
            {
                if (i > first_of_serie)
                {
                    BeamGroup beam(this, first_of_serie, i);
                    beam.doBeam(m_note_render_info, dynamic_cast<ScoreEditor*>(m_editor));
                }

                // reset
                first_of_serie = -1;
                break;
            }

            i++;

        }//wend

    }//next

}
    
// -----------------------------------------------------------------------------------------------------------
