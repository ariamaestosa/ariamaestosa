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

#include "wx/stdpaths.h"
#include "wx/artprov.h"

#include "AriaCore.h"

#include "Actions/AddTrack.h"
#include "Actions/DeleteTrack.h"
#include "Actions/EditAction.h"
#include "Actions/RemoveOverlapping.h"

#include "Dialogs/AboutDialog.h"
#include "Dialogs/CopyrightWindow.h"
#include "Dialogs/CustomNoteSelectDialog.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/PrintSetupDialog.h"
#include "Dialogs/ScaleDialog.h"
#include "Dialogs/TrackPropertiesDialog.h"
#include "Dialogs/WaitWindow.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "IO/IOUtils.h"
#include "IO/AriaFileWriter.h"
#include "IO/MidiFileReader.h"
#include "main.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"
#include "Pickers/KeyPicker.h"
#include "Pickers/TuningPicker.h"

#include "Utils.h"
#include <iostream>

#ifdef __WXMAC__
#include "wx/html/webkit.h"
#endif
#include "wx/filename.h"

namespace AriaMaestosa
{

    enum IDs
    {

        MENU_FILE_NEW = wxID_HIGHEST+1,
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
        MENU_SETTINGS_METRONOME,

        MENU_TRACK_ADD,
        MENU_TRACK_REMOVE,
        MENU_TRACK_BACKG
    };

}

using namespace AriaMaestosa;

// -----------------------------------------------------------------------------------------------------------

void MainFrame::initMenuBar()
{
    wxMenuBar* menuBar = new wxMenuBar();

#define QUICK_ADD_MENU( MENUID, MENUSTRING, METHOD ) Append( MENUID,  MENUSTRING ); Connect(MENUID, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( METHOD ) );
#define QUICK_ADD_CHECK_MENU( MENUID, MENUSTRING, METHOD ) AppendCheckItem( MENUID,  MENUSTRING ); Connect(MENUID, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( METHOD ) );

    // ---- File menu
    fileMenu = new wxMenu();

    //I18N: menu item in the "file" menu
    addIconItem(fileMenu, MENU_FILE_NEW, _("New\tCtrl-N"), wxART_NEW);
    Connect(MENU_FILE_NEW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_new));


    //I18N: menu item in the "file" menu
    addIconItem(fileMenu, MENU_FILE_OPEN, _("Open\tCtrl-O"), wxART_FILE_OPEN);
    Connect(MENU_FILE_OPEN, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_open));


    //I18N: menu item in the "file" menu
    addIconItem(fileMenu, MENU_FILE_SAVE, _("Save\tCtrl-S"), wxART_FILE_SAVE);
    Connect(MENU_FILE_SAVE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_save));

    //I18N: menu item in the "file" menu
    addIconItem(fileMenu, MENU_FILE_SAVE_AS, _("Save As\tCtrl-Shift-S"), wxART_FILE_SAVE_AS);
    Connect(MENU_FILE_SAVE_AS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_saveas));

    //I18N: menu item in the "file" menu
    fileMenu -> QUICK_ADD_MENU ( MENU_FILE_CLOSE, _("Close\tCtrl-W"), MainFrame::menuEvent_close);

    fileMenu->AppendSeparator();
    fileMenu -> QUICK_ADD_MENU ( MENU_FILE_COPYRIGHT, wxString(_("Song info"))+wxT("\tCtrl-I"), MainFrame::menuEvent_copyright );
    //fileMenu->AppendSeparator();

    addIconItem(fileMenu, MENU_FILE_EXPORT_NOTATION, wxString(_("Print musical notation"))+wxT("\tCtrl-P"), wxART_PRINT);
    Connect(MENU_FILE_EXPORT_NOTATION, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_exportNotation));


    fileMenu->AppendSeparator();
    //I18N: menu item in the "file" menu
    fileMenu -> QUICK_ADD_MENU ( MENU_FILE_IMPORT_MIDI, _("Import Midi File"), MainFrame::menuEvent_importmidi );
    //I18N: menu item in the "file" menu
    fileMenu -> QUICK_ADD_MENU ( MENU_FILE_EXPORT_MIDI, _("Export to Midi"), MainFrame::menuEvent_exportmidi );

    // disable export to sampled audio if this feature is not supported by the current PlatformMidiManager
    if (not PlatformMidiManager::get()->getAudioExtension().IsEmpty())
    {
        //I18N: menu item in the "file" menu
        fileMenu -> QUICK_ADD_MENU ( MENU_FILE_EXPORT_SAMPLED_AUDIO, _("Export to Audio"), MainFrame::menuEvent_exportSampledAudio );
    }

    //I18N: menu item in the "file" menu
    addIconItem(fileMenu, wxID_EXIT, _("Quit\tCtrl-Q"), wxART_QUIT);
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_quit));



    //I18N: name of a menu
    menuBar->Append(fileMenu,  _("File") );

    // ---- Edit menu
    editMenu = new wxMenu();

    //I18N: menu item in the "edit" menu
    addIconItem(editMenu, MENU_EDIT_UNDO, _("Undo\tCtrl-Z"), wxART_UNDO);
    Connect(MENU_EDIT_UNDO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_undo));


    editMenu->AppendSeparator();

    //I18N: menu item in the "edit" menu
    addIconItem(editMenu, MENU_EDIT_COPY, _("Copy\tCtrl-C"), wxART_COPY);
    Connect(MENU_EDIT_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_copy));


    //I18N: menu item in the "edit" menu
    addIconItem(editMenu, MENU_EDIT_PASTE, _("Paste\tCtrl-V"), wxART_PASTE);
    Connect(MENU_EDIT_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::menuEvent_paste));


    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_PASTE_AT_CURSOR, _("Paste at cursor\tCtrl-Shift-V"), MainFrame::menuEvent_pasteAtMouse );
    editMenu->AppendSeparator(); // ----- selection
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_SELECT_ALL, _("Select All\tCtrl-A"), MainFrame::menuEvent_selectAll );
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_SELECT_NONE, _("Select None\tCtrl-Shift-A"), MainFrame::menuEvent_selectNone );
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_SELECT_CUSTOM, _("Select Notes...\tCtrl-F"), MainFrame::menuEvent_customNoteSelect );
    editMenu->AppendSeparator(); // ----- actions
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_SNAP_TO_GRID, _("Snap Notes to Grid"), MainFrame::menuEvent_snapToGrid );
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_SCALE, _("Scale"), MainFrame::menuEvent_scale );
    //I18N: menu item in the "edit" menu
    editMenu -> QUICK_ADD_MENU ( MENU_EDIT_REMOVE_OVERLAPPING, _("Remove Overlapping Notes"), MainFrame::menuEvent_removeOverlapping );

    //I18N: name of a menu
    menuBar->Append(editMenu,  _("Edit"));


    // ---- Tracks menu
    trackMenu = new wxMenu();

    //I18N: menu item in the "track" menu
    trackMenu -> QUICK_ADD_MENU ( MENU_TRACK_ADD, wxString(_("Add Track"))+wxT("\tCtrl-Shift-N"), MainFrame::menuEvent_addTrack );
    //I18N: menu item in the "track" menu
    trackMenu -> QUICK_ADD_MENU ( MENU_TRACK_REMOVE, wxString(_("Delete Track"))+wxT("\tCtrl-DEL"), MainFrame::menuEvent_deleteTrack );
    trackMenu->AppendSeparator();
    //I18N: - in the track menu, allows choosing the properties of a track
    trackMenu -> QUICK_ADD_MENU ( MENU_TRACK_BACKG, _("Properties"), MainFrame::menuEvent_trackBackground );

    menuBar->Append(trackMenu,  _("Tracks"));

    // ---- Settings menu
    settingsMenu = new wxMenu();
    followPlaybackMenuItem = settingsMenu -> QUICK_ADD_CHECK_MENU ( MENU_SETTINGS_FOLLOW_PLAYBACK, _("Follow Playback"), MainFrame::menuEvent_followPlayback );
    expandedMeasuresMenuItem = settingsMenu -> QUICK_ADD_CHECK_MENU ( MENU_SETTINGS_MEASURE_EXPANDED, _("Expanded time sig management"), MainFrame::menuEvent_expandedMeasuresSelected );

    followPlaybackMenuItem->Check( Core::getPrefsLongValue("followPlayback") != 0 );

    wxMenu* channelMode_menu = new wxMenu();

    //I18N: - the channel setting. full context : Channel management\n\n* Automatic\n* Manual
    settingsMenu->AppendSubMenu(channelMode_menu,  _("Channel management") );
    //I18N: - the channel setting. full context : Channel management\n\n* Automatic\n* Manual
    channelManagement_automatic = channelMode_menu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_CHANNELS_AUTO,  _("Automatic"), MainFrame::menuEvent_automaticChannelModeSelected);
    channelManagement_automatic->Check();
    //I18N: - the channel setting. full context : Channel management\n\n* Automatic\n* Manual
    channelManagement_manual = channelMode_menu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_CHANNEL_MANUAL,  _("Manual"), MainFrame::menuEvent_manualChannelModeSelected);

    settingsMenu->AppendSeparator(); // ----- global

    wxMenu* playDuringEdits_menu = new wxMenu();
    //I18N: - the note playback setting. full context :\n\nPlay during edit\n\n* Always\n* On note change\n* Never
    settingsMenu->AppendSubMenu(playDuringEdits_menu,  _("Play during edit") );
    //I18N: - the note playback setting. full context :\n\nPlay during edit\n\n* Always\n* On note change\n* Never
    playDuringEdits_always = playDuringEdits_menu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_PLAY_ALWAYS,  _("Always"), MainFrame::menuEvent_playAlways);
    //I18N: - the note playback setting. full context :\n\nPlay during edit\n\n* Always\n* On note change\n* Never
    playDuringEdits_onchange = playDuringEdits_menu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_PLAY_ON_CHANGE,  _("On note change"), MainFrame::menuEvent_playOnChange);
    //I18N: - the note playback setting. full context :\n\nPlay during edit\n\n* Always\n* On note change\n* Never
    playDuringEdits_never = playDuringEdits_menu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_PLAY_NEVER,  _("Never"), MainFrame::menuEvent_playNever);

    m_metronome = settingsMenu->QUICK_ADD_CHECK_MENU(MENU_SETTINGS_METRONOME, _("Play with Metronome"), MainFrame::menuEvent_metronome );

    settingsMenu->QUICK_ADD_MENU( wxID_PREFERENCES,   _("Preferences"), MainFrame::menuEvent_preferences );

    const int playValue = Core::getPrefsLongValue("playDuringEdit");
    if (playValue == PLAY_ON_CHANGE)   playDuringEdits_onchange->Check();
    else if (playValue == PLAY_ALWAYS) playDuringEdits_always->Check();
    else if (playValue == PLAY_NEVER)  playDuringEdits_never->Check();
    else                               {ASSERT(false);}

    menuBar->Append(settingsMenu,  _("Settings"));

    // ----Help menu
    helpMenu = new wxMenu();

    helpMenu->QUICK_ADD_MENU(wxID_ABOUT,  _("About this app"), MainFrame::menuEvent_about);
    //I18N: - in help menu - see the help files
    helpMenu->QUICK_ADD_MENU(wxID_HELP,  _("Manual"), MainFrame::menuEvent_manual);

    menuBar->Append(helpMenu, wxT("&Help"));

    SetMenuBar(menuBar);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::addIconItem(wxMenu* menu, int menuID, const wxString& label, const wxString& stockIconId)
{
    wxMenuItem* menuItem;
    
    menuItem = new wxMenuItem( menu, menuID, label, wxT(""), wxITEM_NORMAL, NULL);
#ifndef __WXMAC__
    menuItem->SetBitmap(wxArtProvider::GetBitmap(stockIconId));
#endif
    menu->Append(menuItem);
}


// -----------------------------------------------------------------------------------------------------------

void MainFrame::disableMenus(const bool disable)
{
    const bool on = !disable;

    fileMenu->Enable(MENU_FILE_NEW, on);
    fileMenu->Enable(MENU_FILE_OPEN, on);
    fileMenu->Enable(MENU_FILE_SAVE, on);
    fileMenu->Enable(MENU_FILE_SAVE_AS, on);
    fileMenu->Enable(MENU_FILE_CLOSE, on);
    fileMenu->Enable(MENU_FILE_IMPORT_MIDI, on);
    fileMenu->Enable(MENU_FILE_EXPORT_MIDI, on);

    if (not PlatformMidiManager::get()->getAudioExtension().IsEmpty())
    {
        fileMenu->Enable(MENU_FILE_EXPORT_SAMPLED_AUDIO, on);
    }

    fileMenu->Enable(MENU_FILE_EXPORT_NOTATION, on);
    fileMenu->Enable(MENU_FILE_COPYRIGHT, on);
    fileMenu->Enable(wxID_EXIT, on);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::updateMenuBarToSequence()
{
    Sequence* sequence = getCurrentSequence();
    ChannelManagementType channelMode = sequence->getChannelManagementType();

    if (channelMode == CHANNEL_AUTO)
    {
        channelManagement_automatic->Check(true);
        channelManagement_manual->Check(false);
    }
    else if (channelMode == CHANNEL_MANUAL)
    {
        channelManagement_automatic->Check(false);
        channelManagement_manual->Check(true);
    }

    followPlaybackMenuItem->Check( sequence->follow_playback );
    expandedMeasuresMenuItem->Check(getMeasureData()->isExpandedMode());
    m_metronome->Check( sequence->playWithMetronome() );
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::updateUndoMenuLabel()
{
    wxMenuBar* menuBar = GetMenuBar();
    wxString undo_what = getCurrentSequence()->getTopActionName();

    if (menuBar!=NULL)
    {
        if (undo_what.size() > 0)
        {
            wxString label =  _("Undo %s\tCtrl-Z");
            label.Replace(wxT("%s"),undo_what );
            menuBar->SetLabel( MENU_EDIT_UNDO, label );
            menuBar->Enable( MENU_EDIT_UNDO, true );
        }
        else
        {
            menuBar->SetLabel( MENU_EDIT_UNDO, _("Can't Undo\tCtrl-Z") );
            menuBar->Enable( MENU_EDIT_UNDO, false );
        }

#ifndef __WXMAC__
        wxMenuItem* undoMenuItem = menuBar->FindItem(MENU_EDIT_UNDO, NULL);
        if (undoMenuItem != NULL)
        {
            undoMenuItem->SetBitmap(wxArtProvider::GetBitmap(wxART_UNDO));
        }
#endif
    }
}

// -----------------------------------------------------------------------------------------------------------
// ------------------------------------------ FILE MENU EVENTS -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark File Menu Events
#endif

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_new(wxCommandEvent& evt)
{
    m_main_pane->forgetClickData();
    addSequence();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_close(wxCommandEvent& evt)
{
    m_main_pane->forgetClickData();
    closeSequence();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_exportNotation(wxCommandEvent& evt)
{
    showPrintSetupDialog( getCurrentSequence() );
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_save(wxCommandEvent& evt)
{
    if (getCurrentSequence()->filepath.IsEmpty()) menuEvent_saveas(evt);
    else saveAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_saveas(wxCommandEvent& evt)
{
    wxString suggestedName = getCurrentSequence()->suggestFileName() + wxT(".aria");

    wxString givenPath = showFileDialog( _("Select destination file"), wxT(""), suggestedName,
                                                     wxT("Aria Maestosa file|*.aria"), true /*save*/);

    if (not givenPath.IsEmpty())
    {


        if (wxFileExists(givenPath))
        {
            int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
                                        wxYES_NO, this);
            if (answer != wxYES) return;
        }

        getCurrentSequence()->filepath = givenPath;
        saveAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);

        // change song name
        getCurrentSequence()->sequenceFileName.set(getCurrentSequence()->filepath.AfterLast('/').BeforeLast('.'));
        Display::render();

    }// end if
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_open(wxCommandEvent& evt)
{
    m_main_pane->forgetClickData();
    wxString filePath = showFileDialog( _("Select file"), wxT(""), wxT(""),  _("Aria Maestosa file|*.aria"), false /*open*/);
    MainFrame::loadAriaFile(filePath);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_importmidi(wxCommandEvent& evt)
{
    m_main_pane->forgetClickData();
    wxString midiFilePath = showFileDialog( _("Select midi file"), wxT(""), wxT(""),  _("Midi file|*.mid;*.midi"), false /*open*/);
    MainFrame::loadMidiFile(midiFilePath);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_exportmidi(wxCommandEvent& evt)
{
    wxString suggestedName = getCurrentSequence()->suggestFileName() + wxT(".mid");

    // show file dialog
    wxString midiFilePath = showFileDialog( _("Select destination file"), wxT(""),
                                            suggestedName, _("Midi file|*.mid"), true /*save*/);

    if (midiFilePath.IsEmpty()) return;

    // if file already exists, ask for overwriting
    if ( wxFileExists(midiFilePath) )
    {
        int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
                                    wxYES_NO, this);
        if (answer != wxYES) return;
    }

    // write data to file
    const bool success = AriaMaestosa::exportMidiFile( getCurrentSequence(), midiFilePath );

    if (not success)
    {
        wxMessageBox( _("Sorry, failed to export midi file."));
    }
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_exportSampledAudio(wxCommandEvent& evt)
{

    wxString extension = PlatformMidiManager::get()->getAudioExtension();
    wxString wildcard = PlatformMidiManager::get()->getAudioWildcard();

    wxString suggestedName = getCurrentSequence()->suggestFileName() + extension;

    // show file dialog
    wxString audioFilePath = showFileDialog(  _("Select destination file"), wxT(""),
                                              suggestedName,
                                              wildcard, true /*save*/);

    if (audioFilePath.IsEmpty()) return;

    // if file already exists, ask for overwriting
    if ( wxFileExists(audioFilePath) )
    {
        int answer = wxMessageBox(  _("The file already exists. Do you wish to overwrite it?"),  _("Confirm"),
                                    wxYES_NO, this);
        if (answer != wxYES) return;
    }


    // show progress bar
    MAKE_SHOW_PROGRESSBAR_EVENT( event, _("Please wait while audio file is being generated.\n\nDepending on the length of your file,\nthis can take several minutes."), false );
    GetEventHandler()->AddPendingEvent(event);

    std::cout << "export audio file " << audioFilePath.mb_str() << std::endl;

    // write data
    PlatformMidiManager::get()->exportAudioFile( getCurrentSequence(), audioFilePath );
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_copyright(wxCommandEvent& evt)
{
    CopyrightWindow::show( getCurrentSequence() );
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_quit(wxCommandEvent& evt)
{
    // close all open sequences
    while (getSequenceAmount()>0)
    {
        if ( not closeSequence() )
        {
            // user canceled, don't quit
            return;
        }
    }

    // quit
    wxWindow::Destroy();
}

// -----------------------------------------------------------------------------------------------------------
// ------------------------------------------ EDIT MENU EVENTS -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Edit Menu Events
#endif

void MainFrame::menuEvent_copy(wxCommandEvent& evt)
{
    getCurrentSequence()->copy();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_customNoteSelect(wxCommandEvent& evt)
{
    if (m_custom_note_select_dialog.raw_ptr == NULL)
    {
        m_custom_note_select_dialog = new CustomNoteSelectDialog();
    }

    m_custom_note_select_dialog->show( getCurrentSequence()->getCurrentTrack() );
    
    // After dialog is dismissed, bring focus back to main pane so key presses are detected
    m_main_pane->SetFocus();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_paste(wxCommandEvent& evt)
{
    getCurrentSequence()->paste();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_pasteAtMouse(wxCommandEvent& evt)
{
    getCurrentSequence()->pasteAtMouse();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_snapToGrid(wxCommandEvent& evt)
{
    getCurrentSequence()->snapNotesToGrid();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_undo(wxCommandEvent& evt)
{
    getCurrentSequence()->undo();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_selectNone(wxCommandEvent& evt)
{
    getCurrentSequence()->selectNone();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_selectAll(wxCommandEvent& evt)
{
    getCurrentSequence()->selectAll();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_scale(wxCommandEvent& evt)
{
    ScaleDialog::pickScale( getCurrentSequence() );
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_removeOverlapping(wxCommandEvent& evt)
{
    getCurrentSequence()->getCurrentTrack()->action( new Action::RemoveOverlapping() );
}

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------- TRACK MENU EVENTS -----------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Track Menu Events
#endif

void MainFrame::menuEvent_addTrack(wxCommandEvent& evt)
{
    //getCurrentSequence()->addTrack();
    getCurrentSequence()->action( new Action::AddTrack(getCurrentSequence()) );
    updateVerticalScrollbar();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_deleteTrack(wxCommandEvent& evt)
{

    int answer = wxMessageBox(  _("Do you really want to delete this track?"),  _("Confirm"),
                              wxYES_NO, this);

    m_main_pane->forgetClickData();

    if (answer == wxYES)
    {
        getCurrentSequence()->action( new Action::DeleteTrack( getCurrentSequence() ) );
        //getCurrentSequence()->deleteTrack();
        updateVerticalScrollbar();
    }

}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_trackBackground(wxCommandEvent& evt)
{
    TrackProperties::show(getCurrentSequence()->getCurrentTrack());
}

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------- SETTINGS MENU EVENTS --------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Settings Menu Events
#endif

void MainFrame::menuEvent_preferences(wxCommandEvent& evt)
{
    if (preferences == NULL) preferences = new PreferencesDialog(this, wxGetApp().prefs);
    preferences->show();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_followPlayback(wxCommandEvent& evt)
{
    getCurrentSequence()->follow_playback = followPlaybackMenuItem->IsChecked();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_playAlways(wxCommandEvent& evt)
{
    playDuringEdits_always->Check(true);
    playDuringEdits_onchange->Check(false);
    playDuringEdits_never->Check(false);
    Core::setPlayDuringEdit(PLAY_ALWAYS);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_playOnChange(wxCommandEvent& evt)
{
    playDuringEdits_always->Check(false);
    playDuringEdits_onchange->Check(true);
    playDuringEdits_never->Check(false);
    Core::setPlayDuringEdit(PLAY_ON_CHANGE);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_playNever(wxCommandEvent& evt)
{
    playDuringEdits_always->Check(false);
    playDuringEdits_onchange->Check(false);
    playDuringEdits_never->Check(true);
    Core::setPlayDuringEdit(PLAY_NEVER);
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_automaticChannelModeSelected(wxCommandEvent& evt)
{

    Sequence* sequence = getCurrentSequence();
    // we were in manual mode... we will need to merge tracks while switching modes. ask user first
    if ( sequence->getChannelManagementType() == CHANNEL_MANUAL )
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
                if (i == j) continue; //don't compare a track with itself

                if (sequence->getTrack(i)->getChannel() == sequence->getTrack(j)->getChannel())
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

    }

    channelManagement_automatic->Check(true);
    channelManagement_manual->Check(false);

    getCurrentSequence()->setChannelManagementType(CHANNEL_AUTO);
    Display::render();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_manualChannelModeSelected(wxCommandEvent& evt)
{
    channelManagement_automatic->Check(false);
    channelManagement_manual->Check(true);

    Sequence* sequence = getCurrentSequence();
    // we were in auto mode... we will need to set channels
    if ( sequence->getChannelManagementType() == CHANNEL_AUTO)
    {
        int channel = 0;
        // iterrate through tarcks, give each one a channel
        for(int i=0; i<sequence->getTrackAmount(); i++)
        {
            //if this is a drum track, give channel 9
            if (sequence->getTrack(i)->graphics->editorMode == DRUM)
            {
                sequence->getTrack(i)->setChannel(9);
            }
            else
                // otherwise, give any channel but 9
            {
                sequence->getTrack(i)->setChannel(channel);
                channel++;
                if (channel==9) channel++;
            }
        }
    }

    getCurrentSequence()->setChannelManagementType(CHANNEL_MANUAL);
    Display::render();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_expandedMeasuresSelected(wxCommandEvent& evt)
{
    getCurrentSequence()->measureData->setExpandedMode( expandedMeasuresMenuItem->IsChecked() );
    updateVerticalScrollbar();
}

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_metronome(wxCommandEvent& evt)
{
    getCurrentSequence()->setPlayWithMetronome( m_metronome->IsChecked() );
}

// -----------------------------------------------------------------------------------------------------------
// ----------------------------------------- HELP MENU EVENTS ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Help Menu Events
#endif

void MainFrame::menuEvent_about(wxCommandEvent& evt)
{
    if (aboutDialog.raw_ptr == NULL) aboutDialog = new AboutDialog();
    aboutDialog->show();
}

// -----------------------------------------------------------------------------------------------------------

#ifdef __WXMAC__
class ManualView : public wxFrame
{
    wxWebKitCtrl* m_html;
    wxBoxSizer* m_sizer;

public:
    ManualView(wxFrame* parent, wxString file) : wxFrame(parent, wxID_ANY, _("Manual"), wxDefaultPosition, wxSize(1000,600))
    {
        m_sizer = new wxBoxSizer(wxHORIZONTAL);
        wxString filepath = wxT("file://") + file ;
        filepath.Replace(wxT(" "), wxT("%20"));
        m_html = new wxWebKitCtrl(this, wxID_ANY, filepath );
        m_sizer->Add(m_html, 1, wxEXPAND);

        SetSizer(m_sizer);

        wxMenuBar* menuBar = new wxMenuBar();

        wxMenu* window = new wxMenu();
        window->Append(wxID_CLOSE, wxString(_("Close"))+wxT("\tCtrl-W"));

        menuBar->Append(window, wxT("Window"));
        SetMenuBar(menuBar);

        Connect(wxID_CLOSE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( ManualView::onClose ));

        Center();
        Show();
    }

    void onClose(wxCommandEvent& evt)
    {
        Hide();
        Destroy();
    }

};
#endif

// -----------------------------------------------------------------------------------------------------------

void MainFrame::menuEvent_manual(wxCommandEvent& evt)
{
    wxString path_to_docs =  getResourcePrefix() + wxT("Documentation") + wxFileName::GetPathSeparator() + wxT("index.html");

    wxString sep = wxFileName::GetPathSeparators();

#ifdef __WXMAC__
    new ManualView(this, path_to_docs);
#else
    if (not wxFileExists( path_to_docs ) or not wxLaunchDefaultBrowser( wxT("file://") + path_to_docs ))
    {
        wxMessageBox(wxT("Sorry, opening docs failed\n(") + path_to_docs +
                     wxT(" does not appear to exist).\nTry ariamaestosa.sourceforge.net instead."));
    }
#endif
}

// -----------------------------------------------------------------------------------------------------------


