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
 51 Franklin Street, Fifth Floor, Boston, MA 0MENU_EDIT_FOLLOW_PLAYBACK0-1301 USA.
 */

#include "wx/utils.h"
#include "wx/stdpaths.h"
#include "wx/html/htmlwin.h"

#include "main.h"

#include "Actions/EditAction.h"
#include "Actions/RemoveOverlapping.h"

#include "GUI/MainFrame.h"
#include "GUI/GLPane.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MeasureBar.h"

#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"

#include "Editors/KeyboardEditor.h"

#include "Images/ImageProvider.h"

#include "Dialogs/CustomNoteSelectDialog.h"
#include "Dialogs/WaitWindow.h"
#include "Dialogs/ScalePicker.h"
#include "Dialogs/CopyrightWindow.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/About.h"

#include "Pickers/InstrumentChoice.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/VolumeSlider.h"
#include "Pickers/TuningPicker.h"
#include "Pickers/KeyPicker.h"

#include "IO/IOUtils.h"
#include "IO/AriaFileWriter.h"
#include "IO/MidiFileReader.h"
#include "IO/NotationExport.h"

#include "Config.h"
#include <iostream>

namespace AriaMaestosa {


enum IDs
{
	PLAY_CLICKED,
	STOP_CLICKED,
	TEMPO,
	ZOOM,
	LENGTH,
	BEGINNING,

	SCROLLBAR_H,
	SCROLLBAR_V,

	MEASURE_NUM,
	MEASURE_DENOM,

	MENU_FILE_NEW,
	MENU_FILE_OPEN,
	MENU_FILE_SAVE,
	MENU_FILE_SAVE_AS,
	MENU_FILE_IMPORT_MIDI,
	MENU_FILE_EXPORT_MIDI,
	MENU_FILE_EXPORT_SAMPLED_AUDIO,
	MENU_FILE_EXPORT_NOTATION,
	MENU_FILE_CLOSE,
	MENU_FILE_COPYRIGHT,

	MENU_EDIT_COPY,
	MENU_EDIT_PASTE,
	MENU_EDIT_SELECT_ALL,
	MENU_EDIT_SELECT_NONE,
	MENU_EDIT_SELECT_CUSTOM,
	MENU_EDIT_PASTE_AT_CURSOR,
	MENU_EDIT_SNAP_TO_GRID,
	MENU_EDIT_SCALE,
	MENU_EDIT_REMOVE_OVERLAPPING,
	MENU_EDIT_UNDO,

	MENU_SETTINGS_FOLLOW_PLAYBACK,
	MENU_SETTINGS_PLAY_ALWAYS,
	MENU_SETTINGS_PLAY_NEVER,
	MENU_SETTINGS_MEASURE_EXPANDED,
	MENU_SETTINGS_PLAY_ON_CHANGE,
	MENU_SETTINGS_CHANNELS_AUTO,
	MENU_SETTINGS_CHANNEL_MANUAL,

	MENU_TRACK_ADD,
	MENU_TRACK_REMOVE
};


// events useful if you need to show a
// progress bar from another thread
DEFINE_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW)
DEFINE_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW)
DEFINE_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW)

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

EVT_SET_FOCUS(MainFrame::onFocus)

/* scrollbar */
EVT_COMMAND_SCROLL_THUMBRELEASE(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_THUMBTRACK(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_PAGEUP(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_PAGEDOWN(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_LINEUP(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)
EVT_COMMAND_SCROLL_LINEDOWN(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)

EVT_COMMAND_SCROLL_THUMBRELEASE(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_THUMBTRACK(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_PAGEUP(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_PAGEDOWN(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_LINEUP(SCROLLBAR_V, MainFrame::verticalScrolling_arrows)
EVT_COMMAND_SCROLL_LINEDOWN(SCROLLBAR_V, MainFrame::verticalScrolling_arrows)

/* top bar */
EVT_BUTTON(PLAY_CLICKED, MainFrame::playClicked)
EVT_BUTTON(STOP_CLICKED, MainFrame::stopClicked)
EVT_TEXT(TEMPO, MainFrame::tempoChanged)
EVT_TEXT(MEASURE_NUM, MainFrame::measureNumChanged)
EVT_TEXT(MEASURE_DENOM, MainFrame::measureDenomChanged)
EVT_TEXT(BEGINNING, MainFrame::firstMeasureChanged)

EVT_SPINCTRL(LENGTH, MainFrame::songLengthChanged)
EVT_SPINCTRL(ZOOM, MainFrame::zoomChanged)

EVT_TEXT(LENGTH, MainFrame::songLengthTextChanged)
EVT_TEXT(ZOOM, MainFrame::zoomTextChanged)

EVT_MENU_CLOSE(MainFrame::menuClosed)

/* file menu */
EVT_MENU(MENU_FILE_NEW, MainFrame::menuEvent_new)
EVT_MENU(MENU_FILE_OPEN, MainFrame::menuEvent_open)
EVT_MENU(MENU_FILE_SAVE, MainFrame::menuEvent_save)
EVT_MENU(MENU_FILE_SAVE_AS, MainFrame::menuEvent_saveas)
EVT_MENU(MENU_FILE_IMPORT_MIDI, MainFrame::menuEvent_importmidi)
EVT_MENU(MENU_FILE_EXPORT_MIDI, MainFrame::menuEvent_exportmidi)
EVT_MENU(MENU_FILE_EXPORT_SAMPLED_AUDIO, MainFrame::menuEvent_exportSampledAudio)
EVT_MENU(MENU_FILE_CLOSE, MainFrame::menuEvent_close)
EVT_MENU(MENU_FILE_EXPORT_NOTATION, MainFrame::menuEvent_exportNotation)
EVT_MENU(MENU_FILE_COPYRIGHT, MainFrame::menuEvent_copyright)

/* edit menu */
EVT_MENU(MENU_EDIT_COPY, MainFrame::menuEvent_copy)
EVT_MENU(MENU_EDIT_PASTE, MainFrame::menuEvent_paste)
EVT_MENU(MENU_EDIT_SELECT_ALL, MainFrame::menuEvent_selectAll)
EVT_MENU(MENU_EDIT_SELECT_NONE, MainFrame::menuEvent_selectNone)
EVT_MENU(MENU_EDIT_PASTE_AT_CURSOR, MainFrame::menuEvent_pasteAtMouse)
EVT_MENU(MENU_EDIT_SELECT_CUSTOM, MainFrame::menuEvent_customNoteSelect)
EVT_MENU(MENU_EDIT_SNAP_TO_GRID, MainFrame::menuEvent_snapToGrid)
EVT_MENU(MENU_EDIT_UNDO, MainFrame::menuEvent_undo)
EVT_MENU(MENU_EDIT_SCALE, MainFrame::menuEvent_scale)
EVT_MENU(MENU_EDIT_REMOVE_OVERLAPPING, MainFrame::menuEvent_removeOverlapping)

/* settings menu */
EVT_MENU(MENU_SETTINGS_FOLLOW_PLAYBACK, MainFrame::menuEvent_followPlayback)
EVT_MENU(MENU_SETTINGS_PLAY_ALWAYS, MainFrame::menuEvent_playAlways)
EVT_MENU(MENU_SETTINGS_PLAY_ON_CHANGE, MainFrame::menuEvent_playOnChange)
EVT_MENU(MENU_SETTINGS_PLAY_NEVER, MainFrame::menuEvent_playNever)
EVT_MENU(MENU_SETTINGS_CHANNELS_AUTO, MainFrame::menuEvent_automaticChannelModeSelected)
EVT_MENU(MENU_SETTINGS_CHANNEL_MANUAL, MainFrame::menuEvent_manualChannelModeSelected)
EVT_MENU(MENU_SETTINGS_MEASURE_EXPANDED, MainFrame::menuEvent_expandedMeasuresSelected)

EVT_MENU(wxID_PREFERENCES, MainFrame::menuEvent_preferences)
EVT_MENU(wxID_EXIT, MainFrame::menuEvent_quit)

/* Track menu */
EVT_MENU(MENU_TRACK_ADD, MainFrame::menuEvent_addTrack)
EVT_MENU(MENU_TRACK_REMOVE, MainFrame::menuEvent_deleteTrack)

/* Help menu */
EVT_MENU(wxID_ABOUT, MainFrame::menuEvent_about)
EVT_MENU(wxID_HELP, MainFrame::menuEvent_manual)

EVT_COMMAND  (100000, wxEVT_DESTROY_VOLUME_SLIDER, MainFrame::evt_freeVolumeSlider)

// events useful if you need to show a
// progress bar from another thread
EVT_COMMAND  (100001, wxEVT_SHOW_WAIT_WINDOW, MainFrame::evt_showWaitWindow)
EVT_COMMAND  (100002, wxEVT_UPDATE_WAIT_WINDOW, MainFrame::evt_updateWaitWindow)
EVT_COMMAND  (100003, wxEVT_HIDE_WAIT_WINDOW, MainFrame::evt_hideWaitWindow)

END_EVENT_TABLE()

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxT("Aria Maestosa"), wxPoint(100,100), wxSize(800,600),
								 wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION)
{
	INIT_LEAK_CHECK();

}

// this needs to be seperate from the constructor to ensure thet getMainFrame() does not return null
void MainFrame::init()
{
    Centre();

	prefs = NULL;
	currentSequence=0;
	playback_mode=false;
	play_during_edit = PLAY_ON_CHANGE;

    customNoteSelectDialog = new CustomNoteSelectDialog();
    changingValues=false;

	SetMinSize(wxSize(670, 330));

    // ------------------------- menus ---------------------

    wxMenuBar* menuBar=new wxMenuBar();

	// File menu
    fileMenu = new wxMenu();
    menuBar->Append(fileMenu,  _("File") );
    fileMenu->Append(MENU_FILE_NEW,  _("New\tCtrl-N") );
    fileMenu->Append(MENU_FILE_OPEN,  _("Open\tCtrl-O") );
    fileMenu->Append(MENU_FILE_SAVE,  _("Save\tCtrl-S") );
    fileMenu->Append(MENU_FILE_SAVE_AS,  _("Save As\tCtrl-Shift-S") );
    fileMenu->Append(MENU_FILE_CLOSE,  _("Close\tCtrl-W") );

    fileMenu->AppendSeparator();
    fileMenu->Append(MENU_FILE_COPYRIGHT,   _("Song info") );
	fileMenu->AppendSeparator();

    fileMenu->Append(MENU_FILE_IMPORT_MIDI,  _("Import Midi File") );
    fileMenu->Append(MENU_FILE_EXPORT_MIDI,  _("Export to Midi") );
    fileMenu->Append(MENU_FILE_EXPORT_SAMPLED_AUDIO,  _("Export to Audio") );
	//fileMenu->Append(MENU_FILE_EXPORT_NOTATION,  _("Export notation") );

    fileMenu->Append(wxID_EXIT,  _("Quit\tCtrl-Q") );

	// Edit menu
    editMenu = new wxMenu();
    menuBar->Append(editMenu,  _("Edit"));
    editMenu->Append(MENU_EDIT_UNDO,  _("Undo\tCtrl-Z"));
    editMenu->AppendSeparator();
    editMenu->Append(MENU_EDIT_COPY,  _("Copy\tCtrl-C"));
    editMenu->Append(MENU_EDIT_PASTE,  _("Paste\tCtrl-V"));
    editMenu->Append(MENU_EDIT_PASTE_AT_CURSOR,  _("Paste at cursor\tCtrl-Shift-V"));
    editMenu->AppendSeparator(); // ----- selection
    editMenu->Append(MENU_EDIT_SELECT_ALL,  _("Select All\tCtrl-A"));
    editMenu->Append(MENU_EDIT_SELECT_NONE,  _("Select None\tCtrl-Shift-A"));
    editMenu->Append(MENU_EDIT_SELECT_CUSTOM,  _("Select Notes...\tCtrl-F"));
    editMenu->AppendSeparator(); // ----- actions
    editMenu->Append(MENU_EDIT_SNAP_TO_GRID,  _("Snap Notes to Grid"));
    editMenu->Append(MENU_EDIT_SCALE,  _("Scale"));
	editMenu->Append(MENU_EDIT_REMOVE_OVERLAPPING,  _("Remove Overlapping Notes"));

	// Tracks menu
    trackMenu = new wxMenu();
    menuBar->Append(trackMenu,  _("Tracks"));
    trackMenu->Append(MENU_TRACK_ADD,  _("Add Track"));
    trackMenu->Append(MENU_TRACK_REMOVE,  _("Delete Track"));

	// Settings menu
	settingsMenu = new wxMenu();
	menuBar->Append(settingsMenu,  _("Settings"));
	followPlaybackMenuItem = settingsMenu->AppendCheckItem(MENU_SETTINGS_FOLLOW_PLAYBACK,   _("Follow Playback") );
	expandedMeasuresMenuItem = settingsMenu->AppendCheckItem(MENU_SETTINGS_MEASURE_EXPANDED,   _("Expanded measure management") );

	wxMenu* playDuringEdits_menu = new wxMenu();
	settingsMenu->AppendSubMenu(playDuringEdits_menu,  _("Play during edit") );
	playDuringEdits_always = playDuringEdits_menu->AppendCheckItem(MENU_SETTINGS_PLAY_ALWAYS,  _("Always"));
	playDuringEdits_onchange = playDuringEdits_menu->AppendCheckItem(MENU_SETTINGS_PLAY_ON_CHANGE,  _("On note change"));
	playDuringEdits_onchange->Check();
	playDuringEdits_never = playDuringEdits_menu->AppendCheckItem(MENU_SETTINGS_PLAY_NEVER,  _("Never"));

	wxMenu* channelMode_menu = new wxMenu();
	settingsMenu->AppendSubMenu(channelMode_menu,  _("Channel management") );
	channelManagement_automatic = channelMode_menu->AppendCheckItem(MENU_SETTINGS_CHANNELS_AUTO,  _("Automatic"));
	channelManagement_automatic->Check();
	channelManagement_manual = channelMode_menu->AppendCheckItem(MENU_SETTINGS_CHANNEL_MANUAL,  _("Manual"));

    settingsMenu->Append( wxID_PREFERENCES,   _("Preferences") );

	// Help menu
	helpMenu = new wxMenu();
    //menuBar->Append(helpMenu,  _("Documentation"));
	menuBar->Append(helpMenu, wxT("&Help"));
    helpMenu->Append(wxID_ABOUT,  _("About this app"));
    helpMenu->Append(wxID_HELP,  _("Manual"));

    SetMenuBar(menuBar);

    // -------------------------- Top Pane ----------------------------
    verticalSizer = new wxBorderSizer();

    topPane=new wxPanel(this);
    boxSizer=new wxBoxSizer(wxHORIZONTAL);

    // ---------------- play/stop buttons -------------------
    wxInitAllImageHandlers();

	wxBitmap playBitmap;
	playBitmap.LoadFile( getResourcePrefix()  + wxT("play.png") , wxBITMAP_TYPE_PNG);
	play=new wxBitmapButton(topPane, PLAY_CLICKED, playBitmap);

	boxSizer->Add(play, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxBitmap stopBitmap;
	stopBitmap.LoadFile( getResourcePrefix()  + wxT("stop.png") , wxBITMAP_TYPE_PNG);
	stop=new wxBitmapButton(topPane, STOP_CLICKED, stopBitmap);

    boxSizer->Add(stop, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    stop->Enable(false);

    wxSize averageTextCtrlSize(wxDefaultSize);
    averageTextCtrlSize.SetWidth(55);

    wxSize smallTextCtrlSize(wxDefaultSize);
    smallTextCtrlSize.SetWidth(35);

    wxSize tinyTextCtrlSize(wxDefaultSize);
    tinyTextCtrlSize.SetWidth(25);

    // ---------------------- tempo ---------------
    boxSizer->Add(new wxStaticText(topPane, wxID_ANY,  _("Tempo: ")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    tempoCtrl=new wxTextCtrl(topPane, TEMPO, wxT("120"), wxDefaultPosition, smallTextCtrlSize ); // ID 2

    boxSizer->Add(tempoCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topPane->SetSizer(boxSizer);

    verticalSizer->Add(topPane, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2, Location::North() );

    // ---------------------- song length ---------------
    boxSizer->Add(new wxStaticText(topPane, wxID_ANY,  _("Duration: ")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    songLength=new wxSpinCtrl(topPane, LENGTH, to_wxString(DEFAULT_SONG_LENGTH), wxDefaultPosition,  // ID 3
#ifdef __WXGTK__
							  averageTextCtrlSize
#else
							  wxDefaultSize
#endif
							  );

    boxSizer->Add(songLength, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    songLength->SetRange(0, 10000);

    // ---------------------- measures ---------------
    boxSizer->Add(new wxStaticText(topPane, wxID_ANY,  _("Measure: ")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    measureTypeTop=new wxTextCtrl(topPane, MEASURE_NUM, wxT("4"), wxDefaultPosition, tinyTextCtrlSize ); // ID 4
    boxSizer->Add(measureTypeTop, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    boxSizer->Add(new wxStaticText(topPane, wxID_ANY, wxT("/")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    measureTypeBottom=new wxTextCtrl(topPane, MEASURE_DENOM, wxT("4"), wxDefaultPosition, tinyTextCtrlSize ); // ID 5
    boxSizer->Add(measureTypeBottom, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    // ---------------------- song beginning ---------------
    boxSizer->Add(new wxStaticText(topPane, wxID_ANY,  _("Start: ")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    firstMeasure=new wxTextCtrl(topPane, BEGINNING, wxT("1"), wxDefaultPosition, smallTextCtrlSize); // ID 6
    boxSizer->Add(firstMeasure, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    topPane->SetSizer(boxSizer);

    // ---------------------- zoom ---------------
    boxSizer->Add(new wxStaticText(topPane, wxID_ANY,  _("Zoom: ")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    displayZoom=new wxSpinCtrl(topPane, ZOOM, wxT("100"), wxDefaultPosition, // ID 7
#ifdef __WXGTK__
							   averageTextCtrlSize
#else
							   wxDefaultSize
#endif
							   );

    boxSizer->Add(displayZoom, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    displayZoom->SetRange(0, 500);

    // -------------------------- GL Pane ----------------------------
    int args[3];
    args[0]=WX_GL_RGBA;
    args[1]=WX_GL_DOUBLEBUFFER;
    args[2]=0;
    glPane=new GLPane(this, args);
    verticalSizer->Add(glPane, 0, wxALL, 2, Location::Center() );


    // -------------------------- Horizontal Scrollbar ----------------------------

    {
        wxPanel* panel_hscrollbar=new wxPanel(this);
        verticalSizer->Add(panel_hscrollbar,  0, wxALL, 0, Location::South() );
        wxBorderSizer* subSizer=new wxBorderSizer();

        horizontalScrollbar=new wxScrollBar(panel_hscrollbar, SCROLLBAR_H);

        // For the first time, set scrollbar manually and not using updateHorizontalScrollbar(), because this method assumes the frame is visible.
        // FIXME - make more methods detect frame visibiltiy (?) right now it causes many problems to init everything in the right order
        const int editor_size=695, total_size=12*128;

        horizontalScrollbar->SetScrollbar(
                                          horizontalScrollbar->GetThumbPosition(),
                                          editor_size,
                                          total_size,
                                          1
                                          );

        subSizer->Add(horizontalScrollbar, 0, wxALL, 0, Location::Center() );

        wxStaticText* lbl_more_or_less = new wxStaticText(panel_hscrollbar, wxID_ANY, wxT(" __"));
        subSizer->Add(lbl_more_or_less, 0, wxALL, 0, Location::East() );

        panel_hscrollbar->SetAutoLayout(TRUE);
        panel_hscrollbar->SetSizer(subSizer);
        subSizer->Layout();
    }


    // -------------------------- Vertical Scrollbar ----------------------------
    verticalScrollbar=new wxScrollBar(this, SCROLLBAR_V, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

    verticalScrollbar->SetScrollbar(
                                    0 /*position*/,
                                    530 /*viewable height / thumb size*/,
                                    530 /*height*/,
                                    5 /*scroll amount*/
                                    );

verticalSizer->Add(verticalScrollbar, 0, wxALL, 0, Location::East() );


    // -------------------------- finish ----------------------------

    SetAutoLayout(TRUE);
    SetSizer(verticalSizer);

    verticalSizer->Layout();

    Show();

	tuningPicker = new TuningPicker();
	keyPicker = new KeyPicker();
	prefs=new Preferences(this);
	aboutDialog = new AboutDialog();
	instrument_picker=new InstrumentChoice();
	drumKit_picker=new DrumChoice();
	//volumeSlider=new VolumeSlider();

	glPane->isNowVisible();

    glPane->setCurrent();
    glPane->initOpenGLFor2D();

	ImageProvider::loadImages();




#ifdef _show_dialog_on_startup
	aboutDialog->show();
#endif
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ MENU EVENTS -----------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_new(wxCommandEvent& evt)
{
    addSequence();
}

void MainFrame::menuEvent_close(wxCommandEvent& evt)
{
    closeSequence();
}

void MainFrame::menuEvent_customNoteSelect(wxCommandEvent& evt)
{
    customNoteSelectDialog->show( getCurrentSequence()->getCurrentTrack() );
}

void MainFrame::menuEvent_copy(wxCommandEvent& evt)
{
    getCurrentSequence()->copy();
}

void MainFrame::menuEvent_exportNotation(wxCommandEvent& evt)
{
    exportNotation( getCurrentSequence() );
}

void MainFrame::menuEvent_paste(wxCommandEvent& evt)
{
    getCurrentSequence()->paste();
}

void MainFrame::menuEvent_pasteAtMouse(wxCommandEvent& evt)
{
    getCurrentSequence()->pasteAtMouse();
}

void MainFrame::menuEvent_save(wxCommandEvent& evt)
{
    if(getCurrentSequence()->filepath.IsEmpty()) menuEvent_saveas(evt);
    else saveAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);
}

void MainFrame::menuEvent_snapToGrid(wxCommandEvent& evt)
{
    getCurrentSequence()->snapNotesToGrid();
}

void MainFrame::menuEvent_saveas(wxCommandEvent& evt)
{
	wxString suggestedName = getCurrentSequence()->suggestFileName() + wxT(".aria");
	
	getCurrentSequence()->filepath = showFileDialog( _("Select destination file"), wxT(""), suggestedName,
											  wxT("Aria Maestosa file|*.aria"), true /*save*/);

    if(!getCurrentSequence()->filepath.IsEmpty())
	{


		if( wxFileExists(getCurrentSequence()->filepath) )
		{
			int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
									  wxYES_NO, this);
			if (answer != wxYES) return;
		}

        saveAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);

        // change song name
        getCurrentSequence()->sequenceFileName = getCurrentSequence()->filepath.AfterLast('/').BeforeLast('.');
        glPane->render();

    }// end if
}

void MainFrame::menuEvent_open(wxCommandEvent& evt)
{
	wxString filePath = showFileDialog( _("Select file"), wxT(""), wxT(""),  _("Aria Maestosa file|*.aria"), false /*open*/);
	MainFrame::loadAriaFile(filePath);
}


void MainFrame::menuEvent_importmidi(wxCommandEvent& evt)
{
	wxString midiFilePath = showFileDialog( _("Select midi file"), wxT(""), wxT(""),  _("Midi file|*.mid;*.midi"), false /*open*/);
	MainFrame::loadMidiFile(midiFilePath);
}

void MainFrame::menuEvent_exportmidi(wxCommandEvent& evt)
{
	wxString suggestedName = getCurrentSequence()->suggestFileName() + wxT(".mid");
	
	// show file dialog
	wxString midiFilePath = showFileDialog( _("Select destination file"), wxT(""),
											suggestedName, _("Midi file|*.mid"), true /*save*/);

    if(midiFilePath.IsEmpty()) return;
	
	// if file already exists, ask for overwriting
	if( wxFileExists(midiFilePath) )
	{
		int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
								  wxYES_NO, this);
		if (answer != wxYES) return;
	}

	// write data to file
	const bool success = PlatformMidiManager::exportMidiFile( getCurrentSequence(), midiFilePath );

	if(!success)
	{
		wxMessageBox( _("Sorry, failed to export midi file."));
	}
}

void MainFrame::menuEvent_exportSampledAudio(wxCommandEvent& evt)
{

	wxString extension = PlatformMidiManager::getAudioExtension();
	wxString wildcard = PlatformMidiManager::getAudioWildcard();

	wxString suggestedName = getCurrentSequence()->suggestFileName() + extension;
	
	// show file dialog
	wxString audioFilePath = showFileDialog(  _("Select destination file"), wxT(""),
											  suggestedName,
											  wildcard, true /*save*/);

    if(audioFilePath.IsEmpty()) return;

	// if file already exists, ask for overwriting
	if( wxFileExists(audioFilePath) )
	{
		int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
								   wxYES_NO, this);
		if (answer != wxYES) return;
	}

	// write data
    //WaitWindow::show( _("Please wait while audio file is being generated.\n\nDepending on the length of your file,\nthis can take several minutes.") );
	//wxYield(); // FIXME - find a better way

    MAKE_SHOW_PROGRESSBAR_EVENT( event, _("Please wait while audio file is being generated.\n\nDepending on the length of your file,\nthis can take several minutes."), false );
    GetEventHandler()->AddPendingEvent(event);

	std::cout << "export audio file " << toCString(audioFilePath) << std::endl;
	PlatformMidiManager::exportAudioFile( getCurrentSequence(), audioFilePath );
}

void MainFrame::menuEvent_undo(wxCommandEvent& evt)
{
    getCurrentSequence()->undo();
}

void MainFrame::menuEvent_selectNone(wxCommandEvent& evt)
{
    getCurrentSequence()->selectNone();
}

void MainFrame::menuEvent_selectAll(wxCommandEvent& evt)
{
    getCurrentSequence()->selectAll();
}

void MainFrame::menuEvent_addTrack(wxCommandEvent& evt)
{
    getCurrentSequence()->addTrack();
    updateVerticalScrollbar();
}

void MainFrame::menuEvent_deleteTrack(wxCommandEvent& evt)
{

    int answer = wxMessageBox(  _("Do you really want to delete this track?"),  _("Confirm"),
                              wxYES_NO, this);

    if (answer == wxYES)
	{
        getCurrentSequence()->deleteTrack();
        updateVerticalScrollbar();
    }

}

void MainFrame::menuEvent_scale(wxCommandEvent& evt)
{
    ScalePicker::pickScale( getCurrentSequence() );
}

void MainFrame::menuEvent_copyright(wxCommandEvent& evt)
{
    CopyrightWindow::show( getCurrentSequence() );
}

void MainFrame::menuEvent_preferences(wxCommandEvent& evt)
{
	prefs->show();
}

void MainFrame::menuEvent_followPlayback(wxCommandEvent& evt)
{
    getCurrentSequence()->follow_playback = followPlaybackMenuItem->IsChecked();
}

void MainFrame::menuEvent_playAlways(wxCommandEvent& evt)
{
	playDuringEdits_always->Check(true);
	playDuringEdits_onchange->Check(false);
	playDuringEdits_never->Check(false);
	play_during_edit = PLAY_ALWAYS;
}

void MainFrame::menuEvent_playOnChange(wxCommandEvent& evt)
{
	playDuringEdits_always->Check(false);
	playDuringEdits_onchange->Check(true);
	playDuringEdits_never->Check(false);
	play_during_edit = PLAY_ON_CHANGE;
}

void MainFrame::menuEvent_playNever(wxCommandEvent& evt)
{
	playDuringEdits_always->Check(false);
	playDuringEdits_onchange->Check(false);
	playDuringEdits_never->Check(true);
	play_during_edit = PLAY_NEVER;
}

void MainFrame::menuEvent_removeOverlapping(wxCommandEvent& evt)
{
	getCurrentSequence()->getCurrentTrack()->action( new Action::RemoveOverlapping() );
}

void MainFrame::menuEvent_quit(wxCommandEvent& evt)
{
	// close all open sequences
	while(getSequenceAmount()>0)
	{
		if( !closeSequence() )
		{
		    // user canceled, don't quit
			return;
		}
	}

	// quit
	wxWindow::Destroy();
}

void MainFrame::menuClosed(wxMenuEvent& evt)
{
    /*
      // I think it should work with my latest OpenGl changes. To be tested.
    // FIXME - to work around a GTK bug. might also a bug with my graphics drivers, unsure.
    #ifdef __WXGTK__
    glPane->render();
    #endif
     */
}

void MainFrame::menuEvent_about(wxCommandEvent& evt)
{
	aboutDialog->show();
}
/*
class ManualView : public wxFrame
{
	wxHtmlWindow* html;
	wxBorderSizer* sizer;
	bool success;
	
public:	
	ManualView(wxString file) : wxFrame(NULL, wxID_ANY, _("Manual"))
	{
		std::cout << "opening " << toCString(file) << std::endl;
		
		sizer = new wxBorderSizer();
		html = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
		sizer->Add(html, 1, wxEXPAND, 0, Location::Center());
		
		success = html->LoadFile(file);
		SetSizer(sizer);
		SetAutoLayout(true);
		
		if(!success)
		{
			wxMessageBox( _("Could not open manual") );
		}
	}
	
	void popup()
	{
		if(success) Show();	
	}
	
};
ManualView* manualView = NULL;
*/

void MainFrame::menuEvent_manual(wxCommandEvent& evt)
{
#ifdef __WXMAC__
	wxString path_to_docs =  wxT("file://") + wxStandardPaths::Get().GetResourcesDir() + wxT("/../../../documentation/man.html");
#endif
	
#ifdef __WXGTK__
	wxString path_to_docs =  wxT("file://") + getResourcePrefix() + wxT("docs/man.html");

	// if kept in place (not installed)
	if(! wxFileExists(path_to_docs) )
		path_to_docs =  wxT("file://") + getResourcePrefix() + wxT("../../docs/man.html");
#endif
	
#ifndef __WXMAC__
#ifndef __WXGTK__
	#warning "Opening the manual has not been implemented on your system"
#endif
#endif
	
	wxLaunchDefaultBrowser( path_to_docs );
}

void MainFrame::menuEvent_automaticChannelModeSelected(wxCommandEvent& evt)
{

	Sequence* sequence = getCurrentSequence();
	// we were in manual mode... we will need to merge tracks while switching modes. ask user first
	if( sequence->getChannelManagementType() == CHANNEL_MANUAL)
	{
		int answer = wxMessageBox(  _("If multiple tracks play on the same channel, they will be merged.\nThis cannot be undone.\n\nDo you really want to continue?"),
								    _("Confirm"),
								   wxYES_NO, this);

		if (answer != wxYES)
		{
			// nothing will be changed, put checks back
			channelManagement_automatic->Check(false);
			channelManagement_manual->Check(true);
			return;
		}

		for(int i=0; i<sequence->getTrackAmount(); i++)
		{
			for(int j=0; j<sequence->getTrackAmount(); j++)
			{
				if(i == j) continue; //don't compare a track with itself

				if(sequence->getTrack(i)->getChannel() == sequence->getTrack(j)->getChannel())
				{
					sequence->getTrack(i)->mergeTrackIn( sequence->getTrack(j) );
					sequence->deleteTrack(j);
					i = 0;
					j = 0;
				}
			}// next j
		}//next i

		sequence->setCurrentTrackID(0);

		// prevent undoing (anyway it would not have worked, would just have given buggy behaviour)
		sequence->clearUndoStack();
		//sequence->getCurrentTrack()->saveUndoMemory();

	}

	channelManagement_automatic->Check(true);
	channelManagement_manual->Check(false);

	getCurrentSequence()->setChannelManagementType(CHANNEL_AUTO);
	glPane->render();
}

void MainFrame::menuEvent_manualChannelModeSelected(wxCommandEvent& evt)
{
	channelManagement_automatic->Check(false);
	channelManagement_manual->Check(true);

	Sequence* sequence = getCurrentSequence();
	// we were in auto mode... we will need to set channels
	if( sequence->getChannelManagementType() == CHANNEL_AUTO)
	{
		int channel = 0;
		// iterrate through tarcks, give each one a channel
		for(int i=0; i<sequence->getTrackAmount(); i++)
		{
			//if this is a drum track, give channel 9
			if(sequence->getTrack(i)->graphics->editorMode == DRUM)
			{
				sequence->getTrack(i)->setChannel(9);
			}
			else
			// otherwise, give any channel but 9
			{
				sequence->getTrack(i)->setChannel(channel);
				channel++;
				if(channel==9) channel++;
			}
		}
	}

	getCurrentSequence()->setChannelManagementType(CHANNEL_MANUAL);
	glPane->render();
}

void MainFrame::menuEvent_expandedMeasuresSelected(wxCommandEvent& evt)
{
	getCurrentSequence()->measureBar->setExpandedMode( expandedMeasuresMenuItem->IsChecked() );
	updateVerticalScrollbar();
}

void MainFrame::onFocus(wxFocusEvent& evt)
{
}

void MainFrame::changeChannelManagement(ChannelManagementType mode)
{
	if(mode == CHANNEL_AUTO)
	{
		channelManagement_automatic->Check(true);
		channelManagement_manual->Check(false);

		getCurrentSequence()->setChannelManagementType(CHANNEL_AUTO);
	}
	else if(mode == CHANNEL_MANUAL)
	{
		channelManagement_automatic->Check(false);
		channelManagement_manual->Check(true);

		getCurrentSequence()->setChannelManagementType(CHANNEL_MANUAL);
	}

	glPane->render();
}


// ------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------- TOP BAR -----------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

void MainFrame::updateTopBarForSequence(Sequence* seq)
{

    changingValues=true; // ignore events thrown while changing values in the top bar

    // first measure
{
    char buffer[4];
    sprintf (buffer, "%d", seq->measureBar->getFirstMeasure()+1);

    firstMeasure->SetValue( fromCString(buffer) );
}

// measure length
{
    char buffer[4];
    sprintf (buffer, "%d", getMeasureBar()->getTimeSigNumerator() );

    measureTypeTop->SetValue( fromCString(buffer) );
}

{
    char buffer[4];
    sprintf (buffer, "%d", getMeasureBar()->getTimeSigDenominator() );

    measureTypeBottom->SetValue( fromCString(buffer) );
}

// tempo
{
    char buffer[4];
    sprintf (buffer, "%d", seq->getTempo() );

    tempoCtrl->SetValue( fromCString(buffer) );
}

// song length
{
    songLength->SetValue( seq->measureBar->getMeasureAmount() );
}

// zoom
displayZoom->SetValue( seq->getZoomInPercent() );

// set zoom (reason to set it again is because the first time you open it, it may not already have a zoom)
getCurrentSequence()->setZoom( seq->getZoomInPercent() );

expandedMeasuresMenuItem->Check( getMeasureBar()->isExpandedMode() );

// scrollbars
updateHorizontalScrollbar();
updateVerticalScrollbar();

changingValues=false;


}

void MainFrame::songLengthTextChanged(wxCommandEvent& evt){

    static wxString previousString = wxT("");

    // only send event if the same string is sent twice (i.e. first time, because it was typed in, second time because 'enter' was pressed)
    // or if the same number of chars is kept (e.g. 100 -> 150 will be updated immediatley, but 100 -> 2 will not, since user may actually be typing 250)
    if(evt.GetString().IsSameAs(previousString) or (previousString.Length()>0 and evt.GetString().Length()>=previousString.Length()) )
	{

        if(!evt.GetString().IsSameAs(previousString))
		{
			if(evt.GetString().Length()==1) return; // too short to update now, user probably just typed the first character of something longer
        }

        wxSpinEvent unused;
        songLengthChanged(unused);
    }

    previousString = evt.GetString();

}

void MainFrame::zoomTextChanged(wxCommandEvent& evt)
{

    static wxString previousString = wxT("");

    // only send event if the same string is sent twice (i.e. first time, because it was typed in, second time because 'enter' was pressed)
    // or if the same number of chars is kept (e.g. 100 -> 150 will be updated immediatley, but 100 -> 2 will not, since user may actually be typing 250)
    if(evt.GetString().IsSameAs(previousString) or (previousString.Length()>0 and evt.GetString().Length()>=previousString.Length()) )
	{

        if(!evt.GetString().IsSameAs(previousString))
		{
            if(evt.GetString().Length()==1) return; // too short to update now, user probably just typed the first character of something longer
            if(evt.GetString().Length()==2 and atoi_u(evt.GetString())<30 )
                return; // zoom too small, user probably just typed the first characters of something longer
        }

        wxSpinEvent unused;
        zoomChanged(unused); // throw event so that the wxSpinEvent method can catch the change
    }

    previousString = evt.GetString();

}


// ------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- PLAY/STOP --------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------


void MainFrame::playClicked(wxCommandEvent& evt)
{

	if(playback_mode) return; // already playing

    toolsEnterPlaybackMode();

	int startTick = -1;

	const bool success = PlatformMidiManager::playSequence( getCurrentSequence(), /*out*/ &startTick );
	if(!success) std::cerr << "Couldn't play" << std::endl;

    glPane->setPlaybackStartTick( startTick );

    if(startTick == -1 or !success /* failure */)
	{
        glPane->exitPlayLoop();
        //if(!PlatformMidiManager::isPlaying()) toolsExitPlaybackMode();
        //glPane->render();
    }
	else
	{
        glPane->enterPlayLoop();
    }

}

void MainFrame::stopClicked(wxCommandEvent& evt)
{
    if(!playback_mode)
	{
		//std::cout << "stop refused, playback_mode==false" << std::endl;
		return;
	}
    glPane->exitPlayLoop();
/*
    toolsExitPlaybackMode();

	PlatformMidiManager::stop();
    glPane->render();
*/

}

// event sent by the MusicPlayer to notify that it has stopped playing because the song is over.
void MainFrame::songHasFinishedPlaying()
{
    toolsExitPlaybackMode();
    glPane->render();
}

void MainFrame::toolsEnterPlaybackMode()
{
	if(playback_mode) return;

	playback_mode = true;


    stop->Enable(true);
    play->Enable(false);

    fileMenu->Enable(MENU_FILE_NEW, false);
    fileMenu->Enable(MENU_FILE_OPEN, false);
    fileMenu->Enable(MENU_FILE_SAVE, false);
    fileMenu->Enable(MENU_FILE_SAVE_AS, false);
    fileMenu->Enable(MENU_FILE_CLOSE, false);
    fileMenu->Enable(MENU_FILE_IMPORT_MIDI, false);
    fileMenu->Enable(MENU_FILE_EXPORT_MIDI, false);
	fileMenu->Enable(MENU_FILE_EXPORT_SAMPLED_AUDIO, false);
    fileMenu->Enable(wxID_EXIT, false);
    
    measureTypeBottom->Enable(false);
    measureTypeTop->Enable(false);
    firstMeasure->Enable(false);
    songLength->Enable(false);
    tempoCtrl->Enable(false);
}

void MainFrame::toolsExitPlaybackMode()
{
	playback_mode = false;

    stop->Enable(false);
    play->Enable(true);

    fileMenu->Enable(MENU_FILE_NEW, true);
    fileMenu->Enable(MENU_FILE_OPEN, true);
    fileMenu->Enable(MENU_FILE_SAVE, true);
    fileMenu->Enable(MENU_FILE_SAVE_AS, true);
    fileMenu->Enable(MENU_FILE_CLOSE, true);
    fileMenu->Enable(MENU_FILE_IMPORT_MIDI, true);
    fileMenu->Enable(MENU_FILE_EXPORT_MIDI, true);
	fileMenu->Enable(MENU_FILE_EXPORT_SAMPLED_AUDIO, true);
    fileMenu->Enable(wxID_EXIT, true);
    
    measureTypeBottom->Enable(true);
    measureTypeTop->Enable(true);
    firstMeasure->Enable(true);
    songLength->Enable(true);
    tempoCtrl->Enable(true);
}

// ------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------- TOP BAR VALUES CHANGED ---------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

void MainFrame::measureNumChanged(wxCommandEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    int top = atoi_u( measureTypeTop->GetValue() );
    int bottom = atoi_u( measureTypeBottom->GetValue() );

    if(bottom < 1 or top<1 or bottom>32 or top>32) return;

    getMeasureBar()->setTimeSig( top, bottom );

	displayZoom->SetValue( getCurrentSequence()->getZoomInPercent() );
}

void MainFrame::measureDenomChanged(wxCommandEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer is changing values

    int top = atoi_u( measureTypeTop->GetValue() );
    int bottom = atoi_u( measureTypeBottom->GetValue() );

    if(bottom < 1 or top<1 or bottom>32 or top>32) return;

    getMeasureBar()->setTimeSig( top, bottom );

	displayZoom->SetValue( getCurrentSequence()->getZoomInPercent() );
	/*
    // recalculate zoom starting from floating-point value, so that we can keep the same zoom even if measure sig changed
    displayZoom->SetValue(
                          (int)round(
                                     (float)getMeasureBar()->measureLengthInPixels()*100.0/128.0
                                     )
                          );
	 */
}

void MainFrame::firstMeasureChanged(wxCommandEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    int start = atoi_u( firstMeasure->GetValue() );

	if(firstMeasure->GetValue().Length()<1) return; // text field empty, wait until user enters something to update data

    if( !firstMeasure->GetValue().IsNumber() or start < 0 or start > getMeasureBar()->getMeasureAmount() )
	{
        wxBell();

		firstMeasure->SetValue( to_wxString(getMeasureBar()->getFirstMeasure()+1) );

    }
    else
	{
		getMeasureBar()->setFirstMeasure( start-1 );
    }

}


void MainFrame::tempoChanged(wxCommandEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    if(!tempoCtrl->GetValue().IsNumber())
	{
        wxBell();
		tempoCtrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
		return;
    }

    int newTempo = atoi_u(tempoCtrl->GetValue());

    if(newTempo<0)
	{
        wxBell();
		tempoCtrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
    }
	else if(newTempo>10 && newTempo<1000)
	{
        getCurrentSequence()->setTempo(newTempo);
    }

	// necessary because tempo controller needs to be visually updated whenever tempo changes
	// better code could maybe check if tempo controller is visible before rendering - but rendering is quick anyway so it's not really bad
	glPane->render();

}

void MainFrame::changeMeasureAmount(int i, bool throwEvent)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    songLength->SetValue(i);
    getMeasureBar()->updateVector(i);

    if(throwEvent)
	{
        wxSpinEvent evt;
        songLengthChanged(evt);
    }
	else
	{
        // when reading from file
        updateHorizontalScrollbar();
    }
}

void MainFrame::changeShownTimeSig(int num, int denom)
{
	changingValues = true;
	measureTypeTop->SetValue( to_wxString(num) );
	measureTypeBottom->SetValue( to_wxString(denom) );
	changingValues = false;
}

void MainFrame::zoomChanged(wxSpinEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    const int newZoom=displayZoom->GetValue();

    if(newZoom<1 or newZoom>500) return;

    const float oldZoom = getCurrentSequence()->getZoom();

    getCurrentSequence()->setZoom( newZoom );

	const int newXScroll = (int)( horizontalScrollbar->GetThumbPosition()/oldZoom );

    getCurrentSequence()->setXScrollInMidiTicks( newXScroll );
    updateHorizontalScrollbar( newXScroll );

    glPane->render();
}

/*
 * Called whenever the user edits the text field containing song length.
 */

void MainFrame::songLengthChanged(wxSpinEvent& evt)
{

    if(changingValues) return; // discard events thrown because the computer changes values

    const int newLength=songLength->GetValue();

    if(newLength>0)
	{
        getMeasureBar()->setMeasureAmount(newLength);

        updateHorizontalScrollbar();

    }

}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ SCROLLBARS ------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

/*
 * User scrolled horizontally by dragging.
 * Just make sure to update the display to the new values.
 */


void MainFrame::horizontalScrolling(wxScrollEvent& evt)
{

    // don't render many times at the same location
    //static int last_scroll_position = 0;

    const int newValue = horizontalScrollbar->GetThumbPosition();
    if(newValue == getCurrentSequence()->getXScrollInPixels())return;
    //last_scroll_position = newValue;

    getCurrentSequence()->setXScrollInPixels(newValue);

}

/*
 * User scrolled horizontally by clicking oin the arrows.
 * We need to ensure it scrolls of one whole measure (cause scrolling pixel by pixel horizontally would be too slow)
 * We by the way need to make sure it doesn't get out of bounds, in which case we need to put the scrollbar back into correct position.
 */

void MainFrame::horizontalScrolling_arrows(wxScrollEvent& evt)
{

    const int newValue = horizontalScrollbar->GetThumbPosition();
    const int factor = newValue - getCurrentSequence()->getXScrollInPixels();

	const int newScrollInMidiTicks =
        (int)(
              getCurrentSequence()->getXScrollInMidiTicks() +
              factor * getMeasureBar()->defaultMeasureLengthInTicks()
              );

    // check new scroll position is not out of bounds
    const int editor_size=glPane->getWidth()-100,
		total_size = getMeasureBar()->getTotalPixelAmount();
       // (int)(
       //       (getCurrentSequence()->getMeasureAmount() * getCurrentSequence()->ticksPerMeasure()) * getCurrentSequence()->getZoom()
       //       );

    const int positionInPixels = (int)( newScrollInMidiTicks*getCurrentSequence()->getZoom() );

	// scrollbar out of bounds
    if( positionInPixels < 0 )
	{
        updateHorizontalScrollbar( 0 );
        getCurrentSequence()->setXScrollInPixels( 0 );
        return;
    }

	// scrollbar out of bounds
    if( positionInPixels >= total_size-editor_size)
	{
        updateHorizontalScrollbar();
        return;
    }

    getCurrentSequence()->setXScrollInPixels( positionInPixels );
    updateHorizontalScrollbar( newScrollInMidiTicks );
}

/*
 * User scrolled vertically by dragging.
 * Just make sure to update the display to the new values.
 */

void MainFrame::verticalScrolling(wxScrollEvent& evt)
{
    getCurrentSequence()->setYScroll( verticalScrollbar->GetThumbPosition() );
    glPane->render();
}



/*
 * User scrolled vertically by clicking on the arrows.
 * Just make sure to update the display to the new values.
 */

void MainFrame::verticalScrolling_arrows(wxScrollEvent& evt)
{
    getCurrentSequence()->setYScroll( verticalScrollbar->GetThumbPosition() );
    glPane->render();
}

/*
 * Called to update the horizontal scrollbar, usually because song length has changed.
 */

void MainFrame::updateHorizontalScrollbar(int thumbPos)
{

    const int editor_size=glPane->getWidth()-100,
    total_size = getMeasureBar()->getTotalPixelAmount();
    //(int)(
    //      (getCurrentSequence()->getMeasureAmount() * getCurrentSequence()->ticksPerMeasure()) * getCurrentSequence()->getZoom()
    //      );

    int position =
		thumbPos == -1 ?
		/*(int)(getCurrentSequence()->getXScrollInMidiTicks()*getCurrentSequence()->getZoom())*/
		getCurrentSequence()->getXScrollInPixels()
					   :
        (int)(
              thumbPos*getCurrentSequence()->getZoom()
              );

    // if given value is wrong and needs to be changed, we'll need to throw a 'scrolling changed' event to make sure display adapts to new value
    bool changedGivenValue = false;
    if( position < 0 )
	{
        position = 0;
        changedGivenValue = true;
    }
    if( position >= total_size-editor_size)
	{
        position = total_size-editor_size-1;
        changedGivenValue = true;
    }

    horizontalScrollbar->SetScrollbar(
                                      position,
                                      editor_size,
                                      total_size,
                                      1
                                      );

	// scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
	// send a scrolling event that will fix that
	// (internal value will be calculated from scrollbar position)
    if( changedGivenValue )
	{
        wxScrollEvent evt;
        horizontalScrolling(evt);
    }
}

void MainFrame::updateVerticalScrollbar()
{

    int position = getCurrentSequence()->getYScroll();

    const int total_size = getCurrentSequence()->getTotalHeight()+25;
    const int editor_size = glPane->getHeight();

    // if given value is wrong and needs to be changed, we'll need to throw a 'scrolling changed' event to make sure display adapts to new value
    bool changedGivenValue = false;
    if( position < 0 )
	{
        position = 0;
        changedGivenValue = true;
    }

    if( position >= total_size-editor_size)
	{
        position = total_size-editor_size-1;
        changedGivenValue = true;
    }

    verticalScrollbar->SetScrollbar(
                                    position,
                                    editor_size,
                                    total_size,
                                    5 /*scroll amount*/
                                    );

	// scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
	// send a scrolling event that will fix that
	// (internal value will be calculated from scrollbar position)
    if( changedGivenValue )
	{

        wxScrollEvent evt;
        verticalScrolling(evt);
    }
}

// ------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- SEQUENCES --------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------
/*
 * Add a new sequence. There can be multiple sequences if user opens or creates multiple files at the same time.
 */
void MainFrame::addSequence()
{
    sequences.push_back(new Sequence());
    glPane->render();
}

/*
 * Returns the amount of open sequences. There can be multiple sequences if user opens or creates  multiple files at the same time.
 */
int MainFrame::getSequenceAmount()
{
    return sequences.size();
}

/*
 * Close the sequence currently being active. There can be multiple sequences if user opens or creates  multiple files at the same time.
 */
bool MainFrame::closeSequence(int id_arg) // -1 means current
{

	std::cout << "close sequence called" << std::endl;

	int id = id_arg;
	if(id==-1) id = currentSequence;


	if(sequences[id].somethingToUndo())
	{
		int answer = wxMessageBox(  _("Changes will be lost if you close the sequence. Do you really want to continue?"),
								    _("Confirm"),
								   wxYES_NO, this);
		if (answer != wxYES) return false;

	}

	sequences.erase( id );

	if(sequences.size()==0)
	{
		// shut down program (we close last window, so wx will shut down the app)
		Hide();
		return true;
	}

	setCurrentSequence(0);

	//if(sequences.size()>0) glPane->render();
	glPane->render();
	return true;

}

/*
 * Returns the sequence currently being active. There can be multiple sequences if user opens or creates  multiple files at the same time.
 */

Sequence* MainFrame::getCurrentSequence()
{
    assertExpr(currentSequence,>=,0);
    assertExpr(currentSequence,<,sequences.size());

    return &sequences[currentSequence];
}

Sequence* MainFrame::getSequence(int n)
{
    assertExpr(n,>=,0);
    assertExpr(n,<,sequences.size());

    return &sequences[n];
}

int MainFrame::getCurrentSequenceID()
{
	return currentSequence;
}

void MainFrame::setCurrentSequence(int n)
{
    assertExpr(n,>=,0);
    assertExpr(n,<,sequences.size());

    currentSequence = n;
	updateTopBarForSequence( getCurrentSequence() );
	changeChannelManagement( getCurrentSequence()->getChannelManagementType() );
}


// ------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------- I/O ---------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

/*
 * Opens the .aria file in filepath, reads it and prepares the editor to display and edit it.
 */

void MainFrame::loadAriaFile(wxString filePath)
{

	if(filePath.IsEmpty()) return;

	const int old_currentSequence = currentSequence;

	addSequence();
    setCurrentSequence( getSequenceAmount()-1 );
    getCurrentSequence()->filepath=filePath;

	WaitWindow::show(_("Please wait while .aria file is loading.") );

    const bool success = AriaMaestosa::loadAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);
	if(!success)
	{
		std::cout << "Loading .aria file failed." << std::endl;
		wxMessageBox(  _("Sorry, loading .aria file failed.") );
		WaitWindow::hide();

		closeSequence();

        return;
	}

	WaitWindow::hide();
    updateVerticalScrollbar();

    // change song name
    getCurrentSequence()->sequenceFileName = getCurrentSequence()->filepath.AfterLast('/').BeforeLast('.');

	// if a song is currently playing, it needs to stay on top
	if(PlatformMidiManager::isPlaying()) setCurrentSequence(old_currentSequence);
    else updateTopBarForSequence( getCurrentSequence() );

    glPane->render();

}

/*
 * Opens the .mid file in filepath, reads it and prepares the editor to display and edit it.
 */

void MainFrame::loadMidiFile(wxString midiFilePath)
{

	if(midiFilePath.IsEmpty()) return;

	const int old_currentSequence = currentSequence;

    addSequence();
    setCurrentSequence( getSequenceAmount()-1 );

    WaitWindow::show( _("Please wait while midi file is loading.") , true);

    if(!AriaMaestosa::loadMidiFile( getCurrentSequence(), midiFilePath ) )
	{
        std::cout << "Loading midi file failed." << std::endl;
		wxMessageBox(  _("Sorry, loading midi file failed.") );
		WaitWindow::hide();

		closeSequence();

        return;
    }

    WaitWindow::hide();
    updateVerticalScrollbar();

    // change song name
    getCurrentSequence()->sequenceFileName = midiFilePath.AfterLast('/').BeforeLast('.');

	// if a song is currently playing, it needs to stay on top
	if(PlatformMidiManager::isPlaying()) setCurrentSequence(old_currentSequence);

    glPane->render();

}

void MainFrame::evt_freeVolumeSlider( wxCommandEvent& evt )
{
	freeVolumeSlider();
    /*
      // I think it should work with my latest OpenGl changes. To be tested.
#ifdef __WXGTK__
	glPane->render();
#endif
     */
}


MainFrame::~MainFrame()
{

#ifdef _MORE_DEBUG_CHECKS
    std::cout << "~MainFrame BEGIN" << std::endl;
#endif

    ImageProvider::unloadImages();
    PlatformMidiManager::freeMidiPlayer();
	CopyrightWindow::free();
	//ScalePicker::free();

	aboutDialog->Destroy();
	customNoteSelectDialog->Destroy();
    prefs->Destroy();

	delete instrument_picker;
	delete drumKit_picker;
	delete keyPicker;
	delete tuningPicker;

#ifdef _MORE_DEBUG_CHECKS
    std::cout << "~MainFrame END" << std::endl;
#endif

}

/*
wxCommandEvent event( wxEVT_DESTROY_VOLUME_SLIDER, 100000 );
	getMainFrame()->GetEventHandler()->AddPendingEvent( event );

event::SetInt/GetInt/SetString/GetString
*/
void MainFrame::evt_showWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::show( evt.GetString(), evt.GetInt() );
}
void MainFrame::evt_updateWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::setProgress( evt.GetInt() );
}
void MainFrame::evt_hideWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::hide();
}


}
