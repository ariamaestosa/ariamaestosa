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

#include "Actions/SetTrackVolume.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

SetTrackVolume::SetTrackVolume(const int volume) :
    //I18N: (undoable) action name
    SingleTrackAction( _("change track volume") )
{
    m_volume = volume;
}


SetTrackVolume::~SetTrackVolume()
{
}

void SetTrackVolume::undo()
{
    m_track->setVolume(m_old_volume);
}

void SetTrackVolume::perform()
{
    ASSERT(m_track != NULL);
    m_old_volume = m_track->getVolume();
    m_track->setVolume(m_volume);
}

