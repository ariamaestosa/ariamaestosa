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
    public:
        LEAK_CHECK();
        
        TimeSigChange(int measure, int num, int denom);
        
        int measure; // indicates the position of the measure
        int denom;
        int num;
        
        // these do not indicate where the measure is, it is just a way to speed up calculations by storing
        // values there instead of recalculating them everytime. they will be changed everytime a time sig even is modified
        // this way no need to recalculate every frame, only on change.
        // MeasureBar::updateMeasureInfo() is the method that should be called whenever any of the TImeSig events has one of
        // its above params changed
        int tick, pixel;
    };
    
}
#endif
