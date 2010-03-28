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

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/ControllerEvent.h"

#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"

#include "Pickers/MagneticGrid.h"
#include "Pickers/InstrumentChoice.h"
#include "Pickers/DrumChoice.h"

#include "Editors/KeyboardEditor.h"
#include "Editors/GuitarEditor.h"
#include "Editors/RelativeXCoord.h"
#include "Editors/DrumEditor.h"
#include "Editors/ControllerEditor.h"

#include "IO/IOUtils.h"

#include <iostream>

#include "wx/utils.h"

using namespace AriaMaestosa;

// -------------------------------------------------------------------------------------------------------

Track::Track(MainFrame* parent, Sequence* sequence)
{
    // init key data
    setKey(0, NATURAL);
    
    m_name.set( wxString( _("Untitled") ) );
    m_name.setMaxWidth(120);
    
    //FIXME: find out why fonts are so different on mac and linux
    //FIXME: what does this do in the data class, and not in the graphics class?
#ifdef __WXMAC__
    m_name.setFont( wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
    m_name.setFont( wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#endif

    m_parent_frame = parent;

    Track::sequence = sequence;

    graphics = new GraphicalTrack(this, sequence);
    graphics->createEditors();

    m_channel = 0;
    if ( sequence->getChannelManagementType() == CHANNEL_MANUAL )
    {
        // if in manual channel management mode, we need to give it a proper channel
        // the following array will store what channels are currently taken
        bool channel_taken[16];
        for(int i=0; i<16; i++) channel_taken[i] = false;

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


    m_instrument = 0;
    m_drum_kit   = 0;

    instrument_name.set(Core::getInstrumentPicker()->getInstrumentName( m_instrument ));
#ifdef __WXMAC__
    instrument_name.setFont( wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
    instrument_name.setFont( wxFont(9,  wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#endif
    
}

// -------------------------------------------------------------------------------------------------------

Track::~Track()
{
}

// -------------------------------------------------------------------------------------------------------

void Track::notifyOthersIWillBeRemoved()
{
    // notify other tracks
    Sequence* seq = getCurrentSequence();
    const int trackAmount = seq->getTrackAmount();
    for(int n=0; n<trackAmount; n++)
    {
        Track* track = seq->getTrack(n);
        if (track != this) track->trackDeleted(this);
    }
}

// -------------------------------------------------------------------------------------------------------

void Track::trackDeleted(Track* track)
{
    graphics->keyboardEditor->trackDeleted(track);

    /*
     // uncomment if these editors get background support too
    graphics->guitarEditor->trackDelete(track);
    graphics->drumEditor->trackDelete(track);
    graphics->controllerEditor->trackDelete(track);
    graphics->scoreEditor->trackDelete(track);
    */
}

// -------------------------------------------------------------------------------------------------------

void Track::action( Action::SingleTrackAction* action)
{
    action->setParentTrack(this);
    sequence->addToUndoStack( action );
    action->perform();
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
    if (sequence->importing)
    {
        m_notes.push_back(note);
        m_note_off.push_back(note); // dont forget to reorder note off vector after importing
        return true;
    }

    bool noteAdded=false;

    //------------------------ place note on -----------------------
    // iterate through notes to place them in order
    const int noteAmount = m_notes.size();
    for(int n=0; n<noteAmount; n++)
    {
        // check for overlapping notes
        // the only time where this is not checked is when pasting, because it is then logical that notes are pasted on top of their originals
        if (check_for_overlapping_notes and m_notes[n].startTick == note->startTick and m_notes[n].pitchID == note->pitchID and
           (graphics->editorMode!=GUITAR or m_notes[n].getString()==note->getString()) /*in guitar mode string must also match to be considered overlapping*/ )
        {
            std::cout << "overlapping notes: rejected" << std::endl;
            return false;
        }

        if (m_notes[n].startTick > note->startTick)
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
    for(int n=0; n<noteOffAmount; n++)
    {
        if (m_note_off[n].endTick > note->endTick)
        {
            m_note_off.add(note, n);
            noteAdded = true;
            break;
        }//endif
    }//next

    if (!noteAdded)
    {
        // add note at the end, or if the track does not yet contain any note
        // (both cases in which the for loop either won't find a location for the note, or won't iterate at all)
        m_note_off.push_back(note);
    }


    return true;

}

// -------------------------------------------------------------------------------------------------------

// FIXME - find nicer solution than this pointer to an int...
void Track::addControlEvent( ControllerEvent* evt, int* previousValue )
{
    ptr_vector<ControllerEvent>* vector;

    if (previousValue != NULL) *previousValue = -1;

    // tempo events
    if (evt->getController()==201) vector = &sequence->tempoEvents;
    // controller and pitch bend events
    else vector = &m_control_events;

    // don't bother checking order if we're importing, we know its in time order and all
    // FIXME - what about 'addControlEvent_import' ??
    if (sequence->importing)
    {
        vector->push_back( evt );
        return;
    }

    ASSERT_E(evt->getController(),<,205);
    ASSERT_E(evt->getValue(),<,128);

    const int eventAmount=vector->size();
    for(int n=0; n<eventAmount; n++)
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

// -------------------------------------------------------------------------------------------------------

void Track::addControlEvent_import(const int x, const int value, const int controller)
{
    ASSERT(sequence->importing); // not to be used when not importing
    m_control_events.push_back(new ControllerEvent(sequence, controller, x, value) );
}

// -------------------------------------------------------------------------------------------------------

bool Track::addNote_import(const int pitchID, const int startTick, const int endTick, const int volume, const int string)
{
    ASSERT(sequence->importing); // not to be used when not importing
    return addNote( new Note(graphics, pitchID, startTick, endTick, volume, string) );
}

// -------------------------------------------------------------------------------------------------------

void Track::setNoteEnd_import(const int tick, const int noteID)
{
    ASSERT(sequence->importing); // not to be used when not importing
    ASSERT(noteID != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    ASSERT(noteID != SELECTED_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)

    ASSERT_E(noteID,<,m_notes.size());
    ASSERT_E(noteID,>=,0);

    m_notes[noteID].setEnd(tick);
}

// -------------------------------------------------------------------------------------------------------

void Track::removeNote(const int id)
{

    // also delete corresponding note off event
    const int namount = m_note_off.size();
    for(int i=0; i<namount; i++)
    {
        if (&m_note_off[i] == &m_notes[id])
        {
            m_note_off.remove(i);
            break;
        }
    }

    m_notes.erase(id);

}

// -------------------------------------------------------------------------------------------------------

void Track::markNoteToBeRemoved(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    // also delete corresponding note off event
    const int namount = m_note_off.size();
    Note* note = m_notes.get(id);

    for(int i=0; i<namount; i++)
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

// -------------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------------

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

    for(int n=0; n<noteAmount-1; n++)
    {

        ASSERT_E(n+1,<,m_notes.size());

        if (m_notes[n].startTick > m_notes[n+1].startTick)
        {
            m_notes.swap(n, n+1);
            if (n>2) n-= 2;
            else n=-1;
        }
    }//next

}

// -------------------------------------------------------------------------------------------------------

void Track::reorderNoteOffVector()
{
    // FIXME - innefficient implementation
    const int noteAmount = m_note_off.size();
#ifdef _MORE_DEBUG_CHECKS
    if (m_notes.size() != m_note_off.size()) std::cout << "WARNING note on and off events differ in amount" << std::endl;
#endif

    for(int n=0; n<noteAmount-1; n++)
    {

        ASSERT_E(n+1,<,m_note_off.size());
        ASSERT(m_note_off.get(n) != NULL);
        ASSERT(m_note_off.get(n+1) != NULL);
        ASSERT(m_note_off.get(n) != 0);
        ASSERT(m_note_off.get(n+1) != 0);

        if (m_note_off[n].endTick > m_note_off[n+1].endTick)
        {
            m_note_off.swap(n, n+1);
            if (n>2) n-= 2;
            else n=-1;
        }//end if
    }//next

}

// -------------------------------------------------------------------------------------------------------

void Track::reorderControlVector()
{
    // FIXME - innefficient implementation
    // FIXME - if bugs in controller editor are fixed that method shouldn't even be necessary
    const int ctrlAmount = m_control_events.size();

    for(int n=0; n<ctrlAmount-1; n++)
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

// -------------------------------------------------------------------------------------------------------

void Track::mergeTrackIn(Track* track)
{
    const int noteAmount = track->m_notes.size();
    for(int n=0; n<noteAmount; n++)
    {
        Note* a = new Note(track->m_notes[n]);
        addNote(a, false);
    }

    const int controllerAmount = track->m_control_events.size();
    for(int n=0; n<controllerAmount; n++)
    {
        addControlEvent( new ControllerEvent(sequence, track->m_control_events[n].getController(),
                                             track->m_control_events[n].getTick(),
                                             track->m_control_events[n].getValue()) );

    }

}

// -------------------------------------------------------------------------------------------------------

// FIXME - debug function, remove
void Track::checkControlEventsOrder()
{
    int ptick = -1;
    for(int n=0; n<m_control_events.size(); n++)
    {
        if (ptick != -1 and m_control_events[n].getTick() < ptick)
        {
            std::cout << "\n\n*** Error: control events in wrong order " << ptick << " then " << m_control_events[n].getTick() << std::endl;
        }
        ptick = m_control_events[n].getTick();
    }
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

// -------------------------------------------------------------------------------------------------------

int Track::getNoteStartInPixels(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return (int)round( m_notes[id].startTick * sequence->getZoom() );
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteEndInPixels(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return (int)round( m_notes[id].endTick * sequence->getZoom() );
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteStartInMidiTicks(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].startTick;
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteEndInMidiTicks(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    return m_notes[id].endTick;
}

// -------------------------------------------------------------------------------------------------------

int Track::getNotePitchID(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].pitchID;
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteAmount() const
{
    return m_notes.size();
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteVolume(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].volume;
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteString(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    //if (m_notes[id].tuning == NULL) m_notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning
    if (m_notes[id].getString() == -1) m_notes[id].findStringAndFretFromNote();

    return m_notes[id].getString();
}


// -------------------------------------------------------------------------------------------------------

int Track::getNoteFret(const int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());

    //if (m_notes[id].tuning == NULL) m_notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning
    
    //FIXME: I think Note::getFret also does this check, so we're duplicating it here...
    if (m_notes[id].getFret() == -1) m_notes[id].findStringAndFretFromNote();

    return m_notes[id].getFret();
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteStringConst(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    
    return m_notes[id].getStringConst();
}

// -------------------------------------------------------------------------------------------------------

int Track::getNoteFretConst(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    
    return m_notes[id].getFretConst();
}


// -------------------------------------------------------------------------------------------------------
    
int Track::findFirstNoteInRange(const int fromTick, const int toTick) const
{
    const int noteAmount = m_notes.size();
    
    for (int n=0; n<noteAmount; n++)
    {
        if (m_notes[n].startTick >= fromTick && m_notes[n].startTick < toTick) return n;
        if (m_notes[n].startTick >= toTick) return -1;
    }
    return -1;
}
    
// -------------------------------------------------------------------------------------------------------

int Track::findLastNoteInRange(const int fromTick, const int toTick) const
{
    const int noteAmount = m_notes.size();
    
    for (int n=noteAmount-1; n>-1; n--)
    {
        if (m_notes[n].startTick >= fromTick && m_notes[n].startTick < toTick) return n;
        if (m_notes[n].startTick < fromTick) return -1;
    }
    return -1;
}

// -------------------------------------------------------------------------------------------------------

int Track::getControllerEventAmount(const int controllerTypeID) const
{
    if (controllerTypeID==201 /*tempo*/) return sequence->tempoEvents.size();
    return m_control_events.size();
}

// -------------------------------------------------------------------------------------------------------

ControllerEvent* Track::getControllerEvent(const int id, const int controllerTypeID)
{
    ASSERT_E(id,>=,0);
    if (controllerTypeID==201 /*tempo*/){
        ASSERT_E(id,<,sequence->tempoEvents.size());
    } else {
        ASSERT_E(id,<,m_control_events.size());
    }

    if (controllerTypeID==201 /*tempo*/) return &sequence->tempoEvents[id];
    return &m_control_events[id];
}

// -------------------------------------------------------------------------------------------------------

int Track::getFirstNoteTick(bool selectionOnly) const
{

    if (!selectionOnly) return m_notes[0].startTick;

    const int noteAmount = m_notes.size();
    int tick = -1;

    for(int n=0; n<noteAmount; n++)
    {
        if ( m_notes[n].isSelected() ) return m_notes[n].startTick;
    }//next

    return tick;

}

// -------------------------------------------------------------------------------------------------------

void Track::selectNote(const int id, const bool selected, bool ignoreModifiers)
{

    ASSERT(id != SELECTED_NOTES); // not supported in this function


    if (!Display::isSelectMorePressed() and !Display:: isSelectLessPressed()) ignoreModifiers=true; // if no modifier is pressed, don't do any special checks

    // ---- select/deselect all notes
    if (id == ALL_NOTES)
    {
        // if this is a 'select none' command, unselect any selected measures in the top bar
        if (selected == FALSE) getMeasureData()->unselect();

        if (graphics->editorMode == CONTROLLER)
        { // controller editor must be handled differently
            graphics->controllerEditor->selectAll( selected );
        }
        else
        {

            if (ignoreModifiers)
            {
                for (int n=0; n<m_notes.size(); n++)
                {
                    m_notes[n].setSelected(selected);
                }//next
            }//end if

        }// end if

    }
    else  // ---- select/deselect one specific note
    {

        ASSERT_E(id,>=,0);
        ASSERT_E(id,<,m_notes.size());

        // if we ignore +/- key modifiers, just set the value right away
        if (ignoreModifiers) m_notes[id].setSelected(selected);
        else
        { // otherwise, check key modifiers and set value accordingly
            if (selected)
            {
                if (Display::isSelectMorePressed()) m_notes[id].setSelected(true);
                else if (Display:: isSelectLessPressed()) m_notes[id].setSelected( !selected );
            }
        }//end if

    }//end if
}

// -------------------------------------------------------------------------------------------------------

bool Track::isNoteSelected(const int id) const
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,m_notes.size());
    return m_notes[id].isSelected();
}

// -------------------------------------------------------------------------------------------------------

void Track::prepareNotesForGuitarEditor()
{

    for(int n=0; n<m_notes.size(); n++)
    {
        m_notes[n].checkIfStringAndFretMatchNote(true);
    }

}

// -------------------------------------------------------------------------------------------------------

void Track::copy()
{
    
    if (graphics->editorMode == CONTROLLER)
    {
        wxBell();
        return; // no copy/paste in controller mode
    }
    
    Clipboard::clear();
    Clipboard::setBeatLength(sequence->ticksPerBeat());
    
    int tickOfFirstSelectedNote=-1;
    // place all selected notes into clipboard
    for (int n=0; n<m_notes.size(); n++)
    {
        if (!m_notes[n].isSelected()) continue;
        
        Note* tmp=new Note(m_notes[n]);
        Clipboard::add(tmp);
        
        // if in guitar mode, make sure string/fret and note match
        if (graphics->editorMode == GUITAR) Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(false);
        else Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(true);
        
        // find tickOfFirstSelectedNote of the first note
        if (m_notes[n].startTick < tickOfFirstSelectedNote or tickOfFirstSelectedNote==-1)
        {
            tickOfFirstSelectedNote=m_notes[n].startTick;
        }
        
    }//next
    
    // remove all empty measures before notes, so that they appear in the current measure when pasting
    const int lastMeasureStart = getMeasureData()->firstTickInMeasure( getMeasureData()->measureAtTick(tickOfFirstSelectedNote) );
    
    const int clipboard_size = Clipboard::getSize();
    for(int n=0; n<clipboard_size; n++)
    {
        //Clipboard::getNote(n)->move( -lastMeasureStart, 0, graphics->editorMode);
        graphics->getCurrentEditor()->moveNote(*Clipboard::getNote(n), -lastMeasureStart, 0);
    }
    
    sequence->notes_shift_when_no_scrolling = lastMeasureStart;
}


// =======================================================================================================
// ============================================ Get/Set ==================================================
// =======================================================================================================

#if 0
#pragma mark -
#pragma mark Get/Set Other Stuff
#endif

// -------------------------------------------------------------------------------------------------------

void Track::setId(const int id)
{
    m_track_id = id;
}

// -------------------------------------------------------------------------------------------------------

void Track::setName(wxString name)
{
    if (name.Trim().IsEmpty()) m_name.set( wxString( _("Untitled") ) );
    else                       m_name.set(name);
}

// -------------------------------------------------------------------------------------------------------

AriaRenderString& Track::getNameRenderer()
{
    return m_name;
}

// -------------------------------------------------------------------------------------------------------

int Track::getGridDivider()
{
    return graphics->grid->divider;
}

// -------------------------------------------------------------------------------------------------------

int Track::getChannel()
{
    // always 9 for drum tracks
    //const bool manual_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;

    if (graphics->editorMode == DRUM) return 9;
    else                              return m_channel;

}

// -------------------------------------------------------------------------------------------------------

void Track::setChannel(int i)
{
    m_channel = i;
}


// -------------------------------------------------------------------------------------------------------

void Track::setInstrument(int i, bool recursive)
{
    m_instrument = i;
    instrument_name.set(Core::getInstrumentPicker()->getInstrumentName( m_instrument ));
    
    // if we're in manual channel management mode, change all tracks of the same channel
    // to have the same instrument
    if (sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive)
    {
        const int trackAmount = sequence->getTrackAmount();
        for(int n=0; n<trackAmount; n++)
        {
            Track* track = sequence->getTrack(n);
            if (track == this) continue; // track must not evaluate itself...
            if ( track->getChannel() == m_channel )
            {
                track->setInstrument(i,true);
            }
        }//next
    }//endif
}

// -------------------------------------------------------------------------------------------------------

int Track::getInstrument()
{
    return m_instrument;
}

// -------------------------------------------------------------------------------------------------------

void Track::setDrumKit(int i, bool recursive)
{

    // check id validity
    if      (i == 0 )  ; // Standard
    else if (i == 8 )  ; // Room kit
    else if (i == 16) ; // Power kit
    else if (i == 24) ; // Electronic
    else if (i == 25) ; // Analog
    else if (i == 32) ; // Jazz
    else if (i == 40) ; // Brush
    else if (i == 48) ; // Orchestral
    else if (i == 56) ; // Special Effects
    else return;         // invalid drum kit ID

    m_drum_kit = i;

    // if we're in manual channel management mode, change all tracks of the same channel to have the same instrument
    if (sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive)
    {

        const int trackAmount = sequence->getTrackAmount();
        for(int n=0; n<trackAmount; n++)
        {
            Track* track = sequence->getTrack(n);
            if (track == this) continue; // track must not evaluate itself...
            track->setDrumKit(i,true);
        }//next
    }//endif
}

// -------------------------------------------------------------------------------------------------------

int Track::getDuration() const
{
    if (m_note_off.size() < 1) return 0;
    
    return m_note_off[m_note_off.size()-1].endTick;
}

// -------------------------------------------------------------------------------------------------------

void Track::setKey(const int symbolAmount, const PitchSign symbol)
{
    // ---- update "key_sharps_amnt" and "key_flats_amnt" members
    if (symbol == SHARP)
    {
        m_key_sharps_amnt = symbolAmount;
        m_key_flats_amnt  = 0;
    }
    else if (symbol == FLAT)
    {
        m_key_sharps_amnt = 0;
        m_key_flats_amnt  = symbolAmount;
    }
    else if (symbol == NATURAL and symbolAmount == 0)
    {
        m_key_sharps_amnt = 0;
        m_key_flats_amnt  = 0;
    }
    else
    {
        std::cerr << "Bogus call to Track::setKey! Symbol must be SHARP or FLAT\n";
        ASSERT(false);
    }
    
    // ---- update 'm_key_notes' array
    // if key is e.g. G Major, "major_note" will be set to note12 equivalent of G.
    // to load a minor key, it's just set to the major one that has same sharps and flats
    // to ease the process
    Note12 major_note12 = NOTE_12_C;
    
    if (symbolAmount == 0 or symbol == NATURAL)
    {
        major_note12 = NOTE_12_C;
    }
    else if (symbol == SHARP)
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
    else if (symbol == FLAT)
    {
        switch(symbolAmount)
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
    
    Note12 noteName;
    int octave;
    
    for (int n=0; n<131; n++)
    {
        if (Editor::findNoteName(n, &noteName, &octave))
        {
            m_key_notes[n] = not note_12_greyed_out[noteName];
        }
        else
        {
            m_key_notes[n] = false;
        }
    }

    // ---- notify graphical track (and editors)
    //FIXME: do not call this while editors are still null
    if (graphics != NULL) graphics->onKeyChange(symbolAmount, symbol);
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

// -------------------------------------------------------------------------------------------------------

/** @return Smallest values, ignoring -1, a being prioritary to b (returns -1 for none, 0 for a, 1 for b) */
int getActiveMin(int a, int b)
{
    if (a==-1 and b==-1) return -1;
    if (a==-1) return 1;//b
    if (b==-1) return 0;//a

    if (a <= b) return 0;//a
    else return 1;//b
}

// -------------------------------------------------------------------------------------------------------

/**
  * @return Smallest values, -1 being considered as 'no value'
  *         if some are equal, 'a' is prioritary to 'b', and 'b' prioritary to 'c'
  *         (returns -1 for none, 0 for a, 1 for b, 2 for c)
  */
int getActiveMin(int a, int b, int c)
{

    const int result1 = getActiveMin(b,c);

    int passnumber;
    if (result1==0) passnumber = b;
    else if (result1==1) passnumber = c;
    else if (result1==-1) passnumber =-1;

    const int result2 = getActiveMin( a, passnumber );

    if (result2==0) return 0;
    else
    {
        if (result1==-1) return -1;
        return result1+1;
    }

}

// -------------------------------------------------------------------------------------------------------

int Track::addMidiEvents(jdkmidi::MIDITrack* midiTrack,
                         int track_ID, /* in manual channel mode, this argument is NOT considered */
                         int firstMeasure,
                         bool selectionOnly,
                         int& startTick)
{

    // ignore track if it's muted
    // (but for some reason drum track can't be completely omitted)
    // if we only play selection, ignore mute and play anyway
    if (graphics->muted and graphics->editorMode != DRUM and !selectionOnly)
        return -1;

    // if in manual mode, use the user-specified channel ID and not the stock one
    const bool manual_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;
    if (manual_mode) track_ID = getChannel();

    // drum tracks
    if (graphics->editorMode == DRUM) track_ID = 9;

    //std::cout << "channel = " << track_ID << std::endl;

    // when previewing selected notes (start by finding the note that plays first, to start playing at
    // the right place and note from the beginning)
    int firstNoteStartTick = -1;
    int selectedNoteAmount=0;

    if (selectionOnly)
    {

        const int noteAmount = m_notes.size();
        for (int n=0; n<noteAmount; n++)
        {
            if (m_notes[n].isSelected())
            {
                if (m_notes[n].startTick < firstNoteStartTick or firstNoteStartTick==-1)
                {
                    firstNoteStartTick = m_notes[n].startTick;
                }
                selectedNoteAmount++;
            }
        }

        if (firstNoteStartTick == -1) return -1; // error, no note was found.
        if (selectedNoteAmount == 0) return -1; // error, no note was found.

    }
    else
    {
        firstNoteStartTick = getMeasureData()->firstTickInMeasure(firstMeasure);
        startTick = firstNoteStartTick;
    }

    // set bank
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 0, 0 );

        if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; ASSERT(0); }

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 32, 0 );

        if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; ASSERT(0); }
    }

    // set instrument
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );

        if (graphics->editorMode == DRUM) m.SetProgramChange( track_ID, m_drum_kit );
        else                              m.SetProgramChange( track_ID, m_instrument );

        if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding instrument at track beginning!" << std::endl; }
    }

    // set track name
    {

        jdkmidi::MIDITimedBigMessage m;
        m.SetText( 3 );
        m.SetByte1( 3 );

        // FIXME - I removed strcpy, but not sure it works anymore...
        jdkmidi::MIDISystemExclusive sysex( (unsigned char*)(const char*)m_name.mb_str(wxConvUTF8),
                                           m_name.size()+1, m_name.size()+1, false);

        m.CopySysEx( &sysex );
        m.SetTime( 0 );
        if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; ASSERT(0); }
    }

    // set maximum volume
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 7, 127 );

        if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; ASSERT(false); }
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
    if (graphics->muted and graphics->editorMode == DRUM and !selectionOnly) return -1;

    //std::cout << "-------------------- TRACK -------------" << std::endl;

    while(true)
    {

        // if we only want to play what's selected, skip unselected notes
        if (selectionOnly)
        {
            while(note_on_id < noteOnAmount   and !m_notes[note_on_id].isSelected())     note_on_id++;
            while(note_off_id < noteOffAmount and !m_note_off[note_off_id].isSelected()) note_off_id++;
        }

        const int tick_on      = (note_on_id < noteOnAmount)     ?   m_notes[note_on_id].startTick - firstNoteStartTick      :  -1;
        const int tick_off     = (note_off_id < noteOffAmount)   ?   m_note_off[note_off_id].endTick - firstNoteStartTick     :  -1;

        // ignore control events when only playing selection
        const int tick_control = (control_evt_id < controllerAmount and !selectionOnly)  ?   m_control_events[control_evt_id].getTick() - firstNoteStartTick : -1;

        const int activeMin = getActiveMin( tick_off, tick_control, tick_on );

        if (activeMin==-1) break; // all events have been added

        jdkmidi::MIDITimedBigMessage m;

        //  ------------------------ add note on event ------------------------
        if (activeMin == 2)
        {

            const int time = m_notes[note_on_id].startTick - firstNoteStartTick;
            if (!(time < 0))
            {

                m.SetTime( time );


                if (graphics->editorMode == DRUM) m.SetNoteOn( track_ID, m_notes[note_on_id].pitchID, m_notes[note_on_id].volume );
                else m.SetNoteOn( track_ID, 131-m_notes[note_on_id].pitchID, m_notes[note_on_id].volume );

                // find track end
                if (m_notes[note_on_id].endTick > last_event_tick) last_event_tick = m_notes[note_on_id].endTick;

                if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
            }

            note_on_id++;
        }
        //  ------------------------ add note off event ------------------------
        else if (activeMin == 0)
        {

            const int time=m_note_off[note_off_id].endTick - firstNoteStartTick;
            if (!(time < 0))
            {

                m.SetTime( time );

                if (graphics->editorMode == DRUM) m.SetNoteOff( track_ID, m_note_off[note_off_id].pitchID, 0 );
                else m.SetNoteOff( track_ID, 131-m_note_off[note_off_id].pitchID, 0 );

                // find track end
                if (time > last_event_tick) last_event_tick = time;

                if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
            }
            note_off_id++;
        }
        //  ------------------------ add control change event ------------------------
        else if (activeMin == 1)
        {
            const int controllerID = m_control_events[control_evt_id].getController();

            // pitch bend
            if (controllerID==200)
            {


                int time=m_control_events[control_evt_id].getTick() - firstNoteStartTick;

                // controller changes happens before the area we play
                // but perhaps it still is affecting the area we want to play - check for that.
                bool addControlEvent = true;
                if (time < 0)
                {
                    if (control_evt_id+1 < controllerAmount)
                    {
                        addControlEvent = false;
                        int checkEventID = control_evt_id+1;
                        // check if there are other controller events of the same type before the area we play.
                        while(true)
                        {

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)<1  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // the current event has no effect, there is another one later, disregard it.
                                addControlEvent = false;
                                break;
                            }

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)>0  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // there is another event, but it's later so current event is still relevant.
                                addControlEvent = true;
                                time = 0;
                                break;
                            }

                            checkEventID++;
                            if ( ! (checkEventID < controllerAmount ) )
                            {
                                // we reaached the end, there are no other events of the same type
                                // this one still affects playback of the area that we're playing.
                                addControlEvent = true;
                                time = 0;
                                break;
                            }
                        }//wend
                    }
                    else
                    {
                        // there are no other events in track. add it.
                        addControlEvent = true;
                        time = 0;
                    }
                }
                else
                {
                    addControlEvent = true;
                }

                if (addControlEvent)
                {

                    m.SetTime( time );

                    // FIXME
                    // ((value * 127) - 8191) ?
                    int pitchBendVal = (127-m_control_events[control_evt_id].getValue())*128-128*128;

                    if (pitchBendVal > 0) pitchBendVal-=8192;
                    else if (pitchBendVal < 0) pitchBendVal +=8192;

                    m.SetPitchBend( track_ID,
                                    pitchBendVal );

                    if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
                }
                control_evt_id++;

            }
            // other controller
            else
            {


                int time=m_control_events[control_evt_id].getTick() - firstNoteStartTick;

                // controller changes happens before the area we play
                // but perhaps it still is affecting the area we want to play - check for that.
                bool addControlEvent = true;
                if (time < 0)
                {
                    if (control_evt_id+1 < controllerAmount)
                    {
                        addControlEvent = false;
                        int checkEventID = control_evt_id+1;
                        // check if there are other controller events of the same type before the area we play.
                        while(true)
                        {

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)<1  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // the current event has no effect, there is another one later, disregard it.
                                addControlEvent = false;
                                break;
                            }

                            if ((m_control_events[checkEventID].getTick() - firstNoteStartTick)>0  and
                               m_control_events[checkEventID].getController() == controllerID)
                            {
                                // there is another event, but it's later so current event is still relevant.
                                addControlEvent = true;
                                time = 0;
                                break;
                            }

                            checkEventID++;
                            if ( ! (checkEventID < controllerAmount ) )
                            {
                                // we reaached the end, there are no other events of the same type
                                // this one still affects playback of the area that we're playing.
                                addControlEvent = true;
                                time = 0;
                                break;
                            }
                        }//wend
                    }
                    else
                    {
                        // there are no other events in track. add it.
                        addControlEvent = true;
                        time = 0;
                    }
                }
                else
                {
                    addControlEvent = true;
                }

                if (addControlEvent)
                {
                    m.SetTime( time );

                    m.SetControlChange( track_ID,
                                        controllerID,
                                        127-m_control_events[control_evt_id].getValue() );

                    if ( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
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

    writeData(wxT("\n<track name=\"") + m_name +
              wxT("\" channel=\"") + to_wxString(m_channel) +
              wxT("\">\n"), fileout );

    writeData(wxT("<key sharps=\"") + to_wxString( getKeySharpsAmount() ) +
              wxT("\" flats=\"")    + to_wxString( getKeyFlatsAmount () ) +
              wxT("\"/>\n"), fileout);
    
    graphics->saveToFile(fileout);

    // notes
    for (int n=0; n<m_notes.size(); n++)
    {
        m_notes[n].saveToFile(fileout);
    }
    
    // controller changes
    for (int n=0; n<m_control_events.size(); n++)
    {
        m_control_events[n].saveToFile(fileout);
    }
    
    writeData(wxT("</track>\n\n"), fileout );


}

// -------------------------------------------------------------------------------------------------------

bool Track::readFromFile(irr::io::IrrXMLReader* xml)
{

    m_notes.clearAndDeleteAll();
    m_note_off.clearWithoutDeleting(); // have already been deleted by previous command
    m_control_events.clearAndDeleteAll();

    // parse XML file
    do
    {

        switch(xml->getNodeType())
        {
            case irr::io::EXN_TEXT:
                break;
            case irr::io::EXN_ELEMENT:
            {
                if (!strcmp("track", xml->getNodeName()))
                {

                    const char* name = xml->getAttributeValue("name");
                    if (name!=NULL)
                    {
                        setName( fromCString((char*)name) );
                    }
                    else
                    {
                        std::cout << "Missing info from file: track name" << std::endl;
                        setName( wxString(_("Untitled")) );
                    }
                    const char* channel_c = xml->getAttributeValue("channel");
                    if (channel_c!=NULL)
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
                        std::cout << "Missing info from file: track channel" << std::endl;
                    }

                }
                else if (!strcmp("editor", xml->getNodeName()))
                {
                    if (! graphics->readFromFile(xml) )
                        return false;
                }
                else if (!strcmp("magneticgrid", xml->getNodeName()))
                {
                    if (! graphics->readFromFile(xml) )
                        return false;
                }
                else if (!strcmp("instrument", xml->getNodeName()))
                {
                    const char* id = xml->getAttributeValue("id");
                    if (id != NULL)
                    {
                        setInstrument( atoi(id) );
                    }
                    else
                    {
                        std::cout << "Missing info from file: instrument ID" << std::endl;
                        m_instrument = 0;
                    }
                }
                else if (!strcmp("drumkit", xml->getNodeName()))
                {
                    const char* id = xml->getAttributeValue("id");
                    if (id != NULL) setDrumKit( atoi(id) );
                    else
                    {
                        std::cout << "Missing info from file: drum ID" << std::endl;
                    }
                }
                else if (!strcmp("guitartuning", xml->getNodeName()))
                {
                    if (! graphics->readFromFile(xml) )
                        return false;
                }
                else if (!strcmp("key", xml->getNodeName()))
                {
                    //std::cout << "Found 'key'!" << std::endl;
                    char* flats_c  = (char*)xml->getAttributeValue("flats");
                    char* sharps_c = (char*)xml->getAttributeValue("sharps");
                    
                    int sharps = 0, flats = 0;
                    if (flats_c != NULL or sharps_c != NULL)
                    {
                        if (flats_c != NULL)  flats = atoi(flats_c);
                        if (sharps_c != NULL) sharps = atoi(sharps_c);
                        //std::cout << "sharps = " << sharps << " flats = " << flats << std::endl;
                        
                        if (sharps > flats) setKey(sharps, SHARP);
                        else                setKey(flats, FLAT);
                    }
                    
                }
                else if (!strcmp("note", xml->getNodeName()))
                {
                    Note* temp = new Note(graphics);
                    if (! temp->readFromFile(xml) )
                    {
                        std::cout << "A note was discarded because it is invalid" << std::endl;
                        delete temp;
                    }
                    else
                    {
                        addNote( temp );
                    }
                }
                else if (!strcmp("controlevent", xml->getNodeName()))
                {

                    ControllerEvent* temp = new ControllerEvent(sequence, 0, 0, 0);
                    if ( !temp->readFromFile(xml) )
                    {
                        // thre was an error when trying to load control event
                        std::cout << "A controller event was discarded because it is invalid" << std::endl;
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

                if (!strcmp("track", xml->getNodeName()))
                {
                    reorderNoteOffVector();
                    return true;
                }
            }
                break;

            default:break;
        }//end switch

    } while(xml && xml->read());

    reorderNoteOffVector();

    return true;

}

