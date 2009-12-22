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
#include <map>
#include "ptr_vector.h"
#include "wx/wx.h"

#include "Printing/PrintLayoutLine.h"
#include "Printing/LayoutElement.h"
#include "Printing/PrintLayoutMeasure.h"


namespace AriaMaestosa
{
    class Track;
    class PrintableSequence;
    class PrintLayoutMeasure;
    
    int getRepetitionMinimalLength();
    void setRepetitionMinimalLength(const int newvalue);
    
    struct LayoutPage
    {
        DECLARE_MAGIC_NUMBER();
        
        ptr_vector<LayoutLine> layoutLines;    
        
        LayoutPage()
        {
            INIT_MAGIC_NUMBER();
        }

        const int getLineCount() const
        {
            assert( MAGIC_NUMBER_OK() );
            return layoutLines.size();
        }
        
        LayoutLine& getLine(const int lineID)
        {
            assert( MAGIC_NUMBER_OK() );
            assert( MAGIC_NUMBER_OK_FOR(&layoutLines) );

            return layoutLines[lineID];
        }
    };
    
    
    /**
     * For non-linear printing.
     * Each tick where there is a note/silence/something else is marked with this structure.
     * The 'proportion' argument allows giving more space for a specific tick, e.g. if there's
     * something that takes more space to draw there.
     */
    struct TickPosInfo
    {
        float relativePosition;
        float relativeEndPosition;
        std::map<int /* track ID */, float /* proportion */> proportions;
        
        /** Keeps in which track this tick is actually used */
        //wxUint64 tracks;
        
        // don't construct this struct directly, let std::map create them as needed
        TickPosInfo()
        {
            TickPosInfo::relativePosition = 0; // will be set later
            //TickPosInfo::proportion = 1;
            //TickPosInfo::tracks = 0;
        }
        
        /** Adds information about a tick (how big the symbol there is, in which track, etc.) */
        void setProportion(float proportion, int fromTrackID)
        {
            assertExpr(fromTrackID,>=,0);
            assertExpr(fromTrackID,<,64);
            
            const bool alreadyInThere = proportions.find(fromTrackID) != proportions.end();
            if (alreadyInThere)
            {
                proportions[fromTrackID] = std::max(proportions[fromTrackID], proportion);
            }
            else
            {
                proportions[fromTrackID] = proportion;
            }
        }
        
        /** Returns whether this tick appears in a specific track. */
        bool appearsInTrack(int trackID)
        {
            assertExpr(trackID,>=,0);
            assertExpr(trackID,<,64);
            return proportions.find(trackID) != proportions.end();
        }
        
    };
    
    class PrintLayoutManager
        {
            PrintableSequence* sequence;
            
            // referencing vectors from PrintableSequence (FIXME: no two objects should reference these vectors)
            ptr_vector<LayoutPage>& layoutPages;

            ptr_vector<PrintLayoutMeasure> measures; 
            std::vector<LayoutElement> layoutElements;
            
            void layInLinesAndPages();
            
            void calculateRelativeLengths();
            
            void createLayoutElements(bool checkRepetitions_bool);
            
            void findSimilarMeasures();
            
            
        public:
            PrintLayoutManager(PrintableSequence* parent,
                               ptr_vector<LayoutPage>& layoutPages  /* out */);
            
            void generateMeasures(ptr_vector<Track, REF>& tracks);

            
            void calculateLayoutElements(ptr_vector<Track, REF>& track, const bool checkRepetitions_bool);
        };
    
}

#endif
