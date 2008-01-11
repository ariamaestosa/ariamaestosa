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

#include "languages.h"
#include <iostream>
#include "wx/config.h"

namespace AriaMaestosa {

wxLocale* locale;
long language;

void initLanguageSupport(wxConfig* prefs)
{

	if(! prefs->Read( wxT("lang"), &language) )
	{
		language = wxLANGUAGE_DEFAULT;
		std::cout << "failed to read prefs" << std::endl;
	}

	if(language == wxLANGUAGE_DEFAULT) std::cout << "language : default" << std::endl;
	else if(language == wxLANGUAGE_ENGLISH) std::cout << "language : english" << std::endl;
	else if(language == wxLANGUAGE_FRENCH) std::cout << "language : french" << std::endl;
    else if(language == wxLANGUAGE_ITALIAN) std::cout << "language : italian" << std::endl;
    else std::cout << "language : UNKNOWN!!!" << std::endl;
    
	if(wxLocale::IsAvailable(language))
	{
	    locale = new wxLocale( language, wxLOCALE_CONV_ENCODING );

        #ifdef __WXGTK__
        // add locale search paths
        locale->AddCatalogLookupPathPrefix(wxT("/usr"));
        locale->AddCatalogLookupPathPrefix(wxT("/usr/local"));
        wxStandardPaths* paths = (wxStandardPaths*) &wxStandardPaths::Get();
        wxString prefix = paths->GetInstallPrefix();
        locale->AddCatalogLookupPathPrefix( prefix );
        #endif

	    locale = new wxLocale( language, wxLOCALE_CONV_ENCODING );
        locale->AddCatalog(wxT("aria_maestosa"));

	    if(! locale->IsOk() )
	    {
		    std::cout << "selected language is wrong" << std::endl;
		    delete locale;
		    locale = new wxLocale( wxLANGUAGE_ENGLISH );
            language = wxLANGUAGE_ENGLISH;
	    }
    }
    else
    {
        std::cout << "The selected language is not supported by your system."
                  << "Try installing support for this language." << std::endl;
		locale = new wxLocale( wxLANGUAGE_ENGLISH );
        language = wxLANGUAGE_ENGLISH;
    }

}

wxArrayString getLanguageList()
{
    wxArrayString list;
    list.Add(wxT("System"));            // 0
    list.Add(wxT("English"));           // 1
    list.Add(wxT("Fran\u00E7ais"));     // 2
    list.Add(wxT("Italiano"));          // 3
    return list;
}

int getDefaultLanguageID()
{
    if(language == wxLANGUAGE_DEFAULT)
        return 0;
    else if(language == wxLANGUAGE_ENGLISH)
        return 1;
    else if(language == wxLANGUAGE_FRENCH)
        return 2;
    else if(language == wxLANGUAGE_ITALIAN)
        return 3;
    else return 0;
}

void setDefaultLanguage(wxString langname)
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    // wxLANGUAGE_DEFAULT
    // wxLANGUAGE_ENGLISH
    // wxLANGUAGE_FRENCH
    
    if( langname == wxT("System") )
    {
        prefs->Write( wxT("lang"), wxLANGUAGE_DEFAULT );
    }
    else if( langname == wxT("English") )
    {
        prefs->Write( wxT("lang"), wxLANGUAGE_ENGLISH );
    }
    else if( langname == wxT("Fran\u00E7ais") )
    {
        prefs->Write( wxT("lang"), wxLANGUAGE_FRENCH );
    }
    else if( langname == wxT("Italiano") )
    {
        prefs->Write( wxT("lang"), wxLANGUAGE_ITALIAN );
    }
    prefs->Flush();
}

}