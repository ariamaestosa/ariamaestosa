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

#include "Actions/RemoveMeasures.h"
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/EditAction.h"
#include "IO/IOUtils.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/TimeSigChange.h"
#include "Midi/MeasureData.h"
#include "AriaCore.h"

#include <iostream>

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

RemoveMeasures::RemoveMeasures(int from_measure, int to_measure) :
    //I18N: (undoable) action name
    MultiTrackAction( _("remove measure(s)") )
{
    m_from_measure = from_measure;
    m_to_measure = to_measure;
}

// ----------------------------------------------------------------------------------------------------------

RemoveMeasures::~RemoveMeasures()
{
}

// ----------------------------------------------------------------------------------------------------------

RemoveMeasures::RemovedTrackPart::~RemovedTrackPart()
{
}

// ----------------------------------------------------------------------------------------------------------

void RemoveMeasures::undo()
{
    Action::InsertEmptyMeasures opposite_action(m_from_measure, (m_to_measure - m_from_measure));
    opposite_action.setParentSequence( m_sequence, m_visitor->clone() );
    opposite_action.perform();
    
    const int rmtrackamount = removedTrackParts.size();
    for (int rm=0; rm<rmtrackamount; rm++)
    {
        RemovedTrackPart* removedBits = removedTrackParts.get(rm);
        
        // add removed notes again
        const int n_amount = removedBits->removedNotes.size();
        for (int n=0; n<n_amount; n++)
        {
            removedBits->track->addNote( removedBits->removedNotes.get(n) );
        }
        // we are using the notes again, so make sure it won't delete them
        removedBits->removedNotes.clearWithoutDeleting();
        
        // add removed control events again
        const int c_amount = removedBits->removedControlEvents.size();
        for (int n=0; n<c_amount; n++)
        {
            removedBits->track->addControlEvent( removedBits->removedControlEvents.get(n) );
        }
        // we are using the events again, so make sure it won't delete them
        removedBits->removedControlEvents.clearWithoutDeleting();
    }
    
    // add removed tempo events again
    const int s_amount = removedTempoEvents.size();
    for (int n=0; n<s_amount; n++)
    {
        m_sequence->addTempoEvent( removedTempoEvents.get(n) );
    }
    // we will be using the events again, make sure it doesn't delete them
    removedTempoEvents.clearWithoutDeleting();
    
    // add removed text events again
    const int text_event_amount = removedTextEvents.size();
    for (int n=0; n<text_event_amount; n++)
    {
        m_sequence->addTextEvent( removedTextEvents.get(n) );
    }
    removedTextEvents.clearWithoutDeleting();
    
    // add removed time sig events again
    MeasureData* md = m_sequence->getMeasureData();
    const int t_amount = timeSigChangesBackup.size();
    
    {
        ScopedMeasureTransaction tr(md->startTransaction());
        
        // add back the backup copy of measure events
        for (int n=0; n<t_amount; n++)
        {
            //timeSigChangesBackup.push_back( measureBar->getTimeSig(n) );
            tr->addTimeSigChange(timeSigChangesBackup[n].getMeasure(),
                                 timeSigChangesBackup[n].getNum(),
                                 timeSigChangesBackup[n].getDenom());
        }
        
    } // end transaction
}

// ----------------------------------------------------------------------------------------------------------

void RemoveMeasures::perform()
{
    
    ASSERT(m_sequence != NULL);
    
    MeasureData* md = m_sequence->getMeasureData();
    
    // find the range of ticks that need to be removed (convert measure IDs to midi ticks)
    const int fromTick = md->firstTickInMeasure(m_from_measure) - 1;
    const int toTick   = md->firstTickInMeasure(m_to_measure);
    
    // find the amount of ticks that will be removed. This will be used to move back notes located
    // after the area that is removed.
    const int amountInTicks = toTick - fromTick - 1;
    
    const int trackAmount = m_sequence->getTrackAmount();
    for (int t=0; t<trackAmount; t++)
    {
        RemovedTrackPart* removedBits = new RemovedTrackPart();
        removedTrackParts.push_back( removedBits );
        
        Track* track = m_sequence->getTrack(t);
        removedBits->track = track;
        
        OwnerPtr<Track::TrackVisitor> tvisitor(m_visitor->getNewTrackVisitor(t));
        ptr_vector<Note>& notes = tvisitor->getNotesVector();
        
        // ------------------------ erase/move notes ------------------------
        const int amount_n = notes.size();
        for (int n=0; n<amount_n; n++)
        {
            Note* note = notes.get(n);
            
            // note is an area that is removed. remove it.
            if (note->getTick() > fromTick and note->getTick() < toTick)
            {
                removedBits->removedNotes.push_back(note);
                track->markNoteToBeRemoved(n);
            }
            // note is in after the removed area. move it back by necessary amound
            else if (notes[n].getTick() >= toTick)
            {
                note->setTick( note->getTick() - amountInTicks);
                note->setEndTick( note->getEndTick() - amountInTicks);
            }
        }
        track->removeMarkedNotes();
        
        // ------------------------ erase/move control events ------------------------
        
        ptr_vector<ControllerEvent>& ctrl = tvisitor->getControlEventVector();
        
        const int c_amount = ctrl.size();
        for (int n=0; n<c_amount; n++)
        {
            // delete all controller events located in the area to be deleted
            if (ctrl[n].getTick() > fromTick and ctrl[n].getTick() < toTick)
            {
                removedBits->removedControlEvents.push_back( ctrl.get(n) );
                ctrl.markToBeRemoved(n);
            }
            // move all controller events that are after given start tick by the necessary amount
            else if (ctrl[n].getTick() >= toTick)
            {
                ctrl[n].setTick(ctrl[n].getTick() - amountInTicks);
            }
        }
        ctrl.removeMarked();
        track->reorderNoteVector();
        track->reorderNoteOffVector();
        
    }
    
    
    // ------------------------ erase/move tempo events ------------------------
    const int s_amount = m_sequence->getTempoEventAmount();

    if (s_amount > 0)
    {
        for (int n=0; n<s_amount; n++)
        {
            const int tick = m_sequence->getTempoEvent(n)->getTick();
            
            // event is in deleted area
            if (tick > fromTick and tick < toTick)
            {
                removedTempoEvents.push_back( m_sequence->extractTempoEvent(n) );
            }
            //event is after deleted area
            else if (tick >= toTick)
            {
                // move it back
                m_sequence->setTempoEventTick( n, tick - (toTick - fromTick - 1) );
            }
        }
        
        m_sequence->removeMarkedTempoEvents();
    }
    
    
    // ------------------------ erase/move text events ------------------------
    const int text_event_amount = m_sequence->getTextEventAmount();

    if (text_event_amount > 0)
    {
        for (int n=0; n<text_event_amount; n++)
        {
            const int tick = m_sequence->getTextEvent(n)->getTick();
            
            // event is in deleted area
            if (tick > fromTick and tick < toTick)
            {
                removedTextEvents.push_back( m_sequence->extractTextEvent(n) );
            }
            //event is after deleted area
            else if (tick >= toTick)
            {
                // move it back
                m_sequence->setTextEventTick( n, tick - (toTick - fromTick - 1) );
            }
        }
        
        m_sequence->removeMarkedTextEvents();
    }
    
    {
        ScopedMeasureTransaction tr(md->startTransaction());
        
        // ----------------------- erase/move time sig events ------------------------
        timeSigChangesBackup.clear();
        
        if (not md->isMeasureLengthConstant())
        {
            
            // keep a backup copy of measure events
            for (int n=0; n<md->getTimeSigAmount(); n++)
            {
                timeSigChangesBackup.push_back( md->getTimeSig(n) );
            }
            
            
            for (int n=0; n<md->getTimeSigAmount(); n++)
            {
                
                if (md->getTimeSig(n).getMeasure() >= m_from_measure and
                    md->getTimeSig(n).getMeasure() <= m_to_measure )
                {
                    // an event is located in the area we are trying to remove.
                    // check if there are measures after the deleted area that still need this event.
                    if ((n < md->getTimeSigAmount() - 1 and
                         md->getTimeSig(n+1).getMeasure() > m_to_measure) or
                        n == md->getTimeSigAmount() - 1)
                    {
                        // dont move if its already there
                        if (md->getTimeSig(n).getMeasure() == m_from_measure) continue; 
                        
                        // check if there already was an event there if so remove it
                        for (int i=0; i<md->getTimeSigAmount(); i++)
                        {
                            if (md->getTimeSig(i).getMeasure() == m_from_measure)
                            {
                                tr->eraseTimeSig(i);
                                i -= 2; if (i<-1) i=-1;
                            }
                        }
                        
                        tr->setTimesigMeasure(n, m_from_measure); // move back event to its new location
                        
                    }
                    else
                    {
                        tr->eraseTimeSig(n);
                    }
                    n -= 2; if (n<-1) n=-1; // restart a bit earlier cause order in vector changed
                    continue;
                    
                }
                
                if (md->firstTickInMeasure( md->getTimeSig(n).getMeasure() ) >= toTick)
                {
                    const int new_measure = md->getTimeSig(n).getMeasure() -
                                           (m_to_measure - m_from_measure);
                    
                    // check if there already was an event there if so remove it
                    for (int i=0; i<md->getTimeSigAmount(); i++)
                    {
                        if ( i != n and md->getTimeSig(i).getMeasure() == new_measure)
                        {
                            tr->eraseTimeSig(i);
                            i -= 2; if (i<0) i=0;
                        }
                    }
                    
                    tr->setTimesigMeasure(n, new_measure);
                }
                
            }//next
                
            
        }//endif
        
        // shorten song accordingly to the number of measures removed
        tr->setMeasureAmount( md->getMeasureAmount() - (m_to_measure - m_from_measure) );
            
    } // end transaction
}

// ----------------------------------------------------------------------------------------------------------


