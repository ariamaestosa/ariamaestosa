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
#include "GUI/MainFrame.h"
#include "Midi/Sequence.h"
#include "Midi/CommonMidiUtils.h"

#include <iostream>

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

#include "Utils.h"

namespace AriaMaestosa
{
    
    /**
      * @ingroup dialogs
      * @brief the dialog where sequence info can be edited (title, author, etc...)
      * @note this is a private class, it won't be instanciated directly
      * @see CopyrightWindow::show
      * @see CopyrightWindow::hide
      * @see CopyrightWindow::free
      */
    class CopyrightWindowClass : public wxDialog
    {
        
        wxBoxSizer* boxSizer;
        wxTextCtrl* copyrightInput;
        wxTextCtrl* nameInput;
        wxButton* okBtn;
        wxButton* cancelBtn;
        
        Sequence* sequence;
        
        wxStaticText* songLength;
        
        int code;
        
    public:
        LEAK_CHECK();
        
        CopyrightWindowClass(Sequence* seq) : wxDialog(getMainFrame(), wxID_ANY,
                                                       //I18N: - title of the copyright/info dialog
                                                       _("Copyright and song info"),
                                                       wxDefaultPosition, wxSize(400,400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER )
        {
            sequence = seq;
            
            boxSizer=new wxBoxSizer(wxVERTICAL);
            
            //I18N: - title of the copyright/info dialog
            boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Song name") ) , 0, wxLEFT|wxTOP|wxEXPAND, 10 );
            
            // song name
            wxSize size_horz = wxDefaultSize;
            size_horz.x = 400;
            nameInput = new wxTextCtrl( this, wxID_ANY,  sequence->getInternalName(), wxDefaultPosition, size_horz);
            boxSizer->Add( nameInput, 0, wxALL|wxEXPAND, 10 );
            
            //I18N: - title of the copyright/info dialog
            boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Copyright") ) , 0, wxLEFT|wxTOP|wxEXPAND, 10  );
            
            // text area
            copyrightInput = new wxTextCtrl( this, wxID_ANY, sequence->getCopyright(), wxDefaultPosition, wxSize(400,75), wxTE_MULTILINE );
            boxSizer->Add( copyrightInput, 1, wxALL|wxEXPAND, 10 );
            
            // song length
            songLength = new wxStaticText(this, wxID_ANY, wxString(_("Song duration :"))+wxT(" ??:??"));
            boxSizer->Add( songLength, 0, wxLEFT|wxTOP|wxEXPAND, 10 );
            
            // ok button
            okBtn = new wxButton( this, wxID_OK, _("OK"));
            okBtn->SetDefault();
            boxSizer->Add( okBtn, 0, wxALL, 0 );

            // ok button
            cancelBtn = new wxButton( this, wxID_CANCEL, _("Cancel"));
            cancelBtn->SetDefault();
            boxSizer->Add( cancelBtn, 0, wxALL, 0 );

            wxStdDialogButtonSizer* stdDialogButtonSizer = new wxStdDialogButtonSizer();
            stdDialogButtonSizer->AddButton(okBtn);
            stdDialogButtonSizer->AddButton(cancelBtn);
            stdDialogButtonSizer->Realize();
            boxSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 10);

            SetSizer( boxSizer );
            boxSizer->Layout();
            boxSizer->SetSizeHints( this );
            
            // find song duration
            int last_tick = -1;
            const int track_amount = seq->getTrackAmount();
            for (int n=0; n<track_amount; n++)
            {
                const int duration = seq->getTrack(n)->getDuration();
                if (duration > last_tick) last_tick = duration;
            }
            
            int song_duration = getTimeAtTick(last_tick, seq);
            
            wxString duration_label = wxString::Format(wxString(_("Song duration :")) + wxT("  %i:%.2i"), (int)(song_duration/60), song_duration%60);
            //std::cout << song_duration << " --> " << duration_label.mb_str() << std::endl;
            songLength->SetLabel(duration_label);
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
        
        void onCancel(wxCommandEvent& evt)
        {
            CopyrightWindow::hide();
        }
        
        DECLARE_EVENT_TABLE();
        
    };
    
    BEGIN_EVENT_TABLE( CopyrightWindowClass, wxDialog )
    
    EVT_BUTTON( wxID_OK, CopyrightWindowClass::okClicked )
    EVT_BUTTON( wxID_CANCEL, CopyrightWindowClass::onCancel )
    EVT_COMMAND(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, CopyrightWindowClass::onCancel )
    
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

