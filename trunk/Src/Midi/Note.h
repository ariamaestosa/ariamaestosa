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

class wxFileOutputStream;

// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }


namespace AriaMaestosa
{
    
    class GraphicalTrack; // forward
    
    /**
      * @brief represents one note
      * @ingroup midi
      */
    class Note
    {
        
    public:
        LEAK_CHECK();
        
        /**
          * used for score editor. by default, the editor will choose a default sign to display accidentals.
          * however computers are not smart enough to guess correctly so the user has the possibility to
          * tell explicitely what sign should be used to render that note (sharp, flat)
          */
        short preferred_accidental_sign;
        
        /**
          * The pitch used by Aria is not a midi pitch, it is (131 - midi pitch), for the simple reason that high
          * notes being drawn near the top of the screen, and low notes near the bottom, this reversed order
          * simplifies drawing routines (and in the code we draw much more often than we play) middle C being 71
          */
        unsigned short pitchID;
        
        int startTick, endTick;
        unsigned short volume;
        
        /** for guitar mode */
        short string, fret;
        
        bool selected;
        
        GraphicalTrack* gtrack;
        
        
        void setSelected(const bool selected);
        bool isSelected() const;
        
        Note(GraphicalTrack* parent, const int pitchID=-1, const int startTick=-1, const int endTick=-1, const int volume=-1, const int string=-1, const int fret=-1); // guitar mode only
        ~Note();
        
        void setParent(GraphicalTrack* parent);
        
        void setVolume(int vol);
        void resize(const int ticks);
        void setEnd(const int ticks);
        
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
        
        int   getTick   () const    { return startTick; }
        short getPitchID() const    { return pitchID;   }
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}
#endif
