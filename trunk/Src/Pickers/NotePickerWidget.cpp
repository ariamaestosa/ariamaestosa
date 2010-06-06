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

#include "Pickers/NotePickerWidget.h"

#include "Editors/Editor.h"
#include "wx/choice.h"
#include "wx/sizer.h"

using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(NotePickerWidget, wxPanel)
EVT_CHOICE(201, NotePickerWidget::somethingSelected)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------------------------------------

NotePickerWidget::NotePickerWidget(wxWindow* parent, bool withCheckbox) : wxPanel(parent)
{
    sizer = new wxBoxSizer(wxHORIZONTAL);
    
    // checkbox
    if (withCheckbox)
    {
        m_active = new wxCheckBox(this, 200, wxT(" "));
        sizer->Add(m_active, 0, wxALL, 5);
    }
    else
    {
        m_active = NULL;
    }
    
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

// ---------------------------------------------------------------------------------------------------------

void NotePickerWidget::enterDefaultValue(int pitchID)
{
    
    // enter default value
    if (pitchID != -1)
    {
        Note12 noteName;
        int octave;
        
        const bool success = Editor::findNoteName(pitchID, &noteName, &octave);
        if (not success) return;
        
        octave_choice->SetStringSelection( to_wxString(octave) );
        if (m_active != NULL) m_active->SetValue(true);
        
        switch (noteName)
        {
            case NOTE_12_B:
                note_choice->SetStringSelection(wxT("B"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_A_SHARP:
                note_choice->SetStringSelection(wxT("A"));
                sign_choice->SetStringSelection(wxT("#"));
                break;
            case NOTE_12_A:
                note_choice->SetStringSelection(wxT("A"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_G_SHARP:
                note_choice->SetStringSelection(wxT("G"));
                sign_choice->SetStringSelection(wxT("#"));
                break;
            case NOTE_12_G:
                note_choice->SetStringSelection(wxT("G"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_F_SHARP:
                note_choice->SetStringSelection(wxT("F"));
                sign_choice->SetStringSelection(wxT("#"));
                break;
            case NOTE_12_F:
                note_choice->SetStringSelection(wxT("F"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_E:
                note_choice->SetStringSelection(wxT("E"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_D_SHARP:
                note_choice->SetStringSelection(wxT("D"));
                sign_choice->SetStringSelection(wxT("#"));
                break;
            case NOTE_12_D:
                note_choice->SetStringSelection(wxT("D"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
            case NOTE_12_C_SHARP:
                note_choice->SetStringSelection(wxT("C"));
                sign_choice->SetStringSelection(wxT("#"));
                break;
            case NOTE_12_C:
                note_choice->SetStringSelection(wxT("C"));
                sign_choice->SetStringSelection(wxT(" "));
                break;
        } // end switch
    }
    else
    {
        if (m_active != NULL) m_active->SetValue(false);
        note_choice->SetStringSelection(wxT("C"));
        sign_choice->SetStringSelection(wxT(" "));
        octave_choice->SetStringSelection( wxT("1") );
    }
}

// ---------------------------------------------------------------------------------------------------------

void NotePickerWidget::somethingSelected(wxCommandEvent& evt)
{
    if (m_active != NULL) m_active->SetValue(true);
}

// ---------------------------------------------------------------------------------------------------------

/**
 * @brief e.g.: you could call findNote('B','#',5) and it would return the correct number
 */
int findNote(char noteLetter, char flatSharpSign, int octave)
{        
    Note7 note;
    PitchSign sign = PITCH_SIGN_NONE;
    
    if      (noteLetter=='B') note = NOTE_7_B;
    else if (noteLetter=='A') note = NOTE_7_A;
    else if (noteLetter=='G') note = NOTE_7_G;
    else if (noteLetter=='F') note = NOTE_7_F;
    else if (noteLetter=='E') note = NOTE_7_E;
    else if (noteLetter=='D') note = NOTE_7_D;
    else if (noteLetter=='C') note = NOTE_7_C;
    else
    {
        wxBell();
        std::cerr << "Invalid note: " << noteLetter << std::endl;
        return 0;
    }
    
    if (flatSharpSign==' ')
    {
        sign = PITCH_SIGN_NONE;
    }
    else if (flatSharpSign=='#')
    {
        sign = SHARP;
    }
    else if (flatSharpSign=='b')
    {
        sign = FLAT;
    }
    else
    {
        wxBell();
        //wxMessageBox( wxString("Error (expected #, b or nothing at all) : ") + wxString(flatSharpSign));
        std::cout << "Invalid sign: " << flatSharpSign << std::endl;
        return 0;
    }
    
    return Editor::findNotePitch(note, sign, octave);
}

// ---------------------------------------------------------------------------------------------------------

int NotePickerWidget::getSelectedNote() const
{
    wxString note = note_choice->GetStringSelection();
    wxString sign = sign_choice->GetStringSelection();
    wxString octave = octave_choice->GetStringSelection();
    
    std::cout << (char)(note.GetChar(0)) << " " << (char)(sign.GetChar(0)) << " " << (char)(octave.GetChar(0)) << std::endl;
    
    const int notepitch = findNote(
                                   (char)(note.GetChar(0)) ,
                                   (char)(sign.GetChar(0)),
                                   atoi_u( octave )
                                   );
    return notepitch;
}

// ---------------------------------------------------------------------------------------------------------

