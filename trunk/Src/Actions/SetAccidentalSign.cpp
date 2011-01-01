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
#include "Midi/Track.h"
//#include "Editors/Editor.h"

// FIXME(DESIGN): actions shouldn't rely on GUI classes
#include "GUI/GraphicalTrack.h"
#include "Editors/ScoreEditor.h"

#include <wx/intl.h>

using namespace AriaMaestosa::Action;

// ----------------------------------------------------------------------------------------------------------

SetAccidentalSign::SetAccidentalSign(const int sign) :
    //I18N: (undoable) action name
    SingleTrackAction( _("accidental change") )
{
    m_sign = sign;
}

// ----------------------------------------------------------------------------------------------------------

SetAccidentalSign::~SetAccidentalSign()
{
}

// ----------------------------------------------------------------------------------------------------------

void SetAccidentalSign::undo()
{
    Note* current_note;
    relocator.setParent(track);
    relocator.prepareToRelocate();

    int n = 0;
    while ((current_note = relocator.getNextNote()) and current_note != NULL)
    {
        current_note->setPreferredAccidentalSign( m_original_signs[n] );
        current_note->setPitchID( m_pitch[n] );
        n++;
    }
}

// ----------------------------------------------------------------------------------------------------------

void SetAccidentalSign::perform()
{

    bool played = false;

    const int noteAmount=track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {
        if (not track->isNoteSelected(n)) continue;

        m_original_signs.push_back( track->m_notes[n].getPreferredAccidentalSign() );
        m_pitch.push_back( track->m_notes[n].getPitchID() );

        track->getGraphics()->getScoreEditor()->setNoteSign(m_sign, n);
        relocator.rememberNote( track->m_notes[n] );

        if (not played)
        {
            track->m_notes[n].play(true);
            played = true;
        }
    }//next

}

// ----------------------------------------------------------------------------------------------------------

