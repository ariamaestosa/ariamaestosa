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

#include "Actions/ScaleTrack.h"
#include "Actions/ScaleSong.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa
{
    namespace Action
{


    ScaleSong::ScaleSong(float factor, int relative_to)
    {
        ScaleSong::factor = factor;
        ScaleSong::relative_to = relative_to;
    }

    void ScaleSong::perform()
    {
        const int trackAmount = sequence->getTrackAmount();
        for(int t=0; t<trackAmount; t++)
        {
            Action::ScaleTrack* action = new Action::ScaleTrack(factor, relative_to, false);
            action->setParentTrack(sequence->getTrack(t));
            action->perform();
            actions.push_back(action);
        }
    }
    void ScaleSong::undo()
    {
        const int amount = actions.size();
        for(int a=0; a<amount; a++)
        {
            actions[a].undo();
        }
    }

    ScaleSong::~ScaleSong()
    {
    }

}
}

