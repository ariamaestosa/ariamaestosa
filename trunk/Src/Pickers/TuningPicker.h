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

#ifndef __TUNING_PICKER_H__
#define __TUNING_PICKER_H__

#include <wx/menu.h>
#include "Utils.h"
#include "Dialogs/TuningDialog.h"

namespace AriaMaestosa
{
    
    class GuitarTuning; // forward
    class Track;

    /**
      * @brief guitar tuning popup menu (for the tablature editor)
      */
    class TuningPicker : public wxMenu
    {
        void resetChecks();
        GuitarTuning* m_model;
        WxOwnerPtr<TuningDialog>  m_tuning_dialog;
        Track* m_parent;
        
    public:
        LEAK_CHECK();
        
        TuningPicker();
        ~TuningPicker();
        
        void menuItemSelected(wxCommandEvent& evt);
        void loadTuning(const int id, const bool user_triggered=true);
        void setModel(GuitarTuning* model, Track* parent);
        
        DECLARE_EVENT_TABLE();
    };
    
}

#endif
