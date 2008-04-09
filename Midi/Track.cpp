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

#include "Config.h"
#include "Clipboard.h"
#include "AriaCore.h"

#include "Actions/EditAction.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/ControllerEvent.h"

#include "GUI/GraphicalTrack.h"
#include "GUI/MeasureBar.h"

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

namespace AriaMaestosa
{

	Track::Track(MainFrame* parent, Sequence* sequence)
    {
		INIT_LEAK_CHECK();

		name = wxString( _("Untitled") );

		Track::frame=parent;

		Track::sequence = sequence;
		graphics = new GraphicalTrack(this, sequence);
		graphics->createEditors();
		channel = 0;

		instrument = 0;
		drumKit = 0;
    }

/*
 * This is the method called for performing any action that can be undone.
 * A EditAction object is used to describe the task, and it also knows how to revert it.
 * The EditAction objects are kept in a stack in Sequence in order to offer multiple undo levels.
 *
 * Track::action does actions that affect only this Track. Also see Sequence::action.
 */

void Track::action( Action::SingleTrackAction* action)
{
	action->setParentTrack(this);
	sequence->addToUndoStack( action );
	action->perform();
}

Track::~Track()
{
    // notify other tracks
    Sequence* seq = getCurrentSequence();
    const int trackAmount = seq->getTrackAmount();
    for(int n=0; n<trackAmount; n++)
    {
        Track* track = seq->getTrack(n);
        if(track != this) track->trackDeleted(this);
    }

    delete graphics;
    notes.clearAndDeleteAll();
	noteOff.clearWithoutDeleting();
    controlEvents.clearAndDeleteAll();
}

// when one track will be removed, all others are notified so they can remove any
// link to that track they could have (like background)
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

// ------------------- add/remove notes --------------------
#pragma mark -


bool Track::addNote(Note* note, bool check_for_overlapping_notes)
{
	// if we're importing, just push it to the end, we know they're in time order
	if(sequence->importing)
	{
		notes.push_back(note);
		noteOff.push_back(note); // dont forget to reorder note off vector after importing
		return true;
	}

	bool noteAdded=false;

	//------------------------ place note on -----------------------
    // iterate through notes to place them in order
    const int noteAmount = notes.size();
    for(int n=0; n<noteAmount; n++)
	{
		// check for overlapping notes
		// the only time where this is not checked is when pasting, because it is then logical that notes are pasted on top of their originals
		if(check_for_overlapping_notes and notes[n].startTick == note->startTick and notes[n].pitchID == note->pitchID and
		   (graphics->editorMode!=GUITAR or notes[n].getString()==note->getString()) /*in guitar mode string must also match to be considered overlapping*/ )
		{
			std::cout << "overlapping notes: rejected" << std::endl;
			return false;
		}

        if(notes[n].startTick > note->startTick)
		{

            notes.add(note, n);
			noteAdded=true;
            break;
        }//endif


    }//next

    if(!noteAdded)
	{
		// add note at the end, or if the track does not yet contain any note
		// (both cases in which the for loop either won't find a location for the note, or won't iterate at all)
		notes.push_back(note);
	}

	//------------------------ place note off -----------------------
    // iterate through notes to place them in order

	noteAdded = false;

    const int noteOffAmount = noteOff.size();
    for(int n=0; n<noteOffAmount; n++)
	{
        if(noteOff[n].endTick > note->endTick)
		{
            noteOff.add(note, n);
			noteAdded = true;
            break;
        }//endif
    }//next

	if(!noteAdded)
	{
		// add note at the end, or if the track does not yet contain any note
		// (both cases in which the for loop either won't find a location for the note, or won't iterate at all)
		noteOff.push_back(note);
	}


    return true;

}

// FIXME - find nicer solution than this pointer to an int...
// here previousValue returns the old value there was, if any, before this new event replaces it.
void Track::addControlEvent( ControllerEvent* evt, int* previousValue )
{
	ptr_vector<ControllerEvent>* vector;

	if(previousValue != NULL) *previousValue = -1;

	// tempo events
	if(evt->getController()==201) vector = &sequence->tempoEvents;
	// controller and pitch bend events
	else vector = &controlEvents;

	// don't bother checking order if we're importing, we know its in time order and all
	// FIXME - what about 'addController_import' ??
	if(sequence->importing)
	{
		vector->push_back( evt );
		return;
	}

	assertExpr(evt->getController(),<,205);
	assertExpr(evt->getValue(),<,128);

	const int eventAmount=vector->size();
	for(int n=0; n<eventAmount; n++)
	{
		if( (*vector)[n].getTick() == evt->getTick())
		{
			// if there is already an even of same type at same time, remove it first
			if((*vector)[n].getController() == evt->getController() )
			{
				if(previousValue != NULL) *previousValue = (*vector)[n].getValue();
				vector->erase(n);
			}
			vector->add( evt, n );
			return;
		}
		else if( (*vector)[n].getTick() > evt->getTick() )
		{
			vector->add( evt, n );
			return;
		}//endif

	}//next

	vector->push_back( evt );
}

/*
 * A midi controller, added from reading a file.
 * If we're reading the even from file, we can add it right away without further checks
 * because we know events won't overlap and are in time order.
 */

void Track::addController_import(const int x, const int value, const int controller)
{
    controlEvents.push_back(new ControllerEvent(sequence, controller, x, value) );
}

bool Track::addNote_import(const int pitchID, const int startTick, const int endTick, const int volume, const int string)
{
	return addNote( new Note(graphics, pitchID, startTick, endTick, volume, string) );
	//return addNote(pitchID, startTick, endTick, volume, string);
}

void Track::setNoteEnd_import(const int tick, const int noteID)
{

    assert(noteID != ALL_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)
    assert(noteID != SELECTED_NOTES); // not supported in this function (mostly bacause not needed, but could logically be implmented)

    assertExpr(noteID,<,notes.size());
    assertExpr(noteID,>=,0);

    notes[noteID].setEnd(tick);

	if(!sequence->importing) reorderNoteOffVector();
}

void Track::removeNote(const int id)
{

	// also delete corresponding note off event
	const int namount = noteOff.size();
	for(int i=0; i<namount; i++)
	{
		if(&noteOff[i] == &notes[id])
		{
			noteOff.remove(i);
			break;
		}
	}

	notes.erase(id);

}

void Track::markNoteToBeRemoved(const int id)
{
	assertExpr(id,>=,0);
	assertExpr(id,<,notes.size());

	// also delete corresponding note off event
	const int namount = noteOff.size();
	Note* note = notes.get(id);

	for(int i=0; i<namount; i++)
	{
		if(noteOff.get(i) == note)
		{
			noteOff.markToBeRemoved(i);
			break;
		}
#ifdef _MORE_DEBUG_CHECKS
		if(i+1 == namount) std::cout << "WARNING could not find note off event corresponding to note on event" << std::endl;
#endif
	}

	notes.markToBeRemoved(id);
}
void Track::removeMarkedNotes()
{

	//std::cout << "removing marked" << std::endl;

	notes.removeMarked();
	noteOff.removeMarked();

#ifdef _MORE_DEBUG_CHECKS
	if(notes.size() != noteOff.size())
	{
		std::cout << "WARNING note on and off events differ in amount in Track::removeMarkedNotes()" << std::endl;
		std::cout << notes.size() << " notes and " << noteOff.size() << " note offs" << std::endl;
	}
#endif
}

/*
 * Make sure all notes in note on vector are in time order.
 */
void Track::reorderNoteVector()
{
	// FIXME - innefficient implementation
	const int noteAmount = notes.size();

#ifdef _MORE_DEBUG_CHECKS
	if(notes.size() != noteOff.size())
	{
		std::cout << "WARNING note on and off events differ in amount" << std::endl;
		std::cout << notes.size() << " notes and " << noteOff.size() << " note offs" << std::endl;
	}
#endif

	for(int n=0; n<noteAmount-1; n++)
	{

		assertExpr(n+1,<,notes.size());

		if(notes[n].startTick > notes[n+1].startTick)
		{
			notes.swap(n, n+1);
			if(n>2) n-= 2;
			else n=0;
		}
	}//next

}

/*
 * Make sure all notes in note off vector are in time order.
 */
void Track::reorderNoteOffVector()
{
	// FIXME - innefficient implementation
	const int noteAmount = noteOff.size();
#ifdef _MORE_DEBUG_CHECKS
	if(notes.size() != noteOff.size()) std::cout << "WARNING note on and off events differ in amount" << std::endl;
#endif

	for(int n=0; n<noteAmount-1; n++)
	{

		assertExpr(n+1,<,noteOff.size());
		assert(noteOff.get(n) != NULL);
		assert(noteOff.get(n+1) != NULL);
		assert(noteOff.get(n) != 0);
		assert(noteOff.get(n+1) != 0);

		if(noteOff[n].endTick > noteOff[n+1].endTick)
		{
			noteOff.swap(n, n+1);
			if(n>2) n-= 2;
			else n=0;
		}//end if
	}//next

}

void Track::reorderControlVector()
{
	// FIXME - innefficient implementation
    // FIXME - if bugs in controller editor are fixed that method shouldn't even be necessary
	const int ctrlAmount = controlEvents.size();

	for(int n=0; n<ctrlAmount-1; n++)
	{

		assertExpr(n+1,<,ctrlAmount);

		if(controlEvents[n].getTick() > controlEvents[n+1].getTick())
		{
			controlEvents.swap(n, n+1);
			if(n>2) n-= 2;
			else n=0;
		}
	}//next

}

void Track::mergeTrackIn(Track* track)
{
	const int noteAmount = track->notes.size();
	for(int n=0; n<noteAmount; n++)
	{
		Note* a = new Note(track->notes[n]);
		addNote(a, false);
	}

	const int controllerAmount = track->controlEvents.size();
	for(int n=0; n<controllerAmount; n++)
	{
		addControlEvent( new ControllerEvent(sequence, track->controlEvents[n].getController(),
											 track->controlEvents[n].getTick(),
											 track->controlEvents[n].getValue()) );

	}

}

// FIXME - debug function, remove
void Track::checkControlEventsOrder()
{
    int ptick = -1;
    for(int n=0; n<controlEvents.size(); n++)
    {
        if(ptick != -1 and controlEvents[n].getTick() < ptick)
        {
            std::cout << "\n\n*** Error: control events in wrong order " << ptick << " then " << controlEvents[n].getTick() << std::endl;
        }
        ptick = controlEvents[n].getTick();
    }
}


// ------------------------------------- notes -------------------------------------

#pragma mark -

Note* Track::getNote(const int id)
{
    assertExpr(id,<,notes.size());
	return notes.get(id);
}

int Track::getNoteStartInPixels(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    return (int)round( notes[id].startTick * sequence->getZoom() );
}

int Track::getNoteEndInPixels(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    return (int)round( notes[id].endTick * sequence->getZoom() );
}

int Track::getNoteStartInMidiTicks(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    return notes[id].startTick;
}

int Track::getNoteEndInMidiTicks(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    return notes[id].endTick;
}

int Track::getNotePitchID(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());
    return notes[id].pitchID;
}

int Track::getNoteAmount()
{
    return notes.size();
}

int Track::getNoteVolume(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());
    return notes[id].volume;
}

int Track::getNoteString(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    //if(notes[id].tuning == NULL) notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning
    if(notes[id].getString() == -1) notes[id].findStringAndFretFromNote();

    return notes[id].getString();
}

int Track::getNoteFret(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());

    //if(notes[id].tuning == NULL) notes[id].setTuning(&graphics->guitarEditor->tuning); // make sure the note knows the tuning
    if(notes[id].getFret() == -1) notes[id].findStringAndFretFromNote();

    return notes[id].getFret();
}

// returns the amount of ALL types of controller, not only of specified type
// the only goal of id is to determine whether the app is searching for a control event or for a tempo event
int Track::getControllerEventAmount(const int controllerTypeID)
{
    if(controllerTypeID==201 /*tempo*/) return sequence->tempoEvents.size();
    return controlEvents.size();
}

ControllerEvent* Track::getControllerEvent(const int id, const int controllerTypeID)
{
    assertExpr(id,>=,0);
    if(controllerTypeID==201 /*tempo*/){
        assertExpr(id,<,sequence->tempoEvents.size());
    } else {
        assertExpr(id,<,controlEvents.size());
    }

    if(controllerTypeID==201 /*tempo*/) return &sequence->tempoEvents[id];
    return &controlEvents[id];
}

/*
 * The tick where the first note of the track starts playing.
 * Used mostly when scaling relative to track
 */

int Track::getFirstNoteTick(bool selectionOnly)
{

	if(!selectionOnly) return notes[0].startTick;

    const int noteAmount = notes.size();
    int tick = -1;

    for(int n=0; n<noteAmount; n++)
	{
		if( notes[n].isSelected() ) return notes[n].startTick;
    }//next

    return tick;

}

void Track::selectNote(const int id, const bool selected, bool ignoreModifiers)
{

    assert(id != SELECTED_NOTES); // not supported in this function


    if(!Display::isSelectMorePressed() and !Display:: isSelectLessPressed()) ignoreModifiers=true; // if no modifier is pressed, don't do any special checks

    // ----------------------------- select/deselect all notes ----------------------------
    if(id==ALL_NOTES)
	{
		// if this is a 'select none' command, unselect any selected measures in the top bar
		if(selected == FALSE) getMeasureBar()->unselect();

        if(graphics->editorMode == CONTROLLER)
		{ // controller editor must be handled differently
            graphics->controllerEditor->selectAll( selected );
        }
		else
		{

            if(ignoreModifiers)
			{
                for(int n=0; n<notes.size(); n++)
				{
                    notes[n].setSelected(selected);
                }//next
            }//end if

        }// end if

    }
	else  // ----------------------------- select/deselect one specific note ----------------------------
	{

        assertExpr(id,>=,0);
        assertExpr(id,<,notes.size());

        // if we ignore +/- key modifiers, just set the value right away
        if(ignoreModifiers) notes[id].setSelected(selected);
        else
		{ // otherwise, check key modifiers and set value accordingly
            if(selected)
			{
                if(Display::isSelectMorePressed()) notes[id].setSelected(true);
                else if(Display:: isSelectLessPressed()) notes[id].setSelected( !selected );
            }
        }//end if

    }//end if
}

bool Track::isNoteSelected(const int id)
{
    assertExpr(id,>=,0);
    assertExpr(id,<,notes.size());
    return notes[id].isSelected();
}


void Track::prepareNotesForGuitarEditor()
{

    for(int n=0; n<notes.size(); n++)
	{
        notes[n].checkIfStringAndFretMatchNote(true);
    }

}

// --------------------- get/set ---------------------
#pragma mark -

void Track::setId(const int id)
{
    trackid = id;
}

void Track::setName(wxString name)
{
    if(name.Trim().IsEmpty()) Track::name = wxString( _("Untitled") );
    else Track::name = name;
}
wxString& Track::getName()
{
    return name;
}


int Track::getGridDivider()
{
    return graphics->grid->divider;
}



// only used in manual channel mode
int Track::getChannel()
{
    // always 9 for drum tracks
    //const bool manual_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;

    if(graphics->editorMode == DRUM) return 9;
    else return channel;

}
void Track::setChannel(int i)
{
	channel = i;
}


// ------------------------------------- instrument -------------------------------------

void Track::setInstrument(int i, bool recursive)
{
	instrument = i;

	// if we're in manual channel management mode, change all tracks of the same channel to have the same instrument
	if(sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive)
	{
		const int trackAmount = sequence->getTrackAmount();
		for(int n=0; n<trackAmount; n++)
		{
			Track* track = sequence->getTrack(n);
			if(track == this) continue; // track must not evaluate itself...
			if( track->getChannel() == channel )
			{
				track->setInstrument(i,true);
			}
		}//next
	}//endif
}
int Track::getInstrument()
{
	return instrument;
}

// recursive is set to true when 'setDrumKit' when the function was called by itself
// it tells not to do any more recursion
void Track::setDrumKit(int i, bool recursive)
{

	// check id validity
	if (i == 0 ) ; //Standard
    else if (i == 8 ) ; //Room kit
    else if (i == 16 ) ; //Power kit
    else if (i == 24 ) ; //Electronic
    else if (i == 25 ) ; //Analog
    else if (i == 32 ) ; //Jazz
    else if (i == 40 ) ; //Brush
    else if (i == 48 ) ; //Orchestral
    else if (i == 56 ) ; //Special Effects
    else
	{
		// invalid drum kit ID
        return;
    }

	drumKit = i;

	// if we're in manual channel management mode, change all tracks of the same channel to have the same instrument
	if(sequence->getChannelManagementType() == CHANNEL_MANUAL and not recursive)
	{

		const int trackAmount = sequence->getTrackAmount();
		for(int n=0; n<trackAmount; n++)
		{
			Track* track = sequence->getTrack(n);
			if(track == this) continue; // track must not evaluate itself...
			track->setDrumKit(i,true);
		}//next
	}//endif
}
int Track::getDrumKit()
{
	return drumKit;
}

// ------------------------------------- midi playback / IO -------------------------------------
#pragma mark -

void Track::playNote(const int id, const bool noteChange)
{
	assertExpr(id,<,notes.size());
	assertExpr(id,>=,0);

	notes[id].play(noteChange);

}

// returns smallest values, ignoring -1, a being prioritary to b (returns -1 for none, 0 for a, 1 for b)
int getActiveMin(int a, int b)
{
	if(a==-1 and b==-1) return -1;
	if(a==-1) return 1;//b
	if(b==-1) return 0;//a

	if(a <= b) return 0;//a
	else return 1;//b
}

// returns smallest values, -1 being considered as 'no value'
// if some are euqal, 'a' is prioritary to 'b', and 'b' prioritary to 'c'
// (returns -1 for none, 0 for a, 1 for b, 2 for c)
int getActiveMin(int a, int b, int c)
{

	const int result1 = getActiveMin(b,c);

	int passnumber;
	if(result1==0) passnumber = b;
	else if(result1==1) passnumber = c;
	else if(result1==-1) passnumber =-1;

	const int result2 = getActiveMin( a, passnumber );

	if(result2==0) return 0;
	else
	{
		if(result1==-1) return -1;
		return result1+1;
	}

}


/*
 * Add Midi Events to JDKMidi track object
 */

int Track::addMidiEvents(jdkmidi::MIDITrack* midiTrack,
						 int track_ID, /* in manual channel mode, this argument is NOT considered */
						 int firstMeasure,
						 bool selectionOnly,
						 int& startTick)
{

	// ignore track if it's muted
	// (but for some reason drum track can't be completely omitted)
	// if we only play selection, ignore mute and play anyway
    if(graphics->muted and graphics->editorMode != DRUM and !selectionOnly)
		return -1;

    // if in manual mode, use the user-specified channel ID and not the stock one
    const bool manual_mode = sequence->getChannelManagementType() == CHANNEL_MANUAL;
    if(!manual_mode) track_ID = getChannel();

    // drum tracks
    if(graphics->editorMode == DRUM) track_ID = 9;

    //std::cout << "channel = " << track_ID << std::endl;

    // when previewing selected notes (start by finding the note that plays first, to start playing at the right place and note from the beginning)
    int firstNoteStartTick = -1;
    int selectedNoteAmount=0;

    if(selectionOnly)
	{

        const int noteAmount=notes.size();
        for(int n=0; n<noteAmount; n++)
		{
            if(notes[n].isSelected())
			{
                if(notes[n].startTick < firstNoteStartTick or firstNoteStartTick==-1) firstNoteStartTick = notes[n].startTick;
                selectedNoteAmount++;
            }
        }

        if(firstNoteStartTick == -1) return -1; // error, no note was found.
        if(selectedNoteAmount == 0) return -1; // error, no note was found.

    }
	else
	{
        firstNoteStartTick = getMeasureBar()->firstTickInMeasure(firstMeasure);
        startTick = firstNoteStartTick;
    }

    // set bank
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 0, 0 );

        if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; assert(0); }

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 32, 0 );

        if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; assert(0); }
    }

    // set instrument
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );

        if(graphics->editorMode == DRUM) m.SetProgramChange( track_ID, drumKit );
        else m.SetProgramChange( track_ID, instrument );

        if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding instrument at track beginning!" << std::endl; }
    }

    // set track name
    {

        jdkmidi::MIDITimedBigMessage m;
        m.SetText( 3 );
        m.SetByte1( 3 );

        const char* name_c = toCString(name);
        jdkmidi::MIDISystemExclusive sysex( (unsigned char*)name_c, name.size()+1, name.size()+1, false);

        m.CopySysEx( &sysex );
        m.SetTime( 0 );
        if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; assert(0); }
    }

    // set maximum volume
    {
        jdkmidi::MIDITimedBigMessage m;

        m.SetTime( 0 );
        m.SetControlChange( track_ID, 7, 127 );

        if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; assert(false); }
    }

	// ----------------------------------- add events in order --------------------------
	/*
	 * The way this section works:
	 *
	 * there are currently 3 possible source of events in a track (apart those above, like instrument and track name): note on, note off, controller change.
	 * each type of event is stored in its own vector, in time order.
	 * the variables below store the current event (i.e. the first event that hasn't yet been added)
	 * the section loops, and with it each iteration it checks the current tick of the 3 current events,
	 * then picks the one with the smallest tick and adds it to the track.
	 *
	 */

	int note_on_id = 0;
	int note_off_id = 0;
	int control_evt_id = 0;

	const int noteOnAmount = notes.size();
	const int noteOffAmount = noteOff.size();
	const int controllerAmount = controlEvents.size();

	// find track end
    int last_event_tick=0;

	// if muted and drums, return now
	if(graphics->muted and graphics->editorMode == DRUM and !selectionOnly) return -1;

	//std::cout << "-------------------- TRACK -------------" << std::endl;

	while(true)
	{

		// if we only want to play what's selected, skip unselected notes
		if(selectionOnly)
		{
			while(note_on_id < noteOnAmount and !notes[note_on_id].isSelected()) note_on_id++;
			while(note_off_id < noteOffAmount and !noteOff[note_off_id].isSelected()) note_off_id++;
		}

		const int tick_on      = (note_on_id < noteOnAmount)     ?   notes[note_on_id].startTick - firstNoteStartTick      :  -1;
		const int tick_off     = (note_off_id < noteOffAmount)   ?   noteOff[note_off_id].endTick - firstNoteStartTick     :  -1;

		// ignore control events when only playing selection
		const int tick_control = (control_evt_id < controllerAmount and !selectionOnly)  ?   controlEvents[control_evt_id].getTick() - firstNoteStartTick : -1;

		const int activeMin = getActiveMin( tick_off, tick_control, tick_on );

		if(activeMin==-1) break; // all events have been added

		jdkmidi::MIDITimedBigMessage m;

		//  ------------------------ add note on event ------------------------
		if(activeMin == 2)
		{

            const int time=notes[note_on_id].startTick - firstNoteStartTick;
            if(!(time < 0))
			{

				m.SetTime( time );


				if(graphics->editorMode == DRUM) m.SetNoteOn( track_ID, notes[note_on_id].pitchID, notes[note_on_id].volume );
				else m.SetNoteOn( track_ID, 131-notes[note_on_id].pitchID, notes[note_on_id].volume );

				// find track end
				if(notes[note_on_id].endTick > last_event_tick) last_event_tick = notes[note_on_id].endTick;

				if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
			}

			note_on_id++;
		}
		//  ------------------------ add note off event ------------------------
		else if(activeMin == 0)
		{

			const int time=noteOff[note_off_id].endTick - firstNoteStartTick;
            if(!(time < 0))
			{

				m.SetTime( time );

				if(graphics->editorMode == DRUM) m.SetNoteOff( track_ID, noteOff[note_off_id].pitchID, 0 );
				else m.SetNoteOff( track_ID, 131-noteOff[note_off_id].pitchID, 0 );

				// find track end
				if(time > last_event_tick) last_event_tick = time;

				if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
			}
			note_off_id++;
		}
		//  ------------------------ add control change event ------------------------
		else if(activeMin == 1)
		{
			const int controllerID = controlEvents[control_evt_id].getController();

			// pitch bend
			if(controllerID==200)
			{


				int time=controlEvents[control_evt_id].getTick() - firstNoteStartTick;

				// controller changes happens before the area we play
				// but perhaps it still is affecting the area we want to play - check for that.
				bool addControlEvent = true;
				if(time < 0)
				{
					if(control_evt_id+1 < controllerAmount)
					{
						addControlEvent = false;
						int checkEventID = control_evt_id+1;
						// check if there are other controller events of the same type before the area we play.
						while(true)
						{

							if((controlEvents[checkEventID].getTick() - firstNoteStartTick)<1  and
							   controlEvents[checkEventID].getController() == controllerID)
							{
								// the current event has no effect, there is another one later, disregard it.
								addControlEvent = false;
								break;
							}

							if((controlEvents[checkEventID].getTick() - firstNoteStartTick)>0  and
							   controlEvents[checkEventID].getController() == controllerID)
							{
								// there is another event, but it's later so current event is still relevant.
								addControlEvent = true;
								time = 0;
								break;
							}

							checkEventID++;
							if( ! (checkEventID < controllerAmount ) )
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

				if(addControlEvent)
				{

					m.SetTime( time );

					// ((value * 127) - 8191) ?
					int pitchBendVal = (127-controlEvents[control_evt_id].getValue())*128-128*128;

					if(pitchBendVal > 0) pitchBendVal-=8192;
					else if(pitchBendVal < 0) pitchBendVal +=8192;

					m.SetPitchBend( track_ID,
									pitchBendVal );

					if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
				}
				control_evt_id++;

			}
			// other controller
			else
			{


				int time=controlEvents[control_evt_id].getTick() - firstNoteStartTick;

				// controller changes happens before the area we play
				// but perhaps it still is affecting the area we want to play - check for that.
				bool addControlEvent = true;
				if(time < 0)
				{
					if(control_evt_id+1 < controllerAmount)
					{
						addControlEvent = false;
						int checkEventID = control_evt_id+1;
						// check if there are other controller events of the same type before the area we play.
						while(true)
						{

							if((controlEvents[checkEventID].getTick() - firstNoteStartTick)<1  and
							   controlEvents[checkEventID].getController() == controllerID)
							{
								// the current event has no effect, there is another one later, disregard it.
								addControlEvent = false;
								break;
							}

							if((controlEvents[checkEventID].getTick() - firstNoteStartTick)>0  and
							   controlEvents[checkEventID].getController() == controllerID)
							{
								// there is another event, but it's later so current event is still relevant.
								addControlEvent = true;
								time = 0;
								break;
							}

							checkEventID++;
							if( ! (checkEventID < controllerAmount ) )
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

				if(addControlEvent)
				{
					m.SetTime( time );

					m.SetControlChange( track_ID,
										controllerID,
										127-controlEvents[control_evt_id].getValue() );

					if( !midiTrack->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
				}

				control_evt_id++;

			}//endif (control/pitch-bend)

		}// if (note/note off/control)

	}//wend


	if(selectionOnly) startTick = firstNoteStartTick;
    return last_event_tick - firstNoteStartTick;


}

void Track::copy()
{

    if(graphics->editorMode == CONTROLLER)
	{
		wxBell();
		return; // no copy/paste in controller mode
    }

	Clipboard::clear();

	int tickOfFirstSelectedNote=-1;
    // place all selected notes into clipboard
    for(int n=0; n<notes.size(); n++)
	{
        if(!notes[n].isSelected()) continue;

        Note* tmp=new Note(notes[n]);
		Clipboard::add(tmp);

		// if in guitar mode, make sure string/fret and note match
		if(graphics->editorMode == GUITAR) Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(false);
		else Clipboard::getNote( Clipboard::getSize()-1 )->checkIfStringAndFretMatchNote(true);

		// find tickOfFirstSelectedNote of the first note
		if(notes[n].startTick < tickOfFirstSelectedNote or tickOfFirstSelectedNote==-1) tickOfFirstSelectedNote=notes[n].startTick;

    }//next

    // remove all empty measures before notes, so that they appear in the current measure when pasting
	const int lastMeasureStart = getMeasureBar()->firstTickInMeasure( getMeasureBar()->measureAtTick(tickOfFirstSelectedNote) );

	const int clipboard_size = Clipboard::getSize();
	for(int n=0; n<clipboard_size; n++)
	{
        //Clipboard::getNote(n)->move( -lastMeasureStart, 0, graphics->editorMode);
        graphics->getCurrentEditor()->moveNote(*Clipboard::getNote(n), -lastMeasureStart, 0);
    }

	sequence->notes_shift_when_no_scrolling = lastMeasureStart;
}


// -----------------------------------------------------------------------------------------------------
// ------------------------------------------- SAVE/LOAD -----------------------------------------------
// -----------------------------------------------------------------------------------------------------

void Track::saveToFile(wxFileOutputStream& fileout)
{
	reorderNoteVector();
	reorderNoteOffVector();

	writeData(wxT("\n<track name=\"") + name +
			  wxT("\" channel=\"") + to_wxString(channel) +
			  wxT("\">\n"), fileout );

	graphics->saveToFile(fileout);

	// notes
    for(int n=0; n<notes.size(); n++)
        notes[n].saveToFile(fileout);

	// controller changes
    for(int n=0; n<controlEvents.size(); n++)
        controlEvents[n].saveToFile(fileout);

	writeData(wxT("</track>\n\n"), fileout );


}

bool Track::readFromFile(irr::io::IrrXMLReader* xml)
{

	notes.clearAndDeleteAll();
	noteOff.clearWithoutDeleting(); // have already been deleted by previous command
    controlEvents.clearAndDeleteAll();

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
					if(name!=NULL) setName( fromCString((char*)name) );
					else
					{
						std::cout << "Missing info from file: track name" << std::endl;
						setName( wxString(_("Untitled")) );
					}
					const char* channel_c = xml->getAttributeValue("channel");
					if(channel_c!=NULL)
					{
						int loaded_channel = atoi(channel_c);
						if(loaded_channel >=-0 and loaded_channel<16)
							channel = loaded_channel;
						else
							std::cerr << "Invalid channel : " << loaded_channel << std::endl;
					}
					else
					{
						std::cout << "Missing info from file: track channel" << std::endl;
					}

                }
                else if (!strcmp("editor", xml->getNodeName()))
				{
					if(! graphics->readFromFile(xml) )
						return false;
                }
				else if (!strcmp("magneticgrid", xml->getNodeName()))
				{
					if(! graphics->readFromFile(xml) )
						return false;
                }
				else if (!strcmp("instrument", xml->getNodeName()))
				{
					const char* id = xml->getAttributeValue("id");
					if(id != NULL) instrument = atoi(id);
					else
					{
						std::cout << "Missing info from file: instrument ID" << std::endl;
						instrument = 0;
					}
                }
				else if (!strcmp("drumkit", xml->getNodeName()))
				{
					const char* id = xml->getAttributeValue("id");
					if(id != NULL) drumKit = atoi(id);
					else
					{
						std::cout << "Missing info from file: drum ID" << std::endl;
					}
                }
				else if (!strcmp("guitartuning", xml->getNodeName()))
				{
					if(! graphics->readFromFile(xml) )
						return false;
                }
                else if (!strcmp("key", xml->getNodeName()))
				{
					if(! graphics->readFromFile(xml) )
						return false;
                }
				else if (!strcmp("note", xml->getNodeName()))
				{
					Note* temp = new Note(graphics);
					if(! temp->readFromFile(xml) )
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
					if( !temp->readFromFile(xml) )
					{
						// thre was an error when trying to load control event
						std::cout << "A controller event was discarded because it is invalid" << std::endl;
						delete temp;
					}
					else
					{
						// controller successfully loaded, add to controllers vector
						controlEvents.push_back( temp );
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

    }while(xml && xml->read());

	reorderNoteOffVector();

	return true;

}


}
