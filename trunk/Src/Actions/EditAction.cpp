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

#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/ControllerEvent.h"

//#include "GUI/GraphicalTrack.h"
#include <vector>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------

EditAction::EditAction(wxString name) : m_name(name)
{
    
}


// ----------------------------------------------------------------------------------------------------

SingleTrackAction::SingleTrackAction(wxString name) : EditAction(name)
{
}

// ----------------------------------------------------------------------------------------------------

void SingleTrackAction::setParentTrack(Track* parent, Track::TrackVisitor* visitor)
{
    ASSERT( MAGIC_NUMBER_OK_FOR(*parent) );
    ASSERT( MAGIC_NUMBER_OK_FOR(*visitor) );
    
    m_track   = parent;
    m_visitor = visitor;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

MultiTrackAction::MultiTrackAction(wxString name) : EditAction(name)
{
}


// ----------------------------------------------------------------------------------------------------

void MultiTrackAction::setParentSequence(Sequence* parent, SequenceVisitor* visitor)
{
    m_sequence = parent;
    m_visitor  = visitor;
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void NoteRelocator::setParent(Track* t)
{
    m_track = t;
}

// ----------------------------------------------------------------------------------------------------

void NoteRelocator::prepareToRelocate()
{
    m_id                      = 0;
    m_noteamount_in_track     = m_track->getNoteAmount();
    m_noteamount_in_relocator = notes.size();
}

// ----------------------------------------------------------------------------------------------------

Note* NoteRelocator::getNextNote()
{
    if (m_id >= m_noteamount_in_relocator) return NULL;
    
    m_id++;
    return notes.get(m_id - 1);
}

// ----------------------------------------------------------------------------------------------------

void NoteRelocator::rememberNote(Note& n)
{
    notes.push_back(&n);
}

// ----------------------------------------------------------------------------------------------------

void NoteRelocator::rememberNote(Note* n)
{
    notes.push_back(n);
}

// ----------------------------------------------------------------------------------------------------

NoteRelocator::~NoteRelocator()
{
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

void ControlEventRelocator::setParent(Track* t, Track::TrackVisitor* visitor)
{
    m_track   = t;
    m_visitor = visitor;
}

// ----------------------------------------------------------------------------------------------------

void ControlEventRelocator::prepareToRelocate()
{
    m_id                  = 0;
    m_amount_in_track     = m_visitor->getControlEventVector().size();
    m_amount_in_relocator = events.size();
}

// ----------------------------------------------------------------------------------------------------

ControllerEvent* ControlEventRelocator::getNextControlEvent()
{
    if (m_id >= m_amount_in_relocator) return NULL;
    
    m_id++;
    return events.get(m_id - 1);
}

// ----------------------------------------------------------------------------------------------------

void ControlEventRelocator::rememberControlEvent(ControllerEvent& n)
{
    events.push_back(&n);
}

// ----------------------------------------------------------------------------------------------------

ControlEventRelocator::~ControlEventRelocator()
{
}

