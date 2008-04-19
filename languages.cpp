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
#include "wx/stdpaths.h"

#include <vector>

namespace AriaMaestosa {

wxLocale* locale;
long language;


class AriaLanguage
{
public:
    long wxlangcode;
    wxString langname;
    
    AriaLanguage(long wxlangcode, wxString langname)
    {
        AriaLanguage::wxlangcode = wxlangcode;
        AriaLanguage::langname = langname;
    }
    
};

std::vector<AriaLanguage> languages;

void initLanguageSupport(wxConfig* prefs)
{
    // build list of existing languages
    languages.push_back( AriaLanguage( wxLANGUAGE_DEFAULT, wxT("System") ) );        // 0
    languages.push_back( AriaLanguage( wxLANGUAGE_ENGLISH, wxT("English") ) );       // 1
    languages.push_back( AriaLanguage( wxLANGUAGE_FRENCH,  wxT("Fran\u00E7ais") ) ); // 2
    languages.push_back( AriaLanguage( wxLANGUAGE_ITALIAN, wxT("Italiano") ) );      // 3
    languages.push_back( AriaLanguage( wxLANGUAGE_GERMAN,  wxT("Deutsch") ) );       // 4
    
    // read language from preferences
	if(! prefs->Read( wxT("lang"), &language) )
	{
        // couldn't read from prefs, use default
		language = wxLANGUAGE_DEFAULT;
		std::cout << "failed to read prefs" << std::endl;
	}

    // check if this language is known. not really necessary, just informative in case something goes wrong
    const int lang_amount = languages.size();
    bool language_known = false;
    for(int i=0; i<lang_amount; i++)
    {
        if(language == languages[i].wxlangcode)
        {
            std::cout << "language : " << languages[i].langname.mb_str() << std::endl;
            language_known = true;
        }
    }
    if(!language_known) std::cout << "Warning, the language code stored in preferences is unknown to Aria" << std::endl;
    
    // load language if possible, fall back to english otherwise
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
    
    const int lang_amount = languages.size();
    for(int i=0; i<lang_amount; i++)
    {
        list.Add(languages[i].langname);
    }
    
    return list;
}

int getDefaultLanguageID()
{
    
    const int lang_amount = languages.size();
    for(int i=0; i<lang_amount; i++)
    {
        if(language == languages[i].wxlangcode)
        {
            return i;
        }
    }
    return 0;
    
}

void setDefaultLanguage(wxString langname)
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    
    // find which one is selected and write its ID to te preferences config file
    const int lang_amount = languages.size();
    for(int i=0; i<lang_amount; i++)
    {
        if(langname == languages[i].langname)
        {
            prefs->Write( wxT("lang"), languages[i].wxlangcode );
        }
    }

    prefs->Flush();
}

}

