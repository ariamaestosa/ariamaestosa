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

#ifndef __TRACK_H__
#define __TRACK_H__

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

namespace jdkmidi { class MIDITrack; }

#include "Midi/ControllerEvent.h"
#include "Midi/DrumChoice.h"
#include "Midi/GuitarTuning.h"
#include "Midi/InstrumentChoice.h"
#include "Midi/Note.h"

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
    
    enum KeyType
    {
        KEY_TYPE_SHARPS,
        KEY_TYPE_FLATS,
        KEY_TYPE_C,
        KEY_TYPE_CUSTOM
    };
    
    enum KeyInclusionType
    {
        /** This note normally appears in this key */
        KEY_INCLUSION_FULL = 2,
        /** This note may appear in this key as accidental */
        KEY_INCLUSION_ACCIDENTAL = 1,
        /** this note is forbidden by this key / cannot be played at all by the instrument */
        KEY_INCLUSION_NONE = 0
    };
    
    /**
      * @brief represents a track within a sequence.
      *
      * Contains notes, control events, etc...
      * @ingroup midi
      */
    class Track : public IInstrumentChoiceListener, public IDrumChoiceListener, public IGuitarTuningListener
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
        
        //MainFrame* m_parent_frame;
        
        /** Holds all notes contained in this track, sorted in time order (of note start) */
        ptr_vector<Note> m_notes;
        
        /** Same contents as 'm_notes', but sorted according to the end of the notes */
        ptr_vector<Note, REF> m_note_off;
        
        /** Holds all controller events from this track */
        ptr_vector<ControllerEvent> m_control_events;
        
        int m_track_id;
        
        /** Name of the track, and also object that can render it */
        AriaRenderString m_name;
        
        /** Only used if in manual channel management mode */
        int m_channel;
        
        OwnerPtr<InstrumentChoice> m_instrument;
        OwnerPtr<DrumChoice> m_drum_kit;
        
        KeyType m_key_type;
        
        int m_key_sharps_amnt;
        int m_key_flats_amnt;
        
        /** 
          * @brief contains wich notes are part of the key of this track.
          * It may be calculated from m_key_sharps_amnt / m_key_flats_amnt or set
          * directly for custom keys.
          */
        KeyInclusionType m_key_notes[131];
        
        /** Whether this track has been muted so that it's not heard on playback. */
        bool m_muted;
        
        OwnerPtr<GuitarTuning> m_tuning;

        /**
         * @brief set the MIDI instrument used by this track
         * @param recursive set to true when the method calls itself
         */
        void doSetInstrument(int i, bool recursive=false);
        
        /**
         * @param recursive Set to true when 'setDrumKit' when the function was called by itself
         *                  It tells not to do any more recursion. Do not set when calling externally.
         */
        void doSetDrumKit(int i, bool recursive=false);
        
    public:
        LEAK_CHECK();
                
        // ------------- read-only -------------
        // FIXME: it should be the graphics that refer to the data, not the data holding the graphics!
        OwnerPtr<GraphicalTrack>  graphics;
        Sequence* sequence;
        int trackUniqueID;
        // -------------------------------------
        
        Track(Sequence* sequence);
        ~Track();
        
        /**
         * when one track will be removed, all others are notified so they can remove any
         * link to that track they could have (like background)
         */
        void trackDeleted(Track* track);
        
        /** @brief place events in time order */
        void reorderNoteVector();
        
        /** @brief place events in time order */
        void reorderNoteOffVector();
        
        /** @brief place events in time order */
        void reorderControlVector();
        
        void removeNote(const int id);
        
        void setId(const int id);
        
        /**
          * @brief set notes while importing files.
          * @note when not importing, use edit actions instead.
          */
        bool addNote_import(const int pitchID, const int startTick, const int endTick, const int volume, const int string=-1);
        
        /** 
          * @brief set notes while importing files.
          * @note when not importing, use edit actions instead.
          */
        void setNoteEnd_import(const int tick, const int noteID);
        
        /**
         * @brief Add a midi control change, added when reading a file.
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
        
        /** 
          * @return the type of key for this track
          */
        KeyType getKeyType    () const { return m_key_type;        }
        
        /** 
          * @return the number of sharp symbols in this track's key (or 0 if the key uses flats)
          * @note if key type (see Track::getKeyType) is not KEY_TYPE_SHARPS, will return 0
          */
        int getKeySharpsAmount() const { return m_key_sharps_amnt; }
        
        /**
          * @return the number of flat symbols in this track's key (or 0 if the key uses sharps)
          * @note if key type (see Track::getKeyType) is not KEY_TYPE_FLATS, will return 0
          */
        int getKeyFlatsAmount () const { return m_key_flats_amnt;  }
        
        /**
          * @brief Sets the key of this track
          *
          * @param symbolAmount Amount of sharp/flat signs this key has
          * @param type         Key type - must be a standard type (i.e. may not be KEY_TYPE_CUSTOM).
          */
        void setKey(const int symbolAmount, const KeyType type);
        
        /**
          * @brief Sets the key of this track.
          *
          * This overload can take any non-standard key.
          */
        void setCustomKey(const KeyInclusionType key_notes[131]);
        
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
         * @pre only use in manual channel management mode
         * if auto mode is on, the playing code must pick a channel for each track.
         * if you use Aria's libjdkmidi/midibytes functions, this will be done for you
         */
        void setChannel(int i);
        
        /** @pre only used in manual channel mode */
        int getChannel();
        
        /**
          * @brief set the MIDI instrument used by this track
          */
        void setInstrument(int i);
        
        /**
          * @brief  the MIDI instrument used by this track
          * @return the MIDI instrument used by this track
          */
        int getInstrument() const { return m_instrument->getSelectedInstrument(); }
        
        InstrumentChoice* getInstrumentModel() { return m_instrument; }
        DrumChoice*       getDrumkitModel   () { return m_drum_kit;   }

        void setDrumKit(int i);
        
        /** @return the selected drum kit (if this track is not a drum track, this value is ignored */
        int  getDrumKit() const { return m_drum_kit->getSelectedDrumkit(); }
        
        void copy();
        
        /** @return an array of bools, for each note (one entry per pitch ID), that indicates
         *         which notes are part of the current key */
        const KeyInclusionType* getKeyNotes() const { return m_key_notes; }
    
        /**
         * @brief Add Midi Events to JDKMidi track object
         */
        int addMidiEvents(jdkmidi::MIDITrack* track, int channel, int firstMeasure,
                          bool selectionOnly, int& startTick); // returns length
        
        /**
          * Call this method before deleting a track, so that it can tell others it's going to
          * be gone. Cannot be called from constructor since it would then cause problems when
          * deleting the entire sequence.
          */
        void notifyOthersIWillBeRemoved();

        /**
          * @brief Get a read-only list of all notes in this track, but ordered by their end tick.
          */
        const ptr_vector<Note, REF>& getNoteOffVector() const
        {
            return m_note_off;
        }

        
        /**
          * @brief toggle the muted state of this track (a muted track becomes non-muted and vice-versa)
          */
        void toggleMuted()         { m_muted = not m_muted; }
        
        /**
          * @return whether this track is muted (not heard when sequence is played)
          */
        bool isMuted() const       { return m_muted;        }
        
        /**
          * @brief set whether this track is muted (not heard when sequence is played)
          * @param muted whether this track is muted (not heard when sequence is played)
          */
        void setMuted(bool muted)  { m_muted = muted;       }
        

        /**
          * @brief Implement callback from IInstrumentChoiceListener
          */
        virtual void onInstrumentChanged(const int newValue);
            
        /**
          * @brief Implement callback from IDrumChoiceListener
          */
        virtual void onDrumkitChanged(const int newValue);

        /**
          * @brief Get the guitar tuning model object
          */
        GuitarTuning* getGuitarTuning() { return m_tuning; }
        
        /**
         * @brief Get the guitar tuning model object
         */
        const GuitarTuning* getConstGuitarTuning() const { return m_tuning.raw_ptr; }
        
        /**
          * @brief Implement callback from IGuitarTuningListener
          */
        virtual void onGuitarTuningUpdated(GuitarTuning* tuning, const bool userTriggered);
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}

#endif
