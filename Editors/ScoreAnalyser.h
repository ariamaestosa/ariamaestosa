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
public:
    // for very short notes, e.g. drum notes. Note will appear as a X.
	bool instant_hit;
    
    // if note has a dot
	bool dotted;
    
    // a 1/4 will have none, a 1/8 has 1, a 1/16 has 2, etc.
	int flag_amount;
    
    bool hollow_head;
    
	bool selected;
    
    // used to display ties (display a tie between this note and specified X coord). a value of -1 means no tie.
	int tied_with_x;
    bool tie_up; // used if stem_type == STEM_NONE, otherwise tie location is determined with stem_type
    
    // is stem up, down? or is there no stem?
	STEM stem_type;
    
    // sould we draw the stem?
    bool draw_stem;
    
    // location and duration of note
	int tick, tick_length;
	int x, y, level;
    int pitch;
    
    // measure where the note begins and ends
    int measureBegin, measureEnd;
    
    // sharp, flat, natural, none
	int sign;
	
	// triplets
	bool triplet_show_above, triplet, drag_triplet_sign;
	int triplet_arc_x_start, triplet_arc_x_end, triplet_arc_y; // where to display the "triplet arc" than contains a "3"
	
    // beams
    // FIXME - is beam_show_above really necessary, since it's always the same direction as stem_type?
    bool beam_show_above, beam;
    // if beam is true, the renderer will draw a beam between the end of this note's stem and the
    // location specified by these variables.
    int beam_to_x, beam_to_y;
    int stem_y; // if != -1, the renderer will use this y as stem end instead of calculating it itself
    
    // chord
    bool chord;
    // since a chord contains many notes, keep info about the highest and lowest note of the chord
    int max_chord_y, min_chord_y, min_chord_level, max_chord_level;

	NoteRenderInfo(int tick, int x, int level, int tick_length, int sign, const bool selected, int pitch);
	void tieWith(NoteRenderInfo& renderInfo);
	void triplet_arc(int pixel1, int pixel2);
    void setTriplet();
    
    int getStemX();
    int getStemYFrom();
    int getStemYTo();
    int getYBase();
};

void addToVector( NoteRenderInfo& renderInfo, std::vector<NoteRenderInfo>& info, const int middleCLevel, const bool recursion );

// the main function of ScoreAnalyser, where everything starts
void analyseNoteInfo( std::vector<NoteRenderInfo>& info, ScoreEditor* editor );

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
