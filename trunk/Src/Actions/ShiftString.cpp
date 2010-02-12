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

#include "Actions/ShiftString.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"

namespace AriaMaestosa
{
    namespace Action
{
    void ShiftString::undo()
    {
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();
        int n=0;
        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            current_note->setStringAndFret( strings[n], frets[n] );
            n++;
        }
    }
    void ShiftString::perform()
    {
        assert(track != NULL);

        if (track->graphics->editorMode != GUITAR) return;

        assert(noteid != ALL_NOTES); // not supported in this function (mostly because not needed, but could logically be implmented)

        // only accept to do this in guitar mode
        if (track->graphics->editorMode != GUITAR)
            return;

        if (noteid==SELECTED_NOTES)
        {

            bool played=false;
            const int amount_n = track->notes.size();
            for(int n=0; n<amount_n; n++)
            {
                if (!track->notes[n].isSelected()) continue;

                frets.push_back( track->notes[n].getFret() );
                strings.push_back( track->notes[n].getString() );

                track->notes[n].shiftString(amount);

                relocator.rememberNote( track->notes[n] );

                if (!played)
                {
                    track->notes[n].play(false);
                    played = true;
                }
            }//next

        }
        else
        {
            assertExpr(noteid,>=,0);
            assertExpr(noteid,<,track->notes.size());

            frets.push_back( track->notes[noteid].getFret() );
            strings.push_back( track->notes[noteid].getString() );
            track->notes[noteid].shiftString(amount);
            relocator.rememberNote( track->notes[noteid] );
            track->notes[noteid].play(false);
        }

}
ShiftString::ShiftString(const int amount, const int noteid)
{
    ShiftString::amount = amount;
    ShiftString::noteid = noteid;

}
ShiftString::~ShiftString()
{
}

}
}
