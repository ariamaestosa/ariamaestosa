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

#include "Actions/SetNoteVolume.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"

namespace AriaMaestosa
{
    namespace Action
{
    void SetNoteVolume::undo()
    {
        std::cout << "undoing set volume" << std::endl;
        //undo_obj.restoreState(track);
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();
        int n = 0;
        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            current_note->setVolume( volumes[n] );
            n++;
        }
    }
    void SetNoteVolume::perform()
    {
        assert(track != NULL);

        if (noteID == SELECTED_NOTES)
        {

            bool played = false;
            const int noteAmount = track->notes.size();
            for(int n=0; n<noteAmount; n++)
            {
                if (track->notes[n].isSelected())
                {
                    volumes.push_back( track->notes[n].volume  );
                    track->notes[n].setVolume( volume );
                    relocator.rememberNote(track->notes[n]);
                    if (!played)
                    {
                        track->notes[n].play(true);
                        played = true;
                    }
                }
            }//next note

        }
        else
        {
            // single note

            assertExpr(noteID,>=,0);
            assertExpr(noteID,<,track->notes.size());

            // if user changed the volume of a note that is not selected, change the volume of this note only
            volumes.push_back( track->notes[noteID].volume  );
            track->notes[noteID].setVolume( volume );
            relocator.rememberNote(track->notes[noteID]);
            track->notes[noteID].play(true);
        }

}
SetNoteVolume::SetNoteVolume(const int volume, const int noteID)
{
    SetNoteVolume::volume = volume;
    SetNoteVolume::noteID = noteID;
}
SetNoteVolume::~SetNoteVolume() {}

}
}
