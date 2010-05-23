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

#include "Actions/MoveNotes.h"
#include "Actions/EditAction.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/Editor.h"

using namespace AriaMaestosa::Action;

MoveNotes::MoveNotes(const int relativeX, const int relativeY, const int noteID) :
    //I18N: (undoable) action name
    SingleTrackAction( _("move note(s)") )
{
    m_relativeX = relativeX;
    m_relativeY = relativeY;
    m_note_ID = noteID;

    m_move_mode = DELTA;
}

MoveNotes::~MoveNotes()
{
}

void MoveNotes::undo()
{
        Note* current_note;
        relocator.setParent(track);
        relocator.prepareToRelocate();

        int n = 0;
        while ((current_note = relocator.getNextNote()) and current_note != NULL)
        {
            if (m_move_mode == SCORE_VERTICAL or m_move_mode == DRUMS_VERTICAL)
            {
                current_note->pitchID = undo_pitch[n];
                track->graphics->getCurrentEditor()->moveNote(*current_note, -m_relativeX, 0);
                n++;
            }
            else if (m_move_mode == GUITAR_VERTICAL)
            {
                current_note->fret = undo_fret[n];
                current_note->string = undo_string[n];
                current_note->findNoteFromStringAndFret();
                track->graphics->getCurrentEditor()->moveNote(*current_note, -m_relativeX, 0);
                n++;
            }
            else
            {
                track->graphics->getCurrentEditor()->moveNote(*current_note, -m_relativeX, -m_relativeY);
            }
        }
        track->reorderNoteVector();
        track->reorderNoteOffVector();
}

void MoveNotes::perform()
{
    ASSERT(track != NULL);

    m_mode = track->graphics->editorMode;
    if      (m_mode == SCORE  and m_relativeY != 0) m_move_mode = SCORE_VERTICAL;
    else if (m_mode == GUITAR and m_relativeY != 0) m_move_mode = GUITAR_VERTICAL;
    else if (m_mode == DRUM   and m_relativeY != 0) m_move_mode = DRUMS_VERTICAL;

    // perform action
    ASSERT(m_note_ID != ALL_NOTES); // not supported in this function (not needed)

    if (m_note_ID == SELECTED_NOTES)
    {

        bool played = false;

        const int noteAmount=track->m_notes.size();
        for(int n=0; n<noteAmount; n++)
        {
            if (!track->m_notes[n].isSelected()) continue;

            doMoveOneNote(n);

            if (not played)
            {
                if (m_relativeY != 0) track->m_notes[n].play(true);
                else                  track->m_notes[n].play(false);
                played = true;
            }
        }//next
    }
    else
    {
        // move a single note
        ASSERT_E(m_note_ID,>=,0);
        ASSERT_E(m_note_ID,<,track->m_notes.size());

        doMoveOneNote(m_note_ID);

        if (m_relativeX != 0)
        {
            if (m_relativeY != 0) track->m_notes[m_note_ID].play(true);
            else                  track->m_notes[m_note_ID].play(false);
        }
    }

    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void MoveNotes::doMoveOneNote(const int noteID)
{
    /*
      In score and drum mode, it is necessary to remember the pitch of the note on vertical moves
         In score editor because sharp/flats are not accounted in the number of steps
         In drum editor because the black headers can introduce additional space vertically between notes
      In guitar editor, it is easier to remember the string and fret, as it's easy for notes to reach the bottom or top
         of the editor. When this happens, some notes moves while others don't. So the amount of steps isn't enough,
         we need to track the moves for all notes individually.
     */
    if (m_move_mode == SCORE_VERTICAL or m_move_mode == DRUMS_VERTICAL)
    {
        undo_pitch.push_back( track->m_notes[noteID].pitchID );
    }
    else if (m_move_mode == GUITAR_VERTICAL)
    {
        undo_fret.push_back( track->m_notes[noteID].fret );
        undo_string.push_back( track->m_notes[noteID].string );
    }

    track->graphics->getCurrentEditor()->moveNote(track->m_notes[noteID], m_relativeX, m_relativeY);
    relocator.rememberNote( track->m_notes[noteID] );
}



