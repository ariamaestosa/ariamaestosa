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

#include "PresetManager.h"
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>

#include "unit_test.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

PresetGroup::PresetGroup(const char* name)
{
    m_name = wxString(name, wxConvUTF8);
}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::add(IPreset* preset)
{
    m_presets.push_back(preset);
}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::remove(IPreset* preset, bool deleteIt)
{
    if (deleteIt) m_presets.erase(preset);
    else          m_presets.remove(preset);
}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::read(wxTextInputStream* source, PresetFactory preset)
{
    wxString countLine = source->ReadLine();
    long val;
    if (not countLine.ToLong(&val))
    {
        // TODO: error handling
        std::cerr << "Invalid preset file!\n";
        return;
    }

    for (int n=0; n<val; n++)
    {
        wxString nameLine = source->ReadLine();
        if (nameLine.IsEmpty())
        {
            // TODO: error handling
            std::cerr << "Invalid preset file!\n";
            return;
        }
        wxString contentsLine = source->ReadLine();
        if (contentsLine.IsEmpty())
        {
            // TODO: error handling
            std::cerr << "Invalid preset file!\n";
            return;
        }

        IPreset* thePreset = preset(nameLine.mb_str(), contentsLine.mb_str());
        m_presets.push_back(thePreset);
    }

}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::write(wxTextOutputStream* where)
{
    const int count = m_presets.size();
    where->WriteString(to_wxString(count) + wxT("\n"));

    for (int n=0; n<count; n++)
    {
        where->WriteString(m_presets[n].getName() + wxT("\n"));
        where->WriteString(m_presets[n].getStringizedForm() + wxT("\n"));
    }
}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::read(PresetFactory presetFactory)
{
    wxString prefix = wxStandardPaths::Get().GetUserDataDir();

    if (not wxFileExists(prefix + wxFileName::GetPathSeparator() + m_name + wxT(".ariapreset")))
    {
        // nothing to read
        return;
    }


    wxFileInputStream input( prefix + wxFileName::GetPathSeparator() + m_name + wxT(".ariapreset"));
    wxTextInputStream text( input );
    read(&text, presetFactory);
}

// ----------------------------------------------------------------------------------------------------------

void PresetGroup::write()
{
    wxString prefix = wxStandardPaths::Get().GetUserDataDir();
    if (not wxDirExists(prefix))
    {
        // FIXME: wxMkDir is not consistent across platforms xD
        #ifdef __WXMSW__
            const bool success = wxMkDir((const wchar_t*)prefix.c_str());
        #else
            const bool success = wxMkDir(prefix.mb_str(), 0777);
        #endif
        if (not success and not wxDirExists(prefix))
        {
            wxMessageBox(wxT("Sorry, would not create ") + prefix);
            return;
        }
    }

    wxFileOutputStream output( prefix + wxFileName::GetPathSeparator() + m_name + wxT(".ariapreset"));
    wxTextOutputStream text( output );
    write(&text);
}

// ----------------------------------------------------------------------------------------------------------
