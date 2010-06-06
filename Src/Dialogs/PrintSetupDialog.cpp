
#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/file.h"
#include "wx/radiobut.h"
#include "wx/print.h"
#include "wx/renderer.h"
#include "wx/clrpicker.h"
#include "wx/spinctrl.h"
//#include <wx/listctrl.h>

#include "Utils.h"
#include "AriaCore.h"

#include "AriaCore.h"
#include "Dialogs/PrintSetupDialog.h"
#include "Dialogs/WaitWindow.h"
#include "Editors/GuitarEditor.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "IO/IOUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Printing/AriaPrintable.h"
#include "Printing/KeyrollPrintableSequence.h"
#include "Printing/SymbolPrinter/SymbolPrintableSequence.h"

namespace AriaMaestosa
{
    
    class KeyrollPrintOptions : public wxDialog
    {
        wxCheckBox* m_compact_cb;
        wxSpinCtrl* m_size_spinner;
        std::vector<wxColourPickerCtrl*> m_color_pickers;
        bool m_ok_pressed;
        
    public:  
        KeyrollPrintOptions(wxWindow* parent, const std::vector< std::pair<Track*, EditorType> >& whatToPrint) :
                wxDialog(parent, wxID_ANY, _("Keyroll Printing Options"), wxDefaultPosition, wxDefaultSize,
                         wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
        {
            m_ok_pressed = false;
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

            wxBoxSizer* hsizer = new wxBoxSizer(wxHORIZONTAL);
            m_size_spinner = new wxSpinCtrl(this, wxID_ANY, wxT("7"), wxDefaultPosition, wxDefaultSize,
                                            wxSP_ARROW_KEYS, 1, 100);
            hsizer->Add( new wxStaticText(this, wxID_ANY, _("Size of a beat : ")), 0, wxALIGN_CENTER_VERTICAL );
            hsizer->Add( m_size_spinner, 0, wxALIGN_CENTER_VERTICAL ) ;
            hsizer->Add( new wxStaticText(this, wxID_ANY, wxT("mm")), 0, wxALIGN_CENTER_VERTICAL );
            sizer->Add(hsizer, 1, wxALL, 5);
            
            wxStaticBoxSizer* box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Colors"));
            wxFlexGridSizer* colorsSizer = new wxFlexGridSizer(2);
            const int size = whatToPrint.size();
            for (int t=0; t<size; t++)
            {
                colorsSizer->Add(new wxStaticText(this, wxID_ANY, whatToPrint[t].first->getName()), 0,
                                 wxALIGN_CENTER_VERTICAL  | wxALL, 5);
                wxColourPickerCtrl* ctrl = new wxColourPickerCtrl(this, wxID_ANY);
                
                // a few factory colors
                if (t == 1)      ctrl->SetColour(wxColour(0,255,0));
                else if (t == 2) ctrl->SetColour(wxColour(255,0,0));
                else if (t == 3) ctrl->SetColour(wxColour(0,0,255));
                else if (t == 4) ctrl->SetColour(wxColour(255,255,0));
                else if (t == 5) ctrl->SetColour(wxColour(255,0,255));
                else if (t == 6) ctrl->SetColour(wxColour(0,255,255));
                else             ctrl->SetColour(wxColour(150,150,150));
                
                colorsSizer->Add(ctrl);
                m_color_pickers.push_back(ctrl);
            }
            box->Add(colorsSizer);
            sizer->Add(box, 0, wxALL, 10);
            
            //I18N: keyroll printing option
            m_compact_cb = new wxCheckBox(this, wxID_ANY, _("Compact (print only key notes)"));
            sizer->Add(m_compact_cb, 0, wxALL, 5);
            
            wxButton* okBtn = new wxButton(this, wxID_ANY, _("OK"));
            sizer->Add(okBtn, 0 , wxALL, 5);
            okBtn->SetDefault();
            
            SetSizerAndFit(sizer);
            
            okBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(KeyrollPrintOptions::onOk), NULL, this);
        }
        
        void onOk(wxCommandEvent& evt)
        {
            EndModal(GetReturnCode());
            m_ok_pressed = true;
        }
        
        bool okPressed() const
        {
            return m_ok_pressed;
        }
        
        bool compact() const
        {
            return m_compact_cb->IsChecked();
        }
        
        std::vector<wxColour> getColors() const
        {
            std::vector<wxColour> out;
            
            for (int n=0; n<(int)m_color_pickers.size(); n++)
            {
                out.push_back( m_color_pickers[n]->GetColour() );
            }
            
            return out;
        }
        
        int getBeatSize()
        {
            return m_size_spinner->GetValue();
        }
        
    };
    
    /** after dialog is shown, and user clicked 'OK', this is called to launch the actual printing */
    void doPrint(std::vector< std::pair<Track*, EditorType> > whatToPrint, Sequence* seq,
                 AriaPrintable* printable, bool detect_repetitions)
    {
        
        bool symbolPrinter = false;
        bool keyrollPrinter = false;
        
        const unsigned int trackCount = whatToPrint.size();
        for (unsigned int n=0; n<trackCount; n++)
        {
            switch (whatToPrint[n].second)
            {
                case GUITAR:
                case SCORE:
                    symbolPrinter = true;
                    break;
                case KEYBOARD:
                    keyrollPrinter = true;
                    break;
                default:
                    // ignore, it will complain below
                    break;
            }
        }
        
        bool keepGoingOn = true;

        if (symbolPrinter and keyrollPrinter)
        {
            wxMessageBox( _("Keyroll tracks and tablature/score tracks cannot be mixed in the same printout") );
            keepGoingOn = false;
        }
        else if (not symbolPrinter and not keyrollPrinter)
        {
            wxMessageBox( _("No printable track selected") );
            keepGoingOn = false;
        }
        
        OwnerPtr<AbstractPrintableSequence> printableSeq;

        if (keepGoingOn)
        {
            if (symbolPrinter) 
            {
                printableSeq = new SymbolPrintableSequence( seq );
            }
            else if (keyrollPrinter)
            {
                KeyrollPrintOptions dialog(getMainFrame(), whatToPrint);
                dialog.Center();
                dialog.ShowModal();
                if (not dialog.okPressed()) 
                {
                    keepGoingOn = false;
                }
                else
                {
                    printableSeq = new KeyrollPrintableSequence( seq, dialog.getBeatSize()/10.f, dialog.compact(), 
                                                                 dialog.getColors() );
                }
            }
            else
            {
                ASSERT(false);
            }
        }
         
        if (keepGoingOn)
        {
            printable->setSequence(printableSeq);
            WaitWindow::show(_("Calculating print layout...") );
                    
            for (unsigned int n=0; n<trackCount; n++)
            {
                if (not printableSeq->addTrack( whatToPrint[n].first, whatToPrint[n].second ))
                {
                    WaitWindow::hide();
                    
                    wxString track_name = whatToPrint[n].first->getName();
                    
                    //I18N: - %s is the name of the track
                    wxString message = _("Track '%s' could not be printed, since its current\nview (editor) does not support printing.");
                    message.Replace(wxT("%s"), track_name); // wxString::Format crashes, so I need to use this stupid workaround
                    wxMessageBox( message );
                    
                    keepGoingOn = false;
                    break;
                }
            }
        }
        
        if (keepGoingOn)
        {
            std::cout << "********************************************************\n";
            std::cout << "******************* CALCULATE LAYOUT *******************\n";
            std::cout << "********************************************************\n\n";
            
            printableSeq->calculateLayout( detect_repetitions );
            
            std::cout << "\n********************************************************\n";
            std::cout << "********************* PRINT RESULT *********************\n";
            std::cout << "********************************************************\n\n";
            
            WaitWindow::hide();
            
            wxPrinterError result = printable->print();
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
                
        delete printable;
        
        getMainFrame()->disableMenus(false);
    }        

    /** TODO: to be replaced with wxDataViewControl when switching to wxWidgets 3.0 */
    class wxTempList : public wxPanel
    {
        wxFlexGridSizer* m_sizer;
        int m_col_count;
        
        std::vector< std::vector<wxWindow*> > m_rows;
        
    public:
        
        wxTempList(wxWindow* parent, const int columnCount, const wxString colNames[], const bool growable[]) :
            wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER)
        {
            SetBackgroundColour(*wxWHITE);
            m_sizer = new wxFlexGridSizer(columnCount);
            m_col_count = columnCount;
            
            for (int n=0; n<columnCount; n++)
            {
                if (growable[n]) m_sizer->AddGrowableCol(n);
                
                const int h = wxRendererNative::GetDefault().GetHeaderButtonHeight(this);

                wxStaticText* header = new wxStaticText(this, wxID_ANY, wxT(" ") + colNames[n] + wxT("  "));
                header->SetMinSize( wxSize(-1, h) );
                header->SetMaxSize( wxSize(999, h) );
                //header->SetBackgroundColour( wxColour(220, 220, 220) );
                m_sizer->Add(header, 1, wxEXPAND);
            }
        }
        
        void addRowWidgets(wxWindow** widgets)
        {
            std::vector<wxWindow*> row;
            for (int n=0; n<m_col_count; n++)
            {
                m_sizer->Add(widgets[n], 1, wxEXPAND | wxALL, 2);
                row.push_back(widgets[n]);
            }
            m_rows.push_back(row);
        }
        
        void done()
        {
            SetSizer(m_sizer);
            Layout();
        }
        
        std::vector<wxWindow*>& getRow(const int rowId)
        {
            return m_rows[rowId];
        }
        
        void onPaint(wxPaintEvent& evt)
        {
            wxPaintDC dc(this);
            //const int h = wxRendererNative::GetDefault().GetHeaderButtonHeight(this);
            
            wxSizerItemList& items = m_sizer->GetChildren();
            wxwxSizerItemListNode *node = items.GetFirst();
            int count = 0;
            while (node)
            {
                wxSizerItem* win = node->GetData();
                wxPoint position = win->GetPosition();
                wxSize size = win->GetSize();
                wxRendererNative::GetDefault().DrawHeaderButton(this, dc, wxRect(position, size));

                node = node->GetNext();
                count++;
                if (count >= m_col_count) break;
            }
            
            
            //wxRendererNative::GetDefault().DrawHeaderButton(this, dc, wxRect(0, 0, GetSize().GetWidth(), h));
        }
        
        DECLARE_EVENT_TABLE()
    };
    BEGIN_EVENT_TABLE(wxTempList, wxPanel)
    EVT_PAINT(wxTempList::onPaint)
    END_EVENT_TABLE()
    
    /**
      * @ingroup dialogs
      * @brief the dialog to set-up printing of track(s) and page setup
      * @note this is a private class, it won't be instanciated directly
      * @see showPrintSetupDialog
      */
    class PrintSetupDialog : public wxFrame
    {
        Sequence* m_current_sequence;

        bool m_detect_repetitions;
        
        wxRadioButton* m_current_track_radiobtn;
        wxRadioButton* m_visible_tracks_radiobtn;
        
        //wxCheckBox* m_detect_repetitions_checkbox; 
        
        wxTempList* m_track_choice;
        //wxCheckListBox* m_track_choice;
        //wxListCtrl* m_track_choice;
        
        wxCheckBox*    m_hide_empty_tracks; 

        wxStaticText* m_page_setup_summary;
        
        AriaPrintable*     m_printable;
        
    public:
        
        LEAK_CHECK();
        
        PrintSetupDialog(wxWindow *parent, Sequence* sequence) : wxFrame(parent, wxID_ANY,
                                  //I18N: - title of the notation-print dialog
                                  _("Print musical notation"),
                                  wxPoint(200,200), wxSize(500, 400), wxCAPTION | wxFRAME_FLOAT_ON_PARENT)
        {
            m_current_sequence = sequence;
            m_detect_repetitions = false;

            bool success = false;
            m_printable = new AriaPrintable( AbstractPrintableSequence::getTitle(sequence), &success );
            
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
            
            const int colCount = 3;
            const bool growable[colCount] = { false, true, true };
            const wxString colNames[] = { wxString( wxT("Print") ), wxString( wxT("Track Name") ), wxString( wxT("Type") ) };
            m_track_choice = new wxTempList(parent_panel, colCount, colNames, growable);
            
            const int track_amount = m_current_sequence->getTrackAmount();
            for (int n=0; n<track_amount; n++)
            {
                Track* track = m_current_sequence->getTrack(n);
                wxCheckBox* cb = new wxCheckBox(m_track_choice, wxID_ANY, wxT(""));
                cb->SetValue(not (track->graphics->collapsed or track->graphics->muted or track->graphics->docked));

                wxStaticText* trackName = new wxStaticText(m_track_choice, wxID_ANY, track->getName() );
                
                wxArrayString choices;
                //I18N: printing notation type
                choices.Add( _("Score (staff)") );
                //I18N: printing notation type
                choices.Add( _("Tablature") );
                //I18N: printing notation type
                choices.Add( _("Keyroll") );
                wxChoice* editorType = new wxChoice( m_track_choice, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices );
                
                switch (track->graphics->editorMode)
                {
                    case SCORE:
                        editorType->SetSelection(0);
                        break;
                    case GUITAR:
                        editorType->SetSelection(1);
                        break;
                    case KEYBOARD:
                        editorType->SetSelection(2);
                        break;
                    default:
                        // unsupported editor, do nothing, leave default selection
                        break;
                }
                //wxStaticText* editorType = new wxStaticText(m_track_choice, wxID_ANY, track->graphics->getCurrentEditor()->getName() );

                m_track_choice->addRowWidgets( (wxWindow*[]){cb, trackName, editorType} );
                                          
                /*
                const int id = m_track_choice->Append( track->getName() + wxT(" (") +
                                                      track->graphics->getCurrentEditor()->getName() +
                                                      wxT(")") );
                m_track_choice->Check(id, not (track->graphics->collapsed or track->graphics->muted or track->graphics->docked));
                 */
            }
            m_track_choice->done();
            m_track_choice->Enable(false);
            
            /*
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
             */
            
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
            //m_detect_repetitions_checkbox = new wxCheckBox(parent_panel, wxID_ANY,  _("Automatically detect repeated measures (experimental!)"));
            //m_detect_repetitions_checkbox->SetValue(false);
            //boxSizer->Add(m_detect_repetitions_checkbox, 0, wxALL, 5);
            
            // hide empty tracks
            //I18N: in printing dialog
            m_hide_empty_tracks = new wxCheckBox(parent_panel, wxID_ANY,  _("Omit instruments that play nothing on the current line"));
            m_hide_empty_tracks->SetValue(false);
            m_hide_empty_tracks->SetToolTip( _("If some instrument plays nothing during a long part of the song, checking this option can make the printed score tighter by eliminating empty lines") );
            boxSizer->Add(m_hide_empty_tracks, 0, wxALL, 5);
            m_hide_empty_tracks->Enable(false);

            // Page setup summary
            {
                wxStaticBoxSizer* pageSetupSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent_panel, _("Page Setup"));
                
                
                m_page_setup_summary = new wxStaticText(parent_panel, wxID_ANY, m_printable->getPageSetupSummary());
                pageSetupSizer->Add( m_page_setup_summary, 1, wxEXPAND | wxTOP | wxBOTTOM, 5 );                
                    
                {
                    wxPanel* buttonsPanel = new wxPanel(parent_panel);
                    wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);
                    
                    
                    wxButton* pageSetupButton = new wxButton(buttonsPanel, wxNewId(), _("Edit Page Setup"));
                    buttonsSizer->Add(pageSetupButton, 0, wxEXPAND | wxTOP | wxBOTTOM, 5 );
                    pageSetupButton->Connect(pageSetupButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                                             wxCommandEventHandler(PrintSetupDialog::onEditPageSetupClicked),
                                             NULL, this);
                    
#ifdef __WXMAC__ 
                    // the mac page setup dialog does not allow editing margins
                    wxButton* marginsButton = new wxButton(buttonsPanel, wxNewId(), _("Margins"));
                    buttonsSizer->Add(marginsButton, 0, wxEXPAND | wxTOP | wxBOTTOM, 5 );
                    marginsButton->Connect(marginsButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                                           wxCommandEventHandler(PrintSetupDialog::onMacEditMargins),
                                           NULL, this);
#endif
                    
                    buttonsPanel->SetSizer( buttonsSizer );
                    pageSetupSizer->Add( buttonsPanel, 0, wxLEFT | wxRIGHT, 5 );   
                }
                
                boxSizer->Add(pageSetupSizer,  0, wxALL | wxEXPAND, 5);
            }
            
            // OK-Cancel buttons
            {
                wxPanel* buttonPanel = new wxPanel(parent_panel);
                boxSizer->Add(buttonPanel, 0, wxALL | wxEXPAND, 5);
                
                wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
                
                wxButton* okButton = new wxButton(buttonPanel, wxID_OK, _("OK"));
                okButton->SetDefault();
                
                wxButton* cancelButton = new wxButton(buttonPanel, wxID_CANCEL,  _("Cancel"));
                
                buttonsSizer->AddStretchSpacer();
                buttonsSizer->Add(cancelButton, 0, wxALL, 7);
                buttonsSizer->Add(okButton,     0, wxALL, 7);

                buttonPanel->SetSizer(buttonsSizer);
                buttonPanel->SetAutoLayout(true);
                buttonsSizer->Layout();
                
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
            m_printable->showPageSetupDialog();
            m_page_setup_summary->SetLabel(m_printable->getPageSetupSummary());
        }
        
#ifdef __WXMAC__
        void onMacEditMargins(wxCommandEvent& evt)
        {
            Hide();
            m_printable->macEditMargins(this);
            Show();
            m_page_setup_summary->SetLabel(m_printable->getPageSetupSummary());
        }
#endif
        
        void onSelectCurrentTrackOnly(wxCommandEvent& evt)
        {
            m_hide_empty_tracks->SetValue(false);
            m_hide_empty_tracks->Enable(false);
            m_track_choice->Enable(false);
            m_track_choice->Refresh();
        }
        
        void onSelectTrackList(wxCommandEvent& evt)
        {
            m_hide_empty_tracks->SetValue(true);
            m_hide_empty_tracks->Enable(true);
            m_track_choice->Enable(true);
            m_track_choice->Refresh();
        }
        
        /**  called when the 'cancel' button is clicked, or 'escape' is pressed */
        void onCancelClicked(wxCommandEvent& evt)
        {
            delete m_printable;
            
            Hide();
            Destroy();
            getMainFrame()->disableMenus(false);
        }
        
        /** when the 'ok' button is clicked */
        void onOkClicked(wxCommandEvent& evt)
        {
            const bool print_one_track = m_current_track_radiobtn->GetValue();
            
            std::vector< std::pair<Track*, EditorType> > what_to_print;
            if (print_one_track)
            {
                what_to_print.push_back( std::pair<Track*, EditorType>(m_current_sequence->getCurrentTrack(),
                                                                       m_current_sequence->getCurrentTrack()->graphics->editorMode) );
            }
            else                            // print all selected from list
            {
                const int track_amount = m_current_sequence->getTrackAmount();
                for (int n=0; n<track_amount; n++)
                {
                    wxCheckBox* cb = dynamic_cast<wxCheckBox*>(m_track_choice->getRow(n)[0]);
                    ASSERT(cb != NULL);
                    
                    if (cb->IsChecked())
                    {
                        const wxChoice* editorChoice = dynamic_cast<wxChoice*>(m_track_choice->getRow(n)[2]);
                        ASSERT(editorChoice != NULL);
                        
                        EditorType printType;
                        if (editorChoice->GetSelection() == 0)
                        {
                            printType = SCORE;
                        }
                        else if (editorChoice->GetSelection() == 1)
                        {
                            printType = GUITAR;
                        }
                        else if (editorChoice->GetSelection() == 2)
                        {
                            printType = KEYBOARD;
                        }
                        else
                        {
                            assert(false);
                            continue;
                        }
                        
                        what_to_print.push_back( std::pair<Track*, EditorType>(m_current_sequence->getTrack(n),
                                                                               printType) );
                    }
                }
            }
            
            m_detect_repetitions = false; //m_detect_repetitions_checkbox->IsChecked();
            
            m_printable->hideEmptyTracks( (print_one_track ? false : m_hide_empty_tracks->IsChecked()) );
            m_printable->showTrackNames( not print_one_track );
            
            // terminate the dialog
            Hide();
            Destroy();
            
            // continue with the printing sequence
            doPrint(what_to_print, m_current_sequence, m_printable, m_detect_repetitions);
        }
        
        
    };
        
    // ----------------------------------------------------------------------------------------------------
    // ------------------------------------- first function called ----------------------------------------
    // ----------------------------------------------------------------------------------------------------
    
    // user wants to export to notation - remember what is the sequence, then show set-up dialog
    void showPrintSetupDialog(Sequence* sequence)
    {
        MainFrame* mainFrame = getMainFrame();
        mainFrame->disableMenus(true);
        new PrintSetupDialog(mainFrame, sequence);
    }

}
