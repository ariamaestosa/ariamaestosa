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

#ifndef __KEY_PRESET_H__
#define __KEY_PRESET_H__

#include "wx/string.h"
#include "PresetManager.h"
#include "Midi/Track.h"

namespace AriaMaestosa
{

    /**
      * @brief contains one user-defined key preset
      * @ingroup midi
      */
    class KeyPreset : public IPreset
    {
        wxString m_name;
        KeyInclusionType m_key_notes[131];
    public:
        /**
          * @brief deserialization constructor : builds and object from its saved stringized representation
          */
        KeyPreset(const char* name, const char* contents);
        
        /**
          * @brief constructor to create a new preset that is not yet saved to disk. Add it to a PresetGroup
          * to make it managed.
          */
        KeyPreset(const char* name, const KeyInclusionType notes[131]);

        virtual ~KeyPreset() {}
        virtual wxString getName() { return m_name; }
        virtual wxString getStringizedForm();
        
        static IPreset* factory(const char* name, const char* stringizedForm)
        {
            return new KeyPreset(name, stringizedForm);
        }
        
        const KeyInclusionType* getArray() const
        {
            return m_key_notes;
        }

    };

    /**
      * @brief holds and manages the list of user-defined key presets
      * @ingroup midi
      */
    class KeyPresetGroup : public PresetGroup, public Singleton<KeyPresetGroup>
    {
        friend class Singleton<KeyPresetGroup>;
        KeyPresetGroup();
    public:
    };

}

#endif
