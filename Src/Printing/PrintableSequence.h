

#ifndef __PRINTABLE_SEQUENCE_H__
#define __PRINTABLE_SEQUENCE_H__


#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/PrintLayout/PrintLayoutNumeric.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

namespace AriaMaestosa
{
    class Track;
    class EditorPrintable;
    
    class PrintableSequence
    {
        ptr_vector<LayoutPage> layoutPages;
        
        OwnerPtr<PrintLayoutAbstract> m_abstract_layout_manager;
        OwnerPtr<PrintLayoutNumeric> m_numeric_layout_manager;
        
        Sequence* sequence;
        
        void printLine(LayoutLine& line, wxDC& dc);

    public:
        bool is_guitar_editor_used;
        bool is_score_editor_used;
        int track_amount;
        int max_signs_in_keysig; // the max number of sharp/flats we'll need to display at header (useful to allocate proper size)
        
        
        ptr_vector<Track, REF> tracks;
        ptr_vector<EditorPrintable> editorPrintables;
        
        
        PrintableSequence(Sequence* parent);
        bool addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */);
        
        void calculateLayout(bool checkRepetitions);
        
        EditorPrintable* getEditorPrintable(const int trackID);
        
        wxString getTitle();
        
        int getPageAmount();
        LayoutPage& getPage(const int id);
        
        /**
          * Called when it is time to print all lines of a page.
          *
          * @param dc                  The wxDC onto which printing occurs
          * @param page                The page that contains the lines we want to print now
          * @param notation_area_y0    Minimum Y coordinate of the actual area where track notation is drawn
          *                            (excluding header/etc...)
          * @param notation_area_h     Height of the area that is actually printable with notation
          *                            (excluding headers, footers, etc...)
          * @param level_y_amount      The total count of levels (vertical units) on this page
          * @param pageHeight          Full height of the page
          * @param x0                  The minimum x coordinate at which printing can occur
          * @param x1                  The maximum x coordinate at which printing can occur
          */
        void printLinesInArea(wxDC& dc, LayoutPage& page, const float notation_area_y0, const float notation_area_h,
                              const int level_y_amount, const int pageHeight,
                              const int x0, const int x1);
    };
    
}

#endif