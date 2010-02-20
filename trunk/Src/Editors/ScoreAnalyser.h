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

#ifndef _scoreanalyser_
#define _scoreanalyser_

/*
 * ScoreAnalyser contains what is necessary to decide how to render the score. Editors like Keyboard or Drum draw all notes
 * seperately from each other, so there rendering is simple and can be done in a single render pass. Score, however,
 * features much more complex interaction. For instance, draw a single triplet sign for 3 consecutive triplets, beam notes,
 * draw chords correctly, etc. The first render pass of the score editor is where the head of each note is rendered.
 * In this pass, a NoteRenderInfo object is added to a vector for each visible note. These objects contain basic information
 * on each note. The vector of visible notes is then passed to the ScoreAnalyser functions. These functions will analyse notes
 * and modify the NoteRenderInfo objects as needed so that they render nicely in the next rendering passes of the score editor.
 */


#include <vector>

namespace AriaMaestosa
{

enum STEM
{
    STEM_UP,
    STEM_DOWN,
    STEM_NONE
};

/**
 *  Contains info about a single visible note. A vector of these objects is created inthe first rendering pass.
 *  This vector contains one of these for each visible note. This vector is then analysed and used in the
 *  next rendering passes. The object starts with a few info fields, passed in the constructors, and builds/tweaks
 *  the others as needed in the next passes. The vector is destroyed and recreated with each render.
 *
 *  A few utility methods will ease setting some variables, but they are usually changed directly from code.
 */
    // FIXME: make less of these public!
class NoteRenderInfo
{
    // used to display ties (display a tie between this note and specified tick). a value of -1 means no tie.
    int tied_with_tick;
    bool tie_up; // used if stem_type == STEM_NONE, otherwise tie location is determined with stem_type

    int y;
public:
    /** for very short notes, e.g. drum notes. Note will appear as a X. */
    bool instant_hit;

    /** if note has a dot */
    bool dotted;

    /** Whether this "note" is a chord */
    bool chord;
    
    /** Since a chord contains many notes, keep info about the highest and lowest note of the chord
      * Only valid if 'chord' is true. */
    int min_chord_level, max_chord_level;

    /** Number of flags on note stem. e.g. a 1/4 will have none, a 1/8 has 1, a 1/16 has 2, etc. */
    int flag_amount;

    /** Whether this note has a hollow head (like 1/1 and 1/2 figures) or a "black" (filled) head */
    bool hollow_head;

    /** Whether this note is selected in the score editor */
    bool selected;

    /** is stem up, down? or is there no stem? */
    STEM stem_type;

    /** Whether to draw the stem. FIXME : stem_type == STEM_NONE can already carry this info!! */
    bool draw_stem;

    /** location and duration of note */
    int tick, tick_length;
    
    /** vertical position of the note, in abstract level units */
    int level;
    
    /** pitch ID of the note */
    int pitch;

    /** measure where the note begins and ends */
    int measureBegin, measureEnd;

    /** which alteration sigh to use, if any (sharp, flat, natural or none) */
    PitchSign sign;

    /** Whether this note has a triplet duration */
    bool triplet;
    
    /** Whether this particular note is responsible to draw a "triplet arc" with a 3 in it
      * (this is a separate options because many notes (e.g. 3) can share the same triplet arc)
      */
    bool draw_triplet_sign;
    
    /** where to display the "triplet arc" than contains a "3" */
    int triplet_arc_tick_start, triplet_arc_tick_end, triplet_arc_level;

    /** Whether the "triplet arc" is rendered above or below its Y base coordinate */
    bool triplet_show_above;
    
    // beams
    // FIXME - is beam_show_above really necessary, since it's always the same direction as stem_type?
    bool beam_show_above, beam;
    // if beam is true, the renderer will draw a beam between the end of this note's stem and the
    // location specified by these variables.
    int beam_to_tick;
    PitchSign beam_to_sign; // sign of the note we beam to. (used for printing where it's not linear)
    float beam_to_level;
    
    /**
      * If != -1, the renderer will use this y as stem end instead of calculating it itself.
      * Use ScoreAnalyser::getStemTo for a higher-level getter
      */
    float stem_y_level;


    NoteRenderInfo(int tick, int level, int tick_length, PitchSign sign, const bool selected, int pitch);

    void tieWith(NoteRenderInfo& renderInfo);
    void tieWith(const int tick);
    int getTiedToTick();
    void setTieUp(const bool up);
    bool isTieUp();

    void setTriplet();

    // those two will be the same for non-chords.
    int getBaseLevel();
    int getStemOriginLevel();

    /**
      * In an attempt t be view-independant, ScoreAnalyser tries to store Y locations as levels
      * and never as direct coordinates. However, to avoid the overhead of converting frequently
      * from level to coordinate, the renderer is given the option to store the Y coordinate
      * inside the noteRenderInfo.
      */
    const int getY() const { return y; }
    
    /**
      * In an attempt t be view-independant, ScoreAnalyser tries to store Y locations as levels
      * and never as direct coordinates. However, to avoid the overhead of converting frequently
      * from level to coordinate, the renderer is given the option to store the Y coordinate
      * inside the noteRenderInfo.
      */
    void setY(const int newY);
};

typedef void(*RenderSilenceCallback)(const int, const int, const int, const int, const bool, const bool, const int, const int);

class BeamGroup;
class ScoreAnalyser
{
    friend class BeamGroup;

    ScoreEditor* editor;
    int stemPivot;

    float stem_height;
    float min_stem_height;
public:
    LEAK_CHECK();

    std::vector<NoteRenderInfo> noteRenderInfo;

    ScoreAnalyser(ScoreEditor* parent, int stemPivot);

    /**
      * Returns a new ScoreAnalyser, that contains a subset of the current one.
      * The returned pointer must be freed.
      */
    ScoreAnalyser* getSubset(const int fromTick, const int toTick);

    float getStemTo(NoteRenderInfo& note);

    /** call when you're done rendering the current frame, to prepare to render the next */
    void clearAndPrepare();

    void addToVector( NoteRenderInfo& renderInfo, const bool recursion );

    /**
      * @brief the main function of ScoreAnalyser, where everything start
      * 
      * This function takes a vector containing information about visible notes.
      * Its job is to analyse them and fill missing data in the contained objects
      * so that they can be rendered correctly on a score.
      */
    void analyseNoteInfo();

    /** set the level below which the stem is up, and above which it is down */
    void setStemPivot(const int level);

    void renderSilences(RenderSilenceCallback renderSilenceCallback,
                                       const int first_visible_measure, const int last_visible_measure,
                                       const int silences_y);
protected:
    // internal methods performing different steps in score analysis
    
    /**
      * Puts notes in time order.
      * Notes that have no stems go last so that they don't disturb note grouping in chords.
      */
    void putInTimeOrder();
    void findAndMergeChords();
    void processTriplets();
    void processNoteBeam();
};

/*
 * returns whether two values are approximately equal. this is because there is no midi
 * standard for note length and i have seen some midis use note lengths slightly different
 * from those Aria uses. This is why i check for approximate lengths, otherwise the score
 * view may end up messed up when you import a midi made in another editor
 */
bool aboutEqual(const float float1, const float float2);
bool aboutEqual_tick(const int int1, const int int2);

}

#endif
