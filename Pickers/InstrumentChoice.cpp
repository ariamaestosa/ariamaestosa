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


#include "Pickers/InstrumentChoice.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"
#include <iostream>

#include "AriaCore.h"
#include "Config.h"

namespace AriaMaestosa {
	
BEGIN_EVENT_TABLE(InstrumentChoice, wxMenu)

//EVT_MENU_RANGE(0,127,InstrumentChoice::menuSelected)

END_EVENT_TABLE()

void InstrumentChoice::setParent(Track* track)
{
	parent = track;	
}
	
InstrumentChoice::InstrumentChoice() : wxMenu()
{
    INIT_LEAK_CHECK();
	
    // piano
    inst_name[0]="Acoustic Grand Piano";
    inst_name[1]="Bright Acoustic Piano";
    inst_name[2]="Electric Grand Piano";
    inst_name[3]="Honky-Tonk Piano";
    inst_name[4]="Electric Piano 1";
    inst_name[5]="Electric Piano 2";
    inst_name[6]="Harpsichord";
    inst_name[7]="Clavinet";
    
    // chromatic
    inst_name[8]="Celesta";
    inst_name[9]="Glockenspiel";
    inst_name[10]="Music Box";
    inst_name[11]="Vibraphone";
    inst_name[12]="Marimba";
    inst_name[13]="Xylophone";
    inst_name[14]="Tubular Bells";
    inst_name[15]="Dulcimer";
    
    // organ
    inst_name[16]="Drawbar Organ";
    inst_name[17]="Percussive Organ";
    inst_name[18]="Rock Organ";
    inst_name[19]="Church Organ";
    inst_name[20]="Reed Organ";
    inst_name[21]="Accordion";
    inst_name[22]="Harmonica";
    inst_name[23]="Tango Accordion";
    
    // guitar
    inst_name[24]="Nylon String Guitar";
    inst_name[25]="Steel String Guitar";
    inst_name[26]="Jazz Guitar";
    inst_name[27]="Clean Electric Guitar";
    inst_name[28]="Muted Electric Guitar";
    inst_name[29]="Overdrive Guitar";
    inst_name[30]="Distortion Guitar";
    inst_name[31]="Guitar Harmonics";
    inst_name[120]="Guitar Fret Noise";
    
    // bass
    inst_name[32]="Acoustic Bass";
    inst_name[33]="Fingered Bass";
    inst_name[34]="Picked Bass";
    inst_name[35]="Fretless Bass";
    inst_name[36]="Slap Bass 1";
    inst_name[37]="Slap bass 2";
    inst_name[38]="Synth Bass 1";
    inst_name[39]="Synth Bass 2";
    
    // orchestra
    inst_name[40]="Violin";
    inst_name[41]="Viola";
    inst_name[42]="Cello";
    inst_name[43]="Contrabass";
    inst_name[44]="Tremolo Strings";
    inst_name[45]="Pizzicato Strings";
    inst_name[46]="Orchestral Harp";
    inst_name[48]="String Ensemble 1";
    inst_name[49]="String Ensemble 2";
    inst_name[50]="Synth Strings 1";
    inst_name[51]="Synth Strings 2";
    inst_name[55]="Orchestral Hit";
    
    // pads, choirs, voices
    inst_name[52]="Choir (ahh)";
    inst_name[53]="Choir (ooh)";
    inst_name[89]="Warm";
    inst_name[91]="Choir";
    inst_name[94]="Halo";
    inst_name[92]="Bowed";
    inst_name[95]="Sweep";
    inst_name[54]="Synth Voice";
    inst_name[85]="Synth Voice 2";
    inst_name[98]="Crystal";
    inst_name[100]="Brightness";
    inst_name[101]="Goblins";
    inst_name[102]="Echo Drops";
    inst_name[93]="Metallic";
    inst_name[88]="New Age";
    inst_name[96]="Rain";
    
    // brass
    inst_name[56]="Trumpet";
    inst_name[57]="Trombone";
    inst_name[58]="Tuba";
    inst_name[59]="Muted Trumpet";
    inst_name[60]="French Horn";
    inst_name[61]="Brass Section";
    inst_name[62]="Synth Brass 1";
    inst_name[63]="Synth Brass 2";
    
    // reed
    inst_name[64]="Soprano Sax";
    inst_name[65]="Alto Sax";
    inst_name[66]="Tenor Sax";
    inst_name[67]="Baritone Sax";
    inst_name[68]="Oboe";
    inst_name[69]="English Horn";
    inst_name[70]="Bassoon";
    inst_name[71]="Clarinet";
    
    // pipe
    inst_name[72]="Piccolo";
    inst_name[73]="Flute";
    inst_name[74]="Recorder";
    inst_name[75]="Pan Flute";
    inst_name[76]="Blown Bottle";
    inst_name[77]="Shakuachi";
    inst_name[78]="Whistle";
    inst_name[79]="Ocarina";
    
    // synths
    inst_name[80]="Square Wave";
    inst_name[81]="Sawtooth wave";
    inst_name[82]="Caliope";
    inst_name[83]="Chiff";
    inst_name[84]="Charang";
    inst_name[86]="Fifths";
    inst_name[87]="Bass & Lead";
    inst_name[90]="Polysynth";
    inst_name[97]="Sound Track";
    inst_name[99]="Atmosphere";
    inst_name[103]="Star Theme";
    
    // ethnic
    inst_name[104]="Sitar";
    inst_name[105]="Banjo";
    inst_name[106]="Shamisen";
    inst_name[107]="Koto";
    inst_name[108]="Kalimba";
    inst_name[109]="Bagpipe";
    inst_name[110]="Fiddle";
    inst_name[111]="Shanai";
    
    // percussion
    inst_name[112]="Tinkle Bell";
    inst_name[113]="Agogo";
    inst_name[114]="Steel Drums";
    inst_name[115]="Woodblock";
    inst_name[116]="Taiko Drum";
    inst_name[117]="Melodic Drum";
    inst_name[47]="Timpani";
    inst_name[118]="Synth Drum";
    inst_name[119]="Reverse Cymbal";
    
    // sound effects
    inst_name[121]="Breath Noise";
    inst_name[122]="Seashore";
    inst_name[123]="Bird Tweet";
    inst_name[124]="Telephone Ring";
    inst_name[125]="Helicopter";
    inst_name[126]="Applause";
    inst_name[127]="Gunshot";
    
    submenu_1_piano=new wxMenu();
    submenu_2_chromatic=new wxMenu();
    submenu_3_organ=new wxMenu();
    submenu_4_guitar=new wxMenu();
    submenu_5_bass=new wxMenu();
    submenu_6_orchestra=new wxMenu();
    submenu_7_choirs_pads=new wxMenu();
    submenu_8_brass=new wxMenu();
    submenu_9_reed=new wxMenu();
    submenu_10_pipe=new wxMenu();
    submenu_11_synths=new wxMenu();
    submenu_12_ethnic=new wxMenu();
    submenu_13_percussion=new wxMenu();
    submenu_14_sound_effects=new wxMenu();
    submenu_15_drums=new wxMenu();
    
#define _ADD_INSTR(id, parent) inst_menus[ id ]= parent -> Append( id+10000 ,fromCString(inst_name[ id ]));
    
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
    _ADD_INSTR(47, submenu_13_percussion);
    
    Append(wxID_ANY,wxT("Sound Effects"), submenu_14_sound_effects);
    _ADD_INSTR(121, submenu_14_sound_effects);
    _ADD_INSTR(122, submenu_14_sound_effects);
    _ADD_INSTR(123, submenu_14_sound_effects);
    _ADD_INSTR(124, submenu_14_sound_effects);
    _ADD_INSTR(125, submenu_14_sound_effects);
    _ADD_INSTR(126, submenu_14_sound_effects);
    _ADD_INSTR(127, submenu_14_sound_effects);
    
#undef _ADD_INSTR

}

InstrumentChoice::~InstrumentChoice()
{
    // FIMXE - this should maybe delete the menus
}
/*
void InstrumentChoice::setInstrumentID(int id)
{
    instrumentID = id;   
}
*/
void InstrumentChoice::menuSelected(wxCommandEvent& evt)
{
    instrumentID=evt.GetId()-10000;

    assertExpr(instrumentID,<,128);
    assertExpr(instrumentID,>=,0);
    
	parent->setInstrument(instrumentID);
    Display::render();
    
}


char* InstrumentChoice::getInstrumentName(int instrumentID)
{
    assertExpr(instrumentID,<,128);
    assertExpr(instrumentID,>=,0);
    
    return inst_name[instrumentID];
}


}
