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
#include <wx/sizer.h>
#include <wx/button.h>

using namespace AriaMaestosa;



#if 0
#pragma mark -
#pragma mark TuningDialog
#endif

// ---------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------

TuningDialog::TuningDialog() :
wxFrame(NULL, wxID_ANY,  _("Custom Tuning Editor"), wxPoint(100,100), wxSize(500,300), wxCAPTION | wxCLOSE_BOX)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    for (int n=0; n<10; n++)
    {
        m_note_pickers[n] = new NotePickerWidget(this, true);
        sizer->Add(m_note_pickers[n], 0, wxALL|wxEXPAND, 5);
    }
    
    wxBoxSizer* buttonsizer = new wxBoxSizer(wxHORIZONTAL);
    
    m_ok_btn = new wxButton(this, wxID_OK, _("OK"));
    m_ok_btn->SetDefault();

    m_cancel_btn = new wxButton(this, wxID_CANCEL,  _("Cancel"));

    wxStdDialogButtonSizer* stdDialogButtonSizer = new wxStdDialogButtonSizer();
    stdDialogButtonSizer->AddButton(m_ok_btn);
    stdDialogButtonSizer->AddButton(m_cancel_btn);
    stdDialogButtonSizer->Realize();
    buttonsizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 10);
    sizer->Add(buttonsizer, 0, wxALL|wxEXPAND, 0);

    m_model = NULL;
    
    SetAutoLayout(true);
    SetSizer(sizer);
    sizer->Layout();
    sizer->SetSizeHints(this); // resize window to take ideal space
    
    m_ok_btn->Connect(m_ok_btn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                      wxCommandEventHandler(TuningDialog::okButton), NULL, this);
    m_cancel_btn->Connect(m_cancel_btn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                          wxCommandEventHandler(TuningDialog::cancelButton), NULL, this);
}

// ---------------------------------------------------------------------------------------------------------

TuningDialog::~TuningDialog()
{
}

// ---------------------------------------------------------------------------------------------------------

void TuningDialog::setModel(GuitarTuning* model)
{
    m_model = model;
}

// ---------------------------------------------------------------------------------------------------------

void TuningDialog::show()
{
    ASSERT(m_model != NULL);
    
    // enter default values
    for (int n=0; n<10; n++)
    {
        int pitchID = -1;
        if (n<(int)m_model->tuning.size()) pitchID = m_model->tuning[n];
        
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
    std::vector<int> newTuning;
    
    // set new tuning
    for (int n=0; n<10; n++)
    {
        if (m_note_pickers[n]->isActive())
        {
            const int notepitch = m_note_pickers[n]->getSelectedNote();
            newTuning.push_back(notepitch);
        }
    }
    
    m_model->setTuning(newTuning, true);
    Display::render();
    
    Hide();
}

// ---------------------------------------------------------------------------------------------------------


