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

#include "AriaCore.h"
#include "Clipboard.h"
#include "Actions/Paste.h"
#include "Actions/EditAction.h"

// FIXME(DESIGN): actions shouldn't depend on GUI classes
#include "Editors/KeyboardEditor.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"

#include "Midi/MeasureData.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

#include <wx/intl.h>
#include <wx/utils.h>
#include <wx/window.h>

using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// -------------------------------------------------------------------------------------------------------------

Paste::Paste(Editor* editor, const bool atMouse) :
    //I18N: (undoable) action name
    SingleTrackAction( _("paste") )
{
    m_at_mouse = atMouse;
    m_editor = editor;
}

// -------------------------------------------------------------------------------------------------------------

Paste::~Paste()
{
}

// -------------------------------------------------------------------------------------------------------------

void Paste::undo()
{
    Note* current_note;
    relocator.setParent(m_track);
    relocator.prepareToRelocate();

    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        const int noteAmount = m_track->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            if (m_track->getNote(n) == current_note)
            {
                m_track->removeNote(n);
                break;
            }//endif
        }//next
    }//wend
}

// -------------------------------------------------------------------------------------------------------------

int Paste::getShiftForRegularPaste()
{
    GraphicalTrack* gtrack = m_track->getGraphics();

    int shift = 0;
    
    Sequence* sequence = m_track->getSequence();
    MeasureData* md = sequence->getMeasureData();
    
    // if not "paste at mouse", find first visible measure
    int measure = md->measureAtTick(gtrack->getSequence()->getXScrollInMidiTicks())-1;
    if (measure < 0) measure = 0;
    const int lastMeasureStart = md->firstTickInMeasure( measure );
    
    shift = m_track->snapMidiTickToGrid( lastMeasureStart );
    
    // find if all track->m_notes will be visible in the location just calculated,
    // otherwise move them one more measure ahead (if measure is half-visible because of scrolling)
    Note& first_note = *(Clipboard::getNote(0));
    
    // check if note is before visible area
    while ((first_note.getTick() + first_note.getEndTick())/2 + shift <
           gtrack->getSequence()->getXScrollInMidiTicks())
    {
        shift = md->firstTickInMeasure( md->measureAtTick( shift )+1 );
    }
    
    return shift;
}

// -------------------------------------------------------------------------------------------------------------

void Paste::perform()
{
    ASSERT(m_track != NULL);
    ASSERT(m_editor != NULL);

    if (m_editor->getNotationType() == CONTROLLER)
    {
        wxBell();
        return; // no copy/paste in controller mode
    }
    if (Clipboard::getSize() == 0) return; // nothing copied

    GraphicalTrack* gtrack = m_track->getGraphics();
    
    const int copiedBeatLength = Clipboard::getBeatLength();
    const int seqBeatLength = m_track->getSequence()->ticksPerBeat();
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
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    const int count = notes.size();
    for (int n=0; n<count; n++) notes[n].setSelected(false);

    // find where track->m_notes begin if necessary
    if (m_at_mouse)
    {
        beginning = Clipboard::getNote(0)->getTick();
    }
    int shift=0;

    // if "paste at mouse", use its location to know where to paste track->m_notes
    wxPoint mouseLoc = wxGetMousePosition();

    int trackMouseLoc_x, trackMouseLoc_y;

    Display::screenToClient(mouseLoc.x, mouseLoc.y, &trackMouseLoc_x, &trackMouseLoc_y);
    
    // Calculate 'shift' (i.e. X position of pasted notes)
    if (m_at_mouse and
        trackMouseLoc_y > m_editor->getEditorYStart() and
        trackMouseLoc_y < m_editor->getYEnd() and
        trackMouseLoc_x > Editor::getEditorXStart())
    {

        RelativeXCoord mx(trackMouseLoc_x, WINDOW, gtrack->getSequence());

        shift = m_track->snapMidiTickToGrid( mx.getRelativeTo(MIDI) );

    }
    else if (not m_at_mouse and
             gtrack->getSequence()->x_scroll_upon_copying == gtrack->getSequence()->getXScrollInPixels() )
    {

        /*
         * scrolling has not changed since copy (i.e. user probably hit 'copy' and 'paste' consecutively)
         * in this case, we want the notes to be pasted exactly where they were copied so we just
         * overwrite the shift variable to make track->m_notes paste where they were copied. after this,
         * check if all track->m_notes will be visible when pasting like this. If not, fall back to regular
         * pasting method that will take care of finding track->m_notes an appropriate location to be
         * pasted onto.
         */
        shift = m_track->getSequence()->getNoteShiftWhenNoScrolling();

        // find if first note will be visible in the location just calculated,
        // otherwise just go to regular pasting code, it will paste them within visible measures
        Note& tmp = *Clipboard::getNote(0);

        // before visible area
        if ((tmp.getTick() + tmp.getEndTick())/2 + shift < gtrack->getSequence()->getXScrollInMidiTicks())
        {
            shift = getShiftForRegularPaste();
        }

        // after visible area
        RelativeXCoord screen_width( Display::getWidth(), WINDOW, gtrack->getSequence() );

        if ((tmp.getTick() + tmp.getEndTick())/2 + shift > screen_width.getRelativeTo(MIDI) )
        {
            shift = getShiftForRegularPaste();
        }

    }
    else
    {
        shift = getShiftForRegularPaste();
    }

    // ---- add new notes
    const int clipboardSize = Clipboard::getSize();
    for (int n=0; n<clipboardSize; n++)
    {
        Note* tmp = new Note( *(Clipboard::getNote(n)) );

        if (needToScalePastedNotes)
        {
            tmp->setTick(tmp->getTick() * scalePastedNotes);
            tmp->setEndTick(tmp->getEndTick() * scalePastedNotes);
        }

        m_editor->moveNote(*tmp, shift , 0);
        if (m_at_mouse)
        {
            m_editor->moveNote(*tmp, -beginning , 0);
        }

        tmp->setParent( m_track );
        tmp->setSelected(true);

        if (m_editor->getNotationType() == GUITAR)
        {
            tmp->checkIfStringAndFretMatchNote(true);
        }

        m_track->addNote( tmp, false );
        relocator.rememberNote( *tmp );
    }//next

    m_track->reorderNoteVector();
    m_track->reorderNoteOffVector();
}

// -------------------------------------------------------------------------------------------------------------

