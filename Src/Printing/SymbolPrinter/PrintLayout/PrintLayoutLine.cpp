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

#include "GUI/GraphicalTrack.h"
#include "Printing/AriaPrintable.h"
#include "Printing/SymbolPrinter/EditorPrintable.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutLine.h"
#include "Printing/SymbolPrinter/PrintLayout/LayoutElement.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/SymbolPrinter/SymbolPrintableSequence.h"

using namespace AriaMaestosa;

/** how many levels (vertical units) to leave between the tracks of a line */
const int INTER_TRACK_MARGIN_LEVELS = 3;

#pragma mark LineTrackRef
    
// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNote() const
{
    const int elements = m_parent->getLayoutElementCount();
    const int track_amount = m_parent->getTrackAmount();
    
    // start searching from last measure in this line
    for (int el=elements-1; el>=0; el--)
    {
        const PrintLayoutMeasure& current_meas = m_parent->getMeasureForElement(el);
        
        if (current_meas == NULL_MEASURE) continue;
        
        for (int i=0; i<track_amount; i++)
        {
            const MeasureTrackReference& ref = current_meas.getTrackRef(i);
            const GraphicalTrack* gtrack = ref.getConstTrack();
            
            if (gtrack == m_track and ref.getLastNote() != -1)
            {
                return ref.getLastNote();
            }
        }
        
    }
    return -1; // empty line
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNote() const
{
    const int track_amount = m_parent->getTrackAmount();
    const int elements = m_parent->getLayoutElementCount();
    
    for (int el=0; el<elements; el++)
    { 
        // start searching from first measure in this line
        const PrintLayoutMeasure& current_meas = m_parent->getMeasureForElement(el);
        for (int i=0; i<track_amount; i++)
        {
            if (current_meas == NULL_MEASURE) continue;
            
            const MeasureTrackReference& ref = current_meas.getTrackRef(i);
            const GraphicalTrack* gtrack = ref.getConstTrack();
            
            if (gtrack == m_track and ref.getFirstNote() != -1)
            {
                return ref.getFirstNote();
            }
        }
    }
    return -1; // empty line
    
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(const int layoutElementID)
{
    return m_parent->getMeasureForElement(layoutElementID).getTrackRef(m_track_id).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(const int layoutElementID)
{
    //std::cout << "last note in element " << layoutElementID << " of track " << m_track_id << " is "
    //          << m_parent->getMeasureForElement(layoutElementID).getTrackRef(m_track_id).getLastNote()
    //          << " from measure " << m_parent->getMeasureForElement(layoutElementID).getMeasureID() << std::endl;
    
    return m_parent->getMeasureForElement(layoutElementID).getTrackRef(m_track_id).getLastNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(LayoutElement* layoutElement)
{
    return m_parent->getMeasureForElement(layoutElement).getTrackRef(m_track_id).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(LayoutElement* layoutElement)
{
    return m_parent->getMeasureForElement(layoutElement).getTrackRef(m_track_id).getLastNote();
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark LayoutLine

LayoutLine::LayoutLine(SymbolPrintableSequence* parent, ptr_vector<PrintLayoutMeasure, REF>& measures)
{
    m_level_height = -1;
    m_printable = parent;
    m_measures = measures;
    
    m_level_from = -1;
    m_level_to = -1;
    
    const int trackAmount = parent->getTrackAmount();
    for (int trackID=0; trackID<trackAmount; trackID++)
    {
        LineTrackRef* newTrack = new LineTrackRef(this, trackID, m_printable->getTrack(trackID));
        
        m_tracks.push_back(newTrack);
    }
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getTrackAmount() const
{
    return m_printable->getTrackAmount();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstNoteInElement(const int trackID, const int layoutElementID) const
{
    return getMeasureForElement(layoutElementID).getTrackRef(trackID).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastNoteInElement(const int trackID, const int layoutElementID) const
{
    //std::cout << "last note in element " << layoutElementID << " of track " << trackID << " is "
    //          << getMeasureForElement(layoutElementID).getTrackRef(trackID).getLastNote()
    //          << " from measure " << getMeasureForElement(layoutElementID).getMeasureID() << std::endl;
    
    return getMeasureForElement(layoutElementID).getTrackRef(trackID).getLastNote();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstNoteInElement(const int trackID, const LayoutElement* layoutElement) const
{
    return getMeasureForElement(layoutElement).getTrackRef(trackID).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastNoteInElement(const int trackID, const LayoutElement* layoutElement) const
{
    return getMeasureForElement(layoutElement).getTrackRef(trackID).getLastNote();
}

// -------------------------------------------------------------------------------------------

const PrintLayoutMeasure& LayoutLine::getMeasureForElement(const int layoutElementID) const
{
    const int measID = m_layout_elements[layoutElementID].m_measure;
    if (measID == -1) return (PrintLayoutMeasure&)NULL_MEASURE;
    return m_measures.getRef(measID);
}

// -------------------------------------------------------------------------------------------

const PrintLayoutMeasure& LayoutLine::getMeasureForElement(const LayoutElement* layoutElement) const
{
    return m_measures[layoutElement->m_measure];
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastMeasure() const
{
    for (int n=m_layout_elements.size()-1; n>=0; n--)
    {
        if ( m_layout_elements[n].m_measure != -1) return m_layout_elements[n].m_measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstMeasure() const
{
    const int amount = m_layout_elements.size();
    for (int n=0; n<amount; n++)
    {
        if ( m_layout_elements[n].m_measure != -1) return m_layout_elements[n].m_measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::calculateHeight(const bool hideEmptyTracks)
{
    m_level_height = 0;
        
    bool nonEmptyTrackMetYet = false;
    
    // calculate the total height of this line (i.e. sum of heights of tracks within the line
    // PLUS empty space between the tracks)
    // FIXME(DESIGN): empty space calculation should go in PrintLayoutAbstract ?
    const int trackAmount = getTrackAmount();
    for (int n=0; n<trackAmount; n++)
    {
        bool empty = false;
        int this_height = m_printable->getEditorPrintable(n)->calculateHeight(n, m_tracks[n], *this, &empty);
        
        // hide hidden tracks
        if (empty and hideEmptyTracks) this_height = 0;
        
        // add space between tracks
        if (this_height > 0 and nonEmptyTrackMetYet)
        {
            m_level_height += INTER_TRACK_MARGIN_LEVELS;
        }
        
        m_tracks[n].setLevelFrom( m_level_height );
        m_level_height += this_height;
        m_tracks[n].setLevelTo( m_level_height );
        
        if (not nonEmptyTrackMetYet) nonEmptyTrackMetYet = (this_height > 0);
    }
    
    return m_level_height;
}
    
// -------------------------------------------------------------------------------------------

void LayoutLine::addLayoutElement( const LayoutElement& newElem )
{
    m_layout_elements.push_back( newElem );
}

