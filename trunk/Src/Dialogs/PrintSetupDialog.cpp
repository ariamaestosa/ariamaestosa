
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>
#include <wx/radiobut.h>
#include <wx/print.h>

#include "Config.h"
#include "AriaCore.h"

#include "AriaCore.h"
#include "Dialogs/PrintSetupDialog.h"
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
    
    
    void doPrint(std::vector<Track*> what_to_export);
    
    bool ignoreMuted_bool = false;
    bool ignoreHidden_bool = false;
    bool checkRepetitions_bool = false;
    
    int lineWidth;
    Sequence* currentSequence;

    //bool repetitionsOf2Measures = false;
    //int repetitionWidth;
    
    // ----------------------------------------------------------------------------------------------------
    // ------------------------------------------- setup dialog -------------------------------------------
    // ----------------------------------------------------------------------------------------------------
    
    class NotationSetup : public wxFrame
    {
        //wxCheckBox* ignoreHidden;
        //wxCheckBox* ignoreMuted;
        
        wxRadioButton* current_track;
        wxRadioButton* visible_tracks;
        
        wxCheckBox* detectRepetitions;
        
        wxPanel* buttonPanel;
        wxButton* okButton;
        wxButton* cancelButton;
        
        wxBoxSizer* boxSizer;
        
        wxCheckListBox* m_track_choice;
        
        // wxTextCtrl* lineWidthCtrl;
        // wxCheckBox* repMinWidth;
        
        
    public:
        
        LEAK_CHECK();
        
        NotationSetup() : wxFrame(NULL, wxID_ANY,
                                  //I18N: - title of the notation-print dialog
                                  _("Print musical notation"),
                                  wxPoint(200,200), wxSize(200,400), wxCAPTION | wxSTAY_ON_TOP)
        {
            
            wxPanel* parent_panel = new wxPanel(this);
            
            boxSizer = new wxBoxSizer(wxVERTICAL);
            
            //I18N: - in print setup dialog. 
            wxStaticBoxSizer* subsizer = new wxStaticBoxSizer(wxVERTICAL, parent_panel, _("Print..."));
            
            //I18N: - in print setup dialog. 
            current_track = new wxRadioButton(parent_panel, wxNewId(), _("Current track only") , wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            current_track->Connect( current_track->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                                    wxCommandEventHandler(NotationSetup::onSelectCurrentTrackOnly), NULL, this );
            
            //I18N: - in print setup dialog. 
            visible_tracks = new wxRadioButton(parent_panel, wxNewId(), _("This list of tracks"));
            visible_tracks->Connect( visible_tracks->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED,
                                     wxCommandEventHandler(NotationSetup::onSelectTrackList), NULL, this );
            
            m_track_choice = new wxCheckListBox(parent_panel, wxID_ANY);
            const int track_amount = currentSequence->getTrackAmount();
            for (int n=0; n<track_amount; n++)
            {
                Track* track = currentSequence->getTrack(n);
                const int id = m_track_choice->Append( track->getName() );
                m_track_choice->Check(id, not (track->graphics->collapsed or track->graphics->muted or track->graphics->docked));
            }
            m_track_choice->Enable(false);

            subsizer->Add(current_track, 0, wxALL, 5); current_track->SetValue(true);
            subsizer->Add(visible_tracks, 0, wxALL, 5);
            subsizer->Add(m_track_choice, 1, wxALL | wxEXPAND, 5);
            
            boxSizer->Add(subsizer, 1, wxALL | wxEXPAND, 5);
            
            // "Show repeated measures only once" checkbox
            //I18N: - in notation export dialog
            detectRepetitions=new wxCheckBox(parent_panel, wxID_ANY,  _("Automatically detect repeated measures (experimental!)"));
            detectRepetitions->SetValue(false);
            boxSizer->Add(detectRepetitions, 0, wxALL, 5);
            
            /*
             wxSize textCtrlSize(wxDefaultSize); textCtrlSize.SetWidth(55);
             
             repMinWidth=new wxCheckBox(this, wxID_ANY,  _("Repetitions must be at least 2 measures long"));
             repMinWidth->SetValue(true);
             boxSizer->Add(repMinWidth, 1, wxALL, 5);
             */
            /*
             // repetition minimal width
             {
             wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
             boxSizer->Add(subsizer);
             
             subsizer->Add(new wxStaticText(this, wxID_ANY,  _("Repetitions must be at least")), 1, wxALL, 5);
             repMinWidth = new wxTextCtrl(this, wxID_ANY, wxT("2"), wxDefaultPosition, textCtrlSize);
             subsizer->Add(repMinWidth, 0, wxALL, 5);
             subsizer->Add(new wxStaticText(this, wxID_ANY,  _("measures long.")), 1, wxALL, 5);
             }
             */
            
            // Line width
            /*
             {
             wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
             boxSizer->Add(subsizer);
             
             subsizer->Add(new wxStaticText(this, wxID_ANY,  _("Maximal number of characters per line")), 1, wxALL, 5);
             
             lineWidthCtrl = new wxTextCtrl(this, wxID_ANY, wxT("100"), wxDefaultPosition, textCtrlSize);
             subsizer->Add(lineWidthCtrl, 0, wxALL, 5);
             
             }*/
            
            // OK-Cancel buttons
            {
                buttonPanel = new wxPanel(parent_panel);
                boxSizer->Add(buttonPanel, 0, wxALL | wxEXPAND, 5);
                
                wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
                
                okButton = new wxButton(buttonPanel, wxID_OK, _("OK"));
                okButton->SetDefault();
                
                cancelButton = new wxButton(buttonPanel, wxID_CANCEL,  _("Cancel"));
                
                subsizer->AddStretchSpacer();
                subsizer->Add(cancelButton, 0, wxALL, 7);
                subsizer->Add(okButton,     0, wxALL, 7);

                buttonPanel->SetSizer(subsizer);
                buttonPanel->SetAutoLayout(true);
                subsizer->Layout();
            }
            
            parent_panel->SetSizer(boxSizer);
            boxSizer->Layout();
            boxSizer->SetSizeHints(parent_panel);
            Fit();
            
            Center();
            Show();
            
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
        
        void cancelClicked(wxCommandEvent& evt)
        {
            Hide();
            Destroy();
        }
        
        void okClicked(wxCommandEvent& evt)
        {
            std::vector<Track*> what_to_print;
            if (current_track->GetValue())  // only print selected track
            {
                what_to_print.push_back( currentSequence->getCurrentTrack() );
            }
            else                            // print all selected from list
            {
                const int track_amount = currentSequence->getTrackAmount();
                for (int n=0; n<track_amount; n++)
                {
                    if (m_track_choice->IsChecked(n))
                    {
                        what_to_print.push_back( currentSequence->getTrack(n) );
                    }
                }
            }
            
            checkRepetitions_bool = detectRepetitions->IsChecked();
            //lineWidth = atoi_u( lineWidthCtrl->GetValue() );
            //repetitionsOf2Measures = repMinWidth->IsChecked();
            
            // terminate the dialog
            Hide();
            Destroy();
            
            // continue with the printing sequence
            doPrint(what_to_print);
        }
        
        DECLARE_EVENT_TABLE();
        
    };
    
    BEGIN_EVENT_TABLE(NotationSetup, wxFrame)
    
    EVT_BUTTON(wxID_OK, NotationSetup::okClicked)
    EVT_BUTTON(wxID_CANCEL, NotationSetup::cancelClicked)
    
    END_EVENT_TABLE()
    
    static NotationSetup* setup;
    
    // ----------------------------------------------------------------------------------------------------
    // ------------------------------------- first function called ----------------------------------------
    // ----------------------------------------------------------------------------------------------------
    
    // user wants to export to notation - remember what is the sequence, then show set-up dialog
    void showPrintSetupDialog(Sequence* sequence)
    {
        currentSequence = sequence;
        //currentTrack = NULL;
        setup = new NotationSetup();
    }
    /*
     void exportNotation(Track* t)
     {
     currentTrack = t;
     currentSequence = NULL;
     setup = new NotationSetup();
     }
     */
    // ----------------------------------------------------------------------------------------------------
    // ---------------------------------------- main writing func -----------------------------------------
    // ----------------------------------------------------------------------------------------------------
    
    // after dialog is shown and user clicked 'OK' this is called to complete the export
    void doPrint(std::vector<Track*> what_to_print)
    {        
        PrintableSequence notationPrint(currentSequence);
        
        for (unsigned int n=0; n<what_to_print.size(); n++)
        {
            if (not notationPrint.addTrack( what_to_print[n], what_to_print[n]->graphics->editorMode ))
            {
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
        
        notationPrint.calculateLayout( checkRepetitions_bool );
        
        std::cout << "\n********************************************************\n";
        std::cout << "********************* PRINT RESULT *********************\n";
        std::cout << "********************************************************\n\n";
        
        AriaPrintable printer( &notationPrint );
        
        wxPrinterError result = (wxPrinterError)printer.print();
        if (result == wxPRINTER_ERROR)
        {
            std::cerr << "error while printing" << std::endl;
            wxMessageBox( _("An error occured during printing.") );
        }
        
    }
    
    
}
