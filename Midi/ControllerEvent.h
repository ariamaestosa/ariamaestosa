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

#include "wx/wfstream.h"

#include "Config.h"
#include "irrXML/irrXML.h"

namespace AriaMaestosa {

class Sequence;

class ControllerEvent {

    int tick;
    unsigned short controller, value;
    Sequence* sequence;

public:
    LEAK_CHECK(ControllerEvent);

    ControllerEvent(Sequence* sequence, unsigned short controller, int tick, unsigned short value);

    unsigned short getController();
    int getPositionInPixels();
    int getTick();
    void setTick(int i);
    unsigned short getValue();
    void setValue(unsigned short value);

    // serialization
    void saveToFile(wxFileOutputStream& fileout);
    bool readFromFile(irr::io::IrrXMLReader* xml);

};

}

#endif
