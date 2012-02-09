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


#ifndef __MAGNETIC_GRID_PICKER_H__
#define __MAGNETIC_GRID_PICKER_H__

#include <wx/menu.h>

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

#include "Utils.h"

namespace AriaMaestosa
{
    
    class GraphicalTrack; // forward
    class MagneticGrid;
    
    /**
      * @ingroup pickers
      * @brief menu where users can choose the size of the magnetic grid and/or the current note length
      *        (depending on the current edit tool)
      */
    class MagneticGridPicker : public wxMenu
    {        
        wxMenuItem* grid1;
        wxMenuItem* grid2;
        wxMenuItem* grid4;
        wxMenuItem* grid8;
        wxMenuItem* grid16;
        wxMenuItem* grid32;
        wxMenuItem* grid64;
        wxMenuItem* grid128;
        wxMenuItem* gridTriplet;
        wxMenuItem* gridDotted;

        GraphicalTrack* m_parent;
        MagneticGrid*   m_model;
        
        void resetChecks();
        
    public:
        LEAK_CHECK();
        
        MagneticGridPicker(GraphicalTrack* parent, MagneticGrid* model);
        ~MagneticGridPicker();
        
        void grid1selected(wxCommandEvent& evt);
        void grid2selected(wxCommandEvent& evt);
        void grid4selected(wxCommandEvent& evt);
        void grid8selected(wxCommandEvent& evt);
        void grid16selected(wxCommandEvent& evt);
        void grid32selected(wxCommandEvent& evt);
        void grid64selected(wxCommandEvent& evt);
        void grid128selected(wxCommandEvent& evt);
        void tripletChanged(wxCommandEvent& evt);
        void dottedChanged(wxCommandEvent& evt);

        void toggleTriplet();
        void toggleDotted();
        
        void syncWithModel();
        MagneticGrid* getModel() { return m_model; }
    };
    
}
#endif
