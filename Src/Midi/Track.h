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
#include "Midi/MagneticGrid.h"
#include "Midi/Note.h"

#include "ptr_vector.h"

namespace AriaMaestosa
{
    
    class Sequence; // forward
    class GraphicalTrack;
    class MainFrame;
    class ControllerEvent;
    class FullTrackUndo;
    class NoteRelocator;
    class SequenceVisitor;
    
    namespace Action
    {
        class EditAction;
        class SingleTrackAction;
        class MultiTrackAction;
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
    
    enum NotationType
    {
        KEYBOARD   = 0,
        SCORE      = 1,
        GUITAR     = 2,
        DRUM       = 3,
        CONTROLLER = 4
    };
    
    class ITrackListener
    {
    public:
        
        virtual ~ITrackListener() {}
        
        virtual void onNotationTypeChange() = 0;
        virtual void onKeyChange(const int symbolAmount, const KeyType symbol) = 0;

        LEAK_CHECK();
    };
    
    /**
      * @brief represents a track within a sequence.
      *
      * Contains notes, control events, etc...
      * @ingroup midi
      */
    class Track : public IInstrumentChoiceListener, public IDrumChoiceListener, public IGuitarTuningListener
    {
        /** Holds all notes contained in this track, sorted in time order (of note start) */
        ptr_vector<Note> m_notes;
        
        /** Same contents as 'm_notes', but sorted according to the end of the notes */
        ptr_vector<Note, REF> m_note_off;
        
        /** Holds all controller events from this track */
        ptr_vector<ControllerEvent> m_control_events;
        
        int m_track_id;
        
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

        OwnerPtr<MagneticGrid> m_magnetic_grid;
        
        OwnerPtr< Model<wxString> > m_track_name;
        
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
        
        /** The sequence this track is part of */
        Sequence* m_sequence;

        NotationType m_editor_mode;

        ITrackListener* m_listener;
    
        IInstrumentChoiceListener* m_next_instrument_listener;
        IDrumChoiceListener* m_next_drumkit_listener;

    public:
        
        /** Through this visitor object, the Track class will let actions manipulate its internals
          * in a controlled sequence
          */
        class TrackVisitor
        {
            friend class Track;
            friend class SequenceVisitor;
            
            Track* m_track;
            
            TrackVisitor(Track* t)
            {
                m_track = t;
            }
            
        public:
            
            ptr_vector<Note>&            getNotesVector()        { return m_track->m_notes;          }
            ptr_vector<Note, REF>&       getNoteOffVector()      { return m_track->m_note_off;       }
            ptr_vector<ControllerEvent>& getControlEventVector() { return m_track->m_control_events; }
            
            LEAK_CHECK();
        };
        
#ifdef _MORE_DEBUG_CHECKS
        int m_track_unique_ID;
#endif
        
        LEAK_CHECK();
        
        Track(Sequence* sequence);
        ~Track();
        
        /**
          * @brief Set a listener to be notified when some track properties change
          * @note  Does not take ownership of the listener (will not delete it)
          */
        void setListener(ITrackListener* listener)
        {
            m_listener = listener;
        }
        
        void setInstrumentListener(IInstrumentChoiceListener* l) { m_next_instrument_listener = l; }
        void setDrumListener      (IDrumChoiceListener* l)       { m_next_drumkit_listener    = l; }
        
        /** @brief place events in time order */
        void reorderNoteVector();
        
        /** @brief place events in time order */
        void reorderNoteOffVector();
        
        /** @brief place events in time order */
        void reorderControlVector();
        
        void removeNote(const int id);
        
        void setId(const int id);
        
        int getId() const { return m_track_id; }
        
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
        
        bool checkControlEventsOrder();
                
        void setName(wxString name);
        
        void selectNote(const int id, const bool selected, bool ignoreModifiers=false);

        const wxString    getName     () const { return m_track_name->getValue(); }
        Model<wxString>*  getNameModel()       { return m_track_name;             }

        /** @brief Get the sequence this track is part of */
        Sequence* getSequence         ()       { return m_sequence;        }
        
        /** @brief Get the sequence this track is part of (const version) */
        const Sequence* getSequence   () const { return m_sequence;        }

        /** Snaps a tick to the magnetic grid
          * @param absolute_x  A midi tick you want to snap
          * @return            The given tick, snapped to the magnetic grid
          */
        int snapMidiTickToGrid(int absolute_x);
        
        /** Same as 'snapMidiTickToGrid', but will only snap the tick to a bigger value */
        int snapMidiTickToGrid_ceil(int absolute_x);
        
        MagneticGrid* getMagneticGrid() { return m_magnetic_grid; }
        
        /** 
          * @return the type of key for this track
          */
        KeyType getKeyType          () const { return m_key_type;        }
        
        /** 
          * @return the number of sharp symbols in this track's key (or 0 if the key uses flats)
          * @note if key type (see Track::getKeyType) is not KEY_TYPE_SHARPS, will return 0
          */
        int getKeySharpsAmount      () const { return m_key_sharps_amnt; }
        
        /**
          * @return the number of flat symbols in this track's key (or 0 if the key uses sharps)
          * @note if key type (see Track::getKeyType) is not KEY_TYPE_FLATS, will return 0
          */
        int getKeyFlatsAmount       () const { return m_key_flats_amnt;  }
        
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
        
        GraphicalTrack* getGraphics();
        const GraphicalTrack* getGraphics() const;

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
        
        static bool isTempoController(const int controllerTypeID)
        {
            return controllerTypeID == 201;
        }
        
        //FIXME: tempo events have nothing to do with tracks and should not be handled here at all
        /**
         * @return          the amount of ALL types of controller events
         * @param isTempo   if true, returns amount of tempo events
         */
        int getControllerEventAmount(const bool isTempo=false) const;
        
        /**
         * @return            the amount of ALL types of controller, not only of specified type
         * @param controller  which controller to count the events of
         */
        int getControllerEventAmount(const int controller) const;
        
        /**
          * @brief get a controller event object
          * @param id of the control event to retrieve (from 0 to count-1)
          * @param controllerTypeID only to determine whether the app is searching for a control event
          *                         or for a tempo event (FIXME: ugly)
          */
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
          * @brief Get the first selected note in this track
          * @return the ID of the first selected note, or -1 if no note is selected
          */
        int getFirstSelectedNote() const;
        
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
         * @param channel in manual channel mode, this argument is NOT considered
         */
        int addMidiEvents(jdkmidi::MIDITrack* track, int channel, int firstMeasure,
                          bool selectionOnly, int& startTick); // returns length

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
          * @brief Get the type of notation this track uses
          */
        NotationType getNotationType() const { return m_editor_mode; }

        void setNotationType(NotationType t);
        
        /**
          * @brief Implement callback from IGuitarTuningListener
          */
        virtual void onGuitarTuningUpdated(GuitarTuning* tuning, const bool userTriggered);
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml, GraphicalSequence* gseq);
    };
    
}

#endif
