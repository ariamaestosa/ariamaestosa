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


#ifndef _ioutils
#define _ioutils

#include "wx/wx.h"
#include <string>
class wxFileOutputStream;

namespace AriaMaestosa
{

class Sequence;

wxString to_wxString(int i);
wxString to_wxString(float f);
void writeData(wxString data, wxFileOutputStream& fileout);

wxString extract_filename(wxString filepath);
wxString extract_path(wxString str);

wxString showFileDialog(wxString message, wxString defaultDir,
                        wxString filename, wxString wildcard, bool save);

/** Returns the path to the directory where resource files are located. The returned path
    always contains a trailing '/', i.e. you can do getResourcePrefix() + "somefile.png" */
wxString getResourcePrefix();

long atoi_u(wxString s);

wxString fromCString(const char* chars);
wxString fromCString(char* chars);

//void checkAppPath();

void assertFailed(wxString message);

}

#endif
