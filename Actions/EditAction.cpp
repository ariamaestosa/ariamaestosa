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

#include "GUI/GraphicalTrack.h"
#include <vector>

namespace AriaMaestosa
{

namespace Action
{

    SingleTrackAction::SingleTrackAction() : EditAction() {}
    MultiTrackAction::MultiTrackAction() : EditAction() {}

EditAction::EditAction()
{

}
void SingleTrackAction::setParentTrack(Track* parent)
{
    track = parent;
}
void MultiTrackAction::setParentSequence(Sequence* parent)
{
    sequence = parent;
}

SingleTrackAction::~SingleTrackAction()
{
}
MultiTrackAction::~MultiTrackAction()
{
}

EditAction::~EditAction() {}
void EditAction::perform() { std::cout << "EditAction::perform called, this shouldn't happen" << std::endl; }
void EditAction::undo() { std::cout << "EditAction::uno called, this shouldn't happen" << std::endl; }

}

void NoteRelocator::setParent(Track* t)
{
    track = t;
}

void NoteRelocator::prepareToRelocate()
{
    id = 0;
    noteamount_in_track = track->notes.size();
    noteamount_in_relocator = notes.size();
}
Note* NoteRelocator::getNextNote()
{
    if(id >= noteamount_in_relocator) return NULL;

    id++;
    return notes.get(id-1);
}
void NoteRelocator::rememberNote(Note& n)
{
    notes.push_back(&n);
}
void NoteRelocator::rememberNote(Note* n)
{
    notes.push_back(n);
}
NoteRelocator::~NoteRelocator()
{
}


// ------

void ControlEventRelocator::setParent(Track* t)
{
    track = t;
}

void ControlEventRelocator::prepareToRelocate()
{
    id = 0;
    amount_in_track = track->controlEvents.size();
    amount_in_relocator = events.size();
}
ControllerEvent* ControlEventRelocator::getNextControlEvent()
{
    if(id >= amount_in_relocator) return NULL;

    id++;
    return events.get(id-1);
}
void ControlEventRelocator::rememberControlEvent(ControllerEvent& n)
{
    events.push_back(&n);
}
ControlEventRelocator::~ControlEventRelocator()
{
}


}
