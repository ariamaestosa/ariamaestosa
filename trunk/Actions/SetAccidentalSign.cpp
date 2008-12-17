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

#include "Actions/SetAccidentalSign.h"
#include "Actions/EditAction.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Track.h"
#include "Editors/Editor.h"
#include "Editors/ScoreEditor.h"

namespace AriaMaestosa
{
namespace Action
{

void SetAccidentalSign::undo()
{
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();

        int n=0;
        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            current_note->preferred_accidental_sign = original_signs[n];
            current_note->pitchID = pitch[n];
            n++;
        }
}
void SetAccidentalSign::perform()
{

    bool played = false;

    const int noteAmount=track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        if(!track->isNoteSelected(n)) continue;

        original_signs.push_back( track->notes[n].preferred_accidental_sign );
        pitch.push_back(  track->notes[n].pitchID );

        track->graphics->scoreEditor->setNoteSign(sign, n);
        relocator.rememberNote( track->notes[n] );

        if(!played)
        {
            track->notes[n].play(true);
            played = true;
        }
    }//next

}
SetAccidentalSign::SetAccidentalSign(const int sign)
{
    SetAccidentalSign::sign = sign;
}
SetAccidentalSign::~SetAccidentalSign() {}

}
}
