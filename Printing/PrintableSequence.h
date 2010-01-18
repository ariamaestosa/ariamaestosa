

#ifndef __PRINTABLE_SEQUENCE_H__
#define __PRINTABLE_SEQUENCE_H__


#include "Printing/PrintLayout.h"
#include "Printing/PrintLayoutLine.h"

namespace AriaMaestosa
{
    class Track;
    class EditorPrintable;
    
    class PrintableSequence
    {
        ptr_vector<LayoutPage> layoutPages;
        
        OwnerPtr<PrintLayoutManager> layout;
        
        Sequence* sequence;
        
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
        
    };
    
}

#endif