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



#ifndef __TUNING_DIALOG_H__
#define __TUNING_DIALOG_H__

#include <wx/frame.h>
class wxPanel;
class wxButton;
class wxBoxSizer;

namespace AriaMaestosa
{
    class GuitarTuning;
    class NotePickerWidget;
    
    /**
      * @ingroup dialogs
      * @brief Frame that lets you enter a custom guitar tuning for the tablature editor
      */
    class TuningDialog : public wxFrame
    {
        wxButton* m_ok_btn;
        wxButton* m_cancel_btn;
                
        GuitarTuning* m_model;
        
        NotePickerWidget* m_note_pickers[10];
        
    public:
        LEAK_CHECK();
        
        TuningDialog();
        ~TuningDialog();
        
        void setModel(GuitarTuning* model);
        
        void show();
        
        /** @brief callback invoked when Cancel button of the tuning picker is pressed */
        void cancelButton(wxCommandEvent& evt);
        
        /** @brief callback invoked when OK button of the tuning picker is pressed */
        void okButton(wxCommandEvent& evt);
    };
    
}

#endif
