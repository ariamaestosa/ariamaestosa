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

#include "Actions/RearrangeNotes.h"
#include "Dialogs/TuningDialog.h"
#include "Pickers/TuningPicker.h"
#include "Editors/GuitarEditor.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include "wx/wx.h"

#include "AriaCore.h"

#include <iostream>

using namespace AriaMaestosa;

enum
{
    TUNING_STD = 1,
    TUNING_DROPD,
    TUNING_DROPC,
    TUNING_DROPB,
    TUNING_BASS,
    TUNING_DROPDBASS,
    TUNING_CUSTOM,
    REARRANGE,

    LAST_ID
};

BEGIN_EVENT_TABLE(TuningPicker, wxMenu)
EVT_MENU_RANGE(1, LAST_ID-1, TuningPicker::menuItemSelected)
END_EVENT_TABLE()

/*
 * This is the dropdown menu that lets you select a new tuning. It also takes care of sending new tuning data to GuitarEditor.
 */

TuningPicker::TuningPicker() : wxMenu()
{
    Append(TUNING_STD,       wxT("Standard"));
    Append(TUNING_DROPD,     wxT("Dropped D"));
    Append(TUNING_DROPC,     wxT("Dropped C"));
    Append(TUNING_DROPB,     wxT("Dropped B"));
    Append(TUNING_BASS,      wxT("Bass"));
    Append(TUNING_DROPDBASS, wxT("Drop-D Bass"));
    AppendSeparator();
    Append(TUNING_CUSTOM,    wxT("Custom"));
    AppendSeparator();
    Append(REARRANGE,        _("Rearrange selected notes"));

    m_dialog_dialog = new TuningDialog();
}

void TuningPicker::setParent(GuitarEditor* parent_arg)
{
    parent = parent_arg;
    m_dialog_dialog->setParent(parent);
}

TuningPicker::~TuningPicker()
{
}


void TuningPicker::menuItemSelected(wxCommandEvent& evt)
{
    loadTuning( evt.GetId() );

    Display::render();
}

void TuningPicker::loadTuning(const int id, const bool user_triggered) // if user-triggered, it will be undoable
{
    switch(id)
    {
        case TUNING_STD: // standard
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 5) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPD: // drop D
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 5) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPC: // drop C
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 5) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_F, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_C, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_C, PITCH_SIGN_NONE, 3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPB: // drop B  //B-F#-B-e-g#-c#'
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_C, SHARP,           5) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, SHARP,           4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 4) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_F, SHARP,           3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_BASS: // bass
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 2) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPDBASS: // drop-D bass
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_G, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 3) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 2) );
            parent->tuning.push_back( Editor::findNotePitch(NOTE_7_D, PITCH_SIGN_NONE, 2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_CUSTOM: // custom
        {
            m_dialog_dialog->show();
            break;
        }

        case REARRANGE: // rearrange notes
            parent->track->action( new Action::RearrangeNotes() );

            break;
    }// end switch

}

