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

#ifndef _keysigchange_
#define _keysigchange_

#include "Utils.h"

namespace AriaMaestosa
{
    
    /**
      * @brief represents a time signature change event
      * @ingroup midi
      */
    class TimeSigChange
    {
        int m_measure;
        int m_denom;
        int m_num;
        
        /** A cache only */
        int m_tick_cache;
        
    public:
        LEAK_CHECK();
        
        /**
          * @param tick  The cached tick value only, will be overwritten
          */
        TimeSigChange(int measure, int tickCache, int num, int denom);
        
        int getMeasure() const { return m_measure; }
        int getDenom  () const { return m_denom;   }
        int getNum    () const { return m_num;     }
                
        void setMeasure(int newValue) { m_measure = newValue; }
        void setDenom  (int newValue) { m_denom   = newValue; }
        void setNum    (int newValue) { m_num     = newValue; }
        
        /** Set the cached tick value */
        void setTickCache(int newValue) 
        { 
            ASSERT_E(newValue, >=, 0);
            m_tick_cache = newValue; 
        }
        
        /** Get the cached tick value */
        int getTickCache() const { ASSERT(m_tick_cache != -1); return m_tick_cache;    }
    };
    
}
#endif
