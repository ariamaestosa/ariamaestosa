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

#ifndef __PRESET_EDITOR_H__
#define __PRESET_EDITOR_H__

#include "wx/dialog.h"

#include "LeakCheck.h"
class wxCommandEvent;
class wxListBox;

namespace AriaMaestosa
{
    class PresetGroup;
    class IPreset;
    
    class PresetEditor : public wxDialog
    {
        wxListBox* m_list;
        
        /** An array of pointers to IPreset objects */
        IPreset** m_presets;
        
        int m_preset_count;
        
        PresetGroup* m_preset_group;
        
        void updateList();
        
    public:
        LEAK_CHECK();
        
        PresetEditor(wxWindow* parent, PresetGroup* presets);
        ~PresetEditor();
        
        void onDuplicate(wxCommandEvent& evt);
        void onDelete(wxCommandEvent& evt);
        void onClose(wxCommandEvent& evt);
    };
    
}

#endif