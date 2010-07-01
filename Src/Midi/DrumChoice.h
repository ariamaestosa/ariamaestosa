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

#ifndef __DRUM_CHOICE_H__
#define __DRUM_CHOICE_H__

class wxString;

namespace AriaMaestosa
{
    
    class IDrumChoiceListener
    {
    public:
        virtual ~IDrumChoiceListener() {}
        virtual void onDrumkitChanged(const int newValue) = 0;
    };
    

    /**
     * @brief class that represents a drumkit choice and contains the current selection (model for the
     *        drumkit picker). 
     * @ingroup midi
     */
    class DrumChoice
    {
        int m_selected_drumkit;
        IDrumChoiceListener* m_listener;
        
    public:
        
        DrumChoice(const int defaultDrum, IDrumChoiceListener* listener);
        
        int getSelectedDrumkit() const { return m_selected_drumkit; }
        void setDrumkit(const int drumkit, const bool generateEvent=true);
        
        static const wxString& getDrumkitName(int id);
        
    };
    
}

#endif