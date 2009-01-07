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


#ifndef _controllerchoice_
#define _controllerchoice_

#include "wx/wx.h"
#include "wx/wfstream.h"

#include "irrXML/irrXML.h"
#include "Renderers/RenderAPI.h"
#include "Config.h"

namespace AriaMaestosa {

class GraphicalTrack; // forward


class ControllerChoice : public wxMenu
{
    GraphicalTrack* parent;
    int controllerID;

    AriaRenderArray controller_names_renderer;
    
public:
    LEAK_CHECK(ControllerChoice);

    ControllerChoice(GraphicalTrack* parent);
    ~ControllerChoice();

    int getControllerID();
    void setControllerID(int id);
    void renderControllerName(const int x, const int y);
    bool isOnOffController(const int id) const;

    void renderTopLabel(const int x, const int y);
    void renderBottomLabel(const int x, const int y);

    void menuSelected(wxCommandEvent& evt);


    DECLARE_EVENT_TABLE();
};

}
#endif
