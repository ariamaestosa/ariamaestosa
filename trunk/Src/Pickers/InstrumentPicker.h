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

#include <wx/menu.h>
#include <wx/string.h>

#include "Utils.h"



namespace AriaMaestosa
{
    class InstrumentChoice;

    struct PickerMenu
    {
        wxString name;
        const int* inst_array;
        int inst_array_size;
    };


    /**
      * @ingroup pickers
      * @brief menu where a MIDI instrument can be selected
      */
    class InstrumentPicker : public wxMenu
    {
        wxMenuItem* m_inst_menus[128];
        InstrumentChoice* m_model;
        int m_current_classification_id;
        
    public:
        
        LEAK_CHECK();
        
        InstrumentPicker();
        ~InstrumentPicker();
                
        void setModel(InstrumentChoice* choice);
        void updateClassification();
        InstrumentChoice* getModel() { return m_model; }
        void menuSelected(wxCommandEvent& evt);

    private:

        void buildNewPicker();
        void buildPicker(const PickerMenu pickerMenuArray[], int size);
        void deleteCurrentPicker();
        void deletePicker(const PickerMenu pickerMenuArray[], int size);
    };

}

#endif
