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

#ifndef _track_h_
#define _track_h_

#include "irrXML/irrXML.h"

#include "wx/wfstream.h"

#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/sysex.h"

#include "jdkmidi/sequencer.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/process.h"

#include <vector>
#include <string>

#include "Editors/Editor.h"
#include "Midi/Note.h"
#include "Midi/ControllerEvent.h"

#include "ptr_vector.h"
#include "Renderers/RenderAPI.h"

namespace AriaMaestosa
{
    
    class Sequence; // forward
    class GraphicalTrack;
    class MainFrame;
    class ControllerEvent;
    class FullTrackUndo;
    class NoteRelocator;
    
    namespace Action
    {
        class EditAction;
        class SingleTrackAction;
        class MultiTrackAction;
        
        class MoveNotes;
        class SetNoteVolume;
        class ResizeNotes;
        class DeleteSelected;
        class RemoveOverlapping;
        class SnapNotesToGrid;
        class ScaleTrack;
        class ScaleSong;
        class InsertEmptyMeasures;
        class RemoveMeasures;
        class AddNote;
        class AddControlEvent;
        class AddControllerSlide;
        class ShiftFrets;
        class ShiftString;
        class NumberPressed;
        class UpdateGuitarTuning;
        class Paste;
        class SetAccidentalSign;
        class ShiftBySemiTone;
    }
    
    const int SELECTED_NOTES = -1;
    const int ALL_NOTES = -2;
    
    class Track
    {
        // FIXME - find better way then friends?
        friend class FullTrackUndo;
        friend class NoteRelocator;
        friend class ControlEventRelocator;
        
        friend class Action::MoveNotes;
        friend class Action::SetNoteVolume;
        friend class Action::ResizeNotes;
        friend class Action::DeleteSelected;
        friend class Action::RemoveOverlapping;
        friend class Action::SnapNotesToGrid;
        friend class Action::ScaleTrack;
        friend class Action::ScaleSong;
        friend class Action::InsertEmptyMeasures;
        friend class Action::RemoveMeasures;
        friend class Action::AddNote;
        friend class Action::AddControlEvent;
        friend class Action::AddControllerSlide;
        friend class Action::ShiftFrets;
        friend class Action::ShiftString;
        friend class Action::NumberPressed;
        friend class Action::UpdateGuitarTuning;
        friend class Action::Paste;
        friend class Action::SetAccidentalSign;
        friend class Action::ShiftBySemiTone;
        
        MainFrame* frame;
        ptr_vector<Note> notes;
        ptr_vector<Note, REF> noteOff;
        ptr_vector<ControllerEvent> controlEvents;
        int trackid;
        
        AriaRenderString m_name;
        
        int channel; // only used if in channel mode
        int instrument, drumKit;
        
        int key_sharps_amnt;
        int key_flats_amnt;
        
        /** contains wich notes appear as gray on the keyboard editor (by default, in C key, it is
         * the sharp ones)
         */
        bool m_key_notes[131];
        
    public:
        LEAK_CHECK();
        
        // ------------- read-only -------------
        // FIXME: it should be the graphics that refer to the data, not the data holding the graphics!
        OwnerPtr<GraphicalTrack>  graphics;
        Sequence* sequence;
        int trackUniqueID;
        
        AriaRenderString instrument_name;
        // -------------------------------------
        
        Track(MainFrame* parent, Sequence* sequence);
        ~Track();
        
        /**
         * when one track will be removed, all others are notified so they can remove any
         * link to that track they could have (like background)
         */
        void trackDeleted(Track* track);
        
        /** [re]place events in time order */
        void reorderNoteVector();
        
        /** [re]place events in time order */
        void reorderNoteOffVector();
        
        /** [re]place events in time order */
        void reorderControlVector();
        
        void removeNote(const int id);
        
        void setId(const int id);
        
        /** set notes while importing files. otherwise, use edit actions. */
        bool addNote_import(const int pitchID, const int startTick, const int endTick, const int volume, const int string=-1);
        
        /** set notes while importing files. otherwise, use edit actions. */
        void setNoteEnd_import(const int tick, const int noteID);
        
        /**
         * A midi control change, added when reading a file.
         * If we're reading the even from file, we can add it right away without further checks
         * because we know events won't overlap and are in time order. (i.e. this exists, as opposed to
         * the regular add method, for performance reasons)
         */
        void addControlEvent_import(const int x, const int value, const int controller);
        
        // FIXME - debug function, remove
        void checkControlEventsOrder();
        
        int getGridDivider();
        
        void setName(wxString name);
        
        AriaRenderString& getNameRenderer();
        const wxString&   getName() const { return m_name; }
        
        void selectNote(const int id, const bool selected, bool ignoreModifiers=false);
        
        /** @return the number of sharp symbols in this track's key (or 0 if the key uses flats) */
        int getKeySharpsAmount() const { return key_sharps_amnt; }
        
        /** @return the number of flat symbols in this track's key (or 0 if the key uses sharps) */
        int getKeyFlatsAmount()  const { return key_flats_amnt;  }
        
        /**
          * Sets the key of this track
          * @paramsymbolAmount Amount of sharp/flat signs this key has
          * @param symbol      FLAT or SHARP.
          */
        void setKey(const int symbolAmount, const PitchSign symbol);
        
        // ---- get info on notes
        int   getNoteAmount           ()             const;
        int   getNoteStartInPixels    (const int id) const;
        int   getNoteEndInPixels      (const int id) const;
        int   getNoteStartInMidiTicks (const int id) const;
        int   getNoteEndInMidiTicks   (const int id) const;
        int   getNotePitchID          (const int id) const;
        bool  isNoteSelected          (const int id) const;
        int   getNoteVolume           (const int id) const;
        /** use only if other getters can't provide what you want! (FIXME) */
        Note* getNote                 (const int id);
        
        /**
         * Returns the first note in the given range, or -1 if there is none
         */
        int findFirstNoteInRange(const int fromTick, const int toTick) const;
        
        /**
         * Returns the last note in the given range, or -1 if there is none
         */
        int findLastNoteInRange(const int fromTick, const int toTick) const;
        
        void playNote(const int id, const bool noteChange=false);
        
        void markNoteToBeRemoved(const int id);
        void removeMarkedNotes();
        
        /** 
         * @param id ID of the note to get the string of
         * @return   The string number of the specified note.
         *
         * For guitar editor. Regular 'getNoteString' method might decide to calculate
         * the string on the fly if it wasn't already available (will happen e.g. when
         * switching from keyboard editor to guitar editor). However, if you know that
         * the string is already calculated (e.g. when printing) and need constness, you
         * can use 'getNoteStringConst'.
         */
        int getNoteString(const int id);
        
        /** 
         * @param id ID of the note to get the string of
         * @return   The string number of the specified note.
         *
         * For guitar editor. Regular 'getNoteString' method might decide to calculate
         * the string on the fly if it wasn't already available (will happen e.g. when
         * switching from keyboard editor to guitar editor). However, if you know that
         * the string is already calculated (e.g. when printing) and need constness, you
         * can use 'getNoteStringConst'.
         */
        int getNoteStringConst(const int id) const;

        /** 
         * @param id ID of the note to get the fret number of
         * @return   The fret number of the specified note.
         *
         * For guitar editor. Regular 'getNoteFret' method might decide to calculate
         * the fret on the fly if it wasn't already available (will happen e.g. when
         * switching from keyboard editor to guitar editor). However, if you know that
         * the fret is already calculated (e.g. when printing) and need constness, you
         * can use 'getNoteFretConst'.
         */
        int getNoteFret(const int id);
        
        /** 
          * @param id ID of the note to get the fret number of
          * @return   The fret number of the specified note.
          *
          * For guitar editor. Regular 'getNoteFret' method might decide to calculate
          * the fret on the fly if it wasn't already available (will happen e.g. when
          * switching from keyboard editor to guitar editor). However, if you know that
          * the fret is already calculated (e.g. when printing) and need constness, you
          * can use 'getNoteFretConst'.
          */
        int getNoteFretConst(const int id) const; 

        void prepareNotesForGuitarEditor();
        
        /**
         * @return                 the amount of ALL types of controller, not only of specified type
         * @param controllerTypeID only to determine whether the app is searching for a control event
         *                         or for a tempo event (FIXME: ugly)
         */
        int getControllerEventAmount(const int controllerTypeID) const;
        
        ControllerEvent* getControllerEvent(const int id, const int controllerTypeID);
        
        /** not to be called during editing, as it does not generate an action in the action stack. */
        bool addNote( Note* note, bool check_for_overlapping_notes=true );
        
        /** Not to be called during editing, as it does not generate an action in the action stack.
         * @param[out] previousValue Returns the old value there was, if any, before this new event replaces it.*/
        void addControlEvent( ControllerEvent* evt, int* previousValue = NULL );
        
        /**
         * This is the method called for performing any action that can be undone.
         * A EditAction object is used to describe the task, and it also knows how to revert it.
         * The EditAction objects are kept in a stack in Sequence in order to offer multiple undo levels.
         *
         * Track::action does actions that affect only this Track. Also see Sequence::action.
         */
        void action( Action::SingleTrackAction* action);
        
        void mergeTrackIn(Track* track);
        
        /**
         * The tick where the first note of the track starts playing.
         * Used mostly when scaling relative to track
         */
        int getFirstNoteTick(bool selectionOnly=false) const;
        
        /**
         * @return Track duration in midi ticks (i.e. returns the last tick in this track)
         */
        int getDuration() const;
        
        /*
         * @precondition only use in manual channel management mode
         * if auto mode is on, the playing code must pick a channel for each track.
         * if you use Aria's libjdkmidi/midibytes functions, this will be done for you
         */
        void setChannel(int i);
        
        /** @precondition only used in manual channel mode */
        int getChannel();
        
        void setInstrument(int i, bool recursive=false); // 'recursive' is set to true when the method calls itself
        int getInstrument();
        
        /**
         * @param recursive Set to true when 'setDrumKit' when the function was called by itself
         *                  It tells not to do any more recursion. Do not set when calling externally.
         */
        void setDrumKit(int i, bool recursive=false);
        
        int getDrumKit();
        
        void copy();
        
        /** @return an array of bools, for each note (one entry per pitch ID), that indicates
         *         which notes are part of the current key */
        const bool* getKeyNotes() const { return m_key_notes; }
    
        /**
         * Add Midi Events to JDKMidi track object
         */
        int addMidiEvents(jdkmidi::MIDITrack* track, int channel, int firstMeasure,
                          bool selectionOnly, int& startTick); // returns length
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}

#endif
