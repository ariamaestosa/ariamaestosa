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

#include "Midi/ControllerEvent.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/KeyboardEditor.h"
#include "IO/IOUtils.h"

namespace AriaMaestosa {
	
ControllerEvent::ControllerEvent(Sequence* sequence, unsigned short controller, int tick, unsigned short value)
{
	
	
    ControllerEvent::controller=controller;
    ControllerEvent::tick=tick;
    ControllerEvent::value=value;
    ControllerEvent::sequence=sequence;
}
    
unsigned short ControllerEvent::getController()
{
    return controller;
}

int ControllerEvent::getTick()
{
    return tick;
}

void ControllerEvent::setTick(int i)
{
	tick = i;
}

unsigned short ControllerEvent::getValue()
{
    return value;
}
void ControllerEvent::setValue(unsigned short value_arg)
{
	value = value_arg;
}

int ControllerEvent::getPositionInPixels()
{
    return (int)(
                 tick*sequence->getZoom() + getEditorsXStart()
                 );
}

void ControllerEvent::saveToFile(wxFileOutputStream& fileout)
{

	writeData( wxT("<controlevent type=\"") + to_wxString(controller), fileout );
	writeData( wxT("\" tick=\"") + to_wxString(tick), fileout );
	writeData( wxT("\" value=\"") + to_wxString(value) + wxT("\"/>\n"), fileout );
	
}

bool ControllerEvent::readFromFile(irr::io::IrrXMLReader* xml)
{
	
	const char* type = xml->getAttributeValue("type");
	if(type!=NULL) controller=atoi(type);
	else
	{
		controller = 0;
		std::cout << "Missing info from file: controller type" << std::endl;
		return false;
	}
	
	const char* tick_c = xml->getAttributeValue("tick");
	if(tick_c!=NULL) tick = atoi(tick_c);
	else
	{
		tick = 0;
		std::cout << "Missing info from file: controller tick" << std::endl;
		return false;
	}
	
	const char* value_c = xml->getAttributeValue("value");
	if(value_c!=NULL) value = atoi(value_c);
	else
	{
		value = 0;
		std::cout << "Missing info from file: controller value" << std::endl;
		return false;
	}
	
	return true;
	
}

}
