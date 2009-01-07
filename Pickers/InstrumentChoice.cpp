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

static const wxString g_inst_names[] =
{
    wxT("Acoustic Grand Piano"), // 0
    wxT("Bright Acoustic Piano"), // 1
    wxT("Electric Grand Piano"), // 2
    wxT("Honky-Tonk Piano"), // 3
    wxT("Electric Piano 1"), // 4
    wxT("Electric Piano 2"), // 5
    wxT("Harpsichord"), // 6
    wxT("Clavinet"), // 7
    wxT("Celesta"), // 8
    wxT("Glockenspiel"), // 9
    wxT("Music Box"), // 10
    wxT("Vibraphone"), // 11
    wxT("Marimba"), // 12
    wxT("Xylophone"), // 13
    wxT("Tubular Bells"), // 14
    wxT("Dulcimer"), // 15
    wxT("Drawbar Organ"), // 16
    wxT("Percussive Organ"), // 17
    wxT("Rock Organ"), // 18
    wxT("Church Organ"), // 19
    wxT("Reed Organ"), // 20
    wxT("Accordion"), // 21
    wxT("Harmonica"), // 22
    wxT("Tango Accordion"), // 23
    wxT("Nylon String Guitar"), // 24
    wxT("Steel String Guitar"), // 25
    wxT("Jazz Guitar"), // 26
    wxT("Clean Electric Guitar"), // 27
    wxT("Muted Electric Guitar"), // 28
    wxT("Overdrive Guitar"), // 29
    wxT("Distortion Guitar"), // 30
    wxT("Guitar Harmonics"), // 31
    wxT("Acoustic Bass"), // 32
    wxT("Fingered Bass"), // 33
    wxT("Picked Bass"), // 34
    wxT("Fretless Bass"), // 35
    wxT("Slap Bass 1"), // 36
    wxT("Slap bass 2"), // 37
    wxT("Synth Bass 1"), // 38
    wxT("Synth Bass 2"), // 39
    wxT("Violin"), // 40
    wxT("Viola"), // 41
    wxT("Cello"), // 42
    wxT("Contrabass"), // 43
    wxT("Tremolo Strings"), // 44
    wxT("Pizzicato Strings"), // 45
    wxT("Orchestral Harp"), // 46
    wxT("Timpani"), // 47
    wxT("String Ensemble 1"), // 48
    wxT("String Ensemble 2"), // 49
    wxT("Synth Strings 1"), // 50
    wxT("Synth Strings 2"), // 51
    wxT("Choir (ahh)"), // 52
    wxT("Choir (ooh)"), // 53
    wxT("Synth Voice"), // 54
    wxT("Orchestral Hit"), // 55
    wxT("Trumpet"), // 56
    wxT("Trombone"), // 57
    wxT("Tuba"), // 58
    wxT("Muted Trumpet"), // 59
    wxT("French Horn"), // 60
    wxT("Brass Section"), // 61
    wxT("Synth Brass 1"), // 62
    wxT("Synth Brass 2"), // 63
    wxT("Soprano Sax"), // 64
    wxT("Alto Sax"), // 65
    wxT("Tenor Sax"), // 66
    wxT("Baritone Sax"), // 67
    wxT("Oboe"), // 68
    wxT("English Horn"), // 69
    wxT("Bassoon"), // 70
    wxT("Clarinet"), // 71
    wxT("Piccolo"), // 72
    wxT("Flute"), // 73
    wxT("Recorder"), // 74
    wxT("Pan Flute"), // 75
    wxT("Blown Bottle"), // 76
    wxT("Shakuachi"), // 77
    wxT("Whistle"), // 78
    wxT("Ocarina"), // 79
    wxT("Square Wave"), // 80
    wxT("Sawtooth wave"), // 81
    wxT("Caliope"), // 82
    wxT("Chiff"), // 83
    wxT("Charang"), // 84
    wxT("Synth Voice 2"), // 85
    wxT("Fifths"), // 86
    wxT("Bass & Lead"), // 87
    wxT("New Age"), // 88
    wxT("Warm"), // 89
    wxT("Polysynth"), // 90
    wxT("Choir"), // 91
    wxT("Bowed"), // 92
    wxT("Metallic"), // 93
    wxT("Halo"), // 94
    wxT("Sweep"), // 95
    wxT("Rain"), // 96
    wxT("Sound Track"), // 97
    wxT("Crystal"), // 98
    wxT("Atmosphere"), // 99
    wxT("Brightness"), // 100
    wxT("Goblins"), // 101
    wxT("Drops"), // 102
    wxT("Star Theme"), // 103
    wxT("Sitar"), // 104
    wxT("Banjo"), // 105
    wxT("Shamisen"), // 106
    wxT("Koto"), // 107
    wxT("Kalimba"), // 108
    wxT("Bagpipe"), // 109
    wxT("Fiddle"), // 110
    wxT("Shanai"), // 111
    wxT("Tinkle Bell"), // 112
    wxT("Agogo"), // 113
    wxT("Steel Drums"), // 114
    wxT("Woodblock"), // 115
    wxT("Taiko Drum"), // 116
    wxT("Melodic Drum"), // 117
    wxT("Synth Drum"), // 118
    wxT("Reverse Cymbal"), // 119
    wxT("Guitar Fret Noise"), // 120
    wxT("Breath Noise"), // 121
    wxT("Seashore"), // 122
    wxT("Bird Tweet"), // 123
    wxT("Telephone Ring"), // 124
    wxT("Helicopter"), // 125
    wxT("Applause"), // 126
    wxT("Gunshot") // 127
};
    
InstrumentChoice::InstrumentChoice() : wxMenu()
{
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

#define _ADD_INSTR(id, parent) inst_menus[ id ]= parent -> Append( id+10000, g_inst_names[ id ] );

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


const char* InstrumentChoice::getInstrumentName(int instrumentID)
{
    assertExpr(instrumentID,<,128);
    assertExpr(instrumentID,>=,0);

    return (const char*)g_inst_names[instrumentID].mb_str();
}


}
