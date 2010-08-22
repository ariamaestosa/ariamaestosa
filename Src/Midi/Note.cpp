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
#include "GUI/GraphicalTrack.h"
#include "IO/IOUtils.h"
#include "Midi/Note.h"
#include "Pickers/MagneticGrid.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Utils.h"

//#include "Editors/GuitarEditor.h"

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
    Note::pitchID=pitchID_arg;
    Note::startTick=startTick_arg;
    Note::endTick=endTick_arg;
    Note::volume=volume_arg;
    Note::string=string_arg;
    Note::fret=fret_arg;

    m_track = parent;

    m_selected = false;

    preferred_accidental_sign = -1;
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
    string=string_arg;
    fret=fret_arg;
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


    if (string == -1 or fret == -1 or pitchID != tuning->tuning[string]-fret)
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

    if (fret+amount < 0)
    {
        pitchID -= amount;
        findStringAndFretFromNote();
    }
    else
    {
        // if the note would be out of bounds after applying this change, do not apply it.
        // An exception is granted if the current fret is under 0 and the user is trying to 'fix' this by making the fret number higher.
        if ( (fret+amount>35) and not(fret < 0 and amount > 0) ) return;
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
    if ((tuning->tuning)[string+amount] - pitchID < 0) return;
    if ((tuning->tuning)[string+amount] - pitchID > 35) return;

    string += amount;
    fret = (tuning->tuning)[string] - pitchID;

}

// ----------------------------------------------------------------------------------------------------------

void Note::findStringAndFretFromNote()
{

    // find string that can hold the value with the smallest fret number possible
    int nearest  = -1;
    int distance = 1000;

    GuitarTuning* tuning = m_track->getGuitarTuning();
    
    if (pitchID > (tuning->tuning)[ tuning->tuning.size()-1] )
    {
        // note is too low to appear on this tab, will have a negative fret number
        string = tuning->tuning.size()-1;
        fret = (tuning->tuning)[ tuning->tuning.size()-1] - pitchID;
        return;
    }

    for (int n=0; n<(int)tuning->tuning.size(); n++)
    {

        // exact match (note can be played on a string at fret 0)
        if ((tuning->tuning)[n] == pitchID)
        {
            string = n;
            fret   = 0;
            return;
        }

        if ((tuning->tuning)[n] > pitchID)
        {
            if ((tuning->tuning)[n] - pitchID < distance)
            {
                nearest  = n;
                distance = (tuning->tuning)[n] - pitchID;
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
    pitchID = (tuning->tuning)[string] - fret;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setSelected(const bool selected)
{
    m_selected = selected;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setVolume(const int vol)
{
    volume = vol;
}

// ----------------------------------------------------------------------------------------------------------

void Note::resize(const int ticks)
{
    if (endTick+ticks <= startTick) return; // refuse to shrink note so much that it disappears

    endTick += ticks;
}

// ----------------------------------------------------------------------------------------------------------

void Note::setEnd(const int ticks)
{
    ASSERT_E(ticks,>=,0);

    endTick = ticks;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Serialization
#endif

void Note::saveToFile(wxFileOutputStream& fileout)
{
    writeData( wxT("<note pitch=\"") + to_wxString(pitchID)  , fileout );
    writeData( wxT("\" start=\"")    + to_wxString(startTick), fileout );
    writeData( wxT("\" end=\"")      + to_wxString(endTick)  , fileout );
    writeData( wxT("\" volume=\"")   + to_wxString(volume)   , fileout );

    if (fret   != -1) writeData( wxT("\" fret=\"") + to_wxString(fret), fileout );
    if (string != -1) writeData( wxT("\" string=\"") + to_wxString(string), fileout );
    if (m_selected)
    {
        writeData( wxT("\" selected=\"") + wxString(m_selected ? wxT("true") : wxT("false")), fileout );
    }

    if (preferred_accidental_sign != -1)
    {
        writeData( wxT("\" accidentalsign=\"") + to_wxString(preferred_accidental_sign), fileout );
    }

    writeData( wxT("\"/>\n"), fileout );
}

// ----------------------------------------------------------------------------------------------------------

bool Note::readFromFile(irr::io::IrrXMLReader* xml)
{
    const char* pitch_c = xml->getAttributeValue("pitch");
    if (pitch_c != NULL)
    {
        pitchID = atoi( pitch_c );
    }
    else
    {
        pitchID = 60;
        std::cout << "ERROR: Missing info from file: note pitch" << std::endl;
        return false;
    }

    const char* start_c = xml->getAttributeValue("start");
    if (start_c != NULL)
    {
        startTick = atoi(start_c);
    }
    else
    {
        startTick = 0;
        std::cout << "ERROR: Missing info from file: note start" << std::endl;
        return false;
    }

    const char* end_c = xml->getAttributeValue("end");
    if (end_c != NULL)
    {
        endTick = atoi(end_c);
    }
    else
    {
        endTick = 0;
        std::cout << "ERROR: Missing info from file: note end" << std::endl;
        return false;
    }

    const char* volume_c = xml->getAttributeValue("volume");
    if (volume_c != NULL) volume = atoi(volume_c);
    else                  volume = 80;

    const char* accsign_c = xml->getAttributeValue("accidentalsign");
    if (accsign_c != NULL) preferred_accidental_sign = atoi(accsign_c);

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
    if (m_track->getSequence()->importing) return;

    const int playSetting = Core::playDuringEdit();

    if (playSetting == PLAY_NEVER) return;
    if (playSetting == PLAY_ON_CHANGE and not change) return;

    int durationMilli = (endTick - startTick)*60*1000 /
                        (m_track->getSequence()->getTempo() * m_track->getSequence()->ticksPerBeat());

    if (m_track->graphics->getEditorMode() == DRUM) 
    {
        PlatformMidiManager::get()->playNote( pitchID, volume, durationMilli, 9, m_track->getDrumKit() );
    }
    else
    {
        PlatformMidiManager::get()->playNote( 131-pitchID, volume, durationMilli, 0, m_track->getInstrument() );
    }
}


