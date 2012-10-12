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

#include "Actions/RearrangeVertically.h"
#include "AriaCore.h"

#include "GUI/GraphicalTrack.h"
#include "Editors/KeyboardEditor.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "UnitTest.h"
#include "UnitTestUtils.h"

#include <wx/intl.h>


using namespace AriaMaestosa;
using namespace AriaMaestosa::Action;

// --------------------------------------------------------------------------------------------------------

RearrangeVertically::RearrangeVertically() :
    //I18N: (undoable) action name
    MultiTrackAction( _("Scroll notes into view") )
{
    
}

// --------------------------------------------------------------------------------------------------------

RearrangeVertically::~RearrangeVertically()
{
}

// --------------------------------------------------------------------------------------------------------

void RearrangeVertically::undo()
{
    int count;
    
    count = m_positions.size();
    for (int n=0 ; n<count ; n++)
    {
        m_sequence->getTrack(n)->getGraphics()->getKeyboardEditor()->setScrollbarPosition(m_positions[n]);
    }
}

// --------------------------------------------------------------------------------------------------------

void RearrangeVertically::perform()
{
    KeyboardEditor* keyboardEditor;
    Track* track;
    int trackCount;
    int noteCount;
    float sbPosition;
    
    wxBeginBusyCursor();
    
    trackCount = m_sequence->getTrackAmount();
    m_positions.clear();
    
    for (int i=0 ; i<trackCount ; i++)
    {
        track = m_sequence->getTrack(i);
        keyboardEditor = track->getGraphics()->getKeyboardEditor();
        
        // Adds current position in array
        sbPosition = keyboardEditor->getScrollbarPosition();
        m_positions.push_back(sbPosition);
        
        // Computes maximum pitch in track (the higher MIDI pitch, 
        // the lower the value returned by Track::getNotePitchID()
        noteCount = track->getNoteAmount();
        if (noteCount > 0)
        {
            double maxNotePitch = 0;
            for (int j=0 ; j<noteCount ; j++)
            {
                maxNotePitch += track->getNotePitchID(j);
            }
            int averageNotePitch = (int)(maxNotePitch / (double)noteCount);
            
            int localY = keyboardEditor->levelToLocalY(std::max(0, averageNotePitch - keyboardEditor->levelsInView()/2));
            keyboardEditor->setYScrollInPixels(std::max(0, localY - 5));
        }
    }

    wxEndBusyCursor();
    
    Display::render();
}
