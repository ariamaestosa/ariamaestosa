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


#include "Midi/InstrumentChoice.h"
#include <map>
#include "wx/string.h"

using namespace AriaMaestosa;

static const std::pair<int, wxString> g_inst_names[] =
{
    std::pair<int, wxString>(0, wxT("Acoustic Grand Piano")),
    std::pair<int, wxString>(1, wxT("Bright Acoustic Piano")),
    std::pair<int, wxString>(2, wxT("Electric Grand Piano")),
    std::pair<int, wxString>(3, wxT("Honky-Tonk Piano")),
    std::pair<int, wxString>(4, wxT("Electric Piano 1")),
    std::pair<int, wxString>(5, wxT("Electric Piano 2")),
    std::pair<int, wxString>(6, wxT("Harpsichord")),
    std::pair<int, wxString>(7, wxT("Clavinet")),
    std::pair<int, wxString>(8, wxT("Celesta")),
    std::pair<int, wxString>(9, wxT("Glockenspiel")),
    std::pair<int, wxString>(10, wxT("Music Box")),
    std::pair<int, wxString>(11, wxT("Vibraphone")),
    std::pair<int, wxString>(12, wxT("Marimba")),
    std::pair<int, wxString>(13, wxT("Xylophone")),
    std::pair<int, wxString>(14, wxT("Tubular Bells")),
    std::pair<int, wxString>(15, wxT("Dulcimer")),
    std::pair<int, wxString>(16, wxT("Drawbar Organ")),
    std::pair<int, wxString>(17, wxT("Percussive Organ")),
    std::pair<int, wxString>(18, wxT("Rock Organ")),
    std::pair<int, wxString>(19, wxT("Church Organ")),
    std::pair<int, wxString>(20, wxT("Reed Organ")),
    std::pair<int, wxString>(21, wxT("Accordion")),
    std::pair<int, wxString>(22, wxT("Harmonica")),
    std::pair<int, wxString>(23, wxT("Tango Accordion")),
    std::pair<int, wxString>(24, wxT("Nylon String Guitar")),
    std::pair<int, wxString>(25, wxT("Steel String Guitar")),
    std::pair<int, wxString>(26, wxT("Jazz Guitar")),
    std::pair<int, wxString>(27, wxT("Clean Electric Guitar")),
    std::pair<int, wxString>(28, wxT("Muted Electric Guitar")),
    std::pair<int, wxString>(29, wxT("Overdrive Guitar")),
    std::pair<int, wxString>(30, wxT("Distortion Guitar")),
    std::pair<int, wxString>(31, wxT("Guitar Harmonics")),
    std::pair<int, wxString>(32, wxT("Acoustic Bass")),
    std::pair<int, wxString>(33, wxT("Fingered Bass")),
    std::pair<int, wxString>(34, wxT("Picked Bass")),
    std::pair<int, wxString>(35, wxT("Fretless Bass")),
    std::pair<int, wxString>(36, wxT("Slap Bass 1")),
    std::pair<int, wxString>(37, wxT("Slap bass 2")),
    std::pair<int, wxString>(38, wxT("Synth Bass 1")),
    std::pair<int, wxString>(39, wxT("Synth Bass 2")),
    std::pair<int, wxString>(40, wxT("Violin")),
    std::pair<int, wxString>(41, wxT("Viola")),
    std::pair<int, wxString>(42, wxT("Cello")),
    std::pair<int, wxString>(43, wxT("Contrabass")),
    std::pair<int, wxString>(44, wxT("Tremolo Strings")),
    std::pair<int, wxString>(45, wxT("Pizzicato Strings")),
    std::pair<int, wxString>(46, wxT("Orchestral Harp")),
    std::pair<int, wxString>(47, wxT("Timpani")),
    std::pair<int, wxString>(48, wxT("String Ensemble 1")),
    std::pair<int, wxString>(49, wxT("String Ensemble 2")),
    std::pair<int, wxString>(50, wxT("Synth Strings 1")),
    std::pair<int, wxString>(51, wxT("Synth Strings 2")),
    std::pair<int, wxString>(52, wxT("Choir (ahh)")),
    std::pair<int, wxString>(53, wxT("Choir (ooh)")),
    std::pair<int, wxString>(54, wxT("Synth Voice")),
    std::pair<int, wxString>(55, wxT("Orchestral Hit")),
    std::pair<int, wxString>(56, wxT("Trumpet")),
    std::pair<int, wxString>(57, wxT("Trombone")),
    std::pair<int, wxString>(58, wxT("Tuba")),
    std::pair<int, wxString>(59, wxT("Muted Trumpet")),
    std::pair<int, wxString>(60, wxT("French Horn")),
    std::pair<int, wxString>(61, wxT("Brass Section")),
    std::pair<int, wxString>(62, wxT("Synth Brass 1")),
    std::pair<int, wxString>(63, wxT("Synth Brass 2")),
    std::pair<int, wxString>(64, wxT("Soprano Sax")),
    std::pair<int, wxString>(65, wxT("Alto Sax")),
    std::pair<int, wxString>(66, wxT("Tenor Sax")),
    std::pair<int, wxString>(67, wxT("Baritone Sax")),
    std::pair<int, wxString>(68, wxT("Oboe")),
    std::pair<int, wxString>(69, wxT("English Horn")),
    std::pair<int, wxString>(70, wxT("Bassoon")),
    std::pair<int, wxString>(71, wxT("Clarinet")),
    std::pair<int, wxString>(72, wxT("Piccolo")),
    std::pair<int, wxString>(73, wxT("Flute")),
    std::pair<int, wxString>(74, wxT("Recorder")),
    std::pair<int, wxString>(75, wxT("Pan Flute")),
    std::pair<int, wxString>(76, wxT("Blown Bottle")),
    std::pair<int, wxString>(77, wxT("Shakuachi")),
    std::pair<int, wxString>(78, wxT("Whistle")),
    std::pair<int, wxString>(79, wxT("Ocarina")),
    std::pair<int, wxString>(80, wxT("Square Wave")),
    std::pair<int, wxString>(81, wxT("Sawtooth wave")),
    std::pair<int, wxString>(82, wxT("Caliope")),
    std::pair<int, wxString>(83, wxT("Chiff")),
    std::pair<int, wxString>(84, wxT("Charang")),
    std::pair<int, wxString>(85, wxT("Synth Voice 2")),
    std::pair<int, wxString>(86, wxT("Fifths")),
    std::pair<int, wxString>(87, wxT("Bass & Lead")),
    std::pair<int, wxString>(88, wxT("New Age")),
    std::pair<int, wxString>(89, wxT("Warm")),
    std::pair<int, wxString>(90, wxT("Polysynth")),
    std::pair<int, wxString>(91, wxT("Choir")),
    std::pair<int, wxString>(92, wxT("Bowed")),
    std::pair<int, wxString>(93, wxT("Metallic")),
    std::pair<int, wxString>(94, wxT("Halo")),
    std::pair<int, wxString>(95, wxT("Sweep")),
    std::pair<int, wxString>(96, wxT("Rain")),
    std::pair<int, wxString>(97, wxT("Sound Track")),
    std::pair<int, wxString>(98, wxT("Crystal")),
    std::pair<int, wxString>(99, wxT("Atmosphere")),
    std::pair<int, wxString>(100, wxT("Brightness")),
    std::pair<int, wxString>(101, wxT("Goblins")),
    std::pair<int, wxString>(102, wxT("Drops")),
    std::pair<int, wxString>(103, wxT("Star Theme")),
    std::pair<int, wxString>(104, wxT("Sitar")),
    std::pair<int, wxString>(105, wxT("Banjo")),
    std::pair<int, wxString>(106, wxT("Shamisen")),
    std::pair<int, wxString>(107, wxT("Koto")),
    std::pair<int, wxString>(108, wxT("Kalimba")),
    std::pair<int, wxString>(109, wxT("Bagpipe")),
    std::pair<int, wxString>(110, wxT("Fiddle")),
    std::pair<int, wxString>(111, wxT("Shanai")),
    std::pair<int, wxString>(112, wxT("Tinkle Bell")),
    std::pair<int, wxString>(113, wxT("Agogo")),
    std::pair<int, wxString>(114, wxT("Steel Drums")),
    std::pair<int, wxString>(115, wxT("Woodblock")),
    std::pair<int, wxString>(116, wxT("Taiko Drum")),
    std::pair<int, wxString>(117, wxT("Melodic Drum")),
    std::pair<int, wxString>(118, wxT("Synth Drum")),
    std::pair<int, wxString>(119, wxT("Reverse Cymbal")),
    std::pair<int, wxString>(120, wxT("Guitar Fret Noise")),
    std::pair<int, wxString>(121, wxT("Breath Noise")),
    std::pair<int, wxString>(122, wxT("Seashore")),
    std::pair<int, wxString>(123, wxT("Bird Tweet")),
    std::pair<int, wxString>(124, wxT("Telephone Ring")),
    std::pair<int, wxString>(125, wxT("Helicopter")),
    std::pair<int, wxString>(126, wxT("Applause")),
    std::pair<int, wxString>(127, wxT("Gunshot"))
};

// ----------------------------------------------------------------------------------------------------------

InstrumentChoice::InstrumentChoice(const int defaultInstrument, IInstrumentChoiceListener* listener)
{
    m_selected_instrument = defaultInstrument;
    m_listener = listener;
}

// ----------------------------------------------------------------------------------------------------------

const wxString& InstrumentChoice::getInstrumentName(int instrumentID)
{
    ASSERT_E(instrumentID,<,128);
    ASSERT_E(instrumentID,>=,0);
    
    return g_inst_names[instrumentID].second;
}

// ----------------------------------------------------------------------------------------------------------

void InstrumentChoice::setInstrument(const int instrument, const bool generateEvent)
{
    ASSERT_E(instrument, <,  128);
    ASSERT_E(instrument, >=, 0);
    m_selected_instrument = instrument;
    
    if (generateEvent and m_listener != NULL) m_listener->onInstrumentChanged(instrument);
}

// ----------------------------------------------------------------------------------------------------------
