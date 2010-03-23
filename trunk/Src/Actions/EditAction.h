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

#ifndef _editaction_h_
#define _editaction_h_

#include "Utils.h"
#include "ptr_vector.h"
#include <vector>
#include "Midi/Note.h"
#include "Midi/ControllerEvent.h"

namespace AriaMaestosa {
    
    class Track;
    class Sequence;
    
    
    /*
     * In the opposite situation, when it is easy to revert the changes, NoteRelocator is used.
     * The EditAction keeps an instance of it and passes it to the Track method that performs the action,
     * and this method is then responsible of notifying the passed NoteRelocator of every note it applies
     * the change to.
     *
     * Both of the following classes use pointers to notes. Be sure to not break those pointers or you will
     * get crashes! If you need to remove notes from a track in an action, remove them from the vector but don't
     * deallocate the memory just now, keep it around so the same pointer can be added back if an undo is done.
     * When the Action is destroyed from the stack it is then safe to deallocate the memory as there is no way
     * the note can ever come back.
     *
     * Relocators won't delete anything.
     */
    class NoteRelocator
    {
        int id, noteamount_in_track, noteamount_in_relocator;
        Track* track;
        friend class Track;
    public:
        
        void rememberNote(Note& n);
        void rememberNote(Note* n);
        ~NoteRelocator();
        
        //std::vector<NoteInfo> notes;
        ptr_vector<Note, REF> notes;
        void setParent(Track* t);
        void prepareToRelocate();
        Note* getNextNote(); // returns one note at a time, and NULL when all of them where given
    };
    
    //class ControlEventInfo { public: int value, control, tick; };
    class ControlEventRelocator
    {
        int id, amount_in_track, amount_in_relocator;
        Track* track;
        friend class Track;
    public:
        ~ControlEventRelocator();
        
        void rememberControlEvent(ControllerEvent& c);
        
        ptr_vector<ControllerEvent, REF> events;
        
        void setParent(Track* t);
        void prepareToRelocate();
        ControllerEvent* getNextControlEvent(); // returns one note at a time, and NULL when all of them where given
    };
    /*
     * In Aria changes to tracks are represented with EditAction subclasses.
     * When a change must be done, a new EditAction object is instanciated and
     * placed in a stack. The goal of this is to be able to provide multiple undos,
     * by reversing the operations of the stack. Each EditAction subclass should
     * also be able to revert its action.
     */
    
    namespace Action
    {
        
        
        class EditAction
        {
            friend class AriaMaestosa::Track;
            wxString m_name;

        public:
            LEAK_CHECK();
            
            EditAction(wxString name);
            virtual void perform();
            virtual void undo();
            virtual ~EditAction();
            
            wxString getName() const { return m_name; }
        };
        
        class SingleTrackAction : public EditAction
        {
        protected:
            Track* track;
        public:
            SingleTrackAction(wxString name);
            void setParentTrack(Track* parent);
            virtual ~SingleTrackAction();
        };
        
        class MultiTrackAction : public EditAction
        {
        protected:
            Sequence* sequence;
        public:
            MultiTrackAction(wxString name);
            void setParentSequence(Sequence* parent); // for multi-track actions
            virtual ~MultiTrackAction();
        };
        
    }
}
#endif
