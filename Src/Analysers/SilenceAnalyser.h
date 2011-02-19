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

#ifndef _SILENCE_ANALYSER_H_
#define _SILENCE_ANALYSER_H_

#include "Range.h"
#include <vector>

namespace AriaMaestosa
{
    class Sequence;
    
    /**
      * @brief Contains the necessary to determine (score) silences from a bunch of notes
      * @ingroup analysers
      */
    namespace SilenceAnalyser
    {
    
        /** "Delegate" type used to give back silence information to caller of 'SilenceAnalyser::findSilences' */
        typedef void(*RenderSilenceCallback)(const Sequence* gseq, const int duration, const int tick,
                                             const int type, const int silences_y, const bool triplet, const bool dotted, 
                                             const int dot_delta_x, const int dot_delta_y, void* userData);

        /** Describes a silence */
        class SilenceInfo
        {
        public:
            /** tick from and to */
            Range<int> m_tick_range;
            
            /** type of silence, where 1/m_type is the "human" name (e.g. m_type == 4 means a 1/4 silence) */
            int m_type;
            
            /** Y coordinate at which the silence is */
            int m_silences_y;
            
            /** whether this silence is dotted */
            bool m_dotted;
            
            /** whether this silence is of triplet duration (and thus needs a "3" sign under it) */
            bool m_triplet;
            
            SilenceInfo(int tickFrom, int tickTo, int type, const int silences_y, bool triplet,
                        bool dotted) : m_tick_range(tickFrom, tickTo)
            {
                m_type        = type;
                m_triplet     = triplet;
                m_silences_y  = silences_y;
                m_dotted      = dotted;
            }
        };
        
        /** 
          * Interface that the silence analyser uses for its operations. Each module that
          * wishes to perform silence analysis must create a class that implements this protocol.
          */
        class INoteSource
        {
        public:
            
            virtual ~INoteSource() { }
            
            /** @return the number of notes that can be obtained through this interface */
            virtual int  getNoteCount() const = 0;
            
            /**
              * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
              * @return       the ID of the measure in which 'noteID' starts
              */
            virtual int  getBeginMeasure(const int noteID) const = 0;
            
            /**
             * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
             * @return       the tick at which 'noteID' starts
             */
            virtual int  getStartTick(const int noteID) const = 0;
            
            /**
             * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
             * @return       the tick at which 'noteID' ends
             */
            virtual int  getEndTick(const int noteID) const = 0;
        };
        
        /** finds all silences, and invokes a passed callback function for each of them */
        void findSilences(const Sequence* seq, RenderSilenceCallback renderSilenceCallback,
                          INoteSource* noteSource, const int first_visible_measure,
                          const int last_visible_measure, const int silences_y, void* userdata);
        
        /** variant of the above, but instead of using a function callback it justs returns a vector
          * @note the returned vector is NOT ordered
          */
        std::vector<SilenceInfo> findSilences(const Sequence* seq, INoteSource* noteSource,
                                              const int first_visible_measure, const int last_visible_measure,
                                              const int silences_y);
        
    }

}

#endif
