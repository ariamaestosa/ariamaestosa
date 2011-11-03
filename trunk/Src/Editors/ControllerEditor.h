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


#ifndef __CONTROLLER_EDITOR_H__
#define __CONTROLLER_EDITOR_H__

#include "Editors/Editor.h"
#include "Pickers/ControllerChoice.h"

#include <wx/intl.h>

namespace AriaMaestosa
{
    
    class Track; // forward
    
    /**
     * @brief editor for MIDI controllers (effects)
     *
     * Since this Editor is very different from others, it does not use much functionnality of the base class
     * editor. It instead overrides all event methods. (FIXME, unclean)
     * @ingroup editors
     */
    class ControllerEditor : public Editor
    {
        
        bool m_has_been_resizing;  //!< true if user has been resizing track anytime since last mouse down
        bool m_mouse_is_in_editor; //!< true if mouse drag is within track bounds. false if is on header on scrollbar, etc.
        
        bool m_selecting;
        
        OwnerPtr<ControllerChoice>  m_controller_choice;
        
        int m_selection_begin, m_selection_end;
        
        int m_mouse_y;
        
        float getYZoom() const
        {
            const int area_from_y = getEditorYStart() + 7;
            const int area_to_y   = getYEnd() - 15;
            return (float)( area_to_y - area_from_y ) / 127.0;
        }
        
        int getAreaYFrom() const
        {
            return getEditorYStart() + 7;
        }
        int getAreaYTo() const
        {
            return getYEnd() - 15;
        }
        
        float mouseYToValue(int mouseY) const
        {
            const float y_zoom = getYZoom();
            const bool on_off = m_controller_choice->isOnOffController( m_controller_choice->getControllerID() );
            const int area_from_y = getAreaYFrom();
            float y_value = (float)(mouseY - area_from_y)/y_zoom;
            if ( on_off )
            {
                // on/off controllers should only use values 0 and 127
                if  (y_value < 64.0f) y_value = 0.0f;
                else                  y_value = 127.0f;
            }
            return y_value;
        }
        
        /** For instrument change controller */
        AriaRenderString m_instrument_name;

    public:
        
        ControllerEditor(GraphicalTrack* track);
        ~ControllerEditor();
        
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);
        
        void renderEvents();
        
        void selectAll( bool selected );
                
        void mouseDown(RelativeXCoord x, const int y);
        void mouseDrag(RelativeXCoord mousex_current, const int mousey_current,
                       RelativeXCoord, const int mousey_initial);
        void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial);
        void rightClick(RelativeXCoord, const int y);
        void mouseExited(RelativeXCoord dragX_arg, int mousey_current,
                         RelativeXCoord XBeforeDrag_arg, int mousey_initial);
        virtual void processMouseMove(RelativeXCoord x, int y);
        virtual void processMouseOutsideOfMe();

        int getSelectionBegin() const { return m_selection_begin; }
        int getSelectionEnd  () const { return m_selection_end;   }
        
        void addPreciseEvent(int tick, wxFloat64 value);
        
        int getCurrentControllerType() const
        {
            return m_controller_choice->getControllerID();
        }
        void setController(int id)
        {
            m_controller_choice->setControllerID(id);
        }
        
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
        
        static int getPositionInPixels(int tick, GraphicalSequence* gseq);
        
        virtual NotationType getNotationType() const { return CONTROLLER; }
    };
    
}

#endif
