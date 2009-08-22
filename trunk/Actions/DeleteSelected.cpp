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

#include "Actions/DeleteSelected.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Note.h"
#include "Midi/Sequence.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/ControllerEditor.h"

namespace AriaMaestosa
{
    namespace Action
{
    void DeleteSelected::undo()
    {
        const int noteAmount = removedNotes.size();
        const int controlAmount = removedControlEvents.size();

        if (noteAmount > 0)
        {

            for(int n=0; n<noteAmount; n++)
            {
                track->addNote( removedNotes.get(n), false );
            }
            // we will be using the notes again, make sure it doesn't delete them
            removedNotes.clearWithoutDeleting();

        }
        else if (controlAmount > 0)
        {

            for(int n=0; n<controlAmount; n++)
            {
                track->addControlEvent( removedControlEvents.get(n) );
            }
            // we will be using the notes again, make sure it doesn't delete them
            removedControlEvents.clearWithoutDeleting();

        }
    }
    void DeleteSelected::perform()
    {
        assert(track != NULL);

        if (track->graphics->editorMode == CONTROLLER)
        {

            const int from = track->graphics->controllerEditor->getSelectionBegin();
            const int to = track->graphics->controllerEditor->getSelectionEnd();
            const int type = track->graphics->controllerEditor->getCurrentControllerType();

            if (type != 201 /*tempo*/)
            {
                // remove controller events
                for(int n=0; n<track->controlEvents.size(); n++)
                {

                    if (track->controlEvents[n].getController() != type) continue; // in another controller

                    const int tick = track->controlEvents[n].getTick();

                    if (tick<from or tick>to) continue; // this event is not concerned by selection

                    removedControlEvents.push_back( track->controlEvents.get(n) );
                    track->controlEvents.remove(n);
                    n--;
                }//next

            }
            else
            {
                // remove tempo events
                const int tempoEventsAmount = track->sequence->tempoEvents.size();
                for(int n=0; n<tempoEventsAmount; n++)
                {

                    const int tick = track->sequence->tempoEvents[n].getTick();

                    if (tick<from or tick>to) continue; // this event is not concerned by selection

                    removedControlEvents.push_back( track->sequence->tempoEvents.get(n) );
                    track->sequence->tempoEvents.markToBeRemoved(n);
                    //n--;
                }//next
                track->sequence->tempoEvents.removeMarked();
            }

        }
        else
        {

            for(int n=0; n<track->notes.size(); n++)
            {
                if (!track->notes[n].isSelected()) continue;

                // also delete corresponding note off event
                for(int i=0; i<track->noteOff.size(); i++)
                {
                    if (&track->noteOff[i] == &track->notes[n])
                    {
                        track->noteOff.remove(i);
                        break;
                    }
                }

                //notes.erase(n);
                removedNotes.push_back( track->notes.get(n) );
                track->notes.remove(n);

                n--;
            }//next

        }


        track->reorderNoteOffVector();
}
DeleteSelected::DeleteSelected()
{
}
DeleteSelected::~DeleteSelected()
{
}
}
}
