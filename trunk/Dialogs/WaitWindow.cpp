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

#include <iostream>
#include "Config.h"

#include "wx/wx.h"

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

class WaitWindowClass : public wxDialog
{
    PulseNotifier pulseNotifier;

    wxBoxSizer* boxSizer;
    wxStaticText* label;
    wxGauge* progress;

    bool progress_known;

public:
    LEAK_CHECK();

    WaitWindowClass(wxString message, bool progress_known) : wxDialog( NULL, wxID_ANY,  _("Please wait..."), wxDefaultPosition, wxSize(400,200), wxCAPTION | wxSTAY_ON_TOP )
    {



        boxSizer=new wxBoxSizer(wxVERTICAL);
        WaitWindowClass::progress_known = progress_known;

        // gauge
        progress = new wxGauge( this, wxID_ANY, 100/*, wxDefaultPosition, wxSize(200, 15)*/ );
        if(progress_known) progress->SetValue(0);
        else
        {
            pulseNotifier.start();
            //progress->Pulse();
        }
        boxSizer->Add( progress, 1, wxEXPAND | wxALL, 10 );

        // label
        label = new wxStaticText( this, wxID_ANY, message, wxPoint(25,25) );
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

    void setProgress(int val)
    {
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
        if(progress_known) pulseNotifier.Stop();
        wxDialog::Hide();
    }

};

namespace WaitWindow {

    void show(wxString message, bool progress_known)
    {

        wxBeginBusyCursor();
        waitWindow = new WaitWindowClass(message, progress_known);
        waitWindow->show();
    }

    void setProgress(int progress)
    {
        waitWindow->setProgress( progress );
    }

    void hide()
    {
        wxEndBusyCursor();
        waitWindow->hide();
        waitWindow->Destroy();
    }

}

PulseNotifier::PulseNotifier() : wxTimer()
{
}

void PulseNotifier::Notify()
{
    if(WaitWindow::waitWindow != NULL) WaitWindow::waitWindow->pulse();
}

void PulseNotifier::start()
{
    wxTimer::Start(100);
}

}
