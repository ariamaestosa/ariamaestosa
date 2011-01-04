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


#ifndef __PRINTABLE_SEQUENCE_H__
#define __PRINTABLE_SEQUENCE_H__

#include "Editors/Editor.h"
#include "Printing/AbstractPrintableSequence.h"
#include "Printing/SymbolPrinter/PrintLayout/LayoutPage.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutNumeric.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutAbstract.h"

class wxDC;

namespace AriaMaestosa
{
    class Track;
    class EditorPrintable;
    class LayoutLine;
    
    /**
      * @brief   public interface to the high-level layout/printing module
      *
      * This object is used to keep track of what tracks to print, and coordinates
      * the layouting of the sequence. One object of this type must be constructed
      * to print anything.
      * @ingroup printing
      */
    class SymbolPrintableSequence : public AbstractPrintableSequence
    {
        ptr_vector<LayoutPage> layoutPages;
        
        OwnerPtr<PrintLayoutAbstract> m_abstract_layout_manager;
        OwnerPtr<PrintLayoutNumeric>  m_numeric_layout_manager;
        
        
        void printLine(LayoutLine& line, wxDC& dc);
        
        /** Whether at least one track with guitar view was added */
        bool m_is_guitar_editor_used;
        
        /** Whether at least one track with score view was added */
        bool m_is_score_editor_used;
        
        /** Holds one EditorPrintable derivate for each track added to this printing job */
        ptr_vector<EditorPrintable> m_editor_printables;

        //int track_amount;

        /** the max number of sharp/flats we'll need to display at header (useful to allocate proper size) */
        int m_max_signs_in_keysig;

    public:
                
        /**
          * Constructor. Creates an empty printable sequence with the given parent.
          * Initially contains no track. Call 'addTrack' to add tracks to this print job.
          *
          * @param parent   Parent sequence of this printable sequence.
          *                 All tracks added later must belong to this Sequence.
          */
        SymbolPrintableSequence(Sequence* parent);
        
        /**
          * @brief Prepare the abstract layout for this sequence.
          * Divides sequence in pages, decides contents of each line, etc.
          * Must be called before actually printing.
          */
        virtual void calculateLayout();
        
        /**
          * @brief         Get the EditorPrintable derivate corresponding to the requested track
          * @param trackID Local ID of the track for which you want the editor printable (local
          *                means it's the ID within this class, NOT the ID within the original Sequence)
          * @return        the EditorPrintable derivate corresponding to the requested track
          */
        EditorPrintable* getEditorPrintable(const int trackID);
        
        /**
         * Add a track (from the parent sequence of this object) to be printed.
         *
         * @pre   The print layout must not have been "frozen" yet with 'caclulateLayout'.
         *
         * @param track    The track from the parent sequence to be added to this print job.
         * @param mode     The type of view to use for printing (score, tablature, etc...)
         * @return         whether the track was successfully added
         */
        virtual bool addTrack(GraphicalTrack* track, NotationType mode);
        
        /** @return the max number of sharp/flats we'll need to display at header (useful to allocate proper size) */
        int getMaxKeySignatureSignCount() const { return m_max_signs_in_keysig; }
        
        /**
          * @return the number of pages that was determined to be necessary in order to print this sequence.
          *         This information is only available after calling 'calculateLayout'; will return 0 before.
          */
        virtual int getPageAmount() const;
        
        /** 
          * @pre Only call after calling 'calculateLayout'.
          *
          * @param id     ID pf the page to retrieve. Must be in range [0 .. getPageAmount() - 1].
          * @return       the nth page of this printable sequence.
          */
        LayoutPage& getPage(const int id);
        
        /** 
          * @pre only meaningful if called after all tracks have been added through 'addTrack'
          * @return whether at least one track with guitar view was added
          */
        bool isGuitarEditorUsed () const { return m_is_guitar_editor_used; }
        
        /**
          * @pre only meaningful if called after all tracks have been added through 'addTrack'
          * @return whether at least one track with score view was added
          */
        bool isScoreEditorUsed  () const { return m_is_score_editor_used;  }


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
          * @pre              'calculateLayout' must have been called first.
          */
        virtual void printLinesInArea(wxDC& dc, const int page, const float notation_area_y0,
                                      const float notation_area_h, const int pageHeight,
                                      const int x0, const int x1);

    };
    
}

#endif