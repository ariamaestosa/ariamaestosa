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

#include "Actions/DeleteTrack.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "AriaCore.h"

#include <wx/intl.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

DeleteTrack::DeleteTrack(Sequence* seq) :
    //I18N: (undoable) action name
    MultiTrackAction( _("delete track") )
{
    m_removed_track   = NULL;
    m_parent_sequence = seq;
}

// --------------------------------------------------------------------------------------------------------

DeleteTrack::~DeleteTrack()
{
    delete m_removed_track;
    m_removed_track = NULL;
}

// --------------------------------------------------------------------------------------------------------

void DeleteTrack::undo()
{
    ASSERT(m_removed_track != NULL)
    
    // add back the track that was removed (FIXME: not added where it was)
    m_parent_sequence->addTrack(m_removed_track);
    m_removed_track = NULL;
}

// --------------------------------------------------------------------------------------------------------

void DeleteTrack::perform()
{
    m_removed_track = m_parent_sequence->removeSelectedTrack();
    ASSERT(m_removed_track != NULL);
}

// --------------------------------------------------------------------------------------------------------


