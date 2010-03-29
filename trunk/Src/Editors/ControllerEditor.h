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


#ifndef _controllereditor_
#define _controllereditor_

#include "Editors/Editor.h"

#include "wx/intl.h"

namespace AriaMaestosa
{
    
    class Track; // forward
    class ControllerChoice;
    
    /*
     * Since this Editor is very different from others, it does not use much functionnality of the base class editor
     * It instead overrides all event methods.
     */
    
    class ControllerEditor : public Editor
    {
        
        bool hasBeenResizing; // true if user has been resizing track anytime since last mouse down
        bool mouse_is_in_editor; // true if mouse drag is within track bounds. false if is on header on scrollbar, etc.
        
        bool selecting;
        
        OwnerPtr<ControllerChoice>  controllerChoice;
        
        int selection_begin, selection_end;
    public:
        
        ControllerEditor(Track* track);
        ~ControllerEditor();
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void renderEvents();
        
        void selectAll( bool selected );
        
        void updatePosition(const int from_y, const int to_y, const int width, const int height, const int barHeight);
        
        void mouseDown(RelativeXCoord x, const int y);
        void mouseDrag(RelativeXCoord mousex_current, const int mousey_current,
                       RelativeXCoord, const int mousey_initial);
        void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial);
        void rightClick(RelativeXCoord, const int y);
        void mouseExited(RelativeXCoord dragX_arg, int mousey_current,
                         RelativeXCoord XBeforeDrag_arg, int mousey_initial);
        
        int getSelectionBegin();
        int getSelectionEnd();
        
        int getCurrentControllerType();
        
        /** implemented from base class Editor's required interface */
        virtual int getYScrollInPixels()
        {
            // no scrolling in this editor
            return 0;
        }        
        
        /** implemented from base class Editor's required interface */
        virtual void addNote(const int snappedX, const int mouseY)
        {
            assert(false); // not supported in this editor
        }
        
        /** to notify the editor that note 'id' was just clicked. */
        virtual void noteClicked(const int id)
        {
            assert(false); // not supported in this editor
        }
        
        /** called when user adds a note */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
        {
            assert(false); // not supported in this editor
        }
        
        /** called when user completes a selection rectangle */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial)
        {
            assert(false); // not supported in this editor
        }
        
        /** called when user completes a selection rectangle */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID)
        {
            assert(false); // not supported in this editor
            return FOUND_NOTHING;
        }
        
        /** called when user completes a selection rectangle */
        virtual void moveNote(Note& note, const int relativeX, const int relativeY)
        {
            assert(false); // not supported in this editor
        }

        virtual wxString getName() const { return _("Controller Editor"); }
    };
    
}

#endif
