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

#include "AriaCore.h"

#include "IO/IOUtils.h"
#include "Midi/Note.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Utils.h"
#include "UnitTest.h"

#include "irrXML/irrXML.h"

#include <iostream>

using namespace AriaMaestosa;

Note::Note(Track* parent,
           const int pitchID_arg,
           const int startTick_arg,
           const int endTick_arg,
           const int volume_arg,
           const int string_arg,
           const int fret_arg)
{
    m_pitch_ID   = pitchID_arg;
    m_start_tick = startTick_arg;
    m_end_tick   = endTick_arg;
    m_volume     = volume_arg;
    Note::string    = string_arg;
    Note::fret      = fret_arg;

    m_track = parent;

    m_selected = false;

    m_preferred_accidental_sign = -1;
}

// ----------------------------------------------------------------------------------------------------------

Note::~Note()
{
}

// ----------------------------------------------------------------------------------------------------------

int Note::getString()
{
    if (string == -1) findStringAndFretFromNote();
    return string;
}

// ----------------------------------------------------------------------------------------------------------

int Note::getFret()
{
    if (fret == -1) findStringAndFretFromNote();
    return fret;
}
    
// ----------------------------------------------------------------------------------------------------------

void Note::setFret(int i)
{
    fret = i;
    findNoteFromStringAndFret();
}

// ----------------------------------------------------------------------------------------------------------

void Note::setStringAndFret(int string_arg, int fret_arg)
{
    string = string_arg;
    fret   = fret_arg;
    findNoteFromStringAndFret();
}

// ----------------------------------------------------------------------------------------------------------

void Note::checkIfStringAndFretMatchNote(const bool fixStringAndFret)
{
    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    // if note is placed on a string that doesn't exist (anymore)
    if (fixStringAndFret and string > (int)tuning->tuning.size()-1)
    {
        findStringAndFretFromNote();
    }


    if (string == -1 or fret == -1 or m_pitch_ID != tuning->tuning[string] - fret)
    {
        if (fixStringAndFret) findStringAndFretFromNote();
        else                  findNoteFromStringAndFret();
    }

}

// ----------------------------------------------------------------------------------------------------------

void Note::setParent(Track* parent)
{
    m_track = parent;
}

// ----------------------------------------------------------------------------------------------------------
/**
  * In guitar editor, changes the number on the fret of a note, by the way changing its pitch.
  * This is mostly called when user hits ctrl + arrows.
  */
void Note::shiftFret(const int amount)
{

    if (fret + amount < 0)
    {
        m_pitch_ID -= amount;
        findStringAndFretFromNote();
    }
    else
    {
        // if the note would be out of bounds after applying this change, do not apply it.
        // An exception is granted if the current fret is under 0 and the user is trying to 'fix' this by making the fret number higher.
        if ( (fret + amount>35) and not (fret < 0 and amount > 0) ) return;
        // if ( (fret+amount < 0 or fret+amount>35) and not(fret < 0 and amount > 0) ) return;

        fret += amount;
        findNoteFromStringAndFret();
    }
}

// ----------------------------------------------------------------------------------------------------------

void Note::shiftString(const int amount)
{
    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    // don't perform if result would be invalid
    if (string + amount < 0) return;
    if (string + amount > (int)tuning->tuning.size()-1) return;
    if ((tuning->tuning)[string + amount] - m_pitch_ID < 0) return;
    if ((tuning->tuning)[string + amount] - m_pitch_ID > 35) return;

    string += amount;
    fret = (tuning->tuning)[string] - m_pitch_ID;

    findNoteFromStringAndFret();
}

// ----------------------------------------------------------------------------------------------------------

void Note::findStringAndFretFromNote()
{

    // find string that can hold the value with the smallest fret number possible
    int nearest  = -1;
    int distance = 1000;

    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    if (m_pitch_ID > (tuning->tuning)[ tuning->tuning.size()-1] )
    {
        // note is too low to appear on this tab, will have a negative fret number
        string = tuning->tuning.size()-1;
        fret = (tuning->tuning)[ tuning->tuning.size() - 1] - m_pitch_ID;
        return;
    }

    for (int n=0; n<(int)tuning->tuning.size(); n++)
    {

        // exact match (note can be played on a string at fret 0)
        if ((tuning->tuning)[n] == m_pitch_ID)
        {
            string = n;
            fret   = 0;
            return;
        }

        if ((tuning->tuning)[n] > m_pitch_ID)
        {
            if ((tuning->tuning)[n] - m_pitch_ID < distance)
            {
                nearest  = n;
                distance = (tuning->tuning)[n] - m_pitch_ID;
            }//end if
        }//end if
    }//next

    string = nearest;
    fret = distance;

}

// ----------------------------------------------------------------------------------------------------------

void Note::findNoteFromStringAndFret()
{
    GuitarTuning* tuning = m_track->getGuitarTuning();
    m_pitch_ID = (tuning->tuning)[string] - fret;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setSelected(const bool selected)
{
    m_selected = selected;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setVolume(const int vol)
{
    m_volume = vol;
}

// ----------------------------------------------------------------------------------------------------------

void Note::resize(const int ticks)
{
    if (m_end_tick + ticks <= m_start_tick) return; // refuse to shrink note so much that it disappears

    m_end_tick += ticks;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setEndTick(const int ticks)
{
    ASSERT_E(ticks,>=,0);

    m_end_tick = ticks;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Serialization
#endif

void Note::saveToFile(wxFileOutputStream& fileout)
{
    writeData( wxT("<note pitch=\"") + to_wxString(m_pitch_ID)  , fileout );
    writeData( wxT("\" start=\"")    + to_wxString(m_start_tick), fileout );
    writeData( wxT("\" end=\"")      + to_wxString(m_end_tick)  , fileout );
    writeData( wxT("\" volume=\"")   + to_wxString(m_volume)    , fileout );

    if (fret   != -1) writeData( wxT("\" fret=\"")   + to_wxString(fret),   fileout );
    if (string != -1) writeData( wxT("\" string=\"") + to_wxString(string), fileout );
    if (m_selected)
    {
        writeData( wxT("\" selected=\"") + wxString(m_selected ? wxT("true") : wxT("false")), fileout );
    }

    if (m_preferred_accidental_sign != -1)
    {
        writeData( wxT("\" accidentalsign=\"") + to_wxString(m_preferred_accidental_sign), fileout );
    }

    writeData( wxT("\"/>\n"), fileout );
}

// ----------------------------------------------------------------------------------------------------------

bool Note::readFromFile(irr::io::IrrXMLReader* xml)
{
    const char* pitch_c = xml->getAttributeValue("pitch");
    if (pitch_c != NULL)
    {
        m_pitch_ID = atoi( pitch_c );
    }
    else
    {
        m_pitch_ID = 60;
        std::cerr << "ERROR: Missing info from file: note pitch" << std::endl;
        return false;
    }

    const char* start_c = xml->getAttributeValue("start");
    if (start_c != NULL)
    {
        m_start_tick = atoi(start_c);
    }
    else
    {
        m_start_tick = 0;
        std::cerr << "ERROR: Missing info from file: note start" << std::endl;
        return false;
    }

    const char* end_c = xml->getAttributeValue("end");
    if (end_c != NULL)
    {
        m_end_tick = atoi(end_c);
    }
    else
    {
        m_end_tick = 0;
        std::cout << "ERROR: Missing info from file: note end" << std::endl;
        return false;
    }

    const char* volume_c = xml->getAttributeValue("volume");
    if (volume_c != NULL) m_volume = atoi(volume_c);
    else                  m_volume = 80;

    const char* accsign_c = xml->getAttributeValue("accidentalsign");
    if (accsign_c != NULL) m_preferred_accidental_sign = atoi(accsign_c);

    const char* fret_c = xml->getAttributeValue("fret");
    if (fret_c != NULL) fret = atoi(fret_c);
    else                fret = -1;

    const char* string_c = xml->getAttributeValue("string");
    if (string_c!=NULL) string = atoi(string_c);
    else                string = -1;

    const char* selected_c = xml->getAttributeValue("selected");
    if (selected_c != NULL)
    {
        if (strcmp(selected_c, "true") == 0)       m_selected = true;
        else if (strcmp(selected_c, "false") == 0) m_selected = false;
        else
        {
            std::cout << "Unknown keyword for attribute 'selected' in note: " << selected_c << std::endl;
            m_selected = false;
        }

    }
    else
    {
        m_selected = false;
    }

    return true;
}

// ----------------------------------------------------------------------------------------------------------

void Note::play(bool change)
{
    if (m_track->getSequence()->isImportMode()) return;

    const int playSetting = Core::playDuringEdit();

    if (playSetting == PLAY_NEVER) return;
    if (playSetting == PLAY_ON_CHANGE and not change) return;

    int durationMilli = (m_end_tick - m_start_tick)*60*1000 /
                        (m_track->getSequence()->getTempo() * m_track->getSequence()->ticksPerBeat());

    // FIXME(DESIGN): remove this 131-pitch ugliness
    if (m_track->getNotationType() == DRUM) 
    {
        PlatformMidiManager::get()->playNote(m_pitch_ID, m_volume, durationMilli, 9, m_track->getDrumKit() );
    }
    else
    {
        PlatformMidiManager::get()->playNote(131 - m_pitch_ID, m_volume, durationMilli, 0,
                                             m_track->getInstrument() );
    }
}

// ----------------------------------------------------------------------------------------------------------

bool Note::findNoteName(const int pitchID, Note12* note_12, int* octave)
{
    if (pitchID < 4)   return false;
    if (pitchID > 131) return false;
    *octave = 9 - pitchID/12;
    
    const int remainder = pitchID - int(pitchID/12) * 12;
    
    switch (remainder)
    {
        case 0:
            *note_12 = NOTE_12_B;
            ASSERT_E( findNotePitch(NOTE_7_B, NATURAL, *octave), ==, pitchID );
            return true;
        case 1:
            *note_12 = NOTE_12_A_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_A, SHARP, *octave), ==, pitchID );
            return true;
        case 2:
            *note_12 = NOTE_12_A;
            ASSERT_E( findNotePitch(NOTE_7_A, NATURAL, *octave), ==, pitchID );
            return true;
        case 3:
            *note_12 = NOTE_12_G_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_G, SHARP, *octave), ==, pitchID );
            return true;
        case 4:
            *note_12 = NOTE_12_G;
            ASSERT_E( findNotePitch(NOTE_7_G, NATURAL, *octave), ==, pitchID );
            return true;
        case 5:
            *note_12 = NOTE_12_F_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_F, SHARP, *octave), ==, pitchID );
            return true;
        case 6:
            *note_12 = NOTE_12_F;
            ASSERT_E( findNotePitch(NOTE_7_F, NATURAL, *octave), ==, pitchID );
            return true;
        case 7:
            *note_12 = NOTE_12_E;
            ASSERT_E( findNotePitch(NOTE_7_E, NATURAL, *octave), ==, pitchID );
            return true;
        case 8:
            *note_12 = NOTE_12_D_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_D, SHARP, *octave), ==, pitchID );
            return true;
        case 9:
            *note_12 = NOTE_12_D;
            ASSERT_E( findNotePitch(NOTE_7_D, NATURAL, *octave), ==, pitchID );
            return true;
        case 10:
            *note_12 = NOTE_12_C_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_C, SHARP, *octave), ==, pitchID );
            return true;
        case 11:
            *note_12 = NOTE_12_C;
            ASSERT_E( findNotePitch(NOTE_7_C, NATURAL, *octave), ==, pitchID );
            return true;
        default:
            return false;
    }
}


UNIT_TEST( TestFindNoteName )
{
    Note12 note;
    int octave;
    
    {
        const bool success = Note::findNoteName(131 - 69, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 71, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_B, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 64, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_E, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 63, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_D_SHARP, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 63, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_E_FLAT, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 57, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 3, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Note::findNoteName(131 - 93, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 6, "Conversion outputs the right octave");
    }
    
}

int Note::findNotePitch(Note7 note_7, PitchSign sharpness, const int octave)
{
    int note = 0;
    
    if      (note_7 == NOTE_7_A) note += 2;
    else if (note_7 == NOTE_7_B) note += 0;
    else if (note_7 == NOTE_7_C) note += 11;
    else if (note_7 == NOTE_7_D) note += 9;
    else if (note_7 == NOTE_7_E) note += 7;
    else if (note_7 == NOTE_7_F) note += 6;
    else if (note_7 == NOTE_7_G) note += 4;
    else
    {
        std::cerr << "[Note::findNotePitch] Invalid note: " << note_7 << std::endl;
        return 0;
    }
    
    if      (sharpness == SHARP) note -= 1;
    else if (sharpness == FLAT)  note += 1;
    
    const int returnv = note + (9 - octave)*12;
    ASSERT_E(returnv, >=, 0);
    ASSERT_E(returnv, <=, 130);
    return returnv;
}

UNIT_TEST( TestFindNotePitch )
{
    require(Note::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 69,
            "findNotePitch works correctly" );
    require(Note::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 71,
            "findNotePitch works correctly" );
    require(Note::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 64,
            "findNotePitch works correctly" );
    require(Note::findNotePitch(NOTE_7_D, SHARP, 4 /* octave */) == 131 - 63,
            "findNotePitch works correctly" );
    require(Note::findNotePitch(NOTE_7_E, FLAT, 4 /* octave */) == 131 - 63,
            "findNotePitch works correctly" );
    
    require(Note::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 3 /* octave */) == 131 - 57,
            "findNotePitch works correctly" );
    require(Note::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 6 /* octave */) == 131 - 93,
            "findNotePitch works correctly" );
}



