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


#ifndef __KEYBOARD_EDITOR_H__
#define __KEYBOARD_EDITOR_H__

#include <wx/intl.h>

#include "Editors/Editor.h"
#include "Editors/RelativeXCoord.h"
#include "Utils.h"

namespace AriaMaestosa
{
    
    class Track; // forward
    class Sequence;
    class GraphicalTrack;
    
    class KeyboardEditor : public Editor
    {
        AriaRenderArray m_sharp_notes_names;
        AriaRenderArray m_flat_notes_names;
        
    public:
        KeyboardEditor(GraphicalTrack* data);
        virtual ~KeyboardEditor();
        
        /** event callback from base class */
        virtual void mouseDown(RelativeXCoord x, const int y);
        virtual void processMouseMove(RelativeXCoord x, int y);
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);


        int levelToLocalY(const int level)
        {
            return level*m_y_step+1;
        }
        
        int levelToY(const int level)
        {
            return level*m_y_step+1 + getEditorYStart() - getYScrollInPixels();
        }
        
        int levelsInView() const
        {
            return (getYEnd() - getEditorYStart()) / m_y_step;
        }
        
        /** implemented from base class Editor's required interface */
        virtual int getYScrollInPixels();
        
        void setYScrollInPixels(int y);
        
        /** implemented from base class Editor's required interface */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        
        /** implemented from base class Editor's required interface */
        virtual void moveNote(Note& note, const int relativeX, const int relativeY);
        
        
        /** implemented from base class Editor's required interface */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial);
        
        /** implemented from base class Editor's required interface */
        virtual wxString getName() const { return _("Keyboard Editor"); }
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snappedX, const int mouseY)
        {
            assert(false); // not supported in this editor
        }
        
        virtual NotationType getNotationType() const { return KEYBOARD; }
        
        virtual void processKeyPress(int keycode, bool commandDown, bool shiftDown);
        
        /** last action when mouse button is up */
        virtual bool performClickedOnNoteFinalAction(RelativeXCoord mousex_current, int mousey_current,
                                                    RelativeXCoord mousex_initial, int mousey_initial);
                                                    
        virtual void updateMovingCursor();
        
        void scrollNotesIntoView();
        
    private:
        

        AriaColor m_white_color;
        AriaColor m_black_color;
        AriaColor m_gray_color;
        bool m_resizing_mode;
        
        int m_octave_height;
    
        wxString getNoteName(int pitchID, bool addOctave = true);
        void applyColor(AriaColor color);
        void applyInvertedColor(AriaColor color);
        float changeComponent(float component, float factor);
        void drawNoteTrack(int x, int y, bool focus);
        void drawMovedNote(int noteId, int x_step_move, int y_step_move, 
                                const AriaColor& ariaColor, bool showNoteNames);
        void drawResizedNote(int noteId, int x_step_resize, 
                                const AriaColor& ariaColor, bool showNoteNames);
        void checkCursor(RelativeXCoord x, int y);
        NoteSearchResult flownOverNoteAt(RelativeXCoord x, const int y, int& noteID);
        
    };

}

#endif
