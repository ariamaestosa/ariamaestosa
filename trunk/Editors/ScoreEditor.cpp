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
#include "Editors/RelativeXCoord.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/KeyPicker.h"
#include "Images/ImageProvider.h"
#include "Images/Drawable.h"
#include "GUI/MeasureBar.h"
#include "GUI/RenderUtils.h"

#include "Actions/EditAction.h"
#include "Actions/AddNote.h"

#include "AriaCore.h"

#include <string>



/*
 * Explainations:
 *
 *		'level'		describes Y position of note, ranges from 0 (highest pitch) to 73 (lowest pitch)
 *					Each level represents and Y postion a note can take.
 *					A black line on the score is a level. A white space between 2 black lines is a level.
 *					White spaces above and below the score are made of levels too.
 *					The height of a level in pixels is defined by 'int y_step'.
 *
 *		'note_12'	means A=0, A#=1, B=2, C=3, C#=4, etc.
 *
 *		'note_7'	means A=0, B=1, C=2, D=3, etc.
 *
 */

namespace AriaMaestosa {
	
// will contain half the height of a note 'main circle', this will be used for centering the note on the string
int halfh = -1;
	
// height in pixels of each level
const int y_step = 5;

/*
 * When you switch to a key other C,flat or sharp signs need to appear next to the G and F keys.
 * These arrays give the X position relative to the left of the key of a flat or sharp sign for each note,
 * where the index of the array is of type note_7.
 */
const int sign_dist = 5;
int sharp_sign_x[] = { 5*sign_dist, 7*sign_dist, 2*sign_dist, 4*sign_dist, 6*sign_dist, 1*sign_dist, 3*sign_dist };

int flat_sign_x[] = { 3*sign_dist, 1*sign_dist, 6*sign_dist, 4*sign_dist, 2*sign_dist, 7*sign_dist, 5*sign_dist };

// FIXME helper func, delete later
const char* note_name[] = {"A","B","C","D","E","F","G"};
void printNoteName(int value)
{
		// enter default value
		if(value!=-1)
		{
			
			const int octave=10 - (value/12);
			const int note=value%12;
			
			std::cout << octave << " : ";
			
			switch(note){
				case 0:
					std::cout << "B";
					std::cout << " " << std::endl;
					break;
				case 1:
					std::cout << "A# aka Bb" << std::endl;
					break;
				case 2:
					std::cout << "A";
					std::cout << " " << std::endl;
					break;
				case 3:
					std::cout << "G# aka Ab" << std::endl;
					break;
				case 4:
					std::cout << "G";
					std::cout << " " << std::endl;
					break;
				case 5:
					std::cout << "F# aka Gb" << std::endl;
					break;
				case 6:
					std::cout << "F";
					std::cout << " " << std::endl;
					break;
				case 7:
					std::cout << "E";
					std::cout << " " << std::endl;
					break;
				case 8:
					std::cout << "D# aka Eb"<< std::endl;
					break;
				case 9:
					std::cout << "D";
					std::cout << " " << std::endl;
					break;
				case 10:
					std::cout << "C# aka Db" << std::endl;
					break;
				case 11:
					std::cout << "C";
					std::cout << " " << std::endl;
					break;
			} // end switch
			
			
		}
		
}


int findNotePitch(int note_7, int sharpness)
{
	int note=0;
	
	if(note_7==0) note+=2; // A
	else if(note_7==1) note+=0; // B
	else if(note_7==2) note+=11; // C
	else if(note_7==3) note+=9; // D
	else if(note_7==4) note+=7; // E
	else if(note_7==5) note+=6; // F
	else if(note_7==6) note+=4; // G
	else
	{
		std::cerr << "Invalid note: " << note_7 << std::endl;
		return 0;
	}
	
	if(sharpness==SHARP) note-=1;
	else if(sharpness==FLAT) note+=1;
	
	return note;
}

#pragma mark -

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//											ScoreMidiConverter
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ScoreMidiConverter::ScoreMidiConverter()
{
    INIT_LEAK_CHECK();
	for(int n=0; n<7; n++) scoreNotesSharpness[n] = NATURAL;
	
    going_in_sharps = false;
    going_in_flats = false;
}

void ScoreMidiConverter::setNoteSharpness(NOTES note, int sharpness)
{

	scoreNotesSharpness[note] = sharpness;
	
	if(sharpness == SHARP)
	{
		going_in_sharps = true;
		going_in_flats = false;
	}
	else if(sharpness == FLAT)
	{
		going_in_sharps = false;
		going_in_flats = true;
	}
}

// are we using a key that will make flat signs appear next to the key?
bool ScoreMidiConverter::goingInSharps()
{
	return going_in_sharps;
}
// are we using a key that will make sharp signs appear next to the key?
bool ScoreMidiConverter::goingInFlats()
{
	return going_in_flats;
}
// what sign should appear next to the key for this note? (FLAT, SHARP or NONE)
int ScoreMidiConverter::getKeySigSharpnessSignForLevel(const unsigned int level)
{
	assertExpr(level,<,73);
	return scoreNotesSharpness[ levelToNote7(level) ];
}

int ScoreMidiConverter::getMiddleCLevel() { return middleCLevel; }

/*
// with the current key, what sign must be shown next to note if we want it to have given pitch? (FLAT, SHARP, NATURAL or NONE)
int ScoreMidiConverter::getSharpnessSignForMidiNote(const unsigned int note)
{
	return showSignNextToNote[ note ];
}
*/

// returns what note is at given level
int ScoreMidiConverter::levelToNote(const int level)
{
    if(level<0 or level>=73) return -1;
	return levelToMidiNote[level];
}

// returns on what level the given note will appear, and with what sign
int ScoreMidiConverter::noteToLevel(Note* noteObj, int* sign)
{
    const int note = noteObj->pitchID;
    if(note>=128 or note<0) return -1;
    
    const int level = midiNoteToLevel[note];
    const NoteToLevelType current_type = midiNoteToLevel_type[note];
    
    if(current_type == SHARP_OR_FLAT)
    {
        // decide whether to use a flat or a sharp to display this note
        // first check if there's a user-specified sign. otherwise pick default (quite arbitrarly)
        bool useFlats = false;
        if(noteObj->preferred_accidental_sign != -1)
        {
            if(noteObj->preferred_accidental_sign == SHARP) useFlats = false;
            else if(noteObj->preferred_accidental_sign == FLAT) useFlats = true;
        }
        else
            useFlats = goingInFlats();
        
        if(useFlats and (note-1)>0)
        {
            if(sign!=NULL) *sign = FLAT;
            return midiNoteToLevel[note-1];
        }
        else if(note < 127)
        {
            if(sign!=NULL) *sign = SHARP;
            return midiNoteToLevel[note+1];
        }
        else return -1; // nothing found  
    }
    else if(current_type == NATURAL_ON_LEVEL)
    {
        if(sign!=NULL) *sign = NATURAL;
        return level;
    }
    else if(current_type == DIRECT_ON_LEVEL)
    {
        if(sign!=NULL) *sign = NONE;
        return level;
    }
    else return -1; // nothing found
}

// what is the name of the note played on this level?
int ScoreMidiConverter::levelToNote7(const unsigned int level)
{
	int r = 7-level%7;
	if(r < 0) r+=7;
	if(r > 6) r-=7;
	return r;
}

// called when key has changed, rebuilds conversion tables and other
// data needed for all conversions and information requests this class provides
void ScoreMidiConverter::updateConversionData()
{
    
    // FIXME - mess
    
	// reset all
	for(int n=0; n<128; n++)
	{
		midiNoteToLevel[n] = -1;
        midiNoteToLevel_type[n] = SHARP_OR_FLAT;
	}
	
	middleCLevel = -1;
	
	int middleCNote128 = 71;
	if( scoreNotesSharpness[C] == SHARP ) middleCNote128 -= 1;
	else if( scoreNotesSharpness[C] == FLAT ) middleCNote128 += 1;
	
	// do levelToMidiNote first
	int note_7 = 0, octave = 0;
	for(int n=0; n<73; n++)
	{
		const int sharpness = scoreNotesSharpness[note_7];
		
		levelToMidiNote[n] = findNotePitch( note_7, sharpness ) + octave*12;
		
		assertExpr(levelToMidiNote[n],<,128);
		assertExpr(levelToMidiNote[n],>,-1);
		
		midiNoteToLevel[ levelToMidiNote[n] ] = n;
		midiNoteToLevel_type[ levelToMidiNote[n] ] = DIRECT_ON_LEVEL;
        
		// find middle C
		if( levelToMidiNote[n] == middleCNote128 ) middleCLevel = n; // set on which line is middle C
		
		// if note is flat or sharp, also find what this line would be with a natural sign
		if(sharpness != NATURAL)
		{
			const int natural_note_on_this_line = findNotePitch( note_7, NATURAL ) + octave*12;
			
            // FIXME - it may not be necessary to fill it again every time
            levelToNaturalNote[n] = natural_note_on_this_line;
            
			// only use natural signs if this note cannot be displayed without natural sign on another line
			// in wich case 'midiNoteToLevel' will have been set (or will be overwritten later)
			if(midiNoteToLevel[ natural_note_on_this_line ] == -1)
			{
				midiNoteToLevel[ natural_note_on_this_line ] = n;
				midiNoteToLevel_type[ natural_note_on_this_line ] = NATURAL_ON_LEVEL;
			}
		}
        else
        {
            // FIXME - it may not be necessary to fill it again every time
            levelToNaturalNote[n] = levelToMidiNote[n];
        }
		
		note_7 --;
		if(note_7 == 1) octave++;
		
		if(note_7 < 0)
			note_7 = 6;
	}
	
	
    /*
	// fill any midi note that has not been assigned a level
    const bool useFlats = goingInFlats();
    if(useFlats)
    {
        for(int n=1; n<128; n++)
        {
            if(midiNoteToLevel[n] == -1)
            {   
                midiNoteToLevel[n] = midiNoteToLevel[n-1];
                showSignNextToNote[n] = FLAT;
            }
        }
    }
    else
    {
        for(int n=0; n<127; n++)
        {
            if(midiNoteToLevel[n] == -1)
            {
                midiNoteToLevel[n] = midiNoteToLevel[n+1];
                showSignNextToNote[n] = SHARP;
            }
        }//next
    }
     */
	
}

int ScoreMidiConverter::getMidiNoteForLevelAndSign(const unsigned int level, int sharpness)
{
    if(level < 0 or level > 73) return -1;
    
    if(sharpness == NONE) return levelToNote(level);
    else if(sharpness == NATURAL) return levelToNaturalNote[level];
    else if(sharpness == SHARP) return levelToNaturalNote[level]-1;
    else if(sharpness == FLAT) return levelToNaturalNote[level]+1;
    else return -1; // shouldn't happen
}

#pragma mark -

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ScoreEditor::ScoreEditor(Track* track) : Editor(track)
{
	GKey = true;
	FKey = true;
	converter = new ScoreMidiConverter();
	
	musicalNotationEnabled=true;
	linearNotationEnabled=true;
	
	converter->updateConversionData();	
	
	setYStep( y_step );
	
	sb_position = converter->getMiddleCLevel() / 73.0;
}
ScoreEditor::~ScoreEditor()
{
	delete converter;
}

void ScoreEditor::enableMusicalNotation(const bool enabled)
{
	musicalNotationEnabled = enabled;
}

void ScoreEditor::enableLinearNotation(const bool enabled)
{
	linearNotationEnabled = enabled;
}

// order in wich signs of the key signature appear
const NOTES sharp_order[] = { F, C, G, D, A, E, B };
const NOTES flat_order[] = { B, E, A, D, G, C, F };

// where parameters are e.g. 5 sharps, 3 flats, etc.
void ScoreEditor::loadKey(const int sharpness_symbol, const int symbol_amount)
{
	// reset key signature before beginning
	converter->setNoteSharpness(A, NATURAL);
	converter->setNoteSharpness(B, NATURAL);
	converter->setNoteSharpness(C, NATURAL);
	converter->setNoteSharpness(D, NATURAL);
	converter->setNoteSharpness(E, NATURAL);
	converter->setNoteSharpness(F, NATURAL);
	converter->setNoteSharpness(G, NATURAL);
	
    key_flats_amnt = 0;
    key_sharps_amnt = 0;
    
	if(sharpness_symbol == SHARP)
	{
        key_sharps_amnt = symbol_amount;
        
		for(int n=A; n<symbol_amount; n++)
		{
			converter->setNoteSharpness( sharp_order[n], sharpness_symbol);
		}
	}
	else if(sharpness_symbol == FLAT)
	{
        key_flats_amnt = symbol_amount;
        
		for(int n=A; n<symbol_amount; n++)
		{
			converter->setNoteSharpness( flat_order[n], sharpness_symbol);
		}
	}
	
	converter->updateConversionData();
}

ScoreMidiConverter* ScoreEditor::getScoreMidiConverter()
{
	return converter;
}

// user clicked on a sign in the track's header
void ScoreEditor::setNoteSign(const int sign, const int noteID)
{
    
    const int noteLevel = converter->noteToLevel(track->getNote(noteID));
    
    const int new_pitch = converter->getMidiNoteForLevelAndSign(noteLevel, sign);
    if(new_pitch == -1)
    {
        wxBell();
        return;
    }
    
    Note* note = track->getNote(noteID);
    note->pitchID = new_pitch;
    
    if(sign != NATURAL) note->preferred_accidental_sign = sign;
    
}

#pragma mark -
// where 'renderInfo' is a 'NoteRenderInfo' object of current note.
// 'vector' is where all visible notes will are added, to be analysed after
void ScoreEditor::renderNote_pass1(NoteRenderInfo& renderInfo, std::vector<NoteRenderInfo>& vector, const bool recursion)
{
    
    AriaRender::lineWidth(2);
	if(renderInfo.selected)
        AriaRender::color(1,0,0);
	else
        AriaRender::color(0,0,0);	

	renderInfo.y = getEditorYStart() + y_step*renderInfo.level - halfh - getYScrollInPixels() + 2;
	
	// check if note lasts more than one measure. If so we need to divide it in 2.
	const int measureBegin = getMeasureBar()->measureAtTick(renderInfo.tick);
	const int measureEnd = getMeasureBar()->measureAtTick(renderInfo.tick + renderInfo.tick_length - 1);
    
	if(measureEnd > measureBegin) // note in longer than mesaure, need to divide it in 2
	{
		const int firstEnd = getMeasureBar()->lastTickInMeasure(measureBegin);
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
		renderNote_pass1(part1, vector, true);
		NoteRenderInfo part2(getMeasureBar()->firstTickInMeasure(measureBegin+1), firstEndRel.getRelativeTo(WINDOW),
						   renderInfo.level, secondLength, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		renderNote_pass1(part2, vector, true);
		
		if(!recursion)
		{
			const int amount = vector.size();
			for(int i=initial_id+1; i<amount; i++)
			{
				vector[i].tieWith(vector[i-1]);
			}
		}
		
		return;
	}
	
    // find how to draw notes. how many tails, dotted, triplet, etc.
    // if note duration is unknown it will be split
	const float relativeLength = renderInfo.tick_length / (float)(getMeasureBar()->beatLengthInTicks()*4);

	renderInfo.tail_type = (renderInfo.level>=converter->getMiddleCLevel()-5 ? TAIL_UP : TAIL_DOWN);
	if(relativeLength>=1) renderInfo.tail_type=TAIL_NONE; // whole notes have no tails
	bool open = false;
	
    const int beat = getMeasureBar()->beatLengthInTicks();
    const int tick_in_measure_start = renderInfo.tick - getMeasureBar()->firstTickInMeasure( measureBegin );
    const int remaining = beat - (tick_in_measure_start % beat);
    const bool starts_on_beat = aboutEqual(remaining,0) or aboutEqual(remaining,beat);
    
	if( aboutEqual(relativeLength, 1.0) ){ open = true; renderInfo.tail_type=TAIL_NONE; }
	else if( aboutEqual(relativeLength, 1.0/2.0) ){ open = true; } // 1/2
	else if( aboutEqual(relativeLength, 1.0/3.0) ){ renderInfo.setTriplet(); open = true; } // triplet 1/2
	else if( aboutEqual(relativeLength, 1.0/4.0) ); // 1/4
	else if( aboutEqual(relativeLength, 1.0/6.0) ){ renderInfo.setTriplet(); } // triplet 1/4
	else if( aboutEqual(relativeLength, 1.0/8.0) ) renderInfo.subtail_amount = 1; // 1/8
	else if( aboutEqual(relativeLength, 1.0/12.0) ){ renderInfo.setTriplet(); renderInfo.subtail_amount = 1; } // triplet 1/8
	else if( aboutEqual(relativeLength, 1.0/16.0) ) renderInfo.subtail_amount = 2; // 1/16
	else if( aboutEqual(relativeLength, 1.0/24.0) ) { renderInfo.setTriplet(); renderInfo.subtail_amount = 2; } // triplet 1/16
	else if( aboutEqual(relativeLength, 1.0/32.0) ) renderInfo.subtail_amount = 3; // 1/32
	else if( aboutEqual(relativeLength, 3.0/4.0) and starts_on_beat){ renderInfo.dotted = true; open=true; } // dotted 1/2
	else if( aboutEqual(relativeLength, 3.0/8.0) and starts_on_beat ) renderInfo.dotted = true; // dotted 1/4
	else if( aboutEqual(relativeLength, 3.0/2.0) and starts_on_beat ){ renderInfo.dotted = true; open=true; } // dotted whole
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
            
            firstLength_tick = closestShorterDuration*(float)(getMeasureBar()->beatLengthInTicks()*4);
		}
        
        const int secondBeginning_tick = renderInfo.tick + firstLength_tick;
        RelativeXCoord secondBeginningRel(secondBeginning_tick, MIDI);
        
		int initial_id = -1;
		
		if(!recursion)
		{
			initial_id = vector.size();
		}
		
		NoteRenderInfo part1(renderInfo.tick, renderInfo.x, renderInfo.level, firstLength_tick, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		renderNote_pass1(part1, vector, true);
		NoteRenderInfo part2(secondBeginning_tick, secondBeginningRel.getRelativeTo(WINDOW), renderInfo.level,
						   renderInfo.tick_length-firstLength_tick, renderInfo.sign, renderInfo.selected, renderInfo.pitch);
		renderNote_pass1(part2, vector, true);
		
		if(!recursion)
		{
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
		renderInfo.triplet_arc_y = renderInfo.y;
	}
	
	assertExpr(renderInfo.level,>,-1);
	
	// main round
    /*
	if(renderInfo.unknown_duration)
	{
		AriaRender::primitives();
        AriaRender::character('?', renderInfo.x, renderInfo.y + 3);
	}
	else
     */
    if(renderInfo.instant_hit)
	{
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::character('X', renderInfo.x, renderInfo.y + 8);
	}
	else if(open)
	{
        AriaRender::images();
		noteOpen->move(renderInfo.x, renderInfo.y);
		noteOpen->render();
	}
	else
	{
		AriaRender::images();
		noteClosed->move(renderInfo.x, renderInfo.y);
		noteClosed->render();
	}
	
	// if note is above or below keys, we need to display small lines from the score
	// to the note so that the amount of levels is visible
    AriaRender::primitives();
    AriaRender::color(0,0,0);
    AriaRender::lineWidth(1);
	const int middle_c_level = converter->getMiddleCLevel();
	if(GKey and renderInfo.level < middle_c_level - 10)
	{
		for(int lvl=middle_c_level - 9; lvl>renderInfo.level+renderInfo.level%2; lvl -= 2)
		{
			const int lvly = getEditorYStart() + y_step*lvl - halfh - getYScrollInPixels() + 2;
            AriaRender::line(renderInfo.x-5, lvly, renderInfo.x+15, lvly);
		}
	}
	if(FKey and renderInfo.level > middle_c_level + 11)
	{
		for(int lvl=middle_c_level + 11; lvl<=renderInfo.level-renderInfo.level%2+2; lvl += 2)
		{
			const int lvly = getEditorYStart() + y_step*lvl - halfh - getYScrollInPixels() + 2;
			AriaRender::line(renderInfo.x-5, lvly, renderInfo.x+15, lvly);
		}
	}
    
	if(renderInfo.level == middle_c_level)
	{
		const int lvly = getEditorYStart() + y_step*(middle_c_level+1) - halfh - getYScrollInPixels() + 2;
		AriaRender::line(renderInfo.x-5, lvly, renderInfo.x+15, lvly);
	}
    
	AriaRender::lineWidth(2);
	
	// dotted
	if(renderInfo.dotted)
	{
        AriaRender::color(1,1,1);
        AriaRender::pointSize(5);
        AriaRender::point(renderInfo.x + 14, renderInfo.y + 5);

		if(renderInfo.selected) AriaRender::color(1,0,0);
		else AriaRender::color(0,0,0);
        AriaRender::pointSize(3);
        
        AriaRender::point(renderInfo.x + 14, renderInfo.y + 5);
	}
	
	// sharpness sign
	if(renderInfo.sign == SHARP)
	{
		AriaRender::images();
		sharpSign->move(renderInfo.x - 5, renderInfo.y + halfh);
		sharpSign->render();
        AriaRender::primitives();
	}
	else if(renderInfo.sign == FLAT)
	{
		AriaRender::images();
		flatSign->move(renderInfo.x - 5, renderInfo.y + halfh);
		flatSign->render();
        AriaRender::primitives();
	}
	else if(renderInfo.sign == NATURAL)
	{
		AriaRender::images();
		naturalSign->move(renderInfo.x - 5, renderInfo.y + halfh);
		naturalSign->render();
		AriaRender::primitives();
	}
	     
	vector.push_back(renderInfo);
}

void ScoreEditor::renderNote_pass2(NoteRenderInfo& renderInfo)
{
    AriaRender::primitives();
	if(renderInfo.selected)
        AriaRender::color(1,0,0);
	else
		AriaRender::color(0,0,0);	
	
	// tail
    if(renderInfo.draw_tail)
    {

        if(renderInfo.tail_type == TAIL_UP or renderInfo.tail_type == TAIL_DOWN)
        {
            AriaRender::line(renderInfo.getTailX(), renderInfo.getTailYFrom(),
                             renderInfo.getTailX(), renderInfo.getTailYTo());
        }
        
        
        // subtails
        if(renderInfo.subtail_amount>0 and not renderInfo.beam)
        {
            const int subtail_y_origin = (renderInfo.tail_type==TAIL_UP ? renderInfo.y - 24 : renderInfo.y + 17);
            const int subtail_x_origin = (renderInfo.tail_type==TAIL_UP ? renderInfo.x + 9 : renderInfo.x + 1);
            const int subtail_step = (renderInfo.tail_type==TAIL_UP ? 7 : -7 );
            
            noteTail->setFlip( false, renderInfo.tail_type!=TAIL_UP );
            
            AriaRender::images();
            for(int n=0; n<renderInfo.subtail_amount; n++)
            {
                noteTail->move( subtail_x_origin , subtail_y_origin + n*subtail_step);	
                noteTail->render();
            }
            AriaRender::primitives();
        }
    }
    
	// triplet
	if (renderInfo.drag_triplet_sign)
	{
		const int center_x = (renderInfo.triplet_arc_x_end == -1 ? renderInfo.triplet_arc_x_start : (renderInfo.triplet_arc_x_start + renderInfo.triplet_arc_x_end)/2);
		const int radius_x = (renderInfo.triplet_arc_x_end == -1 or  renderInfo.triplet_arc_x_end == renderInfo.triplet_arc_x_start ?
							  10 : (renderInfo.triplet_arc_x_end - renderInfo.triplet_arc_x_start)/2);
        AriaRender::arc(center_x, renderInfo.triplet_arc_y + (renderInfo.triplet_show_above ? 0 : 10), radius_x, 10, renderInfo.triplet_show_above);

        AriaRender::color(0,0,0);
        AriaRender::small_character('3', center_x-2, ( renderInfo.triplet_show_above? renderInfo.triplet_arc_y : renderInfo.triplet_arc_y+18));
	}
	
	// tie
	if(renderInfo.tied_with_x != -1)
	{
		const float center_x = (renderInfo.tied_with_x + renderInfo.x)/2.0 + 6;
		const float radius_x = (renderInfo.tied_with_x - renderInfo.x)/2.0;
        const bool show_above = (renderInfo.tail_type == TAIL_NONE ? renderInfo.tie_up : renderInfo.tail_type != TAIL_UP);
        
        const int base_y = renderInfo.getYBase(); 
        AriaRender::arc(center_x, base_y + (show_above ? 0 : 10), radius_x, 15, show_above);
	}
	
    // beam
    if(renderInfo.beam)
    {
        AriaRender::color(0,0,0);
        AriaRender::lineWidth(2);
        
        const int x1 = renderInfo.getTailX();
        int y1 = renderInfo.getTailYTo();
        int y2 = renderInfo.beam_to_y;
        
        const int y_diff = (renderInfo.tail_type == TAIL_UP ? 5 : -5);
        
        AriaRender::lineSmooth(true);
        for(int n=0; n<renderInfo.subtail_amount; n++)
        {
            AriaRender::line(x1, y1, renderInfo.beam_to_x, y2);
            y1 += y_diff;
            y2 += y_diff;
        }
        AriaRender::lineSmooth(false);
    }
}


void ScoreEditor::renderSilence(const int tick, const int tick_length)
{ 
    const int beat = getMeasureBar()->beatLengthInTicks();
    
    if(tick_length<2) return;
    
    // check if silence spawns over more than one measure
    const int end_measure = getMeasureBar()->measureAtTick(tick+tick_length-1);
    if(getMeasureBar()->measureAtTick(tick) != end_measure)
    {
        // we need to plit it in two
        const int split_tick = getMeasureBar()->firstTickInMeasure(end_measure);
        
        // Check split is valid before attempting.
        if(split_tick-tick>0 and tick_length-(split_tick-tick)>0)
        {
            renderSilence(tick, split_tick-tick);
            renderSilence(split_tick, tick_length-(split_tick-tick));
            return;
        }
    }
    
    assertExpr(tick,>,-1);
	RelativeXCoord relX(tick, MIDI);
	const int x = relX.getRelativeTo(WINDOW) + 5;
	const int y = getEditorYStart() + y_step*(converter->getMiddleCLevel()-8) - getYScrollInPixels() + 1;
	bool dotted = false, triplet = false;
	int type = -1;
	
	int dot_delta_x = 0, dot_delta_y = 0;
	
	const float relativeLength = tick_length / (float)(getMeasureBar()->beatLengthInTicks()*4);
	
	const int tick_in_measure_start = (tick) - getMeasureBar()->firstTickInMeasure( getMeasureBar()->measureAtTick(tick) );
    const int remaining = beat - (tick_in_measure_start % beat);
    const bool starts_on_beat = aboutEqual(remaining,0) or aboutEqual(remaining,beat);
    
	if( aboutEqual(relativeLength, 1.0) ) type = 1;
	else if (aboutEqual(relativeLength, 3.0/2.0) and starts_on_beat){ type = 1; dotted = true; dot_delta_x = 5; dot_delta_y = 2;}
	else if (aboutEqual(relativeLength, 1.0/2.0)) type = 2;
    else if (aboutEqual(relativeLength, 1.0/3.0)){ type = 2; triplet = true; }
	else if (aboutEqual(relativeLength, 3.0/4.0) and starts_on_beat){ type = 2; dotted = true; dot_delta_x = 5; dot_delta_y = 2;}
	else if (aboutEqual(relativeLength, 1.0/4.0)) type = 4;
    else if (aboutEqual(relativeLength, 1.0/6.0)){ type = 4; triplet = true; }
	else if (aboutEqual(relativeLength, 3.0/8.0) and starts_on_beat){ type = 4; dotted = true; dot_delta_x = -3; dot_delta_y = 10; }
	else if (aboutEqual(relativeLength, 1.0/8.0)) type = 8;
    else if (aboutEqual(relativeLength, 1.0/12.0)){ triplet = true; type = 8; }
	else if (aboutEqual(relativeLength, 3.0/16.0) and starts_on_beat){ type = 8; dotted = true; }
	else if (aboutEqual(relativeLength, 1.0/16.0)) type = 16;
	else if(relativeLength < 1.0/16.0){ return; }
	else
	{
		// silence is of unknown duration. split it in a serie of silences.
		
		// start by reaching the next beat if not already done
		if(!starts_on_beat and !aboutEqual(remaining,tick_length))
		{
            renderSilence(tick, remaining);
			renderSilence(tick+remaining, tick_length - remaining);
			return;
		}
		
		// split in two smaller halves. render using a simple recursion.
		float closestShorterDuration = 1;
		while(closestShorterDuration >= relativeLength) closestShorterDuration /= 2.0;
		
		const int firstLength = closestShorterDuration*(float)(getMeasureBar()->beatLengthInTicks()*4);

		renderSilence(tick, firstLength);
		renderSilence(tick+firstLength, tick_length - firstLength);
		return;
	}
	
	if( type == 1 )
	{
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x,y, x+15, y+y_step);
	}
	else if( type == 2 )
	{
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        AriaRender::rect(x, y+y_step, x+15, y+y_step*2);
	}
	else if( type == 4 )
	{
        AriaRender::images();
		silence4->move(x, y);
		silence4->render();
	}
	else if( type == 8 )
	{
		AriaRender::images();
		silence8->move(x-3, y);
		silence8->render();
	}
	else if( type == 16 )
	{
		AriaRender::images();
		silence8->move(x-7, y+3);
		silence8->render();
		silence8->move(x-5, y-3);
		silence8->render();
	}
	
    AriaRender::primitives();
    
	// dotted
	if(dotted)
	{
        AriaRender::color(1,1,1);
        AriaRender::pointSize(5);
        AriaRender::point(x + 14 + dot_delta_x, y + 5 + dot_delta_y);
        
        AriaRender::color(0,0,0);
        AriaRender::pointSize(3);
        AriaRender::point(x + 14 + dot_delta_x, y + 5 + dot_delta_y);
	}
    
    // triplet
    if(triplet)
    {
        AriaRender::arc(x+5, y + 25, 10, 10, false);
        
        AriaRender::color(0,0,0);
        AriaRender::small_character('3', x+3, y+31);
    }
	
}

void ScoreEditor::render(RelativeXCoord mousex_current, int mousey_current,
						 RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
	if(halfh == -1) halfh = noteOpen->getImageHeight()/2;

    if(!ImageProvider::imagesLoaded()) return;
    const int yscroll = getYScrollInPixels();
	
    // FIXME - isn't there a 'get track height' function?
    AriaRender::beginScissors(10, (20+height + from_y+barHeight+20), width-15, 20+height);

    // white background
    AriaRender::primitives();
    AriaRender::color(1,1,1);

	const int middle_c_level = converter->getMiddleCLevel();
	
	if(keyG)
		drawVerticalMeasureLines(getEditorYStart() + (middle_c_level-2)*y_step + y_step/2 - yscroll,
							 getEditorYStart() + (middle_c_level-10)*y_step + y_step/2 - yscroll);
	
	if(keyF)
		drawVerticalMeasureLines(getEditorYStart() + (middle_c_level+2)*y_step + y_step/2 - yscroll,
								 getEditorYStart() + (middle_c_level+10)*y_step + y_step/2 - yscroll);
    
    // ---------------------- draw notes ----------------------------
    AriaRender::color(0,0,0);
    AriaRender::pointSize(4);
    
    const int noteAmount = track->getNoteAmount();
	
	std::vector<NoteRenderInfo> gatheredNoteInfo;
	
    const int first_x_to_consider = getMeasureBar()->firstPixelInMeasure( getMeasureBar()->measureAtPixel(0) ) + 1;
    
	// render pass 1. draw linear notation if relevant, gather information and do initial rendering for musical notation
	for(int n=0; n<noteAmount; n++)
	{
		//const int notePitch = track->getNotePitchID(n);
        
        int note_sign;
		const int noteLevel = converter->noteToLevel(track->getNote(n), &note_sign);
        
        if(noteLevel == -1) continue;
		const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
		const int tick = track->getNoteStartInMidiTicks(n);
		
        int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();
        const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();
        
        // don't consider notes that won't be visible
        if(x2 < first_x_to_consider) continue;
        if(x1>width+15) break;
        
		if(linearNotationEnabled)
		{
			if(musicalNotationEnabled) x1 += 8;
			
			if(x1 < x2) // when notes are too short to be visible, don't draw them
			{
				float volume=track->getNoteVolume(n)/127.0;

				// draw the quad with black border that is visible in linear notation mode
                AriaRender::primitives();
                if(track->isNoteSelected(n) and focus)
                    AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
				else
                    AriaRender::color((1-volume*0.7), (1-volume*0.7), 1);
                
                const int y1 = noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels()-1;
                const int y2 = (noteLevel+1)*y_step + getEditorYStart() - getYScrollInPixels()-1;
                    
                if(musicalNotationEnabled)
                    AriaRender::bordered_rect_no_start(x1+1, y1, x2-1, y2);
                else
                    AriaRender::bordered_rect(x1+1, y1, x2-1, y2);
                    
			}
			
			// when musical notation is disabled, we need to render the sharpness sign here
			// (when it's activated, it's done by the note drawing code. When note drawing code
			// is disabled, we need to do it ourselves).
			if(not musicalNotationEnabled)
			{
                AriaRender::images();
                
				if(note_sign == SHARP)
				{
					sharpSign->move(x1 - 5, noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
					sharpSign->render();
				}
				else if(note_sign == FLAT)
				{
					flatSign->move(x1 - 5,  noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
					flatSign->render();
				}
				else if(note_sign == NATURAL)
				{
					naturalSign->move(x1 - 5,  noteLevel*y_step+1 + getEditorYStart() - getYScrollInPixels());
					naturalSign->render();
				}
                
                AriaRender::primitives();
			}
		}// end if linear
		
		if(musicalNotationEnabled)
		{
			// rendering pass 1 of notes
			const int note_x = getEditorXStart() + track->getNoteStartInPixels(n)  - sequence->getXScrollInPixels();
			NoteRenderInfo currentNote(tick, note_x, noteLevel, noteLength, note_sign,
									 track->isNoteSelected(n), track->getNotePitchID(n));
			renderNote_pass1(currentNote, gatheredNoteInfo);
		}
	} // next note
    
	// musical notation requires more than one pass
	if(musicalNotationEnabled)
	{
        // analyse notes to know how to build the score
        analyseNoteInfo(gatheredNoteInfo, this);
        
		// ------------------------- second rendering pass -------------------
		// triplet signs, tied notes, tails and beams
        const int visibleNoteAmount = gatheredNoteInfo.size();
		for(int i=0; i<visibleNoteAmount; i++)
		{
            assertExpr(i,<,(int)gatheredNoteInfo.size());
			renderNote_pass2(gatheredNoteInfo[i]);
		}
        
		// -------------------------- third rendering pass -------------------
		// draw silences
		const unsigned int first_visible_measure = getMeasureBar()->measureAtPixel( getEditorXStart() );
		const unsigned int last_visible_measure = getMeasureBar()->measureAtPixel( getXEnd() );
		const int visible_measure_amount = last_visible_measure-first_visible_measure+1;
		bool measure_empty[visible_measure_amount];
		for(int i=0; i<visible_measure_amount; i++) measure_empty[i] = true;
        
		if(visibleNoteAmount>0)
		{
			// by comparing the ending of the rpevious note to the beginning of the current note,
			// we can know if there is a silence. If multiple notes play at the same time,
			// 'previous_note_end' will contain the end position of the longest note.
			// At this point all notes will already have been split so that they do not overlap on
			// 2 measures so we don't need to care about that.
			int previous_note_end = -1;
            
			int last_measure = -1;
#ifdef _MORE_DEBUG_CHECKS
 int iters = 0;
#endif
			for(int i=0; i<visibleNoteAmount; i++)
			{
#ifdef _MORE_DEBUG_CHECKS
iters++;
assertExpr(iters,<,1000);
#endif
                assertExpr(i,<,(int)gatheredNoteInfo.size());

				const int measure = getMeasureBar()->measureAtTick(gatheredNoteInfo[i].tick);
				assertExpr(measure,>=,0);
                assertExpr(measure,<,99999);
                
                assertExpr(last_measure,>=,-1);
                assertExpr(last_measure,<,99999);
                 
				// we switched to another measure
				if(measure>last_measure)
				{
					// if the last note of previous measure does not finish at the end of the measure,
					// we need to add a silence at the end of it
					if(last_measure != -1 and !aboutEqual(previous_note_end, getMeasureBar()->firstTickInMeasure(measure) ))
					{
						const int silence_length = getMeasureBar()->firstTickInMeasure(measure)-previous_note_end;
						renderSilence(previous_note_end, silence_length);
						
					}
					// if note is not at the very beginning of the new measure, and it's the first note of
					// the measure, we need to add a silence before it
					if(!aboutEqual(gatheredNoteInfo[i].tick, getMeasureBar()->firstTickInMeasure(measure) ))
					{
						const int silence_length = gatheredNoteInfo[i].tick - getMeasureBar()->firstTickInMeasure(measure);
						renderSilence(getMeasureBar()->firstTickInMeasure(measure), silence_length);
					}
					previous_note_end = -1; // we switched to another measure, reset and start again
				}

				last_measure = measure;

                if((int)(measure-first_visible_measure) >= (int)visible_measure_amount) break; // we're too far
				measure_empty[measure-first_visible_measure] = false;
				
				const int current_begin_tick = gatheredNoteInfo[i].tick;
                
				if( previous_note_end != -1 and !aboutEqual(previous_note_end, current_begin_tick) and (current_begin_tick-previous_note_end)>0 )
				{
                    renderSilence(previous_note_end, current_begin_tick-previous_note_end);
				}
				previous_note_end = gatheredNoteInfo[i].tick + gatheredNoteInfo[i].tick_length;
                
				// if there's multiple notes playing at the same time
				while(i+1<visibleNoteAmount and gatheredNoteInfo[i].tick==gatheredNoteInfo[i+1].tick)
				{
					i++;
					previous_note_end = std::max(previous_note_end, gatheredNoteInfo[i].tick + gatheredNoteInfo[i].tick_length);
				}
			}//next visible note
            
			// check for silence after last note
			const unsigned int last_measure_end = getMeasureBar()->lastTickInMeasure(
														getMeasureBar()->measureAtTick(
														gatheredNoteInfo[visibleNoteAmount-1].tick));
			if(!aboutEqual(previous_note_end, last_measure_end ) and previous_note_end>-1)
			{
				const int silence_length = last_measure_end-previous_note_end;
				renderSilence(previous_note_end, silence_length);
			}
            
			
		}// end if there are visible notes

        
		// draw silences in empty measures
		for(int i=0; i<visible_measure_amount; i++)
		{
			if(measure_empty[i])
			{
				renderSilence(getMeasureBar()->firstTickInMeasure(first_visible_measure+i),
							  getMeasureBar()->measureLengthInTicks(first_visible_measure+i));
			}
		}
	}

    
    AriaRender::lineWidth(1);
	// ------------------------- mouse drag (preview) ------------------------
    
    AriaRender::primitives();
	
    if(!clickedOnNote and mouse_is_in_editor)
    {
        // selection
        if(selecting)
        {
            AriaRender::color(0, 0, 0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);
            
        }
        else
        {
            // ----------------------- add note (preview) --------------------
            
            AriaRender::color(1, 0.85, 0);
            
            int preview_x1=
                (int)(
                      (snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI) ) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            int preview_x2=
                (int)(
                      (snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) ) -
                       sequence->getXScrollInMidiTicks())*sequence->getZoom()
                      );
            
            if(!(preview_x1<0 || preview_x2<0) and preview_x2>preview_x1)
            {
                
                const int y_base = ((mousey_initial - getEditorYStart() + getYScrollInPixels())/y_step)*y_step;
                const int y_add = getEditorYStart() - getYScrollInPixels();
                
                AriaRender::rect(preview_x1+getEditorXStart(), y_base-2 + y_add,
                                 preview_x2+getEditorXStart(), y_base+y_step+1 + y_add);
                
            }
            
        }// end if selection or addition
        
    }
    

    // ------------------------- move note (preview) -----------------------
    AriaRender::primitives();
    if(clickedOnNote)
    {
        AriaRender::color(1, 0.85, 0, 0.5);
        
        int x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
        int y_difference = mousey_current - mousey_initial;
        
        const int x_pixel_move = (int)( snapMidiTickToGrid(x_difference) * sequence->getZoom() );
        const int y_step_move = (int)round( (float)y_difference/ (float)y_step );
		
        // move a single note
        if(lastClickedNote!=-1)
        {
			int x1=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorXStart();
			const int x2=track->getNoteEndInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorXStart();
			//const int notePitch = track->getNotePitchID(lastClickedNote);
			const int noteLevel = converter->noteToLevel(track->getNote(lastClickedNote));
            
            AriaRender::rect(x1+1+x_pixel_move, (noteLevel+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels()-1,
                             x2-1+x_pixel_move, (noteLevel+1+y_step_move)*y_step + getEditorYStart() - getYScrollInPixels()-1);
			
        }
        else
        {
            // move a bunch of notes
            
            for(int n=0; n<track->getNoteAmount(); n++)
            {
                if(!track->isNoteSelected(n)) continue;
                
				int x1=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();
				const int x2=track->getNoteEndInPixels(n) - sequence->getXScrollInPixels() + getEditorXStart();
				//const int notePitch = track->getNotePitchID(n);
				const int noteLevel = converter->noteToLevel(track->getNote(n));
				
                AriaRender::rect(x1+1+x_pixel_move, (noteLevel+y_step_move)*y_step+1 + getEditorYStart() - getYScrollInPixels()-1,
                                 x2-1+x_pixel_move, (noteLevel+1+y_step_move)*y_step + getEditorYStart() - getYScrollInPixels()-1);
				
			}//next
            
        }
        
    }// end if clicked on note
    

	// --------------------------- grey left part -----------------------------
    if(!focus) AriaRender::color(0.4, 0.4, 0.4);
    else AriaRender::color(0.8, 0.8, 0.8);
    
    AriaRender::rect(0, getEditorYStart(),
                     getEditorXStart()-3, getYEnd());
    
	// ------------------------------- draw keys and horizontal lines -------------------------------

	AriaRender::color(0,0,0);
    
	if(GKey)
	{
        AriaRender::primitives();
        
        // draw horizontal score lines
		for(int n = middle_c_level - 10 ; n < middle_c_level; n+=2)
		{
			const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            AriaRender::line(0, liney, getXEnd(), liney);
		}

        // draw sharp/flat signs next to key
        AriaRender::images();
        
		int max_level_with_signs = -1;
		int min_level_with_signs = -1;
		
		if(converter->goingInSharps())
		{
			min_level_with_signs = middle_c_level - 11;
			max_level_with_signs = middle_c_level - 4;
		}
		else if(converter->goingInFlats())
		{
			min_level_with_signs = middle_c_level - 9;
			max_level_with_signs = middle_c_level - 2;
		}
		
        
        AriaRender::images();
		for(int n = min_level_with_signs; n < max_level_with_signs; n++)
		{
			const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
			const int sharpness = converter->getKeySigSharpnessSignForLevel(n);

			if(sharpness == SHARP)
			{
				sharpSign->move( 48 + sharp_sign_x[ converter->levelToNote7(n) ], liney-1 );
                sharpSign->render();
			}
			else if(sharpness == FLAT)
			{
				flatSign->move( 48 + flat_sign_x[ converter->levelToNote7(n) ], liney );
                flatSign->render();
			}
		}
	}

	if(FKey)
	{
        AriaRender::primitives();
        AriaRender::color(0,0,0);
        
        // draw horizontal score lines
		for(int n = middle_c_level+2 ; n < middle_c_level + 11; n+=2)
		{
			const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
            AriaRender::line(0, liney, getXEnd(), liney);
		}

        // draw sharp/flat signs next to key
        AriaRender::images();
        
		int max_level_with_signs = -1;
		int min_level_with_signs = -1;
		
		if(converter->goingInSharps())
		{
			min_level_with_signs = middle_c_level + 3;
			max_level_with_signs = middle_c_level + 10;
		}
		else if(converter->goingInFlats())
		{
			min_level_with_signs = middle_c_level + 5;
			max_level_with_signs = middle_c_level + 12;
		}
		
        AriaRender::images();
        
		for(int n = min_level_with_signs; n < max_level_with_signs; n++)
		{
			const int liney = getEditorYStart() + n*y_step + y_step/2 - yscroll;
			const int sharpness = converter->getKeySigSharpnessSignForLevel(n);
			if(sharpness == SHARP)
			{
				sharpSign->move( 48 + sharp_sign_x[ converter->levelToNote7(n) ], liney-1 ); sharpSign->render();
			}
			else if(sharpness == FLAT)
			{
				flatSign->move( 48 + flat_sign_x[ converter->levelToNote7(n) ], liney ); flatSign->render();
			}
		}
	}
	
	
	// --------------------------- key -------------------------
	
    AriaRender::images();
	if(GKey)
	{
		keyG->move(getEditorXStart() - 55, getEditorYStart() + (middle_c_level-6)*y_step -  yscroll + 5);
		keyG->render();
	}
	if(FKey)
	{
		keyF->move(getEditorXStart() - 65, getEditorYStart() + (middle_c_level+4)*y_step -  yscroll + 5);
		keyF->render();
	}

	// ---------------------------- scrollbar -----------------------
    if(!focus) AriaRender::color(0.5, 0.5, 0.5);
    else AriaRender::color(1,1,1);
    
    renderScrollbar();
    
    AriaRender::color(1,1,1);
    AriaRender::endScissors();
}

// ***************************************************************************************************************************************************
// ****************************************************    EVENT METHODS      ************************************************************************
// ***************************************************************************************************************************************************
#pragma mark -

void ScoreEditor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
								RelativeXCoord mousex_initial, int mousey_initial)
{
	Editor::mouseHeldDown(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void ScoreEditor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
							  RelativeXCoord mousex_initial, int mousey_initial)
{
	Editor::mouseExited(mousex_current, mousey_current, mousex_initial, mousey_initial);
}

void ScoreEditor::mouseDown(RelativeXCoord x, const int y)
{
    // user clicked on left bar to change tuning
	if(x.getRelativeTo(EDITOR)<-20 and x.getRelativeTo(WINDOW)>15 and y>getEditorYStart())
	{
        KeyPicker* picker = Core::getKeyPicker();
		picker->setParent(track->graphics);
		picker->setChecks( musicalNotationEnabled, linearNotationEnabled );
        Display::popupMenu( picker,x.getRelativeTo(WINDOW),y);
        return;
	}
    
	Editor::mouseDown(x, y);
	
}
void ScoreEditor::mouseDrag(RelativeXCoord mousex_current, int mousey_current,
							RelativeXCoord mousex_initial, int mousey_initial)
{
	Editor::mouseDrag(mousex_current, mousey_current, mousex_initial, mousey_initial);
    
}
void ScoreEditor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
						  RelativeXCoord mousex_initial, int mousey_initial)
{
	Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);
     
}

void ScoreEditor::rightClick(RelativeXCoord x, int y)
{
	Editor::rightClick(x, y);
    
}

// ****************************************************************************************************************************************************
// ****************************************************    EDITOR METHODS      ************************************************************************
// ****************************************************************************************************************************************************
#pragma mark -


int ScoreEditor::getYScrollInPixels()
{
	// check if visible area is large enough to display everything
	if(73*y_step <= height)
	{
		useVerticalScrollbar(false);
		return 0;
	}
	else
	{
		useVerticalScrollbar(true);
	}
	
	return (int)(  sb_position*(73*y_step-height) );
}

NoteSearchResult ScoreEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
	const int halfh = noteOpen->getImageHeight()/2;
	const int noteAmount = track->getNoteAmount();
	const int mx = x.getRelativeTo(WINDOW);
	
	for(int n=0; n<noteAmount; n++)
	{
		
		//const int notePitch = track->getNotePitchID(n);
		const int noteLevel = converter->noteToLevel(track->getNote(n));
        if(noteLevel == -1) continue;
		const int note_y = getEditorYStart() + y_step*noteLevel - halfh - getYScrollInPixels() + 2;
		const int note_x = getEditorXStart() + track->getNoteStartInPixels(n)  - sequence->getXScrollInPixels();
		const int note_x2 = getEditorXStart() + track->getNoteEndInPixels(n)  - sequence->getXScrollInPixels();
		
		if(linearNotationEnabled)
		{
			if( mx<note_x2 and mx>note_x and y < note_y+11 and y > note_y)
			{
				noteID = n;
				
				if(track->isNoteSelected(n) and !Display:: isSelectLessPressed())
					// clicked on a selected note
					return FOUND_SELECTED_NOTE;
				else
					return FOUND_NOTE;
			}
		}
		else if(musicalNotationEnabled)
		{
			
			if( mx<note_x+11 and mx>note_x and y < note_y+11 and y > note_y+2)
			{
				noteID = n;
				
				if(track->isNoteSelected(n) and !Display:: isSelectLessPressed())
					// clicked on a selected note
					return FOUND_SELECTED_NOTE;
				else
					return FOUND_NOTE;
			}
			
		}
		else { return FOUND_NOTHING; }
		
	}
	
	return FOUND_NOTHING;
}

void ScoreEditor::noteClicked(const int id)
{
	track->selectNote(ALL_NOTES, false);
	track->selectNote(id, true);
	track->playNote(id);
}

void ScoreEditor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{
	const int level = (mouseY - getEditorYStart() + getYScrollInPixels())/y_step;
	if(level<0 or level>=73) return;
	const int note = converter->levelToNote(level);
	if(note == -1) return;
    
	//std::cout << note << std::endl;
	//printNoteName( note );
	
	track->action( new Action::AddNote(note, snapped_start_tick, snapped_end_tick, 80 ) );
}

void ScoreEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
        if(note.startTick+relativeX < 0) return; // refuse to move before song start
        
        note.startTick += relativeX;
        note.endTick   += relativeX;
        
        if(relativeY==0) return;
		
		if(Display::isSelectMorePressed() or Display::isCtrlDown())
		{
			note.pitchID += relativeY;
		}
		else
		{
			int noteLevel = converter->noteToLevel(&note);
			noteLevel += relativeY;
            if(noteLevel > 0 and noteLevel < 73) // reject illegal notes
                note.pitchID = converter->levelToNote(noteLevel);
		}
        
}

void ScoreEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial)
{
	const int halfh = noteOpen->getImageHeight()/2;
	const int noteAmount = track->getNoteAmount();
	const int mxc = mousex_current.getRelativeTo(WINDOW);
	const int mxi = mousex_initial.getRelativeTo(WINDOW);
	
	for(int n=0; n<noteAmount; n++)
	{
		//const int notePitch = track->getNotePitchID(n);
		const int noteLevel = converter->noteToLevel(track->getNote(n));
        if(noteLevel == -1) continue;
		const int note_y = getEditorYStart() + y_step*noteLevel - halfh - getYScrollInPixels() + 2;
		const int note_x = getEditorXStart() + track->getNoteStartInPixels(n)  - sequence->getXScrollInPixels();
		
		if( std::min(mxc, mxi)<note_x and std::max(mxc,mxi)>note_x and
			std::min(mousey_current, mousey_initial) < note_y and
			std::max(mousey_current, mousey_initial) > note_y)
		{
			track->selectNote(n, true);
		}
		else
		{
			track->selectNote(n, false);
		}

	}
}


int ScoreEditor::getYStep(){ return y_step; }
int ScoreEditor::getHalfNoteHeight(){ return halfh; }

}
