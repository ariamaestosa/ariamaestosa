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
#include "Dialogs/AboutDialog.h"

//#include <wx/utils.h>
#include <wx/bitmap.h>

#if wxCHECK_VERSION(2,9,0)
#include <wx/generic/statbmpg.h>
typedef wxGenericStaticBitmap AboutImage;
#else
#include <wx/statbmp.h>
typedef wxStaticBitmap AboutImage;
#endif

#include <wx/textctrl.h>
#include <wx/menu.h>

#include "IO/IOUtils.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

AboutDialog::AboutDialog() : wxFrame(NULL, wxID_ANY,  _("About Aria Maestosa"), wxDefaultPosition, wxSize(517, 500) )
{
    wxBitmap titleBitmap;
    titleBitmap.LoadFile( getResourcePrefix()  + wxT("title.jpg") , wxBITMAP_TYPE_JPEG );

    new AboutImage(this, wxID_ANY, titleBitmap, wxPoint(0,0), wxSize(517,174));
    
    //I18N: - in about dialog
    wxString about_text =  wxString::Format(_("version %s"), wxT("1.3 beta 1") ) +
    //I18N: - in about dialog
    wxString::Format(_("\n\nThanks to:\n\n\tErgonis Software and %s for making EasyBeat,\n\t\ta great app that inspired Aria (www.ergonis.com).\n"), wxT(" G\u00FCnther Blaschek "))  +
    //I18N: - in about dialog
    wxString(_("\tJ.D. Koftinoff Software for libjdkmidi\n\tThe irrXML team for their great XML parser\n")) +
    //I18N: - in about dialog
    wxString(_("\tThe wxWidgets team\n")) +
    //I18N: - in about dialog
    wxString(_("\tThe Tango icon set\n")) +
    //I18N: - in about dialog
    wxString(_("\tWindows port by Alexis Archambault\n")) +
    //I18N: - in about dialog
    wxString(_("\nTranslations:\n") +
             wxString( wxT("\t it : Gianluca Pignalberi\n")) +
             wxString( wxT("\t de : Friedrich Weber\n"))
             );
    
    wxTextCtrl* text_area = new wxTextCtrl(this, 1, about_text, wxPoint(0,174), wxSize(517,500-174),
                                 wxTE_MULTILINE | wxTE_READONLY);
    
#ifdef __WXMAC__
    text_area->MacCheckSpelling(false);
#endif
    
    wxMenuBar* menuBar = new wxMenuBar();
    
    wxMenu* window = new wxMenu();
    window->Append(wxID_CLOSE, wxString(_("Close"))+wxT("\tCtrl-W"));
    
    menuBar->Append(window, wxT("Window"));
    SetMenuBar(menuBar);
    
    Connect(wxID_CLOSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(AboutDialog::onCloseMenu));
    Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(AboutDialog::onClose));
}

// ----------------------------------------------------------------------------------------------------------

void AboutDialog::onCloseMenu(wxCommandEvent& evt)
{
    Destroy();
}

// ----------------------------------------------------------------------------------------------------------

void AboutDialog::onClose(wxCloseEvent& evt)
{
    Destroy();
}

// ----------------------------------------------------------------------------------------------------------

void AboutDialog::show()
{
    Centre();
    Show();
}

// ----------------------------------------------------------------------------------------------------------

