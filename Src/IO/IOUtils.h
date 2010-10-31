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


#ifndef __IO_UTILS_H__
#define __IO_UTILS_H__

/** @defgroup io I/O */

#include <wx/string.h>

class wxFileOutputStream;

namespace AriaMaestosa
{
    
    class Sequence;
    
    /** @ingroup io */
    wxString to_wxString(int i);
    
    /** @ingroup io */
    wxString to_wxString(float f);
    
    /** @ingroup io */
    void writeData(wxString data, wxFileOutputStream& fileout);
    
    wxString extract_filename(wxString filepath);
    
    /** @ingroup io */
    wxString extract_path(wxString str);
    
    /** @ingroup io */
    wxString showFileDialog(wxString message, wxString defaultDir,
                            wxString filename, wxString wildcard, bool save);
    
    /** Returns the path to the directory where resource files are located. The returned path
      * always contains a trailing '/', i.e. you can do getResourcePrefix() + "somefile.png"
      * @ingroup ip
      */
    wxString getResourcePrefix();
    
    /** @ingroup io */
    long atoi_u(wxString s);
    
    /** @ingroup io */
    wxString fromCString(const char* chars);
    
    /** @ingroup io */
    wxString fromCString(char* chars);
    
    /** @ingroup io */
    void assertFailed(wxString message);
    
    /**
      * @ingroup io
      * @return remove the path to a file and file extension, returning file name only
      */
    wxString extractTitle(const wxString& inputPath);

}

#endif
