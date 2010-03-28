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
#include "Dialogs/CustomNoteSelectDialog.h"
#include "Pickers/TuningPicker.h"
#include "Editors/GuitarEditor.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include "wx/tokenzr.h"

#include "AriaCore.h"

#include <iostream>

namespace AriaMaestosa {

    /*
     * e.g.: you could call findNote('B','#',5) and it would return the correct number
     */

    int findNote(char noteLetter, char flatSharpSign, int octave)
{
        int note=(10-octave)*12;

        if (noteLetter=='B') note+=0;
        else if (noteLetter=='A') note+=2;
        else if (noteLetter=='G') note+=4;
        else if (noteLetter=='F') note+=6;
        else if (noteLetter=='E') note+=7;
        else if (noteLetter=='D') note+=9;
        else if (noteLetter=='C') note+=11;
        else{
            wxBell();
            //wxMessageBox( wxString("Invalid note: ") + wxString(noteLetter));
            std::cout << "Invalid note: " << noteLetter << std::endl;
            return 0;
        }

        if (flatSharpSign==' ');
        else if (flatSharpSign=='#') note-=1;
        else if (flatSharpSign=='b') note+=1;
        else{
            wxBell();
            //wxMessageBox( wxString("Error (expected #, b or nothing at all) : ") + wxString(flatSharpSign));
            std::cout << "Invalid sign: " << flatSharpSign << std::endl;
            return 0;
        }

        return note;
}


/*
 * In Custom guitar tuning editor, this is a single string
 */
class StringEditor : public wxPanel
{
public:
    LEAK_CHECK();

    wxBoxSizer* sizer;
    wxCheckBox* active;
    wxChoice* note_choice;
    wxChoice* sign_choice;
    wxChoice* octave_choice;

    void enterDefaultValue(int value)
    {

        // enter default value
        if (value!=-1)
        {

            const int octave=10 - (value/12);
            const int note=value%12;

            octave_choice->SetStringSelection( to_wxString(octave) );
            active->SetValue(true);

            switch(note){
                case 0:
                    note_choice->SetStringSelection(wxT("B"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 1:
                    note_choice->SetStringSelection(wxT("A"));
                    sign_choice->SetStringSelection(wxT("#"));
                    break;
                case 2:
                    note_choice->SetStringSelection(wxT("A"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 3:
                    note_choice->SetStringSelection(wxT("G"));
                    sign_choice->SetStringSelection(wxT("#"));
                    break;
                case 4:
                    note_choice->SetStringSelection(wxT("G"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 5:
                    note_choice->SetStringSelection(wxT("F"));
                    sign_choice->SetStringSelection(wxT("#"));
                    break;
                case 6:
                    note_choice->SetStringSelection(wxT("F"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 7:
                    note_choice->SetStringSelection(wxT("E"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 8:
                    note_choice->SetStringSelection(wxT("D"));
                    sign_choice->SetStringSelection(wxT("#"));
                    break;
                case 9:
                    note_choice->SetStringSelection(wxT("D"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
                case 10:
                    note_choice->SetStringSelection(wxT("C"));
                    sign_choice->SetStringSelection(wxT("#"));
                    break;
                case 11:
                    note_choice->SetStringSelection(wxT("C"));
                    sign_choice->SetStringSelection(wxT(" "));
                    break;
            } // end switch


        }

    }


    StringEditor(wxWindow* parent) : wxPanel(parent)
    {



        sizer = new wxBoxSizer(wxHORIZONTAL);

        // checkbox
        active = new wxCheckBox(this, 200, wxT(" "));
        sizer->Add(active, 0, wxALL, 5);

        // note choice
        {
        wxString choices[7] = { wxT("C"), wxT("D"), wxT("E"), wxT("F"), wxT("G"), wxT("A"), wxT("B")};
        note_choice = new wxChoice(this, 201, wxDefaultPosition, wxDefaultSize, 7, choices);
        sizer->Add(note_choice, 0, wxALL, 5);
        }

        // sign choice
        {
            wxString choices[3] = {wxT(" "), wxT("#"), wxT("b")};
            sign_choice = new wxChoice(this, 201, wxDefaultPosition, wxDefaultSize, 3, choices);
            sizer->Add(sign_choice, 0, wxALL, 5);
        }

        // octave choice
        {
        wxString choices[10] = {wxT("1"), wxT("2"), wxT("3"), wxT("4"), wxT("5"), wxT("6"), wxT("7"), wxT("8"), wxT("9"), wxT("10")};
        octave_choice = new wxChoice(this, 201, wxDefaultPosition, wxDefaultSize, 10, choices);
        sizer->Add(octave_choice, 0, wxALL, 5);
        }

        SetAutoLayout(true);
        SetSizer(sizer);
        sizer->Layout();
        sizer->SetSizeHints(this); // resize to take ideal space


    }

    void somethingSelected(wxCommandEvent& evt)
    {
        active->SetValue(true);
    }

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(StringEditor, wxPanel)
EVT_CHOICE(201, StringEditor::somethingSelected)
END_EVENT_TABLE()

/*
 * This is the rame that lets you enter a custom tuning.
 */

class CustomTuningPicker : public wxFrame
{
    wxPanel* buttonPane;
    wxButton* ok_btn;
    wxButton* cancel_btn;
    wxBoxSizer* buttonsizer;

    wxBoxSizer* sizer;

    GuitarEditor* parent;

    StringEditor* strings[10];

public:
    LEAK_CHECK();

    ~CustomTuningPicker()
    {
    }

    CustomTuningPicker() :
        wxFrame(NULL, wxID_ANY,  _("Custom Tuning Editor"), wxPoint(100,100), wxSize(500,300), wxCAPTION )
    {


        sizer = new wxBoxSizer(wxVERTICAL);

        for(int n=0; n<10; n++)
        {
            strings[n] = new StringEditor(this);
            sizer->Add(strings[n], 0, wxALL, 5);
        }

        buttonPane = new wxPanel(this);
        sizer->Add(buttonPane, 0, wxALL, 5);

        buttonsizer = new wxBoxSizer(wxHORIZONTAL);

        ok_btn = new wxButton(buttonPane, 200, _("OK"));
        ok_btn->SetDefault();
        buttonsizer->Add(ok_btn, 0, wxALL, 5);

        cancel_btn = new wxButton(buttonPane, 202,  _("Cancel"));
        buttonsizer->Add(cancel_btn, 0, wxALL, 5);

        buttonPane->SetSizer(buttonsizer);
        parent = NULL;
        
        SetAutoLayout(true);
        SetSizer(sizer);
        sizer->Layout();
        sizer->SetSizeHints(this); // resize window to take ideal space
    }

    void setParent(GuitarEditor* parent_arg)
    {
        parent = parent_arg;
    }

    void show()
    {
        ASSERT(parent != NULL);
        
        // enter default values
        for(int n=0; n<10; n++)
        {
            int value = -1;
            if (n<(int)parent->tuning.size()) value = parent->tuning[n];

            strings[n]->enterDefaultValue(value);
        }

        Center();
        Show();
    }

    // when Cancel button of the tuning picker is pressed
    void cancelButton(wxCommandEvent& evt)
    {
        Hide();
    }

    // when OK button of the tuning picker is pressed
    void okButton(wxCommandEvent& evt)
    {
        parent->previous_tuning = parent->tuning;
        parent->tuning.clear();

        // set new tuning
        for(int n=0; n<10; n++)
        {
            if ( strings[n]->active->IsChecked() )
            {
                wxString note = strings[n]->note_choice->GetStringSelection();
                wxString sign = strings[n]->sign_choice->GetStringSelection();
                wxString octave = strings[n]->octave_choice->GetStringSelection();

                std::cout << (char)(note.GetChar(0)) << " " << (char)(sign.GetChar(0)) << " " << (char)(octave.GetChar(0)) << std::endl;

                parent->tuning.push_back(
                                         findNote(
                                                  (char)(note.GetChar(0)) ,
                                                  (char)(sign.GetChar(0)),
                                                  atoi_u( octave )
                                                  )
                                         );

            }
        }

        parent->tuningUpdated();
        Display::render();

        Hide();
    }

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CustomTuningPicker, wxFrame)
EVT_BUTTON(200, CustomTuningPicker::okButton)
EVT_BUTTON(202, CustomTuningPicker::cancelButton)
END_EVENT_TABLE()

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

    ctp = new CustomTuningPicker();
}

void TuningPicker::setParent(GuitarEditor* parent_arg)
{
    parent = parent_arg;
    ctp->setParent(parent);
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
    switch(id){
        case TUNING_STD: // standard
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( findNote('E',' ',5) );
            parent->tuning.push_back( findNote('B',' ',4) );
            parent->tuning.push_back( findNote('G',' ',4) );
            parent->tuning.push_back( findNote('D',' ',4) );
            parent->tuning.push_back( findNote('A',' ',3) );
            parent->tuning.push_back( findNote('E',' ',3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPD: // drop D
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( findNote('E',' ',5) );
            parent->tuning.push_back( findNote('B',' ',4) );
            parent->tuning.push_back( findNote('G',' ',4) );
            parent->tuning.push_back( findNote('D',' ',4) );
            parent->tuning.push_back( findNote('A',' ',3) );
            parent->tuning.push_back( findNote('D',' ',3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPC: // drop C
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( findNote('D',' ',5) );
            parent->tuning.push_back( findNote('A',' ',4) );
            parent->tuning.push_back( findNote('F',' ',4) );
            parent->tuning.push_back( findNote('C',' ',4) );
            parent->tuning.push_back( findNote('G',' ',3) );
            parent->tuning.push_back( findNote('C',' ',3) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPB: // drop B  //B-F#-B-e-g#-c#'
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            parent->tuning.push_back( findNote('C','#',5) );
            parent->tuning.push_back( findNote('G','#',4) );
            parent->tuning.push_back( findNote('E',' ',4) );
            parent->tuning.push_back( findNote('B',' ',3) );
            parent->tuning.push_back( findNote('F','#',3) );
            parent->tuning.push_back( findNote('B',' ',2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_BASS: // bass
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            //parent->tuning.push_back( findNote('E',' ',4) );
            parent->tuning.push_back( findNote('B',' ',3) );
            parent->tuning.push_back( findNote('G',' ',3) );
            parent->tuning.push_back( findNote('D',' ',3) );
            parent->tuning.push_back( findNote('A',' ',2) );
            parent->tuning.push_back( findNote('E',' ',2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_DROPDBASS: // drop-D bass
            parent->previous_tuning = parent->tuning;
            parent->tuning.clear();
            //parent->tuning.push_back( findNote('E',' ',4) );
            parent->tuning.push_back( findNote('B',' ',3) );
            parent->tuning.push_back( findNote('G',' ',3) );
            parent->tuning.push_back( findNote('D',' ',3) );
            parent->tuning.push_back( findNote('A',' ',2) );
            parent->tuning.push_back( findNote('D',' ',2) );
            parent->tuningUpdated(user_triggered);
            break;

        case TUNING_CUSTOM: // custom
        {
            ctp->show();
            break;
        }

        case REARRANGE: // rearrange notes
            parent->track->action( new Action::RearrangeNotes() );

            break;
    }// end switch

}

}
