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


#include "Pickers/DrumPicker.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"
#include <iostream>

#include "Utils.h"
#include "AriaCore.h"

using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(DrumPicker, wxMenu)

//EVT_MENU_RANGE(20000 + 0, 20000 + 127, DrumPicker::menuSelected)

END_EVENT_TABLE()


DrumPicker::DrumPicker() : wxMenu()
{
    m_model = NULL;
    
    // FIXME: don't hardcode the choices, get them from the model
    
    Append( 20000 + 0 ,  DrumChoice::getDrumkitName(0));
    Append( 20000 + 8 ,  DrumChoice::getDrumkitName(8));
    Append( 20000 + 16 , DrumChoice::getDrumkitName(16));
    Append( 20000 + 24 , DrumChoice::getDrumkitName(24));
    Append( 20000 + 25 , DrumChoice::getDrumkitName(25));
    Append( 20000 + 32 , DrumChoice::getDrumkitName(32));
    Append( 20000 + 40 , DrumChoice::getDrumkitName(40));
    Append( 20000 + 48 , DrumChoice::getDrumkitName(48));
    Append( 20000 + 56 , DrumChoice::getDrumkitName(56));
}

DrumPicker::~DrumPicker()
{
}

void DrumPicker::menuSelected(wxCommandEvent& evt)
{
    int drumID = evt.GetId() - 20000;

    ASSERT_E(drumID,<,128);
    ASSERT_E(drumID,>=,0);

    m_model->setDrumkit(drumID, true);

    Display::render();
}

void DrumPicker::setModel(DrumChoice* model)
{
    m_model = model;
}
