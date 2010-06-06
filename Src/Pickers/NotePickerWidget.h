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

#ifndef __NOTE_PICKER_WIDGET_H__
#define __NOTE_PICKER_WIDGET_H__

#include "Utils.h"
#include "wx/panel.h"
#include "wx/checkbox.h"

class wxBoxSizer;
class wxChoice;

namespace AriaMaestosa
{    
    /**
     * @brief Widget to pick a note
     */
    class NotePickerWidget : public wxPanel
    {
        wxBoxSizer* sizer;
        wxCheckBox* m_active;
        wxChoice* note_choice;
        wxChoice* sign_choice;
        wxChoice* octave_choice;
        
    public:
        
        LEAK_CHECK();
        
        NotePickerWidget(wxWindow* parent, bool withCheckbox);
        void enterDefaultValue(int pitchID);
        
        void somethingSelected(wxCommandEvent& evt);
        
        
        bool isActive() const { ASSERT(m_active != NULL); return m_active->IsChecked(); }
        
        int getSelectedNote() const;
        
        DECLARE_EVENT_TABLE()
    };
    
}
#endif

