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

namespace AriaMaestosa {

enum STEM
{
	STEM_UP,
	STEM_DOWN,
    STEM_BEAM,
	STEM_NONE
};

/*
 *  Contains info about a single visible. A vector of these objects is created inthe first rendering pass.
 *  This vector contains one of these for each visible note. This vector is then analysed and used in the
 *  next rendering passes. The object starts with a few info fields, passed in the constructors, and builds/tweaks
 *  the others as needed in the next passes. The vector is destroyed and recreated with each render.
 *
 *  A few utility methods will ease setting some variables, but they are usually changed directly from code.
 */
class NoteRenderInfo
{
    // used to display ties (display a tie between this note and specified tick). a value of -1 means no tie.
	int tied_with_tick;
    bool tie_up; // used if stem_type == STEM_NONE, otherwise tie location is determined with stem_type
    
    int y;
public:
    // for very short notes, e.g. drum notes. Note will appear as a X.
	bool instant_hit;
    
    // if note has a dot
	bool dotted;
    
    // chord
    bool chord;
    // since a chord contains many notes, keep info about the highest and lowest note of the chord
    int min_chord_level, max_chord_level;
    
    // a 1/4 will have none, a 1/8 has 1, a 1/16 has 2, etc.
	int flag_amount;
    
    bool hollow_head;
    
	bool selected;
    
    // is stem up, down? or is there no stem?
	STEM stem_type;
    
    // sould we draw the stem?
    // FIXME - doesn't that override stem_type == STEM_NONE ???
    bool draw_stem;
    
    // location and duration of note
	int tick, tick_length;
	int x, level;
    int pitch;
    
    // measure where the note begins and ends
    int measureBegin, measureEnd;
    
    // sharp, flat, natural, none
	int sign;
	
	// triplets
	bool triplet_show_above, triplet, drag_triplet_sign;
	int triplet_arc_x_start, triplet_arc_x_end, triplet_arc_level; // where to display the "triplet arc" than contains a "3"
	
    // beams
    // FIXME - is beam_show_above really necessary, since it's always the same direction as stem_type?
    bool beam_show_above, beam;
    // if beam is true, the renderer will draw a beam between the end of this note's stem and the
    // location specified by these variables.
    int beam_to_x, beam_to_level;
    float stem_y_level; // if != -1, the renderer will use this y as stem end instead of calculating it itself


	NoteRenderInfo(int tick, int x, int level, int tick_length, int sign, const bool selected, int pitch);
    
	void tieWith(NoteRenderInfo& renderInfo);
    void setTiedToTick(const int pixel);
    int getTiedToPixel();
    int getTiedToTick();
    void setTieUp(const bool up);
    bool isTieUp();
    
	void triplet_arc(int pixel1, int pixel2);
    void setTriplet();
    
    int getBaseLevel();
    const int getY() const;
    void setY(const int newY);
};

class BeamGroup;
class ScoreAnalyser
{
    friend class BeamGroup;
    
    ScoreEditor* editor;
    int stemPivot;
    
    int stem_up_x_offset;
    float stem_up_y_offset;
    int stem_down_x_offset;
    float stem_down_y_offset;
    float stem_height;
    float min_stem_height;
public:
    std::vector<NoteRenderInfo> noteRenderInfo;
    
    ScoreAnalyser(ScoreEditor* parent, int stemPivot);
    
    void setStemSize( const int stem_up_x_offset,
                      const float stem_up_y_offset,
                      const int stem_down_x_offset,
                      const float stem_down_y_offset,
                      const float stem_height = -1,
                      const float min_stem_height = -1);
    
    int getStemX(NoteRenderInfo& note);
    float getStemFrom(NoteRenderInfo& note);
    float getStemTo(NoteRenderInfo& note);
    
    // you're done rendering the current frame, prepare to render the next
    void clearAndPrepare();
    
    void addToVector( NoteRenderInfo& renderInfo, const bool recursion );

    // the main function of ScoreAnalyser, where everything starts
    void analyseNoteInfo();

    // set the level below which the stem is up, and above which it is down
    void setStemPivot(const int level);
    
    void renderSilences( void (*renderSilenceCallback)(const int, const int, const int),
                         const int first_visible_measure, const int last_visible_measure,
                         const int silences_y );
protected:
    // fon't call these, 'analyseNoteInfo' will
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
