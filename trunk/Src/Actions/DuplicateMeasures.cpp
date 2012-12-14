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

#include <wx/intl.h>

#include "AriaCore.h"
#include "Actions/EditAction.h"
#include "Actions/DuplicateMeasures.h"
#include "Actions/RemoveMeasures.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "UnitTest.h"

#include <map>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

DuplicateMeasures::DuplicateMeasures(int fromMeasure, int toMeasure) :
    //I18N: (undoable) action name
    MultiTrackAction( _("duplicate measures") )
{
    m_fromMeasure = fromMeasure;
    m_toMeasure = toMeasure;
}

// --------------------------------------------------------------------------------------------------------

DuplicateMeasures::~DuplicateMeasures()
{
}

// --------------------------------------------------------------------------------------------------------

void DuplicateMeasures::undo()
{
    Action::RemoveMeasures opposite_action( m_fromMeasure, m_toMeasure );
    opposite_action.setParentSequence( m_sequence, m_visitor->clone() );
    opposite_action.perform();
}

// --------------------------------------------------------------------------------------------------------

void DuplicateMeasures::perform()
{
    ASSERT(m_sequence != NULL);
    
    MeasureData* md = m_sequence->getMeasureData();

    int amount = m_toMeasure - m_fromMeasure;

    // convert measures into midi ticks
    const int amountInTicks = amount * md->measureLengthInTicks(m_fromMeasure);
    const int afterTick = md->firstTickInMeasure(m_fromMeasure) - 1;
    
    const int stopDuplicatingAtTick = md->firstTickInMeasure(m_toMeasure);
    
    {
        ScopedMeasureTransaction tr(md->startTransaction());
        
        // abusing the import feature to add notes sorting
        OwnerPtr<Sequence::Import> import(m_sequence->startImport());
        
        tr->setMeasureAmount( md->getMeasureAmount() + amount );
    
        std::vector<Note> notesToDuplicate;
        std::map<Track*, std::vector<ControllerEvent> > controllerEventsToDuplicate;
        std::vector<ControllerEvent> tempoEventsToDuplicate;
        
        // move all notes that are after given start tick by the necessary amount
        const int trackAmount = m_sequence->getTrackAmount();
        for (int t=0; t<trackAmount; t++)
        {
            Track* track = m_sequence->getTrack(t);
            
            // ----------------- note events -----------------
            const int noteAmount = track->getNoteAmount();
            for (int n=0; n<noteAmount; n++)
            {
                Note* note = track->getNote(n);
                if (note->getTick() > afterTick)
                {
                    if (note->getTick() < stopDuplicatingAtTick)
                    {
                        // duplicate
                        notesToDuplicate.push_back(Note(track, note->getPitchID(), note->getTick(),
                                                        note->getEndTick(), note->getVolume(),
                                                        note->getString()));
                    }
                    
                    note->setTick(note->getTick() + amountInTicks);
                    note->setEndTick(note->getEndTick() + amountInTicks);
                    
                }
            }
            
            // ----------------- control events -----------------
            OwnerPtr<Track::TrackVisitor> tvisitor(m_visitor->getNewTrackVisitor(t));
            ptr_vector<ControllerEvent>& ctrl = tvisitor->getControlEventVector();
            
            const int controlAmount = ctrl.size();
            for (int n=0; n<controlAmount; n++)
            {
                if (ctrl[n].getTick() > afterTick)
                {
                    if (ctrl[n].getTick() < stopDuplicatingAtTick)
                    {
                        controllerEventsToDuplicate[track].push_back(ControllerEvent(ctrl[n].getController(),
                                                                                     ctrl[n].getTick(),
                                                                                     ctrl[n].getValue()));
                    }
                    
                    ctrl[n].setTick( ctrl[n].getTick() + amountInTicks );
                }
            }
        }
        
        // ----------------- move tempo events -----------------
        const int tempo_event_amount = m_sequence->getTempoEventAmount();
        if (tempo_event_amount>0)
        {
            //const int first_tick = getMeasureData()->firstTickInMeasure(measureID+1) - 1;
            //const int amountInTicks = amount * getMeasureData()->measureLengthInTicks(measureID+1);
            for (int n=0; n<tempo_event_amount; n++)
            {
                const int tick = m_sequence->getTempoEvent(n)->getTick();
                if (tick > afterTick)
                {
                    if (tick < stopDuplicatingAtTick)
                    {
                        tempoEventsToDuplicate.push_back(ControllerEvent(m_sequence->getTempoEvent(n)->getController(),
                                                                         tick, m_sequence->getTempoEvent(n)->getValue()));
                    }
                    
                    //std::couts << "starting at " << seq->tempoEvents[n].getTick() << std::endl;
                    m_sequence->setTempoEventTick(n, tick + amountInTicks);
                }
            }
        }
        
        // ----------------- move time sig changes -----------------
        if (not md->isMeasureLengthConstant())
        {
            const int timeSigAmount = md->getTimeSigAmount();
            for (int n=0; n<timeSigAmount; n++)
            {
                int measure = md->getTimeSig(n).getMeasure();
                if (measure >= m_fromMeasure + 1 and n != 0 /* dont move first time sig event */)
                {
                    if (measure <= m_toMeasure)
                    {
                        
                    }
                    
                    tr->setTimesigMeasure(n, md->getTimeSig(n).getMeasure() + amount);
                }
            }//next
        }//endif
        
        for (size_t n = 0; n < notesToDuplicate.size(); n++)
        {
            notesToDuplicate[n].getParent()->addNote_import(notesToDuplicate[n].getPitchID(),
                                                            notesToDuplicate[n].getTick(),
                                                            notesToDuplicate[n].getEndTick(), 
                                                            notesToDuplicate[n].getVolume(),
                                                            notesToDuplicate[n].getString());
        }
        
        for (std::map<Track*, std::vector<ControllerEvent> >::iterator it = controllerEventsToDuplicate.begin();
             it != controllerEventsToDuplicate.end();
             it++)
        {
            for (size_t n = 0; n < it->second.size(); n++)
            {
                ControllerEvent& evt = it->second[n];
                it->first->addControlEvent_import(evt.getTick(), evt.getValue(), evt.getController());
            }
        }
        
        for (size_t n = 0; n < tempoEventsToDuplicate.size(); n++)
        {
            m_sequence->addTempoEvent(new ControllerEvent(tempoEventsToDuplicate[n].getController(),
                                                          tempoEventsToDuplicate[n].getTick(),
                                                          tempoEventsToDuplicate[n].getValue()));
        }
    } // end transaction
    
    for (int n = 0; n < m_sequence->getTrackAmount(); n++)
    {
        m_sequence->getTrack(n)->reorderNoteVector();
        m_sequence->getTrack(n)->reorderNoteOffVector();
        m_sequence->getTrack(n)->reorderControlVector();
    }
    
    
}


