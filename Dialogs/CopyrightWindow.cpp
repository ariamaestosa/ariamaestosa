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



#include "Dialogs/CopyrightWindow.h"
#include "Midi/Sequence.h"

#include <iostream>

#include "wx/wx.h"

#include "Config.h"

namespace AriaMaestosa {
	
class CopyrightWindowClass : public wxDialog
{
    
    wxBoxSizer* boxSizer;
    wxTextCtrl* copyrightInput;
	wxTextCtrl* nameInput;
    wxButton* okBtn;
    
    Sequence* sequence;
    
    int code;
    
public:
    LEAK_CHECK(CopyrightWindowClass);
    
    CopyrightWindowClass(Sequence* seq) : wxDialog( NULL, wxID_ANY,  _("Copyright"), wxDefaultPosition, wxSize(400,400), wxCAPTION )
	{
            
			
			
            sequence = seq;
            
            boxSizer=new wxBoxSizer(wxVERTICAL);
            
			boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Song name") ) , 0, wxALL, 2 );
			
			// song name
			wxSize size_horz = wxDefaultSize;
			size_horz.x = 400;
            nameInput = new wxTextCtrl( this, wxID_ANY,  sequence->getInternalName(), wxDefaultPosition, size_horz);
            boxSizer->Add( nameInput, 0, wxALL, 10 );
			
			boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Copyright") ) , 0, wxALL, 2 );
			
            // text area
            copyrightInput = new wxTextCtrl( this, wxID_ANY, sequence->getCopyright(), wxDefaultPosition, wxSize(400,75), wxTE_MULTILINE );
            boxSizer->Add( copyrightInput, 0, wxALL, 10 );
            
            // ok button
            okBtn = new wxButton( this, 1, wxT("OK"));
            okBtn->SetDefault();
            boxSizer->Add( okBtn, 0, wxALL, 10 );
            
            SetSizer( boxSizer );
            boxSizer->Layout();
            boxSizer->SetSizeHints( this );
            
        }
    

    void okClicked(wxCommandEvent& evt)
	{
        sequence->setCopyright( copyrightInput->GetValue() );
        sequence->setInternalName( nameInput->GetValue() );
		
        CopyrightWindow::hide();
    }
    
    void show()
	{
		nameInput->SetValue( sequence->getInternalName() );
		copyrightInput->SetValue( sequence->getCopyright() );
		
        wxDialog::Center();
        code = wxDialog::ShowModal();
    }
	
    void hide()
	{
        wxDialog::EndModal(code);   
        //wxDialog::Hide();
    }
    
    DECLARE_EVENT_TABLE();
    
};

BEGIN_EVENT_TABLE( CopyrightWindowClass, wxDialog )

EVT_BUTTON( 1, CopyrightWindowClass::okClicked )

END_EVENT_TABLE()


namespace CopyrightWindow {
    
    CopyrightWindowClass* copyrightWindow=NULL;
    
    void show(Sequence* seq)
	{
        copyrightWindow = new CopyrightWindowClass(seq);
        copyrightWindow->show();
    }

    void hide()
	{
        copyrightWindow->hide();
		copyrightWindow->Destroy();
    }
	
	void free()
	{
	}

}

}

