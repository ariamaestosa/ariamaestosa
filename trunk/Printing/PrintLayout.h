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


#ifndef _print_layout_h_
#define _print_layout_h_

#include <vector>
#include "ptr_vector.h"
#include "wx/wx.h"

namespace AriaMaestosa
{
    class Track;
    
    static const int maxCharItemsPerLine = 45;
    static const int maxLinesInPage = 7;
    
int getRepetitionMinimalLength();
void setRepetitionMinimalLength(const int newvalue);

class MeasureTrackReference
{
public:
    Track* track;
    int firstNote, lastNote;
};

class MeasureToExport
{
public:
	//Track* track;
	
    MeasureToExport(const int measID);
    
    // Finds the notes correcsponding to this measure
    // in the given track and keep the reference.
    // Returns the ID of the last note in this measure
    int addTrackReference(const int firstNote, Track* track);
    
    // used when we print more than one track
    // each track we print will have one entry here
    // for each printed measure
    ptr_vector<MeasureTrackReference> trackRef;
    
	int firstTick, lastTick;

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
	
	bool isSameAs(MeasureToExport& compareWith);

	// if a repetition is found, it is stored in the variables and returns true,
	// otherwise returns false
	bool findConsecutiveRepetition(ptr_vector<MeasureToExport>& measures, const int measureAmount,
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
	
	int measure;
				 
	// used in many-measure repetitions. the first 2 ones are the measures that repeat, the last 2 ones the measures being repeated
	int firstMeasure, lastMeasure, firstMeasureToRepeat, lastMeasureToRepeat;
	
	int amountOfTimes; // used for 'play many times' events
    
    float zoom;
    int charWidth;
};


class LayoutLine
{
    int currentTrack;
public:
    LayoutLine();
    
    int editorMode; // GUITAR, etc.
    
    int charWidth;
    
    // FIXME - the argument is weird
    // find a better way to store the measure vector
    int getTrackAmount(ptr_vector<MeasureToExport>& measures);
    void setCurrentTrack(const int n);
    Track* getTrack(ptr_vector<MeasureToExport>& measures);
    int getFirstNote(ptr_vector<MeasureToExport>& measures, const int layoutElementID);
    int getLastNote(ptr_vector<MeasureToExport>& measures, const int layoutElementID);
    
    std::vector<LayoutElement> layoutElements;
};

class LayoutPage
{
public:
    std::vector<LayoutLine> layoutLines;
};

void calculateLayoutElements(ptr_vector<Track>& track, const bool checkRepetitions_bool, std::vector<LayoutPage>& layoutPages, ptr_vector<MeasureToExport>& mesaures);

}

#endif
