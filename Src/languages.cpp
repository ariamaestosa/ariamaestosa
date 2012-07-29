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
#include "PreferencesData.h"
#include <iostream>


#include <wx/config.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>

#include <vector>

namespace AriaMaestosa
{
    
    wxLocale* locale;
    static long language_aria_id = 0;
    static long language_wx_id = wxLANGUAGE_DEFAULT;
    
#ifdef __WXMSW__
    const wxChar* WINDOWS_LANG_DIR = wxT("Languages");
#endif
    
    class AriaLanguage
    {
    public:
        long     m_wx_langcode;
        wxString m_langname;
        
        AriaLanguage(long wxlangcode)
        {
            m_wx_langcode = wxlangcode;
            
            if (wxlangcode == wxLANGUAGE_DEFAULT)
            {
                m_langname = wxT("System");
            }
            else
            {
            m_langname    = wxLocale::GetLanguageInfo(wxlangcode)->Description;
            }
        }
    };
    
    std::vector<AriaLanguage> languages;
    
    void buildLanguageList()
    {
        // build list of existing languages
        languages.push_back( AriaLanguage(wxLANGUAGE_DEFAULT) );               // 0
        languages.push_back( AriaLanguage(wxLANGUAGE_ENGLISH) );               // 1
        languages.push_back( AriaLanguage(wxLANGUAGE_FRENCH) );                // 2
        languages.push_back( AriaLanguage(wxLANGUAGE_ITALIAN) );               // 3
        languages.push_back( AriaLanguage(wxLANGUAGE_GERMAN) );                // 4
        languages.push_back( AriaLanguage(wxLANGUAGE_JAPANESE) );              // 5
        languages.push_back( AriaLanguage(wxLANGUAGE_SPANISH) );               // 6
        languages.push_back( AriaLanguage(wxLANGUAGE_PORTUGUESE_BRAZILIAN) );  // 7
        languages.push_back( AriaLanguage(wxLANGUAGE_RUSSIAN) );               // 8
        languages.push_back( AriaLanguage(wxLANGUAGE_CHINESE_SIMPLIFIED) );    // 9
        
        /*
#if defined(__WXMAC__)
        wxString dirname = getResourcePrefix();
#elif defined(__WXMSW__)
        wxString dirname = getResourcePrefix() + WINDOWS_LANG_DIR;
#else
        wxStandardPaths* paths = (wxStandardPaths*) &wxStandardPaths::Get();
        wxString dirname = paths->GetInstallPrefix() + "/share/locale";
#endif
        
        wxDir dir(dirname);
        
        if (not dir.IsOpened())
        {
            fprintf(stderr, "[Languages] Failed to list directory '%s'\n", (const char*)dirname.utf8_str());
            return;
        }
        
        wxString filename;

        bool cont = dir.GetFirst(&filename);
        while ( cont )
        {
#ifdef __WXMAC__
            if (filename.EndsWith(".lproj"))
#elif defined(__WXMSW__)
            // no check on Windows
#else
            if (wxFileExists(dirname + "/" + filename + "/LC_MESSAGES/aria_maestosa.mo"))
#endif
            {
                printf("* Language %s\n", (const char*)filename.utf8_str());
            }
            
            cont = dir.GetNext(&filename);
        }
         */
    }
    
    void initLanguageSupport()
    {
        if (languages.size() == 0) buildLanguageList();
        
        // read language from preferences
        language_aria_id = PreferencesData::getInstance()->getIntValue("lang");
        if (language_aria_id == -1)
        {
            // couldn't read from prefs, use default
            language_aria_id = 0;
            language_wx_id = wxLANGUAGE_DEFAULT;
            std::cout << "[initLanguageSupport] failed to read language from prefs (maybe it's not set yet)" << std::endl;
        }
        // preferences contain Aria-ID of supported languages (see list above)
        // we need to convert this to a wx language code before using
        language_wx_id = languages[language_aria_id].m_wx_langcode;
        
        // check if this language is known. not really necessary, just informative
        // in case something goes wrong
        const int lang_amount = languages.size();
        bool language_known = false;
        for (int i=0; i<lang_amount; i++)
        {
            if (language_wx_id == languages[i].m_wx_langcode)
            {
                std::cout << "[initLanguageSupport] language : " << languages[i].m_langname.mb_str() << std::endl;
                language_known = true;
            }
        }
        if (!language_known) std::cerr << "[initLanguageSupport] Warning, the language code stored in preferences is unknown to Aria\n";
        
        // load language if possible, fall back to english otherwise
        if (wxLocale::IsAvailable(language_wx_id))
        {
            locale = new wxLocale( language_wx_id );
            
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
            std::cerr << "The selected language is not supported by your system."
                      << "Try installing support for this language.\n";
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
        for (int i=0; i<lang_amount; i++)
        {
            list.Add(languages[i].m_langname);
        }
        
        return list;
    }
    
    int getDefaultLanguageAriaID()
    {
        return language_aria_id;
    }
    
    
}

