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

#include "Printing/AriaPrintable.h"
#include "Printing/EditorPrintable.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "Printing/PrintLayout/LayoutElement.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/PrintableSequence.h"

using namespace AriaMaestosa;

#pragma mark LineTrackRef
    
// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNote() const
{
    const int elements = parent->getLayoutElementCount();
    const int track_amount = parent->getTrackAmount();
    
    for (int el=elements-1; el>=0; el--)
    { // start searching from last measure in this line
        
        const PrintLayoutMeasure& current_meas = parent->getMeasureForElement(el);
        for (int i=0; i<track_amount; i++)
        {
            if (current_meas.getTrackRefAmount() > 0 && // FIXME - find why it's sometimes 0
                current_meas.getTrackRef(i).getConstTrack() == track &&
               current_meas.getTrackRef(i).getLastNote() != -1)
            {
                return current_meas.getTrackRef(i).getLastNote();
            }
        }
        
    }
    return -1; // empty line
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNote() const
{
    //const int measure = getFirstMeasure();
    // const int from_tick = getMeasureData()->firstTickInMeasure(measure);
    
    const int track_amount = parent->getTrackAmount();
    const int elements = parent->getLayoutElementCount();
    
    for (int el=0; el<elements; el++)
    { // start searching from first measure in this line
        const PrintLayoutMeasure& current_meas = parent->getMeasureForElement(el);
        for (int i=0; i<track_amount; i++)
        {
            if (current_meas.getTrackRefAmount() > 0 && // FIXME - find why it's sometimes empty
                current_meas.getTrackRef(i).getConstTrack() == track &&
                current_meas.getTrackRef(i).getFirstNote() != -1)
            {
                return current_meas.getTrackRef(i).getFirstNote();
            }
        }
    }
    return -1; // empty line
    
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(const int layoutElementID)
{
    return parent->getMeasureForElement(layoutElementID).getTrackRef(trackID).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(const int layoutElementID)
{
    std::cout << "last note in element " << layoutElementID << " of track " << trackID << " is "
        << parent->getMeasureForElement(layoutElementID).getTrackRef(trackID).getLastNote()
        << " from measure " << parent->getMeasureForElement(layoutElementID).getMeasureID() << std::endl;
    
    return parent->getMeasureForElement(layoutElementID).getTrackRef(trackID).getLastNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(LayoutElement* layoutElement)
{
    return parent->getMeasureForElement(layoutElement).getTrackRef(trackID).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(LayoutElement* layoutElement)
{
    return parent->getMeasureForElement(layoutElement).getTrackRef(trackID).getLastNote();
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark LayoutLine

LayoutLine::LayoutLine(PrintableSequence* parent, ptr_vector<PrintLayoutMeasure, REF>& measures)
{
    m_printable = parent;
    m_measures = measures;
    m_last_of_page = false;
    
    const int trackAmount = parent->getTrackAmount();
    for (int trackID=0; trackID<trackAmount; trackID++)
    {
        LineTrackRef* newTrack = new LineTrackRef(this, trackID);
        newTrack->track = m_printable->tracks.get(trackID);
        
        m_track_render_info.push_back(newTrack);
    }
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getTrackAmount() const
{
    return m_printable->tracks.size();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstNoteInElement(const int trackID, const int layoutElementID) const
{
    return getMeasureForElement(layoutElementID).getTrackRef(trackID).getFirstNote();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastNoteInElement(const int trackID, const int layoutElementID) const
{
    std::cout << "last note in element " << layoutElementID << " of track " << trackID << " is "
              << getMeasureForElement(layoutElementID).getTrackRef(trackID).getLastNote()
              << " from measure " << getMeasureForElement(layoutElementID).getMeasureID() << std::endl;
    
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
    const int measID = m_layout_elements[layoutElementID].measure;
    if (measID == -1) return (PrintLayoutMeasure&)NULL_MEASURE;
    return m_measures.getRef(measID);
}

// -------------------------------------------------------------------------------------------

const PrintLayoutMeasure& LayoutLine::getMeasureForElement(const LayoutElement* layoutElement) const
{
    return m_measures[layoutElement->measure];
}

// -------------------------------------------------------------------------------------------

LineTrackRef& LayoutLine::getLineTrackRef(const int trackID)
{
    assertExpr(trackID,>=,0);
    assertExpr(trackID,<,(int)m_track_render_info.size());
    return m_track_render_info[trackID];
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastMeasure() const
{
    for (int n=m_layout_elements.size()-1; n>=0; n--)
    {
        if ( m_layout_elements[n].measure != -1) return m_layout_elements[n].measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstMeasure() const
{
    const int amount = m_layout_elements.size();
    for (int n=0; n<amount; n++)
    {
        if ( m_layout_elements[n].measure != -1) return m_layout_elements[n].measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::calculateHeight()
{
    m_level_height = 0;
    
    std::vector<int> heights;
    
    /* calculate the total height of this line (which many include multiple tracks */
    std::cout << "---- line ----" << std::endl;
    const int trackAmount = getTrackAmount();
    for (int n=0; n<trackAmount; n++)
    {
        const int this_height = m_printable->getEditorPrintable(n)->calculateHeight(n, m_track_render_info[n], *this);
        heights.push_back(this_height);
        m_level_height += this_height;
        std::cout << this_height << "-high" << std::endl;
    }
    
    /* distribute the vertical space between tracks (some track need more vertical space than others) */
    for (int n=0; n<trackAmount; n++)
    {
        m_height_percent.push_back( (int)round( (float)heights[n] * 100.0f / (float)m_level_height ) );
        std::cout << m_height_percent[n] << "%" << std::endl;
    }
    
    // if we're the last of the page, we need less space cause we don't
    // need to leave empty space under
    if (m_last_of_page) m_level_height -= 13;
    
    return m_level_height;
}
    
