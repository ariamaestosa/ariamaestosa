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

#ifndef _scoreanalyser_
#define _scoreanalyser_


/**
  * @defgroup analysers
  */

#include <vector>

#include "Analysers/SilenceAnalyser.h"
//#include "Editors/Editor.h"

namespace AriaMaestosa
{
    class MeasureData;
    class Editor;
    
    enum STEM
    {
        STEM_UP,
        STEM_DOWN,
        STEM_NONE
    };
    
    /**
     * can have 2 uses : describing note and describing visual sign
     * e.g. F# will be described as SHARP as note, but if you put it in a score where all Fs are #,
     * its visible sign will be PITCH_SIGN_NONE. When describing a note's sign, use either NATURAL or
     * PITCH_SIGN_NONE. When describing a note's pitch, PITCH_SIGN_NONE is not to be used
     */
    enum PitchSign
    {
        SHARP = 0,
        FLAT = 1,
        NATURAL = 2,
        PITCH_SIGN_NONE = 3
    };
    
    /** enum to denotate a note's name (A, B, C, ...) regardless of any accidental it may have */
    enum Note7
    {
        NOTE_7_A = 0,
        NOTE_7_B = 1,
        NOTE_7_C = 2,
        NOTE_7_D = 3,
        NOTE_7_E = 4,
        NOTE_7_F = 5,
        NOTE_7_G = 6
    };
    
    /** enum to denotate a note's name (A, B, C, ...) INCLUDING of any accidental it may have.
     * What interests us here is the pitch, so we consider A# is the same as Bb. */
    enum Note12
    {
        NOTE_12_A       = 0,
        NOTE_12_A_SHARP = 1,
        NOTE_12_B       = 2,
        NOTE_12_C       = 3,
        NOTE_12_C_SHARP = 4,
        NOTE_12_D       = 5,
        NOTE_12_D_SHARP = 6,
        NOTE_12_E       = 7,
        NOTE_12_F       = 8,
        NOTE_12_F_SHARP = 9,
        NOTE_12_G       = 10,
        NOTE_12_G_SHARP = 11
    };
    
    /** synonym; @see Note12 */
    const Note12 NOTE_12_A_FLAT = NOTE_12_G_SHARP;
    
    /** synonym; @see Note12 */
    const Note12 NOTE_12_B_FLAT = NOTE_12_A_SHARP;
    
    /** synonym; @see Note12 */
    const Note12 NOTE_12_D_FLAT = NOTE_12_C_SHARP;
    
    /** synonym; @see Note12 */
    const Note12 NOTE_12_E_FLAT = NOTE_12_D_SHARP;
    
    /** synonym; @see Note12 */
    const Note12 NOTE_12_G_FLAT = NOTE_12_F_SHARP;
    
    //FIXME: what about flats?
    static const wxString NOTE_12_NAME[] =
    {
        wxT("A"),
        wxT("A#"),
        wxT("B"),
        wxT("C"),
        wxT("C#"),
        wxT("D"),
        wxT("D#"),
        wxT("E"),
        wxT("F"),
        wxT("F#"),
        wxT("G"),
        wxT("G#")
    };
    
    /**
      * @brief Contains info about how to render a single visible note.
      *
      * A vector of these objects is created inthe first rendering pass.
      * This vector contains one of these for each visible note. This vector is then analysed and used in the
      * next rendering passes. The object starts with a few info fields, passed in the constructors, and builds/tweaks
      * the others as needed in the next passes. The vector is destroyed and recreated with each render.
      *
      * A few utility methods will ease setting some variables, but they are usually changed directly from code.
      * @ingroup analysers
      */
    // FIXME: make less of these public!
    class NoteRenderInfo
    {
        /**
          * used to display ties (display a tie between this note and specified tick).
          * a value of -1 means no tie.
          */
        int m_tied_with_tick;
        
        /** used if stem_type == STEM_NONE, otherwise tie location is determined with stem_type */
        bool m_tie_up;
        
        int m_y;
        
        /** location and duration of note */
        int m_tick, m_tick_length;
        
    public:
        /** for very short notes, e.g. drum notes. Note will appear as a X. */
        bool m_instant_hit;
        
        /** if note has a dot */
        bool m_dotted;
        
        /** Whether this "note" is a chord */
        bool m_chord;
        
        /** Since a chord contains many notes, keep info about the highest and lowest note of the chord
         * Only valid if 'chord' is true. */
        int m_min_chord_level, m_max_chord_level;
        
        /** Number of flags on note stem. e.g. a 1/4 will have none, a 1/8 has 1, a 1/16 has 2, etc. */
        int m_flag_amount;
        
        /** Whether this note has a hollow head (like 1/1 and 1/2 figures) or a "black" (filled) head */
        bool m_hollow_head;
        
        /** Whether this note is selected in the score editor */
        bool m_selected;
        
        /** is stem up, down? or is there no stem? */
        STEM m_stem_type;
        
        /** Whether to draw the stem. FIXME : stem_type == STEM_NONE can already carry this info!! */
        bool m_draw_stem;
        
        /** vertical position of the note, in abstract level units */
        int m_level;
        
        /** pitch ID of the note */
        int m_pitch;
        
        /** measure where the note begins */
        int m_measure_begin;
        
        /** measure where the note ends */
        int m_measure_end;
        
        /** which alteration sigh to use, if any (sharp, flat, natural or none) */
        PitchSign m_sign;
        
        /** Whether this note has a triplet duration */
        bool m_triplet;
        
        /** Whether this particular note is responsible to draw a "triplet arc" with a 3 in it
         * (this is a separate options because many notes (e.g. 3) can share the same triplet arc)
         */
        bool m_draw_triplet_sign;
        
        /** where to display the "triplet arc" than contains a "3" */
        int m_triplet_arc_tick_start, m_triplet_arc_tick_end, m_triplet_arc_level;
        
        /** Whether the "triplet arc" is rendered above or below its Y base coordinate */
        bool m_triplet_show_above;
        
        // beams
        // FIXME - is beam_show_above really necessary, since it's always the same direction as stem_type?
        bool m_beam_show_above, m_beam;
        
        /**
          * if beam is true, the renderer will draw a beam between the end of this note's stem and the
          * location specified by these variables.
          */
        int m_beam_to_tick;
        PitchSign m_beam_to_sign; //!< sign of the note we beam to. (used for printing where it's not linear)
        float m_beam_to_level;
        
        /**
         * If != -1, the renderer will use this y as stem end instead of calculating it itself.
         * Use ScoreAnalyser::getStemTo for a higher-level getter
         */
        float m_stem_y_level;
        
        
        NoteRenderInfo(int tick, int level, int tick_length, PitchSign sign, const bool selected, int pitch,
                       const int measureBegin, const int measureEnd);
        
        static NoteRenderInfo factory(int tick, int level, int tick_length, PitchSign sign, const bool selected, int pitch,
                                      MeasureData* md);
        
        void tieWith(NoteRenderInfo& renderInfo);
        void tieWith(const int tick);
        void setTieUp(const bool up);
        int  getTiedToTick() const { return m_tied_with_tick; }
        bool isTieUp      () const { return (m_stem_type == STEM_NONE ? m_tie_up : m_stem_type != STEM_UP); }
        
        void setTriplet();
        
        // those two will be the same for non-chords.
        int getBaseLevel();
        int getStemOriginLevel();
        
        /**
         * In an attempt t be view-independant, ScoreAnalyser tries to store Y locations as levels
         * and never as direct coordinates. However, to avoid the overhead of converting frequently
         * from level to coordinate, the renderer is given the option to store the Y coordinate
         * inside the noteRenderInfo.
         */
        const int getY() const { return m_y; }
        
        /**
         * In an attempt t be view-independant, ScoreAnalyser tries to store Y locations as levels
         * and never as direct coordinates. However, to avoid the overhead of converting frequently
         * from level to coordinate, the renderer is given the option to store the Y coordinate
         * inside the noteRenderInfo.
         */
        void setY(const int newY);
        
        /** Get start of notes in midi ticks */
        int getTick() const { return m_tick; }
        
        /** Get length of note in midi ticks */
        int getTickLength() const { return m_tick_length; }
        
        void setLength(const int newLength)
        {
            ASSERT_E(newLength, >=, 0);
            m_tick_length = newLength;
        }
    };
        
    class BeamGroup;
    
    /**
      * @brief analyses a bunch of notes to make a score out of them
      *
      * ScoreAnalyser contains what is necessary to decide how to render the score. Editors like Keyboard or
      * Drum draw all notes seperately from each other, so there rendering is simple and can be done in a single
      * render pass. Score, however, features much more complex interaction. For instance, draw a single triplet
      * sign for 3 consecutive triplets, beam notes, draw chords correctly, etc. The first render pass of the
      * score editor is where the head of each note is rendered.
      * In this pass, a NoteRenderInfo object is added to a vector for each visible note. These objects contain
      * basic information on each note. The vector of visible notes is then passed to the ScoreAnalyser functions.
      * These functions will analyse notes and modify the NoteRenderInfo objects as needed so that they render
      * nicely in the next rendering passes of the score editor.
      *
      * @ingroup analysers
      */
    class ScoreAnalyser : public SilenceAnalyser::INoteSource
    {
        friend class BeamGroup;
        
        // REMEMBER: on adding new members, don't forget to update the cloning code in 'getSubset'
        Editor* m_editor;
        int m_stem_pivot;
        
        float stem_height;
        float min_stem_height;
        
        void addToVector( NoteRenderInfo& renderInfo, const bool recursion );
        
        ScoreAnalyser() {}
        
    public:
        LEAK_CHECK();
        
        std::vector<NoteRenderInfo> noteRenderInfo;
        
        ScoreAnalyser(Editor* parent, int stemPivot);
        
        virtual ~ScoreAnalyser() {}
        
        /**
         * @return a new ScoreAnalyser, that contains a subset of the current one.
         * @note   The returned pointer must be freed.
         */
        ScoreAnalyser* getSubset(const int fromTick, const int toTick) const;
        
        float getStemTo(NoteRenderInfo& note);
        
        /** @brief call when you're done rendering the current frame, to prepare to render the next */
        void clearAndPrepare();
        
        /** @brief Add a note to the score analyser. */
        void addToVector( NoteRenderInfo& renderInfo );
        
        /**
         * @brief the main function of ScoreAnalyser, where everything start
         * 
         * This function takes a vector containing information about visible notes.
         * Its job is to analyse them and fill missing data in the contained objects
         * so that they can be rendered correctly on a score.
         *
         * Note that note heads are assumed to be already rendered at this point; this
         * will set up stems, beams, triplet signs, etc... but might very well remove
         * some note heads, especially in chords (FIXME?)
         */
        void analyseNoteInfo();
        
        /** @brief set the level below which the stem is up, and above which it is down */
        void setStemPivot(const int level);
        
        /**
         * @brief Puts notes in time order.
         * Notes that have no stems go last so that they don't disturb note grouping in chords.
         * Note that the 'analyseNoteInfo' method will automatically call this; so this method
         * might be useful only if you want to iterate through the NoteRenderInfo objects of
         * the analyser before calling 'analyseNoteInfo'.
         */
        void putInTimeOrder();
        
        /** Get whether a note at the given level should have its stem up (if false, then down) */
        bool stemUp(const int level) const { return level >= m_stem_pivot; }
        
        /** @brief implementing the INoteSource interface for the SilenceAnalyser to use */
        virtual int  getNoteCount() const
        {
            return noteRenderInfo.size();
        }
        
        /** @brief implementing the INoteSource interface for the SilenceAnalyser to use */
        virtual int  getBeginMeasure(const int noteID) const
        {
            return noteRenderInfo[noteID].m_measure_begin;
        }
        
        /** @brief implementing the INoteSource interface for the SilenceAnalyser to use */
        virtual int  getStartTick(const int noteID) const
        {
            return noteRenderInfo[noteID].getTick();
        }
        
        /** @brief implementing the INoteSource interface for the SilenceAnalyser to use */
        virtual int  getEndTick(const int noteID) const
        {
            return noteRenderInfo[noteID].getTick() + noteRenderInfo[noteID].getTickLength();
        }
        
    protected:
        // internal methods performing different steps in score analysis
        
        void findAndMergeChords();
        void processTriplets();
        void processNoteBeam();
    };
    
}

#endif
