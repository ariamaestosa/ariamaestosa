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

#include "AriaCore.h"
#include "Actions/DeleteSelected.h"
#include "Actions/EditAction.h"
#include "Midi/Note.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"

// FIXME(DESIGN): this action should not rely on GUI classes
#include "GUI/GraphicalTrack.h"
#include "Editors/ControllerEditor.h"

#include "UnitTest.h"
#include <cmath>

#include <wx/intl.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

DeleteSelected::DeleteSelected() :
    //I18N: (undoable) action name
    SingleTrackAction( _("delete note(s)") )
{
}

// ----------------------------------------------------------------------------------------------------------

DeleteSelected::~DeleteSelected()
{
}

// ----------------------------------------------------------------------------------------------------------

void DeleteSelected::undo()
{
    const int noteAmount    = removedNotes.size();
    const int controlAmount = removedControlEvents.size();
    
    if (noteAmount > 0)
    {
        
        for (int n=0; n<noteAmount; n++)
        {
            m_track->addNote( removedNotes.get(n), false );
        }
        // we will be using the notes again, make sure it doesn't delete them
        removedNotes.clearWithoutDeleting();
        
    }
    else if (controlAmount > 0)
    {
        
        for (int n=0; n<controlAmount; n++)
        {
            m_track->addControlEvent( removedControlEvents.get(n) );
        }
        // we will be using the notes again, make sure it doesn't delete them
        removedControlEvents.clearWithoutDeleting();
        
    }
}

// ----------------------------------------------------------------------------------------------------------

void DeleteSelected::perform()
{
    ASSERT(m_track != NULL);
    
    // FIXME: controllers need an exceptional treatment
    if (m_track->getNotationType() == CONTROLLER)
    {
        
        ptr_vector<ControllerEvent>& ctrls = m_visitor->getControlEventVector();
        
        ControllerEditor* editor = m_track->getGraphics()->getControllerEditor();
        int selBegin   = editor->getSelectionBegin();
        int selEnd     = editor->getSelectionEnd();
        const int type = editor->getCurrentControllerType();

        const int from = std::min(selBegin, selEnd);
        const int to   = std::max(selBegin, selEnd);
        
        if (type != 201 /*tempo*/)
        {
            // remove controller events
            for (int n=0; n<ctrls.size(); n++)
            {
                
                if (ctrls[n].getController() != type) continue; // in another controller
                
                const int tick = ctrls[n].getTick();
                
                if (tick < from or tick > to) continue; // this event is not concerned by selection
                
                removedControlEvents.push_back( ctrls.get(n) );
                ctrls.remove(n);
                n--;
            }//next
            
        }
        else
        {
            // remove tempo events
            Sequence* sequence = m_track->getSequence();
            const int tempoEventsAmount = sequence->getTempoEventAmount();
            for (int n=0; n<tempoEventsAmount; n++)
            {
                const int tick = sequence->getTempoEvent(n)->getTick();
                
                if (tick < from or tick > to) continue; // this event is not concerned by selection
                
                removedControlEvents.push_back( sequence->extractTempoEvent(n) );
            }//next
            sequence->removeMarkedTempoEvents();
        }
        
    }
    else
    {
        ptr_vector<Note>& notes        = m_visitor->getNotesVector();
        ptr_vector<Note, REF>& noteOff = m_visitor->getNoteOffVector();

        for (int n=0; n<notes.size(); n++)
        {
            if (not notes[n].isSelected()) continue;
            
            // also delete corresponding note off event
            for (int i=0; i<noteOff.size(); i++)
            {
                if (noteOff.get(i) == notes.get(n))
                {
                    noteOff.remove(i);
                    break;
                }
            }
            
            //notes.erase(n);
            removedNotes.push_back( notes.get(n) );
            notes.remove(n);
            
            n--;
        }//next
        
    }
    
    m_track->reorderNoteOffVector();
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Unit Tests
#endif

#if _MORE_DEBUG_CHECKS // so that utility classes are not compiled in when unit tests are disabled
namespace DeleteSelectedTest
{

    class TestSeqProvider : public ICurrentSequenceProvider
    {
    public:
        Sequence* m_seq;
        
        TestSeqProvider()
        {
            m_seq = new Sequence(NULL, NULL, NULL, NULL, false);
            AriaMaestosa::setCurrentSequenceProvider(this);
            
            Track* t = new Track(m_seq);

            // make a factory sequence to work from
            {
                OwnerPtr<Sequence::Import> import(m_seq->startImport());
                t->addNote_import(100 /* pitch */, 0   /* start */, 100 /* end */, 127 /* volume */, -1);
                t->addNote_import(101 /* pitch */, 101 /* start */, 200 /* end */, 127 /* volume */, -1);
                t->addNote_import(102 /* pitch */, 201 /* start */, 300 /* end */, 127 /* volume */, -1);
                t->addNote_import(103 /* pitch */, 301 /* start */, 400 /* end */, 127 /* volume */, -1);
            }
            
            require(t->getNoteAmount() == 4, "sanity check"); // sanity check on the way...
            
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
            
            require(t->getNoteAmount() == 4, "the number of events was restored on undo");
            require(t->getNote(0)->getTick()    == 0,   "events were properly ordered");
            require(t->getNote(0)->getPitchID() == 100, "events were properly ordered");
            
            require(t->getNote(1)->getTick()    == 101, "events were properly ordered");
            require(t->getNote(1)->getPitchID() == 101, "events were properly ordered");
            
            require(t->getNote(2)->getTick()    == 201, "events were properly ordered");
            require(t->getNote(2)->getPitchID() == 102, "events were properly ordered");
            
            require(t->getNote(3)->getTick()    == 301, "events were properly ordered");
            require(t->getNote(3)->getPitchID() == 103, "events were properly ordered");
            
            require(t->getNoteOffVector().size() == 4, "Note off vector was restored on undo");
            require(t->getNoteOffVector()[0].getEndTick() == 100, "Note off vector is properly ordered");
            require(t->getNoteOffVector()[1].getEndTick() == 200, "Note off vector is properly ordered");
            require(t->getNoteOffVector()[2].getEndTick() == 300, "Note off vector is properly ordered");
            require(t->getNoteOffVector()[3].getEndTick() == 400, "Note off vector is properly ordered");  
        }
    };
    
    UNIT_TEST(TestDelete)
    {
        TestSeqProvider provider;
        Track* t = provider.m_seq->getTrack(0);
        
        t->selectNote(1, true);
        t->selectNote(2, true);

        // test the action
        t->action(new DeleteSelected());
        
        require(t->getNoteAmount() == 2, "the number of events was decreased");
        require(t->getNote(0)->getTick()    == 0,   "events were properly ordered");
        require(t->getNote(0)->getPitchID() == 100, "events were properly ordered");
        
        require(t->getNote(1)->getTick()    == 301, "events were properly ordered");
        require(t->getNote(1)->getPitchID() == 103, "events were properly ordered");
        
        require(t->getNoteOffVector().size() == 2, "Note off vector was decreased");
        require(t->getNoteOffVector()[0].getEndTick() == 100, "Note off vector is properly ordered");
        require(t->getNoteOffVector()[1].getEndTick() == 400, "Note off vector is properly ordered");
        
        // Now test undo
        provider.m_seq->undo();
        provider.verifyUndo();
    }
    
    UNIT_TEST(TestDeleteFirst)
    {
        TestSeqProvider provider;
        Track* t = provider.m_seq->getTrack(0);
        
        t->selectNote(0, true);
        
        // test the action
        t->action(new DeleteSelected());
        
        require(t->getNoteAmount() == 3, "the number of events was decreased");

        require(t->getNote(0)->getTick()    == 101, "events were properly ordered");
        require(t->getNote(0)->getPitchID() == 101, "events were properly ordered");
        
        require(t->getNote(1)->getTick()    == 201, "events were properly ordered");
        require(t->getNote(1)->getPitchID() == 102, "events were properly ordered");
        
        require(t->getNote(2)->getTick()    == 301, "events were properly ordered");
        require(t->getNote(2)->getPitchID() == 103, "events were properly ordered");
        
        require(t->getNoteOffVector().size() == 3, "Note off vector was decreased");
        require(t->getNoteOffVector()[0].getEndTick() == 200, "Note off vector is properly ordered");
        require(t->getNoteOffVector()[1].getEndTick() == 300, "Note off vector is properly ordered");
        require(t->getNoteOffVector()[2].getEndTick() == 400, "Note off vector is properly ordered");
        
        // Now test undo
        provider.m_seq->undo();
        provider.verifyUndo();
    }
    
    UNIT_TEST(TestDeleteLast)
    {
        TestSeqProvider provider;
        Track* t = provider.m_seq->getTrack(0);
        
        t->selectNote(3, true);
        
        // test the action
        t->action(new DeleteSelected());
        
        require(t->getNoteAmount() == 3, "the number of events was decreased");
        
        require(t->getNote(0)->getTick()    == 0,   "events were properly ordered");
        require(t->getNote(0)->getPitchID() == 100, "events were properly ordered");
        
        require(t->getNote(1)->getTick()    == 101, "events were properly ordered");
        require(t->getNote(1)->getPitchID() == 101, "events were properly ordered");
        
        require(t->getNote(2)->getTick()    == 201, "events were properly ordered");
        require(t->getNote(2)->getPitchID() == 102, "events were properly ordered");
        

        require(t->getNoteOffVector().size() == 3, "Note off vector was decreased");
        require(t->getNoteOffVector()[0].getEndTick() == 100, "Note off vector is properly ordered");
        require(t->getNoteOffVector()[1].getEndTick() == 200, "Note off vector is properly ordered");
        require(t->getNoteOffVector()[2].getEndTick() == 300, "Note off vector is properly ordered");
        
        // Now test undo
        provider.m_seq->undo();
        provider.verifyUndo();      
    }
    
}
#endif
