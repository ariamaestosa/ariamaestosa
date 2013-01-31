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

using namespace AriaMaestosa;

MagneticGridPicker::MagneticGridPicker(GraphicalTrack* parent, MagneticGrid* model) : wxMenu()
{
    m_model = model;
    m_parent = parent;
    
    grid1   = AppendCheckItem(wxID_ANY, wxT("1/1\tCtrl+1"));
    grid2   = AppendCheckItem(wxID_ANY, wxT("1/2\tCtrl+2"));
    grid4   = AppendCheckItem(wxID_ANY, wxT("1/4\tCtrl+3"));
    grid8   = AppendCheckItem(wxID_ANY, wxT("1/8\tCtrl+4"));
    grid16  = AppendCheckItem(wxID_ANY, wxT("1/16\tCtrl+5"));
    grid32  = AppendCheckItem(wxID_ANY, wxT("1/32\tCtrl+6"));
    grid64  = AppendCheckItem(wxID_ANY, wxT("1/64\tCtrl+7"));
    grid128 = AppendCheckItem(wxID_ANY, wxT("1/128\tCtrl+8"));

    AppendSeparator();
    gridTriplet = AppendCheckItem(wxID_ANY, _("Triplet"));
    gridDotted  = AppendCheckItem(wxID_ANY, _("Dotted"));
    gridTriplet->Check(m_model->isTriplet());
    gridDotted->Check(m_model->isDotted());
    
    grid8->Check(true);

    Connect(grid1->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid1selected), NULL, this);
    Connect(grid2->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid2selected), NULL, this);
    Connect(grid4->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid4selected), NULL, this);
    Connect(grid8->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid8selected), NULL, this);
    Connect(grid16->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid16selected), NULL, this);
    Connect(grid32->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid32selected), NULL, this);
    Connect(grid64->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid64selected), NULL, this);
    Connect(grid128->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::grid128selected), NULL, this);
    
    Connect(gridTriplet->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::tripletChanged), NULL, this);
    Connect(gridDotted->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MagneticGridPicker::dottedChanged), NULL, this);
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
    gridTriplet->Check(m_model->isTriplet());
    gridDotted->Check(m_model->isDotted());
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::syncWithModel()
{
    gridTriplet->Check(m_model->isTriplet());
    gridDotted->Check(m_model->isDotted());
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid1selected(wxCommandEvent& evt)
{
    resetChecks();
    grid1->Check(true);
    
    m_model->setDivider(1);

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid2selected(wxCommandEvent& evt)
{
    resetChecks();
    grid2->Check(true);
    
    if (not m_model->isTriplet()) m_model->setDivider(2);
    else                          m_model->setDivider(3);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid4selected(wxCommandEvent& evt)
{
    resetChecks();
    grid4->Check(true);

    if (not m_model->isTriplet()) m_model->setDivider(4);
    else                          m_model->setDivider(6);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid8selected(wxCommandEvent& evt)
{
    resetChecks();
    grid8->Check(true);
    
    if (not m_model->isTriplet()) m_model->setDivider(8);
    else                          m_model->setDivider(12);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid16selected(wxCommandEvent& evt)
{
    resetChecks();
    grid16->Check(true);

    if (not m_model->isTriplet()) m_model->setDivider(16);
    else                          m_model->setDivider(24);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid32selected(wxCommandEvent& evt)
{
    resetChecks();
    grid32->Check(true);

    if (not m_model->isTriplet()) m_model->setDivider(32);
    else                          m_model->setDivider(48);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid64selected(wxCommandEvent& evt)
{
    resetChecks();
    grid64->Check(true);

    if (not m_model->isTriplet()) m_model->setDivider(64);
    else                          m_model->setDivider(96);
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGridPicker::grid128selected(wxCommandEvent& evt)
{
    resetChecks();
    grid128->Check(true);

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

void MagneticGridPicker::toggleDotted()
{
    gridDotted->Check( not gridDotted->IsChecked() );
    wxCommandEvent useless;
    dottedChanged(useless);
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

// ----------------------------------------------------------------------------------------------------------

/*
 * user selected or unselected 'triplet' in the combo box menu dropdown. update data accordignly
 */
void MagneticGridPicker::dottedChanged(wxCommandEvent& evt)
{
    m_model->setDotted( gridDotted->IsChecked() );
    
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
  

