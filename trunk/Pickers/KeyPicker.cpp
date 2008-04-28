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
    key_c = AppendCheckItem(3,wxT("C, Am"));
    key_c->Check(true);
    
	AppendSeparator();
	key_sharps_1 = AppendCheckItem(4,wxT("G, Em")); // # 1
	key_sharps_2 = AppendCheckItem(5,wxT("D, Bm")); // # 2
	key_sharps_3 = AppendCheckItem(6,wxT("A, F#m")); // # 3
	key_sharps_4 = AppendCheckItem(7,wxT("E, C#m")); // # 4
	key_sharps_5 = AppendCheckItem(8,wxT("B, G#m")); // # 5
	key_sharps_6 = AppendCheckItem(9,wxT("F#, D#m")); // # 6
	key_sharps_7 = AppendCheckItem(10,wxT("C#, A#m")); // # 7
    
	AppendSeparator();
	key_flats_1 = AppendCheckItem(11,wxT("F, Dm")); // b 1
	key_flats_2 = AppendCheckItem(12,wxT("Bb, Gm")); // b 2
	key_flats_3 = AppendCheckItem(13,wxT("Eb, Cm")); // b 3
	key_flats_4 = AppendCheckItem(14,wxT("Ab, Fm")); // b 4
    key_flats_5 = AppendCheckItem(15,wxT("Db, Bbm")); // b 5
	key_flats_6 = AppendCheckItem(16,wxT("Gb, Ebm")); // b 6 
	key_flats_7 = AppendCheckItem(17,wxT("Cb, Abm")); // b 7
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
    
    // FIXME - all code confusingly assumes score editor and keyboard editor always use the same key
    // decide whether they do - if they don't then split the code. if they do, put the common code in a single place
    key_c->Check(false);
    key_sharps_1->Check(false);
    key_sharps_2->Check(false);
    key_sharps_3->Check(false);
    key_sharps_4->Check(false);
    key_sharps_5->Check(false);
    key_sharps_6->Check(false);
    key_sharps_7->Check(false);
    key_flats_1->Check(false);
    key_flats_2->Check(false);
    key_flats_3->Check(false);
    key_flats_4->Check(false);
    key_flats_5->Check(false);
    key_flats_6->Check(false);
    key_flats_7->Check(false);
    const int sharps = parent->scoreEditor->getKeySharpsAmount();
    const int flats = parent->scoreEditor->getKeyFlatsAmount();
    if(sharps==0 and flats==0)
        key_c->Check(true);
    else if(sharps > flats)
    {
        if(sharps == 1) key_sharps_1->Check(true);
        else if(sharps == 2) key_sharps_2->Check(true);
        else if(sharps == 3) key_sharps_3->Check(true);
        else if(sharps == 4) key_sharps_4->Check(true);
        else if(sharps == 5) key_sharps_5->Check(true);
        else if(sharps == 6) key_sharps_6->Check(true);
        else if(sharps == 7) key_sharps_7->Check(true);
    }
    else
    {
        if(flats == 1) key_flats_1->Check(true);
        else if(flats == 2) key_flats_2->Check(true);
        else if(flats == 3) key_flats_3->Check(true);
        else if(flats == 4) key_flats_4->Check(true);
        else if(flats == 5) key_flats_5->Check(true);
        else if(flats == 6) key_flats_6->Check(true);
        else if(flats == 7) key_flats_7->Check(true);
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
        parent -> keyboardEditor -> loadKey(NATURAL, 0);
    }
	else if( id <= 10 )
    {
        parent -> scoreEditor -> loadKey(SHARP, id-3);
        parent -> keyboardEditor -> loadKey(SHARP, id-3);
    }
	else if( id <= 17 )
    {
        parent -> scoreEditor -> loadKey(FLAT, id-10);
        parent -> keyboardEditor -> loadKey(FLAT, id-10);
    }
    
    Display::render();
}

}
