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
 FIXME : this is outdated, update
 
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
 to a notation mode is left to the child. 'drawLine' is to be overridden by children.
 Children are to call 'beginLine' early. Then they can iterate through LayoutElements
 by calling continueWithNextElement() in a loop until NULL is returned. Then, for
 returned elements of type SINGLE_MEASURE, the child can get note information by
 using getFirstNoteInElement(), getLastNoteInElement(), getNotePrintX() and render
 them appropriately.
 
 */
const bool PRINT_LAYOUT_HINTS = false;

namespace AriaMaestosa
{
    class PrintableSequence;    
    
class AriaPrintable
{
    DECLARE_MAGIC_NUMBER();
    
    friend class AriaMaestosa::LayoutLine;

    PrintableSequence* seq;
    
    void placeLinesInPage(LayoutPage& page, const int text_height, const float track_area_height, const int level_y_amount,
                          const int pageHeight, const int x0, const int y0, const int x1);
    void divideLineAmongTracks(LayoutLine& line, const int x0, const int y0, const int x1, const int y1,
                               int margin_below, int margin_above);
    
public:
    // ---------------------------------------
    // global info for printables, read-only
    // FIXME - find cleaner way
    int text_height;
    int text_height_half;
    // ---------------------------------------
    
    /**
      * 'seq' remains owned by the caller, AriaPrintable will not delete it. Caller must not delete 'seq' before
      * AriaPrintable is deleted too.
      */
    AriaPrintable(PrintableSequence* seq);
    
    virtual ~AriaPrintable();

    void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h);
    void printLine(LayoutLine& line, wxDC& dc);
    
    int print();
};

AriaPrintable* getCurrentPrintable();

    
}

#endif
