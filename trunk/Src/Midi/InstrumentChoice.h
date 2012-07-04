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

#ifndef __INSTRUMENT_CHOICE_H__
#define __INSTRUMENT_CHOICE_H__

#include "Utils.h"
class wxString;

namespace AriaMaestosa
{
    
    class IInstrumentChoiceListener
    {
    public:
        virtual ~IInstrumentChoiceListener() {}
        virtual void onInstrumentChanged(const int newValue) = 0;
    };
    
    /**
     * @brief class that represents an instrument choice and contains the current selection (model for the
     *        instrument picker). 
     * @ingroup midi
     */
    class InstrumentChoice
    {
        int m_selected_instrument;
        IInstrumentChoiceListener* m_listener;
        
    public:
        
        InstrumentChoice(const int defaultInstrument, IInstrumentChoiceListener* listener);
        int getSelectedInstrument() const { return m_selected_instrument; }
        void setInstrument(const int instrument, const bool generateEvent=true);
        static const wxString getInstrumentName(int id);

    };

}
#endif