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


#ifndef __GUITAR_EDITOR_H__
#define __GUITAR_EDITOR_H__

#include "Editors/Editor.h"

#include <vector>

#include "Editors/RelativeXCoord.h"
#include "wx/intl.h"

namespace AriaMaestosa
{
    
    class Track; // forward
    class TuningPicker;
    
    class GuitarEditor : public Editor
    {
        
    public:
        
        std::vector<int> tuning;
        std::vector<int> previous_tuning; // for undo purposes
        
        GuitarEditor(Track* track);
        ~GuitarEditor();
        
        void render();
        void render(RelativeXCoord mousex_current, int mousey_current,
                    RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void tuningUpdated(const bool user_triggered=true); // if user-triggered, it will be undoable
        void updatePosition(const int from_y, const int to_y, const int width, const int height, const int barHeight);
        
        void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);
        
        void mouseDown(RelativeXCoord, int y);
        
        NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        void noteClicked(const int id);
        void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        void moveNote(Note& note, const int relativeX, const int relativeY);
        
        virtual wxString getName() const { return _("Tablature Editor"); }
    };
    
}

#endif
