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

#include "AriaCore.h"
#include "Dialogs/CustomKeyDialog.h"
#include "Dialogs/PresetEditor.h"
#include "Editors/ScoreEditor.h"
#include "Editors/KeyboardEditor.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/Track.h"
#include "Midi/KeyPresets.h"
#include "Pickers/KeyPicker.h"

#include "wx/wx.h"
#include "wx/notebook.h"
#include "wx/textdlg.h"

namespace AriaMaestosa
{
    
#if 0
#pragma mark -
#pragma mark KeyPicker
#endif

    enum IDs
    {
        MUSICAL_NOTATION = 1,
        LINEAR_NOTATION,
        F_CLEF,
        G_CLEF,
        OCTAVE_ABOVE,
        OCTAVE_BELOW,

        KEY_C_AM,
        KEY_SHARPS_1,
        KEY_SHARPS_2,
        KEY_SHARPS_3,
        KEY_SHARPS_4,
        KEY_SHARPS_5,
        KEY_SHARPS_6,
        KEY_SHARPS_7,
        KEY_FLATS_1,
        KEY_FLATS_2,
        KEY_FLATS_3,
        KEY_FLATS_4,
        KEY_FLATS_5,
        KEY_FLATS_6,
        KEY_FLATS_7,
        KEY_GUESS,
        KEY_CUSTOM,

        ID_AMOUNT
    };
}
using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(KeyPicker, wxMenu)
EVT_MENU_RANGE(1, ID_AMOUNT-1, KeyPicker::menuItemSelected)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------------------------------------

KeyPicker::KeyPicker() : wxMenu()
{
    //I18N: - in the view settings for score editor. whether to show notes in a traditional musical way.
    musical_checkbox = AppendCheckItem(MUSICAL_NOTATION,_("Musical notation")); musical_checkbox->Check(true);
    //I18N: - in the view settings for score editor. whether to show notes in a "computer" linear way.
    linear_checkbox = AppendCheckItem(LINEAR_NOTATION,_("Linear Notation")); linear_checkbox->Check(true);

    //AppendSeparator();
    gclef = AppendCheckItem(G_CLEF, _("G Clef")); gclef->Check(true);
    fclef = AppendCheckItem(F_CLEF, _("F Clef")); fclef->Check(true);
    //I18N: - show score an octave higher
    octave_above = AppendCheckItem(OCTAVE_ABOVE, _("Octave +1"));
    //I18N: - show score an octave lower
    octave_below = AppendCheckItem(OCTAVE_BELOW, _("Octave -1"));

    score_items_added = true;

    AppendSeparator();
    key_c = AppendCheckItem(KEY_C_AM, wxT("C, Am"));
    key_c->Check(true);

    AppendSeparator();
    key_sharps_1 = AppendCheckItem( KEY_SHARPS_1, wxT("G, Em"));
    key_sharps_2 = AppendCheckItem( KEY_SHARPS_2, wxT("D, Bm"));
    key_sharps_3 = AppendCheckItem( KEY_SHARPS_3, wxT("A, F#m"));
    key_sharps_4 = AppendCheckItem( KEY_SHARPS_4, wxT("E, C#m"));
    key_sharps_5 = AppendCheckItem( KEY_SHARPS_5, wxT("B, G#m"));
    key_sharps_6 = AppendCheckItem( KEY_SHARPS_6, wxT("F#, D#m"));
    key_sharps_7 = AppendCheckItem( KEY_SHARPS_7, wxT("C#, A#m"));

    AppendSeparator();
    key_flats_1 = AppendCheckItem( KEY_FLATS_1, wxT("F, Dm"));
    key_flats_2 = AppendCheckItem( KEY_FLATS_2, wxT("Bb, Gm"));
    key_flats_3 = AppendCheckItem( KEY_FLATS_3, wxT("Eb, Cm"));
    key_flats_4 = AppendCheckItem( KEY_FLATS_4, wxT("Ab, Fm"));
    key_flats_5 = AppendCheckItem( KEY_FLATS_5, wxT("Db, Bbm"));
    key_flats_6 = AppendCheckItem( KEY_FLATS_6, wxT("Gb, Ebm"));
    key_flats_7 = AppendCheckItem( KEY_FLATS_7, wxT("Cb, Abm"));

    AppendSeparator();
    
    m_user_presets = NULL;
    
    ptr_vector<IPreset, REF> userDefinedKeys = KeyPresetGroup::getInstance()->getPresets();
    const int userDefinedKeysCount = userDefinedKeys.size();
    if (userDefinedKeysCount > 0)
    {
        updateUserPresetsMenu();
    }
    
    Append( KEY_GUESS,  _("Guess Key"));
    m_custom_key_menu = AppendCheckItem( KEY_CUSTOM, _("Custom Key"));
}

// ----------------------------------------------------------------------------------------------------------

KeyPicker::~KeyPicker()
{
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::updateUserPresetsMenu()
{
    // FIXME: after being updated, the user presets menu has moved to the bottom
    if (m_user_presets != NULL)
    {
        wxMenuItem* removedItem = Remove(m_user_presets);
        ASSERT(removedItem != NULL);
        delete removedItem;
    }
    
    ptr_vector<IPreset, REF> userDefinedKeys = KeyPresetGroup::getInstance()->getPresets();
    const int userDefinedKeysCount = userDefinedKeys.size();
    
    wxMenu* userDefinedMenu = new wxMenu();
    m_user_presets = AppendSubMenu(userDefinedMenu, _("User presets"));
    for (int n=0; n<userDefinedKeysCount; n++)
    {
        int id = wxNewId();
        userDefinedMenu->Append( id, userDefinedKeys[n].getName() );
        KeyPreset* keyPreset = dynamic_cast<KeyPreset*>(userDefinedKeys.get(n));
        ASSERT(keyPreset != NULL);
        m_custom_keys[id] = keyPreset;
        userDefinedMenu->Connect(id, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(KeyPicker::onUserPresetSelected),
                                 NULL, this);
    }
    userDefinedMenu->AppendSeparator();
    
    wxMenuItem* editPresets = userDefinedMenu->Append(wxNewId(), _("Edit Presets..."));
    userDefinedMenu->Connect(editPresets->GetId(),
                             wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(KeyPicker::onEditPresets),
                             NULL, this);
    
    AppendSeparator();    
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::setParent(Track* parent_arg)
{
    parent = parent_arg->graphics;
    if (parent->getEditorMode() == KEYBOARD)
    {
        if (score_items_added)
        {
            Remove(musical_checkbox);
            Remove(linear_checkbox);
            Remove(fclef);
            Remove(gclef);
            Remove(octave_above);
            Remove(octave_below);
            score_items_added = false;
        }
    }
    else if (parent->getEditorMode() == SCORE)
    {
        if (not score_items_added)
        {
            Prepend(octave_below);
            Prepend(octave_above);
            Prepend(gclef);
            Prepend(fclef);
            //PrependSeparator();
            Prepend(linear_checkbox);
            Prepend(musical_checkbox);
            score_items_added = true;
        }
    }

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
    m_custom_key_menu->Check(false);

    const KeyType type = parent_arg->getKeyType();

    switch (type)
    {
        case KEY_TYPE_C:
        {
            key_c->Check(true);
            break;
        }

        case KEY_TYPE_SHARPS:
        {
            const int sharps = parent_arg->getKeySharpsAmount();

            if (sharps == 1) key_sharps_1->Check(true);
            else if (sharps == 2) key_sharps_2->Check(true);
            else if (sharps == 3) key_sharps_3->Check(true);
            else if (sharps == 4) key_sharps_4->Check(true);
            else if (sharps == 5) key_sharps_5->Check(true);
            else if (sharps == 6) key_sharps_6->Check(true);
            else if (sharps == 7) key_sharps_7->Check(true);
            break;
        }

        case KEY_TYPE_FLATS:
        {
            const int flats  = parent_arg->getKeyFlatsAmount();

            if (flats == 1) key_flats_1->Check(true);
            else if (flats == 2) key_flats_2->Check(true);
            else if (flats == 3) key_flats_3->Check(true);
            else if (flats == 4) key_flats_4->Check(true);
            else if (flats == 5) key_flats_5->Check(true);
            else if (flats == 6) key_flats_6->Check(true);
            else if (flats == 7) key_flats_7->Check(true);
            break;
        }

        case KEY_TYPE_CUSTOM:
        {
            m_custom_key_menu->Check(true);
            break;
        }
    }
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::setChecks(bool musicalNotationEnabled, bool linearNotationEnabled,
                          bool f_clef, bool g_clef, int octave_shift)
{
    musical_checkbox -> Check(musicalNotationEnabled);
    linear_checkbox  -> Check(linearNotationEnabled);
    fclef            -> Check(f_clef);
    gclef            -> Check(g_clef);

    if (octave_shift == -1)
    {
        octave_above -> Check(false);
        octave_below -> Check(true);
    }
    else if (octave_shift == 1)
    {
        octave_above -> Check(true);
        octave_below -> Check(false);
    }
    else
    {
        octave_above -> Check(false);
        octave_below -> Check(false);
    }
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::menuItemSelected(wxCommandEvent& evt)
{
    const int id = evt.GetId();

    if ( id < 0 or id > ID_AMOUNT ) return;

    if ( id == MUSICAL_NOTATION )
    {
        parent -> scoreEditor -> enableMusicalNotation( musical_checkbox->IsChecked() );

        // don't allow disabling both
        if (not musical_checkbox->IsChecked() and not linear_checkbox->IsChecked())
        {
            parent->scoreEditor->enableLinearNotation(true);
        }
    }
    else if ( id == LINEAR_NOTATION )
    {
        parent -> scoreEditor -> enableLinearNotation( linear_checkbox->IsChecked() );

        // don't allow disabling both
        if (not musical_checkbox->IsChecked() and not linear_checkbox->IsChecked())
        {
            parent->scoreEditor->enableMusicalNotation(true);
        }
    }
    else if ( id == F_CLEF )
    {
        parent -> scoreEditor -> enableFClef( fclef->IsChecked() );

        // don't allow disabling both
        if (not gclef->IsChecked() and not fclef->IsChecked())
        {
            parent->scoreEditor->enableGClef( true );
        }
    }
    else if ( id == G_CLEF )
    {
        parent -> scoreEditor -> enableGClef( gclef->IsChecked() );

        // don't allow disabling both
        if (not gclef->IsChecked() and not fclef->IsChecked())
        {
            parent->scoreEditor->enableFClef( true );
        }
    }
    else if ( id == OCTAVE_ABOVE )
    {
        octave_below -> Check(false);
        parent -> scoreEditor -> getScoreMidiConverter() -> setOctaveShift( octave_above->IsChecked() ? 1 : 0 );
    }
    else if ( id == OCTAVE_BELOW )
    {
        octave_above -> Check(false);
        parent -> scoreEditor -> getScoreMidiConverter() -> setOctaveShift( octave_below->IsChecked() ? -1 : 0 );
    }
    else if ( id == KEY_C_AM )
    {
        parent->getTrack()->setKey(0, KEY_TYPE_C);
    }
    else if ( id >= KEY_SHARPS_1 and id <= KEY_SHARPS_7 )
    {
        parent->getTrack()->setKey(id-KEY_SHARPS_1+1, KEY_TYPE_SHARPS);
    }
    else if ( id >= KEY_FLATS_1 and id <= KEY_FLATS_7 )
    {
        parent->getTrack()->setKey(id-KEY_FLATS_1+1, KEY_TYPE_FLATS);
    }
    else if ( id == KEY_CUSTOM )
    {
        CustomKeyDialog dialog( (wxFrame*)getMainFrame(), parent );
        dialog.ShowModal();
    }
    else if ( id == KEY_GUESS )
    {
        int note_12_occurance[12];
        for (int n=0; n<12; n++)
        {
            note_12_occurance[n] = 0;
        }

        // count how many A's, how many B's, etc., we have in this track
        const int noteAmount = parent->getTrack()->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            const int pitch = parent->getTrack()->getNotePitchID(n);
            int r = 11 - pitch%12;
            if (r < 0)  r+=12;
            if (r > 11) r-=12;
            //std::cout << "r=" << r << std::endl;
            note_12_occurance[r] += 1;
        }

        // find max value, will make it easier to compare values
        //int max = 0;
        //for(int n=0; n<12; n++)
        //    if (note_12_occurance[n] > max) max = note_12_occurance[n];
        // print values for debug purposes
        //for(int n=0; n<12; n++)
        //    std::cout << (note_12_occurance[n] * 100 / max) << std::endl;

        // test the note repartition pattern found against keys
        // and grade them ( smaller = less alike, bigger = much alike )
        // here we only test major keys; a minor one will get the same key sig
        int key_test[12];
        for (int n=0; n<12; n++)
        {
            key_test[n] = 0; // reset before starting

            // some values are somehwat arbitrarly multiplied by 2 cause
            // they are more likely/unlikely to be found in a given key

            key_test[n] += note_12_occurance[n]*2; // tonic
            key_test[n] -= note_12_occurance[(n+1)%12]*2; // 2nd minor
            key_test[n] += note_12_occurance[(n+2)%12]; // 2nd major
            key_test[n] -= note_12_occurance[(n+3)%12]; // 3rd minor
            key_test[n] += note_12_occurance[(n+4)%12]; // 3rd major
            key_test[n] += note_12_occurance[(n+5)%12]; // 4th
            key_test[n] -= note_12_occurance[(n+6)%12]*2; // tritone
            key_test[n] += note_12_occurance[(n+7)%12]*2; // fifth
                                                          //key_test[n] -= note_12_occurance[(n+8)%12]/2; // 6th minor
                                                          //key_test[n] += note_12_occurance[(n+9)%12]/2; // 6th major
                                                          // 6ths are less accounted for, since they could be the 7th of a minor key
            key_test[n] -= note_12_occurance[(n+10)%12]; // 7th minor
            key_test[n] += note_12_occurance[(n+11)%12]; // 7th major
        }

        int best_candidate = -1;
        for (int n=0; n<12; n++)
        {
            if (best_candidate==-1 or key_test[n]>key_test[best_candidate]) best_candidate = n;
        }
        if (best_candidate==-1) return;

        KeyType type = KEY_TYPE_C;
        int amount = 0;
        switch (best_candidate)
        {
            case 0: //C
                break;

            case 1: // C#
                type = KEY_TYPE_SHARPS;
                amount = 7;
                break;
            case 2: // D
                type = KEY_TYPE_SHARPS;
                amount = 2;
                break;
            case 4: // E
                type = KEY_TYPE_SHARPS;
                amount = 4;
                break;
            case 6: // F#
                type = KEY_TYPE_SHARPS;
                amount = 6;
                break;
            case 7: // G
                type = KEY_TYPE_SHARPS;
                amount = 1;
                break;
            case 9: // A
                type = KEY_TYPE_SHARPS;
                amount = 3;
                break;
            case 11: // B
                type = KEY_TYPE_SHARPS;
                amount = 5;
                break;

            case 3: // D#
                type = KEY_TYPE_FLATS;
                amount = 3;
                break;
            case 5: // F
                type = KEY_TYPE_FLATS;
                amount = 1;
                break;
            case 8: // G#
                type = KEY_TYPE_FLATS;
                amount = 4;
                break;
            case 10: // A#
                type = KEY_TYPE_FLATS;
                amount = 2;
                break;
        }
        parent->getTrack()->setKey(amount, type);
    }

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::onUserPresetSelected(wxCommandEvent& evt)
{
    const int id = evt.GetId();
    if (m_custom_keys.find(id) == m_custom_keys.end())
    {
        std::cerr << "Invalid key selected!\n";
        ASSERT(false);
    }
    
    KeyPreset* selection = m_custom_keys[id];
    
    parent->getTrack()->setCustomKey( selection->getArray() );
}

// ----------------------------------------------------------------------------------------------------------

void KeyPicker::onEditPresets(wxCommandEvent& evt)
{
    PresetEditor presetEditor((wxWindow*)getMainFrame(), (PresetGroup*)KeyPresetGroup::getInstance());
}

// ----------------------------------------------------------------------------------------------------------
