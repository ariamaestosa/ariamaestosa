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
#include "Dialogs/CustomNoteSelectDialog.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/MeasureData.h"
#include "GUI/GraphicalTrack.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/checkbox.h>
#include <wx/button.h>

namespace AriaMaestosa
{
    
    enum
    {
        ID_OK,
        ID_CANCEL,
        ID_TXT1 = 8025,
        ID_TXT2,
        ID_TXT3
    };
    
}
using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(CustomNoteSelectDialog, wxDialog)

EVT_BUTTON(ID_OK, CustomNoteSelectDialog::okClicked)
EVT_BUTTON(ID_CANCEL, CustomNoteSelectDialog::cancelClicked)
EVT_KEY_DOWN(CustomNoteSelectDialog::keyPress)

//EVT_SET_FOCUS(CustomNoteSelectDialog::onFocus)
//EVT_KILL_FOCUS(CustomNoteSelectDialog::onFocus)

END_EVENT_TABLE()

// ----------------------------------------------------------------------------------------------------------

CustomNoteSelectDialog::CustomNoteSelectDialog() : wxDialog(NULL, wxID_ANY,
                                                            //I18N: - title of the custom note select dialog
                                                            _("Select notes..."),
                                                            wxPoint(300,100), wxSize(200,400))
{
    
    wxSize smallTextCtrlSize(wxDefaultSize);
    smallTextCtrlSize.SetWidth(35);
    
    boxSizer = new wxBoxSizer(wxVERTICAL);
    
    //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
    //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
    wxStaticText* txt=new wxStaticText(this, wxID_ANY,  _("Select notes that..."));
    boxSizer->Add(txt, 0, wxALL, 15);
    
    //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
    //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
    cb_pitch=new wxCheckBox(this, wxID_ANY,  _("have the same pitch"));
    boxSizer->Add(cb_pitch, 0, wxALL, 5);
    
    
    // --------------------------------------
    {
        wxPanel* panel_volume=new wxPanel(this);
        boxSizer->Add(panel_volume, 0, wxALL, 0);
        wxBoxSizer* subBoxSizer=new wxBoxSizer(wxHORIZONTAL);
        
        //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
        //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
        cb_volume=new wxCheckBox(panel_volume, wxID_ANY,  _("have a similar volume"));
        subBoxSizer->Add(cb_volume, 0, wxALL, 5);
        
        wxStaticText* lbl_more_or_less = new wxStaticText(panel_volume, wxID_ANY, wxT("+/-"));
        subBoxSizer->Add(lbl_more_or_less, 00, wxALL, 5);
        
        volume_tolerance = new wxTextCtrl(panel_volume, ID_TXT1, wxT("5"), wxDefaultPosition, smallTextCtrlSize, wxTAB_TRAVERSAL);
        subBoxSizer->Add(volume_tolerance, 0, wxALL, 5);
        
        volume_tolerance->Connect( volume_tolerance->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(CustomNoteSelectDialog::onFocus), 0, this);
        
        panel_volume->SetAutoLayout(TRUE);
        panel_volume->SetSizer(subBoxSizer);
        subBoxSizer->Layout();
    }
    // --------------------------------------
    
    //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
    //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
    cb_string=new wxCheckBox(this, wxID_ANY,  _("are on the same string"));
    boxSizer->Add(cb_string, 0, wxALL, 5);
    
    //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
    //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
    cb_fret=new wxCheckBox(this, wxID_ANY,  _("have the same fret"));
    boxSizer->Add(cb_fret, 0, wxALL, 5);
    
    // --------------------------------------
    {
        wxPanel* panel_duration=new wxPanel(this);
        boxSizer->Add(panel_duration, 0, wxALL, 0);
        wxBoxSizer* subBoxSizer=new wxBoxSizer(wxHORIZONTAL);
        
        //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
        //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
        cb_duration=new wxCheckBox(panel_duration, wxID_ANY,  _("have a similar duration"));
        subBoxSizer->Add(cb_duration, 0, wxALL, 5);
        
        panel_duration->SetAutoLayout(TRUE);
        panel_duration->SetSizer(subBoxSizer);
        subBoxSizer->Layout();
    }
    // --------------------------------------
    
    // --------------------------------------
    {
        wxPanel* panel_measures=new wxPanel(this);
        boxSizer->Add(panel_measures, 0, wxALL, 0);
        wxBoxSizer* subBoxSizer=new wxBoxSizer(wxHORIZONTAL);
        
        //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
        //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
        cb_measure=new wxCheckBox(panel_measures, wxID_ANY,  _("are located in measures "));
        subBoxSizer->Add(cb_measure, 0, wxALL, 5);
        
        from_measure = new wxTextCtrl(panel_measures, ID_TXT2, wxT("1"), wxDefaultPosition, smallTextCtrlSize);
        subBoxSizer->Add(from_measure, 0, wxALL, 5);
        from_measure->Connect( from_measure->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(CustomNoteSelectDialog::onFocus), 0, this);
        
        //I18N: - in custom note select dialog. full context :\n\n Select notes that...\n\n* have the same pitch\n* have a similar volume *
        //I18N: - * are on the same string * have the same fret * have a similar duration * are located in measures ... to ...
        wxStaticText* lbl_to = new wxStaticText(panel_measures, wxID_ANY,  _(" to "));
        subBoxSizer->Add(lbl_to, 0, wxALL, 5);
        
        to_measure = new wxTextCtrl(panel_measures, ID_TXT3, wxT("2"), wxDefaultPosition, smallTextCtrlSize);
        subBoxSizer->Add(to_measure, 0, wxALL, 5);
        to_measure->Connect( to_measure->GetId(), wxEVT_SET_FOCUS, wxFocusEventHandler(CustomNoteSelectDialog::onFocus), 0, this);
        
        panel_measures->SetAutoLayout(TRUE);
        panel_measures->SetSizer(subBoxSizer);
        subBoxSizer->Layout();
    }
    // --------------------------------------
    
    // --------------------------------------
    {
        wxPanel* panel_ok_cancel=new wxPanel(this);
        boxSizer->Add(panel_ok_cancel, 0, wxALL, 0);
        wxBoxSizer* subBoxSizer=new wxBoxSizer(wxHORIZONTAL);
        
        wxButton* okbtn=new wxButton(panel_ok_cancel, ID_OK, _("OK"));
        subBoxSizer->Add(okbtn, 0, wxALL, 15);
        
        okbtn->SetDefault();
        
        wxButton* cancelbtn=new wxButton(panel_ok_cancel, ID_CANCEL,  _("Cancel"));
        subBoxSizer->Add(cancelbtn, 0, wxALL, 15);
        
        panel_ok_cancel->SetAutoLayout(TRUE);
        panel_ok_cancel->SetSizer(subBoxSizer);
        subBoxSizer->Layout();
    }
    // --------------------------------------
    
    SetAutoLayout(TRUE);
    SetSizer(boxSizer);
    boxSizer->Layout();
    boxSizer->SetSizeHints(this);
}

// ----------------------------------------------------------------------------------------------------------

void CustomNoteSelectDialog::onFocus(wxFocusEvent& evt)
{
    wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(FindFocus());
    if (ctrl != NULL) ctrl->SetSelection(-1, -1);
}

// ----------------------------------------------------------------------------------------------------------

void CustomNoteSelectDialog::show(Track* currentTrack)
{
    m_current_track = currentTrack;
    
    if (currentTrack->graphics->getEditorMode() == GUITAR)
    {
        cb_string->Enable(true);
        cb_fret->Enable(true);
    }
    else
    {
        cb_string->Enable(false);
        cb_fret->Enable(false);
    }
    returnCode = ShowModal();
}

void CustomNoteSelectDialog::keyPress(wxKeyEvent& evt)
{
    
    if (evt.GetKeyCode() == WXK_RETURN)
    {
        wxCommandEvent unused;
        okClicked(unused);
    }
}

// ----------------------------------------------------------------------------------------------------------

void CustomNoteSelectDialog::okClicked(wxCommandEvent& evt)
{
    
    const bool pitch    = cb_pitch->IsChecked();
    const bool volume   = cb_volume->IsChecked();
    const bool string   = cb_string->IsChecked() and m_current_track->graphics->getEditorMode() == GUITAR;
    const bool fret     = cb_fret->IsChecked()   and m_current_track->graphics->getEditorMode() == GUITAR;
    const bool duration = cb_duration->IsChecked();
    const bool measure  = cb_measure->IsChecked();
    
    wxString from_measure_c = from_measure->GetValue();
    wxString to_measure_c = to_measure->GetValue();
    wxString volume_tolerance_c = volume_tolerance->GetValue();
    
    long from_measure_value;
    long to_measure_value;
    long volume_tolerance_value;
    
    if (
        not from_measure_c.ToLong(&from_measure_value) or
        not to_measure_c.ToLong(&to_measure_value) or
        not volume_tolerance_c.ToLong(&volume_tolerance_value)
        )
    {
        wxMessageBox( _("Illegal values"));
        return;
    }
    
    const int duration_tolerance_value = 0;
    
    
    //const int from_measure_value = atoi( from_measure_c );
    //const int to_measure_value = atoi( to_measure_c );
    //const int volume_tolerance_value = atoi( volume_tolerance_c );
    //const int duration_tolerance_value = 0;
    
    EndModal(returnCode);
    
    // collect information about currently selected notes, to be then used for finding similar notes
    int referenceNoteAmount=0;
    const int noteAmount = m_current_track->getNoteAmount();
    for (int n=0; n<noteAmount; n++)
    {
        if (m_current_track->isNoteSelected(n)) referenceNoteAmount++;
    }
    
    int referencePitches[referenceNoteAmount];
    int referenceVolumes[referenceNoteAmount];
    int referenceStrings[referenceNoteAmount];
    int referenceFrets[referenceNoteAmount];
    int referenceDurations[referenceNoteAmount];
    
    int currentID=0;
    for (int n=0; n<noteAmount; n++)
    {
        if (m_current_track->isNoteSelected(n))
        {
            referencePitches[currentID]   = m_current_track->getNotePitchID(n);
            referenceVolumes[currentID]   = m_current_track->getNoteVolume(n);
            referenceStrings[currentID]   = m_current_track->getNoteString(n);
            referenceFrets[currentID]     = m_current_track->getNoteFret(n);
            referenceDurations[currentID] = m_current_track->getNoteEndInMidiTicks(n) -
                                            m_current_track->getNoteStartInMidiTicks(n);
            currentID++;
        }
    }
    
    m_current_track->selectNote(ALL_NOTES, false, true); // deselect all currently selected notes
    
    // test all notes one by one
    for (int n=0; n<noteAmount; n++)
    {
        
        // test for pitch, if pitch checkbox was checked
        if (pitch)
        {
            bool passTest = false;
            
            const int test_value = m_current_track->getNotePitchID(n);
            for (int i=0; i<referenceNoteAmount; i++)
            {
                if (test_value == referencePitches[i]) passTest=true;
            }
            
            if (not passTest) continue;
        }
        
        // test for string, if string checkbox was checked
        if (string)
        {
            bool passTest = false;
            
            const int test_value = m_current_track->getNoteString(n);
            for(int i=0; i<referenceNoteAmount; i++)
            {
                if (test_value == referenceStrings[i]) passTest=true;
            }
            
            if (not passTest) continue;
        }
        
        // test for fret, if fret checkbox was checked
        if (fret)
        {
            bool passTest = false;
            
            const int test_value = m_current_track->getNoteFret(n);
            for (int i=0; i<referenceNoteAmount; i++)
            {
                if (test_value == referenceFrets[i]) passTest = true;
            }
            
            if (not passTest) continue;
        }
        
        // test for volume, if volume checkbox was checked
        if (volume)
        {
            bool passTest = false;
            
            const int test_value = m_current_track->getNoteVolume(n);
            for (int i=0; i<referenceNoteAmount; i++)
            {
                if ( abs(test_value - referenceVolumes[i]) <= volume_tolerance_value ) passTest=true;
            }
            
            if (not passTest) continue;
        }
        
        // test for duration, if duration checkbox was checked
        if (duration)
        {
            bool passTest = false;
            
            const int test_value = m_current_track->getNoteEndInMidiTicks(n) -
                                   m_current_track->getNoteStartInMidiTicks(n);
            for (int i=0; i<referenceNoteAmount; i++)
            {
                if (abs(test_value - referenceDurations[i]) <= duration_tolerance_value ) passTest=true;
            }
            
            if (not passTest) continue;
        }
        
        // test for measure, if measure checkbox was checked
        if (measure)
        {
            bool passTest = false;
            
            // find in which measure the note is
            const int test_value = getMeasureData()->measureAtTick( m_current_track->getNoteStartInMidiTicks(n) );
            
            if (test_value >= from_measure_value-1 and test_value <= to_measure_value-1) passTest=true;
            
            if (not passTest) continue;
        }
        
        // if the flow reaches this part, it's because all checked tests succeeded. Select the note.
        m_current_track->selectNote(n, true, true);
        
    }//next
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void CustomNoteSelectDialog::cancelClicked(wxCommandEvent& evt)
{
    EndModal(returnCode);
}

// ----------------------------------------------------------------------------------------------------------
