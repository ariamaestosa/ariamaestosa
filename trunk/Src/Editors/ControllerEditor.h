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

#include <vector>

namespace AriaMaestosa {

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

    void render();
    void render(RelativeXCoord mousex_current, int mousey_current,
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
    void TrackPropertiesDialog(RelativeXCoord dragX_arg, int mousey_current,
                     RelativeXCoord XBeforeDrag_arg, int mousey_initial);

    int getSelectionBegin();
    int getSelectionEnd();

    int getCurrentControllerType();

    int getYScrollInPixels();

    ~ControllerEditor();
};

}

#endif
