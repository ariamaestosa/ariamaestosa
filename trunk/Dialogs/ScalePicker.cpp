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



#include "Dialogs/ScalePicker.h"
#include "Midi/Sequence.h"

#include "Config.h"

#include <iostream>

#include "wx/wx.h"

namespace AriaMaestosa {
	
class ScalePickerFrame : public wxDialog
{
    
	DECLARE_LEAK_CHECK();
	
    wxStaticText* label;
    wxStaticText* label2;
    wxStaticText* label3;
    
    wxPanel* contentPane;
    wxBoxSizer* contentPaneSizer;
    
    wxPanel* topPane;
    wxPanel* bottomPane;
    
    wxBoxSizer* verticalSizer;
    wxTextCtrl* textInput;
    
    wxBoxSizer* horizontalSizerTop;
    wxBoxSizer* horizontalSizerBottom;
    
    wxButton* okBtn;
    wxButton* cancelBtn;
    
    wxRadioButton* rel_first_note;
    wxRadioButton* rel_begin;
    
    wxRadioButton* affect_selection;
    wxRadioButton* affect_track;
    wxRadioButton* affect_song;
    
    int code;
    
    Sequence* sequence;
    
public:
	ScalePickerFrame(Sequence* seq) : wxDialog( NULL, wxID_ANY,  _("Scale"), wxDefaultPosition, wxSize(400,200), wxCAPTION )
	{
            
			INIT_LEAK_CHECK();
			
            sequence = seq;
            
            contentPane = new wxPanel(this);
            contentPaneSizer=new wxBoxSizer(wxHORIZONTAL);
            contentPaneSizer->Add(contentPane, 0, wxLEFT | wxRIGHT | wxTOP, 10);
            
            topPane = new wxPanel(contentPane);
            bottomPane = new wxPanel(contentPane);
            
            verticalSizer=new wxBoxSizer(wxVERTICAL);
            verticalSizer->Add(topPane, 0, wxALL, 8);

            // --------------------------------------------------- percent ---------------------------------------------------
            
            horizontalSizerTop=new wxBoxSizer(wxHORIZONTAL);
            
            // text control
            textInput = new wxTextCtrl( topPane, wxID_ANY, wxString(wxT("100")));
            horizontalSizerTop->Add( textInput, 0, wxALL, 3 );
            
            // label
            label = new wxStaticText( topPane, wxID_ANY, wxString(wxT("%")), wxPoint(25,25) );
            horizontalSizerTop->Add( label, 0, wxALL, 2 );

            topPane->SetSizer( horizontalSizerTop );
            horizontalSizerTop->Layout();
            horizontalSizerTop->SetSizeHints( topPane );
            
            // --------------------------------------------------- options ---------------------------------------------------
            
            // scale notes in...
            label2 = new wxStaticText( contentPane, wxID_ANY,  _("Scale notes in..."), wxPoint(25,25) );
            verticalSizer->Add( label2, 0, wxALL, 5 );
            
            affect_selection=new wxRadioButton( contentPane, 3,  _("selection"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
            verticalSizer->Add( affect_selection, 0, wxALL, 5 );
            affect_selection->SetValue(contentPane);
            
            affect_track=new wxRadioButton( contentPane, 3,  _("track"));
            verticalSizer->Add( affect_track, 0, wxALL, 5 );
            
            affect_song=new wxRadioButton( contentPane, 3,  _("song"));
            verticalSizer->Add( affect_song, 0, wxALL, 5 );
            
            // relative to...
            label3 = new wxStaticText( contentPane, wxID_ANY,  _("relative to..."), wxPoint(25,25) );
            verticalSizer->Add( label3, 0, wxALL, 5 );
            
            rel_first_note=new wxRadioButton( contentPane, 3,  _("first affected note"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
            verticalSizer->Add( rel_first_note, 0, wxALL, 5 );
            rel_first_note->SetValue(contentPane);
                
            rel_begin=new wxRadioButton( contentPane, 3,  _("song beginning"));
            verticalSizer->Add( rel_begin, 0, wxALL, 5 );
            
            // --------------------------------------------------- bottom pane ---------------------------------------------------
            
            horizontalSizerBottom=new wxBoxSizer(wxHORIZONTAL);
            
            cancelBtn = new wxButton( bottomPane, 1,  _("Cancel"));
            horizontalSizerBottom->Add( cancelBtn, 0, wxALL, 5 );
            

            okBtn = new wxButton( bottomPane, 2, wxT("OK"));
            horizontalSizerBottom->Add( okBtn, 0, wxALL, 5 );
            
            okBtn->SetDefault();
            
            bottomPane->SetSizer( horizontalSizerBottom );
            horizontalSizerBottom->Layout();
            horizontalSizerBottom->SetSizeHints( bottomPane );
            
            // --------------------------------------------------------------------------------------------------------------

            verticalSizer->Add(bottomPane, 0, wxALL, 8);
                
            contentPane->SetSizer( verticalSizer );
            verticalSizer->Layout();
            verticalSizer->SetSizeHints( contentPane );

            SetSizer( contentPaneSizer );
            contentPaneSizer->Layout();
            contentPaneSizer->SetSizeHints( this );
        }
    
    
    void show()
	{
        wxDialog::Center();
        code = wxDialog::ShowModal();
    }
    void hide()
	{
        wxDialog::EndModal(code);
    }
    
    void ok_clicked(wxCommandEvent& evt)
	{
        hide();
        
        wxString factor_str = textInput->GetValue();
        
        if(!factor_str.IsNumber()){
            wxBell();
            return;
        }
        
        float factor = atof(
                            toCString(factor_str)
                            ) / 100.0;
        
        //std::cout << "factor: " << factor << std::endl;
        
        sequence->scale(
                        factor,
                        
                        rel_first_note->GetValue(),
                        rel_begin->GetValue(),
                        
                        affect_selection->GetValue(),
                        affect_track->GetValue(),
                        affect_song->GetValue()
                        
                        );
		
		ScalePicker::free();
    }

    void cancel_clicked(wxCommandEvent& evt)
	{
        hide();
		ScalePicker::free();
    }
    
    DECLARE_EVENT_TABLE();
    
};


BEGIN_EVENT_TABLE(ScalePickerFrame, wxDialog)

EVT_BUTTON(1, ScalePickerFrame::cancel_clicked)
EVT_BUTTON(2, ScalePickerFrame::ok_clicked)

END_EVENT_TABLE()

namespace ScalePicker{
    
    ScalePickerFrame* frame = NULL;
    
    void pickScale(Sequence* seq)
	{
        
        if(frame==NULL) frame=new ScalePickerFrame(seq);
        frame->show();
        
    }
	void free()
	{
		if(frame!=NULL)
		{
			frame->Destroy();	
			frame = NULL;
		}
	}
    
}

}
