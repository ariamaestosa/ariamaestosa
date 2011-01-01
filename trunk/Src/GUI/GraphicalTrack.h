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

#ifndef _graphical_track_
#define _graphical_track_


#include "Utils.h"
#include "Editors/Editor.h"
#include "Midi/Track.h"

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

namespace AriaMaestosa
{
    
    class Track;
    class MagneticGrid;
    class KeyboardEditor;
    class ControllerEditor;
    class GuitarEditor;
    class DrumEditor;
    class ScoreEditor;
    class RelativeXCoord;
    class GraphicalSequence;
        
    // lightweight components
    class BlankField;
    class ComboBox;
    class BitmapButton;
    class WidgetLayoutManager;
    class BlankField;
    template<typename PARENT> class ToolBar;
    
    /**
      * @brief The graphical part of a track (the data being held in Track)
      * @ingroup gui
      */
    class GraphicalTrack : public ITrackListener
    {
        int m_height;
        int m_last_mouse_y;
        
        // widgets
        OwnerPtr< WidgetLayoutManager>  m_components;
        BitmapButton*                   m_collapse_button;
        BitmapButton*                   m_mute_button;
        ToolBar<BlankField>*            m_dock_toolbar;
        BlankField*                     m_track_name;
        ToolBar<ComboBox>*              m_grid_combo;
        BitmapButton*                   m_score_button;
        BitmapButton*                   m_piano_button;
        BitmapButton*                   m_tab_button;
        BitmapButton*                   m_drum_button;
        BitmapButton*                   m_ctrl_button;
        ToolBar<BlankField>*            m_sharp_flat_picker;
        BlankField*                     m_instrument_field;
        BlankField*                     m_channel_field;
        
        /** Whether this track was collapsed with the triangle widget so that only its header is seen */
        bool m_collapsed;
        
        /** Whether this track was "docked", i.e. minimized so that not even its header is visible */
        bool m_docked;
        
        GraphicalSequence* m_gsequence;
        Track* m_track;

        // editors
        OwnerPtr<KeyboardEditor>    m_keyboard_editor  ;
        OwnerPtr<GuitarEditor>      m_guitar_editor    ;
        OwnerPtr<DrumEditor>        m_drum_editor      ;
        OwnerPtr<ControllerEditor>  m_controller_editor;
        OwnerPtr<ScoreEditor>       m_score_editor     ;
        
        bool m_dragging_resize;
        
        
        /** Y coord on the current display where this track starts (this is updated on each rendering).
         * Worth -1 when track is docked.
         */
        int m_from_y;
        
        /** Y coord on the current display where this track ends (this is updated on each rendering).
         * Worth -1 when track is docked.
         */
        int m_to_y;
        
        OwnerPtr<MagneticGrid>  m_grid;

        ptr_vector<Editor, REF> m_all_editors;
        
        AriaRenderString        m_instrument_name;
        
    public:
        LEAK_CHECK();
        
        GraphicalTrack(Track* track, GraphicalSequence* parent);
        ~GraphicalTrack();
        
        int getEditorHeight();
        int getTotalHeight();
        
        void renderHeader(const int x, const int y, const bool close, const bool focus=false);
        
        int render(const int y, const int currentTick, bool focus);
        void setCollapsed(const bool collapsed);
        void setHeight(const int height);
        void maximizeHeight(bool maximize=true);
                
        bool isCollapsed() const { return m_collapsed; }
        bool isDocked   () const { return m_docked;    }
        
        Track* getTrack()                { return m_track;     }
        const Track* getTrack() const     { return m_track;     }
        GraphicalSequence* getSequence()  { return m_gsequence;  }
                
        /**
          * @pre Track, Sequence and GraphicalTrack must be initialized properly upon calling this
          */
        void createEditors();
        
        Editor* getCurrentEditor();
        
        // editors
              KeyboardEditor* getKeyboardEditor()       { return m_keyboard_editor;   }
        const KeyboardEditor* getKeyboardEditor() const { return m_keyboard_editor;   }
              GuitarEditor*   getGuitarEditor  ()       { return m_guitar_editor;     }
        const GuitarEditor*   getGuitarEditor  () const { return m_guitar_editor;     }
              DrumEditor*     getDrumEditor    ()       { return m_drum_editor;       }
        const DrumEditor*     getDrumEditor    () const { return m_drum_editor;       }
              ScoreEditor*    getScoreEditor   ()       { return m_score_editor;      }
        const ScoreEditor*    getScoreEditor   () const { return m_score_editor;      }

        ControllerEditor*   getControllerEditor() { return m_controller_editor; }

        void dock(const bool dock=true);
        
        bool mouseWheelMoved(int x, int y, int value);
        
        bool isDragResize() const { return m_dragging_resize; }
        
        /**
         * Callback for mouse down events. The click may or may not be within this particular track; it's partly
         * the job of this method to determine whether the click belongs to itself.
         * 
         * @param x The x coordinate where the mouse click occurred
         * @param y The y coordinate where the mouse click occurred
         *
         * @return true: the event does not belong to this track and so the program should continue searching to
         *         whom the event belongs. false: the event belongs to this track and was processed
         */
        bool processMouseClick(RelativeXCoord x, int y);
        
        bool processRightMouseClick(RelativeXCoord x, int y);
        bool processMouseDrag(RelativeXCoord x, int y);
        void processMouseRelease();
        void processMouseExited(RelativeXCoord x_now, int y_now,
                                RelativeXCoord x_initial, int y_initial);
        
        void onInstrumentChange(const int newInstrument, const bool isDrumKit);

        /** Called when a track's key changes */
        void onKeyChange(const int symbolAmount, const KeyType symbol);
        
        int getNoteStartInPixels(const int id) const;
        int getNoteEndInPixels(const int id) const;
        
        int getGridDivider() const;
        
        /** @brief Implement callback from ITrackListener */
        virtual void onNotationTypeChange();
        
        void selectNote(const int id, const bool selected, bool ignoreModifiers=false);

        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
        
    };
    
}

#endif
