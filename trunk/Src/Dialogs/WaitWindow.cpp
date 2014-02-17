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



#include "Dialogs/WaitWindow.h"

#include "Utils.h"

#include <wx/timer.h>
#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/gauge.h>
#include <wx/sizer.h>


namespace AriaMaestosa
{
    class WaitWindowClass;
    
    namespace WaitWindow
    {
        WaitWindowClass* waitWindow=NULL;
    }
    
    class PulseNotifier : public wxTimer
    {
    public:
        PulseNotifier();
        void Notify();
        void start();
    };
    
    /**
     * @ingroup dialogs
     * @brief the dialog where a progress bar is shown to tell the user to wait
     * @note this is a private class, you won't use it directly
     * @see WaitWindow::show
     * @see WaitWindow::setProgress
     * @see WaitWindow::hide
     */
    class WaitWindowClass : public wxDialog
    {
        PulseNotifier pulseNotifier;
        
        wxBoxSizer* boxSizer;
        wxStaticText* label;
        wxGauge* progress;
        
        bool m_progress_known;
        
    public:
        LEAK_CHECK();
        
        WaitWindowClass(wxWindow* parent, wxString message, bool progressKnown) :
            wxDialog( parent, wxID_ANY,  _("Please wait..."), wxDefaultPosition, wxSize(250,200),
                      wxCAPTION | wxSTAY_ON_TOP )
        {
            boxSizer = new wxBoxSizer(wxVERTICAL);
            m_progress_known = progressKnown;
            
            // gauge
            progress = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize(200, 20) );
            if (progressKnown)
            {
                progress->SetValue(0);
            }
            else
            {
                pulseNotifier.start();
            }
            boxSizer->Add( progress, 1, wxEXPAND | wxALL, 10 );
            
            // label
            label = new wxStaticText( this, wxID_ANY, message, wxPoint(25,30));
            boxSizer->Add( label, 0, wxALL, 10 );
            
            SetSizer( boxSizer );
            boxSizer->Layout();
            boxSizer->SetSizeHints( this );
            
            wxSize windowSize = GetSize();
            wxSize gaugeSize = progress->GetSize();
            gaugeSize.x = windowSize.x - 20;
            progress->SetSize(gaugeSize);
        }
        
        void pulse()
        {
            progress->Pulse();
        }
        
        /** sets the progress, between 0 and 100. Value is clipped if out of bounds */
        void setProgress(int val)
        {
            if (!m_progress_known)
            {
                if (pulseNotifier.IsRunning())
                {
                    pulseNotifier.Stop();
                }
                m_progress_known = true;
            }
            
            if      (val < 0)   val = 0;
            else if (val > 100) val = 100;
            progress->SetValue( val );
            Update();
        }
        
        void show()
        {
            wxDialog::Center();
            wxDialog::Show();
            wxYield();
        }
        
        void hide()
        {
            if (m_progress_known) pulseNotifier.Stop();
            wxDialog::Hide();
        }
        
    };
    
    namespace WaitWindow
    {
        
        void show(wxWindow* parent, wxString message, bool progress_known)
        {
            if (waitWindow != NULL)
            {
                hide();
            }
            wxBeginBusyCursor();
            waitWindow = new WaitWindowClass(parent, message, progress_known);
            waitWindow->show();
        }
        
        void setProgress(int progress)
        {
            waitWindow->setProgress( progress );
        }
        
        void hide()
        {
            if (waitWindow != NULL)
            {
                wxEndBusyCursor();
                waitWindow->hide();
                waitWindow->Destroy();
                waitWindow = NULL;
            }
        }
        
        bool isShown()
        {
            return waitWindow != NULL;
        }
        
    }
    
    PulseNotifier::PulseNotifier() : wxTimer()
    {
    }
    
    void PulseNotifier::Notify()
    {
        if (WaitWindow::waitWindow != NULL) WaitWindow::waitWindow->pulse();
    }
    
    void PulseNotifier::start()
    {
        wxTimer::Start(100);
    }
    
}
