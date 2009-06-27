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
#include "LayoutTree.h"
#include "AriaCore.h"
#include "Midi/MeasureData.h"
#include "Midi/Track.h"
#include "Printing/PrintingBase.h"

namespace AriaMaestosa
{
const MeasureToExport nullMeasure(-1);

MeasureToExport::MeasureToExport(const int measID)
{
    shortestDuration = -1;
    firstSimilarMeasure = -1;
    cutApart = false;
    id = measID;
    
    if(measID != -1)
    {
        firstTick = getMeasureData()->firstTickInMeasure( measID );
        lastTick = getMeasureData()->lastTickInMeasure( measID );
    }
}

bool MeasureToExport::calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure)
{
    
    const int trackRefAmount = trackRef.size();
    int total_note_amount = 0;
    
    for(int tref=0; tref<trackRefAmount; tref++)
    {
        const int my_first_note = trackRef[tref].firstNote;
        const int my_last_note = trackRef[tref].lastNote;
        const int his_first_note = checkMeasure.trackRef[tref].firstNote;
        const int his_last_note = checkMeasure.trackRef[tref].lastNote;
        
        assert( trackRef.size() == checkMeasure.trackRef.size() );
        assertExpr( tref,<,(int)trackRef.size() );
        assert( trackRef[tref].track == checkMeasure.trackRef[tref].track );
        Track* track = trackRef[tref].track;
        
        // if these 2 measures don't even have the same number of notes, they're definitely not the same
        if( (his_last_note - his_first_note + 1) != (my_last_note - my_first_note + 1) )
        {
            return false;
        }
        
        
        const int noteAmount = (his_last_note - his_first_note);
        total_note_amount += noteAmount;
        
        // don't count empty measures as repetitions 
        if(noteAmount<1)
        {
            // when comparing a multiple-track line, don't stop on empty measures, for other tracks may not be empty
            // for multi-track lines, variable 'total_note_amount' will be checked at the end to verify measure is not empty
            if(trackRefAmount>1) continue;
            return false;
        }
        
        /*
         if we get till there, the 2 measures have the same amount of notes.
         to know whether they are truly identitcal, we need to compare note by note
         we will match each notes from the first measure to the identical one in the second.
         If ever one note failes to be matched, then the 2 measures are different.
         */
        int noteMatched_this[noteAmount];
        int noteMatched_other[noteAmount];
        for(int n=0; n<noteAmount; n++)
        {
            noteMatched_this[n] = false;
            noteMatched_other[n] = false;
        }
        
        for(int checkNote_this=0; checkNote_this<=noteAmount; checkNote_this++)
        {
            for(int checkNote_other=0; checkNote_other<=noteAmount; checkNote_other++)
            {
                if(noteMatched_other[checkNote_other]) continue; // this note was already matched
                
                // check start tick matches
                if(track->getNoteStartInMidiTicks(his_first_note + checkNote_other) - checkMeasure.firstTick !=
                   track->getNoteStartInMidiTicks(my_first_note + checkNote_this) - firstTick)
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check end tick matches
                if(track->getNoteEndInMidiTicks(his_first_note + checkNote_other) - checkMeasure.firstTick !=
                   track->getNoteEndInMidiTicks(my_first_note + checkNote_this) - firstTick)
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check pitch matches
                if(track->getNotePitchID(his_first_note + checkNote_other) !=
                   track->getNotePitchID(my_first_note + checkNote_this))
                {
                    // they dont match, check the next one
                    continue;
                }
                
                noteMatched_this[checkNote_this] = true;
                noteMatched_other[checkNote_other] = true;
                
                
            }//next note
            
            // we couldn't find a note in the other measure that matches this one
            if(noteMatched_this[checkNote_this] == false)
            {
                // std::cout << "  not the same cause couldn't find a note in the other measure that matches this one\n";
                return false;
            }
            
        }//next note
        
    } // next track reference
    
    if(total_note_amount == 0) return false; // don't count empty measures as repeitions
    return true;
}

int MeasureToExport::addTrackReference(const int firstNote, Track* track)
{
    const int noteAmount = track->getNoteAmount();
    
    MeasureTrackReference* newTrackRef = new MeasureTrackReference();
    newTrackRef->track = track;
    
    // if firstNote is -1, it means all notes were processed. just add the track ref without searching for notes
    if(firstNote == -1)
    {
        newTrackRef->firstNote = -1;
        newTrackRef->lastNote = -1;
        trackRef.push_back( newTrackRef );
        return -1;
    }
    
    // first note in measure
    newTrackRef->firstNote = firstNote;
    if(firstNote >= noteAmount) newTrackRef->firstNote = noteAmount-1;
    
    // find what the first, last and shortest note in current measure
    int lastNote = firstNote;
    int last_note_end = -1, last_note_start = -1;
    
    bool measure_empty = true;
    for(int note=newTrackRef->firstNote; note<noteAmount; note++)
    {
        const int start_tick = track->getNoteStartInMidiTicks(note);
        const int end_tick = track->getNoteEndInMidiTicks(note);
        const int currentNoteDuration = end_tick - start_tick;
        
        if(currentNoteDuration <= 0)  continue; // skip malformed notes if any
        
        // stop when we're at next measure
        if( start_tick >= lastTick ) break;
        
        // find last note - if many notes end at the same time, keep the one that started last
        if(start_tick > last_note_start || end_tick > last_note_start ||
           (end_tick == last_note_end && start_tick >= last_note_start)
           )
        {
            lastNote = note;
            last_note_end = end_tick;
            last_note_start = start_tick;
            measure_empty = false;
        }
        
        // store duration if it's the shortest yet (but ignore dead/instant-hit notes)
        const float relativeLength = (end_tick - start_tick) / (float)(getMeasureData()->beatLengthInTicks()*4);
        if( relativeLength < 1.0/32.0 ) continue;
        if( currentNoteDuration < shortestDuration or shortestDuration==-1) shortestDuration = currentNoteDuration;
    }
    assertExpr(lastNote,>,-1);
    assertExpr(lastNote,<,noteAmount);
    
    if(measure_empty)
    {
        newTrackRef->firstNote = -1;
        newTrackRef->lastNote = -1;
    }
    else newTrackRef->lastNote = lastNote; // ID of the last note in this measure
    
    //std::cout << "measure " << id << " empty=" << measure_empty << " from=" << newTrackRef->firstNote << " to=" << newTrackRef->lastNote << std::endl;
    
    // std::cout << "--- measure " << (id+1) << " ranges from note  "<< newTrackRef->firstNote << " to " << newTrackRef->lastNote << std::endl;
    trackRef.push_back( newTrackRef );
    
    // check if all notes were used
    if(lastNote == noteAmount-1) return -1;
    
    // if this measure is empty, return the same note as the one given in input (i.e. it was not used)
    // if this measure is not empty, add 1 so next measure will start from the next
    return lastNote + ( measure_empty ? 0 : 1);
}

#if 0
#pragma mark -
#endif

// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility
LayoutElement::LayoutElement(LayoutElementType type_arg, int measure_arg)
{
    type = type_arg;
    measure = measure_arg;
    
    x = -1;
    x2 = -1;
}

#if 0
#pragma mark -
#endif

LayoutLine::LayoutLine(AriaPrintable* parent)
{
    printable = parent;
    currentTrack = 0;
    last_of_page = false;
    
    while((int)trackRenderInfo.size() < parent->track_amount)
        trackRenderInfo.push_back(TrackRenderInfo());
}

int LayoutLine::getTrackAmount() const
{
    return printable->tracks.size();
}
void LayoutLine::setCurrentTrack(const int n)
{
    currentTrack = n;
}
Track* LayoutLine::getTrack() const
{
    assertExpr(currentTrack,>=,0);
    assertExpr(currentTrack,<,printable->tracks.size());
    return printable->tracks.get(currentTrack);
}
int LayoutLine::getFirstNoteInElement(const int layoutElementID)
{
    return getMeasureForElement(layoutElementID).trackRef[currentTrack].firstNote;
}
int LayoutLine::getLastNoteInElement(const int layoutElementID)
{
    std::cout << "last note in element " << layoutElementID << " of track " << currentTrack << " is " << getMeasureForElement(layoutElementID).trackRef[currentTrack].lastNote << " from measure " << getMeasureForElement(layoutElementID).id << std::endl;
    return getMeasureForElement(layoutElementID).trackRef[currentTrack].lastNote;
}
int LayoutLine::getFirstNoteInElement(LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[currentTrack].firstNote;
}
int LayoutLine::getLastNoteInElement(LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[currentTrack].lastNote;
}

MeasureToExport& LayoutLine::getMeasureForElement(const int layoutElementID) const
{
    const int measID = layoutElements[layoutElementID].measure;
    if(measID == -1) return (MeasureToExport&)nullMeasure;
    return printable->measures[measID];
}
MeasureToExport& LayoutLine::getMeasureForElement(LayoutElement* layoutElement)
{
    return printable->measures[layoutElement->measure];
}
    
TrackRenderInfo& LayoutLine::getTrackRenderInfo()
{
    assertExpr(currentTrack,>=,0);
    assertExpr(currentTrack,<,(int)trackRenderInfo.size());
    return trackRenderInfo[currentTrack];
}

int LayoutLine::getLastMeasure() const
{
    for(int n=layoutElements.size()-1; n>=0; n--)
    {
        if( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}
int LayoutLine::getFirstMeasure() const
{
    const int amount = layoutElements.size();
    for(int n=0; n<amount; n++)
    {
        if( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}
int LayoutLine::getLastNote() const
{
    const int track_amount = getTrackAmount();
    const Track* t = getTrack();
    
    const int elements = layoutElements.size();
    for(int el=elements-1; el>=0; el--)
    { // start searching from last measure in this line
        MeasureToExport& current_meas = getMeasureForElement(el);
        for(int i=0; i<track_amount; i++)
        {
            if(current_meas.trackRef.size() > 0 && // FIXME - find why it's sometimes 0
               current_meas.trackRef[i].track == t &&
               current_meas.trackRef[i].lastNote != -1)
            {
                return current_meas.trackRef[i].lastNote;
            }
        }
    }
    return -1; // empty line
}

int LayoutLine::getFirstNote() const
{
    //const int measure = getFirstMeasure();
    // const int from_tick = getMeasureData()->firstTickInMeasure(measure);
    
    const int track_amount = getTrackAmount();
    
    const Track* t = getTrack();
    
    const int elements = layoutElements.size();
    for(int el=0; el<elements; el++)
    { // start searching from first measure in this line
        MeasureToExport& current_meas = getMeasureForElement(el);
        for(int i=0; i<track_amount; i++)
        {
            if(current_meas.trackRef.size() > 0 && // FIXME - find why it's sometimes empty
               current_meas.trackRef[i].track == t &&
               current_meas.trackRef[i].firstNote != -1)
                return current_meas.trackRef[i].firstNote;
        }
    }
    return -1; // empty line
    
}
int LayoutLine::calculateHeight()
{
    level_height = 0;
    
    std::vector<int> heights;
    
    /* calculate the total height of this line (which many include multiple tracks */
    std::cout << "---- line ----" << std::endl;
    const int trackAmount = getTrackAmount();
    for(int n=0; n<trackAmount; n++)
    {
        setCurrentTrack(n);
        const int this_height = printable->editorPrintables.get(currentTrack)->calculateHeight(*this);
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
    if(last_of_page) level_height -= 13;
    
    return level_height;
}


}

