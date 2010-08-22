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
        int m_tick;
        
    public:
        LEAK_CHECK();
        
        TimeSigChange(int measure, int num, int denom);
        
        int getMeasure() const { return m_measure; }
        int getDenom  () const { return m_denom;   }
        int getNum    () const { return m_num;     }
        
        int getTick   () const { return m_tick;    }
        
        void setMeasure(int newValue) { m_measure = newValue; }
        void setDenom  (int newValue) { m_denom   = newValue; }
        void setNum    (int newValue) { m_num     = newValue; }
        
        void setTick   (int newValue) { m_tick    = newValue; }
        
        //int m_tick, m_pixel;
    };
    
}
#endif
