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

#include "Pickers/BackgroundPicker.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "AriaCore.h"
#include "GUI/GraphicalTrack.h"
#include "Editors/Editor.h"

#include "wx/tokenzr.h"

#include "AriaCore.h"

#include <iostream>

namespace AriaMaestosa {

/*
 * In Custom guitar tuning editor, this is a single string
 */
class BackgroundChoicePanel : public wxPanel
{
	DECLARE_LEAK_CHECK();
	
public:
	wxBoxSizer* sizer;
	wxCheckBox* active;

	BackgroundChoicePanel(wxWindow* parent, const int trackID) : wxPanel(parent)
    {
	
		INIT_LEAK_CHECK();
		
		sizer = new wxBoxSizer(wxHORIZONTAL);
		
		// checkbox
		active = new wxCheckBox(this, 200, wxT(" "));
		sizer->Add(active, 0, wxALL, 5);
		
		sizer->Add( new wxStaticText(this, wxID_ANY, to_wxString(trackID) + wxT(" : ") + getCurrentSequence()->getTrack(trackID)->getName()) , 1, wxALL, 5);

		SetSizer(sizer);
		sizer->Layout();
		sizer->SetSizeHints(this); // resize to take ideal space
    }
	
    bool isChecked()
    {
        return active->GetValue();
    }
	
};


/*
 * This is the rame that lets you enter a custom tuning.
 */

class BackgroundPickerFrame : public wxDialog
{
	
	DECLARE_LEAK_CHECK();
	
	wxPanel* buttonPane;
	wxButton* ok_btn;
	wxButton* cancel_btn;
	wxBoxSizer* buttonsizer;
	
	wxBoxSizer* sizer;
	
    ptr_vector<BackgroundChoicePanel> choicePanels;
    
	Track* parent;
	
    int modalid;
    
public:
		
	~BackgroundPickerFrame()
	{
	}
		
	BackgroundPickerFrame(Track* parent) :
		wxDialog(NULL, wxID_ANY,  _("Track Background"), wxPoint(100,100), wxSize(500,300), wxCAPTION )
	{
		INIT_LEAK_CHECK();
			
        BackgroundPickerFrame::parent = parent;
            
		sizer = new wxBoxSizer(wxVERTICAL);
		
        const int trackAmount = getCurrentSequence()->getTrackAmount();
		for(int n=0; n<trackAmount; n++)
		{
            BackgroundChoicePanel* bcp = new BackgroundChoicePanel(this, n);
			sizer->Add(bcp, 0, wxALL, 5);
            choicePanels.push_back(bcp);
		}
		
		buttonPane = new wxPanel(this);
		sizer->Add(buttonPane, 0, wxALL, 5);
		
		buttonsizer = new wxBoxSizer(wxHORIZONTAL);
		
		ok_btn = new wxButton(buttonPane, 200, wxT("OK"));
		ok_btn->SetDefault();
		buttonsizer->Add(ok_btn, 0, wxALL, 5);
		
		cancel_btn = new wxButton(buttonPane, 202,  _("Cancel"));
		buttonsizer->Add(cancel_btn, 0, wxALL, 5);
				 
		buttonPane->SetSizer(buttonsizer);
		
		SetAutoLayout(true);
		SetSizer(sizer);
		sizer->Layout();
		sizer->SetSizeHints(this); // resize window to take ideal space
        // FIXME - if too many tracks for current screen space, may cause problems
	}
	
	void show()
	{
		Center();
		modalid = ShowModal();
	}
	
	// when Cancel button of the tuning picker is pressed
	void cancelButton(wxCommandEvent& evt)
	{
		wxDialog::EndModal(modalid);
	}
	
	// when OK button of the tuning picker is pressed
	void okButton(wxCommandEvent& evt)
	{
        const int amount = choicePanels.size();
        Editor* editor = parent->graphics->getCurrentEditor();
        Sequence* seq = getCurrentSequence();
        
        for(int n=0; n<amount; n++)
        {
            if(choicePanels[n].isChecked())
            {
                editor->addBackgroundTrack( seq->getTrack(n) );
                std::cout << "Adding track " << n << " as background to track " << std::endl;
            }
        }
        
        wxDialog::EndModal(modalid);
        Display::render();
    }
	
DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BackgroundPickerFrame, wxDialog)
EVT_BUTTON(200, BackgroundPickerFrame::okButton)
EVT_BUTTON(202, BackgroundPickerFrame::cancelButton)
END_EVENT_TABLE()
		
		

namespace BackgroundPicker{


    void show(Track* parent)
    {
        BackgroundPickerFrame frame(parent);
        frame.show();
    }
        
}


}
