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


#ifndef _NotationExport_
#define _NotationExport_

#include <vector>

namespace AriaMaestosa
{
    class Track;
    

int getRepetitionMinimalLength();
void setRepetitionMinimalLength(const int newvalue);



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
	
	MeasureToExport();
	
	void setID(int id_arg);
	
	bool isSameAs(MeasureToExport* array, int compareWithID);

	// if a repetition is found, it is stored in the variables and returns true,
	// otherwise returns false
	bool findConsecutiveRepetition(MeasureToExport* measures, const int measureAmount,
								   int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
								   int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/);
	
    bool calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure);
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
	LayoutElement(LayoutElementType type_arg, int measure_arg = -1);
	
	LayoutElementType type;
	
	int measure; // used in single measure mode
				 
	// used in many-measure repetitions. the first 2 ones are the measures that repeat, the last 2 ones the measures being repeated
	int firstMeasure, lastMeasure, firstMeasureToRepeat, lastMeasureToRepeat;
	
	int amountOfTimes; // used for 'play many times' events
};

void getLayoutElements(Track* track, const bool checkRepetitions_bool, std::vector<LayoutElement>& layoutElements, MeasureToExport* measures);


}

#endif
