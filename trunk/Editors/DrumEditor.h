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


#ifndef _drumeditor_
#define _drumeditor_

#include "Editors/Editor.h"
#include "Renderers/RenderAPI.h"
#include <vector>

namespace AriaMaestosa {

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
    bool strings_consolidated;
    
public:
    // ------------------ read-only -------------------
    std::vector<DrumInfo> drums; // says in which order the drums are drawn, how they're divided in sections, etc.
    int midiKeyToVectorID[128]; // says where each midiKey is located into the vector.

    // ------------------------------------------------

    DrumEditor(Track* track);
    ~DrumEditor();

    void useCustomDrumSet();
    void useDefaultDrumSet();

    void render();
    void render(RelativeXCoord mousex_current, int mousey_current,
                RelativeXCoord mousex_initial, int mousey_initial, bool focus=false);

    void updatePosition(const int from_y, const int to_y, const int width, const int height, const int barHeight);

    void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);

    //void TrackPropertiesDialog(RelativeXCoord dragX_arg, int mousey_current,
    //                 RelativeXCoord XBeforeDrag_arg, int mousey_initial);

    //void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
    //                   RelativeXCoord mousex_initial, int mousey_initial);
    void mouseDown(RelativeXCoord x, int y);
    //void mouseDrag(RelativeXCoord mousex_current, int mousey_current,
    //               RelativeXCoord mousex_initial, int mousey_initial);
    void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                 RelativeXCoord mousex_initial, int mousey_initial);
    //void rightClick(RelativeXCoord x, int y);

    NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
    void noteClicked(const int id);
    void addNote(const int snappedX, const int mouseY);

    int getDrumAtY(const int given_y);
    int getYForDrum(const int given_drumID);

    void moveNote(Note& note, const int relativeX, const int relativeY);

    int getYScrollInPixels();
};

}

#endif
