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

#ifndef __ABSTRACT_PRINTABLE_SEQUENCE_H__
#define __ABSTRACT_PRINTABLE_SEQUENCE_H__

class wxDC;
class wxString;

#include "Utils.h"
#include "ptr_vector.h"
#include "Editors/Editor.h"

namespace AriaMaestosa
{
    
    class Sequence;
    class Track;
    
    class AbstractPrintableSequence
    {
    protected:
        /** The parent sequence */
        Sequence* m_sequence;
        
        /** A list of all tracks added to be printed. */
        ptr_vector<Track, REF> m_tracks;        

        /** Whether 'calculateLayout' was called on this object */
        bool m_layout_calculated;
        
    public:
        
        /**
         * Constructor. Creates an empty printable sequence with the given parent.
         * Initially contains no track. Call 'addTrack' to add tracks to this print job.
         *
         * @param parent   Parent sequence of this printable sequence.
         *                 All tracks added later must belong to this Sequence.
         */
        AbstractPrintableSequence(Sequence* parent);
        
        virtual ~AbstractPrintableSequence() {}
        
        /**
         * Add a track (from the parent sequence of this object) to be printed.
         *
         * @pre   The print layout must not have been "frozen" yet with 'caclulateLayout'.
         * Derive classes will likely want to augment the behavior of this class.
         *
         * @param track    The track from the parent sequence to be added to this print job.
         * @param mode     The type of view to use for printing (score, tablature, etc...)
         * @return         whether the track was successfully added
         */
        virtual bool addTrack(Track* track, NotationType mode)
        {
            m_tracks.push_back(track);
            return true;
        }
        
        /** @return the Sequence object associated with this printable sequence */
        const Sequence* getSequence() const { return m_sequence; }
        
        /** @return the number of tracks added through 'addTrack' */
        const int getTrackAmount() const { return m_tracks.size();         }
        
        /**
         * Get access to the tracks added for printing. This method returns the tracks added through 'addTrack'.
         * @param id   ID of the track to get. Must be in range [0 .. getTrackAmount() - 1]
         * @return     The track
         */
        const Track* getTrack(const int id) const { return m_tracks.getConst(id); }
        
        /** @return the title of this sequence. If none was provided, makes up one from the context if it can */
        static wxString getTitle(const Sequence* seq);
        
        /**
         * @brief Prepare the abstract layout for this sequence.
         * Divides sequence in pages, decides contents of each line, etc.
         * Must be called before actually printing.
         * Children classes will likely want to augment the default behavior.
         */
        virtual void calculateLayout()
        {
            m_layout_calculated = true;
        }
        
        /**
         * @return the number of pages that was determined to be necessary in order to print this sequence.
         *         This information is only available after calling 'calculateLayout'; will return 0 before.
         */
        virtual int getPageAmount() const = 0;
        
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
         * @pre                       'calculateLayout' must have been called first.
         */
        virtual void printLinesInArea(wxDC& dc, const int page, const float notation_area_y0,
                                      const float notation_area_h, const int pageHeight,
                                      const int x0, const int x1) = 0;
        
        /** @return whether 'calculteLayout' was called on this object */
        bool isLayoutCalculated() const { return m_layout_calculated; }
        
        LEAK_CHECK()
        
    };
    
}

#endif
