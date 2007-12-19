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


#include "IO/IOUtils.h"
#include "IO/AriaFileWriter.h"
#include "GUI/MainFrame.h"
#include "Midi/Sequence.h"

#include "main.h"

#include "wx/wx.h"
#include "wx/wfstream.h"
#include "wx/stdpaths.h"

#include <iostream>

namespace AriaMaestosa {

wxString to_wxString(int i)
{
	char buffer[10];
	sprintf (buffer, "%d", i);

	return fromCString(buffer);
}

wxString to_wxString(float f)
{
	char buffer[15];
	sprintf (buffer, "%f", f);
	return fromCString(buffer);
}

void assertFailed(wxString message)
{
	std::cerr << toCString(message) << std::endl;

	wxMessageBox( message );
/*
	wxString filepath;

save:
	filepath = showFileDialog(_("Select destination file"), wxT(""),  _("crash_recovery.aria"),
								wxT("Aria Maestosa file|*.aria"), true);

	if(!filepath.IsAscii())
	{
		wxMessageBox( _("Invalid file name. (Sorry, accentuated characters are not supported)"));
		goto save;
	}

    if(!filepath.IsEmpty())
	{

		if( wxFileExists(filepath) )
		{
			wxMessageBox(  _("The file already exists. Please choose another name."));
			goto save;
		}

        saveAriaFile(getMainFrame()->getCurrentSequence(), filepath);

    }// end if
*/
	exit(1);

}

void writeData(wxString data, wxFileOutputStream& fileout)
{
	fileout.Write( toCString(data), data.size());
}

wxString extract_filename(wxString filepath)
{
	return filepath.AfterLast('/');
}

wxString extract_path(wxString str)
{
        if(str.GetChar(str.Length()) == '/') return str.BeforeLast('/').BeforeLast('/');
	else return str.BeforeLast('/');
}


wxString fromCString(const char* chars)
{
	return wxString (chars, wxConvUTF8);
	//wchar_t* output;
	//FromWChar(output, wxNO_LEN, chars, wxNO_LEN);
	//return wxString(output);
}
wxString fromCString(char* chars)
{
	return wxString (chars, wxConvUTF8);
	//return s.mb_str();
	//return (char*)(s.ToAscii().data());
}


/*
 enum wxFontEncoding
 {
	 wxFONTENCODING_SYSTEM = -1,     // system default
	 wxFONTENCODING_DEFAULT,         // current default encoding

	 // ISO8859 standard defines a number of single-byte charsets
	 wxFONTENCODING_ISO8859_1,       // West European (Latin1)
	 wxFONTENCODING_ISO8859_2,       // Central and East European (Latin2)
	 wxFONTENCODING_ISO8859_3,       // Esperanto (Latin3)
	 wxFONTENCODING_ISO8859_4,       // Baltic (old) (Latin4)
	 wxFONTENCODING_ISO8859_5,       // Cyrillic
	 wxFONTENCODING_ISO8859_6,       // Arabic
	 wxFONTENCODING_ISO8859_7,       // Greek
	 wxFONTENCODING_ISO8859_8,       // Hebrew
	 wxFONTENCODING_ISO8859_9,       // Turkish (Latin5)
	 wxFONTENCODING_ISO8859_10,      // Variation of Latin4 (Latin6)
	 wxFONTENCODING_ISO8859_11,      // Thai
	 wxFONTENCODING_ISO8859_12,      // doesn't exist currently, but put it
									 // here anyhow to make all ISO8859
									 // consecutive numbers
	 wxFONTENCODING_ISO8859_13,      // Baltic (Latin7)
	 wxFONTENCODING_ISO8859_14,      // Latin8
	 wxFONTENCODING_ISO8859_15,      // Latin9 (a.k.a. Latin0, includes euro)
	 wxFONTENCODING_ISO8859_MAX,

	 // Cyrillic charset soup (see http://czyborra.com/charsets/cyrillic.html)
	 wxFONTENCODING_KOI8,            // we don't support any of KOI8 variants
	 wxFONTENCODING_ALTERNATIVE,     // same as MS-DOS CP866
	 wxFONTENCODING_BULGARIAN,       // used under Linux in Bulgaria


	 wxFONTENCODING_UTF7,            // UTF-7 Unicode encoding
	 wxFONTENCODING_UTF8,            // UTF-8 Unicode encoding

	 wxFONTENCODING_UNICODE,         // Unicode - currently used only by
									 // wxEncodingConverter class

	 wxFONTENCODING_MAX
 };
 */


long atoi_u(wxString s)
{
	long value;
	if(s.ToLong(&value)) return value;
	else
	{
		std::cerr << "WARNING: Could not parse number " << toCString(s) << std::endl;
		return -1;
	}
}


wxString showFileDialog(wxString message, wxString defaultDir,
						wxString filename,
						wxString wildcard, bool save)
{
	wxFileDialog* dialog = new wxFileDialog( NULL, message, defaultDir, filename, wildcard, (save?wxFD_SAVE:wxFD_OPEN));
	int answer = dialog->ShowModal();
	wxString path = dialog->GetPath();
	dialog->Hide();
	delete dialog;
	if(answer != wxID_OK) return wxT("");

	return path;
}

wxString getResourcePrefix()
{
#if defined(__WXMAC__)
		return wxStandardPaths::Get().GetResourcesDir() + wxT("/");
#elif defined(__WXGTK__)

		static bool app_in_place = wxFileExists( extract_path(wxStandardPaths::Get().GetExecutablePath())  + wxT("/Resources/collapse.jpg") );
		
		if(app_in_place)
		    return extract_path( wxStandardPaths::Get().GetExecutablePath() ) + wxT("/Resources/");
		else
		    return wxStandardPaths::Get().GetResourcesDir() + wxT("/");
#else

#warning "Resource Prefix undefined for your platform"
return wxT("./");

#endif
}

}
