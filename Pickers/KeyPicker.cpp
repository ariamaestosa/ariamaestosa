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

#include "Midi/Track.h"
#include "Pickers/KeyPicker.h"
#include "Editors/ScoreEditor.h"
#include "Editors/KeyboardEditor.h"
#include "GUI/GraphicalTrack.h"
#include "AriaCore.h"

namespace AriaMaestosa {
	
BEGIN_EVENT_TABLE(KeyPicker, wxMenu)
EVT_MENU_RANGE(1,18,KeyPicker::menuItemSelected)
END_EVENT_TABLE()

KeyPicker::KeyPicker() : wxMenu()
{
	INIT_LEAK_CHECK();
    
    musical_checkbox = AppendCheckItem(1,_("Musical notation")); musical_checkbox->Check(true);
    linear_checkbox = AppendCheckItem(2,_("Linear Notation")); linear_checkbox->Check(true);
	AppendSeparator();
    Append(3,wxT("C, Am"));
	AppendSeparator();
	Append(4,wxT("G, Em")); // # 1
	Append(5,wxT("D, Bm")); // # 2
	Append(6,wxT("A, F#m")); // # 3
	Append(7,wxT("E, C#m")); // # 4
	Append(8,wxT("B, G#m")); // # 5
	Append(9,wxT("F#, D#m")); // # 6
	Append(10,wxT("C#, A#m")); // # 7
	AppendSeparator();
	Append(11,wxT("F, Dm")); // b 1
	Append(12,wxT("Bb, Gm")); // b 2
	Append(13,wxT("Eb, Cm")); // b 3
	Append(14,wxT("Ab, Fm")); // b 4
    Append(15,wxT("Db, Bbm")); // b 5
	Append(16,wxT("Gb, Ebm")); // b 6 
	Append(17,wxT("Cb, Abm")); // b 7
}

void KeyPicker::setParent(Track* parent_arg)
{
	parent = parent_arg->graphics;
    if(parent->editorMode == KEYBOARD)
    {
        musical_checkbox->Enable(false);
        linear_checkbox->Enable(false);
    }
    else if(parent->editorMode == SCORE)
    {
        musical_checkbox->Enable(true);
        linear_checkbox->Enable(true);
    }
}

KeyPicker::~KeyPicker()
{  
}

void KeyPicker::setChecks( bool musicalNotationEnabled, bool linearNotationEnabled)
{
	musical_checkbox->Check(musicalNotationEnabled);
	linear_checkbox->Check(linearNotationEnabled);
}

void KeyPicker::menuItemSelected(wxCommandEvent& evt)
{
	const int id = evt.GetId();
	
	if( id < 0 or id > 17 ) return;

	if( id == 1 ) parent -> scoreEditor -> enableMusicalNotation( musical_checkbox->IsChecked() );
	else if( id == 2 ) parent -> scoreEditor -> enableLinearNotation( linear_checkbox->IsChecked() );
	else if( id == 3 )
    {
        parent -> scoreEditor -> loadKey(NATURAL, 0);
    }
	else if( id <= 10 )
    {
        parent -> scoreEditor -> loadKey(SHARP, id-3);
    }
	else if( id <= 17 )
    {
        parent -> scoreEditor -> loadKey(FLAT, id-10);
    }
    
    // load in keyboard editor too
    const int note7_to_note12[] = {
        /* A */ 0,
        /* B */ 2,
        /* C */ 3,
        /* D */ 5,
        /* E */ 7,
        /* F */ 8,
        /* G */ 10};
    
    // A, E, B, F#
    
    switch(id)
    {
    case 3:
        parent -> keyboardEditor -> loadKey( note7_to_note12[C] /* C */ );
        break;
    case 4:
        parent -> keyboardEditor -> loadKey( note7_to_note12[G] /* G */ );
        break;
    case 5:
        parent -> keyboardEditor -> loadKey( note7_to_note12[D] /* D */ );
        break;
    case 6:
        parent -> keyboardEditor -> loadKey( note7_to_note12[A] /* A */ );
        break;
    case 7:
        parent -> keyboardEditor -> loadKey( note7_to_note12[E] /* E */ );
        break;
    case 8:
        parent -> keyboardEditor -> loadKey( note7_to_note12[B] /* B */ );
        break;
    case 9:
        parent -> keyboardEditor -> loadKey( note7_to_note12[F]+1 /* F# */ );
        break;
    case 10:
        parent -> keyboardEditor -> loadKey( note7_to_note12[C]+1 /* C# */ );
        break;
    case 11:
        parent -> keyboardEditor -> loadKey( note7_to_note12[F] /* F */ );
        break;
    case 12:
        parent -> keyboardEditor -> loadKey( note7_to_note12[B]-1 /* Bb */ );
        break;
    case 13:
        parent -> keyboardEditor -> loadKey( note7_to_note12[E]-1 /* Eb */ );
        break;
    case 14:
        parent -> keyboardEditor -> loadKey( note7_to_note12[A]-1 + 12 /* Ab */ );
        break;
    case 15:
        parent -> keyboardEditor -> loadKey( note7_to_note12[D]-1 /* Db */ );
        break;
    case 16:
        parent -> keyboardEditor -> loadKey( note7_to_note12[G]-1 /* Gb */ );
        break;
    case 17:
        parent -> keyboardEditor -> loadKey( note7_to_note12[C]-1 /* Cb */ );
        break;
    }
    
    Display::render();
}

}
