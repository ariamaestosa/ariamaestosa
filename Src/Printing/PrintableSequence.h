

#ifndef __PRINTABLE_SEQUENCE_H__
#define __PRINTABLE_SEQUENCE_H__

#include "Editors/Editor.h"
#include "Printing/PrintLayout/LayoutPage.h"
#include "Printing/PrintLayout/PrintLayoutNumeric.h"
#include "Printing/PrintLayout/PrintLayoutAbstract.h"

class wxDC;

namespace AriaMaestosa
{
    class Track;
    class EditorPrintable;
    class LayoutLine;
    
    class PrintableSequence
    {
        ptr_vector<LayoutPage> layoutPages;
        
        OwnerPtr<PrintLayoutAbstract> m_abstract_layout_manager;
        OwnerPtr<PrintLayoutNumeric> m_numeric_layout_manager;
        
        /** The parent sequence */
        Sequence* m_sequence;
        
        void printLine(LayoutLine& line, wxDC& dc);
        
        /** Whether at least one track with guitar view was added */
        bool m_is_guitar_editor_used;
        
        /** Whether at least one track with score view was added */
        bool m_is_score_editor_used;

        /** Whether 'calculateLayout' was called on this object */
        bool m_layout_calculated;
        
        /** Holds one EditorPrintable derivate for each track added to this printing job */
        ptr_vector<EditorPrintable> m_editor_printables;

        //int track_amount;

        /** the max number of sharp/flats we'll need to display at header (useful to allocate proper size) */
        int m_max_signs_in_keysig;

        /** A list of all tracks added to be printed. This is vector is parallel to 'm_editor_printables'. */
        ptr_vector<Track, REF> m_tracks;        

    public:
                
        /**
          * Constructor. Creates an empty printable sequence with the given parent.
          * Initially contains no track. Call 'addTrack' to add tracks to this print job.
          *
          * @param parent   Parent sequence of this printable sequence.
          *                 All tracks added later must belong to this Sequence.
          */
        PrintableSequence(Sequence* parent);
        
        /** @return the Sequence object associated with this printable sequence */
        const Sequence* getSequence() const { return m_sequence; }
        
        /**
          * Add a track (from the parent sequence of this object) to be printed.
          *
          * @precondition   The print layout must not have been "frozen" yet with 'caclulateLayout'.
          *
          * @param track    The track from the parent sequence to be added to this print job.
          * @param mode     The type of view to use for printing (score, tablature, etc...)
          */
        bool addTrack(Track* track, EditorType mode);
        
        /**
          * Prepare the abstract layout for this sequence; divides sequence in pages, decides contents of
          * each line, etc. Must be called before actually printing.
          */
        void calculateLayout(bool checkRepetitions);
        
        /**
          * @param trackID Local ID of the track for which you want the editor printable (local
          *                means it's the ID within this class, NOT the ID within the original Sequence)
          * @return        the EditorPrintable derivate corresponding to the requested track
          */
        EditorPrintable* getEditorPrintable(const int trackID);
        
        /** @return the title of this sequence. If none was provided, makes up one from the context if it can */
        wxString getTitle() const;
        
        /** @return the max number of sharp/flats we'll need to display at header (useful to allocate proper size) */
        int getMaxKeySignatureSignCount() const { return m_max_signs_in_keysig; }
        
        /**
          * @return the number of pages that was determined to be necessary in order to print this sequence.
          *         This information is only available after calling 'calculateLayout'; will return 0 before.
          */
        int getPageAmount() const;
        
        /** 
          * @precondition Only call after calling 'calculateLayout'.
          *
          * @param id     ID pf the page to retrieve. Must be in range [0 .. getPageAmount() - 1].
          * @return       the nth page of this printable sequence.
          */
        LayoutPage& getPage(const int id);
        
        /** 
          * @precondition only meaningful if called after all tracks have been added through 'addTrack'
          * @return whether at least one track with guitar view was added
          */
        bool isGuitarEditorUsed () const { return m_is_guitar_editor_used; }
        
        /**
          * @precondition only meaningful if called after all tracks have been added through 'addTrack'
          * @return whether at least one track with score view was added
          */
        bool isScoreEditorUsed  () const { return m_is_score_editor_used;  }
        
        /** @return the number of tracks added through 'addTrack' */
        const int getTrackAmount() const { return m_tracks.size();         }
        
        /**
          * Get access to the tracks added for printing. This method returns the tracks added through 'addTrack'.
          * @param id   ID of the track to get. Must be in range [0 .. getTrackAmount() - 1]
          * @return     The track
          */
        const Track* getTrack(const int id) const { return m_tracks.getConst(id); }

        /**
          * Call when it is time to print all lines of a page.
          *
          * @param dc                  The wxDC onto which printing occurs
          * @param page                The page that contains the lines we want to print now
          * @param notation_area_y0    Minimum Y coordinate of the actual area where track notation is drawn
          *                            (excluding header/etc...)
          * @param notation_area_h     Height of the area that is actually printable with notation
          *                            (excluding headers, footers, etc...)
          * @param pageHeight          Full height of the page
          * @param x0                  The minimum x coordinate at which printing can occur
          * @param x1                  The maximum x coordinate at which printing can occur
          *
          * @precondition              'calculateLayout' must have been called first.
          */
        void printLinesInArea(wxDC& dc, LayoutPage& page, const float notation_area_y0, const float notation_area_h,
                              const int pageHeight, const int x0, const int x1);
        
        /** @return whether 'calculteLayout' was called on this object */
        bool isLayoutCalculated() const { return m_layout_calculated; }
    };
    
}

#endif