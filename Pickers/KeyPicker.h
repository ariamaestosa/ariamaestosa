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

#ifndef _key_picker_
#define _key_picker_

#include "wx/wx.h"
#include "Config.h"

namespace AriaMaestosa {
	
	class GraphicalTrack; // forward
	
	class KeyPicker : public wxMenu
    {
		
		GraphicalTrack* parent;
		
		wxMenuItem* musical_checkbox;
		wxMenuItem* linear_checkbox;
        
        wxMenuItem* fclef;
        wxMenuItem* gclef;
        wxMenuItem* octave_above;
        wxMenuItem* octave_below;

        wxMenuItem* key_c;
		
        wxMenuItem* key_sharps_1;
        wxMenuItem* key_sharps_2;
        wxMenuItem* key_sharps_3;
        wxMenuItem* key_sharps_4;
        wxMenuItem* key_sharps_5;
        wxMenuItem* key_sharps_6;
        wxMenuItem* key_sharps_7;
        
        wxMenuItem* key_flats_1;
        wxMenuItem* key_flats_2;
        wxMenuItem* key_flats_3;
        wxMenuItem* key_flats_4;
        wxMenuItem* key_flats_5;
        wxMenuItem* key_flats_6;
        wxMenuItem* key_flats_7;
public:
        LEAK_CHECK(KeyPicker);
        
		KeyPicker();
		~KeyPicker();
		
		void setChecks( bool musicalNotationEnabled, bool linearNotationEnabled, bool f_clef, bool g_clef, int octave_shift);
		
		void menuItemSelected(wxCommandEvent& evt);    
		void setParent(Track* parent_arg);
		
		DECLARE_EVENT_TABLE();
	};
	
}

#endif
