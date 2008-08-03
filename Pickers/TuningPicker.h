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

#ifndef _tuning_picker_
#define _tuning_picker_

#include "wx/wx.h"
#include "Config.h"

namespace AriaMaestosa {
	
class GuitarEditor; // forward
class CustomTuningPicker;
	
class TuningPicker : public wxMenu
{
    void resetChecks();
    GuitarEditor* parent;
    WX_PTR_HOLD(CustomTuningPicker, ctp);
	
public:
    LEAK_CHECK(TuningPicker);
        
    TuningPicker();
    ~TuningPicker();

    void menuItemSelected(wxCommandEvent& evt);
    void loadTuning(const int id, const bool user_triggered=true);    
    void setParent(GuitarEditor* parent_arg);
		
    DECLARE_EVENT_TABLE();
};

}

#endif
