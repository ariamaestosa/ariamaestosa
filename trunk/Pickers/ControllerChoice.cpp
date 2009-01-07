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

    
static const wxString g_controller_names[] =
{
    wxT(""), // 0
    wxT("Modulation"), // 1, fine 33
    wxT("Breath"), // 2
    wxT(""), // 3
    wxT("Foot"), // 4
    wxT("Portamento Time"), // 5, fine 37
    wxT(""), // 6
    wxT("Volume"), // 7, fine 39
    wxT("Balance"), // 8
    wxT(""), // 9
    wxT("Pan"), // 10, fine 42
    wxT("Expression"), // 11
    wxT("Effect 1"), // 12
    wxT("Effect 2"), // 13
    wxT(""), // 14
    wxT(""), // 15
    wxT("General Purpose 1"), // 16
    wxT("General Purpose 2"), // 17
    wxT("General Purpose 3"), // 18
    wxT("General Purpose 4"), // 19
    wxT(""), // 20
    wxT(""), // 21
    wxT(""), // 22
    wxT(""), // 23
    wxT(""), // 24
    wxT(""), // 25
    wxT(""), // 26
    wxT(""), // 27
    wxT(""), // 28
    wxT(""), // 29
    // these strings aren't actually controllers, but it'll be quicker to re-use empty spots
    // so they can join the same openGL string list, and thus render faster
    wxT("Pitch Bend"), // 30
    wxT("Tempo (global)"), // 31
    wxT(""), // 32
    wxT("On"), // 33
    wxT("Off"), // 34
    wxT("Min"), // 35
    wxT("Max"), // 36
    wxT("Left"), // 37
    wxT("Right"), // 38
    wxT("+2"), // 39
    wxT("-2"), // 40
    wxT(""), // 41
    wxT(""), // 42
    wxT(""), // 43
    wxT(""), // 44
    wxT(""), // 45
    wxT(""), // 46
    wxT(""), // 47
    wxT(""), // 48
    wxT(""), // 49
    wxT(""), // 50
    wxT(""), // 51
    wxT(""), // 52
    wxT(""), // 53
    wxT(""), // 54
    wxT(""), // 55
    wxT(""), // 56
    wxT(""), // 57
    wxT(""), // 58
    wxT(""), // 59
    wxT(""), // 60
    wxT(""), // 61
    wxT(""), // 62
    wxT(""), // 63
    wxT("Sustain"), // 64
    wxT("Portamento"), // 65
    wxT("Sostenuto"), // 66
    wxT("Soft Pedal"), // 67
    wxT("Legato footswitch"), // 68
    wxT("Hold"), // 69
    wxT("Timber Variation"), // 70
    wxT("Timber/Harmonic"), // 71
    wxT("Release Time"), // 72
    wxT("Attack Time"), // 73
    wxT("Brightness"), // 74
    wxT("Decay Time"), // 75
    wxT("Vibrato Rate"), // 76
    wxT("Vibrato Depth"), // 77
    wxT("Vibrato Delay"), // 78
    wxT(""), // 79
    wxT("General Purpose 5"), // 80
    wxT("General Purpose 6"), // 81
    wxT("General Purpose 7"), // 82
    wxT("General Purpose 8"), // 83
    wxT("Portamento Control"), // 84
    wxT(""), // 85
    wxT(""), // 86
    wxT(""), // 87
    wxT(""), // 88
    wxT(""), // 89
    wxT(""), // 90
    wxT("Reverb"), // 91
    wxT("Tremolo"), // 92
    wxT("Chorus"), // 93
    wxT("Celeste"), // 94
    wxT("Phaser"), // 95
    wxT(""), // 96
    wxT(""), // 97
    wxT(""), // 98
    wxT(""), // 99
    wxT(""), // 100
    wxT(""), // 101
    wxT(""), // 102
    wxT(""), // 103
    wxT(""), // 104
    wxT(""), // 105
    wxT(""), // 106
    wxT(""), // 107
    wxT(""), // 108
    wxT(""), // 109
    wxT(""), // 110
    wxT(""), // 111
    wxT(""), // 112
    wxT(""), // 113
    wxT(""), // 114
    wxT(""), // 115
    wxT(""), // 116
    wxT(""), // 117
    wxT(""), // 118
    wxT(""), // 119
    wxT("All sound off"), // 120
    wxT("Reset control"), // 121
    wxT(""), // 122
    wxT("All notes off"), // 123
    wxT(""), // 124
    wxT(""), // 125
    wxT(""), // 126
    wxT(""), // 127
};
    // 32-63 (0x20-0x3F)     LSB for controllers 0-31
    
ControllerChoice::ControllerChoice(GraphicalTrack* parent) : wxMenu(), controller_names_renderer(g_controller_names, 124)
{
    // keep strings translatable
    //I18N: - in controller, when a controller can only be on/off
    controller_names_renderer.get(33) = wxString(_("On"));
    //I18N: - in controller, when a controller can only be on/off
    controller_names_renderer.get(34) = wxString(_("Off"));
    //I18N: - in controller, for a controller with min/max range
    controller_names_renderer.get(35) = wxString(_("Min"));
    //I18N: - in controller, for a controller with min/max range
    controller_names_renderer.get(36) = wxString(_("Max"));
    //I18N: - when setting pan in controller
    controller_names_renderer.get(37) = wxString(_("Left"));
    //I18N: - when setting pan in controller
    controller_names_renderer.get(38) = wxString(_("Right"));
    
    controller_names_renderer.setFont( wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
    
    controllerID=7;

    Append( 7 , g_controller_names[7 ] ); // Volume // fine:39
    Append( 10 , g_controller_names[10 ] ); // Pan // fine:42
    Append( 1 , g_controller_names[1 ] ); // Modulation // fine:33
    Append( 91 , g_controller_names[91 ] ); // Reverb
    Append( 64 , g_controller_names[64 ] ); // Sustain

    // In the midi specs, pitch bend is not a controller. However, i found it just made sense to place it among controllers, so i assigned it arbitrary ID 200.
    Append( 200 , wxT("Pitch Bend") ); // Pitch Bend

    // Tempo bend is not a controller but for now it goes there
    Append( 201 , wxT("Tempo (global)") ); // Tempo (global)

    AppendSeparator();

    Append( 2 , g_controller_names[2 ] ); // Breath
    Append( 4 , g_controller_names[4 ] ); // Foot
    Append( 8 , g_controller_names[8 ] ); // Balance
    Append( 11 , g_controller_names[11 ] ); // Expression
    Append( 92 , g_controller_names[92 ] ); // Tremolo
    Append( 93 , g_controller_names[93 ] ); // Chorus
    Append( 94 , g_controller_names[94 ] ); // Celeste
    Append( 95 , g_controller_names[95 ] ); // Phaser

    
    Append( 70 , g_controller_names[70 ] ); // Timber Variation
    Append( 71 , g_controller_names[71 ] ); // Timber/Harmonic
    Append( 72 , g_controller_names[72 ] ); // Release Time
    Append( 73 , g_controller_names[73 ] ); // Attack Time
    Append( 74 , g_controller_names[74 ] ); // Brightness
    Append( 75 , g_controller_names[75 ] ); // Decay Time
    
    AppendSeparator();
    
    wxMenu* misc_menu = new wxMenu();
    Append(wxID_ANY,wxT("Misc"), misc_menu);

    misc_menu->Append( 66 , g_controller_names[66 ] ); // Sostenuto
    misc_menu->Append( 67 , g_controller_names[67 ] ); // Soft Pedal
    misc_menu->Append( 68 , g_controller_names[68 ] ); // Legato footswitch
    misc_menu->Append( 69 , g_controller_names[69 ] ); // Hold

    misc_menu->AppendSeparator();
    misc_menu->Append( 65 , g_controller_names[65 ] ); // Portamento
    misc_menu->Append( 5 , g_controller_names[5 ] ); // Portamento Time // fine:37
    misc_menu->Append( 84 , g_controller_names[84 ] ); // Portamento Control
    misc_menu->AppendSeparator();
    misc_menu->Append( 76 , g_controller_names[76 ] ); // Vibrato Rate
    misc_menu->Append( 77 , g_controller_names[77 ] ); // Vibrato Depth
    misc_menu->Append( 78 , g_controller_names[78 ] ); // Vibrato Delay

    misc_menu->AppendSeparator();
    misc_menu->Append( 120 , g_controller_names[120 ] ); // All sound off
    misc_menu->Append( 121 , g_controller_names[121 ] ); // Reset control
    misc_menu->Append( 123 , g_controller_names[123 ] ); // All notes off
    misc_menu->AppendSeparator();

    misc_menu->Append( 12 , g_controller_names[12 ] ); // Effect 1
    misc_menu->Append( 13 , g_controller_names[13 ] ); // Effect 2

    misc_menu->Append( 16 , g_controller_names[16 ] ); // General Purpose 1
    misc_menu->Append( 17 , g_controller_names[17 ] ); // General Purpose 2
    misc_menu->Append( 18 , g_controller_names[18 ] ); // General Purpose 3
    misc_menu->Append( 19 , g_controller_names[19 ] ); // General Purpose 4
    misc_menu->Append( 80 , g_controller_names[80 ] ); // General Purpose 5
    misc_menu->Append( 81 , g_controller_names[81 ] ); // General Purpose 6
    misc_menu->Append( 82 , g_controller_names[82 ] ); // General Purpose 7
    misc_menu->Append( 83 , g_controller_names[83 ] ); // General Purpose 8

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

void ControllerChoice::renderControllerName(const int x, const int y)
{
    controller_names_renderer.bind();
    
    // special cases (non-controllers)
    if(controllerID==200)
        controller_names_renderer.get(30).render(x, y);
    else if(controllerID == 201)
        controller_names_renderer.get(31).render(x, y);
    else
    {
        // default (all other controllers)
        wxGLString& string = controller_names_renderer.get(controllerID);
        if(string.getWidth() > 75)
        {
            string.rotate(45); // if string is too long, rotate it so it fits
            string.render(x+5, y);
        }
        else
            string.render(x, y);
    }
    
}


bool ControllerChoice::isOnOffController(const int id) const
{
    if(controllerID== 66 or controllerID== 67 or
       controllerID== 68 or controllerID== 69 or
       controllerID== 64 or controllerID== 65 )
        return true;

    return false;
}


/*
 * Which label should appear at the top of controller editor for current Controller?
 */

void ControllerChoice::renderTopLabel(const int x, const int y)
{
    controller_names_renderer.bind();
    
    // pan
    if(controllerID== 10 or controllerID== 42)
        controller_names_renderer.get(38).render(x, y);
    
    // pitch bend
    else if(controllerID==200)
        controller_names_renderer.get(39).render(x, y);
        
    // on/offs
    else if( isOnOffController(controllerID) )
        controller_names_renderer.get(33).render(x, y);

    // tempo
    else if(controllerID == 201) AriaRender::renderNumber( wxT("400"), x, y );
    
    else
        controller_names_renderer.get(36).render(x, y);
    /*
    // pan
    if(controllerID== 10 or controllerID== 42)
        //I18N: - when setting pan in controller
        return wxString( _("Right") );

    // pitch bend
    if(controllerID==200) return wxT("+2");

    // on/offs
    if( isOnOffController(controllerID) )
        //I18N: - in controller, when a controller can only be on/off
        return wxString( _("On") );

    // tempo
    if(controllerID == 201) return wxT("400");

    //I18N: - in controller, for a controller with min/max range
    return _("Max");
     */
}

/*
 * Which label should appear at the bottom of controller editor for current Controller?
 */

void ControllerChoice::renderBottomLabel(const int x, const int y)
{    
    // pan
    if(controllerID== 10 or controllerID== 42)
        controller_names_renderer.get(37).render(x, y);
    
    // pitch bend
    else if(controllerID==200)
        controller_names_renderer.get(40).render(x, y);
    
    // on/offs
    else if( isOnOffController(controllerID) )
        controller_names_renderer.get(34).render(x, y);
    
    // tempo
    else if(controllerID == 201)  AriaRender::renderNumber( wxT("20"), x, y );
    
    else
        controller_names_renderer.get(35).render(x, y);
    
    /*
    // pan
    if(controllerID== 10 or controllerID== 42)
        //I18N: - when setting pan in controller
        return wxString( _("Left") );

    // pitch bend
    if(controllerID==200) return wxT("-2");

    // on/offs
    if( isOnOffController(controllerID) )
        //I18N: - in controller, when a controller can only be on/off
        return wxString( _("Off") );

    // tempo
    if(controllerID == 201) return wxT("20");

    //I18N: - in controller, for a controller with min/max range
    return _("Min");
     */
}

}
