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

namespace AriaMaestosa
{
    
    enum NoteToLevelType
    {
        DIRECT_ON_LEVEL,
        NATURAL_ON_LEVEL,
        SHARP_OR_FLAT
    };
    
    class NoteRenderInfo;
    class Note;
    class ScoreAnalyser;
    
    /**
     * When you switch to a key other C,flat or sharp signs need to appear next to the G and F keys.
     * These arrays give the X position relative to the left of the key of a flat or sharp sign for each note,
     * where the index of the array is of type note_7.
     */
    const int sign_dist = 5;
    const unsigned short int sharp_sign_x[] =
    {   5*sign_dist,
        7*sign_dist,
        2*sign_dist,
        4*sign_dist,
        6*sign_dist,
        1*sign_dist,
        3*sign_dist
    };
    const unsigned short int flat_sign_x[] =
    {   3*sign_dist,
        1*sign_dist,
        6*sign_dist,
        4*sign_dist,
        2*sign_dist,
        7*sign_dist,
        5*sign_dist
    };
    
    
    /**
     * Each note can be represented in many ways (its Y position on screen, its note_7, its note_12,
     * its midi pitch...). This class eases the burden by doing convertions and managing the location where
     * notes should appear and what signs should appear next to them, + it cares about the key.
     */
    class ScoreMidiConverter
    {
        /** 
          * indicates which notes on the staff are sharp/flat/natural,
          * where the array index is an element of the Note7 enum declared in ScoreEditor.h
          */
        PitchSign scoreNotesSharpness[7];
        bool going_in_sharps;
        bool going_in_flats;
        
        /** 0 = regular, +1 = alta, -1 = bassa */
        int octave_shift;
        
        // for accidentals
        bool accidentals;
        int accidentalScoreNotesSharpness[7];
        int accidentalsMeasure;         //!< because accidentals last only one measure
        
        int midiNoteToLevel[128];
        NoteToLevelType midiNoteToLevel_type[128];
        
        int levelToMidiNote[73];        //!< we need approximately 73 staff lines total to cover all midi notes
        int levelToNaturalNote[73];
        
        int middleCLevel;
        int ottavaAltaCLevel;
        int ottavaBassaCLevel;
        
    public:
        LEAK_CHECK();
        
        ScoreMidiConverter();
        void setNoteSharpness(Note7 note, PitchSign sharpness);
        
        /** @return are we using a key that will make flat signs appear next to the clef? */
        bool goingInSharps();
        
        /** @return are we using a key that will make sharp signs appear next to the clef? */
        bool goingInFlats();
        
        int  getMiddleCLevel();
        int  getScoreCenterCLevel();
        int  getOctaveShift();
        
        /** @return what sign should appear next to the key for this note? (FLAT, SHARP or PITCH_SIGN_NONE) */
        PitchSign getKeySigSharpnessSignForLevel(const unsigned int level);
        
        int getMidiNoteForLevelAndSign(const unsigned int level, int sharpness);
        
        /** @return what note is at given level */
        int levelToNote(const int level);
        
        /**
          * @param noteObj   the note object for which we want to know the level
          * @param[out] sign whether this note must appear sharp/flat/natural on this level
          * @return          on what level the given note will appear, and with what sign
          *                  or -1 on error
          */
        int noteToLevel(const Note* noteObj, PitchSign* sign=NULL);
        
        /** what is the name of the note played on this level? */
        int levelToNote7(const unsigned int level);
        
        /** called when key has changed, rebuilds conversion tables and other
         * data needed for all conversions and information requests this class provides
         */
        void updateConversionData();
        
        void setOctaveShift(int octaves);
        
        void resetAccidentalsForNewRender();
    };
    
    
    class ScoreEditor : public Editor
    {
        bool g_clef;
        bool f_clef;
        OwnerPtr<ScoreMidiConverter>  converter;
        OwnerPtr<ScoreAnalyser>  g_clef_analyser;
        OwnerPtr<ScoreAnalyser>  f_clef_analyser;
        
        bool musicalNotationEnabled, linearNotationEnabled;
        
    public:
        ScoreEditor(Track* track);
        ScoreMidiConverter* getScoreMidiConverter();
        
        void enableFClef(bool enabled);
        void enableGClef(bool enabled);
        
        /** get info about clefs
          * @return whether the G clef is enabled in this score */
        bool isGClefEnabled() const;
        
        /** get info about clefs
          * @return whether the F clef is enabled in this score */
        bool isFClefEnabled() const;
        
        void enableMusicalNotation(const bool enabled);
        void enableLinearNotation(const bool enabled);
        
        /** parameters are e.g. 5 sharps, 3 flats, etc. */
        void loadKey(const PitchSign sharpness_symbol, const int symbol_amount);
        
        void render(RelativeXCoord mousex_current, int mousey_current,
                    RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        void renderScore(ScoreAnalyser* analyser, const int silences_y);
        //void renderSilence(const int tick, const int tick_length, const int silences_y);
        void renderNote_pass1(NoteRenderInfo& renderInfo);
        void renderNote_pass2(NoteRenderInfo& renderInfo, ScoreAnalyser* analyser);
        
        void updatePosition(const int from_y, const int to_y, const int width, const int height, const int barHeight);
        
        void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                           RelativeXCoord mousex_initial, int mousey_initial);
        void mouseDown(RelativeXCoord, int y);
        void mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                       RelativeXCoord mousex_initial, int mousey_initial);
        void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial);
        void rightClick(RelativeXCoord x, int y);
        void TrackPropertiesDialog(RelativeXCoord mousex_current, int mousey_current,
                                   RelativeXCoord mousex_initial, int mousey_initial);
        
        int getYScrollInPixels();
        NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        void noteClicked(const int id);
        void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        void moveNote(Note& note, const int x_steps_to_move, const int y_steps_to_move);
        void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);
        
        /** user clicked on a sign in the track's header (called from 'SetAccidentalSign' Action) */
        void setNoteSign(const int sign, const int noteID);
        
        // int getYStep();
        // int getHalfNoteHeight();
        
        ~ScoreEditor();
    };
    
}

#endif

