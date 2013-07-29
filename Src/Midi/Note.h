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

#include "Utils.h"
#include <wx/intl.h>

class wxFileOutputStream;

// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }


namespace AriaMaestosa
{
    
    class Track; // forward
    
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
    
    
    static const wxString NOTE_12_NAME[] =
    {
        _("A"),
        _("A#"),
        _("B"),
        _("C"),
        _("C#"),
        _("D"),
        _("D#"),
        _("E"),
        _("F"),
        _("F#"),
        _("G"),
        _("G#")
    };
    
    static const wxString NOTE_12_NAME_FLATS[] =
    {
        _("A"),
        _("Bb"),
        _("B"),
        _("C"),
        _("Db"),
        _("D"),
        _("Eb"),
        _("E"),
        _("F"),
        _("Gb"),
        _("G"),
        _("Ab")
    };
    
    /**
      * @brief represents one note
      * @ingroup midi
      */
    class Note
    {
        Track* m_track;
        
        bool   m_selected;
        
        /**
          * used for score editor. by default, the editor will choose a default sign to display accidentals.
          * however computers are not smart enough to guess correctly so the user has the possibility to
          * tell explicitely what sign should be used to render that note (sharp, flat)
          */
        short m_preferred_accidental_sign;
        
        /**
          * The pitch used by Aria is not a midi pitch, it is (131 - midi pitch), for the simple reason that high
          * notes being drawn near the top of the screen, and low notes near the bottom, this reversed order
          * simplifies drawing routines (and in the code we draw much more often than we play) middle C being 71
          */
        unsigned short m_pitch_ID;
        
        int m_start_tick, m_end_tick;
        
        unsigned short m_volume;
        
        /** for guitar mode */
        short string, fret;
        
    public:
        LEAK_CHECK();
        

        void setSelected(const bool selected);
        bool isSelected() const { return m_selected; }
        
        Note(Track* parent, const int pitchID=-1, const int startTick=-1, const int endTick=-1, const int volume=-1, const int string=-1, const int fret=-1); // guitar mode only
        ~Note();
        
        void setParent(Track* parent);
        Track* getParent() { return m_track; }
        
        /**
          * @note for guitar mode only
          */
        void shiftFret(const int amount);
        
        /**
          * @note for guitar mode only
          */
        void shiftString(const int amount);
        
        /**
          * @brief calculates the string and fret of the note from its pitch
          * @note for guitar mode only
          *
          * Discards the previous string and fret data that was held in this note and replaces it with the
          * newly calculated one.
          */
        void findStringAndFretFromNote();
        
        /**
          * @brief calculates the pitch of the note from its string and fret
          * @note for guitar mode only
          *
          * Discards the previous note data that was held in this note and replaces it with the new one.
          */
        void findNoteFromStringAndFret();
        
        /**
          * @brief re-sync the note and string+fret data to represent the same note
          *
          * because tablature edition adds a challenge : there's both pitch data and string+fret data,
          * and both must matcht he same note.
          * @note for guitar mode only
          */
        void checkIfStringAndFretMatchNote(const bool fixStringAndFret);
        
        int  getString();
        int  getFret();
        
        int getFretConst  () const { return fret; }
        int getStringConst() const { return string; }
        
        void setStringAndFret(int string_arg, int fret_arg);
        void setFret(int i);

        /**
         * Requests that note be played.
         * Change will be true if the sound of the note has been changed. This, with user settings, will determine if it is needed to play note or not.
         */
        void play(bool change);
        
        int   getTick   () const    { return m_start_tick; }
        int   getEndTick() const    { return m_end_tick;   }
        int   getLength () const    { return m_end_tick - m_start_tick; }
        short getPitchID() const    { return m_pitch_ID;   }
        
        
        short getPreferredAccidentalSign() const { return m_preferred_accidental_sign; }
        void  setPreferredAccidentalSign(short newval) { m_preferred_accidental_sign = newval; }

        int  getVolume() const { return m_volume; }
        void setVolume(int vol);
        
        /** @brief Resize note by a delta
          * @param ticksDelta positive or negative delta in ticks by which to resize this note
          */
        void resize(const int ticksDelta);
        
        void setTick(const int tick)     { ASSERT_E(tick,>=,0); m_start_tick = tick; }
        void setPitchID(const int pitch) { m_pitch_ID = pitch; }
        void setEndTick(const int ticks);
        
        /**
         * Returns the pitch ID of a note from its name, sharpness sign and octave
         */
        static int findNotePitch(Note7 note_7, PitchSign sharpness, const int octave);
        
        /**
         * Finds a nome name (A, A#, B, etc..) and octave from its pitch ID
         * @param pitchID      pitch for which you want to find the note name/octave
         * @param[out] note_12 The name of the note at this pitch (A, A#, B, etc...)
         * @param[out] octave  The octave at which this note is
         * @return             Whether conversion was successful. If a problem occurred,
         *                     returns false and out parameters should not be read.
         */ 
        static bool findNoteName(const int pitchID, Note12* note_12, int* octave);
        
        /** @brief Converts a Note7 to its Note12 counterpart */
        static Note12 note7ToNote12(Note7 note7)
        {
            static const Note12 note7_to_note12[] =
            {
                NOTE_12_A,
                NOTE_12_B,
                NOTE_12_C,
                NOTE_12_D,
                NOTE_12_E,
                NOTE_12_F,
                NOTE_12_G
            };
            return note7_to_note12[note7];
        }
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}
#endif
