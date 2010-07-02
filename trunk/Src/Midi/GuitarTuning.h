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

#ifndef __GUITAR_TUNING_H__
#define __GUITAR_TUNING_H__

#include <vector>

namespace AriaMaestosa
{

    class GuitarTuning;
    
    class IGuitarTuningListener
    {
    public:
        virtual ~IGuitarTuningListener() {}
        
        virtual void onGuitarTuningUpdated(GuitarTuning* tuning, const bool userTriggered) = 0;
    };
    
    /**
      * @brief The model to contain a guitar tuning
      * @ingroup midi
      */
    class GuitarTuning
    {
        IGuitarTuningListener* m_listener;
    public:
        /** Contains one entry per string */
        std::vector<int> tuning;
        std::vector<int> previous_tuning; //!< for undo purposes
        
        GuitarTuning(IGuitarTuningListener* listener);
        
        void setTuning(const std::vector<int>& newTuning, const bool userTriggered);
    };

}

#endif
