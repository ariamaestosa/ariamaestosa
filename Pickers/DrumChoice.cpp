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


#include "Pickers/DrumChoice.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"
#include <iostream>

#include "Config.h"
#include "AriaCore.h"

namespace AriaMaestosa {

BEGIN_EVENT_TABLE(DrumChoice, wxMenu)

//EVT_MENU_RANGE(20000 + 0, 20000 + 127, DrumChoice::menuSelected)

END_EVENT_TABLE()


DrumChoice::DrumChoice() : wxMenu()
{


    Append( 20000 + 0 ,wxT("Standard"));
    Append( 20000 + 8 ,wxT("Room kit"));
    Append( 20000 + 16 ,wxT("Power kit"));
    Append( 20000 + 24 ,wxT("Electronic"));
    Append( 20000 + 25 ,wxT("Analog"));
    Append( 20000 + 32 ,wxT("Jazz"));
    Append( 20000 + 40 ,wxT("Brush"));
    Append( 20000 + 48 ,wxT("Orchestral"));
    Append( 20000 + 56 ,wxT("Special Effects"));

    DrumChoice::parent = parent;
}

DrumChoice::~DrumChoice()
{
}

void DrumChoice::menuSelected(wxCommandEvent& evt)
{

    int drumID=evt.GetId() - 20000;

    assertExpr(drumID,<,128);
    assertExpr(drumID,>=,0);

    parent->setDrumKit(drumID);

    Display::render();
}

void DrumChoice::setParent(Track* t)
{
    parent = t;
}

char* DrumChoice::getDrumName(int drumID)
{
    if (drumID == 0 ) return "Standard";
    else if (drumID == 8 ) return "Room kit";
    else if (drumID == 16 ) return "Power kit";
    else if (drumID == 24 ) return "Electronic";
    else if (drumID == 25 ) return "Analog";
    else if (drumID == 32 ) return "Jazz";
    else if (drumID == 40 ) return "Brush";
    else if (drumID == 48 ) return "Orchestral";
    else if (drumID == 56 ) return "Special Effects";
    else
    {
        std::cout << "wrong drumset ID: " << drumID << std::endl;
        Sequence* seq = getCurrentSequence();
        const int trackAmount = seq->getTrackAmount();
        for(int n=0; n<trackAmount; n++)
        {
        seq->getTrack(n)->setDrumKit(0);
        }
        return "Standard";
    }
}

}
