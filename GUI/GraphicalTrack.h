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

#include "wx/wx.h"
#include "wx/wfstream.h"

#include "irrXML/irrXML.h"

#include "Config.h"

namespace AriaMaestosa {
	
class GLPane; // forward
class Track;
class MagneticGrid;
class KeyboardEditor;
class ControllerEditor;
class GuitarEditor;
class DrumEditor;
class ScoreEditor;
class RelativeXCoord;
class Editor;

enum {
    KEYBOARD,
    SCORE,
    GUITAR,
    DRUM,
    CONTROLLER
};

class GraphicalTrack
{
    
	DECLARE_LEAK_CHECK();
	
    int height;
    int lastMouseY;
	
public:
        
    
    // ----------- read-only fields -----------
	bool dragging_resize;
	
    int editorMode;
    int from_y;
    int to_y;
    bool collapsed;
    bool muted;
    bool docked;
	
    Sequence* sequence;
    MagneticGrid* grid;
	Track* track;
    KeyboardEditor* keyboardEditor;
    GuitarEditor* guitarEditor;
    DrumEditor* drumEditor;
    ControllerEditor* controllerEditor;
	ScoreEditor* scoreEditor;
    // ----------------------------------------
    
    GraphicalTrack(Track* track, Sequence* parent);
    ~GraphicalTrack();
    
    int getEditorHeight();
    int getTotalHeight();
        
    void renderHeader(const int x, const int y, const bool close, const bool focus=false);
    
    int render(const int y, const int currentTick, bool focus);
    void setCollapsed(const bool collapsed);
    void setHeight(const int height);
    
    void setEditorMode(int mode);
    void createEditors(); // call when you're sure Track, Sequence and GraphicalTrack are set-up properly
    Editor* getCurrentEditor();
    
    bool mouseWheelMoved(int x, int y, int value);
    bool processMouseClick(RelativeXCoord x, int y);
    bool processRightMouseClick(RelativeXCoord x, int y);
    bool processMouseDrag(RelativeXCoord x, int y);
    void processMouseRelease();
    void processMouseExited(RelativeXCoord x_now, int y_now,
							RelativeXCoord x_initial, int y_initial);
	
    // serialization
    void saveToFile(wxFileOutputStream& fileout);
    bool readFromFile(irr::io::IrrXMLReader* xml);

};

}

#endif
