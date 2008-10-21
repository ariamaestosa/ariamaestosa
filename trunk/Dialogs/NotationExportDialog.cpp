
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Config.h"
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
int lineWidth;
//bool repetitionsOf2Measures = false;
//int repetitionWidth;

// ----------------------------------------------------------------------------------------------------
// ------------------------------------------- setup dialog -------------------------------------------
// ----------------------------------------------------------------------------------------------------

class NotationSetup : public wxFrame
{
	wxCheckBox* ignoreHidden;
	wxCheckBox* ignoreMuted;
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
		
		//if(mode==-1)
		//{
			// "ignore hidden tracks" checkbox
            //I18N: - in notation export dialog
			ignoreHidden=new wxCheckBox(this, wxID_ANY,  _("Ignore hidden tracks"));
			ignoreHidden->SetValue(true);
			boxSizer->Add(ignoreHidden, 1, wxALL, 5);
			
			// "ignore muted tracks" checkbox
            //I18N: - in notation export dialog
			ignoreMuted=new wxCheckBox(this, wxID_ANY,  _("Ignore muted tracks"));
			ignoreMuted->SetValue(true);
			boxSizer->Add(ignoreMuted, 1, wxALL, 5);
		//}
		//else
		//{
			ignoreHidden = NULL;
			ignoreMuted = NULL;
		//}
		
		// "Show repeated measures only once" checkbox
        //I18N: - in notation export dialog
		detectRepetitions=new wxCheckBox(this, wxID_ANY,  _("Show repeated measures (e.g. chorus) only once"));
		detectRepetitions->SetValue(true);
		boxSizer->Add(detectRepetitions, 1, wxALL, 5);
		
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
		if(ignoreMuted != NULL)
		{
			ignoreMuted_bool = ignoreMuted->IsChecked();
			ignoreHidden_bool = ignoreHidden->IsChecked();
		}
        else
        {
            ignoreMuted_bool = true;
            ignoreHidden_bool = true;
        }
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
Track* currentTrack;

// ----------------------------------------------------------------------------------------------------
// ------------------------------------- first function called ----------------------------------------
// ----------------------------------------------------------------------------------------------------

// user wants to export to notation - remember what is the sequence, then show set-up dialog
void exportNotation(Sequence* sequence)
{	
	currentSequence = sequence;
	currentTrack = NULL;
	setup = new NotationSetup();
}

void exportNotation(Track* t)
{	
	currentTrack = t;
	currentSequence = NULL;
	setup = new NotationSetup();
}

// ----------------------------------------------------------------------------------------------------
// ---------------------------------------- main writing func -----------------------------------------
// ----------------------------------------------------------------------------------------------------

// after dialog is shown and user clicked 'OK' this is called to complete the export
void completeExport(bool accepted)
{
	if(!accepted) return;
    if(currentSequence == NULL) currentSequence = currentTrack->sequence;

    AriaPrintable notationPrint(currentSequence);
    
	// we want to export the entire song
	if(currentTrack == NULL)
	{

		// iterate through all the the tracks of the sequence, only consider those that are visible and not muted
		const int track_amount = currentSequence->getTrackAmount();
		for(int n=0; n<track_amount; n++)
		{
			Track* track = currentSequence->getTrack(n);
			if( (track->graphics->muted and ignoreMuted_bool) or
				((track->graphics->collapsed or track->graphics->docked) and ignoreHidden_bool)
				)
				// track is disabled, ignore it
				continue;
			
			std::cout << "Generating notation for " << track->getName() << std::endl;
            
            notationPrint.addTrack( track, track->graphics->editorMode );
            
        }// next track
        
        
	}
	// we want to export a single track
	else
	{
        notationPrint.addTrack( currentTrack, currentTrack->graphics->editorMode );        
	}
    
    notationPrint.calculateLayout( checkRepetitions_bool );
    
    if(!printResult(&notationPrint))
        std::cerr << "error while printing" << std::endl;
	
}


}
