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

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "UnitTest.h"
#include "UnitTestUtils.h"

#include <wx/intl.h>


using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

AddTrack::AddTrack(Track* model) :
    //I18N: (undoable) action name
    MultiTrackAction( _("add track") )
{
    m_added_track = NULL;
    m_model       = model;
}

// --------------------------------------------------------------------------------------------------------

AddTrack::~AddTrack()
{
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::undo()
{
    ASSERT(m_added_track != NULL)
    m_sequence->deleteTrack(m_added_track);
    m_added_track = NULL;
}

// --------------------------------------------------------------------------------------------------------

void AddTrack::perform()
{
    m_added_track = m_sequence->addTrack();
    
    if (m_model != NULL)
    {
        m_added_track->setMuted(m_model->isMuted());
        m_added_track->setInstrument(m_model->getInstrument());
        m_added_track->setDrumKit(m_model->getDrumKit());
        
        m_added_track->setNotationType(SCORE,      m_model->isNotationTypeEnabled(SCORE));
        m_added_track->setNotationType(KEYBOARD,   m_model->isNotationTypeEnabled(KEYBOARD));
        m_added_track->setNotationType(GUITAR,     m_model->isNotationTypeEnabled(GUITAR));
        m_added_track->setNotationType(DRUM,       m_model->isNotationTypeEnabled(DRUM));
        m_added_track->setNotationType(CONTROLLER, m_model->isNotationTypeEnabled(CONTROLLER));

        m_added_track->setName(m_model->getName());
        
        KeyType key = m_model->getKeyType();
        switch (key)
        {
            case KEY_TYPE_SHARPS:
                m_added_track->setKey(m_model->getKeySharpsAmount(), KEY_TYPE_SHARPS);
                break;
                
            case KEY_TYPE_FLATS:
                m_added_track->setKey(m_model->getKeyFlatsAmount(), KEY_TYPE_FLATS);
                break;
                
            case KEY_TYPE_C:
                m_added_track->setKey(0, KEY_TYPE_C);
                break;
                
            case KEY_TYPE_CUSTOM:
                break;
        }
        
        
        m_added_track->getMagneticGrid()->setDivider( m_model->getMagneticGrid()->getDivider() );
        m_added_track->getMagneticGrid()->setDotted( m_model->getMagneticGrid()->isDotted() );
        m_added_track->getMagneticGrid()->setTriplet( m_model->getMagneticGrid()->isTriplet() );
        m_added_track->setDefaultVolume( m_model->getDefaultVolume() );
    }
}

// --------------------------------------------------------------------------------------------------------

namespace TestAddTrack
{
    using namespace AriaMaestosa;
    
    UNIT_TEST( TestAction )
    {
        Sequence* seq = new Sequence(NULL, NULL, NULL, NULL, false);
        
        TestSequenceProvider provider(seq);
        AriaMaestosa::setCurrentSequenceProvider(&provider);
        
        Track* t = new Track(seq);
        t->setName(wxT("TestTrack"));
        seq->addTrack(t);
        
        t = new Track(seq);
        t->setName(wxT("TestTrack2"));
        seq->addTrack(t);
        
        // test the action
        seq->action(new AddTrack());
        
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

