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

#ifndef _mainframe_
#define _mainframe_

#include "wx/wx.h"
#include "wx/scrolbar.h"
#include "wx/image.h"
#include "wx/spinctrl.h"

#include "wxAdditions/bsizer.h"

#include "Config.h"
#include "AriaCore.h"
#include "ptr_vector.h"

namespace AriaMaestosa {

class CustomNoteSelectDialog;
class Sequence;
class Preferences;
class AboutDialog;
class InstrumentChoice;
class DrumChoice;
class VolumeSlider;
class TuningPicker;
class KeyPicker;

// events useful if you need to show a
// progress bar from another thread
DECLARE_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW, -1)
DECLARE_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW, -1)
DECLARE_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW, -1)

#define MAKE_SHOW_PROGRESSBAR_EVENT(eventname, message, time_known) wxCommandEvent eventname( wxEVT_SHOW_WAIT_WINDOW, 100001 ); eventname.SetString(message); eventname.SetInt(time_known)
#define MAKE_UPDATE_PROGRESSBAR_EVENT(eventname, progress) wxCommandEvent eventname( wxEVT_UPDATE_WAIT_WINDOW, 100002 ); eventname.SetInt(progress)
#define MAKE_HIDE_PROGRESSBAR_EVENT(eventname) wxCommandEvent eventname( wxEVT_HIDE_WAIT_WINDOW, 100003 )

#if wxMAJOR_VERSION != 3
#define NO_WX_TOOLBAR   
#endif
    
class MainFrame : public wxFrame
{
	WxOwnerPtr<AboutDialog>  aboutDialog;
    WxOwnerPtr<CustomNoteSelectDialog>  customNoteSelectDialog;
    WxOwnerPtr<Preferences>  prefs;
    
    wxBorderSizer* verticalSizer;
#ifdef NO_WX_TOOLBAR
    wxPanel* topPane;
    wxFlexGridSizer* toolbarSizer;
#else
    wxToolBar* toolbar;
#endif
    wxScrollBar* horizontalScrollbar;
    wxScrollBar* verticalScrollbar;

    wxTextCtrl* measureTypeBottom;
    wxTextCtrl* measureTypeTop;
    wxTextCtrl* firstMeasure;
    wxSpinCtrl* songLength;
    wxSpinCtrl* displayZoom;

    wxMenu* fileMenu;
    wxMenu* editMenu;
	wxMenu* settingsMenu;
    wxMenu* trackMenu;
	wxMenu* helpMenu;

	wxMenuItem* followPlaybackMenuItem;

	wxMenuItem* playDuringEdits_always;
	wxMenuItem* playDuringEdits_onchange;
	wxMenuItem* playDuringEdits_never;

	wxMenuItem* channelManagement_automatic;
	wxMenuItem* channelManagement_manual;
	wxMenuItem* expandedMeasuresMenuItem;

	int currentSequence;
    ptr_vector<Sequence> sequences; // contains all open sequences


    wxBitmapButton* play;
    wxBitmapButton* stop;

public:
    LEAK_CHECK(MainFrame);
    
	// READ AND WRITE
	bool changingValues; // set this to true when modifying the controls in the top bar, this allows to ignore all events thrown by their modification.
	wxTextCtrl* tempoCtrl;

    // ------- read-only -------
	int play_during_edit; // what is the user's preference for note preview during edits
	bool playback_mode;
    MainPane* mainPane;
	OwnerPtr<InstrumentChoice>  instrument_picker;
	OwnerPtr<DrumChoice>        drumKit_picker;
	OwnerPtr<TuningPicker>      tuningPicker;
	OwnerPtr<KeyPicker>         keyPicker;
    // ----------------------

    MainFrame();
	void init();
    void initMenuBar();
    ~MainFrame();

    // top bar controls updated
    void tempoChanged(wxCommandEvent& evt);
    void songLengthChanged(wxSpinEvent& evt);
    void zoomChanged(wxSpinEvent& evt);
    void songLengthTextChanged(wxCommandEvent& evt);
    void zoomTextChanged(wxCommandEvent& evt);
    void measureNumChanged(wxCommandEvent& evt);
    void measureDenomChanged(wxCommandEvent& evt);
    void firstMeasureChanged(wxCommandEvent& evt);
	void changeMeasureAmount(int i, bool throwEvent=true);
    void disableMenusForPlayback(const bool disable);
    
    void enterPressedInTopBar(wxCommandEvent& evt);
    
    // wait window events
    void evt_showWaitWindow(wxCommandEvent& evt);
    void evt_updateWaitWindow(wxCommandEvent& evt);
    void evt_hideWaitWindow(wxCommandEvent& evt);

	// menus
    void on_close(wxCloseEvent& evt);
    void menuEvent_new(wxCommandEvent& evt);
    void menuEvent_close(wxCommandEvent& evt);
	void menuEvent_exportNotation(wxCommandEvent& evt);
    void menuEvent_open(wxCommandEvent& evt);
    void menuEvent_copy(wxCommandEvent& evt);
    void menuEvent_undo(wxCommandEvent& evt);
    void menuEvent_paste(wxCommandEvent& evt);
    void menuEvent_pasteAtMouse(wxCommandEvent& evt);
    void menuEvent_save(wxCommandEvent& evt);
    void menuEvent_saveas(wxCommandEvent& evt);
    void menuEvent_selectNone(wxCommandEvent& evt);
    void menuEvent_selectAll(wxCommandEvent& evt);
    void menuEvent_addTrack(wxCommandEvent& evt);
    void menuEvent_deleteTrack(wxCommandEvent& evt);
    void menuEvent_trackBackground(wxCommandEvent& evt);
    void menuEvent_importmidi(wxCommandEvent& evt);
    void menuEvent_exportmidi(wxCommandEvent& evt);
    void menuEvent_exportSampledAudio(wxCommandEvent& evt);
    void menuEvent_customNoteSelect(wxCommandEvent& evt);
    void menuEvent_snapToGrid(wxCommandEvent& evt);
    void menuEvent_scale(wxCommandEvent& evt);
    void menuEvent_copyright(wxCommandEvent& evt);
	void menuEvent_preferences(wxCommandEvent& evt);
	void menuEvent_followPlayback(wxCommandEvent& evt);
	void menuEvent_removeOverlapping(wxCommandEvent& evt);
	void menuEvent_playAlways(wxCommandEvent& evt);
	void menuEvent_playOnChange(wxCommandEvent& evt);
	void menuEvent_playNever(wxCommandEvent& evt);
    void menuEvent_quit(wxCommandEvent& evt);
	void menuEvent_about(wxCommandEvent& evt);
    void menuEvent_manual(wxCommandEvent& evt);
	void menuEvent_automaticChannelModeSelected(wxCommandEvent& evt);
	void menuEvent_manualChannelModeSelected(wxCommandEvent& evt);
	void menuEvent_expandedMeasuresSelected(wxCommandEvent& evt);

    void updateMenuBarToSequence();
    
    // playback
    void songHasFinishedPlaying();
    void toolsEnterPlaybackMode();
    void toolsExitPlaybackMode();
	void playClicked(wxCommandEvent& evt);
    void stopClicked(wxCommandEvent& evt);

	// i/o
	void updateTopBarAndScrollbarsForSequence(Sequence* seq);
	void loadAriaFile(wxString path);
	void loadMidiFile(wxString path);

	// scrollbars
    void updateVerticalScrollbar();
    void updateHorizontalScrollbar(int thumbPos=-1);
	void horizontalScrolling(wxScrollEvent& evt);
    void verticalScrolling(wxScrollEvent& evt);
    void horizontalScrolling_arrows(wxScrollEvent& evt);
    void verticalScrolling_arrows(wxScrollEvent& evt);

	// sequences
	void addSequence();
    int getSequenceAmount();
    bool closeSequence(int id=-1); // -1 means current sequence
    Sequence* getCurrentSequence();
    Sequence* getSequence(int n);
	int getCurrentSequenceID();
	void setCurrentSequence(int n);

	void changeShownTimeSig(int num, int denom);

	void evt_freeVolumeSlider( wxCommandEvent& evt );

    DECLARE_EVENT_TABLE();
};

}

#endif
