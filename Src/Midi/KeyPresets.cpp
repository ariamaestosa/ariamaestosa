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
#include "Midi/KeyPresets.h"

using namespace AriaMaestosa;

DEFINE_SINGLETON(AriaMaestosa::KeyPresetGroup);

// ----------------------------------------------------------------------------------------------------------

KeyPresetGroup::KeyPresetGroup() : PresetGroup("key")
{
    read(KeyPreset::factory);
}

// ----------------------------------------------------------------------------------------------------------

KeyPreset::KeyPreset(const char* name, const KeyInclusionType notes[131])
{
    m_name = wxString(name, wxConvUTF8);
    for (int n=0; n<131; n++)
    {
        m_key_notes[n] = notes[n];
    }
}

// ----------------------------------------------------------------------------------------------------------

KeyPreset::KeyPreset(const char* name, const char* contents)
{
    m_name = wxString(name, wxConvUTF8);

    // saved in MIDI order, not in my weird pitch ID order
    for (int n=4; n<131; n++)
    {
        char c = contents[n-4];
        if (c == '2') m_key_notes[n] = KEY_INCLUSION_FULL;
        else if (c == '1') m_key_notes[n] = KEY_INCLUSION_ACCIDENTAL;
        else if (c == '0') m_key_notes[n] = KEY_INCLUSION_NONE;
        else
        {
            std::cerr << "Warning : malformed key in .aria file for track "
                      << getName().mb_str()
                      << " : 'custom' key type needs a value made of 0/1 characters"
                      << std::endl;
            if (c == '\0') break;
        }
    }
    m_key_notes[3] = KEY_INCLUSION_NONE;
    m_key_notes[2] = KEY_INCLUSION_NONE;
    m_key_notes[1] = KEY_INCLUSION_NONE;
    m_key_notes[0] = KEY_INCLUSION_NONE;    
}

// ----------------------------------------------------------------------------------------------------------

IPreset* KeyPreset::clone(wxString newName)
{
    return new KeyPreset(newName.mb_str(), m_key_notes);
}

// ----------------------------------------------------------------------------------------------------------

wxString KeyPreset::getStringizedForm()
{
    // saved in MIDI order, not in my weird pitch ID order
    char value[128];
    for (int n=130; n>3; n--)
    {
        value[n-4] = '0' + (int)m_key_notes[n];
    }
    value[127] = '\0';
    return wxString(value, wxConvUTF8);
}

// ----------------------------------------------------------------------------------------------------------
