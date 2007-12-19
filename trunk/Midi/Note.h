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

#ifndef _note_h_
#define _note_h_

#include <vector>
#include "Config.h"

#include "wx/wfstream.h"

#include "irrXML/irrXML.h"

namespace AriaMaestosa {
	
class GraphicalTrack; // forward

class Note {
	
	DECLARE_LEAK_CHECK();

public:
    
	// The pitch used by Aria is not a midi pitch, it is (131 - midi pitch), for the simple reason that high notes
	// being drawn near the top of the screen, and low notes near the bottom, this reversed order simplifies drawing
	// routines (and in the code we draw much more often than we play)
	// middle C being 71
	int pitchID;
		
	int startTick, endTick, volume;
    
    // for guitar mode
    int string, fret;
    
     bool selected;
     
    //std::vector<int> *tuning;
    GraphicalTrack* gtrack;
    
    int getString();
	void setStringAndFret(int string_arg, int fret_arg);
    int getFret();
    void setFret(int i);
    
    void setSelected(const bool selected);
    bool isSelected();
    
    //Note(GraphicalTrack* parent, const int pitchID=-1, const int startTick=-1, const int endTick=-1, const int volume=-1);
    Note(GraphicalTrack* parent, const int pitchID=-1, const int startTick=-1, const int endTick=-1, const int volume=-1, const int string=-1, const int fret=-1); // guitar mode only

    void setParent(GraphicalTrack* parent);
    
    void setVolume(int vol);
    void resize(const int ticks);
    void setEnd(const int ticks);
    
    // guitar mode only
    void shiftFret(const int amount);
    void shiftString(const int amount);
    void findStringAndFretFromNote();
    void findNoteFromStringAndFret();
    void checkIfStringAndFretMatchNote(const bool fixStringAndFret);
    
	void play(bool change);
	
    // serialization
    void saveToFile(wxFileOutputStream& fileout);
    bool readFromFile(irr::io::IrrXMLReader* xml);
};

}
#endif
