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

#include "Actions/InsertEmptyMeasures.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Actions/RemoveMeasures.h"
#include "AriaCore.h"

using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

InsertEmptyMeasures::InsertEmptyMeasures(int measureID, int amount) :
    //I18N: (undoable) action name
    MultiTrackAction( _("insert measures") )
{
    m_measure_ID = measureID;
    m_amount = amount;
}

// --------------------------------------------------------------------------------------------------------

InsertEmptyMeasures::~InsertEmptyMeasures()
{
}

// --------------------------------------------------------------------------------------------------------

void InsertEmptyMeasures::undo()
{
    Action::RemoveMeasures opposite_action( m_measure_ID, m_measure_ID + m_amount );
    opposite_action.setParentSequence( sequence );
    opposite_action.perform();
    //undo_obj.restoreState(track);
}

// --------------------------------------------------------------------------------------------------------

void InsertEmptyMeasures::perform()
{
    ASSERT(sequence != NULL);
    
    // convert measures into midi ticks
    const int amountInTicks = m_amount * getMeasureData()->measureLengthInTicks(m_measure_ID);
    const int afterTick = getMeasureData()->firstTickInMeasure(m_measure_ID) - 1;
    
    DisplayFrame::changeMeasureAmount( getMeasureData()->getMeasureAmount() + m_amount );
    
    // move all notes that are after given start tick by the necessary amount
    const int trackAmount = sequence->getTrackAmount();
    for(int t=0; t<trackAmount; t++)
    {
        Track* track = sequence->getTrack(t);
        
        // ----------------- move note events -----------------
        const int noteAmount = track->m_notes.size();
        for(int n=0; n<noteAmount; n++)
        {
            if (track->m_notes[n].startTick > afterTick)
            {
                track->m_notes[n].startTick += amountInTicks;
                track->m_notes[n].endTick += amountInTicks;
            }
        }
        // ----------------- move control events -----------------
        const int controlAmount = track->m_control_events.size();
        for(int n=0; n<controlAmount; n++)
        {
            if (track->m_control_events[n].getTick() > afterTick)
            {
                track->m_control_events[n].setTick( track->m_control_events[n].getTick() + amountInTicks );
            }
        }
        
        track->reorderNoteVector();
        track->reorderNoteOffVector();
    }
    
    // ----------------- move tempo events -----------------
    const int tempo_event_amount = sequence->tempoEvents.size();
    if (tempo_event_amount>0)
    {
        //const int first_tick = getMeasureData()->firstTickInMeasure(measureID+1) - 1;
        //const int amountInTicks = amount * getMeasureData()->measureLengthInTicks(measureID+1);
        for(int n=0; n<tempo_event_amount; n++)
        {
            if (sequence->tempoEvents[n].getTick() > afterTick)
            {
                //std::cout << "starting at " << seq->tempoEvents[n].getTick() << std::endl;
                sequence->tempoEvents[n].setTick( sequence->tempoEvents[n].getTick() + amountInTicks );
            }
        }
    }
    
    // ----------------- move time sig changes -----------------
    if (!getMeasureData()->isMeasureLengthConstant())
    {
        const int timeSigAmount = getMeasureData()->getTimeSigAmount();
        for(int n=0; n<timeSigAmount; n++)
        {
            if ( getMeasureData()->getTimeSig(n).measure >= m_measure_ID+1 and
                n!=0 /* dont move first time sig event*/ )
            {
                getMeasureData()->getTimeSig(n).measure += m_amount;
            }
        }//next
    }//endif
    
    getMeasureData()->updateMeasureInfo();
}

// --------------------------------------------------------------------------------------------------------


