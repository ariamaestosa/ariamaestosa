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

#include "Utils.h"
#include "AriaCore.h"

namespace AriaMaestosa {

BEGIN_EVENT_TABLE(DrumChoice, wxMenu)

//EVT_MENU_RANGE(20000 + 0, 20000 + 127, DrumChoice::menuSelected)

END_EVENT_TABLE()


static const wxString g_drumkit_names[] =
{
    wxT("Standard"), // 0
    wxT("Room kit"), // 1
    wxT("Power kit"), // 2
    wxT("Electronic"), // 3
    wxT("Analog"), // 4
    wxT("Jazz"), // 5
    wxT("Brush"), // 6
    wxT("Orchestral"), // 7
    wxT("Special Effects"), // 8
};

DrumChoice::DrumChoice() : wxMenu(), drumkit_names_renderer(g_drumkit_names, 9)
{
    Append( 20000 + 0 ,  g_drumkit_names[0]);
    Append( 20000 + 8 ,  g_drumkit_names[1]);
    Append( 20000 + 16 , g_drumkit_names[2]);
    Append( 20000 + 24 , g_drumkit_names[3]);
    Append( 20000 + 25 , g_drumkit_names[4]);
    Append( 20000 + 32 , g_drumkit_names[5]);
    Append( 20000 + 40 , g_drumkit_names[6]);
    Append( 20000 + 48 , g_drumkit_names[7]);
    Append( 20000 + 56 , g_drumkit_names[8]);

#ifdef __WXMAC__
    drumkit_names_renderer.setFont( wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
    drumkit_names_renderer.setFont( wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#endif


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

void DrumChoice::renderDrumKitName(const int drumID, const int x, const int y)
{
    drumkit_names_renderer.bind();

    if (drumID == 0 )       drumkit_names_renderer.get(0).render(x, y);
    else if (drumID == 8 )  drumkit_names_renderer.get(1).render(x, y);
    else if (drumID == 16 ) drumkit_names_renderer.get(2).render(x, y);
    else if (drumID == 24 ) drumkit_names_renderer.get(3).render(x, y);
    else if (drumID == 25 ) drumkit_names_renderer.get(4).render(x, y);
    else if (drumID == 32 ) drumkit_names_renderer.get(5).render(x, y);
    else if (drumID == 40 ) drumkit_names_renderer.get(6).render(x, y);
    else if (drumID == 48 ) drumkit_names_renderer.get(7).render(x, y);
    else if (drumID == 56 ) drumkit_names_renderer.get(8).render(x, y);
    else
    {
        std::cerr << "wrong drumset ID: " << drumID << std::endl;
        Sequence* seq = getCurrentSequence();
        const int trackAmount = seq->getTrackAmount();
        for(int n=0; n<trackAmount; n++)
            seq->getTrack(n)->setDrumKit(0);
    }
}

const wxString& DrumChoice::getDrumName(int drumID) const
{
    if (drumID == 0 )       return g_drumkit_names[0];
    else if (drumID == 8 )  return g_drumkit_names[1];
    else if (drumID == 16 ) return g_drumkit_names[2];
    else if (drumID == 24 ) return g_drumkit_names[3];
    else if (drumID == 25 ) return g_drumkit_names[4];
    else if (drumID == 32 ) return g_drumkit_names[5];
    else if (drumID == 40 ) return g_drumkit_names[6];
    else if (drumID == 48 ) return g_drumkit_names[7];
    else if (drumID == 56 ) return g_drumkit_names[8];
    else
    {
        std::cerr << "DrumChoice::getDrumName: invalid drumkit ID : " << drumID << std::endl;
        static wxString dummy_answer = wxEmptyString; // should not be necessary, only there to shut up warnings
        return dummy_answer;
    }
}

}
