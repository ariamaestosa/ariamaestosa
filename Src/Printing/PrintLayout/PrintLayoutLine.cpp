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
    
    for(int el=elements-1; el>=0; el--)
    { // start searching from last measure in this line
        PrintLayoutMeasure& current_meas = parent->getMeasureForElement(el);
        for(int i=0; i<track_amount; i++)
        {
            if (current_meas.trackRef.size() > 0 && // FIXME - find why it's sometimes 0
               current_meas.trackRef[i].track == track &&
               current_meas.trackRef[i].lastNote != -1)
            {
                return current_meas.trackRef[i].lastNote;
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
    
    for(int el=0; el<elements; el++)
    { // start searching from first measure in this line
        PrintLayoutMeasure& current_meas = parent->getMeasureForElement(el);
        for(int i=0; i<track_amount; i++)
        {
            if (current_meas.trackRef.size() > 0 && // FIXME - find why it's sometimes empty
               current_meas.trackRef[i].track == track &&
               current_meas.trackRef[i].firstNote != -1)
                return current_meas.trackRef[i].firstNote;
        }
    }
    return -1; // empty line
    
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(const int layoutElementID)
{
    return parent->getMeasureForElement(layoutElementID).trackRef[trackID].firstNote;
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(const int layoutElementID)
{
    std::cout << "last note in element " << layoutElementID << " of track " << trackID << " is " <<
    parent->getMeasureForElement(layoutElementID).trackRef[trackID].lastNote << " from measure " <<
    parent->getMeasureForElement(layoutElementID).id << std::endl;
    return parent->getMeasureForElement(layoutElementID).trackRef[trackID].lastNote;
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getFirstNoteInElement(LayoutElement* layoutElement)
{
    return parent->getMeasureForElement(layoutElement).trackRef[trackID].firstNote;
}

// -------------------------------------------------------------------------------------------

int LineTrackRef::getLastNoteInElement(LayoutElement* layoutElement)
{
    return parent->getMeasureForElement(layoutElement).trackRef[trackID].lastNote;
}

// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark LayoutLine
// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------

LayoutLine::LayoutLine(PrintableSequence* parent, ptr_vector<PrintLayoutMeasure, REF>& measures)
{
    this->printable = parent;
    this->measures = measures;
    last_of_page = false;
    
    for (int trackID=0; trackID<parent->track_amount; trackID++)
    {
        LineTrackRef* newTrack = new LineTrackRef(this, trackID);
        newTrack->track = printable->tracks.get(trackID);
        
        trackRenderInfo.push_back(newTrack);
    }
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getTrackAmount() const
{
    return printable->tracks.size();
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstNoteInElement(const int trackID, const int layoutElementID)
{
    return getMeasureForElement(layoutElementID).trackRef[trackID].firstNote;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastNoteInElement(const int trackID, const int layoutElementID)
{
    std::cout << "last note in element " << layoutElementID << " of track " << trackID << " is " <<
    getMeasureForElement(layoutElementID).trackRef[trackID].lastNote << " from measure " <<
    getMeasureForElement(layoutElementID).id << std::endl;
    return getMeasureForElement(layoutElementID).trackRef[trackID].lastNote;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstNoteInElement(const int trackID, LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[trackID].firstNote;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastNoteInElement(const int trackID, LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[trackID].lastNote;
}

// -------------------------------------------------------------------------------------------

PrintLayoutMeasure& LayoutLine::getMeasureForElement(const int layoutElementID) const
{
    const int measID = layoutElements[layoutElementID].measure;
    if (measID == -1) return (PrintLayoutMeasure&)nullMeasure;
    return measures.getRef(measID);
}

// -------------------------------------------------------------------------------------------

PrintLayoutMeasure& LayoutLine::getMeasureForElement(LayoutElement* layoutElement)
{
    return measures[layoutElement->measure];
}

// -------------------------------------------------------------------------------------------

LineTrackRef& LayoutLine::getLineTrackRef(const int trackID)
{
    assertExpr(trackID,>=,0);
    assertExpr(trackID,<,(int)trackRenderInfo.size());
    return trackRenderInfo[trackID];
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getLastMeasure() const
{
    for(int n=layoutElements.size()-1; n>=0; n--)
    {
        if ( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::getFirstMeasure() const
{
    const int amount = layoutElements.size();
    for(int n=0; n<amount; n++)
    {
        if ( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------

int LayoutLine::calculateHeight()
{
    level_height = 0;
    
    std::vector<int> heights;
    
    /* calculate the total height of this line (which many include multiple tracks */
    std::cout << "---- line ----" << std::endl;
    const int trackAmount = getTrackAmount();
    for(int n=0; n<trackAmount; n++)
    {
        const int this_height = printable->editorPrintables.get(n)->calculateHeight(n, trackRenderInfo[n], *this);
        heights.push_back(this_height);
        level_height += this_height;
        std::cout << this_height << "-high" << std::endl;
    }
    
    /* distribute the vertical space between tracks (some track need more vertical space than others) */
    for(int n=0; n<trackAmount; n++)
    {
        height_percent.push_back( (int)round( (float)heights[n] * 100.0f / (float)level_height ) );
        std::cout << height_percent[n] << "%" << std::endl;
    }
    
    // if we're the last of the page, we need less space cause we don't
    // need to leave empty space under
    if (last_of_page) level_height -= 13;
    
    return level_height;
}
    
