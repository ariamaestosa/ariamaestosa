

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
        
        void printLinesInArea(wxDC& dc, LayoutPage& page, const int text_height, const float track_area_height,
                              const int level_y_amount, const int pageHeight,
                              const int x0, const int y0, const int x1);
    };
    
}

#endif