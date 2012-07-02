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
#include "PreferencesData.h"

using namespace AriaMaestosa;

#define SIZE(x) sizeof(x)/sizeof(x[0])

static const int MENU_REF_ID = 100;
static const int SUB_MENU_REF_ID = 10000;
static const int MIDI_INSTR_START_ID = 0;
static const int MIDI_INSTR_STOP_ID = 127;
static const int STANDARD_MIDI_SIZE = 8;


// Standard MIDI classes
static const int g_piano_insts[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static const int g_chromatic_percussion_insts[] = { 8, 9, 10, 11, 12, 13 ,14, 15 };
static const int g_organ_insts[] = { 16, 17, 18, 19, 20, 21, 22, 23 };
static const int g_guitar_insts[] = { 24, 25, 26, 27, 28, 29, 30, 31 };
static const int g_bass_insts[] = { 32, 33, 34, 35, 36, 37, 38, 39 };
static const int g_strings_insts[] = { 40, 41, 42, 43, 44, 45, 46, 47 };
static const int g_ensemble_insts[] = { 48, 49, 50, 51, 52, 53, 54, 55 };
static const int g_brass_insts[] = { 56, 57, 58, 59, 60, 61, 62, 63 };
static const int g_reed_insts[] = { 64, 65, 66, 67, 68, 69, 70, 71 };
static const int g_pipe_insts[] = { 72, 73, 74, 75, 76, 77, 78, 79 };
static const int g_synth_lead_insts[] = { 80, 81, 82, 83, 84, 85, 86, 87 };
static const int g_synth_pad_insts[] = { 88, 89, 90, 91, 92, 93, 94, 95 };
static const int g_synth_effects_insts[] = { 96, 97, 98, 99, 100, 101, 102, 103 };
static const int g_ethnic_insts[] = { 104, 105, 106, 107, 108, 109, 110, 111 };
static const int g_percussive_insts[] = { 112, 113, 114, 115, 116, 117, 118, 119 };
static const int g_sound_effects_insts[] = { 120, 121, 122, 123, 124, 125, 126, 127 };


// Aria classes
static const int g_aria_guitar_insts[] = { 24, 25, 26, 27, 28, 29, 30, 31, 120 };
static const int g_aria_strings_and_orchestra_insts[] = { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 55 };
static const int g_aria_choirs_pads_voices_insts[] = { 52, 53, 89, 91, 94, 92, 95, 54, 85, 98, 100, 101, 102, 93, 88, 96};
static const int g_aria_synth_lead_insts[] = { 80, 81, 82, 83, 84, 85, 86, 87, 90, 97, 99, 103 };
static const int g_aria_percussion_insts[] = { 112, 113, 114, 115, 116, 117, 118, 119, 47 };
static const int g_aria_sound_effects_insts[] = { 121, 122, 123, 124, 125, 126, 127 };


// Buzzwood classes
static const int g_buzzwood_piano_insts[] = { 0, 1, 2, 3, 4, 5, 6, 7, 15 };
static const int g_buzzwood_pads_insts[] = { 52, 53, 54, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103 };
static const int g_buzzwood_reed_insts[] = { 64, 65, 66, 67, 68, 69, 70, 71, 109, 111 };
static const int g_buzzwood_guitar_insts[] = { 24, 25, 26, 27, 28, 29, 30, 31, 120 };
static const int g_buzzwood_plucked_insts[] = { 45, 46, 104, 105, 106, 107, 108 };
static const int g_buzzwood_bowed_insts[] = { 110, 40, 41, 42, 43, 44, 48, 49, 50, 51, 55};
static const int g_buzzwood_bass_insts[] = { 32, 33, 34, 35, 36, 37, 38, 39, 43 };
static const int g_buzzwood_percussion_insts[] = { 8, 9, 10, 11, 12, 13, 47, 108, 112, 113, 114, 115, 116, 117, 118, 119 };




// See http://www.zikinf.com/articles/mao/tablemidi.php
static const PickerMenu g_standard_midi_picker[] =
{
    { wxT("Piano"), g_piano_insts, STANDARD_MIDI_SIZE },
    { wxT("Chromatic Percussion"), g_chromatic_percussion_insts, STANDARD_MIDI_SIZE },
    { wxT("Organ"), g_organ_insts, STANDARD_MIDI_SIZE },
    { wxT("Guitar"), g_guitar_insts, STANDARD_MIDI_SIZE },
    { wxT("Bass"), g_bass_insts, STANDARD_MIDI_SIZE },
    { wxT("Strings"), g_strings_insts, STANDARD_MIDI_SIZE },
    { wxT("Ensemble"), g_ensemble_insts, STANDARD_MIDI_SIZE },
    { wxT("Brass"), g_brass_insts, STANDARD_MIDI_SIZE },
    { wxT("Reed"), g_reed_insts, STANDARD_MIDI_SIZE },
    { wxT("Pipe"), g_pipe_insts, STANDARD_MIDI_SIZE },
    { wxT("Synth Lead"), g_synth_lead_insts, STANDARD_MIDI_SIZE },
    { wxT("Synth Pad"), g_synth_pad_insts, STANDARD_MIDI_SIZE },
    { wxT("Synth Effects"), g_synth_effects_insts, STANDARD_MIDI_SIZE },
    { wxT("Ethnic"), g_ethnic_insts, STANDARD_MIDI_SIZE },
    { wxT("Percussive"), g_percussive_insts, STANDARD_MIDI_SIZE },
    { wxT("Sound Effects"), g_sound_effects_insts, STANDARD_MIDI_SIZE }
};


static const PickerMenu g_aria_picker[] =
{
    { wxT("Piano"), g_piano_insts, SIZE(g_piano_insts) },
    { wxT("Chromatic"), g_chromatic_percussion_insts, SIZE(g_chromatic_percussion_insts) },
    { wxT("Organ"), g_organ_insts, SIZE(g_organ_insts) },
    { wxT("Guitar"), g_aria_guitar_insts, SIZE(g_aria_guitar_insts) },
    { wxT("Bass"), g_bass_insts, SIZE(g_bass_insts) },
    { wxT("Strings and Orchestra"), g_aria_strings_and_orchestra_insts, SIZE(g_aria_strings_and_orchestra_insts) },
    { wxT("Choirs, Pads and Voices"), g_aria_choirs_pads_voices_insts, SIZE(g_aria_choirs_pads_voices_insts) },
    { wxT("Brass"), g_brass_insts, SIZE(g_brass_insts) },
    { wxT("Reed"), g_reed_insts, SIZE(g_reed_insts) },
    { wxT("Flutes and Pipe"), g_pipe_insts, SIZE(g_pipe_insts) },
    { wxT("Synth Lead"), g_aria_synth_lead_insts, SIZE(g_aria_synth_lead_insts) },
    { wxT("Ethnic"), g_ethnic_insts, SIZE(g_ethnic_insts) },
    { wxT("Percussion"), g_aria_percussion_insts, SIZE(g_aria_percussion_insts) },
    { wxT("Sound Effects"), g_aria_sound_effects_insts, SIZE(g_aria_sound_effects_insts) }
};


// See http://www.buzzwood.com/midtest.htm
static const PickerMenu g_buzzwood_picker[] =
{
    { wxT("Piano"), g_buzzwood_piano_insts, SIZE(g_buzzwood_piano_insts) },
    { wxT("Organ"), g_organ_insts, SIZE(g_organ_insts) },
    { wxT("Brass"), g_brass_insts, SIZE(g_brass_insts) },
    { wxT("Pads"), g_buzzwood_pads_insts, SIZE(g_buzzwood_pads_insts) },
    { wxT("Synth Lead"), g_synth_lead_insts, SIZE(g_synth_lead_insts) },
    { wxT("Reed"), g_buzzwood_reed_insts, SIZE(g_buzzwood_reed_insts) },
    { wxT("Pipe"), g_pipe_insts, SIZE(g_pipe_insts) },
    { wxT("Guitar"), g_buzzwood_guitar_insts, SIZE(g_buzzwood_guitar_insts) },
    { wxT("Plucked"), g_buzzwood_plucked_insts, SIZE(g_buzzwood_plucked_insts) },
    { wxT("Bowed"), g_buzzwood_bowed_insts, SIZE(g_buzzwood_bowed_insts) },
    { wxT("Bass"), g_buzzwood_bass_insts, SIZE(g_buzzwood_bass_insts) },
    { wxT("Percussion"), g_buzzwood_percussion_insts, SIZE(g_buzzwood_percussion_insts) },
    { wxT("Sound Effects"), g_sound_effects_insts, SIZE(g_sound_effects_insts) }
};



InstrumentPicker::InstrumentPicker() : wxMenu()
{
    m_model = NULL;
    m_current_classification_id = -1;
    buildNewPicker();
}

InstrumentPicker::~InstrumentPicker()
{
    deleteCurrentPicker();
    m_current_classification_id = -1;
}


void InstrumentPicker::menuSelected(wxCommandEvent& evt)
{
    int instrumentID = evt.GetId() - SUB_MENU_REF_ID;

    ASSERT_E(instrumentID, <=, MIDI_INSTR_STOP_ID);
    ASSERT_E(instrumentID, >=, MIDI_INSTR_START_ID);

    m_model->setInstrument(instrumentID);
    Display::render();
}


void InstrumentPicker::setModel(InstrumentChoice* model)
{
    m_model = model;
}


void InstrumentPicker::updateClassification()
{
    buildNewPicker();
}


void InstrumentPicker::buildNewPicker()
{
    deleteCurrentPicker();

    m_current_classification_id = PreferencesData::getInstance()->getIntValue(SETTING_ID_INSTRUMENT_CLASSIFICATION);

    switch (m_current_classification_id)
    {
        case 0  : buildPicker(g_standard_midi_picker, SIZE(g_standard_midi_picker)); break;
        case 1  : buildPicker(g_aria_picker, SIZE(g_aria_picker)); break;
        case 2  : buildPicker(g_buzzwood_picker, SIZE(g_buzzwood_picker)); break;
        default : buildPicker(g_standard_midi_picker, SIZE(g_standard_midi_picker)); break;
    }
}


void InstrumentPicker::buildPicker(const PickerMenu pickerMenuArray[], int size)
{
    for( int i = 0; i< size ; i++ )
    {
        wxMenu* menu = new wxMenu();
        const PickerMenu pickerMenu = pickerMenuArray[i];
        Append(MENU_REF_ID+i, ::wxGetTranslation(pickerMenu.name), menu);

        for( int j = 0; j< pickerMenu.inst_array_size ; j++ )
        {
            int id = pickerMenu.inst_array[j];
            m_inst_menus[id] = menu->Append(id + SUB_MENU_REF_ID, ::wxGetTranslation(InstrumentChoice::getInstrumentName(id)));
        }
    }
}


void InstrumentPicker::deleteCurrentPicker()
{
    if (m_current_classification_id != -1)
    {
        switch (m_current_classification_id)
        {
            case 0 : deletePicker(g_standard_midi_picker, SIZE(g_standard_midi_picker)); break;
            case 1 : deletePicker(g_aria_picker, SIZE(g_aria_picker)); break;
            case 2 : deletePicker(g_buzzwood_picker, SIZE(g_buzzwood_picker)); break;
        }
    }
}


void InstrumentPicker::deletePicker(const PickerMenu pickerMenuArray[], int size)
{
    for(int i = 0; i< size ; i++ )
    {
        Destroy(MENU_REF_ID+i);
    }
}
