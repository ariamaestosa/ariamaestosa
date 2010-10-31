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

#include <wx/intl.h>
#include <wx/utils.h>
#include <wx/window.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// -------------------------------------------------------------------------------------------------------------

Paste::Paste(const bool atMouse) :
    //I18N: (undoable) action name
    SingleTrackAction( _("paste") )
{
    m_at_mouse = atMouse;
}

// -------------------------------------------------------------------------------------------------------------

Paste::~Paste()
{
}

// -------------------------------------------------------------------------------------------------------------

void Paste::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();

    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        const int noteAmount = track->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            if (track->getNote(n) == current_note)
            {
                track->removeNote(n);
                break;
            }//endif
        }//next
    }//wend
}

// -------------------------------------------------------------------------------------------------------------

void Paste::perform()
{
    ASSERT(track != NULL);

    if (track->graphics->getEditorMode() == CONTROLLER)
    {
        wxBell();
        return; // no copy/paste in controller mode
    }
    if (Clipboard::getSize() == 0) return; // nothing copied

    const int copiedBeatLength = Clipboard::getBeatLength();
    const int seqBeatLength = track->getSequence()->ticksPerBeat();
    float scalePastedNotes = 1;
    bool needToScalePastedNotes = false;

    if (copiedBeatLength != seqBeatLength)
    {
        scalePastedNotes = (float)seqBeatLength / (float)copiedBeatLength;
        needToScalePastedNotes = true;
        std::cout << "pasting from a song with beat=" << copiedBeatLength << " to one with beat=" << seqBeatLength <<
            ", it will be necessary to scale notes by " << scalePastedNotes << std::endl;
    }

    // used to find when the first note is played.
    // If we're pasting at the mouse cursor, it is necessary to know that so that the first note is next to the cursor.
    // However, if we're pasting in the first measure, we don't want this info since we want the track->m_notes to keep their location within the measure.
    int beginning=-1;
    if (not m_at_mouse) beginning=0;

    // unselected previously selected track->m_notes
    for (int n=0; n<track->m_notes.size(); n++) track->m_notes[n].setSelected(false);

    // find where track->m_notes begin if necessary
    if (m_at_mouse)
    {
        beginning = Clipboard::getNote(0)->startTick;
    }
    int shift=0;

    // if "paste at mouse", use its location to know where to paste track->m_notes
    wxPoint mouseLoc = wxGetMousePosition();

    int trackMouseLoc_x, trackMouseLoc_y;

    Display::screenToClient(mouseLoc.x, mouseLoc.y, &trackMouseLoc_x, &trackMouseLoc_y);

    Editor* editor = track->graphics->getCurrentEditor();
    
    // Calculate 'shift' (i.e. X position of pasted notes)
    if (m_at_mouse and
        trackMouseLoc_y > editor->getEditorYStart() and
        trackMouseLoc_y < editor->getYEnd() and
        trackMouseLoc_x > Editor::getEditorXStart())
    {

        RelativeXCoord mx(trackMouseLoc_x, WINDOW);

        shift=track->graphics->getCurrentEditor()->snapMidiTickToGrid( mx.getRelativeTo(MIDI) );

    }
    else if (not m_at_mouse and
             track->getSequence()->x_scroll_upon_copying == track->getSequence()->getXScrollInPixels() )
    {

        /*
         * scrolling has not changed since copy (i.e. user probably hit 'copy' and 'paste' consecutively)
         * in this case, we want the track->m_notes to be pasted exactly where they were copied
         * so we just overwrite the shift variable to make track->m_notes paste where they were copied.
         * after this, check if all track->m_notes will be visible when pasting like this. If not, fall back to regular pasting method
         * that will take care of finding track->m_notes an appropriate location to be pasted onto.
         */
        shift = track->getSequence()->notes_shift_when_no_scrolling;

        // find if first note will be visible in the location just calculated,
        // otherwise just go to regular pasting code, it will paste them within visible measures
        Note& tmp = *Clipboard::getNote(0);

        // before visible area
        if ((tmp.startTick + tmp.endTick)/2 + shift < track->getSequence()->getXScrollInMidiTicks())
        {
            goto regular_paste;
        }

        // after visible area
        RelativeXCoord screen_width( Display::getWidth(), WINDOW );

        if ((tmp.startTick + tmp.endTick)/2 + shift > screen_width.getRelativeTo(MIDI) )
        {
            goto regular_paste;
        }

    }
    else
    {

regular_paste: // FIXME - find better way than goto
        /*
         * This part will change to value of 'shift' variable to make sure pasted notes will be visible
         */

        // if not "paste at mouse", find first visible measure
        int measure = getMeasureData()->measureAtTick(track->getSequence()->getXScrollInMidiTicks())-1;
        if (measure < 0) measure = 0;
        const int lastMeasureStart = getMeasureData()->firstTickInMeasure( measure );
        shift = track->graphics->keyboardEditor->snapMidiTickToGrid(lastMeasureStart);

        // find if all track->m_notes will be visible in the location just calculated,
        // otherwise move them one more measure ahead (if measure is half-visible because of scrolling)
        Note& first_note = *(Clipboard::getNote(0));

        // check if note is before visible area
        while ((first_note.startTick + first_note.endTick)/2 + shift <
               track->getSequence()->getXScrollInMidiTicks())
        {
            shift = getMeasureData()->firstTickInMeasure( getMeasureData()->measureAtTick( shift )+1 );
        }

    }

    // add new notes
    const int clipboardSize = Clipboard::getSize();
    for (int n=0; n<clipboardSize; n++)
    {
        Note* tmp = new Note( *(Clipboard::getNote(n)) );

        if (needToScalePastedNotes)
        {
            tmp->startTick *= scalePastedNotes;
            tmp->endTick   *= scalePastedNotes;
        }

        track->graphics->getCurrentEditor()->moveNote(*tmp, shift , 0);
        if (m_at_mouse)
        {
            track->graphics->getCurrentEditor()->moveNote(*tmp, -beginning , 0);
        }

        tmp->setParent( track );
        tmp->setSelected(true);

        if (track->graphics->getEditorMode() == GUITAR)
        {
            tmp->checkIfStringAndFretMatchNote(true);
        }

        track->addNote( tmp, false );
        relocator.rememberNote( *tmp );
    }//next

    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

// -------------------------------------------------------------------------------------------------------------

