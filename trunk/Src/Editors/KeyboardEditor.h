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


#ifndef _scrollpane_
#define _scrollpane_

#include "Editors/Editor.h"

#include "Config.h"

#include "Editors/RelativeXCoord.h"

namespace AriaMaestosa
{
    
    class Track; // forward
    class Sequence;
    class GraphicalTrack;
    
    class KeyboardEditor : public Editor
    {
        static const int Y_STEP_HEIGHT = 10;
                
    public:
        KeyboardEditor(Track* data);
        virtual ~KeyboardEditor();
        
        void mouseDown(RelativeXCoord x, const int y);
        
        void render();
        void render(RelativeXCoord mousex_current, int mousey_current,
                    RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                               RelativeXCoord& mousex_initial, int mousey_initial);
        
        int getYScrollInPixels();
        
        NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        void noteClicked(const int id);
        void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        void moveNote(Note& note, const int relativeX, const int relativeY);
        
        int levelToY(const int level)
        {
            return level*Y_STEP_HEIGHT+1 + getEditorYStart() - getYScrollInPixels();
        }
        
        virtual wxString getName() const { return _("Keyboard Editor"); }
        
    };
}

#endif
