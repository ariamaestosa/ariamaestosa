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
    MoveNotes::relativeX = relativeX;
    MoveNotes::relativeY = relativeY;
    MoveNotes::noteID = noteID;

    move_mode = DELTA;
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
        while( (current_note = relocator.getNextNote()) and current_note != NULL)
        {
            if ( move_mode == SCORE_VERTICAL or move_mode == DRUMS_VERTICAL )
            {
                current_note->pitchID = undo_pitch[n];
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, 0);
                n++;
            }
            else if ( move_mode == GUITAR_VERTICAL )
            {
                current_note->fret = undo_fret[n];
                current_note->string = undo_string[n];
                current_note->findNoteFromStringAndFret();
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, 0);
                n++;
            }
            else
                track->graphics->getCurrentEditor()->moveNote(*current_note, -relativeX, -relativeY);
        }
        track->reorderNoteVector();
        track->reorderNoteOffVector();
}
void MoveNotes::perform()
{
    assert(track != NULL);

    mode = track->graphics->editorMode;
    if (mode == SCORE and relativeY != 0) move_mode = SCORE_VERTICAL;
    else if (mode == GUITAR and relativeY != 0) move_mode = GUITAR_VERTICAL;
    else if (mode == DRUM and relativeY != 0) move_mode = DRUMS_VERTICAL;

    // perform action
    assert(noteID != ALL_NOTES); // not supported in this function (not needed)

    if (noteID==SELECTED_NOTES)
    {

        bool played = false;

        const int noteAmount=track->m_notes.size();
        for(int n=0; n<noteAmount; n++)
        {
            if (!track->m_notes[n].isSelected()) continue;

            doMoveOneNote(n);

            if (!played)
            {
                if (relativeY != 0) track->m_notes[n].play(true);
                else track->m_notes[n].play(false);
                played = true;
            }
        }//next
    }
    else
    {
        // move a single note
        assertExpr(noteID,>=,0);
        assertExpr(noteID,<,track->m_notes.size());

        doMoveOneNote(noteID);

        if (relativeX != 0)
        {
            if (relativeY != 0) track->m_notes[noteID].play(true);
            else track->m_notes[noteID].play(false);
        }
    }

    track->reorderNoteVector();
    track->reorderNoteOffVector();
}

void MoveNotes::doMoveOneNote(const int noteid)
{
    /*
      In score and drum mode, it is necessary to remember the pitch of the note on vertical moves
         In score editor because sharp/flats are not accounted in the number of steps
         In drum editor because the black headers can introduce additional space vertically between notes
      In guitar editor, it is easier to remember the string and fret, as it's easy for notes to reach the bottom or top
         of the editor. When this happens, some notes moves while others don't. So the amount of steps isn't enough,
         we need to track the moves for all notes individually.
     */
    if ( move_mode == SCORE_VERTICAL or move_mode == DRUMS_VERTICAL ) undo_pitch.push_back( track->m_notes[noteid].pitchID );
    else if ( move_mode == GUITAR_VERTICAL )
    {
        undo_fret.push_back( track->m_notes[noteid].fret );
        undo_string.push_back( track->m_notes[noteid].string );
    }

    track->graphics->getCurrentEditor()->moveNote(track->m_notes[noteid], relativeX, relativeY);
    relocator.rememberNote( track->m_notes[noteid] );
}



