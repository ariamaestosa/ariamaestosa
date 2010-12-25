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

#ifndef _ControllerEvent_
#define _ControllerEvent_

#include "Utils.h"

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

namespace AriaMaestosa
{
    
    class GraphicalSequence;
    
    /**
      * @brief represents a single control event
      * @ingroup midi
      */
    class ControllerEvent
    {
        
        int                m_tick;
        unsigned short     m_controller;
        unsigned short     m_value;
        
    public:
        LEAK_CHECK();
        
        ControllerEvent(unsigned short controller, int tick, unsigned short value);
        
        unsigned short getController() const { return m_controller; }
        int            getTick      () const { return m_tick;       }    
        unsigned short getValue     () const { return m_value;      }
        
        
        int getPositionInPixels(GraphicalSequence* gseq);
        
        void setTick(int i);
        void setValue(unsigned short value);
          
        // ---- serialization
        void saveToFile(wxFileOutputStream& fileout);
        bool readFromFile(irr::io::IrrXMLReader* xml);
    };
    
}

#endif
