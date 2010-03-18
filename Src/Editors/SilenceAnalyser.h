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


namespace AriaMaestosa
{
    namespace SilenceAnalyser
    {
    
        /** "Delegate" type used to give back silence information to caller of 'SilenceAnalyser::findSilences' */
        typedef void(*RenderSilenceCallback)(const int duration, const int tick, const int type,
                                             const int silences_y, const bool triplet,
                                             const bool dotted, const int dot_delta_x,
                                             const int dot_delta_y);

        class INoteSource
        {
        public:
            
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
        
        void findSilences(RenderSilenceCallback renderSilenceCallback, INoteSource* noteSource,
                            const int first_visible_measure, const int last_visible_measure,
                            const int silences_y);
        
    }

}

#endif
