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


#include "Pickers/ControllerChoice.h"

#include "Pickers/ControllerChoice.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include <iostream>

#include "Config.h"
#include "AriaCore.h"

namespace AriaMaestosa {
    

BEGIN_EVENT_TABLE(ControllerChoice, wxMenu)

EVT_MENU_RANGE(0,202,ControllerChoice::menuSelected)

END_EVENT_TABLE()


ControllerChoice::ControllerChoice(GraphicalTrack* parent) : wxMenu()
{
    INIT_LEAK_CHECK();
	
    controllerID=7;
    
    Append( 7 ,wxT("Volume")); // fine:39
    Append( 10 ,wxT("Pan")); // fine:42
    Append( 1 ,wxT("Modulation")); // fine:33
    Append( 91 ,wxT("Reverb"));
    Append( 64 ,wxT("Sustain"));
    
    // In the midi specs, pitch bend is not a controller. However, i found it just made sense to place it among controllers, so i assigned it arbitrary ID 200.
    Append( 200 ,wxT("Pitch Bend"));

    // Tempo bend is not a controller but for now it goes there
    Append( 201 ,wxT("Tempo"));
    
    AppendSeparator();
    
    Append( 2 ,wxT("Breath"));
    Append( 4 ,wxT("Foot"));
    Append( 8 ,wxT("Balance"));
    Append( 11 ,wxT("Expression"));
    Append( 92 ,wxT("Tremolo"));
    Append( 93 ,wxT("Chorus"));
    Append( 94 ,wxT("Celeste"));
    Append( 95 ,wxT("Phaser"));
    
    Append( 70 ,wxT("Timber Variation"));
    Append( 71 ,wxT("Timber/Harmonic"));
    Append( 72 ,wxT("Release Time"));
    Append( 73 ,wxT("Attack Time"));
    Append( 74 ,wxT("Brightness"));
    Append( 75 ,wxT("Decay Time"));

    
    Append( 66 ,wxT("Sostenuto"));
    Append( 67 ,wxT("Soft Pedal"));
    Append( 68 ,wxT("Legato footswitch"));
    Append( 69 ,wxT("Hold"));
    
    AppendSeparator();
    Append( 65 ,wxT("Portamento"));
    Append( 5 ,wxT("Portamento Time")); // fine:37
    Append( 84 ,wxT("Portamento Control"));
    AppendSeparator();
    Append( 76 ,wxT("Vibrato Rate"));
    Append( 77 ,wxT("Vibrato Depth"));
    Append( 78 ,wxT("Vibrato Delay"));

    AppendSeparator();
    Append( 12 ,wxT("Effect 1"));
    Append( 13 ,wxT("Effect 2"));
    
    Append( 16 ,wxT("General Purpose 1"));
    Append( 17 ,wxT("General Purpose 2"));
    Append( 18 ,wxT("General Purpose 3"));
    Append( 19 ,wxT("General Purpose 4"));
    Append( 80 ,wxT("General Purpose 5"));
    Append( 81 ,wxT("General Purpose 6"));
    Append( 82 ,wxT("General Purpose 7"));
    Append( 83 ,wxT("General Purpose 8"));
    
    ControllerChoice::parent = parent;
}

ControllerChoice::~ControllerChoice()
{
}

void ControllerChoice::menuSelected(wxCommandEvent& evt)
{
    
    controllerID=evt.GetId();

    assertExpr(controllerID,<,205);
    assertExpr(controllerID,>=,0);
    
    Display::render();
}

void ControllerChoice::setControllerID(int id)
{
    controllerID = id;
}

int ControllerChoice::getControllerID()
{
    return controllerID;   
}

char* ControllerChoice::getControllerName()
{
    if(controllerID== 10 or controllerID== 42) return "Pan"; // fine:42
    else if(controllerID== 7 or controllerID== 39) return "Volume"; // fine:39
    else if(controllerID== 1 or controllerID==33) return "Modulation"; // fine:33
    else if(controllerID== 91 ) return "Reverb";
    else if(controllerID== 64 ) return "Sustain";
    
    else if(controllerID== 200 ) return "Pitch Bend";
    else if(controllerID== 201 ) return "Tempo";
    
    else if(controllerID== 2 ) return "Breath";
    else if(controllerID== 4 ) return "Foot";
    else if(controllerID== 8 ) return "Balance";
    else if(controllerID== 11 ) return "Expression";
	
    else if(controllerID== 70 ) return "Timber Variation";
    else if(controllerID== 71 ) return "Timber/ Harmonic";
    else if(controllerID== 72 ) return "Release Time";
    else if(controllerID== 73 ) return "Attack Time";
    else if(controllerID== 74 ) return "Brightness";
    else if(controllerID== 75 ) return "Decay Time";
    
    else if(controllerID== 92 ) return "Tremolo";
    else if(controllerID== 93 ) return "Chorus";
    else if(controllerID== 94 ) return "Celeste";
    else if(controllerID== 95 ) return "Phaser";
    
    else if(controllerID== 66 ) return "Sostenuto";
    else if(controllerID== 67 ) return "Soft Pedal";
    else if(controllerID== 68 ) return "Legato footswitch";
    else if(controllerID== 69 ) return "Hold";

    else if(controllerID== 65 ) return "Portamento";
    else if(controllerID== 5 or controllerID== 37 ) return "Portamento Time"; // fine:37
    else if(controllerID== 84 ) return "Portamento Control"; // !

    else if(controllerID== 76 ) return "Vibrato Rate";
    else if(controllerID== 77 ) return "Vibrato Depth";
    else if(controllerID== 78 ) return "Vibrato Delay";

    else if(controllerID== 12 ) return "Effect 1";
    else if(controllerID== 13 ) return "Effect 2";
    
    
    else if(controllerID== 16 ) return "General Purpose 1";
    else if(controllerID== 17 ) return "General Purpose 2";
    else if(controllerID== 18 ) return "General Purpose 3";
    else if(controllerID== 19 ) return "General Purpose 4";
    else if(controllerID== 80 ) return "General Purpose 5";
    else if(controllerID== 81 ) return "General Purpose 6";
    else if(controllerID== 82 ) return "General Purpose 7";
    else if(controllerID== 83 ) return "General Purpose 8";
    
    // 32-63 (0x20-0x3F) 	LSB for controllers 0-31
    
    else
    {
        std::cout << "wrong controller ID: " << controllerID << std::endl;
        return (char*) _("Wrong controller ID");
    }
}

/*
 * Which label should appear at the top of controller editor for current Controller?
 */

wxString ControllerChoice::getTopLabel()
{
	
	// pan
	if(controllerID== 10 or controllerID== 42)
		return wxString( _("Right") );
	
	// pitch bend
	if(controllerID==200) return wxT("+2");
	
	// on/offs
	if(controllerID== 66 or controllerID== 67 or
	   controllerID== 68 or controllerID== 69 or
	   controllerID== 64 or controllerID== 65 )
		return wxString( _("On") );
	
	// tempo
	if(controllerID == 201) return wxT("400");
	
	return wxT("Max");
}

/*
 * Which label should appear at the bottom of controller editor for current Controller?
 */

wxString ControllerChoice::getBottomLabel()
{
	// pan
	if(controllerID== 10 or controllerID== 42)
		return wxString( _("Left") );

	// pitch bend
	if(controllerID==200) return wxT("-2");
	
	// on/offs
	if(controllerID== 66 or controllerID== 67 or
	   controllerID== 68 or controllerID== 69 or
	   controllerID== 64 or controllerID== 65 )
		return wxString( _("Off") );
	
	// tempo
	if(controllerID == 201) return wxT("20");
	
	
	return wxT("Min");
}

}
