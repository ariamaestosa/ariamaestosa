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

#include "Utils.h"
#include "Clipboard.h"
#include "AriaCore.h"

#include "Actions/EditAction.h"
#include "Actions/UpdateGuitarTuning.h"

// FIXME(DESIGN) : data classes shouldn't refer to GUI classes
#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "Editors/ControllerEditor.h"
#include "Editors/DrumEditor.h"

#include "IO/IOUtils.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/ControllerEvent.h"
#include "Midi/DrumChoice.h"
#include "Midi/MeasureData.h"
#include "PreferencesData.h"

#include <iostream>

#include "jdksmidi/world.h"
#include "jdksmidi/track.h"

#include <wx/intl.h>
#include <wx/utils.h>
#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

Track::Track(Sequence* sequence)
{
#ifdef _MORE_DEBUG_CHECKS
    static int id = 1000;
    m_track_unique_ID = id++;
#endif

    m_magnetic_grid = new MagneticGrid();
    
    m_volume = 100;
    m_muted = false;
    m_soloed = false;
    m_played = true;

    for (int n=0; n<NOTATION_TYPE_COUNT; n++)
    {
        m_editor_mode[n] = false;
    }

    // set default editor
    switch (PreferencesData::getInstance()->getIntValue(SETTING_ID_DEFAULT_EDITOR))
    {
        case 2:
            m_editor_mode[GUITAR] = true;
            break;
        case 1:
            m_editor_mode[SCORE] = true;
            break;
        case 0:
        default:
            m_editor_mode[KEYBOARD] = true;
            break;
    }

    m_listener = NULL;

    // init key data
    setKey(sequence->getDefaultKeySymbolAmount(),
            sequence->getDefaultKeyType());

    m_tuning = new GuitarTuning(this);

    m_track_name = new Model<wxString>( _("Untitled") );

    m_next_instrument_listener = NULL;
    m_next_drumkit_listener = NULL;
    m_default_volume = 80;
    m_sequence = sequence;

    m_channel = 0;
    if (sequence->getChannelManagementType() == CHANNEL_MANUAL)
    {
        // if in manual channel management mode, we need to give it a proper channel
        // the following array will store what channels are currently taken
        bool channel_taken[16];
        for (int i=0; i<16; i++) channel_taken[i] = false;

        const int track_amount = sequence->getTrackAmount();
        for (int i=0; i<track_amount; i++)
        {
            ASSERT_E(sequence->getTrack(i)->getChannel(),>=,0);
            ASSERT_E(sequence->getTrack(i)->getChannel(),<,16);
            channel_taken[sequence->getTrack(i)->getChannel()] = true;
        }
        for (int i=0; i<16; i++)
        {
            if (i==9) continue; // don't use channel 9, it's for drums
            // use first not-yet-used channel
            if (not channel_taken[i])
            {
                m_channel = i;
                break;
            }
            else if (i == 15)
            {
                // we reached the end and still haven't found any...
                // FIXME - the given instrument might be wrong
            }
        }
    }


    m_instrument = new InstrumentChoice(0, this);
    m_drum_kit   = new DrumChoice(0, this);
}

// ----------------------------------------------------------------------------------------------------------

Track::~Track()
{
#ifdef _MORE_DEBUG_CHECKS
    m_track_unique_ID = -m_track_unique_ID;
#endif
}

// ----------------------------------------------------------------------------------------------------------

void Track::action( Action::SingleTrackAction* actionObj)
{
    actionObj->setParentTrack(this, new TrackVisitor(this));
    m_sequence->addToUndoStack( actionObj );
    actionObj->perform();
}

// ----------------------------------------------------------------------------------------------------------

GraphicalTrack* Track::getGraphics()
{
    return getMainFrame()->getCurrentGraphicalSequence()->getGraphicsFor(this);
}

// ----------------------------------------------------------------------------------------------------------

const GraphicalTrack* Track::getGraphics() const
{
    return getMainFrame()->getCurrentGraphicalSequence()->getGraphicsFor(this);
}


// =======================================================================================================
// ======================================= Add/Remove Notes ==============================================
// =======================================================================================================

#if 0
#pragma mark -
#pragma mark Add/Remove Notes
#endif


bool Track::addNote(Note* note, bool check_for_overlapping_notes)
{
    // if we're importing, just push it to the end, we know they're in time order
    if (m_sequence->isImportMode())
    {
        m_notes.push_back(note);
        m_note_off.push_back(note); // dont forget to reorder note off vector after importing
        return true;
    }

    bool noteAdded=false;

    //------------------------ place note on -----------------------
    // iterate through notes to place them in order
    const int noteAmount = m_notes.size();
    for (int n=0; n<noteAmount; n++)
    {
        // check for overlapping notes
        // the only time where this is not checked is when pasting, because it is then logical that notes are pasted on top of their originals
        if (check_for_overlapping_notes and m_notes[n].getTick() == note->getTick() and
            m_notes[n].getPitchID() == note->getPitchID() and
           (not m_editor_mode[GUITAR] or m_notes[n].getString() == note->getString()) /*in guitar mode string must also match to be considered overlapping*/ )
        {
            std::cout << "overlapping notes: rejected" << std::endl;
            return false;
        }

        if (m_notes[n].getTick() > note->getTick())
        {

            m_notes.add(note, n);
            noteAdded=true;
            break;
        }//endif


    }//next

    if (!noteAdded)
    {
        // add note at the end, or if the track does not yet contain any note
        // (both cases in which the for loop either won't find a location for the note, or won't iterate at all)
        m_notes.push_back(note);
    }

    //------------------------ place note off -----------------------
    // iterate through notes to place them in order

    noteAdded = false;

    const int noteOffAmount = m_note_off.size();
    for (int n=0; n<noteOffAmount; n++)
    {
        if (m_note_off[n].getEndTick() > note->getEndTick())
        {
            m_note_off.add(note, n);
            noteAdded = true;
            break;
        }//endif
    }//next

    if (not noteAdded)
    {
        // add note at the end, or if the track does not yet contain any note
        // (both cases in which the for loop either won't find a location for the note, or won't iterate at all)
        m_note_off.push_back(note);
    }


    return true;

}

// ----------------------------------------------------------------------------------------------------------

void Track::addControlEvent( ControllerEvent* evt, wxFloat64* previousValue )
{
    ptr_vector<ControllerEvent>* vector;

    if (previousValue != NULL) *previousValue = -1;

    // tempo events
    if (evt->getController() == PSEUDO_CONTROLLER_TEMPO) vector = &m_sequence->m_tempo_events;
    // controller and pitch bend events
    else vector = &m_control_events;

    // don't bother checking order if we're importing, we know its in time order and all
    // FIXME - what about 'addControlEvent_import' ??
    if (m_sequence->isImportMode())
    {
        vector->push_back( evt );
        return;
    }

    ASSERT_E(evt->getController(),<,205);
    ASSERT_E(evt->getValue(),<,128);

    const int eventAmount=vector->size();
    for (int n=0; n<eventAmount; n++)
    {
        if ( (*vector)[n].getTick() == evt->getTick())
        {
            // if there is already an even of same type at same time, remove it first
            if ((*vector)[n].getController() == evt->getController() )
            {
                if (previousValue != NULL) *previousValue = (*vector)[n].getValue();
                vector->erase(n);
            }
            vector->add( evt, n );
            return;
        }
        else if ( (*vector)[n].getTick() > evt->getTick() )
        {
            vector->add( evt, n );
            return;
        }//endif

    }//next

    vector->push_back( evt );
}

// ----------------------------------------------------------------------------------------------------------

void Track::addControlEvent_import(const int x, const wxFloat64 value, const int controller)
{
    ASSERT(m_sequence->isImportMode()); // not to be used when not importing
    m_control_events.push_back(new ControllerEvent(controller, x, value) );
}

// ----------------------------------------------------------------------------------------------------------

bool Track::addNote_import(const int pitchID, const int startTick, const int endTick, const int volume, const int string)
{
    ASSERT(m_sequence->isImportMode()); // not to be used when not importing
    return addNote( new Note(this, pitchID, startTick, endTick, volume, string) );
}

// ----------------------------------------------------------------------------------------------------------

void Track::setNoteEnd_import(const int tick, const int noteID)
{
    ASSERT(m_sequence->isImportMode()); // not to be used when not importing
    ASSERT(noteID != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    ASSERT(noteID != SELECTED_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)

    ASSERT_E(noteID,<,m_notes.size());
    ASSERT_E(noteID,>=,0);

    m_notes[noteID].setEndTick(tick);
}

// ----------------------------------------------------------------------------------------------------------

void Track::removeNote(const int id)
{

    // also delete corresponding note off event
    const int namount = m_note_off.size();
    for (int i=0; i<namount; i++)
    {
        if (&m_note_off[i] == &m_notes[id])
        {
            m_note_off.remove(i);
            break;
        }
    }

    m_notes.erase(id);

}

// ----------------------------------------------------------------------------------------------------------

void Track::markNoteToBeRemoved(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    // also delete corresponding note off event
    const int namount = m_note_off.size();
    Note* note = m_notes.get(id);

    for (int i=0; i<namount; i++)
    {
        if (m_note_off.get(i) == note)
        {
            m_note_off.markToBeRemoved(i);
            break;
        }
#ifdef _MORE_DEBUG_CHECKS
        if (i+1 == namount) std::cout << "WARNING could not find note off event corresponding to note on event" << std::endl;
#endif
    }

    m_notes.markToBeRemoved(id);
}

// ----------------------------------------------------------------------------------------------------------

void Track::removeMarkedNotes()
{

    //std::cout << "removing marked" << std::endl;

    m_notes.removeMarked();
    m_note_off.removeMarked();

#ifdef _MORE_DEBUG_CHECKS
    if (m_notes.size() != m_note_off.size())
    {
        std::cout << "WARNING note on and off events differ in amount in Track::removeMarkedNotes()" << std::endl;
        std::cout << m_notes.size() << " m_notes and " << m_note_off.size() << " note offs" << std::endl;
    }
#endif
}

// ----------------------------------------------------------------------------------------------------------

void Track::reorderNoteVector()
{
    // FIXME - innefficient implementation
    const int noteAmount = m_notes.size();

#ifdef _MORE_DEBUG_CHECKS
    if (m_notes.size() != m_note_off.size())
    {
        std::cout << "WARNING note on and off events differ in amount" << std::endl;
        std::cout << m_notes.size() << " notes and " << m_note_off.size() << " note offs" << std::endl;
    }
#endif

    for (int n=0; n<noteAmount-1; n++)
    {

        ASSERT_E(n+1,<,m_notes.size());

        if (m_notes[n].getTick() > m_notes[n+1].getTick())
        {
            m_notes.swap(n, n+1);
            if (n>2) n-= 2;
            else n=-1;
        }
        else if (m_notes[n].getTick() == m_notes[n+1].getTick() and
                 m_notes[n].getPitchID() > m_notes[n+1].getPitchID())
        {
            m_notes.swap(n, n+1);
            if (n>2) n-= 2;
            else n=-1;
        }
    }//next

}

// ----------------------------------------------------------------------------------------------------------

void Track::reorderNoteOffVector()
{
    // FIXME - innefficient implementation
    const int noteAmount = m_note_off.size();
#ifdef _MORE_DEBUG_CHECKS
    if (m_notes.size() != m_note_off.size()) std::cout << "WARNING note on and off events differ in amount" << std::endl;
#endif

    for (int n=0; n<noteAmount-1; n++)
    {

        ASSERT_E(n+1,<,m_note_off.size());
        ASSERT(m_note_off.get(n) != NULL);
        ASSERT(m_note_off.get(n+1) != NULL);
        ASSERT(m_note_off.get(n) != 0);
        ASSERT(m_note_off.get(n+1) != 0);

        if (m_note_off[n].getEndTick() > m_note_off[n+1].getEndTick())
        {
            m_note_off.swap(n, n+1);
            if (n>2) n-= 2;
            else n=-1;
        }//end if
    }//next

}

// ----------------------------------------------------------------------------------------------------------

void Track::reorderControlVector()
{
    // FIXME - innefficient implementation
    // FIXME - if bugs in controller editor are fixed that method shouldn't even be necessary
    const int ctrlAmount = m_control_events.size();

    for (int n=0; n<ctrlAmount-1; n++)
    {

        ASSERT_E(n+1,<,ctrlAmount);

        if (m_control_events[n].getTick() > m_control_events[n+1].getTick())
        {
            m_control_events.swap(n, n+1);
            if (n>2) n-= 2;
            else n=0;
        }
    }//next

}

// ----------------------------------------------------------------------------------------------------------

void Track::mergeTrackIn(Track* track)
{
    const int noteAmount = track->m_notes.size();
    for (int n=0; n<noteAmount; n++)
    {
        Note* a = new Note(track->m_notes[n]);
        addNote(a, false);
    }

    const int controllerAmount = track->m_control_events.size();
    for (int n=0; n<controllerAmount; n++)
    {
        addControlEvent( new ControllerEvent(track->m_control_events[n].getController(),
                                             track->m_control_events[n].getTick(),
                                             track->m_control_events[n].getValue()) );

    }

}

// ----------------------------------------------------------------------------------------------------------

bool Track::checkControlEventsOrder()
{
    int ptick = -1;
    for (int n=0; n<m_control_events.size(); n++)
    {
        if (ptick != -1 and m_control_events[n].getTick() < ptick)
        {
            return false;
        }
        ptick = m_control_events[n].getTick();
    }
    return true;
}

// =======================================================================================================
// ============================================== Notes ==================================================
// =======================================================================================================

#if 0
#pragma mark -
#pragma mark Notes
#endif

Note* Track::getNote(const int id)
{
    ASSERT_E(id,<,m_notes.size());
    return m_notes.get(id);
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteStartInMidiTicks(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].getTick();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteEndInMidiTicks(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].getEndTick();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNotePitchID(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].getPitchID();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteAmount() const
{
    return m_notes.size();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteVolume(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].getVolume();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteString(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    //if (m_notes[id].tuning == NULL) m_notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning
    if (m_notes[id].getString() == -1) m_notes[id].findStringAndFretFromNote();

    return m_notes[id].getString();
}


// ----------------------------------------------------------------------------------------------------------

int Track::getNoteFret(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    //if (m_notes[id].tuning == NULL) m_notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning

    //FIXME: I think Note::getFret also does this check, so we're duplicating it here...
    if (m_notes[id].getFret() == -1) m_notes[id].findStringAndFretFromNote();

    return m_notes[id].getFret();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteStringConst(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].getStringConst();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getNoteFretConst(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].getFretConst();
}


// ----------------------------------------------------------------------------------------------------------

int Track::findFirstNoteInRange(const int fromTick, const int toTick) const
{
    const int noteAmount = m_notes.size();

    for (int n=0; n<noteAmount; n++)
    {
        if (m_notes[n].getTick() >= fromTick and m_notes[n].getTick() < toTick) return n;
        if (m_notes[n].getTick() >= toTick) return -1;
    }
    return -1;
}

// ----------------------------------------------------------------------------------------------------------

int Track::findLastNoteInRange(const int fromTick, const int toTick) const
{
    const int noteAmount = m_notes.size();

    for (int n=noteAmount-1; n>-1; n--)
    {
        if (m_notes[n].getTick() >= fromTick and m_notes[n].getTick() < toTick) return n;
        if (m_notes[n].getTick() < fromTick) return -1;
    }
    return -1;
}

// ----------------------------------------------------------------------------------------------------------

int Track::getControllerEventAmount(const bool isLyrics, const bool isTempo) const
{
    if (isTempo)       return m_sequence->getTempoEventAmount();
    else if (isLyrics) return m_sequence->m_text_events.size();
    else               return m_control_events.size();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getControllerEventAmount(const int controller) const
{
    if (Track::isTempoController(controller))
    {
        return m_sequence->getTempoEventAmount();
    }
    else
    {
        int occurrences = 0;

        const int count = m_control_events.size();
        for (int n=0; n<count; n++)
        {
            if (m_control_events[n].getController() == controller) occurrences++;
        }

        return occurrences;
    }
}

// ----------------------------------------------------------------------------------------------------------

ControllerEvent* Track::getControllerEvent(const int id, const int controllerTypeID)
{
    ASSERT_E(id,>=,0);
    if (controllerTypeID == PSEUDO_CONTROLLER_TEMPO)
    {
        // FIXME: silly to access sequence events through Track!!
        ASSERT_E(id,<,m_sequence->getTempoEventAmount());
        return &m_sequence->m_tempo_events[id];
    }
    else if (controllerTypeID == PSEUDO_CONTROLLER_LYRICS)
    {
        // FIXME: silly to access sequence events through Track!!
        ASSERT_E(id,<,m_sequence->m_text_events.size());
        return &m_sequence->m_text_events[id];
    }
    else
    {
        ASSERT_E(id,<,m_control_events.size());
        return &m_control_events[id];
    }
}

// ----------------------------------------------------------------------------------------------------------

int Track::getFirstNoteTick(bool selectionOnly) const
{

    if (not selectionOnly) return m_notes[0].getTick();

    const int noteAmount = m_notes.size();
    int tick = -1;

    for (int n=0; n<noteAmount; n++)
    {
        if ( m_notes[n].isSelected() ) return m_notes[n].getTick();
    }//next

    return tick;

}

// ----------------------------------------------------------------------------------------------------------

int Track::getFirstSelectedNote() const
{
    const int count = m_notes.size();
    for (int n=0; n<count; n++)
    {
        if (m_notes[n].isSelected()) return n;
    }
    return -1;
}

// ----------------------------------------------------------------------------------------------------------

void Track::selectNote(const int id, const bool selected, bool ignoreModifiers)
{
    ASSERT(id != SELECTED_NOTES); // not supported in this function

    if (not ignoreModifiers and not Display::isSelectMorePressed() and
        not Display::isSelectLessPressed())
    {
        ignoreModifiers = true; // if no modifier is pressed, don't do any special checks
    }

    // ---- select/deselect all notes
    if (id == ALL_NOTES)
    {
        // FIXME: controller ediors checks don't belong here
        /*
        if (m_editor_mode != CONTROLLER)
        {
         */

            if (ignoreModifiers)
            {
                const int count = m_notes.size();
                for (int n=0; n<count; n++)
                {
                    m_notes[n].setSelected(selected);
                }//next
            }//end if

        /*
        }// end if
         */
    }
    else  // ---- select/deselect one specific note
    {

        ASSERT_E(id,>=,0);
        ASSERT_E(id,<,m_notes.size());

        // if we ignore +/- key modifiers, just set the value right away
        if (ignoreModifiers)
        {
            m_notes[id].setSelected(selected);
        }
        else
        {
            // otherwise, check key modifiers and set value accordingly
            if (selected)
            {
                if      (Display::isSelectMorePressed()) m_notes[id].setSelected(true);
                else if (Display::isSelectLessPressed()) m_notes[id].setSelected(not selected);
            }
        }//end if

    }//end if
}

// ----------------------------------------------------------------------------------------------------------

bool Track::isNoteSelected(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].isSelected();
}

// ----------------------------------------------------------------------------------------------------------

void Track::updateNotesForGuitarEditor()
{
    const int amount = m_notes.size();
    for (int n=0; n<amount; n++)
    {
        m_notes[n].checkIfStringAndFretMatchNote(true);
    }
}

// ----------------------------------------------------------------------------------------------------------

int Track::snapMidiTickToGrid(int tick, bool isNoteStart, bool is_ceil)
{
    int origin_tick = 0;
    MeasureData* md = m_sequence->getMeasureData();
    if (not md->isMeasureLengthConstant())
    {
        const int measure = md->measureAtTick(tick);
        origin_tick = md->firstTickInMeasure(measure);
    }

    int divider = getMagneticGrid()->getDivider();

    const bool dotted = getMagneticGrid()->isDotted();

    float ticklen;
    if (dotted)
    {
        ticklen = (float)(m_sequence->ticksPerBeat()*4 / divider);
        if (isNoteStart)
        {
            // when note is dotted, allow to start note at halves too
            ticklen = std::min(ticklen/2.0f, (float)m_sequence->ticksPerBeat());
        }
        else
        {
            ticklen = ticklen*1.5f;
        }
    }
    else
    {
        if (isNoteStart and divider < 4) divider = 4; // for note start, allow starting at every bea at least
        ticklen = (float)(m_sequence->ticksPerBeat()*4 / divider);
    }

    if (is_ceil)
    {
        return origin_tick + (int)(ceil((float)(tick - origin_tick)/ticklen)*ticklen);
    }

    return origin_tick + (int)(round((float)(tick - origin_tick)/ticklen)*ticklen);
}

// -------------------------------------------------------------------------------------------------------

void Track::copy()
{
    // FIXME: controller editor checks don't belong here
    /*
    if (m_editor_mode == CONTROLLER)
    {
        wxBell();
        return; // no copy/paste in controller mode
    }
     */

    Clipboard::clear();
    Clipboard::setBeatLength(m_sequence->ticksPerBeat());

    int tickOfFirstSelectedNote=-1;
    // place all selected notes into clipboard
    for (int n=0; n<m_notes.size(); n++)
    {
        if (!m_notes[n].isSelected()) continue;

        Note* tmp=new Note(m_notes[n]);
        Clipboard::add(tmp);

        // if in guitar mode, make sure string/fret and note match
        if (m_editor_mode[GUITAR])
        {
            Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(false);
        }
        else
        {
            Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(true);
        }

        // find tickOfFirstSelectedNote of the first note
        if (m_notes[n].getTick() < tickOfFirstSelectedNote or tickOfFirstSelectedNote==-1)
        {
            tickOfFirstSelectedNote=m_notes[n].getTick();
        }

    }//next

    // remove all empty measures before notes, so that they appear in the current measure when pasting
    MeasureData* md = m_sequence->getMeasureData();
    const int lastMeasureStart = md->firstTickInMeasure( md->measureAtTick(tickOfFirstSelectedNote) );

    const int clipboard_size = Clipboard::getSize();
    for (int n=0; n<clipboard_size; n++)
    {
        getGraphics()->getFocusedEditor()->moveNote(*Clipboard::getNote(n), -lastMeasureStart, 0);
    }

    m_sequence->setNoteShiftWhenNoScrolling( lastMeasureStart );
}


// =======================================================================================================
// ============================================ Get/Set ==================================================
// =======================================================================================================

#if 0
#pragma mark -
#pragma mark Get/Set Other Stuff
#endif

// ----------------------------------------------------------------------------------------------------------

void Track::setId(const int id)
{
    m_track_id = id;
}

// ----------------------------------------------------------------------------------------------------------

void Track::setName(wxString name)
{
    if (name.Trim().IsEmpty()) m_track_name->setValue( wxString( _("Untitled") ) );
    else                       m_track_name->setValue(name);
}

// ----------------------------------------------------------------------------------------------------------

int Track::getChannel()
{
    // always 9 for drum tracks
    //const bool manual_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;

    if (m_editor_mode[DRUM]) return 9;
    else                     return m_channel;

}

// ----------------------------------------------------------------------------------------------------------

void Track::setChannel(int i)
{
    m_channel = i;

    // check what is the instrument currently used in this channel, if any
    const int trackAmount = m_sequence->getTrackAmount();
    for (int n=0; n<trackAmount; n++) // find another track that has same channel and use the same instrument
    {
        if (m_sequence->getTrack(n) != this &&
            m_sequence->getTrack(n)->getChannel() == m_channel)
        {
            // FIXME: remove this abuse of the 'recursive' parameter
            this->doSetInstrument(m_sequence->getTrack(n)->getInstrument(), true);
            break;
        }
    }//next
}

// ----------------------------------------------------------------------------------------------------------

void Track::setInstrument(int i)
{
    m_instrument->setInstrument(i);
}

// ----------------------------------------------------------------------------------------------------------

void Track::doSetInstrument(int i, bool recursive)
{
    if (m_instrument->getSelectedInstrument() != i)
    {
        m_instrument->setInstrument(i, false);
        if (m_next_instrument_listener != NULL) m_next_instrument_listener->onInstrumentChanged( getInstrument() );
    }

    // if we're in manual channel management mode, change all tracks of the same channel
    // to have the same instrument
    if (m_sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive)
    {
        const int trackAmount = m_sequence->getTrackAmount();
        for (int n=0; n<trackAmount; n++)
        {
            Track* track = m_sequence->getTrack(n);
            if (track == this) continue; // track must not evaluate itself...
            if (track->getChannel() == m_channel)
            {
                track->doSetInstrument(i, true);
            }
        }//next
    }//endif
}

// ----------------------------------------------------------------------------------------------------------

void Track::setDrumKit(int i)
{
    m_drum_kit->setDrumkit(i);
}

// ----------------------------------------------------------------------------------------------------------

void Track::doSetDrumKit(int i, bool recursive)
{

    // check id validity
    if      (i == 0 ) ;  // Standard
    else if (i == 8 ) ;  // Room kit
    else if (i == 16) ;  // Power kit
    else if (i == 24) ;  // Electronic
    else if (i == 25) ;  // Analog
    else if (i == 32) ;  // Jazz
    else if (i == 40) ;  // Brush
    else if (i == 48) ;  // Orchestral
    else if (i == 56) ;  // Special Effects
    else return;         // invalid drum kit ID

    if (m_drum_kit->getSelectedDrumkit() != i)
    {
        m_drum_kit->setDrumkit(i, false);
        if (m_next_drumkit_listener != NULL) m_next_drumkit_listener->onDrumkitChanged( i );
    }


    // if we're in manual channel management mode, change all tracks of the same channel to
    // have the same instrument.
    // FIXME: we ignore when importing since the midi importing algorithm will make sure all tracks on
    //        the same channel have the same instrument, and this code would disrupt the importer.
    //        But we should find a nicer way here...
    if (m_sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive and not m_sequence->isImportMode())
    {

        const int trackAmount = m_sequence->getTrackAmount();
        for (int n=0; n<trackAmount; n++)
        {
            Track* track = m_sequence->getTrack(n);
            if (track == this) continue; // track must not evaluate itself...
            track->doSetDrumKit(i, true);
        }//next
    }//endif
}



// ----------------------------------------------------------------------------------------------------------

int Track::getDuration() const
{
    if (m_note_off.size() < 1) return 0;

    return m_note_off[m_note_off.size()-1].getEndTick();
}

// ----------------------------------------------------------------------------------------------------------

void Track::setKey(const int symbolAmount, const KeyType type)
{
    assert(symbolAmount < 8);

    // This is to support older file formats. TODO: eventually remove compat.
    KeyType actualType = type;
    if (symbolAmount == 0) actualType = KEY_TYPE_C;

    // ---- update "key_sharps_amnt" and "key_flats_amnt" members
    if (actualType == KEY_TYPE_SHARPS)
    {
        m_key_type = actualType;
        m_key_sharps_amnt = symbolAmount;
        m_key_flats_amnt  = 0;
    }
    else if (actualType == KEY_TYPE_FLATS)
    {
        m_key_type = actualType;
        m_key_sharps_amnt = 0;
        m_key_flats_amnt  = symbolAmount;
    }
    else if (actualType == KEY_TYPE_C and symbolAmount == 0)
    {
        m_key_type = actualType;
        m_key_sharps_amnt = 0;
        m_key_flats_amnt  = 0;
    }
    else
    {
        std::cerr << "Bogus call to Track::setKey! Invalid key type.\n";
        ASSERT(false);
    }

    // ---- update 'm_key_notes' array
    // if key is e.g. G Major, "major_note" will be set to note12 equivalent of G.
    // to load a minor key, it's just set to the major one that has same sharps and flats
    // to ease the process
    Note12 major_note12 = NOTE_12_C;

    if (symbolAmount == 0 or m_key_type == KEY_TYPE_C)
    {
        major_note12 = NOTE_12_C;
    }
    else if (m_key_type == KEY_TYPE_SHARPS)
    {
        switch (symbolAmount)
        {
            case 1: major_note12 = NOTE_12_G;       break;
            case 2: major_note12 = NOTE_12_D;       break;
            case 3: major_note12 = NOTE_12_A;       break;
            case 4: major_note12 = NOTE_12_E;       break;
            case 5: major_note12 = NOTE_12_B;       break;
            case 6: major_note12 = NOTE_12_F_SHARP; break;
            case 7: major_note12 = NOTE_12_C_SHARP; break;
        }
    }
    else if (m_key_type == KEY_TYPE_FLATS)
    {
        switch (symbolAmount)
        {
            case 1: major_note12 = NOTE_12_F;      break;
            case 2: major_note12 = NOTE_12_B_FLAT; break;
            case 3: major_note12 = NOTE_12_E_FLAT; break;
            case 4: major_note12 = NOTE_12_A_FLAT; break;
            case 5: major_note12 = NOTE_12_D_FLAT; break;
            case 6: major_note12 = NOTE_12_G_FLAT; break;
            case 7: major_note12 = NOTE_12_B;      break; // C flat
        }
    }

    bool note_12_greyed_out[12];

    {
#define NEXT n--; if (n<0) n+=12
    int n = int(major_note12) + 7;
    if (n > 11) n -= 12;

    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;  NEXT;
    note_12_greyed_out[n] = false; NEXT;
    note_12_greyed_out[n] = true;
#undef NEXT
    }

    Note12 noteName;
    int octave;

    for (int n=0; n<131; n++)
    {
        if (Note::findNoteName(n, &noteName, &octave))
        {
            m_key_notes[n] = (note_12_greyed_out[noteName] ? KEY_INCLUSION_ACCIDENTAL : KEY_INCLUSION_FULL);
        }
        else
        {
            m_key_notes[n] = KEY_INCLUSION_NONE;
        }
    }

    if (m_listener != NULL) 
    {
        m_listener->onKeyChange(symbolAmount, m_key_type);
    }
}

// ----------------------------------------------------------------------------------------------------------

void Track::setCustomKey(const KeyInclusionType key_notes[131])
{
    m_key_type = KEY_TYPE_CUSTOM;
    m_key_sharps_amnt = 0;
    m_key_flats_amnt = 0;

    for (int n=0; n<131; n++)
    {
        m_key_notes[n] = key_notes[n];
    }
}


// ----------------------------------------------------------------------------------------------------------
void Track::setVolume(int volume)
{
    m_volume = volume;
}

int Track::getVolume() const
{
    return m_volume;
}

void Track::toggleMuted()
{
    m_muted = not m_muted;
    m_sequence->updateTrackPlayingStatus();
}

bool Track::isMuted() const
{
    return m_muted;
}

void Track::setMuted(bool muted)
{
    m_muted = muted;
    m_sequence->updateTrackPlayingStatus();
}

void Track::toggleSoloed()
{
    m_soloed = not m_soloed;
    m_sequence->updateTrackPlayingStatus();
}

bool Track::isSoloed() const
{
    return m_soloed;
}
    
void Track::setSoloed(bool soloed)
{
    m_soloed = m_soloed;
    m_sequence->updateTrackPlayingStatus();
}


// True if track if actually played (depends on m_muted and m_soloed)
bool Track::isPlayed() const
{
    return m_played;
}

void Track::setPlayed(bool played)
{
    m_played = played;
}


// ----------------------------------------------------------------------------------------------------------

void Track::onInstrumentChanged(const int newValue)
{
    if (m_next_instrument_listener != NULL)
    {
        m_next_instrument_listener->onInstrumentChanged(newValue);
    }

    doSetInstrument(newValue);
}

// ----------------------------------------------------------------------------------------------------------

void Track::onDrumkitChanged(const int newValue)
{
    if (m_next_drumkit_listener != NULL)
    {
        m_next_drumkit_listener->onDrumkitChanged(newValue);
    }

    doSetDrumKit(newValue);
}

// ----------------------------------------------------------------------------------------------------------

void Track::onGuitarTuningUpdated(GuitarTuning* tuning, const bool userTriggered)
{
    if (userTriggered)
    {
        action( new Action::UpdateGuitarTuning() );
    }
    else
    {
        Action::UpdateGuitarTuning actionObj;
        actionObj.setParentTrack(this, new TrackVisitor(this));
        actionObj.perform();
    }
}

// ----------------------------------------------------------------------------------------------------------

void Track::setNotationType(NotationType t, bool enabled)
{
    m_editor_mode[t] = enabled;
    if (m_listener != NULL) m_listener->onNotationTypeChange();

    // TODO: move this code to the guitar editor
    if (t == GUITAR and enabled) updateNotesForGuitarEditor();
}

// ----------------------------------------------------------------------------------------------------------

int Track::getEnabledEditorCount() const
{
    int count = 0;
    for (int n=0; n<NOTATION_TYPE_COUNT; n++)
    {
        if (m_editor_mode[n]) count++;
    }

    return count;
}

// ----------------------------------------------------------------------------------------------------------

void Track::setDefaultVolume(const int v)
{
    m_default_volume = v;
}

// =======================================================================================================
// ========================================== Midi playback ==============================================
// =======================================================================================================
#if 0
#pragma mark -
#pragma mark Playback
#endif

void Track::playNote(const int id, const bool noteChange)
{
    ASSERT_E(id,<,m_notes.size());
    ASSERT_E(id,>=,0);

    m_notes[id].play(noteChange);
}

// ----------------------------------------------------------------------------------------------------------

/** @return Smallest values, ignoring -1, a being prioritary to b (returns -1 for none, 0 for a, 1 for b) */
int getActiveMin(bool have_a, int a, bool have_b, int b)
{
    if (not have_a and not have_b) return -1;
    if (not have_a) return 1;//b
    if (not have_b) return 0;//a

    if (a <= b) return 0;//a
    else return 1;//b
}

// ----------------------------------------------------------------------------------------------------------

/**
  * @return Smallest values, -1 being considered as 'no value'
  *         if some are equal, 'a' is prioritary to 'b', and 'b' prioritary to 'c'
  *         (returns -1 for none, 0 for a, 1 for b, 2 for c)
  */
int getActiveMin(bool have_a, int a, bool have_b, int b, bool have_c, int c)
{

    const int result1 = getActiveMin(have_b, b, have_c, c);

    bool have_passnumber = true;
    int passnumber;
    if (result1 == 0) passnumber = b;
    else if (result1 == 1) passnumber = c;
    else if (result1 == -1)
    {
        passnumber = -1;
        have_passnumber = false;
    }

    const int result2 = getActiveMin( have_a, a, have_passnumber, passnumber );

    if (result2 == 0) return 0;
    else
    {
        if (result1 == -1) return -1;
        return result1 + 1;
    }

}

// ----------------------------------------------------------------------------------------------------------

int Track::addMidiEvents(jdksmidi::MIDITrack* midiTrack,
                         int channel,
                         int firstMeasure,
                         bool selectionOnly,
                         int& startTick)
{

    // ignore track if it has been muted
    // (but for some reason drum track can't be completely omitted)
    // if we only play selection, ignore mute and play anyway
    if (!m_played and not m_editor_mode[DRUM] and not selectionOnly)
    {
        return -1;
    }

    MeasureData* md = m_sequence->getMeasureData();
    const int lastTickInSong = md->firstTickInMeasure( md->getMeasureAmount() );

    // if in manual mode, use the user-specified channel ID and not the stock one
    const bool manual_mode = (m_sequence->getChannelManagementType() == CHANNEL_MANUAL);

    if (manual_mode) channel = getChannel();

    // drum tracks
    if (m_editor_mode[DRUM]) channel = 9;

    //std::cout << "channel = " << track_ID << std::endl;

    // when previewing selected notes (start by finding the note that plays first, to start playing at
    // the right place and note from the beginning)
    int firstNoteStartTick = -1;
    int selectedNoteAmount = 0;

    if (selectionOnly)
    {
        const int noteAmount = m_notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (m_notes[n].isSelected())
            {
                if (m_notes[n].getTick() < firstNoteStartTick or firstNoteStartTick==-1)
                {
                    firstNoteStartTick = m_notes[n].getTick();
                }
                selectedNoteAmount++;
            }
        }

        if (firstNoteStartTick == -1) return -1; // error, no note was found.
        if (selectedNoteAmount == 0)  return -1; // error, no note was found.

    }
    else
    {
        firstNoteStartTick = m_sequence->getMeasureData()->firstTickInMeasure(firstMeasure);
        startTick = firstNoteStartTick;
    }

    // set bank
    {
        jdksmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( channel, 0, 0 );

        if (not midiTrack->PutEvent( m ))
        {
            std::cerr << "Error adding event" << std::endl;
            ASSERT(false);
        }

        m.SetTime( 0 );
        m.SetControlChange( channel, 32, 0 );

        if (not midiTrack->PutEvent( m ))
        {
            std::cout << "Error adding event" << std::endl;
            ASSERT(false);
        }
    }

    // set instrument
    {
        jdksmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );

        if (m_editor_mode[DRUM]) m.SetProgramChange( channel, getDrumKit() );
        else                     m.SetProgramChange( channel, getInstrument() );

        if (not midiTrack->PutEvent( m ))
        {
            std::cerr << "Error adding instrument at track beginning!" << std::endl;
        }
    }

    // set track name
    {

        jdksmidi::MIDITimedBigMessage m;
        m.SetText( 3 );
        m.SetByte1( 3 );


        wxString track_name = m_track_name->getValue();

        /* This doesn't work under Linux: no track name seen in MIDI track
        jdksmidi::MIDISystemExclusive sysex((unsigned char*)(const char*)track_name.mb_str(wxConvUTF8),
                                           track_name.size()+1, track_name.size()+1, false);
        */

        int size = track_name.size()+1;
        char* charTrackName;
        charTrackName = new char[size];
        memcpy(charTrackName, track_name.mb_str(), size);
        charTrackName[size-1] = '\0';
        jdksmidi::MIDISystemExclusive sysex((unsigned char*)charTrackName, size, size, false);

        m.CopySysEx( &sysex );
        m.SetTime( 0 );
        if (not midiTrack->PutEvent( m ))
        {
            std::cout << "Error adding event" << std::endl;
            ASSERT(FALSE);
        }

        delete[] charTrackName;
    }

    // set maximum volume
    {
        jdksmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( channel, 7, SCHAR_MAX);

        if (not midiTrack->PutEvent( m ))
        {
            std::cerr << "Error adding event" << std::endl;
            ASSERT(false);
        }
    }

    // ----------------------------------- add events in order --------------------------
    /*
     * The way this section works:
     *
     * there are currently 3 possible source of events in a track (apart those above, like instrument and
     * track name): note on, note off, controller change.
     * each type of event is stored in its own vector, in time order.
     * the variables below store the current event (i.e. the first event that hasn't yet been added)
     * the section loops, and with it each iteration it checks the current tick of the 3 current events,
     * then picks the one with the smallest tick and adds it to the track.
     *
     */

    int note_on_id     = 0;
    int note_off_id    = 0;
    int control_evt_id = 0;

    const int noteOnAmount     = m_notes.size();
    const int noteOffAmount    = m_note_off.size();
    const int controllerAmount = m_control_events.size();

    // find track end
    int last_event_tick = 0;

    // if muted and drums, return now
    if (!m_played and m_editor_mode[DRUM] and not selectionOnly) return -1;

    //std::cout << "-------------------- TRACK -------------" << std::endl;

    while (true)
    {

        // if we only want to play what's selected, skip unselected notes
        if (selectionOnly)
        {
            while (note_on_id < noteOnAmount   and not m_notes[note_on_id].isSelected())
            {
                note_on_id++;
            }
            while (note_off_id < noteOffAmount and not m_note_off[note_off_id].isSelected())
            {
                note_off_id++;
            }
        }

        bool have_tick_on = (note_on_id < noteOnAmount);
        bool have_tick_off = (note_off_id < noteOffAmount);

        const int tick_on  = have_tick_on   ?
                              m_notes[note_on_id].getTick() - firstNoteStartTick   :  -1;
        const int tick_off = have_tick_off ?
                              m_note_off[note_off_id].getEndTick() - firstNoteStartTick :  -1;
        
        // ignore control events when only playing selection
        bool have_tick_control = (control_evt_id < controllerAmount and not selectionOnly);
        const int tick_control = have_tick_control ?
                                  m_control_events[control_evt_id].getTick() - firstNoteStartTick : -1;


        if (not have_tick_control and not have_tick_off and not have_tick_on)
        {
            // all events have been added
            break;
        }
        
        const int activeMin = getActiveMin(have_tick_off, tick_off,
                                           have_tick_control, tick_control,
                                           have_tick_on, tick_on );
        
        jdksmidi::MIDITimedBigMessage m;

        //  ------------------------ add note on event ------------------------
        if (activeMin == 2)
        {
            const int time = m_notes[note_on_id].getTick() - firstNoteStartTick;
            if (time >= 0 and (time + firstNoteStartTick) <= lastTickInSong)
            {
                m.SetTime( time );

                if (m_editor_mode[DRUM])
                {
                    m.SetNoteOn(channel, m_notes[note_on_id].getPitchID(), computeNoteVolume(note_on_id));
                }
                else
                {
                    m.SetNoteOn(channel, 131-m_notes[note_on_id].getPitchID(), computeNoteVolume(note_on_id));
                }

                // find track end
                if (m_notes[note_on_id].getEndTick() > last_event_tick)
                {
                    last_event_tick = m_notes[note_on_id].getEndTick();
                }

                if (not midiTrack->PutEvent( m ))
                {
                    std::cerr << "Error adding midi event!" << std::endl;
                }
            }

            note_on_id++;
        }
        //  ------------------------ add note off event ------------------------
        else if (activeMin == 0)
        {

            const int time=m_note_off[note_off_id].getEndTick() - firstNoteStartTick;
            if (time >= 0 and (time + firstNoteStartTick) <= lastTickInSong)
            {
                m.SetTime( time );

                if (m_editor_mode[DRUM])
                {
                    m.SetNoteOff( channel, m_note_off[note_off_id].getPitchID(), 0 );
                }
                else
                {
                    m.SetNoteOff( channel, 131 - m_note_off[note_off_id].getPitchID(), 0 );
                }

                // find track end
                if (time > last_event_tick) last_event_tick = time;

                if (not midiTrack->PutEvent( m ))
                {
                    std::cerr << "Error adding midi event!" << std::endl;
                }
            }
            note_off_id++;
        }
        //  ------------------------ add control change event ------------------------
        else if (activeMin == 1)
        {
            const int controllerID = m_control_events[control_evt_id].getController();

            // pitch bend
            if (controllerID == PSEUDO_CONTROLLER_PITCH_BEND)
            {
                int time = m_control_events[control_evt_id].getTick() - firstNoteStartTick;


                // controller changes happens before the area we play
                // but perhaps it still is affecting the area we want to play - check for that.
                bool doAddControlEvent = true;
                if (time < 0)
                {
                    if (control_evt_id+1 < controllerAmount)
                    {
                        doAddControlEvent = false;
                        int checkEventID = control_evt_id+1;

                        // check if there are other controller events of the same type before the area we play.
                        while (true)
                        {

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)<1  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // the current event has no effect, there is another one later, disregard it.
                                doAddControlEvent = false;
                                break;
                            }

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)>0  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // there is another event, but it's later so current event is still relevant.
                                doAddControlEvent = true;
                                time = 0;
                                break;
                            }

                            checkEventID++;
                            if (not (checkEventID < controllerAmount))
                            {
                                // we reaached the end, there are no other events of the same type
                                // this one still affects playback of the area that we're playing.
                                doAddControlEvent = true;
                                time = 0;
                                break;
                            }
                        }//wend
                    }
                    else
                    {
                        // there are no other events in track. add it.
                        doAddControlEvent = true;
                        time = 0;
                    }
                }
                else
                {
                    doAddControlEvent = true;
                }

                if (doAddControlEvent and (time + firstNoteStartTick) <= lastTickInSong)
                {
                    m.SetTime( time );

                    /** In range [-8192, 8191] */
                    const int pitchBendVal = m_control_events[control_evt_id].getPitchBendValue();

                    m.SetPitchBend(channel, pitchBendVal);

                    if (not midiTrack->PutEvent(m)) { std::cout << "Error adding midi event!" << std::endl; }
                }
                control_evt_id++;

            }
            else if (controllerID == PSEUDO_CONTROLLER_INSTRUMENT_CHANGE)
            {
                int time = m_control_events[control_evt_id].getTick() - firstNoteStartTick;
                if ((time + firstNoteStartTick) <= lastTickInSong)
                {
                    m.SetTime( time );
                    m.SetProgramChange(channel, (int)round(m_control_events[control_evt_id].getValue()));
                    control_evt_id++;

                    if (not midiTrack->PutEvent( m ))
                    {
                        std::cerr << "Error adding midi event!" << std::endl;
                    }
                }
            }
            // other controller
            else
            {
                int time = m_control_events[control_evt_id].getTick() - firstNoteStartTick;

                // controller changes happens before the area we play
                // but perhaps it still is affecting the area we want to play - check for that.
                bool doAddControlEvent = true;
                if (time < 0)
                {
                    if (control_evt_id+1 < controllerAmount)
                    {
                        doAddControlEvent = false;
                        int checkEventID = control_evt_id+1;

                        // check if there are other controller events of the same type before the area we play.
                        while (true)
                        {

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)<1  and
                                m_control_events[checkEventID].getController() == controllerID)
                            {
                                // the current event has no effect, there is another one later, disregard it.
                                doAddControlEvent = false;
                                break;
                            }

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)>0  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // there is another event, but it's later so current event is still relevant.
                                doAddControlEvent = true;
                                time = 0;
                                break;
                            }

                            checkEventID++;
                            if (not (checkEventID < controllerAmount ))
                            {
                                // we reaached the end, there are no other events of the same type
                                // this one still affects playback of the area that we're playing.
                                doAddControlEvent = true;
                                time = 0;
                                break;
                            }
                        }//wend
                    }
                    else
                    {
                        // there are no other events in track. add it.
                        doAddControlEvent = true;
                        time = 0;
                    }
                }
                else
                {
                    doAddControlEvent = true;
                }

                if (doAddControlEvent and (time + firstNoteStartTick) <= lastTickInSong)
                {
                    m.SetTime( time );

                    // FIXME: also write fine values
                    m.SetControlChange(channel,
                                       controllerID,
                                       127 - (int)round(m_control_events[control_evt_id].getValue()) );

                    if (not midiTrack->PutEvent( m ))
                    {
                        std::cerr << "Error adding midi event!" << std::endl;
                    }
                }

                control_evt_id++;

            }//endif (control/pitch-bend)

        }// if (note/note off/control)

    }//wend


    if (selectionOnly) startTick = firstNoteStartTick;

    return last_event_tick - firstNoteStartTick;
}

// =======================================================================================================
// ================================================ IO ===================================================
// =======================================================================================================
#if 0
#pragma mark -
#pragma mark Serialization
#endif

void Track::saveToFile(wxFileOutputStream& fileout)
{
    reorderNoteVector();
    reorderNoteOffVector();

    writeData(wxT("\n<track name=\"") + m_track_name->getValue() +
              wxT("\" id=\"") + to_wxString(m_track_id) +
              wxT("\" channel=\"") + to_wxString(m_channel) +
              (m_muted ? wxT("\" muted=\"true") : wxT("") )  +
              (m_soloed ? wxT("\" soloed=\"true") : wxT("") )  +
              wxT("\" volume=\"") + to_wxString(m_volume) +
              wxT("\" default_volume=\"") + to_wxString(m_default_volume) +
              wxT("\">\n"), fileout );

    switch (m_key_type)
    {
        case KEY_TYPE_C:
            writeData(wxT("  <key type=\"C\" />\n"), fileout);
            break;
        case KEY_TYPE_SHARPS:
            writeData(wxT("  <key type=\"sharps\" value=\"") + to_wxString( getKeySharpsAmount() ) +
                      wxT("\" />\n"), fileout);
            break;

        case KEY_TYPE_FLATS:
            writeData(wxT("  <key type=\"flats\" value=\"") + to_wxString( getKeyFlatsAmount() ) +
                      wxT("\" />\n"), fileout);
            break;

        case KEY_TYPE_CUSTOM:
            writeData( wxT("  <key type=\"custom\" value=\""), fileout);

            // saved in MIDI order, not in my weird pitch ID order
            char value[128];
            for (int n=130; n>3; n--)
            {
                value[n-4] = '0' + (int)m_key_notes[n];
            }
            value[127] = '\0';
            wxString wxStringValue(value, wxConvUTF8);
            writeData( wxStringValue, fileout );
            writeData( wxT("\" />"), fileout );
            break;
    }

    getGraphics()->saveToFile(fileout);

    // notes
    const int noteCount = m_notes.size();
    for (int n=0; n<noteCount; n++)
    {
        m_notes[n].saveToFile(fileout);
    }

    // controller changes
    const int ctrlCount = m_control_events.size();
    for (int n=0; n<ctrlCount; n++)
    {
        m_control_events[n].saveToFile(fileout);
    }

    writeData(wxT("</track>\n\n"), fileout );


}

// ----------------------------------------------------------------------------------------------------------

// FIXME(DESIGN): remove references to GraphicalSequence from model classes
bool Track::readFromFile(irr::io::IrrXMLReader* xml, GraphicalSequence* gseq)
{

    m_notes.clearAndDeleteAll();
    m_note_off.clearWithoutDeleting(); // have already been deleted by previous command
    m_control_events.clearAndDeleteAll();

    // parse XML file
    do
    {

        switch (xml->getNodeType())
        {
            case irr::io::EXN_TEXT:
            {
                break;
            }
            case irr::io::EXN_ELEMENT:
            {
                if (strcmp("track", xml->getNodeName()) == 0)
                {
                    const char* muted_c = xml->getAttributeValue("muted");
                    if (muted_c != NULL)
                    {
                        if (strcmp(muted_c, "true") == 0)       m_muted = true;
                        else if (strcmp(muted_c, "false") == 0) m_muted = false;
                        else
                        {
                            m_muted = false;
                            std::cerr << "Unknown keyword for attribute 'muted' in track: " << muted_c << std::endl;
                        }

                    }
                    else
                    {
                        m_muted = false;
                    }
                    
                    
                    const char* soloed_c = xml->getAttributeValue("soloed");
                    if (soloed_c != NULL)
                    {
                        if (strcmp(soloed_c, "true") == 0)       m_soloed = true;
                        else if (strcmp(soloed_c, "false") == 0) m_soloed = false;
                        else
                        {
                            m_soloed = false;
                            std::cerr << "Unknown keyword for attribute 'muted' in track: " << soloed_c << std::endl;
                        }

                    }
                    else
                    {
                        m_soloed = false;
                    }
                    
                    
                    const char* track_id_c = xml->getAttributeValue("id");
                    if (track_id_c != NULL)
                    {
                        m_track_id = atoi(track_id_c);
                    }
                    
                    
                    const char* volume_c = xml->getAttributeValue("volume");
                    if (volume_c != NULL)
                    {
                        m_volume = atoi(volume_c);
                    }

                    const char* default_volume_c = xml->getAttributeValue("default_volume");
                    if (default_volume_c != NULL)
                    {
                        m_default_volume = atoi(default_volume_c);
                    }

                    const char* name = xml->getAttributeValue("name");
                    if (name != NULL)
                    {
                        setName( fromCString((char*)name) );
                    }
                    else
                    {
                        std::cerr << "Missing info from file: track name" << std::endl;
                        setName( wxString(_("Untitled")) );
                    }

                    const char* channel_c = xml->getAttributeValue("channel");
                    if (channel_c != NULL)
                    {
                        int loaded_channel = atoi(channel_c);
                        if (loaded_channel >=-0 and loaded_channel<16)
                        {
                            m_channel = loaded_channel;
                        }
                        else
                        {
                            std::cerr << "Invalid channel : " << loaded_channel << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << "Missing info from file: track channel" << std::endl;
                    }

                }
                else if (strcmp("instrument", xml->getNodeName()) == 0)
                {
                    const char* id = xml->getAttributeValue("id");
                    if (id != NULL)
                    {
                        // FIXME: remove this abuse of the 'recursive' parameter
                        doSetInstrument(atoi(id), true);
                    }
                    else
                    {
                        std::cerr << "Missing info from file: instrument ID" << std::endl;
                    }
                }
                else if (strcmp("magneticgrid", xml->getNodeName()) == 0)
                {
                    if (not m_magnetic_grid->readFromFile(xml)) return false;
                }
                // TODO: this is for backwards compatibility only, eventually remove
                else if (strcmp("editor", xml->getNodeName()) == 0)
                {
                    const char* mode_c = xml->getAttributeValue("mode");

                    setNotationType(SCORE, false);
                    setNotationType(KEYBOARD, false);
                    setNotationType(DRUM, false);
                    setNotationType(GUITAR, false);
                    setNotationType(CONTROLLER, false);

                    // In support for old format (TODO: eventually remove)
                    if ( mode_c != NULL )
                    {
                        setNotationType((NotationType)atoi( mode_c ), true);
                    }
                    else
                    {
                        // New format
                        const char* mode_score = xml->getAttributeValue("score");
                        if ( mode_score != NULL ) setNotationType(SCORE, strcmp(mode_score, "true") == 0);

                        const char* mode_keyb = xml->getAttributeValue("keyboard");
                        if ( mode_keyb!= NULL ) setNotationType(KEYBOARD, strcmp(mode_keyb, "true") == 0);

                        const char* mode_guitar = xml->getAttributeValue("guitar");
                        if ( mode_guitar != NULL ) setNotationType(GUITAR, strcmp(mode_guitar, "true") == 0);

                        const char* mode_drum = xml->getAttributeValue("drum");
                        if ( mode_drum != NULL ) setNotationType(DRUM, strcmp(mode_drum, "true") == 0);

                        const char* mode_ctrl = xml->getAttributeValue("controller");
                        if ( mode_ctrl != NULL ) setNotationType(CONTROLLER, strcmp(mode_ctrl, "true") == 0);
                    }


                    if (not isNotationTypeEnabled(SCORE)  and not isNotationTypeEnabled(KEYBOARD) and
                        not isNotationTypeEnabled(GUITAR) and not isNotationTypeEnabled(DRUM) and
                        not isNotationTypeEnabled(CONTROLLER))
                    {
                        std::cerr << "Missing info from file: notation type" << std::endl;
                        setNotationType(KEYBOARD, true);
                    }

                    getGraphics()->readFromFile(xml);
                }
                else if (strcmp("editors", xml->getNodeName()) == 0)
                {
                    setNotationType(SCORE, false);
                    setNotationType(KEYBOARD, false);
                    setNotationType(DRUM, false);
                    setNotationType(GUITAR, false);
                    setNotationType(CONTROLLER, false);
                    getGraphics()->readFromFile(xml);
                }
                else if (strcmp("guitartuning", xml->getNodeName()) == 0)
                {
                    GuitarTuning* tuning = getGuitarTuning();

                    std::vector<int> newTuning;

                    int n=0;
                    char* string_v = (char*)xml->getAttributeValue("string0");

                    while (string_v != NULL)
                    {
                        newTuning.push_back( atoi(string_v) );

                        n++;
                        wxString tmp = wxT("string") + to_wxString(n);
                        string_v = (char*)xml->getAttributeValue( tmp.mb_str() );
                    }

                    if (newTuning.size() < 3)
                    {
                        std::cout << "FATAL ERROR: Invalid tuning!! only " << newTuning.size()
                                  << " strings found" << std::endl;
                    }
                    else
                    {
                        tuning->setTuning(newTuning, false);
                    }
                }
                // FIXME: this is SAVED in GraphicalTrack but LOADED here. wtf.
                else if (strcmp("drumkit", xml->getNodeName()) == 0)
                {
                    const char* id = xml->getAttributeValue("id");
                    if (id != NULL)
                    {
                        // FIXME: remove this abuse of the 'recursive' parameter
                        doSetDrumKit(atoi(id), true);
                    }
                    else
                    {
                        std::cerr << "Missing info from file: drum ID" << std::endl;
                    }

                    const char* collapse = xml->getAttributeValue("collapseView");
                    if (collapse != NULL && strcmp(collapse, "true") == 0)
                    {
                        getGraphics()->getDrumEditor()->setShowOnlyUsedDrums(true);
                    }
                }
                // TODO: for backwards compatibility only, eventually remove
                else if (strcmp("controller", xml->getNodeName()) == 0)
                {
                    const char* id = xml->getAttributeValue("id");
                    if (id != NULL)
                    {
                        // FIXME: remove GUI calls from here
                        getGraphics()->getControllerEditor()->setController(atoi(id));
                    }
                }
                else if (strcmp("key", xml->getNodeName()) == 0)
                {
                    char* key_type  = (char*)xml->getAttributeValue("type");
                    if (key_type != NULL)
                    {
                        if (strcmp(key_type, "C") == 0)
                        {
                            setKey(0, KEY_TYPE_C);
                        }
                        else if (strcmp(key_type, "sharps") == 0)
                        {
                            char* count_c = (char*)xml->getAttributeValue("value");
                            int count;

                            if (count_c != NULL)  count = atoi(count_c);

                            // For now we tolerator 0 because older file formats used it.
                            // TODO: eventually remove old format compat
                            if (count_c == NULL or count < 0 or count > 7)
                            {
                                std::cerr << "Warning : malformed key in .aria file for track "
                                          << getName().mb_str()
                                          << " : 'sharp' key type needs a value in range [1 .. 7]"
                                          << std::endl;
                            }
                            else
                            {
                                setKey(count, KEY_TYPE_SHARPS);
                            }
                        }
                        else if (strcmp(key_type, "flats") == 0)
                        {
                            char* count_c = (char*)xml->getAttributeValue("value");
                            int count;

                            if (count_c != NULL)  count = atoi(count_c);

                            // For now we tolerate 0 because older file formats used it.
                            // TODO: eventually remove old format compat
                            if (count_c == NULL or count < 0 or count > 7)
                            {
                                std::cerr << "Warning : malformed key in .aria file for track "
                                          << getName().mb_str()
                                          << " : 'flats' key type needs a value in range [1 .. 7]"
                                          << std::endl;
                            }
                            else
                            {
                                setKey(count, KEY_TYPE_FLATS);
                            }
                        }
                        else if (strcmp(key_type, "custom") == 0)
                        {
                            char* value_c = (char*)xml->getAttributeValue("value");
                            if (value_c == NULL or strlen(value_c) != 127)
                            {
                                std::cerr << "Warning : malformed key in .aria file for track "
                                          << getName().mb_str()
                                          << " : 'custom' key type needs a value of 127 characters"
                                          << std::endl;
                            }
                            else
                            {
                                // saved in MIDI order, not in my weird pitch ID order
                                for (int n=4; n<131; n++)
                                {
                                    char c = value_c[n-4];
                                    if (c == '2') m_key_notes[n] = KEY_INCLUSION_FULL;
                                    else if (c == '1') m_key_notes[n] = KEY_INCLUSION_ACCIDENTAL;
                                    else if (c == '0') m_key_notes[n] = KEY_INCLUSION_NONE;
                                    else
                                    {
                                        std::cerr << "Warning : malformed key in .aria file for track "
                                                  << getName().mb_str()
                                                  << " : 'custom' key type needs a value made of 0/1 characters"
                                                  << std::endl;
                                    }
                                }
                                m_key_notes[3] = KEY_INCLUSION_NONE;
                                m_key_notes[2] = KEY_INCLUSION_NONE;
                                m_key_notes[1] = KEY_INCLUSION_NONE;
                                m_key_notes[0] = KEY_INCLUSION_NONE;
                            }
                        }
                        else
                        {
                            std::cerr << "Warning : malformed key in .aria file for track "
                                      << getName().mb_str() << " : unknown key type '" << key_type << "'"
                                      << std::endl;
                        }

                    }
                    else
                    {
                        // current key format not found, check for old format
                        // TODO: eventuall remove support for old format (file format version 1.0000)
                        char* flats_c  = (char*)xml->getAttributeValue("flats");
                        char* sharps_c = (char*)xml->getAttributeValue("sharps");

                        int sharps = 0, flats = 0;
                        if (flats_c != NULL or sharps_c != NULL)
                        {
                            if (flats_c != NULL)  flats = atoi(flats_c);
                            if (sharps_c != NULL) sharps = atoi(sharps_c);
                            //std::cout << "sharps = " << sharps << " flats = " << flats << std::endl;

                            if (sharps > flats) setKey(sharps, KEY_TYPE_SHARPS);
                            else                setKey(flats,  KEY_TYPE_FLATS);
                        }
                    }

                }
                else if (strcmp("note", xml->getNodeName()) == 0)
                {
                    Note* temp = new Note(this);
                    if (not temp->readFromFile(xml))
                    {
                        std::cerr << "A note was discarded because it is invalid" << std::endl;
                        delete temp;
                    }
                    else
                    {
                        addNote( temp );
                    }
                }
                else if (strcmp("controlevent", xml->getNodeName()) == 0)
                {

                    ControllerEvent* temp = new ControllerEvent(0, 0, 0);
                    if (not temp->readFromFile(xml))
                    {
                        // thre was an error when trying to load control event
                        std::cerr << "A controller event was discarded because it is invalid" << std::endl;
                        delete temp;
                    }
                    else
                    {
                        // controller successfully loaded, add to controllers vector
                        m_control_events.push_back( temp );
                    }
                }

            }// end case

                break;
            case irr::io::EXN_ELEMENT_END:
            {

                if (strcmp("track", xml->getNodeName()) == 0)
                {
                    reorderNoteOffVector();

                    // now that we have the set of notes, we can collapse the view if needed
                    GraphicalTrack* gtrack = getGraphics();
                    if (gtrack->getDrumEditor()->showOnlyUsedDrums())
                    {
                        gtrack->getDrumEditor()->useCustomDrumSet();
                    }

                    return true;
                }
            }
                break;

            default:break;
        }//end switch

    } while (xml != NULL and xml->read());

    // FIXME: this code is useless, will be never reached (see "return" above)
    reorderNoteOffVector();


    return true;

}


// Gets note volume
// Applies track volume
// In the MIDI standard, a note velocity of 0 turns a note on event into a note
// off event, don't let that happen by using a max
// prevents value from being greater than 127 by using a min
int Track::computeNoteVolume(int noteId)
{
    return std::min(SCHAR_MAX, std::max(1, m_notes[noteId].getVolume() * m_volume / 100));
}
