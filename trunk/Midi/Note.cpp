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

#include "main.h"
#include "Midi/Note.h"
#include "GUI/MainFrame.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/KeyboardEditor.h"
#include "Editors/ScoreEditor.h"
#include "Editors/GuitarEditor.h"
#include "Editors/DrumEditor.h"
#include "Config.h"
#include "IO/IOUtils.h"
#include "Pickers/MagneticGrid.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/InstrumentChoice.h"
#include "GUI/GLPane.h"


#include <iostream>

namespace AriaMaestosa {
	
int Note::getString()
{
    if(string==-1) findStringAndFretFromNote();
    return string;
}

int Note::getFret()
{
    if(fret==-1) findStringAndFretFromNote();
    return fret;
}

void Note::setFret(int i)
{
    fret=i;
    findNoteFromStringAndFret();
}

void Note::setStringAndFret(int string_arg, int fret_arg)
{
    string=string_arg;
	fret=fret_arg;
	findNoteFromStringAndFret();
}

Note::Note(GraphicalTrack* parent,
		   const int pitchID_arg,
		   const int startTick_arg,
		   const int endTick_arg,
		   const int volume_arg,
		   const int string_arg,
		   const int fret_arg)
{
    
	INIT_LEAK_CHECK();
	
    Note::pitchID=pitchID_arg;
    Note::startTick=startTick_arg;
    Note::endTick=endTick_arg;
    Note::volume=volume_arg;
    Note::string=string_arg;
    Note::fret=fret_arg;
    
    gtrack = parent;

    selected=false;
    
    
}

void Note::checkIfStringAndFretMatchNote(const bool fixStringAndFret)
{
	// if note is placed on a string that doesn't exist (anymore)
    if(fixStringAndFret and string > (int)gtrack->guitarEditor->tuning.size()-1)
	{
		findStringAndFretFromNote();
	}
	
	
    if(string==-1 || fret==-1 || pitchID != gtrack->guitarEditor->tuning[string]-fret)
	{
        if(fixStringAndFret) findStringAndFretFromNote();
        else findNoteFromStringAndFret();
    }
    
}

void Note::setParent(GraphicalTrack* parent)
{
    gtrack = parent;   
}

/*
 * In guitar editor, changes the number on the fret of a note, by the way changing its pitch. This is mostly called when user hits ctrl + arrows.
 */

void Note::shiftFret(const int amount)
{
	
	if(fret+amount < 0)
	{
		pitchID -= amount;
		findStringAndFretFromNote();
	}
	else
	{
		// if the note would be out of bounds after applying this change, do not apply it.
		// An exception is granted if the current fret is under 0 and the user is trying to 'fix' this by making the fret number higher.
		if( (fret+amount>35) and not(fret < 0 and amount > 0) ) return;
		// if( (fret+amount < 0 or fret+amount>35) and not(fret < 0 and amount > 0) ) return;
		
		fret += amount;
		findNoteFromStringAndFret();
	}
}

void Note::shiftString(const int amount)
{
        
    // don't perform if result would be invalid
    if(string + amount < 0) return;
    if(string + amount > (int)gtrack->guitarEditor->tuning.size()-1) return;
    if((gtrack->guitarEditor->tuning)[string+amount] - pitchID < 0) return;
    if((gtrack->guitarEditor->tuning)[string+amount] - pitchID > 35) return;
    
    string += amount;
    fret = (gtrack->guitarEditor->tuning)[string] - pitchID;

}

void Note::findStringAndFretFromNote()
{
    
    // find string that can hold the value with the smallest fret number possible
    int nearest=-1;
    int distance=1000;

	if(pitchID > (gtrack->guitarEditor->tuning)[ gtrack->guitarEditor->tuning.size()-1] )
	{
		// note is too low to appear on this tab, will have a negative fret number
		string = gtrack->guitarEditor->tuning.size()-1;
		fret = (gtrack->guitarEditor->tuning)[ gtrack->guitarEditor->tuning.size()-1] - pitchID;
		return;
	}
	
    for(int n=0; n<(int)gtrack->guitarEditor->tuning.size(); n++)
	{
        
		// exact match (note can be played on a string at fret 0)
        if( (gtrack->guitarEditor->tuning)[n] == pitchID)
		{
            string=n;
            fret=0;
            return;
        }
        
        if( (gtrack->guitarEditor->tuning)[n] > pitchID)
		{
            if((gtrack->guitarEditor->tuning)[n] - pitchID < distance)
			{
                nearest=n;
                distance=(gtrack->guitarEditor->tuning)[n] - pitchID;
            }//end if
        }//end if
    }//next
    
    string = nearest;
    fret = distance;
    
}

void Note::findNoteFromStringAndFret()
{
    pitchID = (gtrack->guitarEditor->tuning)[string] - fret;
}

void Note::setSelected(const bool selected)
{
    Note::selected = selected;
}

bool Note::isSelected()
{
    return selected;   
}

void Note::setVolume(const int vol)
{
    volume=vol;   
}

/*
void Note::move(const int relativeX, const int relativeY, const int mode)
{
    
    if(startTick+relativeX < 0) return; // refuse to move before the start
    
    startTick+=relativeX;
    endTick+=relativeX;
    
	// FIXME - belongs to the editors, probably
    if(mode==GUITAR)
	{
        if(string+relativeY<0 or string+relativeY>(int)gtrack->guitarEditor->tuning.size()-1)
            return; // note will end on a string that doesn't exist if we move it like that
        
        string += relativeY;
        findNoteFromStringAndFret();
    }
	else if(mode==SCORE)
	{
		if(relativeY==0) return;
		
		GLPane* glPane = getGLPane();
		
		if(glPane->isSelectMorePressed() or glPane->isCtrlDown())
		{
			pitchID+=relativeY;
		}
		else
		{
			ScoreMidiConverter* conv = gtrack->scoreEditor->getScoreMidiConverter();
			int noteLevel = conv->noteToLevel(pitchID);
			noteLevel += relativeY;
			pitchID = conv->levelToNote(noteLevel);
		}
	}
	else if(mode==DRUM)
	{
        
        if(relativeY==0) return;
        
        //
        //  Since the drums are not in midi order on screen, the procedure to move drums up or down is to:
        //      1. Find the screen location of this midi key
        //      2. Move the drum in screen location
        //      3. Find the new midi key at the new screen location
        //
        
        assert(pitchID>=0);
        assert(pitchID<128);
        
        // find where on screen this particular drum is drawn (their screen position is not in the same order as the midi order)
        int newVectorLoc = gtrack->drumEditor->midiKeyToVectorID[pitchID];
        if(newVectorLoc == -1) return;
        
        // move to the new location (in screen order)
        if(newVectorLoc + relativeY < 0 or
           newVectorLoc + relativeY > (int)gtrack->drumEditor->drums.size()-1 ) return; // discard moves that would result in an out-of-bound note
        
        newVectorLoc += relativeY;
        
        // skip sections
        while(gtrack->drumEditor->drums[newVectorLoc].midiKey==-1)
		{

            newVectorLoc+=(relativeY/abs(relativeY)); // keep the same sign, but only move 1 step at a time from now on
            
			// discard moves that would result in an out-of-bound note
            if(newVectorLoc < 0 or newVectorLoc > (int)gtrack->drumEditor->drums.size()-1 )
				return;
        }
        
        // find the midi key at the new location
		const int new_pitchID = gtrack->drumEditor->drums[newVectorLoc].midiKey;
		if(new_pitchID < 0 or new_pitchID > 127)
			return; // invalid location - discard
		
        pitchID = new_pitchID;

    }
	else
	{
        pitchID+=relativeY;
    }
    
}*/

void Note::resize(const int ticks)
{
    if(endTick+ticks <= startTick) return; // refuse to shrink note so much that it disappears
    
    endTick+=ticks;
}

void Note::setEnd(const int ticks)
{
    assertExpr(ticks,>=,0);
    
    endTick = ticks;
}

// serialization
void Note::saveToFile(wxFileOutputStream& fileout)
{
	
	writeData( wxT("<note pitch=\"") + to_wxString(pitchID), fileout );
	writeData( wxT("\" start=\"") + to_wxString(startTick), fileout );
	writeData( wxT("\" end=\"") + to_wxString(endTick), fileout );
	writeData( wxT("\" volume=\"") + to_wxString(volume), fileout );
	writeData( wxT("\" fret=\"") + to_wxString(fret), fileout );
	writeData( wxT("\" string=\"") + to_wxString(string), fileout );
	writeData( wxT("\" selected=\"") + wxString( selected?wxT("true"):wxT("false")), fileout );
	writeData( wxT("\"/>\n"), fileout );

}

bool Note::readFromFile(irr::io::IrrXMLReader* xml)
{
	
	const char* pitch_c = xml->getAttributeValue("pitch");
	if(pitch_c!=NULL) pitchID = atoi( pitch_c );
	else
	{
		pitchID = 60;
		std::cout << "ERROR: Missing info from file: note pitch" << std::endl;
		return false;
	}
	
	const char* start_c = xml->getAttributeValue("start");
	if(start_c!=NULL) startTick = atoi(start_c);
	else
	{
		startTick = 0;
		std::cout << "ERROR: Missing info from file: note start" << std::endl;
		return false;
	}
	
	const char* end_c = xml->getAttributeValue("end");
	if(end_c!=NULL) endTick = atoi(end_c);
	else
	{
		endTick = 0;
		std::cout << "ERROR: Missing info from file: note end" << std::endl;
		return false;
	}
	
	const char* volume_c = xml->getAttributeValue("volume");
	if(volume_c!=NULL) volume = atoi(volume_c);
	else
	{
		volume = 80;
		std::cout << "Missing info from file: note volume" << std::endl;
	}

	
	const char* fret_c = xml->getAttributeValue("fret");
	if(fret_c!=NULL) fret = atoi(fret_c);
	else
	{
		fret = -1;
		std::cout << "Missing info from file: note fret" << std::endl;
	}
	
	const char* string_c = xml->getAttributeValue("string");
	if(string_c!=NULL) string = atoi(string_c);
	else
	{
		string = -1;
		std::cout << "Missing info from file: note string" << std::endl;
	}
	
	const char* selected_c = xml->getAttributeValue("selected");
	if(selected_c!=NULL)
	{
		if( !strcmp(selected_c, "true") ) selected = true;
		else if( !strcmp(selected_c, "false") ) selected = false;
		else
		{
			std::cout << "Unknown keyword for attribute 'selected' in note: " << selected_c << std::endl;
			selected = false;
		}
		
	}
	else
	{
		selected = false;
		std::cout << "Missing info from file: note selected" << std::endl;
	}
			
	
	return true;
}

/*
 * Requests that note be played.
 * Change will be true if the sound of the note has been changed. This, with user settings, will determine if it is needed to play note or not.
 */

void Note::play(bool change)
{
	
	if(gtrack->sequence->importing) return;
	
	const int play = getMainFrame()->play_during_edit;
	
	if(play == PLAY_NEVER) return;
	if(play == PLAY_ON_CHANGE and !change) return;
	
	int durationMilli = (endTick-startTick)*60*1000 / ( gtrack->sequence->getTempo() * gtrack->sequence->ticksPerBeat() );
	
	if(gtrack->editorMode == DRUM) PlatformMidiManager::playNote( pitchID, volume, durationMilli, 9, gtrack->track->getDrumKit() );
	else PlatformMidiManager::playNote( 131-pitchID, volume, durationMilli, 0, gtrack->track->getInstrument() );
	
}

}
