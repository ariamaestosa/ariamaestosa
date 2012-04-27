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



#include "Dialogs/ScaleDialog.h"
#include "Midi/Sequence.h"

#include "Utils.h"

#include <iostream>

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/button.h>

namespace AriaMaestosa
{

    /**
      * @ingroup dialogs
      * @brief the dialog to set-up resizing of track portions
      * @note this is a private class, it won't be instanciated directly
      * @see ScaleDialog::pickScale
      * @see ScaleDialog::free
      */
    class ScalePickerFrame : public wxDialog
    {
        wxStaticText* m_label;

        wxComboBox* m_text_input;

        wxRadioButton* m_rel_first_note;
        wxRadioButton* m_rel_begin;

        wxRadioButton* m_affect_selection;
        wxRadioButton* m_affect_track;
        wxRadioButton* m_affect_song;

        int m_code;

        Sequence* m_sequence;

    public:
        LEAK_CHECK();

        ScalePickerFrame(Sequence* seq) : wxDialog( NULL, wxID_ANY,
                                                   //I18N: - title of the scale dialog
                                                   _("Scale"),
                                                   wxDefaultPosition, wxSize(400,200), wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP)
        {
            m_sequence = seq;

            wxPanel* contentPane = new wxPanel(this);
            wxBoxSizer* contentPaneSizer = new wxBoxSizer(wxHORIZONTAL);
            contentPaneSizer->Add(contentPane, 0, wxLEFT | wxRIGHT | wxTOP, 10);

            wxPanel* topPane = new wxPanel(contentPane);
            wxPanel* bottomPane = new wxPanel(contentPane);

            wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);
            verticalSizer->Add(topPane, 0, wxALL, 8);

            // --------------------------------------------------- percent ---------------------------------------------------

            wxBoxSizer* horizontalSizerTop = new wxBoxSizer(wxHORIZONTAL);

            // text control
            wxArrayString choices;
            choices.Add(wxT("25"));
            choices.Add(wxT("50"));
            choices.Add(wxT("100"));
            choices.Add(wxT("150"));
            choices.Add(wxT("200"));
            choices.Add(wxT("400"));
            m_text_input = new wxComboBox( topPane, wxID_ANY, wxString(wxT("100")), wxDefaultPosition, wxDefaultSize, choices);
            horizontalSizerTop->Add( m_text_input, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3 );

            // label
            wxStaticText* label = new wxStaticText( topPane, wxID_ANY, wxString(wxT("%")), wxPoint(25,25) );
            horizontalSizerTop->Add( label, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2 );

            topPane->SetSizer( horizontalSizerTop );
            horizontalSizerTop->Layout();
            horizontalSizerTop->SetSizeHints( topPane );

            // --------------------------------------------------- options ---------------------------------------------------

            //I18N: - in the scale dialog. full context : "Scale notes in...\n\n* selection\n* track\n* song"
            wxStaticBoxSizer* first_box = new wxStaticBoxSizer(wxVERTICAL, contentPane, _("Scale notes in..."));
            verticalSizer->Add(first_box, 0, wxALL, 5);

            //I18N: - in the scale dialog. full context : "Scale notes in...\n\n* selection\n* track\n* song"
            m_affect_selection = new wxRadioButton( contentPane, wxID_ANY,  _("selection"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
            first_box->Add( m_affect_selection, 0, wxALL, 5 );
            m_affect_selection->SetValue(contentPane);

            //I18N: - in the scale dialog. full context : "Scale notes in...\n\n* selection\n* track\n* song"
            m_affect_track = new wxRadioButton( contentPane, wxID_ANY,  _("track"));
            first_box->Add( m_affect_track, 0, wxALL, 5 );

            //I18N: - in the scale dialog. full context : "Scale notes in...\n\n* selection\n* track\n* song"
            m_affect_song = new wxRadioButton( contentPane, wxID_ANY,  _("song"));
            first_box->Add( m_affect_song, 0, wxALL, 5 );

            // relative to...

            //I18N: - in the scale dialog. full context : "relative to...\n\n* first affected note\n* song beginning"
            wxStaticBoxSizer* second_box = new wxStaticBoxSizer(wxVERTICAL, contentPane, _("relative to..."));
            verticalSizer->Add(second_box, 0, wxALL, 5);

            //I18N: - in the scale dialog. full context : "relative to...\n\n* first affected note\n* song beginning"
            m_rel_first_note = new wxRadioButton( contentPane, wxID_ANY,  _("first affected note"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
            second_box->Add( m_rel_first_note, 0, wxALL, 5 );
            m_rel_first_note->SetValue(contentPane);

            //I18N: - in the scale dialog. full context : "relative to...\n\n* first affected note\n* song beginning"
            m_rel_begin = new wxRadioButton( contentPane, wxID_ANY,  _("song beginning"));
            second_box->Add( m_rel_begin, 0, wxALL, 5 );

            // --------------------------------------------------- bottom pane ------------------------------
                
        
    
            wxBoxSizer* horizontalSizerBottom = new wxBoxSizer(wxHORIZONTAL);

            wxButton* cancelBtn = new wxButton( bottomPane, wxID_ANY,  _("Cancel"));
            horizontalSizerBottom->Add( cancelBtn, 0, wxALL, 5 );


            wxButton* okBtn = new wxButton( bottomPane, wxID_ANY, _("OK"));
            horizontalSizerBottom->Add( okBtn, 0, wxALL, 5 );

            okBtn->SetDefault();
            
            wxStdDialogButtonSizer* stdDialogButtonSizer = new wxStdDialogButtonSizer();
            stdDialogButtonSizer->AddButton(cancelBtn);
            stdDialogButtonSizer->AddButton(okBtn);
            stdDialogButtonSizer->Realize();
            horizontalSizerBottom->Add(stdDialogButtonSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
            
            bottomPane->SetSizer( horizontalSizerBottom );

            horizontalSizerBottom->Layout();
            horizontalSizerBottom->SetSizeHints( bottomPane );
            
            

            // ----------------------------------------------------------------------------------------------

            verticalSizer->Add(bottomPane, 0, wxALL, 8);

            contentPane->SetSizer( verticalSizer );
            verticalSizer->Layout();
            verticalSizer->SetSizeHints( contentPane );

            SetSizer( contentPaneSizer );
            contentPaneSizer->Layout();
            contentPaneSizer->SetSizeHints( this );
            
            cancelBtn->Connect(cancelBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(ScalePickerFrame::cancel_clicked), NULL, this);
            okBtn->Connect(okBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(ScalePickerFrame::ok_clicked), NULL, this);
            this->Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(ScalePickerFrame::onIDCancel), NULL, this);
        }


        void show()
        {
            wxDialog::Center();
            m_text_input->SetSelection(-1, -1);
            m_code = wxDialog::ShowModal();
        }
        void hide()
        {
            wxDialog::EndModal(m_code);
        }

        void ok_clicked(wxCommandEvent& evt)
        {
            hide();

            wxString factor_str = m_text_input->GetValue();

            if (not factor_str.IsNumber())
            {
                wxBell();
                return;
            }

            float factor = atof(
                                factor_str.mb_str()
                                ) / 100.0;

            //std::cout << "factor: " << factor << std::endl;

            m_sequence->scale(factor,

                              m_rel_first_note->GetValue(),
                              m_rel_begin->GetValue(),

                              m_affect_selection->GetValue(),
                              m_affect_track->GetValue(),
                              m_affect_song->GetValue()

                              );

            ScaleDialog::free();
        }

        void cancel_clicked(wxCommandEvent& evt)
        {
            hide();
            ScaleDialog::free();
        }

        void onIDCancel(wxCommandEvent& evt)
        {
            hide();
            ScaleDialog::free();
        }
    };
    
    namespace ScaleDialog
    {

        ScalePickerFrame* frame = NULL;

        void pickScale(Sequence* seq)
        {

            if (frame==NULL) frame=new ScalePickerFrame(seq);
            frame->show();

        }
        void free()
        {
            if (frame!=NULL)
            {
                frame->Destroy();
                frame = NULL;
            }
        }

    }

}
