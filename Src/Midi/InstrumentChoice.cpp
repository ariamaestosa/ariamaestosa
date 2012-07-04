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

// Necessary to handle translations
#include <wx/wx.h>


using namespace AriaMaestosa;

static const std::pair<int, wxString> g_inst_names[] =
{
    std::pair<int, wxString>(0, _("Acoustic Grand Piano")),
    std::pair<int, wxString>(1, _("Bright Acoustic Piano")),
    std::pair<int, wxString>(2, _("Electric Grand Piano")),
    std::pair<int, wxString>(3, _("Honky-Tonk Piano")),
    std::pair<int, wxString>(4, _("Electric Piano 1")),
    std::pair<int, wxString>(5, _("Electric Piano 2")),
    std::pair<int, wxString>(6, _("Harpsichord")),
    std::pair<int, wxString>(7, _("Clavinet")),
    std::pair<int, wxString>(8, _("Celesta")),
    std::pair<int, wxString>(9, _("Glockenspiel")),
    std::pair<int, wxString>(10, _("Music Box")),
    std::pair<int, wxString>(11, _("Vibraphone")),
    std::pair<int, wxString>(12, _("Marimba")),
    std::pair<int, wxString>(13, _("Xylophone")),
    std::pair<int, wxString>(14, _("Tubular Bells")),
    std::pair<int, wxString>(15, _("Dulcimer")),
    std::pair<int, wxString>(16, _("Drawbar Organ")),
    std::pair<int, wxString>(17, _("Percussive Organ")),
    std::pair<int, wxString>(18, _("Rock Organ")),
    std::pair<int, wxString>(19, _("Church Organ")),
    std::pair<int, wxString>(20, _("Reed Organ")),
    std::pair<int, wxString>(21, _("Accordion")),
    std::pair<int, wxString>(22, _("Harmonica")),
    std::pair<int, wxString>(23, _("Tango Accordion")),
    std::pair<int, wxString>(24, _("Nylon String Guitar")),
    std::pair<int, wxString>(25, _("Steel String Guitar")),
    std::pair<int, wxString>(26, _("Jazz Guitar")),
    std::pair<int, wxString>(27, _("Clean Electric Guitar")),
    std::pair<int, wxString>(28, _("Muted Electric Guitar")),
    std::pair<int, wxString>(29, _("Overdrive Guitar")),
    std::pair<int, wxString>(30, _("Distortion Guitar")),
    std::pair<int, wxString>(31, _("Guitar Harmonics")),
    std::pair<int, wxString>(32, _("Acoustic Bass")),
    std::pair<int, wxString>(33, _("Fingered Bass")),
    std::pair<int, wxString>(34, _("Picked Bass")),
    std::pair<int, wxString>(35, _("Fretless Bass")),
    std::pair<int, wxString>(36, _("Slap Bass 1")),
    std::pair<int, wxString>(37, _("Slap Bass 2")),
    std::pair<int, wxString>(38, _("Synth Bass 1")),
    std::pair<int, wxString>(39, _("Synth Bass 2")),
    std::pair<int, wxString>(40, _("Violin")),
    std::pair<int, wxString>(41, _("Viola")),
    std::pair<int, wxString>(42, _("Cello")),
    std::pair<int, wxString>(43, _("Contrabass")),
    std::pair<int, wxString>(44, _("Tremolo Strings")),
    std::pair<int, wxString>(45, _("Pizzicato Strings")),
    std::pair<int, wxString>(46, _("Orchestral Harp")),
    std::pair<int, wxString>(47, _("Timpani")),
    std::pair<int, wxString>(48, _("String Ensemble 1")),
    std::pair<int, wxString>(49, _("String Ensemble 2")),
    std::pair<int, wxString>(50, _("Synth Strings 1")),
    std::pair<int, wxString>(51, _("Synth Strings 2")),
    std::pair<int, wxString>(52, _("Choir (ahh)")),
    std::pair<int, wxString>(53, _("Choir (ooh)")),
    std::pair<int, wxString>(54, _("Synth Voice")),
    std::pair<int, wxString>(55, _("Orchestral Hit")),
    std::pair<int, wxString>(56, _("Trumpet")),
    std::pair<int, wxString>(57, _("Trombone")),
    std::pair<int, wxString>(58, _("Tuba")),
    std::pair<int, wxString>(59, _("Muted Trumpet")),
    std::pair<int, wxString>(60, _("French Horn")),
    std::pair<int, wxString>(61, _("Brass Section")),
    std::pair<int, wxString>(62, _("Synth Brass 1")),
    std::pair<int, wxString>(63, _("Synth Brass 2")),
    std::pair<int, wxString>(64, _("Soprano Sax")),
    std::pair<int, wxString>(65, _("Alto Sax")),
    std::pair<int, wxString>(66, _("Tenor Sax")),
    std::pair<int, wxString>(67, _("Baritone Sax")),
    std::pair<int, wxString>(68, _("Oboe")),
    std::pair<int, wxString>(69, _("English Horn")),
    std::pair<int, wxString>(70, _("Bassoon")),
    std::pair<int, wxString>(71, _("Clarinet")),
    std::pair<int, wxString>(72, _("Piccolo")),
    std::pair<int, wxString>(73, _("Flute")),
    std::pair<int, wxString>(74, _("Recorder")),
    std::pair<int, wxString>(75, _("Pan Flute")),
    std::pair<int, wxString>(76, _("Blown Bottle")),
    std::pair<int, wxString>(77, _("Shakuachi")),
    std::pair<int, wxString>(78, _("Whistle")),
    std::pair<int, wxString>(79, _("Ocarina")),
    std::pair<int, wxString>(80, _("Square Wave")),
    std::pair<int, wxString>(81, _("Sawtooth wave")),
    std::pair<int, wxString>(82, _("Caliope")),
    std::pair<int, wxString>(83, _("Chiff")),
    std::pair<int, wxString>(84, _("Charang")),
    std::pair<int, wxString>(85, _("Synth Voice 2")),
    std::pair<int, wxString>(86, _("Fifths")),
    std::pair<int, wxString>(87, _("Bass and Lead")),
    std::pair<int, wxString>(88, _("New Age")),
    std::pair<int, wxString>(89, _("Warm")),
    std::pair<int, wxString>(90, _("Polysynth")),
    std::pair<int, wxString>(91, _("Choir")),
    std::pair<int, wxString>(92, _("Bowed")),
    std::pair<int, wxString>(93, _("Metallic")),
    std::pair<int, wxString>(94, _("Halo")),
    std::pair<int, wxString>(95, _("Sweep")),
    std::pair<int, wxString>(96, _("Rain")),
    std::pair<int, wxString>(97, _("Sound Track")),
    std::pair<int, wxString>(98, _("Crystal")),
    std::pair<int, wxString>(99, _("Atmosphere")),
    std::pair<int, wxString>(100, _("Brightness")),
    std::pair<int, wxString>(101, _("Goblins")),
    std::pair<int, wxString>(102, _("Drops")),
    std::pair<int, wxString>(103, _("Star Theme")),
    std::pair<int, wxString>(104, _("Sitar")),
    std::pair<int, wxString>(105, _("Banjo")),
    std::pair<int, wxString>(106, _("Shamisen")),
    std::pair<int, wxString>(107, _("Koto")),
    std::pair<int, wxString>(108, _("Kalimba")),
    std::pair<int, wxString>(109, _("Bagpipe")),
    std::pair<int, wxString>(110, _("Fiddle")),
    std::pair<int, wxString>(111, _("Shanai")),
    std::pair<int, wxString>(112, _("Tinkle Bell")),
    std::pair<int, wxString>(113, _("Agogo")),
    std::pair<int, wxString>(114, _("Steel Drums")),
    std::pair<int, wxString>(115, _("Woodblock")),
    std::pair<int, wxString>(116, _("Taiko Drum")),
    std::pair<int, wxString>(117, _("Melodic Drum")),
    std::pair<int, wxString>(118, _("Synth Drum")),
    std::pair<int, wxString>(119, _("Reverse Cymbal")),
    std::pair<int, wxString>(120, _("Guitar Fret Noise")),
    std::pair<int, wxString>(121, _("Breath Noise")),
    std::pair<int, wxString>(122, _("Seashore")),
    std::pair<int, wxString>(123, _("Bird Tweet")),
    std::pair<int, wxString>(124, _("Telephone Ring")),
    std::pair<int, wxString>(125, _("Helicopter")),
    std::pair<int, wxString>(126, _("Applause")),
    std::pair<int, wxString>(127, _("Gunshot"))
};

// ----------------------------------------------------------------------------------------------------------

InstrumentChoice::InstrumentChoice(const int defaultInstrument, IInstrumentChoiceListener* listener)
{
    m_selected_instrument = defaultInstrument;
    m_listener = listener;
}

// ----------------------------------------------------------------------------------------------------------

const wxString InstrumentChoice::getInstrumentName(int instrumentID)
{
    ASSERT_E(instrumentID,<,128);
    ASSERT_E(instrumentID,>=,0);

    return ::wxGetTranslation(g_inst_names[instrumentID].second);
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
