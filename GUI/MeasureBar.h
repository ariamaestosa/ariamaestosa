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

#ifndef _measurebar_
#define _measurebar_


/*
 *  This class takes care of drawing the measure bar and handles
 *  mouse events on it. It is closely related to MeasureData,
 *  the class managing the actual measure data used to draw the bar.
 */

#include "ptr_vector.h"
#include "Config.h"
#include "Editors/RelativeXCoord.h"
#include "Midi/TimeSigChange.h"
#include "irrXML/irrXML.h"

namespace AriaMaestosa {
	
class MainFrame;
class MeasureData;
	
class UnselectedMenu;
class SelectedMenu;

class MeasureBar
{
    friend class MeasureData;

	int measureBarY; // remember the latest value given by the renderer
	
    int lastMeasureInDrag;
	
	OwnerPtr<UnselectedMenu>  unselectedMenu;
	OwnerPtr<SelectedMenu>  selectedMenu;
	
    MeasureData* data; /* ref */
    
public:
    LEAK_CHECK(MeasureBar);
        
	MeasureBar(MeasureData* parent);
	~MeasureBar();
	
	void render(int from_y);
	void mouseDown(int x, int y);
	void mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
    void mouseUp(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
    void rightClick(int x, int y);
    
	
	int getMeasureBarHeight();
};

}

#endif
