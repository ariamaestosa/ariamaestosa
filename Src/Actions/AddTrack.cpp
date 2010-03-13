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

#include "Actions/AddTrack.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "AriaCore.h"

using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

AddTrack::AddTrack(Sequence* seq) :
    //I18N: (undoable) action name
    MultiTrackAction( _("add track") )
{
    m_added_track     = NULL;
    m_parent_sequence = seq;
}

// --------------------------------------------------------------------------------------------------------

AddTrack::~AddTrack()
{
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::undo()
{
    assert(m_added_track != NULL)
    m_parent_sequence->deleteTrack(m_added_track);
    m_added_track = NULL;
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::perform()
{
    m_added_track = m_parent_sequence->addTrack();
}

// --------------------------------------------------------------------------------------------------------


