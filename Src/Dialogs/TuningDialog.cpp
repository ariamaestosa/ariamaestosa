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

#include "Utils.h"

#include "AriaCore.h"
#include "Dialogs/TuningDialog.h"
#include "Editors/GuitarEditor.h"
#include "Pickers/NotePickerWidget.h"
#include "Utils.h"

#include <iostream>

#include "wx/wx.h"

namespace AriaMaestosa
{
    
    
#if 0
#pragma mark -
#pragma mark TuningDialog
#endif
    
    // ---------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------
    
    TuningDialog::TuningDialog() :
        wxFrame(NULL, wxID_ANY,  _("Custom Tuning Editor"), wxPoint(100,100), wxSize(500,300), wxCAPTION )
    {
        
        
        sizer = new wxBoxSizer(wxVERTICAL);
        
        for(int n=0; n<10; n++)
        {
            m_note_pickers[n] = new NotePickerWidget(this, true);
            sizer->Add(m_note_pickers[n], 0, wxALL, 5);
        }
        
        buttonPane = new wxPanel(this);
        sizer->Add(buttonPane, 0, wxALL, 5);
        
        buttonsizer = new wxBoxSizer(wxHORIZONTAL);
        
        ok_btn = new wxButton(buttonPane, 200, _("OK"));
        ok_btn->SetDefault();
        buttonsizer->Add(ok_btn, 0, wxALL, 5);
        
        cancel_btn = new wxButton(buttonPane, 202,  _("Cancel"));
        buttonsizer->Add(cancel_btn, 0, wxALL, 5);
        
        buttonPane->SetSizer(buttonsizer);
        parent = NULL;
        
        SetAutoLayout(true);
        SetSizer(sizer);
        sizer->Layout();
        sizer->SetSizeHints(this); // resize window to take ideal space
    }
    
    // ---------------------------------------------------------------------------------------------------------
    
    TuningDialog::~TuningDialog()
    {
    }
    
    // ---------------------------------------------------------------------------------------------------------
    
    void TuningDialog::setParent(GuitarEditor* parent_arg)
    {
        parent = parent_arg;
    }
    
    // ---------------------------------------------------------------------------------------------------------
    
    void TuningDialog::show()
    {
        ASSERT(parent != NULL);
        
        // enter default values
        for (int n=0; n<10; n++)
        {
            int pitchID = -1;
            if (n<(int)parent->tuning.size()) pitchID = parent->tuning[n];
            
            m_note_pickers[n]->enterDefaultValue(pitchID);
        }
        
        Center();
        Show();
    }
    
    // ---------------------------------------------------------------------------------------------------------
    
    void TuningDialog::cancelButton(wxCommandEvent& evt)
    {
        Hide();
    }
    
    // ---------------------------------------------------------------------------------------------------------
    
    void TuningDialog::okButton(wxCommandEvent& evt)
    {
        parent->previous_tuning = parent->tuning;
        parent->tuning.clear();
        
        // set new tuning
        for (int n=0; n<10; n++)
        {
            if (m_note_pickers[n]->isActive())
            {
                const int notepitch = m_note_pickers[n]->getSelectedNote();
                parent->tuning.push_back(notepitch);
            }
        }
        
        parent->tuningUpdated();
        Display::render();
        
        Hide();
    }

    // ---------------------------------------------------------------------------------------------------------
    
    BEGIN_EVENT_TABLE(TuningDialog, wxFrame)
    EVT_BUTTON(200, TuningDialog::okButton)
    EVT_BUTTON(202, TuningDialog::cancelButton)
    END_EVENT_TABLE()

} // end namespace
