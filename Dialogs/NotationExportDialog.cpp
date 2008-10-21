
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>
#include <wx/radiobut.h>

#include "Config.h"
#include "AriaCore.h"

#include "AriaCore.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Editors/GuitarEditor.h"
#include "IO/IOUtils.h"
#include "Printing/TabPrint.h"
#include "Printing/PrintingBase.h"
#include "Dialogs/NotationExportDialog.h"

namespace AriaMaestosa
{
	
	enum IDS
{
	ID_OK,
	ID_CANCEL
};

void completeExport(bool accepted);
void exportTablature(Track* t, wxFile* file);

bool ignoreMuted_bool = false;
bool ignoreHidden_bool = false;
bool checkRepetitions_bool = false;

bool only_selected_track_bool = false;
int lineWidth;
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
	
	wxTextCtrl* lineWidthCtrl;
	
	wxCheckBox* repMinWidth;
	

public:
		
    LEAK_CHECK(NotationSetup);
	
	NotationSetup() : wxFrame(NULL, wxID_ANY,
                                           //I18N: - title of the notation-export dialog
                                           _("Export to musical notation"),
                                           wxPoint(200,200), wxSize(200,400), wxCAPTION | wxSTAY_ON_TOP)
    {
		
		
		boxSizer=new wxBoxSizer(wxVERTICAL);
		
        //I18N: - in notation export dialog. user can choose to print 'Current track' or 'Visible/enabled tracks'
        wxStaticBoxSizer* subsizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Print..."));
        //I18N: - in notation export dialog. user can choose to print 'Current track' or 'Visible/enabled tracks'
        current_track = new wxRadioButton(this, wxID_ANY, _("Current track") , wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
        //I18N: - in notation export dialog. user can choose to print 'Current track' or 'Visible/enabled tracks'
        visible_tracks = new wxRadioButton(this, wxID_ANY, _("Visible/enabled tracks"));
        
        subsizer->Add(current_track, 1, wxALL, 5); current_track->SetValue(true);
        subsizer->Add(visible_tracks, 1, wxALL, 5);
        boxSizer->Add(subsizer, 1, wxALL, 5);
        
		// "Show repeated measures only once" checkbox
        //I18N: - in notation export dialog
		detectRepetitions=new wxCheckBox(this, wxID_ANY,  _("Show repeated measures (e.g. chorus) only once"));
		detectRepetitions->SetValue(true);
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
			buttonPanel = new wxPanel(this);
			boxSizer->Add(buttonPanel, 0, wxALL, 0);
			
			wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
			
			okButton=new wxButton(buttonPanel, ID_OK, _("OK"));
			okButton->SetDefault();
			subsizer->Add(okButton, 0, wxALL, 15);
			
			cancelButton=new wxButton(buttonPanel, ID_CANCEL,  _("Cancel"));
			subsizer->Add(cancelButton, 0, wxALL, 15);
			
			buttonPanel->SetSizer(subsizer);
			buttonPanel->SetAutoLayout(true);
			subsizer->Layout();
		}
		
		SetAutoLayout(true);
		SetSizer(boxSizer);
		boxSizer->Layout();
		boxSizer->SetSizeHints(this);
		
		Center();
		Show();

    }
	
	void cancelClicked(wxCommandEvent& evt)
	{
		Hide();
        Destroy();
		completeExport(false);
	}
	
	void okClicked(wxCommandEvent& evt)
	{
        only_selected_track_bool = current_track->GetValue();
		checkRepetitions_bool = detectRepetitions->IsChecked();
		//lineWidth = atoi_u( lineWidthCtrl->GetValue() );
		//repetitionsOf2Measures = repMinWidth->IsChecked();
		Hide();
        Destroy();
		completeExport(true);
	}
	
	DECLARE_EVENT_TABLE();
	
};

BEGIN_EVENT_TABLE(NotationSetup, wxFrame)

EVT_BUTTON(ID_OK, NotationSetup::okClicked)
EVT_BUTTON(ID_CANCEL, NotationSetup::cancelClicked)

END_EVENT_TABLE()

static NotationSetup* setup;
Sequence* currentSequence;

// ----------------------------------------------------------------------------------------------------
// ------------------------------------- first function called ----------------------------------------
// ----------------------------------------------------------------------------------------------------

// user wants to export to notation - remember what is the sequence, then show set-up dialog
void exportNotation(Sequence* sequence)
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
void completeExport(bool accepted)
{
	if(!accepted) return;
   // if(currentSequence == NULL) currentSequence = currentTrack->sequence;

    AriaPrintable notationPrint(currentSequence);
    
    // check if we print everything or just one track
	if(only_selected_track_bool)
	{
        notationPrint.addTrack( getCurrentSequence()->getCurrentTrack(), getCurrentSequence()->getCurrentTrack()->graphics->editorMode );
    }
    else
    {
		// iterate through all the the tracks of the sequence, only consider those that are visible and not muted

        const int track_amount = getCurrentSequence()->getTrackAmount();
		for(int n=0; n<track_amount; n++)
		{
			Track* track = currentSequence->getTrack(n);
            
            // ignore disabled or hidden tracks
            if(track->graphics->muted     or
               track->graphics->collapsed or
               track->graphics->docked) continue;
			
			std::cout << "Generating notation for track " << n << " : " << track->getName() << std::endl;
            
            notationPrint.addTrack( track, track->graphics->editorMode );
            
        }// next track
        
        
	}
    
    notationPrint.calculateLayout( checkRepetitions_bool );
    
    if(!printResult(&notationPrint))
        std::cerr << "error while printing" << std::endl;
	
}


}
