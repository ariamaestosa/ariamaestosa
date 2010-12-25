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
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/RemoveMeasures.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "UnitTest.h"

using namespace AriaMaestosa;
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
    
    MeasureData* md = sequence->getMeasureData();

    // convert measures into midi ticks
    const int amountInTicks = m_amount * md->measureLengthInTicks(m_measure_ID);
    const int afterTick = md->firstTickInMeasure(m_measure_ID) - 1;
    
    DisplayFrame::changeMeasureAmount( md->getMeasureAmount() + m_amount );
    
    // move all notes that are after given start tick by the necessary amount
    const int trackAmount = sequence->getTrackAmount();
    for (int t=0; t<trackAmount; t++)
    {
        Track* track = sequence->getTrack(t);
        
        // ----------------- move note events -----------------
        const int noteAmount = track->m_notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (track->m_notes[n].startTick > afterTick)
            {
                track->m_notes[n].startTick += amountInTicks;
                track->m_notes[n].endTick += amountInTicks;
            }
        }
        // ----------------- move control events -----------------
        const int controlAmount = track->m_control_events.size();
        for (int n=0; n<controlAmount; n++)
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
        for (int n=0; n<tempo_event_amount; n++)
        {
            if (sequence->tempoEvents[n].getTick() > afterTick)
            {
                //std::cout << "starting at " << seq->tempoEvents[n].getTick() << std::endl;
                sequence->tempoEvents[n].setTick( sequence->tempoEvents[n].getTick() + amountInTicks );
            }
        }
    }
    
    // ----------------- move time sig changes -----------------
    if (not md->isMeasureLengthConstant())
    {
        
        const int timeSigAmount = md->getTimeSigAmount();
        for (int n=0; n<timeSigAmount; n++)
        {
            if (md->getTimeSig(n).getMeasure() >= m_measure_ID+1 and
                n != 0 /* dont move first time sig event*/ )
            {
                md->setTimesigMeasure(n, md->getTimeSig(n).getMeasure() + m_amount);
            }
        }//next
    }//endif
    
    md->updateMeasureInfo();
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Unit Tests
#endif

#if _MORE_DEBUG_CHECKS // so that utility classes are not compiled in when unit tests are disabled
namespace InsertMeasuresTest
{
    
    class TestSeqProvider : public ICurrentSequenceProvider
    {
    public:
        Sequence* m_seq;
        
        TestSeqProvider()
        {
            m_seq = new Sequence(NULL, NULL, NULL, false);
            AriaMaestosa::setCurrentSequenceProvider(this);
            
            Track* t = new Track(m_seq);
            
            const int beatLen = m_seq->ticksPerBeat();
            
            // make a factory sequence to work from
            m_seq->importing = true;
            for (int n=0; n<16; n++)
            {
                t->addNote_import(100 + n           /* pitch  */,
                                  n*beatLen         /* start  */,
                                  (n+1)*beatLen - 1 /* end    */,
                                  127               /* volume */, -1);
            }
            for (int n=0; n<32; n++)
            {
                t->addControlEvent_import((n*beatLen)/2 /* tick */, 64+n*2 /* value */,  0 /* controller */);
            }
            for (int n=0; n<32; n++)
            {
                t->addControlEvent_import((n*beatLen)/2 /* tick */, 64-n*2 /* value */,  1 /* controller */);
            }
            m_seq->importing = false;
            
            require_e(t->getNoteAmount(), ==, 16, "sanity check"); // sanity check on the way...
            require_e(t->getControllerEventAmount(0), ==, 32, "Controller events OK");
            require_e(t->getControllerEventAmount(1), ==, 32, "Controller events OK");

            m_seq->addTrack(t);            
        }
        
        ~TestSeqProvider()
        {
            delete m_seq;
        }
        
        virtual Sequence* getCurrentSequence()
        {
            return m_seq;
        }
        
        virtual GraphicalSequence* getCurrentGraphicalSequence()
        {
            return NULL;
        }
        
        void verifyUndo()
        {
            Track* t = m_seq->getTrack(0);
            
            const int beatLen = m_seq->ticksPerBeat();
            
            require(t->getNoteAmount() == 16, "the number of events is fine on undo");
            require(t->getNoteOffVector().size() == 16, "Note off vector is fine on undo");

            for (int n=0; n<16; n++)
            {
                require(t->getNote(n)->getTick()    == n*beatLen, "events were properly restored");
                require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly restored");
                require(t->getNoteOffVector()[n].endTick == (n + 1)*beatLen - 1,
                        "Note off vector was properly restored");
            }
            
            for (int n=0; n<32; n++)
            {
                require(t->getControllerEvent(n, 0 /* controller */)->getTick() == (n*beatLen)/2,
                        "control events were properly restored");
                require(t->getControllerEvent(n, 0 /* controller */)->getValue() == 64+n*2,
                        "control events were properly restored");
            }
            for (int n=0; n<32; n++)
            {
                require(t->getControllerEvent(n, 1 /* controller */)->getTick() == (n*beatLen)/2,
                        "control events were properly restored");
                require(t->getControllerEvent(n, 1 /* controller */)->getValue() == 64-n*2,
                        "control events were properly restored");
            }
        }
    };
    
    UNIT_TEST(TestInsert)
    {
        TestSeqProvider provider;
        Track* t = provider.m_seq->getTrack(0);
        
        // test the action
        provider.m_seq->action(new InsertEmptyMeasures(2 /* insert at */, 2 /* amount of measures to insert */));
        
        // TODO: test this action on multiple tracks, not only one
        // TODO: test this action on tempo events too
        
        // verify the data is OK        
        const int beatLen = provider.m_seq->ticksPerBeat();
        
        require(t->getNoteAmount() == 16, "the number of events is fine");
        require(t->getNoteOffVector().size() == 16, "Note off vector is fine");
        require(t->getControllerEventAmount(0) == 32, "Controller 0 events OK");
        require(t->getControllerEventAmount(1) == 32, "Controller 1 events OK");

        const int insertedShift = 2*(beatLen*4); // two measures of 4 beats were inserted

        // verify notes
        for (int n=0; n<8; n++)
        {
            require(t->getNote(n)->getTick()    == n*beatLen, "events were properly modified by action");
            require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly modified by action");
            require(t->getNoteOffVector()[n].endTick == (n + 1)*beatLen - 1,
                    "Note off vector was properly modified by action");
        }
        for (int n=8; n<16; n++)
        {
            require(t->getNote(n)->getTick() == insertedShift + n*beatLen, "events were properly modified by action");
            require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly modified by action");
            require(t->getNoteOffVector()[n].endTick == insertedShift + (n + 1)*beatLen - 1,
                    "Note off vector was properly modified by action");
        }
        
        // verify control events
        // FIXME: this fails because of the sloppy semantics of 'getControllerEvent'
        for (int n=0; n<16; n++)
        {
            require_e(t->getControllerEvent(n, 0 /* controller */)->getTick(), ==, (n*beatLen)/2,
                      "control events were properly modified");
            require_e(t->getControllerEvent(n, 0 /* controller */)->getValue(), ==, 64+n*2,
                      "control events were properly modified");
        }
        for (int n=0; n<16; n++)
        {
            require_e(t->getControllerEvent(n, 1 /* controller */)->getTick(), ==, (n*beatLen)/2,
                    "control events were properly modified");
            require_e(t->getControllerEvent(n, 1 /* controller */)->getValue(), ==, 64-n*2,
                    "control events were properly modified");
        }
        for (int n=16; n<32; n++)
        {
            require_e(t->getControllerEvent(n, 0 /* controller */)->getTick(), ==, insertedShift + (n*beatLen)/2,
                      "control events were properly modified");
            require_e(t->getControllerEvent(n, 0 /* controller */)->getValue(), ==, 64+n*2,
                      "control events were properly modified");
        }
        for (int n=16; n<32; n++)
        {
            require_e(t->getControllerEvent(n, 1 /* controller */)->getTick(), ==, insertedShift + (n*beatLen)/2,
                      "control events were properly modified");
            require_e(t->getControllerEvent(n, 1 /* controller */)->getValue(), ==, 64-n*2,
                      "control events were properly modified");
        }
        
        // verify undo
        provider.m_seq->undo();
        provider.verifyUndo();
    }
}
#endif
