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

#include "Actions/AddTrack.h"
#include "Actions/EditAction.h"
#include "AriaCore.h"

// FIXME(DESIGN) : actions shouldn't rely on GUI classes
#include "GUI/GraphicalSequence.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "UnitTest.h"
#include "UnitTestUtils.h"

#include <wx/intl.h>


using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

// FIXME(DESIGN): refers to 'GraphicalSequence', how to make this action independent of GUI while still making GUI follow along...
AddTrack::AddTrack(GraphicalSequence* seq) :
    //I18N: (undoable) action name
    MultiTrackAction( _("add track") )
{
    m_added_track     = NULL;
    m_parent_sequence = seq;
}

// --------------------------------------------------------------------------------------------------------

AddTrack::~AddTrack()
{
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::undo()
{
    ASSERT(m_added_track != NULL)
    sequence->deleteTrack(m_added_track);
    m_added_track = NULL;
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::perform()
{
    m_added_track = m_parent_sequence->addTrack();
}

// --------------------------------------------------------------------------------------------------------

// TODO: add back unit test when GraphicalSequence is not required anymore
/*
namespace TestAddTrack
{
    using namespace AriaMaestosa;
    
    UNIT_TEST( TestAction )
    {
        Sequence* seq = new Sequence(NULL, NULL, NULL, false);
        
        TestSequenceProvider provider(seq);
        AriaMaestosa::setCurrentSequenceProvider(&provider);
        
        Track* t = new Track(seq);
        t->setName(wxT("TestTrack"));
        seq->addTrack(t);
        
        t = new Track(seq);
        t->setName(wxT("TestTrack2"));
        seq->addTrack(t);
        
        // test the action
        seq->action(new AddTrack(seq));
        
        require(seq->getTrackAmount() == 3, "track amount increased after performing action");
        bool foundFirst = false, foundSecond = false, foundNew = false;
        for (int n=0; n<3; n++)
        {
            if (seq->getTrack(n)->getName() == wxT("TestTrack")) foundFirst  = true;
            else if (seq->getTrack(n)->getName() == wxT("TestTrack2")) foundSecond = true;
            else foundNew = true;
        }
        
        require(foundFirst and foundSecond and foundNew, "The right tracks are found in the sequence");
        
        // test undo
        seq->undo();
        
        require(seq->getTrackAmount() == 2, "track amount decreased after undoing action");
        
        foundFirst = false; foundSecond = false; foundNew = false;
        for (int n=0; n<2; n++)
        {
            if (seq->getTrack(n)->getName() == wxT("TestTrack")) foundFirst  = true;
            else if (seq->getTrack(n)->getName() == wxT("TestTrack2")) foundSecond = true;
            else foundNew = true;
        }
        require(foundFirst and foundSecond and not foundNew, "The right tracks are found in the sequence after undoing");
        
        delete seq;
    }
}
*/
