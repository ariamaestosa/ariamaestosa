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

#include "wx/intl.h"

#include "AriaCore.h"
#include "Actions/EditAction.h"
#include "Actions/InsertEmptyMeasures.h"
#include "Actions/RemoveMeasures.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "unit_test.h"

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
            // FIXME: creating the graphics object shouldn't be manual nor necessary for tests
            t->graphics = new GraphicalTrack(t, m_seq);
            t->graphics->createEditors();
            
            MeasureData* measures = m_seq->measureData;
            const int beatLen = measures->beatLengthInTicks();
            
            // make a factory sequence to work from
            m_seq->importing = true;
            for (int n=0; n<16; n++)
            {
                t->addNote_import(100 + n           /* pitch  */,
                                  n*beatLen         /* start  */,
                                  (n+1)*beatLen - 1 /* end    */,
                                  127               /* volume */, -1);
            }
            m_seq->importing = false;
            
            require(t->getNoteAmount() == 16, "sanity check"); // sanity check on the way...
            
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
        
        void verifyUndo()
        {
            Track* t = m_seq->getTrack(0);
            
            MeasureData* measures = m_seq->measureData;
            const int beatLen = measures->beatLengthInTicks();
            
            require(t->getNoteAmount() == 16, "the number of events is fine on undo");
            require(t->getNoteOffVector().size() == 16, "Note off vector is fine on undo");

            for (int n=0; n<16; n++)
            {
                require(t->getNote(n)->getTick()    == n*beatLen, "events were properly restored");
                require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly restored");
                require(t->getNoteOffVector()[n].endTick == (n + 1)*beatLen - 1,
                        "Note off vector was properly restored");
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
        // TODO: test this action on control events too (and don't forget tempo events)
        
        // verify the data is OK        
        MeasureData* measures = provider.m_seq->measureData;
        const int beatLen = measures->beatLengthInTicks();
        
        require(t->getNoteAmount() == 16, "the number of events is fine on undo");
        require(t->getNoteOffVector().size() == 16, "Note off vector is fine on undo");
        
        for (int n=0; n<8; n++)
        {
            require(t->getNote(n)->getTick()    == n*beatLen, "events were properly modified by action");
            require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly modified by action");
            require(t->getNoteOffVector()[n].endTick == (n + 1)*beatLen - 1,
                    "Note off vector was properly modified by action");
        }
        const int insertedShift = 2*(beatLen*4); // two measures of 4 beats were inserted
        for (int n=8; n<16; n++)
        {
            require(t->getNote(n)->getTick() == insertedShift + n*beatLen, "events were properly modified by action");
            require(t->getNote(n)->getPitchID() == 100 + n,   "events were properly modified by action");
            require(t->getNoteOffVector()[n].endTick == insertedShift + (n + 1)*beatLen - 1,
                    "Note off vector was properly modified by action");
        }
        
        // verify undo
        provider.m_seq->undo();
        provider.verifyUndo();
    }
}
#endif
