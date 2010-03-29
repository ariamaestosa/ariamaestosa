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
#include "wx/intl.h"

namespace AriaMaestosa
{
    
    class Track; // forward
    class DrumChoice;
    class RelativeXCoord;
    
    class DrumInfo
    {
    public:
        DrumInfo(int midiKey, const bool a_section=false);
        int midiKey;
        bool section;
        bool sectionExpanded;
    };
    
    class DrumEditor : public Editor
    {
        bool showUsedDrumsOnly;
        
        AriaRenderArray drum_names_renderer;
        
        /** says in which order the drums are drawn, how they're divided in sections, etc. */
        std::vector<DrumInfo> drums;
        
        /** says where each midiKey is located in the vector. */
        int midiKeyToVectorID[128];

    public:
        
        DrumEditor(Track* track);
        ~DrumEditor();
        
        void useCustomDrumSet();
        void useDefaultDrumSet();
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void updatePosition(const int from_y, const int to_y, const int width, const int height,
                            const int barHeight);
        

        /** @return vector that says in which order the drums are drawn, how they're divided in sections, etc. */
        const std::vector<DrumInfo>& getDrums() const { return drums; }
        
        /** @return array (of size 128) that says where each midiKey is located inside the vector. */
        const int* getMidiKeyToVectorID() const { return midiKeyToVectorID; }
        
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
        virtual void noteClicked(const int id);
        
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
    };
    
}

#endif
