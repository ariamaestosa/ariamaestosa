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

#include "Midi/DrumChoice.h"

#include <wx/string.h>

// Necessary to handle translations
#include <wx/wx.h>

#include <iostream>

using namespace AriaMaestosa;

static const std::pair<int, wxString> g_drumkit_names[] =
{
    std::pair<int, wxString>(0, _("Standard")),
    std::pair<int, wxString>(8, _("Room kit")),
    std::pair<int, wxString>(16, _("Power kit")),
    std::pair<int, wxString>(24, _("Electronic")),
    std::pair<int, wxString>(25, _("Analog")),
    std::pair<int, wxString>(32, _("Jazz")),
    std::pair<int, wxString>(40, _("Brush")),
    std::pair<int, wxString>(48, _("Orchestral")),
    std::pair<int, wxString>(56, _("Special Effects")),
    /** End sentinel */
    std::pair<int, wxString>(-1, _("Invalid Drumkit"))
};


DrumChoice::DrumChoice(const int defaultDrum, IDrumChoiceListener* listener)
{
    m_selected_drumkit = defaultDrum;
    m_listener = listener;
}

void DrumChoice::setDrumkit(const int drumkit, const bool generateEvent)
{
    m_selected_drumkit = drumkit;
    
    if (generateEvent and m_listener != NULL) m_listener->onDrumkitChanged(drumkit);
}


const wxString DrumChoice::getDrumkitName(int id)
{
    int n;
    for (n=0; g_drumkit_names[n].first != -1; n++)
    {
        if (g_drumkit_names[n].first == id)
        {
            return wxGetTranslation(g_drumkit_names[n].second);
        }
    }
    
    std::cerr << "wrong drumset ID: " << id << std::endl;
    return wxGetTranslation(g_drumkit_names[n].second);
}
