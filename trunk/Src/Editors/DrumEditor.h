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


#ifndef __DRUM_EDITOR_H__
#define __DRUM_EDITOR_H__

#include "Editors/Editor.h"
#include "Renderers/RenderAPI.h"
#include <vector>
#include <wx/intl.h>

namespace AriaMaestosa
{
    
    class Track; // forward
    class DrumChoice;
    class RelativeXCoord;
    
    
    /**
      * @brief   a note editor for drum tracks 
      * @ingroup editors
      */
    class DrumEditor : public Editor
    {
        class DrumInfo
        {
        public:
            DrumInfo(int midiKey, const bool a_section=false);
            int  m_midi_key;
            bool m_section;
            bool m_section_expanded;
        };
        
        bool m_show_used_drums_only;
        
        AriaRenderArray m_drum_names_renderer;
        
        /** says in which order the drums are drawn, how they're divided in sections, etc. */
        std::vector<DrumInfo> m_drums;
        
        /** says where each midiKey is located in the vector. */
        int m_midi_key_to_vector_ID[128];

    public:
        
        DrumEditor(GraphicalTrack* track);
        ~DrumEditor();
        
        void useCustomDrumSet();
        void useDefaultDrumSet();
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        /** @return vector that says in which order the drums are drawn, how they're divided in sections, etc. */
        const std::vector<DrumInfo>& getDrums() const { return m_drums; }
        
        /** @return array (of size 128) that says where each midiKey is located inside the vector. */
        const int* getMidiKeyToVectorID() const { return m_midi_key_to_vector_ID; }
        
        /** event callback from base class */
        virtual void mouseDown(RelativeXCoord x, int y);

        /** event callback from base class */
        virtual void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                             RelativeXCoord mousex_initial, int mousey_initial);
        
        int getDrumAtY(const int given_y);
        int getYForDrum(const int given_drumID);
        
        /** implemented from base class Editor's required interface */
        virtual int getYScrollInPixels();
        
        /** implemented from base class Editor's required interface */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snappedX, const int mouseY);
        
        /** implemented from base class Editor's required interface */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial);
        
        /** implemented from base class Editor's required interface */
        virtual void moveNote(Note& note, const int relativeX, const int relativeY);
        
        /** implemented from base class Editor's required interface */
        virtual wxString getName() const { return _("Drum Editor"); }
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
        {
            // not supported in this editor
            assert(false);
        }
        
        virtual NotationType getNotationType() const { return DRUM; }
        
        bool showOnlyUsedDrums() const { return m_show_used_drums_only; }
        
        /** Set the value of m_show_used_drums_only.
          * @note You need to call DrumEditor::useCustomDrumSet or DrumEditor::useDefaultDrumSet
          *       manually after calling this.
          */
        void setShowOnlyUsedDrums(bool b) { m_show_used_drums_only = b; }
    };
    
}

#endif
