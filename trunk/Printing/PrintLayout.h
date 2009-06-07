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
    class AriaPrintable;

    static const int max_line_width_in_units = 70;
    static const int maxLinesInPage = 10;

int getRepetitionMinimalLength();
void setRepetitionMinimalLength(const int newvalue);

/*
 A description of a measure to print. If we print more than one track at once,
 each measure will hold multiple 'MeasureTrackReference' instances.
 A 'MeasureTrackReference' "ties" a MeasureToExport to a Track object,
 by keeping a pointer to it and holding the range of notes in that track
 that belong to this measure.
 */

class MeasureTrackReference
{
public:
    Track* track;
    int firstNote, lastNote;
};

class MeasureToExport
{
public:
    MeasureToExport(const int measID);

    // Finds the notes correcsponding to this measure
    // in the given track and keep the reference.
    // Returns the ID of the last note in this measure
    int addTrackReference(const int firstNote, Track* track);

    // used when we print more than one track
    // each track we print will have one entry here
    // for each printed measure
    ptr_vector<MeasureTrackReference> trackRef;

    // first and last tick in this measure
    int firstTick, lastTick;

    // if this measure is later repeated and is not a repetition of a previous measure,
    // contains ID of all later measures similar to this one
    std::vector<int> similarMeasuresFoundLater;

    // if this measure is a repetition of a previous measure, contains the ID of which one
    int firstSimilarMeasure;

    // shortest note in the measure (will be used to
    // determine "zoom" on measure. e.g. a measure with very
    // short notes takes more room).
    int shortestDuration;

    // ID of the measure
    int id;

    // true if measure needs to be apart from others
    // mostly used with repetitions (e.g. x4) to tell where the repetition starts
    // FIXME - doesn't really belong here, should be a layout element
    bool cutApart;

    bool calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure);

    // if a repetition is found, it is stored in the variables and returns true,
    // otherwise returns false
    bool findConsecutiveRepetition(ptr_vector<MeasureToExport>& measures, const int measureAmount,
                                   int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                   int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/);
};

extern const MeasureToExport nullMeasure;

// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility
enum LayoutElementType
{
    SINGLE_MEASURE,
    SINGLE_REPEATED_MEASURE,
    EMPTY_MEASURE,
    REPEATED_RIFF,
    PLAY_MANY_TIMES,
    LINE_HEADER, // like the clef and key (on a score) or the word TAB and the tuning (for tabs)
    TIME_SIGNATURE
};
class LayoutElement
{
public:
    LayoutElement(LayoutElementType type_arg, int measure_arg = -1);

    LayoutElementType type;

    int measure;

    // used in many-measure repetitions. the first 2 ones are the measures that repeat, the last 2 ones the measures being repeated
    int firstMeasure, lastMeasure, firstMeasureToRepeat, lastMeasureToRepeat;

    // used for time sig elements
    int num, denom;
    
    int amountOfTimes; // used for 'play many times' events

    //float zoom;
    int width_in_units;

    // filled by EditorPrintable::continueWithNextElement() as the X position is calculated in
    // case location is needed later (for instance, if more than one rendering pass)
    int x, x2;
};

class EditorData
    {
    public:
        virtual ~EditorData() {}
    };
    
/*
 A line on a notation to print. Can contain more than one track.
 Essentially holds some 'LayoutElement' objects (the ones that fit
 on this line)
 */
class LayoutLine
{
    int currentTrack;
    AriaPrintable* printable;

    /** used to store what percentage of this line's height this track should take.
     e.g. a score with F+G clefs will need more space than a 4-string bass tab
     so vertical space must not be divided equally */
    std::vector<short int> height_percent;
public:
    LayoutLine(AriaPrintable* parent);

    // editors can put data of their own there.
    OwnerPtr<EditorData> editor_data;
    
    int width_in_units;
    int level_height;

    bool last_of_page;

    int getTrackAmount() const;
    void setCurrentTrack(const int n);
    Track* getTrack() const;
    int getFirstNoteInElement(const int layoutElementID);
    int getLastNoteInElement(const int layoutElementID);
    int getFirstNoteInElement(LayoutElement* layoutElement);
    int getLastNoteInElement(LayoutElement* layoutElement);

    void printYourself(wxDC& dc, const int x0, const int y0, const int x1, const int y1, int margin_below, int margin_above);
    int calculateHeight();

    MeasureToExport& getMeasureForElement(const int layoutElementID) const;
    MeasureToExport& getMeasureForElement(LayoutElement* layoutElement);

    int getLastMeasure() const;
    int getFirstMeasure() const;

    int getLastNote() const;
    int getFirstNote() const;

    std::vector<LayoutElement> layoutElements;
};

struct LayoutPage
{
    int first_line;
    int last_line;
};

class PrintLayoutManager
{
    AriaPrintable* parent;
    
    // referencing vectors from AriaPrintable
    std::vector<LayoutLine>& layoutLines;
    std::vector<LayoutPage>& layoutPages;
    ptr_vector<MeasureToExport>& measures;
    
    std::vector<LayoutElement> layoutElements;
    
    void calculateLineLayout();
    
    void calculateRelativeLengths();
    
    void generateOutputOrder(bool checkRepetitions_bool);
    void generateMeasures(ptr_vector<Track, REF>& tracks);
    
    void findSimilarMeasures();
public:
    PrintLayoutManager(AriaPrintable* parent,
                       std::vector<LayoutLine>& layoutLines  /* out */,
                       std::vector<LayoutPage>& layoutPages  /* out */,
                       ptr_vector<MeasureToExport>& mesaures /* out */);
    
    void calculateLayoutElements(ptr_vector<Track, REF>& track, const bool checkRepetitions_bool);
};
    
}

#endif
