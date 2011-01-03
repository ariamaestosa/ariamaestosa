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


#ifndef __MAGNETIC_GRID_H__
#define __MAGNETIC_GRID_H__

#include "Utils.h"

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }


namespace AriaMaestosa
{
        
    /**
     * @ingroup midi
     */
    class MagneticGrid
    {
        bool m_triplet;
        
        wxString m_label;
        int m_divider;
        
        void updateLabel();
        
    public:
        LEAK_CHECK();
        
        MagneticGrid();
        virtual ~MagneticGrid();
        
        void setTriplet(bool isTriplet);
        bool isTriplet() const { return m_triplet; }
        
        int  getDivider() const { return m_divider; }
        void setDivider(const int newVal);
        
        // serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}
#endif
