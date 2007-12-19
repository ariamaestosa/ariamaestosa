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

#ifndef _volumeslider_
#define _volumeslider_

#include "wx/wx.h"
#include "wx/slider.h"
#include "wx/event.h"

#include "Config.h"

namespace AriaMaestosa {
	
class Note; // forward
class Track;
/*
class wxDestroyVolumeSliderEvent : public wxNotifyEvent
{
public:
	wxDestroyVolumeSliderEvent( wxEventType commandType = wxEVT_NULL, int id = 0 );
	
	// required for sending with wxPostEvent()
	wxEvent* Clone();
};

DECLARE_EVENT_MACRO( wxEVT_DESTROY_VOLUME_SLIDER, -1 )

typedef void (wxEvtHandler::*wxDestroyVolumeSliderEventFunction)(wxDestroyVolumeSliderEvent&);

#define EVT_DESTROY_VOLUME_SLIDER(id, fn) \
DECLARE_EVENT_TABLE_ENTRY( wxEVT_DESTROY_VOLUME_SLIDER, id, -1, \
						   (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
						   wxStaticCastEvent( wxDestroyVolumeSliderEventFunction, & fn ), (wxObject *) NULL ),


// code implementing the event type and the event class

DEFINE_EVENT_TYPE( wxEVT_DESTROY_VOLUME_SLIDER )
*/

DECLARE_EVENT_TYPE(wxEVT_DESTROY_VOLUME_SLIDER, -1)

void showVolumeSlider(int x, int y, int noteID, Track* track);
void freeVolumeSlider();

class VolumeSlider : public wxDialog
{
	
	DECLARE_LEAK_CHECK();
	
    wxSlider* slider;
    wxTextCtrl* valueText;
    int returnCode;
    
    int noteID;
    Track* currentTrack;
	
public:
    VolumeSlider();
    
    void show(int x, int y, int noteID, Track* track);
    void closeWindow();
	
    void volumeSlideChanging(wxScrollEvent& evt);
    void volumeSlideChanged(wxScrollEvent& evt);
    void volumeTextChanged(wxCommandEvent& evt);
    void enterPressed(wxCommandEvent& evt);

	void closed(wxCloseEvent& evt);
    void keyPress(wxKeyEvent& evt);
        
    DECLARE_EVENT_TABLE();
};

}

#endif
