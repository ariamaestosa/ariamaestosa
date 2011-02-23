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
#include <wx/intl.h>

namespace AriaMaestosa
{
    
    class Track; // forward
    class TuningPicker;

    
    /**
      * @brief a note editor using tablature notation
      * @ingroup editors
      */
    class GuitarEditor : public Editor
    {
        
    public:
        
        GuitarEditor(GraphicalTrack* track);
        ~GuitarEditor();
        
        /** event callback from base class */
        virtual void mouseDown(RelativeXCoord, int y);
        
         virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        /** implemented from base class Editor's required interface */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        
        /** implemented from base class Editor's required interface */
        virtual void noteClicked(const int id);
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        
        /** implemented from base class Editor's required interface */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);
        
        /** implemented from base class Editor's required interface */
        virtual void moveNote(Note& note, const int relativeX, const int relativeY);
        
        /** implemented from base class Editor's required interface */
        virtual wxString getName() const { return _("Tablature Editor"); }
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snappedX, const int mouseY)
        {
            assert(false); // not supported in this editor
        }
        
        /** implemented from base class Editor's required interface */
        virtual int getYScrollInPixels()
        {
            // no scrolling in this editor
            return 0;
        }
        
        virtual NotationType getNotationType() const { return GUITAR; }
        
        virtual void processKeyPress(int keycode, bool commandDown, bool shiftDown);
    };
    
}

#endif
