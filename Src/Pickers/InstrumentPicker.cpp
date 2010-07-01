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


#include "Pickers/InstrumentPicker.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"
#include <iostream>

#include "AriaCore.h"
#include "Utils.h"

namespace AriaMaestosa {

BEGIN_EVENT_TABLE(InstrumentPicker, wxMenu)

//EVT_MENU_RANGE(0,127,InstrumentPicker::menuSelected)

END_EVENT_TABLE()

void InstrumentPicker::setParent(InstrumentChoice* parent)
{
    m_model = parent;
}

    
InstrumentPicker::InstrumentPicker() : wxMenu()//, inst_names_renderer(g_inst_names, 128)
{
    m_model = NULL;
    
    submenu_1_piano       = new wxMenu();
    submenu_2_chromatic   = new wxMenu();
    submenu_3_organ       = new wxMenu();
    submenu_4_guitar      = new wxMenu();
    submenu_5_bass        = new wxMenu();
    submenu_6_orchestra   = new wxMenu();
    submenu_7_choirs_pads = new wxMenu();
    submenu_8_brass       = new wxMenu();
    submenu_9_reed        = new wxMenu();
    submenu_10_pipe       = new wxMenu();
    submenu_11_synths     = new wxMenu();
    submenu_12_ethnic     = new wxMenu();
    submenu_13_percussion = new wxMenu();
    submenu_14_sound_fx   = new wxMenu();
    submenu_15_drums      = new wxMenu();

    // TODO: get the structure from the 'InstrumentChoice' model instead of hardcoding it there
#define _ADD_INSTR(id, parent) inst_menus[ id ]= parent -> Append( id+10000, InstrumentChoice::getInstrumentName( id ));

    Append(wxID_ANY,wxT("Piano"), submenu_1_piano);
    _ADD_INSTR(0, submenu_1_piano);
    _ADD_INSTR(1, submenu_1_piano);
    _ADD_INSTR(2, submenu_1_piano);
    _ADD_INSTR(3, submenu_1_piano);
    _ADD_INSTR(4, submenu_1_piano);
    _ADD_INSTR(5, submenu_1_piano);
    _ADD_INSTR(6, submenu_1_piano);
    _ADD_INSTR(7, submenu_1_piano);

    Append(wxID_ANY,wxT("Chromatic"), submenu_2_chromatic);
    _ADD_INSTR(8, submenu_2_chromatic);
    _ADD_INSTR(9, submenu_2_chromatic);
    _ADD_INSTR(10, submenu_2_chromatic);
    _ADD_INSTR(11, submenu_2_chromatic);
    _ADD_INSTR(12, submenu_2_chromatic);
    _ADD_INSTR(13, submenu_2_chromatic);
    _ADD_INSTR(14, submenu_2_chromatic);
    _ADD_INSTR(15, submenu_2_chromatic);

    Append(wxID_ANY,wxT("Organ"), submenu_3_organ);
    _ADD_INSTR(16, submenu_3_organ);
    _ADD_INSTR(17, submenu_3_organ);
    _ADD_INSTR(18, submenu_3_organ);
    _ADD_INSTR(19, submenu_3_organ);
    _ADD_INSTR(20, submenu_3_organ);
    _ADD_INSTR(21, submenu_3_organ);
    _ADD_INSTR(22, submenu_3_organ);
    _ADD_INSTR(23, submenu_3_organ);

    Append(wxID_ANY,wxT("Guitar"), submenu_4_guitar);
    _ADD_INSTR(24, submenu_4_guitar);
    _ADD_INSTR(25, submenu_4_guitar);
    _ADD_INSTR(26, submenu_4_guitar);
    _ADD_INSTR(27, submenu_4_guitar);
    _ADD_INSTR(28, submenu_4_guitar);
    _ADD_INSTR(29, submenu_4_guitar);
    _ADD_INSTR(30, submenu_4_guitar);
    _ADD_INSTR(31, submenu_4_guitar);
    _ADD_INSTR(120, submenu_4_guitar);

    Append(wxID_ANY,wxT("Bass"), submenu_5_bass);
    _ADD_INSTR(32, submenu_5_bass);
    _ADD_INSTR(33, submenu_5_bass);
    _ADD_INSTR(34, submenu_5_bass);
    _ADD_INSTR(35, submenu_5_bass);
    _ADD_INSTR(36, submenu_5_bass);
    _ADD_INSTR(37, submenu_5_bass);
    _ADD_INSTR(38, submenu_5_bass);
    _ADD_INSTR(39, submenu_5_bass);

    Append(wxID_ANY,wxT("Strings and Orchestra"), submenu_6_orchestra);
    _ADD_INSTR(40, submenu_6_orchestra);
    _ADD_INSTR(41, submenu_6_orchestra);
    _ADD_INSTR(42, submenu_6_orchestra);
    _ADD_INSTR(43, submenu_6_orchestra);
    _ADD_INSTR(44, submenu_6_orchestra);
    _ADD_INSTR(45, submenu_6_orchestra);
    _ADD_INSTR(46, submenu_6_orchestra);
    _ADD_INSTR(47, submenu_6_orchestra);
    _ADD_INSTR(48, submenu_6_orchestra);
    _ADD_INSTR(49, submenu_6_orchestra);
    _ADD_INSTR(50, submenu_6_orchestra);
    _ADD_INSTR(51, submenu_6_orchestra);
    _ADD_INSTR(55, submenu_6_orchestra);

    Append(wxID_ANY,wxT("Choirs, Pads and Voices"), submenu_7_choirs_pads);
    _ADD_INSTR(52, submenu_7_choirs_pads);
    _ADD_INSTR(53, submenu_7_choirs_pads);
    _ADD_INSTR(89, submenu_7_choirs_pads);
    _ADD_INSTR(91, submenu_7_choirs_pads);
    _ADD_INSTR(94, submenu_7_choirs_pads);
    _ADD_INSTR(92, submenu_7_choirs_pads);
    _ADD_INSTR(95, submenu_7_choirs_pads);
    _ADD_INSTR(54, submenu_7_choirs_pads);
    _ADD_INSTR(85, submenu_7_choirs_pads);
    _ADD_INSTR(98, submenu_7_choirs_pads);
    _ADD_INSTR(100, submenu_7_choirs_pads);
    _ADD_INSTR(101, submenu_7_choirs_pads);
    _ADD_INSTR(102, submenu_7_choirs_pads);
    _ADD_INSTR(93, submenu_7_choirs_pads);
    _ADD_INSTR(88, submenu_7_choirs_pads);
    _ADD_INSTR(96, submenu_7_choirs_pads);

    Append(wxID_ANY,wxT("Brass"), submenu_8_brass);
    _ADD_INSTR(56, submenu_8_brass);
    _ADD_INSTR(57, submenu_8_brass);
    _ADD_INSTR(58, submenu_8_brass);
    _ADD_INSTR(59, submenu_8_brass);
    _ADD_INSTR(60, submenu_8_brass);
    _ADD_INSTR(61, submenu_8_brass);
    _ADD_INSTR(62, submenu_8_brass);
    _ADD_INSTR(63, submenu_8_brass);

    Append(wxID_ANY,wxT("Reed"), submenu_9_reed);
    _ADD_INSTR(64, submenu_9_reed);
    _ADD_INSTR(65, submenu_9_reed);
    _ADD_INSTR(66, submenu_9_reed);
    _ADD_INSTR(67, submenu_9_reed);
    _ADD_INSTR(68, submenu_9_reed);
    _ADD_INSTR(69, submenu_9_reed);
    _ADD_INSTR(70, submenu_9_reed);
    _ADD_INSTR(71, submenu_9_reed);

    Append(wxID_ANY,wxT("Flutes and Pipe"), submenu_10_pipe);
    _ADD_INSTR(72, submenu_10_pipe);
    _ADD_INSTR(73, submenu_10_pipe);
    _ADD_INSTR(74, submenu_10_pipe);
    _ADD_INSTR(75, submenu_10_pipe);
    _ADD_INSTR(76, submenu_10_pipe);
    _ADD_INSTR(77, submenu_10_pipe);
    _ADD_INSTR(78, submenu_10_pipe);
    _ADD_INSTR(79, submenu_10_pipe);

    Append(wxID_ANY,wxT("Synth Lead"), submenu_11_synths);
    _ADD_INSTR(80, submenu_11_synths);
    _ADD_INSTR(81, submenu_11_synths);
    _ADD_INSTR(82, submenu_11_synths);
    _ADD_INSTR(83, submenu_11_synths);
    _ADD_INSTR(84, submenu_11_synths);
    _ADD_INSTR(86, submenu_11_synths);
    _ADD_INSTR(87, submenu_11_synths);
    _ADD_INSTR(90, submenu_11_synths);
    _ADD_INSTR(97, submenu_11_synths);
    _ADD_INSTR(99, submenu_11_synths);
    _ADD_INSTR(103, submenu_11_synths);

    Append(wxID_ANY,wxT("Ethnic"), submenu_12_ethnic);
    _ADD_INSTR(104, submenu_12_ethnic);
    _ADD_INSTR(105, submenu_12_ethnic);
    _ADD_INSTR(106, submenu_12_ethnic);
    _ADD_INSTR(107, submenu_12_ethnic);
    _ADD_INSTR(108, submenu_12_ethnic);
    _ADD_INSTR(109, submenu_12_ethnic);
    _ADD_INSTR(110, submenu_12_ethnic);
    _ADD_INSTR(111, submenu_12_ethnic);

    Append(wxID_ANY,wxT("Percussion"), submenu_13_percussion);
    _ADD_INSTR(112, submenu_13_percussion);
    _ADD_INSTR(113, submenu_13_percussion);
    _ADD_INSTR(114, submenu_13_percussion);
    _ADD_INSTR(115, submenu_13_percussion);
    _ADD_INSTR(116, submenu_13_percussion);
    _ADD_INSTR(117, submenu_13_percussion);
    _ADD_INSTR(118, submenu_13_percussion);
    _ADD_INSTR(119, submenu_13_percussion);
    _ADD_INSTR(47 , submenu_13_percussion);

    Append(wxID_ANY,wxT("Sound Effects"), submenu_14_sound_fx);
    _ADD_INSTR(121, submenu_14_sound_fx);
    _ADD_INSTR(122, submenu_14_sound_fx);
    _ADD_INSTR(123, submenu_14_sound_fx);
    _ADD_INSTR(124, submenu_14_sound_fx);
    _ADD_INSTR(125, submenu_14_sound_fx);
    _ADD_INSTR(126, submenu_14_sound_fx);
    _ADD_INSTR(127, submenu_14_sound_fx);

#undef _ADD_INSTR


}

InstrumentPicker::~InstrumentPicker()
{
    // FIMXE - this should maybe delete the menus
}
    
void InstrumentPicker::menuSelected(wxCommandEvent& evt)
{
    int instrumentID = evt.GetId() - 10000;

    ASSERT_E(instrumentID, <,  128);
    ASSERT_E(instrumentID, >=, 0);

    m_model->setInstrument(instrumentID);
    Display::render();
}



    
}
