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



#ifndef _customnoteselectdlg_
#define _customnoteselectdlg_

#include "wx/wx.h"

#include "Utils.h"

namespace AriaMaestosa
{
    
    class Track;
    
    /**
      * @ingroup dialogs
      * @brief the dialog that lets you setup a custom note selection from various criteria
      */
    class CustomNoteSelectDialog : public wxDialog
    {
        
        wxBoxSizer* boxSizer;
        wxCheckBox* cb_pitch;
        //wxCheckBox* cb_note;
        wxCheckBox* cb_volume;
        wxCheckBox* cb_string;
        wxCheckBox* cb_fret;
        wxCheckBox* cb_duration;
        wxCheckBox* cb_measure;
        wxTextCtrl* from_measure;
        wxTextCtrl* to_measure;
        wxTextCtrl* volume_tolerance;
        wxTextCtrl* duration_tolerance;
        int returnCode;
        
        Track* currentTrack;
        
    public:
        LEAK_CHECK();
        CustomNoteSelectDialog();
        
        void okClicked(wxCommandEvent& evt);
        void cancelClicked(wxCommandEvent& evt);
        void show(Track* currentTrack);
        void keyPress(wxKeyEvent& evt);
        void onFocus(wxFocusEvent& evt);
        
        DECLARE_EVENT_TABLE()
    };
    
}

#endif
