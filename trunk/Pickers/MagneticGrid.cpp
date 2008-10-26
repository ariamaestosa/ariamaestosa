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

#include "Pickers/MagneticGrid.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include "AriaCore.h"

namespace AriaMaestosa {
	
BEGIN_EVENT_TABLE(MagneticGrid, wxMenu)

EVT_MENU(1, MagneticGrid::grid1selected)
EVT_MENU(2, MagneticGrid::grid2selected)
EVT_MENU(4, MagneticGrid::grid4selected)
EVT_MENU(8, MagneticGrid::grid8selected)
EVT_MENU(16, MagneticGrid::grid16selected)
EVT_MENU(32, MagneticGrid::grid32selected)
EVT_MENU(64, MagneticGrid::grid64selected)
EVT_MENU(128, MagneticGrid::grid128selected)

EVT_MENU(3, MagneticGrid::tripletChanged)

END_EVENT_TABLE()


MagneticGrid::MagneticGrid(GraphicalTrack* parent) : wxMenu()
{
	triplet= false;
	
    grid1 = AppendCheckItem(1,wxT("1/1"));
    grid2 = AppendCheckItem(2,wxT("1/2"));
    grid4 = AppendCheckItem(4,wxT("1/4"));
    grid8 = AppendCheckItem(8,wxT("1/8"));
    grid16 = AppendCheckItem(16,wxT("1/16"));
    grid32 = AppendCheckItem(32,wxT("1/32"));
    grid64 = AppendCheckItem(64,wxT("1/64"));
    grid128 = AppendCheckItem(128,wxT("1/128"));
    
	AppendSeparator();
	gridTriplet = AppendCheckItem(3,wxT("Triplet"));
	
    grid8->Check(true);
    
    divider=8;
    label= wxT("1/8");
    
    MagneticGrid::parent = parent;
}

MagneticGrid::~MagneticGrid()
{
}

void MagneticGrid::resetChecks()
{
    grid1->Check(false);
    grid2->Check(false);
    grid4->Check(false);
    grid8->Check(false);
    grid16->Check(false);
    grid32->Check(false);
    grid64->Check(false);
    grid128->Check(false);
	gridTriplet->Check(false);
}

void MagneticGrid::grid1selected(wxCommandEvent& evt)
{
    resetChecks();
    grid1->Check(true);
	if(triplet) gridTriplet->Check(true);
	
    divider=1;
    label=wxT("1/1");
	
    Display::render();
}


void MagneticGrid::grid2selected(wxCommandEvent& evt)
{
    resetChecks();
    grid2->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=2;
		label=wxT("1/2");
	}
	else
	{
		divider=3;
		label=wxT("1/2T");
	}
    Display::render();
}

void MagneticGrid::grid4selected(wxCommandEvent& evt)
{
    resetChecks();
    grid4->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=4;
		label=wxT("1/4");
	}
	else
	{
		divider=6;
		label=wxT("1/4T");
	}
    Display::render();
}

void MagneticGrid::grid8selected(wxCommandEvent& evt)
{
    resetChecks();
    grid8->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=8;
		label=wxT("1/8");
	}
	else
	{
		divider=12;
		label=wxT("1/8T");
	}
    Display::render();
}

void MagneticGrid::grid16selected(wxCommandEvent& evt)
{
    resetChecks();
    grid16->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=16;
		label=wxT("1/16");
	}
	else
	{
		divider=24;
		label=wxT("1/16T");
	}
    Display::render();
}

void MagneticGrid::grid32selected(wxCommandEvent& evt)
{
    resetChecks();
    grid32->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=32;
		label=wxT("1/32");
	}
	else
	{
		divider=48;
		label=wxT("1/32T");
	}
    Display::render();
}

void MagneticGrid::grid64selected(wxCommandEvent& evt)
{
    resetChecks();
    grid64->Check(true);
	if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=64;
		label=wxT("1/64");
	}
	else
	{
		divider=96;
		label=wxT("1/64T");
	}
    Display::render();
}

void MagneticGrid::grid128selected(wxCommandEvent& evt)
{
    resetChecks();
    grid128->Check(true);
    if(triplet) gridTriplet->Check(true);
	
	if(!triplet)
	{
		divider=128;
		label=wxT("1/128");
    }
	else
	{
		divider=192;
		label=wxT("1/128T");
	}
    Display::render();
}

void MagneticGrid::toggleTriplet()
{
    gridTriplet->Check( not gridTriplet->IsChecked() );
    wxCommandEvent useless;
    tripletChanged(useless);
}

/*
 * user selected or unselected 'triplet' in the combo box menu dropdown. update data accordignly
 */
void MagneticGrid::tripletChanged(wxCommandEvent& evt)
{
	triplet = gridTriplet->IsChecked();
	
	wxCommandEvent useless;
	
	// update divider and label by calling corresponding event method
	if(grid1->IsChecked()) grid1selected(useless);
    else if(grid2->IsChecked()) grid2selected(useless);
    else if(grid4->IsChecked()) grid4selected(useless);
    else if(grid8->IsChecked()) grid8selected(useless);
    else if(grid16->IsChecked()) grid16selected(useless);
    else if(grid32->IsChecked()) grid32selected(useless);
    else if(grid64->IsChecked()) grid64selected(useless);
    else if(grid128->IsChecked()) grid128selected(useless);
}

/*
 * Write the MagneticGrid part to a .aria file
 */

void MagneticGrid::saveToFile(wxFileOutputStream& fileout)
{
	
	writeData( wxT("<magneticgrid ") +
			   wxString(wxT("divider=\"")) + to_wxString(divider) +
			   wxT("\" triplet=\"") + wxString(triplet?wxT("true"):wxT("false")) +
			   wxT("\"/>\n"), fileout);
	
}

/*
 * Load the MagneticGrid part of a .aria file
 */

bool MagneticGrid::readFromFile(irr::io::IrrXMLReader* xml)
{
	
	const char* divider_c = xml->getAttributeValue("divider");
	const char* triplet_c = xml->getAttributeValue("triplet");

	if(divider_c != NULL)  divider = atoi( divider_c );
	else
	{
		std::cout << "Missing info from file: grid divider" << std::endl;
		divider = 8;	
	}
	
	if(triplet_c != NULL)
	{
		if(!strcmp(triplet_c, "true")) triplet = true;
		else if(!strcmp(triplet_c, "false")) triplet = false;
		else
		{
			std::cout << "Unknown keyword for attribute 'triplet' in magneticgrid: " << triplet_c << std::endl;
			triplet = false;
		}
	}
	else
	{
		std::cout << "Missing info from file: triplet" << std::endl;
		triplet = false;	
	}
	
    resetChecks();
	
	// check right menu item
	if(divider==1) grid1->Check(true);
	else if(divider==2 or divider==3) grid2->Check(true);
	else if(divider==4 or divider==6) grid4->Check(true);
	else if(divider==8 or divider==12) grid8->Check(true);
	else if(divider==16 or divider==24) grid16->Check(true);
	else if(divider==32 or divider==48) grid32->Check(true);
	else if(divider==64 or divider==96) grid64->Check(true);
    else if(divider==128 or divider==192) grid128->Check(true);
	
	// set right label to combo box
	if(divider==1) label=wxT("1/1");
	else if(divider==2) label=wxT("1/2");
	else if(divider==3) label=wxT("1/2T");
	else if(divider==4) label=wxT("1/4");
	else if(divider==6) label=wxT("1/4T");
	else if(divider==8) label=wxT("1/8");
	else if(divider==12) label=wxT("1/8T");
	else if(divider==16) label=wxT("1/16");
	else if(divider==24) label=wxT("1/16T");
	else if(divider==32) label=wxT("1/32");
	else if(divider==48) label=wxT("1/32T");
	else if(divider==64) label=wxT("1/64");
	else if(divider==96) label=wxT("1/64T");
	else if(divider==128) label=wxT("1/128");
	else if(divider==192) label=wxT("1/128T");
	
	// check triplet menu item if necessary
	if(triplet) gridTriplet->Check(true);
	
	return true;
}

}
