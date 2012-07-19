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

#ifndef __MEASURE_BAR_H__
#define __MEASURE_BAR_H__

#include "Utils.h"

namespace AriaMaestosa
{
    class MeasureData;
    class GraphicalSequence;
    
    class UnselectedMenu;
    class SelectedMenu;
    
    /**
      * @brief takes care of drawing the measure bar and handles mouse events on it.
      *
      * It is closely related to MeasureData, the class managing the actual measure data used
      * to draw the bar.
      *
      * @ingroup gui
      */    
    class MeasureBar
    {
        friend class MeasureData;
        
        int m_measure_bar_y; //!< remember the latest value given by the renderer
        
        int m_last_measure_in_drag;
        
        OwnerPtr<UnselectedMenu> m_unselected_menu;
        OwnerPtr<SelectedMenu>   m_selected_menu;
        
        MeasureData* m_data;
        
        GraphicalSequence* m_gseq;
        
        
        void getFirstAndLastSelectedMeasure(int* first, int* last);
        
    public:
        LEAK_CHECK();
        
        MeasureBar(MeasureData* parent, GraphicalSequence* gseq);
        ~MeasureBar();
        
        void render(int from_y);
        void mouseDown(int x, int y, bool shiftPressed, bool altPressed);
        void mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
        void mouseUp(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
        void rightClick(int x, int y);
        
        float measureLengthInPixels(int measure=-1);
        int   measureAtPixel(int pixel);

        int getMeasureBarHeight();
        float defaultMeasureLengthInPixels();
        
        void  selectTimeSig(const int id);
        void  unselect();
        
        int   getTotalPixelAmount();

        int   measureDivisionAt(int pixel);
        int   firstPixelInMeasure(int id);
        int   lastPixelInMeasure(int id);
    };
    
}

#endif
