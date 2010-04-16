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
#include "AriaCore.h"
#include <iostream>


#include "wx/config.h"
#include "wx/intl.h"
#include "wx/stdpaths.h"

#include <vector>

namespace AriaMaestosa
{
    
    wxLocale* locale;
    static long language_aria_id = 0;
    static long language_wx_id = wxLANGUAGE_DEFAULT;
    
    const char* WINDOWS_LANG_DIR = "Languages";
    
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
    
    void buildLanguageList()
    {
        // build list of existing languages
        languages.push_back( AriaLanguage( wxLANGUAGE_DEFAULT, wxT("System") ) );        // 0
        languages.push_back( AriaLanguage( wxLANGUAGE_ENGLISH, wxT("English") ) );       // 1
        languages.push_back( AriaLanguage( wxLANGUAGE_FRENCH,  wxT("Fran\u00E7ais") ) ); // 2
        languages.push_back( AriaLanguage( wxLANGUAGE_ITALIAN, wxT("Italiano") ) );      // 3
        languages.push_back( AriaLanguage( wxLANGUAGE_GERMAN,  wxT("Deutsch") ) );       // 4
    }
    
    void initLanguageSupport()
    {
        if (languages.size() == 0) buildLanguageList();
        
        // read language from preferences
        language_aria_id = Core::getPrefsValue("lang");
        if (language_aria_id == -1)
        {
            // couldn't read from prefs, use default
            language_aria_id = 0;
            language_wx_id = wxLANGUAGE_DEFAULT;
            std::cout << "failed to read language from prefs" << std::endl;
        }
        // preferences contain Aria-ID of supported languages (see list above)
        // we need to convert this to a wx language code before using
        language_wx_id = languages[language_aria_id].wxlangcode;
        
        // check if this language is known. not really necessary, just informative
        // in case something goes wrong
        const int lang_amount = languages.size();
        bool language_known = false;
        for(int i=0; i<lang_amount; i++)
        {
            if (language_wx_id == languages[i].wxlangcode)
            {
                std::cout << "language : " << languages[i].langname.mb_str() << std::endl;
                language_known = true;
            }
        }
        if (!language_known) std::cout << "Warning, the language code stored in preferences is unknown to Aria" << std::endl;
        
        // load language if possible, fall back to english otherwise
        if (wxLocale::IsAvailable(language_wx_id))
        {
            locale = new wxLocale( language_wx_id, wxLOCALE_CONV_ENCODING );
            
#if defined(__WXGTK__)
            // add locale search paths
            //TODO: enable wx to find the catalogs when Aria is ran in-place, if that's possible at all. otherwise print a warning
            locale->AddCatalogLookupPathPrefix(wxT("/usr"));
            locale->AddCatalogLookupPathPrefix(wxT("/usr/local"));
            wxStandardPaths* paths = (wxStandardPaths*) &wxStandardPaths::Get();
            wxString prefix = paths->GetInstallPrefix();
            locale->AddCatalogLookupPathPrefix( prefix );
#elif defined(__WXMSW__)
            locale->AddCatalogLookupPathPrefix(getResourcePrefix() + WINDOWS_LANG_DIR); 
#endif
            
            locale->AddCatalog(wxT("aria_maestosa"));
            
            if (! locale->IsOk() )
            {
                std::cout << "selected language is wrong" << std::endl;
                delete locale;
                locale = new wxLocale( wxLANGUAGE_ENGLISH );
                language_aria_id = 0;
                language_wx_id = wxLANGUAGE_ENGLISH;
            }
        }
        else
        {
            std::cout << "The selected language is not supported by your system."
            << "Try installing support for this language." << std::endl;
            locale = new wxLocale( wxLANGUAGE_ENGLISH );
            language_aria_id = 0;
            language_wx_id = wxLANGUAGE_ENGLISH;
        }
        
    }
    
    /*
     * Returns the list of langauges in a human-readable form.
     * Useful to e.g. create a combo of available languages.
     */
    wxArrayString getLanguageList()
    {
        if (languages.size() == 0) buildLanguageList();
        
        wxArrayString list;
        
        const int lang_amount = languages.size();
        for(int i=0; i<lang_amount; i++)
        {
            list.Add(languages[i].langname);
        }
        
        return list;
    }
    
    int getDefaultLanguageAriaID()
    {
        return language_aria_id;
    }
    
    
}

