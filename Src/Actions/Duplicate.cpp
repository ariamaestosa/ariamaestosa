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
#include "Actions/Duplicate.h"
#include "Actions/EditAction.h"
#include "Actions/MoveNotes.h"

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

Duplicate::Duplicate(Editor* editor) :
    //I18N: (undoable) action name
    SingleTrackAction( _("duplicate") )
{
    m_editor = editor;
}

// -------------------------------------------------------------------------------------------------------------

Duplicate::~Duplicate()
{
}

// -------------------------------------------------------------------------------------------------------------

void Duplicate::undo()
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

void Duplicate::perform()
{
    ASSERT(m_track != NULL);
    ASSERT(m_editor != NULL);

    if (m_editor->getNotationType() == CONTROLLER)
    {
        wxBell();
        return; // no duplicate in controller mode
    }

    //GraphicalTrack* gtrack = m_track->getGraphics();
    //MeasureData* md = m_track->getSequence()->getMeasureData();
    
    ptr_vector<Note>& notes = m_visitor->getNotesVector();
    const int count = notes.size();
    
    std::vector<Note*> to_add;
    
    for (int n=0; n<count; n++)
    {
        if (notes[n].isSelected())
        {
            Note* tmp = new Note( notes[n] );
            to_add.push_back(tmp);
            notes[n].setSelected(false);
        }
    }
    for (unsigned int n=0; n<to_add.size(); n++)
    {
        m_track->addNote( to_add[n], false );
        relocator.rememberNote( to_add[n] );
        to_add[n]->setSelected(true);
    }

}

// -------------------------------------------------------------------------------------------------------------

void Duplicate::moveEvent(Action::MoveNotes* event)
{
    event->setParentTrack(m_track, new Track::TrackVisitor(*m_visitor.raw_ptr));
    event->perform();
    delete event;
}
