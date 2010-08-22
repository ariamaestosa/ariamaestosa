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
    class GraphicalTrack
    {
        int m_height;
        int m_last_mouse_y;
        
        // widgets
        OwnerPtr< WidgetLayoutManager>  components ;
        BitmapButton* collapseButton;
        BitmapButton* muteButton;
        ToolBar<BlankField>* dockToolBar;
        BlankField* trackName;
        ToolBar<ComboBox>* gridCombo;
        BitmapButton* scoreButton;
        BitmapButton* pianoButton;
        BitmapButton* tabButton;
        BitmapButton* drumButton;
        BitmapButton* ctrlButton;
        ToolBar<BlankField>* sharpFlatPicker;
        BlankField* instrumentName;
        BlankField* channelButton;
        
        /** Whether this track was collapsed with the triangle widget so that only its header is seen */
        bool m_collapsed;
        
        /** Whether this track was "docked", i.e. minimized so that not even its header is visible */
        bool m_docked;
        
        Sequence* m_sequence;
        Track*    m_track;
        
        EditorType m_editor_mode;

    public:
        LEAK_CHECK();
        
        // ----------- read-only fields -----------
        bool m_dragging_resize;
        
        
        /** Y coord on the current display where this track starts (this is updated on each rendering).
          * Worth -1 when track is docked.
          */
        int m_from_y;
        
        /** Y coord on the current display where this track ends (this is updated on each rendering).
          * Worth -1 when track is docked.
          */
        int m_to_y;
        

        OwnerPtr<MagneticGrid>  m_grid ;
        
        // editors
        OwnerPtr<KeyboardEditor>    keyboardEditor   ;
        OwnerPtr<GuitarEditor>      guitarEditor     ;
        OwnerPtr<DrumEditor>        drumEditor       ;
        OwnerPtr<ControllerEditor>  controllerEditor ;
        OwnerPtr<ScoreEditor>       scoreEditor      ;
        ptr_vector<Editor, REF>     m_all_editors    ;
        
        AriaRenderString m_instrument_name;

        // ----------------------------------------
        
        GraphicalTrack(Track* track, Sequence* parent);
        ~GraphicalTrack();
        
        int getEditorHeight();
        int getTotalHeight();
        
        void renderHeader(const int x, const int y, const bool close, const bool focus=false);
        
        int render(const int y, const int currentTick, bool focus);
        void setCollapsed(const bool collapsed);
        void setHeight(const int height);
        void maximizeHeight(bool maximize=true);
        
        void setEditorMode(EditorType mode);
        
        bool isCollapsed() const { return m_collapsed; }
        
        bool isDocked   () const { return m_docked;    }
        
        Track* getTrack()        { return m_track;     }
        
        Sequence* getSequence()  { return m_sequence;  }
        
        EditorType getEditorMode() const { return m_editor_mode; }
        
        /**
          * @pre Track, Sequence and GraphicalTrack must be initialized properly upon calling this
          */
        void createEditors();
        
        Editor* getCurrentEditor();
        
        void dock(const bool dock=true);
        
        bool mouseWheelMoved(int x, int y, int value);
        
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
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
        
    };
    
}

#endif
