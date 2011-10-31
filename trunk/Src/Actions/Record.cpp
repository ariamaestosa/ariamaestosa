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

#include "Actions/Record.h"

#include "AriaCore.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

Record::Record() :
    //I18N: (undoable) action name
    SingleTrackAction( _("record from MIDI instrument") )
{
}

// ----------------------------------------------------------------------------------------------------------

Record::~Record()
{
}

// ----------------------------------------------------------------------------------------------------------

void Record::undo()
{
    for (int n=m_actions.size() - 1; n >= 0; n--)
    {
        m_actions[n].undo();
    }
    m_actions.clearAndDeleteAll();
}

// ----------------------------------------------------------------------------------------------------------

void Record::perform()
{
    ASSERT( MAGIC_NUMBER_OK_FOR(*m_visitor.raw_ptr) );
}

// ----------------------------------------------------------------------------------------------------------

void Record::action(SingleTrackAction* actionObj)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    actionObj->setParentTrack(m_track, new Track::TrackVisitor(*m_visitor.raw_ptr));
    m_actions.push_back( actionObj );
    actionObj->perform();
}

// ----------------------------------------------------------------------------------------------------------

bool Record::canUndoNow()
{
    return not PlatformMidiManager::get()->isRecording();
}

