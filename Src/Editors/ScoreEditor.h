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


#ifndef __SCORE_EDITOR_H__
#define __SCORE_EDITOR_H__

#include "Editors/Editor.h"

#include <wx/intl.h>

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
    class Track;
    class ScoreAnalyser;
    
    const int sign_dist = 5;
    
    /**
     * When you switch to a key other C,flat or sharp signs need to appear next to the G and F keys.
     * These arrays give the X position relative to the left of the key of a flat or sharp sign for each note,
     * where the index of the array is of type note_7.
     */
    const unsigned short int sharp_sign_x[] =
    {   5*sign_dist,
        7*sign_dist,
        2*sign_dist,
        4*sign_dist,
        6*sign_dist,
        1*sign_dist,
        3*sign_dist
    };
    
    /**
     * When you switch to a key other C,flat or sharp signs need to appear next to the G and F keys.
     * These arrays give the X position relative to the left of the key of a flat or sharp sign for each note,
     * where the index of the array is of type note_7.
     */
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
     * @ingroup editors
     */
    class ScoreMidiConverter
    {
        /** 
          * indicates which notes on the staff are sharp/flat/natural,
          * where the array index is an element of the Note7 enum declared in ScoreEditor.h
          */
        PitchSign m_score_notes_sharpness[7];
        bool m_going_in_sharps;
        bool m_going_in_flats;
        
        /** 0 = regular, +1 = alta, -1 = bassa */
        int m_octave_shift;
        
        // for accidentals
        bool m_accidentals;
        int m_accidental_score_notes_sharpness[7];
        int m_accidentals_measure;         //!< because accidentals last only one measure
        
        int m_midi_note_to_level[128];
        NoteToLevelType m_midi_note_to_level_type[128];
        
        int m_level_to_midi_note[73];        //!< we need approximately 73 staff lines total to cover all midi notes
        int m_level_to_natural_note[73];
        
        int m_middle_C_level;
        int m_ottava_alta_C_level;
        int m_ottava_bassa_C_level;
        
        GraphicalSequence* m_sequence;
        
    public:
        
        LEAK_CHECK();
        
        ScoreMidiConverter(GraphicalSequence* parent);
        void setNoteSharpness(Note7 note, PitchSign sharpness);
        
        /** @return are we using a key that will make flat signs appear next to the clef? */
        bool goingInSharps() const { return m_going_in_sharps; }
        
        /** @return are we using a key that will make sharp signs appear next to the clef? */
        bool goingInFlats() const { return m_going_in_flats; }
        
        int  getMiddleCLevel() const { return m_middle_C_level; }
        int  getScoreCenterCLevel() const;
        int  getOctaveShift() const { return m_octave_shift; }
        
        /** @return what sign should appear next to the key for this note? (FLAT, SHARP or PITCH_SIGN_NONE) */
        PitchSign getKeySigSharpnessSignForLevel(const unsigned int level) const;
        
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
        
        int noteToLevel(const Note* noteObj, const int note, PitchSign* sign=NULL);
        
        /** what is the name of the note played on this level? */
        int levelToNote7(const unsigned int level) const;
        
        /** called when key has changed, rebuilds conversion tables and other
         * data needed for all conversions and information requests this class provides
         */
        void updateConversionData();
        
        void setOctaveShift(int octaves);
        
        void resetAccidentalsForNewRender();
    };
    
    
    /**
      * @brief note editor using score (staff) notation
      * @ingroup editors
      */
    class ScoreEditor : public Editor
    {
        bool m_g_clef;
        bool m_f_clef;
        OwnerPtr<ScoreMidiConverter>  m_converter;
        OwnerPtr<ScoreAnalyser>  m_g_clef_analyser;
        OwnerPtr<ScoreAnalyser>  m_f_clef_analyser;
        
        bool m_musical_notation_enabled;
        bool m_linear_notation_enabled;
        
        /** Used when clicking on notes in the left part to hear them */
        int m_clicked_note;
        
        /** helper method for rendering */
        void renderScore(ScoreAnalyser* analyser, const int silences_y);
        
        /** helper method for rendering */
        void renderNote_pass1(NoteRenderInfo& renderInfo);
        
        /** helper method for rendering */
        void renderNote_pass2(NoteRenderInfo& renderInfo, ScoreAnalyser* analyser);
        
    public:
        
        ScoreEditor(GraphicalTrack* track);
        ~ScoreEditor();

        ScoreMidiConverter* getScoreMidiConverter() { return m_converter; }
        const ScoreMidiConverter* getScoreMidiConverter() const { return m_converter; }

        void enableFClef(bool enabled);
        void enableGClef(bool enabled);
        
        /** get info about clefs
          * @return whether the G clef is enabled in this score */
        bool isGClefEnabled() const { return m_g_clef; }
        
        /** get info about clefs
          * @return whether the F clef is enabled in this score */
        bool isFClefEnabled() const { return m_f_clef; }
        
        void enableMusicalNotation(const bool enabled);
        void enableLinearNotation(const bool enabled);
        
        /** Called when user changes key. parameters are e.g. 5 sharps, 3 flats, etc. */
        virtual void onKeyChange(const int symbol_amount, const KeyType sharpness_symbol);
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
                
        /** event callback from base class */
        virtual void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                   RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback from base class */
        virtual void mouseDown(RelativeXCoord, int y);
        
        /** event callback from base class */
        virtual void mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                               RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback from base class */
        virtual void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                             RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback from base class */
        virtual void rightClick(RelativeXCoord x, int y);
        
        virtual void mouseExited(RelativeXCoord mousex_current, int mousey_current,
                                 RelativeXCoord mousex_initial, int mousey_initial);
        
        /** implemented from base class Editor's required interface */
        virtual int getYScrollInPixels();
        
        /** user clicked on a sign in the track's header (called from 'SetAccidentalSign' Action) */
        void setNoteSign(const int sign, const int noteID);
        
        /** implemented from base class Editor's required interface */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        
        /** implemented from base class Editor's required interface */
        virtual void noteClicked(const int id);
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        
        /** implemented from base class Editor's required interface */
        virtual void moveNote(Note& note, const int x_steps_to_move, const int y_steps_to_move);
        
        /** implemented from base class Editor's required interface */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial);
        
        /** implemented from base class Editor's required interface */
        virtual wxString getName() const { return _("Score Editor"); }
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snappedX, const int mouseY)
        {
            // not supported in this editor
            assert(false);
        }

        virtual NotationType getNotationType() const { return SCORE; }
        
        virtual void processKeyPress(int keycode, bool commandDown, bool shiftDown);
    };
    
}

#endif

