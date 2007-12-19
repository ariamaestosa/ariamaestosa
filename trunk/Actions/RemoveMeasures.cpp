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
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/TimeSigChange.h"
#include "GUI/MeasureBar.h"
#include "main.h"

#include <iostream>
#include "IO/IOUtils.h"

namespace AriaMaestosa
{
	namespace Action
{
	void RemoveMeasures::undo()
    {
		Action::InsertEmptyMeasures opposite_action(from_measure, to_measure-from_measure);
		opposite_action.setParentSequence( sequence );
		opposite_action.perform();
		
		const int rmtrackamount = removedTrackParts.size();
		for(int rm=0; rm<rmtrackamount; rm++)
		{
			RemovedTrackPart* undo = removedTrackParts.get(rm);
			
			// add removed notes again
			const int n_amount = undo->removedNotes.size();
			for(int n=0; n<n_amount; n++)
			{
				undo->track->addNote( undo->removedNotes.get(n) );
			}
			undo->removedNotes.clearWithoutDeleting();
			
			// add removed control events again
			const int c_amount = undo->removedControlEvents.size();
			for(int n=0; n<c_amount; n++)
			{
				undo->track->addControlEvent( undo->removedControlEvents.get(n) );
			}
			undo->removedControlEvents.clearWithoutDeleting();
		}
		
		// add removed tempo events again
		const int s_amount = removedTempoEvents.size();
		for(int n=0; n<s_amount; n++)
		{
			sequence->addTempoEvent( removedTempoEvents.get(n) );
		}
		removedTempoEvents.clearWithoutDeleting();
		
		// add removed time sig events again
		
		MeasureBar* measureBar = getMeasureBar();
		const int t_amount = timeSigChangesBackup.size();
		
		// keep a backup copy of measure events
		for(int n=0; n<t_amount; n++)
		{
			//timeSigChangesBackup.push_back( measureBar->getTimeSig(n) );
			measureBar->addTimeSigChange( timeSigChangesBackup[n].measure,
										  timeSigChangesBackup[n].num,
										  timeSigChangesBackup[n].denom);
		}
		
		measureBar->timeSigEventsUpdated();
		
	}
	
	void RemoveMeasures::perform()
	{
		
		assert(sequence != NULL);
		
		// find the range of ticks that need to be removed (convert measure IDs to midi ticks)
		const int from_tick = getMeasureBar()->firstTickInMeasure(from_measure) - 1;
		const int to_tick = getMeasureBar()->firstTickInMeasure(to_measure);
		
		// find the amount of ticks that will be removed. This will be used to move back notes located after the area that is removed.
		const int amountInTicks = to_tick - from_tick - 1;
		
		const int trackAmount = sequence->getTrackAmount();
		for(int t=0; t<trackAmount; t++)
		{
			RemovedTrackPart* undo = new RemovedTrackPart();
			removedTrackParts.push_back( undo );
			
			Track* track = sequence->getTrack(t);
			undo->track = track;
			
			// ------------------------ erase/move notes ------------------------
			const int amount_n = track->notes.size();
			for(int n=0; n<amount_n; n++)
			{
				// note is an area that is removed. remove it.
				if(track->notes[n].startTick > from_tick and track->notes[n].startTick < to_tick)
				{
					undo->removedNotes.push_back(track->notes.get(n));
					track->markNoteToBeRemoved(n); 
				}
				// note is in after the removed area. move it back by necessary amound
				else if(undo->track->notes[n].startTick >= to_tick)
				{
					track->notes[n].startTick -= amountInTicks;
					track->notes[n].endTick -= amountInTicks;
				}
			}
			track->removeMarkedNotes();
			
			// ------------------------ erase/move control events ------------------------
			const int c_amount = track->controlEvents.size();
			for(int n=0; n<c_amount; n++)
			{
				// delete all controller events located in the area to be deleted
				if(track->controlEvents[n].getTick() > from_tick and track->controlEvents[n].getTick() < to_tick)
				{
					undo->removedControlEvents.push_back( track->controlEvents.get(n) );
					track->controlEvents.markToBeRemoved(n);
				}
				// move all controller events that are after given start tick by the necessary amount
				else if(track->controlEvents[n].getTick() >= to_tick)
				{
					undo->track->controlEvents[n].setTick( track->controlEvents[n].getTick() - amountInTicks );
				}
			}
			track->controlEvents.removeMarked();
			track->reorderNoteVector();
			track->reorderNoteOffVector();
			
		}
		
		
		// ------------------------ erase/move tempo events ------------------------
		if(sequence->tempoEvents.size()>0)
		{
			const int s_amount = sequence->tempoEvents.size();
			for(int n=0; n<s_amount; n++)
			{
				// event is in deleted area
				if(sequence->tempoEvents[n].getTick() > from_tick and sequence->tempoEvents[n].getTick() < to_tick)
				{
					removedTempoEvents.push_back( sequence->tempoEvents.get(n) );
					sequence->tempoEvents.markToBeRemoved(n);
				}
				//event is after deleted area
				else if(sequence->tempoEvents[n].getTick() >= to_tick)
				{
					// move it back
					sequence->tempoEvents[n].setTick( sequence->tempoEvents[n].getTick() - (to_tick-from_tick-1) );
				}
			}
			
			sequence->tempoEvents.removeMarked();
		}
		
		// ----------------------- erase/move time sig events ------------------------
		MeasureBar* measureBar = getMeasureBar();
		timeSigChangesBackup.clear();
		
		if(!measureBar->isMeasureLengthConstant())
		{
			// keep a backup copy of measure events
			for(int n=0; n<measureBar->getTimeSigAmount(); n++)
			{
				timeSigChangesBackup.push_back( measureBar->getTimeSig(n) );
			}
			
			
			for(int n=0; n<measureBar->getTimeSigAmount(); n++)
			{
				
				if( measureBar->getTimeSig(n).measure >= from_measure and measureBar->getTimeSig(n).measure <= to_measure )
				{
					// an event is located in the area we are trying to remove.
					// check if there are measures after the deleted area that still need this event.
					if(
					   (n<measureBar->getTimeSigAmount()-1 and measureBar->getTimeSig(n+1).measure > to_measure)
					   or
					   n==measureBar->getTimeSigAmount()-1)
					{
						if(measureBar->getTimeSig(n).measure == from_measure) continue; // dont move if its already there
						
						// check if there already was an event there if so remove it
						for(int i=0; i<measureBar->getTimeSigAmount(); i++)
						{
							if( measureBar->getTimeSig(i).measure == from_measure)
							{
								//removedTimeSigChanges.push_back( measureBar->removeTimeSig(i) );
								measureBar->eraseTimeSig(i);
								i -= 2; if(i<-1) i=-1;
							}
						}
						
						measureBar->getTimeSig(n).measure = from_measure; // move back event to its new location
						
					}
					else
					{
						//removedTimeSigChanges.push_back( measureBar->removeTimeSig(n) );
						measureBar->eraseTimeSig(n);
					}
					n -= 2; if(n<-1) n=-1; // restart a bit earlier cause order in vector changed
					continue;
					
				}
				
				if( measureBar->getTimeSig(n).tick >= to_tick )
				{
					const int new_measure = measureBar->getTimeSig(n).measure - (to_measure - from_measure);
					// check if there already was an event there if so remove it
					for(int i=0; i<measureBar->getTimeSigAmount(); i++)
					{
						if( i!=n and measureBar->getTimeSig(i).measure == new_measure)
						{
							//removedTimeSigChanges.push_back( measureBar->removeTimeSig(i) );
							measureBar->eraseTimeSig(i);
							i -= 2; if(i<0) i=0;
						}
					}
					
					measureBar->getTimeSig(n).measure = new_measure;
				}
				
			}//next
		}//endif
		
		// shorten song accordingly to the number of measures removed
		getMainFrame()->changeMeasureAmount( sequence->measureBar->getMeasureAmount() - (to_measure - from_measure) );
		getMeasureBar()->timeSigEventsUpdated();
}
RemoveMeasures::RemoveMeasures(int from_measure, int to_measure)
{
	RemoveMeasures::from_measure = from_measure;
	RemoveMeasures::to_measure = to_measure;
}
RemoveMeasures::~RemoveMeasures()
{
	removedTempoEvents.clearAndDeleteAll();
	removedTrackParts.clearAndDeleteAll();
}

RemovedTrackPart::~RemovedTrackPart()
{
	removedNotes.clearAndDeleteAll();
	removedControlEvents.clearAndDeleteAll();
}

}
}
