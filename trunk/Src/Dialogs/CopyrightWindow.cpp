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
#include "Midi/CommonMidiUtils.h"

#include <iostream>

#include "wx/wx.h"

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
        
        Sequence* sequence;
        
        wxStaticText* songLength;
        
        int code;
        
    public:
        LEAK_CHECK();
        
        CopyrightWindowClass(Sequence* seq) : wxDialog( NULL, wxID_ANY,
                                                       //I18N: - title of the copyright/info dialog
                                                       _("Copyright and song info"),
                                                       wxDefaultPosition, wxSize(400,400), wxCAPTION )
        {
            sequence = seq;
            
            boxSizer=new wxBoxSizer(wxVERTICAL);
            
            //I18N: - title of the copyright/info dialog
            boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Song name") ) , 0, wxALL, 2 );
            
            // song name
            wxSize size_horz = wxDefaultSize;
            size_horz.x = 400;
            nameInput = new wxTextCtrl( this, wxID_ANY,  sequence->getInternalName(), wxDefaultPosition, size_horz);
            boxSizer->Add( nameInput, 0, wxALL, 10 );
            
            //I18N: - title of the copyright/info dialog
            boxSizer->Add( new wxStaticText(this, wxID_ANY,  _("Copyright") ) , 0, wxALL, 2 );
            
            // text area
            copyrightInput = new wxTextCtrl( this, wxID_ANY, sequence->getCopyright(), wxDefaultPosition, wxSize(400,75), wxTE_MULTILINE );
            boxSizer->Add( copyrightInput, 0, wxALL, 10 );
            
            // song length
            songLength = new wxStaticText(this, wxID_ANY, wxString(_("Song duration :"))+wxT(" ??:??"));
            boxSizer->Add( songLength, 0, wxALL, 10 );
            
            // ok button
            okBtn = new wxButton( this, 1, wxT("OK"));
            okBtn->SetDefault();
            boxSizer->Add( okBtn, 0, wxALL, 10 );
            
            SetSizer( boxSizer );
            boxSizer->Layout();
            boxSizer->SetSizeHints( this );
            
            // find song duration
            int last_tick = -1;
            const int track_amount = seq->getTrackAmount();
            for(int n=0; n<track_amount; n++)
            {
                const int duration = seq->getTrack(n)->getDuration();
                if (duration > last_tick) last_tick = duration;
            }
            
            std::vector<int> tempos;
            std::vector<int> duration;
            
            const int tempo_events_amount = seq->tempoEvents.size();
            //std::cout << "tempo_events_amount = " << tempo_events_amount << std::endl;
            if (tempo_events_amount < 1 or (tempo_events_amount==1 and seq->tempoEvents[0].getTick()==0) )
            {
                tempos.push_back(seq->getTempo());
                duration.push_back(last_tick);
            }
            else
            {
                // multiple tempo changes
                
                // from beginning to first event
                tempos.push_back(seq->getTempo());
                duration.push_back(seq->tempoEvents[0].getTick());
                
                //std::cout << "Firstly, " << (seq->tempoEvents[0].getTick()) << " ticks at " << seq->getTempo() << std::endl;
                
                // other events
                for(int n=1; n<tempo_events_amount; n++)
                {
                    tempos.push_back( convertTempoBendToBPM(seq->tempoEvents[n-1].getValue()) );
                    duration.push_back(seq->tempoEvents[n].getTick()-seq->tempoEvents[n-1].getTick());
                    //std::cout << (duration[duration.size()-1]) << " ticks at " << (tempos[tempos.size()-1]) << std::endl;
                }
                
                // after last event
                tempos.push_back( convertTempoBendToBPM(seq->tempoEvents[seq->tempoEvents.size()-1].getValue()) );
                duration.push_back( last_tick - seq->tempoEvents[seq->tempoEvents.size()-1].getTick() );
                //std::cout << "Finally, " << (duration[duration.size()-1]) << " ticks at " << (tempos[tempos.size()-1]) << std::endl;
            }
            
            
            int song_duration = 0;
            const int amount = tempos.size();
            for(int n=0; n<amount; n++)
            {
                // std::cout << " -- adding " << (int)round( ((float)duration[n] * 60.0f ) / ((float)seq->ticksPerBeat() * (float)tempos[n])) << std::endl;
                song_duration += (int)round( ((float)duration[n] * 60.0f ) / ((float)seq->ticksPerBeat() * (float)tempos[n]));
            }
            
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
    
    EVT_BUTTON( 1, CopyrightWindowClass::okClicked )
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

