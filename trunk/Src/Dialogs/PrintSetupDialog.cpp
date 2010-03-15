
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>
#include <wx/radiobut.h>
#include <wx/print.h>
//#include <wx/listctrl.h>

#include "Config.h"
#include "AriaCore.h"

#include "AriaCore.h"
#include "Dialogs/PrintSetupDialog.h"
#include "Dialogs/WaitWindow.h"
#include "Editors/GuitarEditor.h"
#include "GUI/GraphicalTrack.h"
#include "IO/IOUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Printing/TabPrint.h"
#include "Printing/AriaPrintable.h"
#include "Printing/PrintableSequence.h"

namespace AriaMaestosa
{
    
    class PrintSetupDialog : public wxFrame
    {
        Sequence* m_current_sequence;

        bool m_detect_repetitions;
        
        wxRadioButton* m_current_track_radiobtn;
        wxRadioButton* m_visible_tracks_radiobtn;
        
        //wxCheckBox* m_detect_repetitions_checkbox; 
        
        wxCheckListBox* m_track_choice;
        //wxListCtrl* m_track_choice;
        
        wxStaticText* m_page_setup_summary;
        
        OwnerPtr<PrintableSequence> m_printable_sequence;
        OwnerPtr<AriaPrintable>     m_printable;
        
    public:
        
        LEAK_CHECK();
        
        PrintSetupDialog(Sequence* sequence) : wxFrame(NULL, wxID_ANY,
                                  //I18N: - title of the notation-print dialog
                                  _("Print musical notation"),
                                  wxPoint(200,200), wxSize(500, 400), wxCAPTION | wxSTAY_ON_TOP)
        {
            m_printable_sequence = new PrintableSequence(sequence);
            m_current_sequence = sequence;
            m_detect_repetitions = false;

            bool success = false;
            m_printable = new AriaPrintable( m_printable_sequence, &success );
            
            if (not success)
            {                
                std::cerr << "error while performing page setup : " << __FILE__ << ":" << __LINE__ << std::endl;
                wxMessageBox( _("An error occurred while preparing to print.") );
                return;
            }
            
            // --- Setup dialog
            SetMinSize( wxSize(500, 400) );
            wxPanel* parent_panel = new wxPanel(this);
            
            wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
            
            //I18N: - in print setup dialog. 
            wxStaticBoxSizer* subsizer = new wxStaticBoxSizer(wxVERTICAL, parent_panel, _("Print..."));
            
            //I18N: - in print setup dialog. 
            m_current_track_radiobtn = new wxRadioButton(parent_panel, wxNewId(), _("Current track only") , wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            m_current_track_radiobtn->Connect( m_current_track_radiobtn->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                                    wxCommandEventHandler(PrintSetupDialog::onSelectCurrentTrackOnly), NULL, this );
            
            //I18N: - in print setup dialog. 
            m_visible_tracks_radiobtn = new wxRadioButton(parent_panel, wxNewId(), _("This list of tracks"));
            m_visible_tracks_radiobtn->Connect( m_visible_tracks_radiobtn->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                                     wxCommandEventHandler(PrintSetupDialog::onSelectTrackList), NULL, this );
            
            m_track_choice = new wxCheckListBox(parent_panel, wxID_ANY);
            const int track_amount = m_current_sequence->getTrackAmount();
            for (int n=0; n<track_amount; n++)
            {
                Track* track = m_current_sequence->getTrack(n);
                const int id = m_track_choice->Append( track->getName() + wxT(" (") +
                                                       track->graphics->getCurrentEditor()->getName() +
                                                       wxT(")") );
                m_track_choice->Check(id, not (track->graphics->collapsed or track->graphics->muted or track->graphics->docked));
            }
            m_track_choice->Enable(false);
            
            /*
            m_track_choice = new wxListCtrl(parent_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
            
            wxListItem col0;
            col0.SetId(0);
            //I18N: In the print dialog, name of the column containing the checkboxes where you choose if one track is printed
            col0.SetText( _("Print") );
            col0.SetWidth(50);
            m_track_choice->InsertColumn(0, col0);
            
            wxListItem col1;
            col1.SetId(1);
            col1.SetText( _("Track Name") );
            m_track_choice->InsertColumn(1, col1);
            
            wxListItem col2;
            col2.SetId(2);
            //I18N: In print dialog, will be score, tablature, etc...
            col2.SetText( _("Notation Type") );
            m_track_choice->InsertColumn(2, col2);
            
            
            const int track_amount = m_current_sequence->getTrackAmount();
            for (int n=0; n<track_amount; n++)
            {
                Track* track = m_current_sequence->getTrack(n);
                
                wxListItem item;
                item.SetId(n);
                item.SetText( track->getName() );
                
                m_track_choice->InsertItem( item );
                
                if (track->graphics->collapsed or track->graphics->muted or track->graphics->docked)
                {
                    m_track_choice->SetItem(n, 0, wxT("[ ]"));
                }
                else
                {
                    m_track_choice->SetItem(n, 0, wxT("[√]"));
                }
                m_track_choice->SetItem(n, 1, track->getName());
                m_track_choice->SetItem(n, 2, track->graphics->getCurrentEditor()->getName());
                
                //m_track_choice->Check(id, not (track->graphics->collapsed or track->graphics->muted or track->graphics->docked));
            }
            m_track_choice->Enable(false);
            */
            
            subsizer->Add(m_current_track_radiobtn, 0, wxALL, 5); m_current_track_radiobtn->SetValue(true);
            subsizer->Add(m_visible_tracks_radiobtn, 0, wxALL, 5);
            subsizer->Add(m_track_choice, 1, wxALL | wxEXPAND, 5);
            
            boxSizer->Add(subsizer, 1, wxALL | wxEXPAND, 5);
            
            // "Show repeated measures only once" checkbox
            //I18N: - in notation export dialog
            //m_detect_repetitions_checkbox = new wxCheckBox(parent_panel, wxID_ANY,  _("Automatically detect repeated measures (experimental!)"));
            //m_detect_repetitions_checkbox->SetValue(false);
            //boxSizer->Add(m_detect_repetitions_checkbox, 0, wxALL, 5);
            
            // Page setup summary
            {
                wxStaticBoxSizer* pageSetupSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent_panel, _("Page Setup"));
                
                m_page_setup_summary = new wxStaticText(parent_panel, wxID_ANY, m_printable->getPageSetupSummary());
                pageSetupSizer->Add( m_page_setup_summary, 1, wxEXPAND | wxTOP | wxBOTTOM, 5 );                
                
                wxButton* pageSetupButton = new wxButton(parent_panel, wxNewId(), _("Edit Page Setup"));
                pageSetupSizer->Add( pageSetupButton, 0, wxTOP | wxBOTTOM, 5 );                
                pageSetupButton->Connect(pageSetupButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                                         wxCommandEventHandler(PrintSetupDialog::onEditPageSetupClicked), NULL, this);
                
                boxSizer->Add(pageSetupSizer,  0, wxALL | wxEXPAND, 5);
            }
            
            // OK-Cancel buttons
            {
                wxPanel* buttonPanel = new wxPanel(parent_panel);
                boxSizer->Add(buttonPanel, 0, wxALL | wxEXPAND, 5);
                
                wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
                
                wxButton* okButton = new wxButton(buttonPanel, wxID_OK, _("OK"));
                okButton->SetDefault();
                
                wxButton* cancelButton = new wxButton(buttonPanel, wxID_CANCEL,  _("Cancel"));
                
                subsizer->AddStretchSpacer();
                subsizer->Add(cancelButton, 0, wxALL, 7);
                subsizer->Add(okButton,     0, wxALL, 7);

                buttonPanel->SetSizer(subsizer);
                buttonPanel->SetAutoLayout(true);
                subsizer->Layout();
                
                Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED,
                                         wxCommandEventHandler(PrintSetupDialog::onOkClicked), NULL, this);
                Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED,
                                         wxCommandEventHandler(PrintSetupDialog::onCancelClicked), NULL, this);
                
            }
            
            parent_panel->SetSizer(boxSizer);
            boxSizer->Layout();
            boxSizer->SetSizeHints(parent_panel);
            
            Fit();
            Center();
            Show();
            
        }
        
        void onEditPageSetupClicked(wxCommandEvent& evt)
        {
            //TODO
        }
        
        void onSelectCurrentTrackOnly(wxCommandEvent& evt)
        {
            m_track_choice->Enable(false);
            m_track_choice->Refresh();
        }
        
        void onSelectTrackList(wxCommandEvent& evt)
        {
            m_track_choice->Enable(true);
            m_track_choice->Refresh();
        }
        
        /**  called when the 'cancel' button is clicked, or 'escape' is pressed */
        void onCancelClicked(wxCommandEvent& evt)
        {
            Hide();
            Destroy();
        }
        
        /** when the 'ok' button is clicked */
        void onOkClicked(wxCommandEvent& evt)
        {
            std::vector<Track*> what_to_print;
            if (m_current_track_radiobtn->GetValue())  // only print selected track
            {
                what_to_print.push_back( m_current_sequence->getCurrentTrack() );
            }
            else                            // print all selected from list
            {
                const int track_amount = m_current_sequence->getTrackAmount();
                for (int n=0; n<track_amount; n++)
                {
                    if (m_track_choice->IsChecked(n))
                    {
                        what_to_print.push_back( m_current_sequence->getTrack(n) );
                    }
                }
            }
            
            m_detect_repetitions = false; //m_detect_repetitions_checkbox->IsChecked();
            
            // terminate the dialog
            Hide();
            Destroy();
            
            // continue with the printing sequence
            doPrint(what_to_print);
        }
        
        /** after dialog is shown, and user clicked 'OK', this is called to launch the actual printing */
        void doPrint(std::vector<Track*> what_to_print)
        {        
            WaitWindow::show(_("Calculating print layout...") );
            
            for (unsigned int n=0; n<what_to_print.size(); n++)
            {
                if (not m_printable_sequence->addTrack( what_to_print[n], what_to_print[n]->graphics->editorMode ))
                {
                    WaitWindow::hide();
                    
                    wxString track_name = what_to_print[n]->getName();
                    
                    //I18N: - %s is the name of the track
                    wxString message = _("Track '%s' could not be printed, since its current\nview (editor) does not support printing.");
                    message.Replace(wxT("%s"), track_name); // wxString::Format crashes, so I need to use this stupid workaround
                    wxMessageBox( message );
                    return;
                }
            }
            
            std::cout << "********************************************************\n";
            std::cout << "******************* CALCULATE LAYOUT *******************\n";
            std::cout << "********************************************************\n\n";
            
            m_printable_sequence->calculateLayout( m_detect_repetitions );
            
            std::cout << "\n********************************************************\n";
            std::cout << "********************* PRINT RESULT *********************\n";
            std::cout << "********************************************************\n\n";
            
            WaitWindow::hide();
            
            wxPrinterError result = m_printable->print();
            if (result == wxPRINTER_ERROR)
            {
                std::cerr << "error while printing : " << __FILE__ << ":" << __LINE__ << std::endl;
                wxMessageBox( _("An error occurred during printing.") );
            }
            else if (result == wxPRINTER_CANCELLED)
            {
                std::cerr << "Printing was cancelled\n";
            }
            
        }        
        
        
    };
        
    // ----------------------------------------------------------------------------------------------------
    // ------------------------------------- first function called ----------------------------------------
    // ----------------------------------------------------------------------------------------------------
    
    // user wants to export to notation - remember what is the sequence, then show set-up dialog
    void showPrintSetupDialog(Sequence* sequence)
    {
        new PrintSetupDialog(sequence);
    }

}
