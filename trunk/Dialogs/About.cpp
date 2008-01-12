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

#include "Dialogs/About.h"

#include "wx/utils.h"
#include "wx/stdpaths.h"

#include "IO/IOUtils.h"

namespace AriaMaestosa {
	
AboutDialog::AboutDialog() : wxDialog(NULL, wxID_ANY,  _("About Aria Maestosa"), wxDefaultPosition, wxSize(517, 500) )
{
	INIT_LEAK_CHECK();
	
	//wxImage* titleImage=new wxImage(wxStandardPaths::Get().GetResourcesDir() + wxT("/")  + wxT("title.jpg"), wxBITMAP_TYPE_JPEG);
    //wxBitmap* titleBitmap=new wxBitmap(*titleImage, wxBITMAP_TYPE_PNG);

    wxBitmap titleBitmap;
    titleBitmap.LoadFile( getResourcePrefix()  + wxT("title.jpg") , wxBITMAP_TYPE_JPEG );
    picture=new wxBitmapButton(this, 0, titleBitmap, wxPoint(0,0), wxSize(517,174) );

	wxString about_text =  wxString(_("version ")) + wxT("1.1b7") +
		wxString(_("\n\nThanks to:\n\n\tErgonis Software and")) + wxT(" G\u00FCnther Blaschek ") +
		wxString(_("for making EasyBeat,\n\t\ta great app that inspired Aria (www.ergonis.com).\n")) +
		wxString(_("\tJ.D. Koftinoff Software for libjdkmidi\n\tThe irrXML team for their great XML parser\n"))+
		wxString(_("\tThe wxWidgets team\n\twww.freesmug.org for allowing me to access an intel mac")) +
        wxString(_("\n\nTranslations:\n") +
                 wxString( wxT("\t it : Gianluca Pignalberi")) );


	textArea = new wxTextCtrl(this, 1, about_text, wxPoint(0,174), wxSize(517,500-174), wxTE_MULTILINE | wxTE_READONLY);
#ifdef __WXMAC__
	textArea->MacCheckSpelling(false);
#endif
}

void AboutDialog::show()
{
	Centre();
	Show();
}

}
