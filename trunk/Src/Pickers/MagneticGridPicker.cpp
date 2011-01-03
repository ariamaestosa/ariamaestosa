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

#include "Pickers/MagneticGridPicker.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MagneticGrid.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include "AriaCore.h"
#include "irrXML/irrXML.h"

namespace AriaMaestosa
{

    BEGIN_EVENT_TABLE(MagneticGridPicker, wxMenu)

    EVT_MENU(1, MagneticGridPicker::grid1selected)
    EVT_MENU(2, MagneticGridPicker::grid2selected)
    EVT_MENU(4, MagneticGridPicker::grid4selected)
    EVT_MENU(8, MagneticGridPicker::grid8selected)
    EVT_MENU(16, MagneticGridPicker::grid16selected)
    EVT_MENU(32, MagneticGridPicker::grid32selected)
    EVT_MENU(64, MagneticGridPicker::grid64selected)
    EVT_MENU(128, MagneticGridPicker::grid128selected)

    EVT_MENU(3, MagneticGridPicker::tripletChanged)

    END_EVENT_TABLE()

}

using namespace AriaMaestosa;

MagneticGridPicker::MagneticGridPicker(GraphicalTrack* parent, MagneticGrid* model) : wxMenu()
{
    m_model = model;

    grid1   = AppendCheckItem(1,wxT("1/1"));
    grid2   = AppendCheckItem(2,wxT("1/2"));
    grid4   = AppendCheckItem(4,wxT("1/4"));
    grid8   = AppendCheckItem(8,wxT("1/8"));
    grid16  = AppendCheckItem(16,wxT("1/16"));
    grid32  = AppendCheckItem(32,wxT("1/32"));
    grid64  = AppendCheckItem(64,wxT("1/64"));
    grid128 = AppendCheckItem(128,wxT("1/128"));

    AppendSeparator();
    gridTriplet = AppendCheckItem(3,wxT("Triplet"));

    grid8->Check(true);

    m_parent = parent;
}

// ----------------------------------------------------------------------------------------------------------

MagneticGridPicker::~MagneticGridPicker()
{
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::resetChecks()
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

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid1selected(wxCommandEvent& evt)
{
    resetChecks();
    grid1->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    m_model->setDivider(1);

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid2selected(wxCommandEvent& evt)
{
    resetChecks();
    grid2->Check(true);
    gridTriplet->Check( m_model->isTriplet() );
    
    if (not m_model->isTriplet()) m_model->setDivider(2);
    else                          m_model->setDivider(3);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid4selected(wxCommandEvent& evt)
{
    resetChecks();
    grid4->Check(true);
    gridTriplet->Check(m_model->isTriplet());

    if (not m_model->isTriplet()) m_model->setDivider(4);
    else                          m_model->setDivider(6);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid8selected(wxCommandEvent& evt)
{
    resetChecks();
    grid8->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    if (not m_model->isTriplet()) m_model->setDivider(8);
    else                          m_model->setDivider(12);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid16selected(wxCommandEvent& evt)
{
    resetChecks();
    grid16->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    if (not m_model->isTriplet()) m_model->setDivider(16);
    else                          m_model->setDivider(24);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid32selected(wxCommandEvent& evt)
{
    resetChecks();
    grid32->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    if (not m_model->isTriplet()) m_model->setDivider(32);
    else                          m_model->setDivider(48);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid64selected(wxCommandEvent& evt)
{
    resetChecks();
    grid64->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    if (not m_model->isTriplet()) m_model->setDivider(64);
    else                          m_model->setDivider(96);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid128selected(wxCommandEvent& evt)
{
    resetChecks();
    grid128->Check(true);
    gridTriplet->Check( m_model->isTriplet() );

    if (not m_model->isTriplet()) m_model->setDivider(128);
    else                          m_model->setDivider(192);
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::toggleTriplet()
{
    gridTriplet->Check( not gridTriplet->IsChecked() );
    wxCommandEvent useless;
    tripletChanged(useless);
}

// ----------------------------------------------------------------------------------------------------------

/*
 * user selected or unselected 'triplet' in the combo box menu dropdown. update data accordignly
 */
void MagneticGridPicker::tripletChanged(wxCommandEvent& evt)
{
    m_model->setTriplet( gridTriplet->IsChecked() );

    wxCommandEvent useless;

    // update divider and label by calling corresponding event method
    if      (grid1->IsChecked())   grid1selected  (useless);
    else if (grid2->IsChecked())   grid2selected  (useless);
    else if (grid4->IsChecked())   grid4selected  (useless);
    else if (grid8->IsChecked())   grid8selected  (useless);
    else if (grid16->IsChecked())  grid16selected (useless);
    else if (grid32->IsChecked())  grid32selected (useless);
    else if (grid64->IsChecked())  grid64selected (useless);
    else if (grid128->IsChecked()) grid128selected(useless);
}


/*

switch (m_divider)
{
    case 1:
        grid1->Check(true);
        break;
        
    case 2:
    case 3:
        grid2->Check(true);
        break;
        
    case 4:
    case 6:
        grid4->Check(true);
        break;
        
    case 8:
    case 12:
        grid8->Check(true);
        break;
        
    case 16:
    case 24:
        grid16->Check(true);
        break;
        
    case 32:
    case 48:
        grid32->Check(true);
        break;
        
    case 64:
    case 96:
        grid64->Check(true);
        break;
        
    case 128:
    case 192:
        grid128->Check(true);
        break;
        
    default:
        std::cerr << "[MagneticGrid] Invalid divider : " << m_divider << std::endl;
        }
*/        

