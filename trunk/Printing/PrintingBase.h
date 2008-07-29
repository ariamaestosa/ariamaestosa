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


#ifndef _print_base_h_
#define _print_base_h_

#include <vector>
#include "Printing/PrintLayout.h"
#include "wx/wx.h"

/*
 Printing module overview :
 
 An 'AriaPrintable' object is created. Then tracks are added to it through 'addTrack'.
 When each new track is added, its editor type is checked (guitar, score, etc.)
 A child class of 'EditorPrintable' that can print this type of notation is then
 created and added into a vector.
 
 Then 'calculateLayout()' is called. This takes us into 'PrintLayout.h'. This part lays
 out the measures and other elements in pages, and finds repetitions if necessary. It
 builds a tree of lines (that can contain one ore more tracks), layout elements
 (can be a measure, a repetition, etc.)
 
 Once the song structure is built,
 the AriaPrintable is sent to the printer. The method 'printPage' is then called
 for each page. The AriaPrintable will then render the page header, etc. and split the
 remaining space between tracks. Then, for each line, the method 'printYourself'
 is called. There, the line further divides itself into individual tracks and calls
 the corresponding EditorPrintable to draw itself.
 
 The EditorPrintable base class manages stuff like drawing measures, repetitions,
 determining the position of notes, etc. Only the part of the drawing that is specific
 to a notation mode is left to the child. 'drawLine' is to b overridden by children.
 Children are to call 'beginLine' early. Then they can iterate through LayoutElements
 by calling getNextElement() in a loop until NULL is returned. Then, for
 returned elements of type SINGLE_MEASURE, the child can get note information by
 using getFIrstNoteInElement(), getLastNoteInElement(), getNotePrintX() and render
 them appropriately.
 
 */
namespace AriaMaestosa
{
    class Track;
    
class EditorPrintable
{
    // keeps some info on the track currently being printed
    int x0, y0, x1, y1;
    bool show_measure_number;
    int xloc, currentLayoutElement;
    LayoutLine* currentLine;
    int layoutElementsAmount;
    wxDC* dc;
protected:
    float widthOfAChar;
public:
    EditorPrintable();
    virtual ~EditorPrintable();
        
    virtual void drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1, bool show_measure_number);

    void beginLine(wxDC* dc, LayoutLine* line, int x0, const int y0, const int x1, const int y1, bool show_measure_number);
    int getCurrentElementXStart();
    int getCurrentElementXEnd();
    LayoutElement* getNextElement();
    int getNotePrintX(int noteID);
    int tickToX(const int tick);
};
    
class AriaPrintable
{
    friend class AriaMaestosa::LayoutLine;
protected:

    std::vector<LayoutPage> layoutPages;
    ptr_vector<MeasureToExport> measures;
    
    Sequence* sequence;
public:
    // ---------------------------------------
    // global info for printables, read-only
    // FIXME - find cleaner way
    int text_height;
    int text_height_half;
    
    ptr_vector<Track> tracks;
    ptr_vector<EditorPrintable> editorPrintables;
    // ---------------------------------------
    
    AriaPrintable(Sequence* parent);
    void addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */);
    void calculateLayout(bool checkRepetitions_bool);
    
    virtual ~AriaPrintable();
    wxString getTitle();
    int getPageAmount();
    void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h);

    bool portraitOrientation();
    
};

AriaPrintable* getCurrentPrintable();

bool printResult(AriaPrintable* printable);
    
}

#endif
