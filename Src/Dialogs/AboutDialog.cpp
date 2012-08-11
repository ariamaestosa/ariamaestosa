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
#include "version.h"


#if wxUSE_WEBVIEW && !defined(__WXMSW__)
#include <wx/webview.h>
#endif

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
#include <wx/sizer.h>

#include "IO/IOUtils.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

AboutDialog::AboutDialog(wxWindow* parent) : wxFrame(parent, wxID_ANY,  _("About Aria Maestosa"),
                                                     wxDefaultPosition, wxSize(517, 600) )
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    wxBitmap titleBitmap;
    titleBitmap.LoadFile( getResourcePrefix() + wxT("title.jpg") , wxBITMAP_TYPE_JPEG );
    
    AboutImage* img = new AboutImage(this, wxID_ANY, titleBitmap);
    sizer->Add(img, 0, wxEXPAND);
    
    //I18N: - in about dialog
    wxString about_text = wxString(wxT("[h1]")) + wxString::Format(_("version %s"), VERSION_STRING ) +
                          wxT("[/h1]\n\n") + 
    //I18N: - in about dialog
    wxString(wxT("[h1]")) + _("Thanks to:") + wxT("[/h1]\n\n\t") +
    //I18N: - in about dialog
    wxString::Format(_("Ergonis Software and %s for making EasyBeat,\n\ta great app that inspired Aria (www.ergonis.com)."), wxT(" G\u00FCnther Blaschek "))  +
    //I18N: - in about dialog
    wxT("\n\t") + wxString(_("J.D. Koftinoff Software for libjdkmidi")) +
    //I18N: - in about dialog
    wxT("\n\t") + wxString(_("The irrXML team for their XML parser")) +
    //I18N: - in about dialog
    wxT("\n\t") + wxString(_("The wxWidgets team")) +
    //I18N: - in about dialog
    wxT("\n\t") + wxString(_("The Tango icon set")) +
    //I18N: - in about dialog
    wxT("\n\t") + wxString(_("Windows port by Alexis Archambault")) +
    //I18N: - in about dialog
    wxT("\n\n[h1]") + wxString(_("Translations:") +
             wxString(wxT("[/h1]\n\n\t")) + wxString( wxT("Chinese : Scilenso")) +
             wxString(wxT("\n\t")) + wxString( wxT("German : Friedrich Weber")) +
             wxString(wxT("\n\t")) + wxString( wxT("Italian : Gianluca Pignalberi")) +
             wxString(wxT("\n\t")) + wxString( wxT("Japanese : Jessie Wanner") ) +
             wxString(wxT("\n\t")) + wxString( wxT("Portuguese : guiagge") ) +
             wxString(wxT("\n\t")) + wxString( wxT("Russian : Ruslan Tertyshny & Artem Krosheninnikov") ) +
             wxString(wxT("\n\t")) + wxString( wxT("Spanish : Othyro") )
             );
    
#if wxUSE_WEBVIEW && !defined(__WXMSW__)
    about_text.Replace("\n","<br/>");
    about_text.Replace("[h1]","<b>");
    about_text.Replace("[/h1]","</b>");
    about_text.Replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
    wxWebView* text_area = wxWebView::New(this, wxID_ANY, "about:blank");
    text_area->SetPage(wxString("<html><body style=\"padding: 0px; margin: 5px; font-size: 90%;\">") +
                       about_text + wxString("</body></html>"), "file://");
    sizer->Add(text_area, 1, wxEXPAND);
#else
    about_text.Replace(wxT("[h1]"),"");
    about_text.Replace(wxT("[/h1]"),"");
    wxTextCtrl* text_area = new wxTextCtrl(this, 0, about_text, wxPoint(5,179), wxSize(507,396),
                                           wxTE_MULTILINE | wxTE_READONLY | wxNO_BORDER);
    sizer->Add(text_area, 1, wxEXPAND);
    #ifdef __WXMAC__
    text_area->MacCheckSpelling(false);
    #endif
#endif

    wxMenuBar* menuBar = new wxMenuBar();
    
    wxMenu* window = new wxMenu();
    window->Append(wxID_CLOSE, wxString(_("Close"))+wxT("\tCtrl-W"));
    
    menuBar->Append(window, wxT("Window"));
    SetMenuBar(menuBar);

    SetBackgroundColour(*wxWHITE);
    
    Connect(wxID_CLOSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(AboutDialog::onCloseMenu));
    Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(AboutDialog::onClose));
    
    SetSizer(sizer);
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

