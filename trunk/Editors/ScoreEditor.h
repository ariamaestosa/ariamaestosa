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


#ifndef _scoreeditor_
#define _scoreeditor_

#include "Editors/Editor.h"

#include <vector>

namespace AriaMaestosa {

enum NoteToLevelType
{
    DIRECT_ON_LEVEL,
    NATURAL_ON_LEVEL,
    SHARP_OR_FLAT
};
    
    class NoteRenderInfo;
    class Note;    

    /*
     * Each note can be represented in many ways (its Y position on screen, its note_7, its note_12, its midi pitch...)
     * This class eases the burden by doing convertions and managing the location where notes should appear and
     * what signs should appear next to them, + it cares about the key.
     */
    
    class ScoreMidiConverter
    {	
        // indicates which notes on the staff are sharp/flat/natural,
        // where the array index is an element of the NOTES enum declared in ScoreEditor.h
        int scoreNotesSharpness[7];
        bool going_in_sharps;
        bool going_in_flats;
        
        // for accidentals
        bool accidentals;
        int accidentalScoreNotesSharpness[7];
        int accidentalsMeasure; // because accidentals last only one measure
        
        int midiNoteToLevel[128];
        NoteToLevelType midiNoteToLevel_type[128]; // 
        //int showSignNextToNote[128];
        
        int levelToMidiNote[73]; // we need approximately 73 staff lines total to cover all midi notes
        int levelToNaturalNote[73];
        
        int middleCLevel;
public:
        LEAK_CHECK(ScoreMidiConverter);
        
        ScoreMidiConverter();
        void setNoteSharpness(NOTES note, int sharpness);
        bool goingInSharps();
        bool goingInFlats();
        int getMiddleCLevel();
        int getKeySigSharpnessSignForLevel(const unsigned int level);
       // int getSharpnessSignForMidiNote(const unsigned int note);
        int getMidiNoteForLevelAndSign(const unsigned int level, int sharpness);
        int levelToNote(const int level);
        int noteToLevel(Note* noteObj, int* sign=NULL);
        int levelToNote7(const unsigned int level);
        void updateConversionData();
        
        void resetAccidentalsForNewRender();
    };
    
	
    class ScoreEditor : public Editor
    {
        bool GKey;
        bool FKey;
        PTR_HOLD(ScoreMidiConverter, converter);
        
        bool musicalNotationEnabled, linearNotationEnabled;
        
public:
            
            
        ScoreEditor(Track* track);
        
        ScoreMidiConverter* getScoreMidiConverter();
        
        void enableMusicalNotation(const bool enabled);
        void enableLinearNotation(const bool enabled);
        void loadKey(const int sharpness_symbol, const int symbol_amount);
        
        void render(RelativeXCoord mousex_current, int mousey_current,
                    RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void updatePosition(const int from_y, const int to_y, const int width, const int height, const int barHeight);
        
        void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                           RelativeXCoord mousex_initial, int mousey_initial);
        void mouseDown(RelativeXCoord, int y);
        void mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                       RelativeXCoord mousex_initial, int mousey_initial);
        void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial);
        void rightClick(RelativeXCoord x, int y);
        void mouseExited(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial);
        
        int getYScrollInPixels();
        NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        void noteClicked(const int id);
        void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        void moveNote(Note& note, const int x_steps_to_move, const int y_steps_to_move);
        void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);
		
        void renderSilence(const int tick, const int tick_length);
        void renderNote_pass1(NoteRenderInfo& renderInfo);
        void renderNote_pass2(NoteRenderInfo& renderInfo);
		
        // called from 'SetAccidentalSign' Action
        void setNoteSign(const int sign, const int noteID);
        
        int getYStep();
        int getHalfNoteHeight();
        
        ~ScoreEditor();
    };
    
}

#endif

