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

#ifndef __PRESET_MANAGER_H__
#define __PRESET_MANAGER_H__

#include "Singleton.h"
#include "ptr_vector.h"
#include "wx/string.h"

class wxTextOutputStream;
class wxTextInputStream;

namespace AriaMaestosa
{

    class IPreset
    {
    public:
        virtual ~IPreset(){};
        virtual wxString getName() = 0;
        virtual wxString getStringizedForm() = 0;
    };
    
    
    /** "Delegate" type used to instantiate presets */
    typedef IPreset*(*PresetFactory)(const char* name, const char* stringizedForm);
    
    
    class PresetGroup
    {
        ptr_vector<IPreset> m_presets;
        wxString m_name;

        void read(wxTextInputStream* source, PresetFactory presetFactory);
        void write(wxTextOutputStream* where);
        
    public:
        
        PresetGroup(const char* name);
        
        ptr_vector<IPreset, REF> getPresets()  {  return m_presets.getWeakView();  }
        
        void add(IPreset* preset);
        
        void read(PresetFactory presetFactory);
        void write();
    };
    


}
#endif