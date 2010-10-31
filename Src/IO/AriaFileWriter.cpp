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

#include "AriaFileWriter.h"

#include "Midi/Sequence.h"

#include <wx/string.h>
#include <wx/wfstream.h>
#include "irrXML/irrXML.h"

namespace AriaMaestosa
{
    
    void saveAriaFile(Sequence* sequence, wxString filepath)
    {
        // do not override a file previously there. If a file was there, move it to a different name and do not delete
        // it until we know the new file was successfully saved
        wxString temp_name = filepath + wxT("~");
        const bool overriding_file = wxFileExists(filepath);
        if (overriding_file) wxRenameFile( filepath, temp_name, false );
        
        wxFileOutputStream file( filepath );
        sequence->saveToFile(file);
        
        if (overriding_file) wxRemoveFile( temp_name );
    }
    
    bool loadAriaFile(Sequence* sequence, wxString filepath)
    {
        wxCSConv cs( wxFONTENCODING_UTF8 );
        wxCharBuffer output = cs.cWC2MB(filepath.wc_str());
        
        irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader( (char*)output.data() );
        
        if (! sequence->readFromFile(xml) )
        {
            std::cout << "LOADING SEQUENCE FAILED" << std::endl;
            delete xml;
            return false;
        }
        
        delete xml;
        return true;
    }
    
}
