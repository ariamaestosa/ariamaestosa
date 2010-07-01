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

#ifndef __INSTRUMENT_PICKER_H__
#define __INSTRUMENT_PICKER_H__

/**
  * @defgroup pickers
  * @brief This module contains all picker menus that are meant to be quickly shown
  *        and hidden.
  * This includes all pop-up menus that appear for the user to make a quick selection,
  * and other dialogs that act in a "pop-up-menu-like" fashion, i.e. they're short-lived
  * and only used to make a quick selection.
  */

#include "wx/menu.h"
#include "wx/string.h"
#include "Renderers/RenderAPI.h"

#include "Utils.h"

namespace AriaMaestosa
{
    
    class InstrumentChoice;
    
    /**
      * @ingroup pickers
      * @brief menu where a MIDI instrument can be selected
      */
    class InstrumentPicker : public wxMenu
    {
        wxMenuItem* inst_menus[128];
        
        wxMenu* submenu_1_piano;
        wxMenu* submenu_2_chromatic;
        wxMenu* submenu_3_organ;
        wxMenu* submenu_4_guitar;
        wxMenu* submenu_5_bass;
        wxMenu* submenu_6_orchestra;
        wxMenu* submenu_7_choirs_pads;
        wxMenu* submenu_8_brass;
        wxMenu* submenu_9_reed;
        wxMenu* submenu_10_pipe;
        wxMenu* submenu_11_synths;
        wxMenu* submenu_12_ethnic;
        wxMenu* submenu_13_percussion;
        wxMenu* submenu_14_sound_fx;
        wxMenu* submenu_15_drums;
                
        InstrumentChoice* m_model;
        
    public:
        
        LEAK_CHECK();
        
        InstrumentPicker();
        ~InstrumentPicker();
                
        void setModel(InstrumentChoice* choice);
        
        void menuSelected(wxCommandEvent& evt);
        
        
        DECLARE_EVENT_TABLE();
    };
    
}

#endif
