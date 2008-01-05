
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "GUI/MeasureBar.h"
#include "Editors/GuitarEditor.h"
#include "IO/IOUtils.h"
#include "IO/TablatureExporter.h"

namespace AriaMaestosa
{

// ----------------------------------------------------------------------------------------------------
// ------------------------------------- first function called ----------------------------------------
// ----------------------------------------------------------------------------------------------------

	int repetitionMinimalLength;
	
TablatureExporter::TablatureExporter()
{
	INIT_LEAK_CHECK();	
	max_length_of_a_line = 110;
}

void TablatureExporter::setMaxLineWidth(int i)
{
	max_length_of_a_line = i;
}

void TablatureExporter::flush()
{
	file->Write( title_line + wxT("\n") );
	for(int s=0; s<string_amount; s++)
	{
		file->Write( strings[s] + wxT("\n"));
		strings[s] = wxT("|");	;
	}
	// for last string, skip one more line
	file->Write( wxT("\n") );
	measures_appended = 0;
	charAmountWhenBeginningMeasure = 2;
	repetitionMinimalLength = 2;
	title_line = wxT("");
}

void TablatureExporter::setRepetitionMinimalWidth(int measureAmount)
{
	repetitionMinimalLength = measureAmount;
}

class MeasureToExport
{
public:
	Track* track;
	
	int firstTick, lastTick, firstNote, lastNote;

	// if this measure is later repeated and is not a repetition of a previous measure,
	// contains ID of all later measures similar to this one
	std::vector<int> similarMeasuresFoundLater;
	
	// if this measure is a repetition of a previous measure, contains the ID of which one
	int firstSimilarMeasure;
	
	int shortestDuration;
	int id;
	
	// true if measure needs to be apart from others
	// mostly used with repetitions (e.g. x4) to tell where the repetition starts
	bool cutApart;
	
	MeasureToExport()
	{
		shortestDuration = -1;
		firstSimilarMeasure = -1;
		cutApart = false;
	}
	
	void setID(int id_arg)
	{
		id = id_arg;
	}
	
	bool isSameAs(MeasureToExport* array, int compareWithID)
	{
		return (array[compareWithID].firstSimilarMeasure == firstSimilarMeasure) or (array[compareWithID].firstSimilarMeasure == id);
	}
	
	// if a repetition is found, it is stored in the variables and returns true,
	// otherwise returns false
	bool findConsecutiveRepetition(MeasureToExport* measures, const int measureAmount,
								   int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
								   int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/)
	{
		//similarMeasuresFoundLater
			
		// check if it works with first measure occurence of similar measures
		if(id+1<measureAmount and measures[id+1].firstSimilarMeasure == measures[id].firstSimilarMeasure+1 )
		{
			int amount = 0;
			
			for(int iter=1; iter<measureAmount; iter++)
			{
				if(id+iter<measureAmount and measures[id+iter].firstSimilarMeasure == measures[id].firstSimilarMeasure+iter )
				{
					amount++;
				}
				else
				{
					break;
				}
			}//next
			firstMeasureThatRepeats = id;
			lastMeasureThatRepeats = id + amount;
			firstMeasureRepeated = measures[id].firstSimilarMeasure;
			lastMeasureRepeated = measures[id].firstSimilarMeasure + amount;
			return true;
		}
		// check if it works with a later occurence of a similar measure
		else
		{
			const int first_measure =  measures[id].firstSimilarMeasure;
			const int amount = measures[ first_measure ].similarMeasuresFoundLater.size();
			for(int laterOccurence=0; laterOccurence<amount; laterOccurence++)
			{
				const int checkFromMeasure = measures[ first_measure ].similarMeasuresFoundLater[laterOccurence];
				std::cout << "		{ lvl 2, testing measure " << checkFromMeasure << std::endl;
				//if(checkFromMeasure+1<id and measures[checkFromMeasure+1].firstSimilarMeasure ==
				//   measures[checkFromMeasure].firstSimilarMeasure+1 )
				//{
					int amount = 0;
					
					// check if there is a consecutive repetition with measures from this area
					
					for(int iter=0; iter</*id-checkFromMeasure*/measureAmount; iter++)
					{
						if(not(checkFromMeasure+iter<id and
							   checkFromMeasure+iter<measureAmount and id+iter<measureAmount)) continue;
						
						// check if they are identical
						
						if( // they are identical because they both are repetitions of the same one
						   (measures[checkFromMeasure+iter].firstSimilarMeasure == measures[id+iter].firstSimilarMeasure and
							measures[checkFromMeasure+iter].firstSimilarMeasure != -1)
						   or
							// they are identical because the second is a repetition of the first
							(checkFromMeasure+iter == measures[id+iter].firstSimilarMeasure)
						   )
						{
							std::cout << "			//" << (checkFromMeasure+iter+1) << " is same as " << (id+iter+1) << std::endl;
							amount++;
						}
						else
						{
							std::cout << "			//but " << (checkFromMeasure+iter+1) << " is NOT same as " << (id+iter+1) << " (" << measures[checkFromMeasure+iter].firstSimilarMeasure+1 << " != " << measures[id+iter].firstSimilarMeasure+1 << ")" << std::endl;
							break;
						}
					}//next
					std::cout << "		} amount=" << amount << std::endl;
					if(amount<repetitionMinimalLength) continue;
					std::cout << "measure " << id+1  << " is a level 2 repetition" << std::endl;
					firstMeasureThatRepeats = id;
					lastMeasureThatRepeats = id + amount-1;
					firstMeasureRepeated = checkFromMeasure;
					lastMeasureRepeated = checkFromMeasure + amount-1;
					return true;
				//}
			}//next
			
			// if we get there, it never works
			return false;
		}
	}
	
	bool calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure)
	{

		// if these 2 measures don't even have the same number of notes, they're definitely not the same
		if(
		   (checkMeasure.lastNote - checkMeasure.firstNote) != (lastNote - firstNote)
		   ) return false;
		

		const int noteAmount = (checkMeasure.lastNote - checkMeasure.firstNote);

		
		if(noteAmount<1) return false; //empty measure
		
		/*
		 if we get till there, the 2 measures have the same amount of notes.
		 to know whether they are truly identitcal, we need to compare note by note
		 we will match each notes from the first measure to the identical one in the second.
		 If ever one note failes to be matched, then the 2 measures are different.
		 */
		int noteMatched_this[noteAmount];
		int noteMatched_other[noteAmount];
		for(int n=0; n<noteAmount; n++)
		{
			noteMatched_this[n] = false;
			noteMatched_other[n] = false;
		}
		
		for(int checkNote_this=0; checkNote_this<noteAmount; checkNote_this++)
		{
			for(int checkNote_other=0; checkNote_other<noteAmount; checkNote_other++)
			{
				if(noteMatched_other[checkNote_other]) continue; // this note was already matched
				
				// check start tick matches
				if(track->getNoteStartInMidiTicks(checkMeasure.firstNote + checkNote_other) - checkMeasure.firstTick !=
				   track->getNoteStartInMidiTicks(firstNote + checkNote_this) - firstTick)
				{
					// they dont match, check the next one
					continue;
				}
				
				// check end tick matches
				if(track->getNoteEndInMidiTicks(checkMeasure.firstNote + checkNote_other) - checkMeasure.firstTick !=
				   track->getNoteEndInMidiTicks(firstNote + checkNote_this) - firstTick)
				{
					// they dont match, check the next one
					continue;
				}
				
				// check pitch matches
				if(track->getNotePitchID(checkMeasure.firstNote + checkNote_other) !=
				   track->getNotePitchID(firstNote + checkNote_this))
				{
					// they dont match, check the next one
					continue;
				}
				
				noteMatched_this[checkNote_this] = true;
				noteMatched_other[checkNote_other] = true;
				
				
			}//next
			
			// we couldn't find a note in the other measure that matches this one
			if(noteMatched_this[checkNote_this] == false) return false;
			
		}//next
		
		return true;
	}
};

// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility
enum LayoutElementType
{
	SINGLE_MEASURE,
	SINGLE_REPEATED_MEASURE,
	EMPTY_MEASURE,
	REPEATED_RIFF,
	PLAY_MANY_TIMES
};
class LayoutElement
{
public:
	LayoutElement(LayoutElementType type_arg, int measure_arg = -1)
	{
		type = type_arg;
		measure = measure_arg;
	}
	
	LayoutElementType type;
	
	int measure; // used in single measure mode
				 
	// used in many-measure repetitions. the first 2 ones are the measures that repeat, the last 2 ones the measures being repeated
	int firstMeasure, lastMeasure, firstMeasureToRepeat, lastMeasureToRepeat;
	
	int amountOfTimes; // used for 'play many times' events
};

void TablatureExporter::exportTablature(Track* track_arg, wxFile* file_arg, bool checkRepetitions_bool_arg)
{

	checkRepetitions_bool = checkRepetitions_bool_arg;
	file = file_arg;
	track = track_arg;
	
	measureAmount = getMeasureBar()->getMeasureAmount();
	ticksPerBeat = getMainFrame()->getCurrentSequence()->ticksPerBeat();
	noteAmount = track->getNoteAmount();
	editor = track->graphics->guitarEditor;
	string_amount = editor->tuning.size();
	
	int note=0;
	
	measures_appended = 0;
	for(int i=0; i<string_amount; i++) strings.push_back( wxT("") );
	
	for(int s=0; s<string_amount; s++)
	{
		strings[s] = wxT("|");	
	}
	title_line = wxT("");
	
	MeasureToExport measures[measureAmount];
	
	// -------------------- gather measure information -------------------- 
	for(int measure=0; measure<measureAmount; measure++)
	{
		measures[measure].setID(measure);
		
		measures[measure].firstTick = getMeasureBar()->firstTickInMeasure( measure );
		measures[measure].lastTick = getMeasureBar()->lastTickInMeasure( measure );
		
		// first note in measure (which is also last note of previous measure, that was set in last iteration of the for loop)
		measures[measure].firstNote = note;
		
		// ------------------------------------ find what the first, last and shortest note in current measure --------------------------------------
		// find what is the shortest note in this measure. this will help determine how big it must be made.
		// to do this, iterate through notes, keeping the smallest length we find, stopping when we reach next measure
		// this by the way finds where the measure begins and end, thus filling up 'measureFirstNote' and 'measureLastNote' variables.
		for(; note<noteAmount; note++)
		{
			// stop when we're at next measure - it will be done in next measure iteration
			if( track->getNoteStartInMidiTicks(note) >= measures[measure].lastTick ) break;
			
			const int currentNoteDuration = track->getNoteEndInMidiTicks(note) - track->getNoteStartInMidiTicks(note);
			
			if(currentNoteDuration <= 0)  continue; // skpi malformed notes if any
			if( currentNoteDuration < measures[measure].shortestDuration or measures[measure].shortestDuration==-1) measures[measure].shortestDuration = currentNoteDuration;
			
		}
		
		measures[measure].lastNote = note; // ID of the last note in this measure
	}
	
	 //-------------------- search for repeated measures if necessary  -------------------- 
	if(checkRepetitions_bool)
	{
		for(int measure=0; measure<measureAmount; measure++)
		{
			measures[measure].track = track;
			
			// check current measure against all previous measures to see if it is not a repetition
			for(int checkMeasure=0; checkMeasure<measure; checkMeasure++)
			{
				const bool isSameAs = measures[measure].calculateIfMeasureIsSameAs(measures[checkMeasure]);
				
				if(!isSameAs) continue;
				measures[measure].firstSimilarMeasure = checkMeasure;
				measures[checkMeasure].similarMeasuresFoundLater.push_back(measure);
				break;
			}//next
		}//next
		
	}//endif check repetitions
	
	// -------------------- calculate tablature layout ----------------------
	
	std::vector<LayoutElement> layoutElements;
	for(int measure=0; measure<measureAmount; measure++)
	{
#ifdef _verbose
		std::cout << (measure+1) << ":" << std::endl;
#endif
		int firstMeasureThatRepeats, lastMeasureThatRepeats, firstRepeatedMeasure, lastRepeatedMeasure; // used when finding repetitions
		
		// ----- empty measure -----
		if(measures[measure].shortestDuration==-1)
		{
			layoutElements.push_back( LayoutElement(EMPTY_MEASURE) );
		}
		
		// repetition
		else if(checkRepetitions_bool and measures[measure].firstSimilarMeasure!=-1)
		{
			
			if(repetitionMinimalLength<2)
			{
				LayoutElement element(SINGLE_REPEATED_MEASURE, measure);
				layoutElements.push_back( element );
				continue;
			}
			
			// user requires that repetitions are longer than one measure
			// check if it is the case
			
			// -------- play same measure multiple times --------
			// check if next measure is the same as current measure
			if(measure+1<measureAmount and measures[measure+1].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
			{
				int amountOfTimes = 1;
				for(int iter=1; iter<measureAmount; iter++)
				{
					if(measure+iter<measureAmount and measures[measure+iter].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
					{
						amountOfTimes++;
					}
					else
					{
						break;
					}
				}//next
				
				if(amountOfTimes < repetitionMinimalLength)
				{
#ifdef _verbose
					std::cout << "play many times refused, measures " << (measure+1) << " to " << (measure+amountOfTimes+1) << " are normal" << std::endl;
#endif
					// not enough repetitions, add as regular measures
					for(int i=0; i<amountOfTimes; i++)
					{
						layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
						measure++;
					}
					measure--;
				}
				else
				{
					// check if we need to display the repetition first before adding play many times
					if(measures[measure].firstSimilarMeasure != -1 and measures[measure].firstSimilarMeasure != measure-1)
						// we don't need to if measure was not a repeptition, in which case it is already there
						// we need neither if it is the measure just before
					{
						LayoutElement element(SINGLE_REPEATED_MEASURE, measure);
						layoutElements.push_back( element );
					}
					else
					{
						amountOfTimes++; // if measure was already displayed... there were e.g. 3 additional repetitions, but we want to show x4
					}
#ifdef _verbose
					std::cout << "measure " << (measure+1) << " is played " << amountOfTimes << " times. all are the same as " << (measures[measure].firstSimilarMeasure+1) << std::endl;
#endif
					LayoutElement element(PLAY_MANY_TIMES);
					element.amountOfTimes = amountOfTimes;
					measures[measure].cutApart = true;
					if(measures[measure].firstSimilarMeasure == measure-1) measure = measure + amountOfTimes-2;
					else measure = measure + amountOfTimes-1;
					layoutElements.push_back( element );
				}
				
			}
			
			// ------- repeat a riff --------
			// check if next measure is a reptition, and check this repetition is the next one compared to the current repeated measure
			else if( measures[measure].findConsecutiveRepetition(measures, measureAmount, firstMeasureThatRepeats, lastMeasureThatRepeats,
																 firstRepeatedMeasure, lastRepeatedMeasure) )
			{
				
				const int amount = lastMeasureThatRepeats - firstMeasureThatRepeats;
				if(amount+1 >= repetitionMinimalLength)
				{
#ifdef _verbose
					std::cout << "repetition from " << (firstMeasureThatRepeats+1) << " to " <<
					(lastMeasureThatRepeats+1) << "(" << (firstRepeatedMeasure+1) << ", " <<
					(lastRepeatedMeasure+1) << ")"  << std::endl;
#endif
					
					LayoutElement element(REPEATED_RIFF);
					element.firstMeasure = firstMeasureThatRepeats;
					element.lastMeasure = lastMeasureThatRepeats;
					element.firstMeasureToRepeat = firstRepeatedMeasure;
					element.lastMeasureToRepeat = lastRepeatedMeasure;
					measure = lastMeasureThatRepeats;//measure + amount;
					layoutElements.push_back( element );
				}
				else
					// repetition is not long enough, use normal measures
				{
#ifdef _verbose
					std::cout << "repetition refused because " << (amount+1) << " < " << repetitionMinimalLength << " measures " << (measure+1) << " to " << (measure+repetitionMinimalLength+1) << " are normal" << std::endl;
#endif
					for(int iter=0; iter<repetitionMinimalLength; iter++)
					{
						layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure+iter) );
					}
					measure += repetitionMinimalLength-1;
				}
				
			}
			else
			{
#ifdef _verbose
				std::cout << "measure " << (measure+1) << " looks like a repetition but treated as normal" << std::endl;
#endif
				// despite looking like a repetition, user settings are set to treat it as a normal measure
				layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
			}
			
		}
		else
			// ------ normal measure -----
		{
#ifdef _verbose
			std::cout << "measure " << (measure+1) << " is normal" << std::endl;
#endif
			layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
		}
		
	}//next measure
	
	// -------------------- generate the tablature  -------------------- 
	const int layoutElementsAmount = layoutElements.size();

	for(int n=0; n<layoutElementsAmount; n++)
	{
	
		zoom = 1;
		// contains the number of characters each line contained, before adding new ones for this measure.
		charAmountWhenBeginningMeasure = -1; // will be set later, after adding the "|-" of the measure beginning
		added_chars = 0;
		
		// ****** empty measure
		if(layoutElements[n].type == EMPTY_MEASURE)
		{
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("--|"));
			}
			title_line.Append(wxT("   "));
			measures_appended++;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + 3 > max_length_of_a_line)
			{
				flush();
			}
			
			continue;
		} 
		
		// ****** repetitions
		else if(layoutElements[n].type == SINGLE_REPEATED_MEASURE or layoutElements[n].type == REPEATED_RIFF)
		{
			// if measure must be set apart from others
			// if it is the first of its line, ignore that, it's already apart
			if(measures[layoutElements[n].measure].cutApart and measures_appended>0)
			{
				for(int s=0; s<string_amount; s++)
				{
					strings[s].Append(wxT("|"));
				}
			}
			
			wxString message;
			if(layoutElements[n].type == SINGLE_REPEATED_MEASURE)
			{
				message = to_wxString(measures[layoutElements[n].measure].firstSimilarMeasure+1);
				measures_appended++;
			}
			else if(layoutElements[n].type == REPEATED_RIFF)
			{
				message =	to_wxString(layoutElements[n].firstMeasureToRepeat+1) +
				wxT(" - ") + 
				to_wxString(layoutElements[n].lastMeasureToRepeat+1);
				measures_appended += (layoutElements[n].lastMeasure - layoutElements[n].firstMeasure);
			}
			const int chars_in_message = message.Length();
			
			// check if we reached maximal line width
			const int needed_length = chars_in_message + 4;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			for(int n=0; n<string_amount; n++)
			{
				if(n == string_amount/2) strings[n].Append( wxT(" *") + message + wxT(" |") );
				else strings[n].Append('-', chars_in_message+3 ).Append(wxT("|"));
			}
			title_line.Append(' ', chars_in_message+4 );
		}
		// ****** play again
		else if(layoutElements[n].type == PLAY_MANY_TIMES)
		{
			wxString message;
			message = to_wxString(layoutElements[n].amountOfTimes);
			measures_appended += layoutElements[n].amountOfTimes;
			const int chars_in_message = message.Length();
			
			// check if we reached maximal line width
			const int needed_length = chars_in_message + 4;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			for(int n=0; n<string_amount; n++)
			{
				if(n == string_amount/2) strings[n].Append( wxT(" x") + message + wxT(" |") );
				else strings[n].Append(' ', chars_in_message+3 ).Append(wxT("|"));
			}
			title_line.Append(' ', chars_in_message+4 );
			
		}
		// ****** normal measure
		else if(layoutElements[n].type == SINGLE_MEASURE)
		{
			assertExpr(measures[layoutElements[n].measure].shortestDuration,!=,0);
			
			const int divider = (int)(
									  getMeasureBar()->getTimeSigNumerator(layoutElements[n].measure) * (float)ticksPerBeat /
									  (float)measures[layoutElements[n].measure].shortestDuration
									  );
			
			// if notes are very long, zoom a bit because we don't want a too short measure
			if( divider <= 2 ) zoom = 4;
			if( divider <= 1 ) zoom = 8;
			
			// ------------------------------------ check for end of line --------------------------------------
			// if current line is full (i.e. no more horizontal space), output current line to file and prepare a new line
			
			// check how much place the next measure would take
			const int needed_length = (int)(
											divider * getMeasureBar()->getTimeSigNumerator(layoutElements[n].measure)  / 2.0
											);
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			// ------------------------------------ begin the new measure --------------------------------------
			// if measure must be set apart from others
			// if it is the first of its line, ignore that, it's already apart
			if(measures[layoutElements[n].measure].cutApart and measures_appended>0)
			{
				for(int s=0; s<string_amount; s++)
				{
					strings[s].Append(wxT("|"));
				}
			}
			
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("-"));
			}
			charAmountWhenBeginningMeasure = strings[0].Length();
			
			// fret number higher than 9 take more place, so we one is met, aditionnal room is added
			// this bool remembers whether we already met one of these fret numbers
			bool found_fret_higher_than_9=false;

			// ------------------------------------ write notes on the strings --------------------------------------
			int currentStep = 0;
			// use the IDs of the first and the last note in the measure we previously found, and iterate through all the notes in the measure
			for(int note=measures[layoutElements[n].measure].firstNote; note<measures[layoutElements[n].measure].lastNote; note++)
			{
				// gather information about current note
				const int note_fret = track->getNoteFret(note);
				const int note_string = track->getNoteString(note);
				
				// here, what i call "step" is a "-" followed by a number or another "-". e.g. tab |-2-3-4---5-| contains 5 steps.
				// think of it as X Position, but counted in characters
				const int note_step = (int)(
											(float)divider * zoom *
											(float)(track->getNoteStartInMidiTicks(note) - (measures[layoutElements[n].measure].firstTick) ) /
											(float)getMeasureBar()->measureLengthInTicks( layoutElements[n].measure )
											);
				
				// if current note is farther than current step, add "-"s until we reach the position of the current's note step
				if(note_step > currentStep)
				{
					// we're moving to a new lcoation, so reset this variable to be able to find more 2-digit fret numbers
					found_fret_higher_than_9 = false;
					
					// calculate how many characters we need on each string to go to next note
					const int wantedStep = currentStep + (note_step - currentStep);
					
					const int wantedChars = charAmountWhenBeginningMeasure + wantedStep*2 + added_chars;
					for(int s=0; s<string_amount; s++)
					{
						while( (int)strings[s].Length() < wantedChars )
						{
							strings[s].Append(wxT("-"));
						}
					}
					currentStep = wantedStep;
				}
				
				// if fret number is greater than 9, it takes 2 digits so we need to make more place for it
				if(note_fret > 9 and !found_fret_higher_than_9)
				{
					added_chars++;
					found_fret_higher_than_9 = true;
				}
				strings[note_string].Append( to_wxString(note_fret)  );
			}//next

			// ------------------------------------ end of measure --------------------------------------
			// fill all lines that are not already full with "-"s
			const int wantedChars = charAmountWhenBeginningMeasure + divider * 2 * zoom + added_chars;
			for(int s=0; s<string_amount; s++)
			{
				while( (int)strings[s].Length() < wantedChars) strings[s].Append(wxT("-"));
			}
			
			// ------------------------------------ title line --------------------------------------
			// create title line for this measure
			wxString measureTitle = to_wxString(layoutElements[n].measure+1);
			title_line.Append( measureTitle );
			const int title_length = measureTitle.Length()/* + 1 */;
			const int chars_in_measure = (divider * 2 * zoom + added_chars);
			
			for(int i=title_length; i<chars_in_measure; i++)
			{
				title_line.Append(wxT(" "));	
			}
			while( (int)title_line.Length() < wantedChars ) title_line.Append(wxT(" "));
			
			currentStep = 0;
			
			// end measure with a vertical bar
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("|"));
			}
			
			measures_appended++;
		}
		
	}

	// output to file anything that remains
	flush();
	
	
}

}
