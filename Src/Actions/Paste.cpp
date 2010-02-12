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

#include "Actions/Paste.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Clipboard.h"
#include "AriaCore.h"
#include "Editors/KeyboardEditor.h"

namespace AriaMaestosa
{
    namespace Action
{
    void Paste::undo()
    {
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();

        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            const int noteAmount = track->getNoteAmount();
            for(int n=0; n<noteAmount; n++)
            {
                if (track->getNote(n) == current_note)
                {
                    track->removeNote(n);
                    break;
                }//endif
            }//next
        }//wend
    }

    void Paste::perform()
    {
        assert(track != NULL);

        if (track->graphics->editorMode == CONTROLLER)
        {
            wxBell();
            return; // no copy/paste in controller mode
        }
        if (Clipboard::getSize() == 0) return; // nothing copied

        const int copied_beat_length = Clipboard::getBeatLength();
        const int seq_beat_length = track->sequence->ticksPerBeat();
        float scale_pasted_notes = 1;
        bool need_to_scale_pasted_notes = false;

        if (copied_beat_length != seq_beat_length)
        {
            scale_pasted_notes = (float)seq_beat_length / (float)copied_beat_length;
            need_to_scale_pasted_notes = true;
            std::cout << "pasting from a song with beat=" << copied_beat_length << " to one with beat=" << seq_beat_length <<
                ", it will be necessary to scale notes by " << scale_pasted_notes << std::endl;
        }

        // used to find when the first note is played.
        // If we're pasting at the mouse cursor, it is necessary to know that so that the first note is next to the cursor.
        // However, if we're pasting in the first measure, we don't want this info since we want the track->notes to keep their location within the measure.
        int beginning=-1;
        if (!atMouse) beginning=0;

        // unselected previously selected track->notes
        for(int n=0; n<track->notes.size(); n++) track->notes[n].setSelected(false);

        // find where track->notes begin if necessary
        if (atMouse)
            beginning = Clipboard::getNote(0)->startTick;

        int shift=0;

        // if "paste at mouse", use its location to know where to paste track->notes
        wxPoint mouseLoc=wxGetMousePosition();

        int trackMouseLoc_x, trackMouseLoc_y;

        Display::screenToClient(mouseLoc.x, mouseLoc.y, &trackMouseLoc_x, &trackMouseLoc_y);

        /*
         * Calculate 'shift' (i.e. X position of pasted notes)
         */
        if ( atMouse and
            trackMouseLoc_y > track->graphics->getCurrentEditor()->getEditorYStart() and
            trackMouseLoc_y < track->graphics->getCurrentEditor()->getYEnd() and
            trackMouseLoc_x > getEditorsXStart())
        {

            RelativeXCoord mx(trackMouseLoc_x, WINDOW);

            shift=track->graphics->getCurrentEditor()->snapMidiTickToGrid( mx.getRelativeTo(MIDI) );

        }
        else if (!atMouse and track->sequence->x_scroll_upon_copying == track->sequence->getXScrollInPixels() )
        {

            /*
             * scrolling has not changed since copy (i.e. user probably hit 'copy' and 'paste' consecutively)
             * in this case, we want the track->notes to be pasted exactly where they were copied
             * so we just overwrite the shift variable to make track->notes paste where they were copied.
             * after this, check if all track->notes will be visible when pasting like this. If not, fall back to regular pasting method
             * that will take care of finding track->notes an appropriate location to be pasted onto.
             */
            shift = track->sequence->notes_shift_when_no_scrolling;

            // find if first note will be visible in the location just calculated,
            // otherwise just go to regular pasting code, it will paste them within visible measures
            Note& tmp = *Clipboard::getNote(0);

            // before visible area
            if ((tmp.startTick + tmp.endTick)/2 + shift < track->sequence->getXScrollInMidiTicks())
                goto regular_paste;

            // after visible area
            RelativeXCoord screen_width( Display::getWidth(), WINDOW );

            if ((tmp.startTick + tmp.endTick)/2 + shift > screen_width.getRelativeTo(MIDI) )
                goto regular_paste;

        }
        else
        {

regular_paste: // FIXME - find better way than goto
            /*
             * This part will change to value of 'shift' variable to make sure pasted notes will be visible
             */

            // if not "paste at mouse", find first visible measure
            int measure = getMeasureData()->measureAtTick(track->sequence->getXScrollInMidiTicks())-1;
            if (measure < 0) measure = 0;
            const int lastMeasureStart = getMeasureData()->firstTickInMeasure( measure );
            shift = track->graphics->keyboardEditor->snapMidiTickToGrid(lastMeasureStart);

            // find if all track->notes will be visible in the location just calculated,
            // otherwise move them one more measure ahead (if measure is half-visible because of scrolling)
            Note& first_note = *(Clipboard::getNote(0));

            // check if note is before visible area
            while((first_note.startTick + first_note.endTick)/2 + shift < track->sequence->getXScrollInMidiTicks())
            {
                shift = getMeasureData()->firstTickInMeasure( getMeasureData()->measureAtTick( shift )+1 );
            }//end if

        }

        // add new notes
        const int clipboardSize = Clipboard::getSize();
        for(int n=0; n<clipboardSize; n++)
        {
            Note* tmp=new Note( *(Clipboard::getNote(n)) );

            if (need_to_scale_pasted_notes)
            {
                tmp->startTick *= scale_pasted_notes;
                tmp->endTick *= scale_pasted_notes;
            }

            track->graphics->getCurrentEditor()->moveNote(*tmp, shift , 0);
            if (atMouse) track->graphics->getCurrentEditor()->moveNote(*tmp, -beginning , 0);

            tmp->setParent( track->graphics );
            tmp->setSelected(true);

            if (track->graphics->editorMode == GUITAR) tmp->checkIfStringAndFretMatchNote(true);

            track->addNote( tmp, false );
            relocator.rememberNote( *tmp );
        }//next

        track->reorderNoteVector();
        track->reorderNoteOffVector();
}

Paste::Paste(const bool atMouse)
{
    Paste::atMouse = atMouse;
}
Paste::~Paste() {}
}
}
