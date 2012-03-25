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

#include "AriaCore.h"
#include "Clipboard.h"
#include "Utils.h"

#include "Actions/EditAction.h"
#include "Actions/RemoveOverlapping.h"

#include "Dialogs/AboutDialog.h"
#include "Dialogs/CopyrightWindow.h"
#include "Dialogs/CustomNoteSelectDialog.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/ScaleDialog.h"
#include "Dialogs/WaitWindow.h"

#include "GUI/GraphicalTrack.h"
#include "GUI/ImageProvider.h"
#include "GUI/Machelper.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "GUI/MeasureBar.h"

#include "IO/AriaFileWriter.h"
#include "IO/IOUtils.h"
#include "IO/MidiFileReader.h"

#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"

#include "Pickers/InstrumentPicker.h"
#include "Pickers/DrumPicker.h"
#include "Pickers/VolumeSlider.h"
#include "Pickers/TuningPicker.h"
#include "Pickers/KeyPicker.h"
#include "Pickers/TimeSigPicker.h"

#include <iostream>
#include <sstream>

#include <wx/image.h> 
#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/scrolbar.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/notebook.h>
#include <wx/imaglist.h>
#include <wx/log.h>

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef __WXMSW__
#include "win32/Aria.xpm"
#endif

using namespace AriaMaestosa;

namespace AriaMaestosa
{
    // events useful if you need to show a progress bar from another thread
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_EXTEND_TICK)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_NEW_VERSION_AVAILABLE)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_ASYNC_ERROR_MESSAGE)
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MainFrame, wxFrame)

//EVT_SET_FOCUS(MainFrame::onFocus)

/* scrollbar */
EVT_COMMAND_SCROLL_THUMBRELEASE(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_THUMBTRACK(SCROLLBAR_H, MainFrame::horizontalScrolling)
EVT_COMMAND_SCROLL_PAGEUP(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)
EVT_COMMAND_SCROLL_PAGEDOWN(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)
EVT_COMMAND_SCROLL_LINEUP(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)
EVT_COMMAND_SCROLL_LINEDOWN(SCROLLBAR_H, MainFrame::horizontalScrolling_arrows)

EVT_COMMAND_SCROLL_THUMBRELEASE(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_THUMBTRACK(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_PAGEUP(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_PAGEDOWN(SCROLLBAR_V, MainFrame::verticalScrolling)
EVT_COMMAND_SCROLL_LINEUP(SCROLLBAR_V, MainFrame::verticalScrolling_arrows)
EVT_COMMAND_SCROLL_LINEDOWN(SCROLLBAR_V, MainFrame::verticalScrolling_arrows)

EVT_CLOSE(MainFrame::on_close)

/* top bar */
#ifdef NO_WX_TOOLBAR
EVT_BUTTON(PLAY_CLICKED,   MainFrame::playClicked)
EVT_BUTTON(STOP_CLICKED,   MainFrame::stopClicked)
EVT_BUTTON(RECORD_CLICKED, MainFrame::recordClicked)
#else
EVT_TOOL(PLAY_CLICKED,   MainFrame::playClicked)
EVT_TOOL(STOP_CLICKED,   MainFrame::stopClicked)
EVT_TOOL(RECORD_CLICKED, MainFrame::recordClicked)
#endif

EVT_TEXT(TEMPO, MainFrame::tempoChanged)

EVT_BUTTON(TIME_SIGNATURE, MainFrame::timeSigClicked)
EVT_TEXT(BEGINNING, MainFrame::firstMeasureChanged)

EVT_TEXT_ENTER(TEMPO, MainFrame::enterPressedInTopBar)
EVT_TEXT_ENTER(BEGINNING, MainFrame::enterPressedInTopBar)

EVT_SPINCTRL(LENGTH, MainFrame::songLengthChanged)
EVT_SPINCTRL(ZOOM, MainFrame::zoomChanged)

EVT_TEXT(LENGTH, MainFrame::songLengthTextChanged)
EVT_TEXT(ZOOM, MainFrame::zoomTextChanged)

#ifdef NO_WX_TOOLBAR
EVT_BUTTON(TOOL_BUTTON, MainFrame::toolButtonClicked)
#else
EVT_TOOL(TOOL_BUTTON, MainFrame::toolButtonClicked)
#endif

EVT_COMMAND  (DESTROY_SLIDER_EVENT_ID, wxEVT_DESTROY_VOLUME_SLIDER, MainFrame::evt_freeVolumeSlider)
EVT_COMMAND  (DESTROY_TIMESIG_EVENT_ID, wxEVT_DESTROY_TIMESIG_PICKER, MainFrame::evt_freeTimeSigPicker)

// events useful if you need to show a progress bar from another thread
EVT_COMMAND  (SHOW_WAIT_WINDOW_EVENT_ID, wxEVT_SHOW_WAIT_WINDOW,   MainFrame::evt_showWaitWindow)
EVT_COMMAND  (UPDT_WAIT_WINDOW_EVENT_ID, wxEVT_UPDATE_WAIT_WINDOW, MainFrame::evt_updateWaitWindow)
EVT_COMMAND  (HIDE_WAIT_WINDOW_EVENT_ID, wxEVT_HIDE_WAIT_WINDOW,   MainFrame::evt_hideWaitWindow)

EVT_COMMAND  (wxID_ANY, wxEVT_EXTEND_TICK, MainFrame::evt_extendTick)
EVT_COMMAND  (wxID_ANY, wxEVT_NEW_VERSION_AVAILABLE, MainFrame::evt_newVersionAvailable)

EVT_COMMAND(ASYNC_ERR_MESSAGE_EVENT_ID, wxEVT_ASYNC_ERROR_MESSAGE, MainFrame::evt_asyncErrMessage)

EVT_MOUSEWHEEL(MainFrame::onMouseWheel)

END_EVENT_TABLE()

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------


#ifndef NO_WX_TOOLBAR

CustomToolBar::CustomToolBar(wxWindow* parent) : wxToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_TEXT | wxTB_HORIZONTAL | wxNO_BORDER)
{
}

void CustomToolBar::add(wxControl* ctrl, wxString label)
{
#if wxCHECK_VERSION(2,9,0)
    // wxWidgets 3 supports labels under components in toolbar.
    AddControl(ctrl, label);
#else
    AddControl(ctrl);
#endif

    /*
#ifdef __WXMAC__
    if (not label.IsEmpty())
    {
        // work around wxMac limitation (labels under controls in toolbar don't seem to work)
        // will work only if wx was patched with the supplied patch....
        wxToolBarToolBase* tool = (wxToolBarToolBase*)FindById(ctrl->GetId());
        if (tool != NULL) tool->SetLabel(label);
        else std::cerr << "Failed to set label : " << label.mb_str() << std::endl;
    }
#endif
     */
}
void CustomToolBar::realize()
{
    Realize();
    /*
     wxToolBarTool* tool = (wxToolBarTool*)FindById(TIME_SIGNATURE);
     HIToolbarItemRef ref = tool->m_toolbarItemRef;
     HIToolbarItemSetLabel( ref , CFSTR("Time Sig") );
     */
}
#else
// my generic toolbar
CustomToolBar::CustomToolBar(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
    toolbarSizer=new wxFlexGridSizer(2, 6, 1, 15);
    this->SetSizer(toolbarSizer);
}

void CustomToolBar::AddTool(const int id, wxString label, wxBitmap& bmp)
{
    wxBitmapButton* btn = new wxBitmapButton(this, id, bmp);
    toolbarSizer->Add(btn, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    labels.push_back(label);
}

void CustomToolBar::AddStretchableSpace()
{
    toolbarSizer->AddStretchSpacer();
}

void CustomToolBar::add(wxControl* ctrl, wxString label)
{
    toolbarSizer->Add(ctrl, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    labels.push_back(label);
}
void CustomToolBar::realize()
{
    const int label_amount = labels.size();
    toolbarSizer->SetCols( label_amount );

    for (int n=0; n<label_amount; n++)
    {
        toolbarSizer->Add(new wxStaticText(this, wxID_ANY,  labels[n]), 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 1);
    }
}
void CustomToolBar::SetToolNormalBitmap(const int id, wxBitmap& bmp)
{
    wxWindow* win = wxWindow::FindWindowById( id, this );
    wxBitmapButton* btn = dynamic_cast<wxBitmapButton*>(win);
    if (btn != NULL) btn->SetBitmapLabel(bmp);
}
void CustomToolBar::EnableTool(const int id, const bool enabled)
{
    wxWindow* win = wxWindow::FindWindowById( id, this );
    if ( win != NULL ) win->Enable(enabled);
}
#endif



// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark MainFrame
#endif


#define ARIA_WINDOW_FLAGS wxCLOSE_BOX | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxT("Aria Maestosa"), wxPoint(100,100), wxSize(900,600),
                                 ARIA_WINDOW_FLAGS )
{
    wxLogVerbose( wxT("MainFrame::MainFrame") );
    m_main_panel = new wxPanel(this);
    m_disabled_for_welcome_screen = false;
    m_paused = false;
    
    m_root_sizer = new wxBoxSizer(wxVERTICAL);
    m_root_sizer->Add(m_main_panel, 1, wxEXPAND | wxALL, 0);
    SetSizer(m_root_sizer);

#ifdef NO_WX_TOOLBAR
    m_toolbar = new CustomToolBar(m_main_panel);
#else
    m_toolbar = new CustomToolBar(this);
#endif
    
#ifdef __WXMAC__
    ProcessSerialNumber PSN;
    GetCurrentProcess(&PSN);
    TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif

#ifdef __WXMSW__
    // Windows only
    DragAcceptFiles(true);
#endif

#ifndef NO_WX_TOOLBAR
    SetToolBar(m_toolbar);
#endif
}

#undef ARIA_WINDOW_FLAGS

// ----------------------------------------------------------------------------------------------------------

MainFrame::~MainFrame()
{
    wxLogVerbose( wxT("MainFrame::~MainFrame") );
    
    m_border_sizer->Detach(m_main_panel);
    m_main_panel->Destroy();
    
    ImageProvider::unloadImages();
    PlatformMidiManager::get()->freeMidiPlayer();
    CopyrightWindow::free();
    Clipboard::clear();
    SingletonBase::deleteAll();
#if USE_WX_LOGGING
    dynamic_cast<wxWidgetApp*>(wxTheApp)->closeLogWindow();
#endif
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::init()
{
    wxLogVerbose( wxT("MainFrame::init") );
    changingValues = true;

    Centre();

    m_current_sequence = 0;
    m_playback_mode    = false;

    SetMinSize(wxSize(750, 330));

    initMenuBar();

    wxInitAllImageHandlers();
#ifdef NO_WX_TOOLBAR
    m_border_sizer = new wxFlexGridSizer(3, 2, 0, 0);
    m_border_sizer->AddGrowableCol(0);
    m_border_sizer->AddGrowableRow(1);
#else
    m_border_sizer = new wxFlexGridSizer(2, 2, 0, 0);
    m_border_sizer->AddGrowableCol(0);
    m_border_sizer->AddGrowableRow(0);
#endif

    // a few presets
    wxSize averageTextCtrlSize(wxDefaultSize);
#if defined(__WXOSX_COCOA__)
    averageTextCtrlSize.SetWidth(65);
#else
    averageTextCtrlSize.SetWidth(55);
#endif
    
    wxSize smallTextCtrlSize(wxDefaultSize);
    smallTextCtrlSize.SetWidth(45);

    // -------------------------- Toolbar ----------------------------
    wxLogVerbose( wxT("MainFrame::init (creating toolbar)") );
    
#ifdef NO_WX_TOOLBAR
    m_border_sizer->Add(m_toolbar, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    m_border_sizer->AddSpacer(10);
#endif

    m_play_bitmap.LoadFile( getResourcePrefix()  + wxT("play.png") , wxBITMAP_TYPE_PNG);
    if (not m_play_bitmap.IsOk())
    {
        fprintf(stderr, "Cannot locate data files, aria cannot start. Please check your installation (you may set environment variable ARIA_MAESTOSA_DATA to help Aria find its data if the defaults fail)\n");
        wxMessageBox(wxT("Cannot locate data files, aria cannot start. Please check your installation"));
        exit(1);
    }
    m_record_bitmap.LoadFile( getResourcePrefix()  + wxT("record.png") , wxBITMAP_TYPE_PNG);
    m_record_down_bitmap.LoadFile( getResourcePrefix()  + wxT("record_down.png") , wxBITMAP_TYPE_PNG);
    m_toolbar->AddTool(RECORD_CLICKED, _("Record"), m_record_bitmap);

    m_pause_bitmap.LoadFile( getResourcePrefix()  + wxT("pause.png") , wxBITMAP_TYPE_PNG);
    m_pause_down_bitmap.LoadFile( getResourcePrefix()  + wxT("pause_down.png") , wxBITMAP_TYPE_PNG);
    m_toolbar->AddTool(PLAY_CLICKED, _("Play"), m_play_bitmap);

    wxBitmap stopBitmap;
    stopBitmap.LoadFile( getResourcePrefix()  + wxT("stop.png") , wxBITMAP_TYPE_PNG);
    m_toolbar->AddTool(STOP_CLICKED, _("Stop"), stopBitmap);
    m_toolbar->EnableTool(STOP_CLICKED, false);
    
    m_toolbar->AddSeparator();

    m_song_length = new wxSpinCtrl(m_toolbar, LENGTH, to_wxString(DEFAULT_SONG_LENGTH), wxDefaultPosition,
#if defined(__WXGTK__) || defined(__WXMSW__) || defined(__WXOSX_COCOA__)
                              averageTextCtrlSize
#else
                              wxDefaultSize
#endif
                              , wxTE_PROCESS_ENTER);
    m_song_length->SetRange(1, 9999);
    m_toolbar->add(m_song_length, _("Duration"));

#if defined(__WXMSW__)
    m_toolbar->AddSeparator();
    m_toolbar->AddSeparator();
#endif

    m_tempo_ctrl = new wxTextCtrl(m_toolbar, TEMPO, wxT("120"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER );
    m_toolbar->add(m_tempo_ctrl, _("Tempo"));

#if defined(__WXMSW__)
    m_toolbar->AddSeparator();
    m_toolbar->AddSeparator();
#endif

    m_time_sig = new wxButton(m_toolbar, TIME_SIGNATURE, wxT("4/4"));
#if defined(__WXOSX_COCOA__)
    m_time_sig->SetMinSize( wxSize(60, 30) );
    m_time_sig->SetSize( wxSize(60, 30) );
#endif
    m_toolbar->add(m_time_sig, _("Time Sig") );
    
    m_toolbar->AddSeparator();
#if defined(__WXMSW__)
    m_toolbar->AddSeparator();
#endif

    m_first_measure = new wxTextCtrl(m_toolbar, BEGINNING, wxT("1"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER);
    m_toolbar->add(m_first_measure, _("Start"));

    m_display_zoom = new wxSpinCtrl(m_toolbar, ZOOM, wxT("100"), wxDefaultPosition,
    #if defined(__WXGTK__) || defined(__WXMSW__) || defined(__WXOSX_COCOA__)
                           averageTextCtrlSize
    #else
                           wxDefaultSize
    #endif
                           );

#if defined(__WXMSW__)
    m_toolbar->AddSeparator();
    m_toolbar->AddSeparator();
#endif

    m_display_zoom->SetRange(25,500);
    m_toolbar->add(m_display_zoom, _("Zoom"));

    // seems broken for now
//#if defined(NO_WX_TOOLBAR) || wxMAJOR_VERSION > 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION == 9)
//    toolbar->AddStretchableSpace();
//#else
    m_toolbar->AddSeparator();
//#endif

    m_tool1_bitmap.LoadFile( getResourcePrefix()  + wxT("tool1.png") , wxBITMAP_TYPE_PNG);
    m_tool2_bitmap.LoadFile( getResourcePrefix()  + wxT("tool2.png") , wxBITMAP_TYPE_PNG);

#if !defined(__WXOSX_CARBON__)
    /*
    wxNotebook* test = new wxNotebook(m_toolbar, wxID_ANY);
    wxImageList* imglist = new wxImageList();
    int id1 = imglist->Add( wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_OTHER, wxSize(32,32) ) );
    int id2 = imglist->Add( wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(32,32) ) );
    test->AssignImageList(imglist);
    test->AddPage(new wxPanel(test), "A", true, id1);
    test->AddPage(new wxPanel(test), "B", true, id2);
    test->SetMinSize( wxSize(80, 30) );
    test->SetSize( wxSize(80, 30) );
    m_toolbar->add(test, _("Tool"));
    */
    
    m_tools_bitmap = new wxStaticBitmap(m_toolbar /* test2 */, wxID_ANY, m_tool1_bitmap);
    //wxBitmapButton* stbmp = new wxBitmapButton(m_toolbar /* test2 */, wxID_ANY, m_tool1_bitmap,
    //                                           wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE);
    m_tools_bitmap->Connect(m_tools_bitmap->GetId(), wxEVT_LEFT_DOWN, wxMouseEventHandler(MainFrame::onToolsBitmapMousedown), NULL, this);
    m_tools_bitmap->Connect(m_tools_bitmap->GetId(), wxEVT_LEFT_UP, wxMouseEventHandler(MainFrame::onToolsBitmapMouseup), NULL, this);
    m_toolbar->add(m_tools_bitmap, _("Tool"));
#else
    m_toolbar->AddTool(TOOL_BUTTON, _("Tool"), m_tool1_bitmap);
#endif
    
    m_toolbar->realize();

#if defined(__WXOSX_COCOA__)
    skinButton( m_time_sig->GetHandle() );
    //skinToolbar( m_toolbar->GetHandle() );
    skinFrame( MacGetTopLevelWindowRef() );
#endif
    
    // -------------------------- Notification Panel ----------------------------
	{
        wxBoxSizer* notification_sizer = new wxBoxSizer(wxHORIZONTAL);		
        m_notification_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
        m_notification_text = new wxStaticText(m_notification_panel, wxID_ANY, wxT("[No message]"));
        notification_sizer->Add( new wxStaticBitmap(m_notification_panel, wxID_ANY,
                                                    wxArtProvider::GetBitmap(wxART_WARNING, wxART_OTHER , wxSize(48, 48))),
                                0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );	
        notification_sizer->Add(m_notification_text, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        //I18N: to hide the panel that is shown when a file could not be imported successfully
        wxButton* hideNotif = new wxButton(m_notification_panel, wxID_ANY, _("Hide"));
        notification_sizer->Add(hideNotif, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        m_notification_panel->SetSizer(notification_sizer);
        m_notification_panel->SetBackgroundColour(wxColor(255,225,110));
        m_notification_panel->Hide();
        hideNotif->Connect(hideNotif->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(MainFrame::onHideNotifBar), NULL, this);
	}
    m_root_sizer->Add( m_notification_panel, 0, wxEXPAND | wxALL, 2);
    
    // -------------------------- Main Pane ----------------------------
    wxLogVerbose( wxT("MainFrame::init (creating main pane)") );

#ifdef RENDERER_OPENGL
    
    int args[3];
    args[0]=WX_GL_RGBA;
    args[1]=WX_GL_DOUBLEBUFFER;
    args[2]=0;
    m_main_pane = new MainPane(m_main_panel, args);
    m_border_sizer->Add( static_cast<wxGLCanvas*>(m_main_pane), 1, wxEXPAND | wxTOP | wxLEFT, 1);
    
#elif defined(RENDERER_WXWIDGETS)
    
    m_main_pane = new MainPane(m_main_panel, NULL);
    m_border_sizer->Add( static_cast<wxPanel*>(m_main_pane), 1, wxEXPAND | wxTOP | wxLEFT, 1);
    
#endif

    // give a pointer to our GL Pane to AriaCore
    Core::setMainPane(m_main_pane);

    // -------------------------- Vertical Scrollbar ----------------------------
    wxLogVerbose( wxT("MainFrame::init (creating scrollbars)") );

    m_vertical_scrollbar = new wxScrollBar(m_main_panel, SCROLLBAR_V,
                                           wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

    m_vertical_scrollbar->SetScrollbar(0   /* position*/,
                                       530 /* viewable height / thumb size*/,
                                       530 /* height*/,
                                       5   /* scroll amount*/);

    m_border_sizer->Add(m_vertical_scrollbar, 1, wxEXPAND | wxALL, 0 );


    // -------------------------- Horizontal Scrollbar ----------------------------
    m_horizontal_scrollbar = new wxScrollBar(m_main_panel, SCROLLBAR_H);
    wxBoxSizer* h_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_status_text = new wxStaticText(m_main_panel, wxID_ANY, wxT(""));
    m_status_text->SetMinSize( wxSize(90, -1) );
    m_status_text->SetMaxSize( wxSize(90, -1) );
    
    m_status_text->SetFont( getNumberFont() );
    h_sizer->Add(m_status_text, 0, wxEXPAND | wxLEFT, 2);
    
    h_sizer->Add(m_horizontal_scrollbar, 1, wxEXPAND | wxALL, 0);
    m_border_sizer->Add(h_sizer, 1, wxEXPAND | wxALL, 0);

    // For the first time, set scrollbar manually and not using updateHorizontalScrollbar(), because this method assumes the frame is visible.
    const int editor_size=695, total_size=12*128;

    m_horizontal_scrollbar->SetScrollbar(m_horizontal_scrollbar->GetThumbPosition(),
                                         editor_size,
                                         total_size,
                                         1);

    m_border_sizer->AddSpacer(10);

    // -------------------------- finish ----------------------------

    m_main_panel->SetSizer(m_border_sizer);
    Centre();

#ifdef __WXMSW__
    // Main frame icon
    //wxIcon FrameIcon(Aria_xpm);
    //SetIcon(FrameIcon);
    wxIcon ariaIcon(getResourcePrefix()+wxT("/aria64.png"), wxBITMAP_TYPE_PNG);
    if (not ariaIcon.IsOk())
    {
        fprintf(stderr, "Aria icon not found! (application will have a generic icon)\n");
    }
    else
    {
        SetIcon(ariaIcon);
    }
#elif defined(__WXGTK__)
    wxIcon ariaIcon(getResourcePrefix()+wxT("/aria64.png"), wxBITMAP_TYPE_PNG);
    if (not ariaIcon.IsOk())
    {
        fprintf(stderr, "Aria icon not found! (application will have a generic icon)\n");
    }
    else
    {
        SetIcon(ariaIcon);
    }
#endif

    wxLogVerbose( wxT("MainFrame::init (showing)") );
    Maximize(true);
    Layout();
    Show();
    //Maximize(true);

#ifdef __WXMSW__
    Layout();
#endif

    changingValues = false;

#ifdef __WXMSW__
    // Drag files
    Connect(wxID_ANY, wxEVT_DROP_FILES, wxDropFilesEventHandler(MainFrame::onDropFile),NULL, this);
#endif

    wxLogVerbose( wxT("MainFrame::init (creating pickers)") );

    // create pickers
    m_tuning_picker       =  NULL;
    m_key_picker          =  new KeyPicker();
    m_instrument_picker   =  new InstrumentPicker();
    m_drumKit_picker      =  new DrumPicker();

    wxLogVerbose( wxT("MainFrame::init (loading images)") );
    ImageProvider::loadImages();
    
    wxLogVerbose( wxT("MainFrame::init (main pane now visible)") );
    m_main_pane->isNowVisible();

    //ImageProvider::loadImages();

#ifdef _show_dialog_on_startup
    if (aboutDialog.raw_ptr == NULL) aboutDialog = new AboutDialog();
    aboutDialog->show();
#endif
    wxLogVerbose( wxT("MainFrame::init (done)") );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onHideNotifBar(wxCommandEvent& evt)
{
    m_notification_panel->Hide();
    Layout();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::on_close(wxCloseEvent& evt)
{
    wxLogVerbose(wxT("MainFrame::on_close"));
    
    //wxCommandEvent dummy;
    //menuEvent_quit(dummy);
    
    // the quit menu is greyed out in playback mode, but there are other ways to get this code called
    // (like closing the frame)
    if (m_playback_mode)
    {
        m_main_pane->exitPlayLoop();
    }
    
    // close all open sequences
    while (getSequenceAmount() > 0)
    {
        if (not closeSequence())
        {
            // user canceled, don't quit
            return;
        }
    }
    
    evt.Skip();
    //closeSequence();
}

// ----------------------------------------------------------------------------------------------------------


#ifdef __WXMSW__
void MainFrame::onDropFile(wxDropFilesEvent& event)
{
    int i;
    wxString fileName;
    for (i=0 ; i<event.GetNumberOfFiles() ;i++)
    {
        fileName = event.m_files[i];
        if (fileName.EndsWith(wxT("aria")))
        {
            loadAriaFile(fileName);
        }
        else if (fileName.EndsWith(wxT("mid")) or fileName.EndsWith(wxT("midi")))
        {
            loadMidiFile(fileName);
        }
    }
}
#endif

// ----------------------------------------------------------------------------------------------------------

/** Apparently on Windows we need to catch events here too */
void MainFrame::onMouseWheel(wxMouseEvent& event)
{
    m_main_pane->mouseWheelMoved(event);
}


// ----------------------------------------------------------------------------------------------------------

void MainFrame::setStatusText(wxString text)
{
    m_status_text->SetLabel(text);
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Play/Stop
#endif


void MainFrame::playClicked(wxCommandEvent& evt)
{
    Sequence* seq = getCurrentSequence();
    
    if (m_playback_mode)
    {
        // already playing, this button does "pause" instead
        m_pause_location = m_main_pane->getCurrentTick();
        m_paused = true;
        m_main_pane->exitPlayLoop();
        return;
    }
    
    toolsEnterPlaybackMode();
    
    int startTick = -1;
    int previousStartMeasure = -1;
    
    if (m_paused)
    {
        m_paused = false;
        startTick = m_pause_location;
        
        MeasureData* md = seq->getMeasureData();
        previousStartMeasure = md->getFirstMeasure();
        md->setFirstMeasure( md->measureAtTick(m_pause_location) );
    }
    
    const bool success = PlatformMidiManager::get()->playSequence( seq, /*out*/ &startTick );
    if (not success) std::cerr << "Couldn't play" << std::endl;
    
    seq->setPlaybackStartTick( startTick );
    
    if (previousStartMeasure != -1)
    {
        MeasureData* md = seq->getMeasureData();
        md->setFirstMeasure( previousStartMeasure );
    }
    
    if (startTick == -1 or not success) m_main_pane->exitPlayLoop();
    else                                m_main_pane->enterPlayLoop();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::stopClicked(wxCommandEvent& evt)
{    
    if (m_paused)
    {
        m_paused = false;
        toolsExitPlaybackMode();
    }
    else
    {
        if (not m_playback_mode) return;
        m_main_pane->exitPlayLoop();
    }
    
    setStatusText( wxT("") );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::recordClicked(wxCommandEvent& evt)
{
    if (m_playback_mode)
    {
        stopClicked(evt);
        return;
    }
    
    wxString input = PreferencesData::getInstance()->getValue(SETTING_ID_MIDI_INPUT);

    if (input == _("No MIDI input") or PlatformMidiManager::get()->getInputChoices().IsEmpty())
    {
        wxMessageBox( _("Please select an input port from the 'input' menu before recording") );
        return;
    }
    
    if (m_sequences.size() == 0)
    {
        wxBell();
        return;
    }
    
    Sequence* seq = getCurrentSequence();
    
    if (seq->getCurrentTrack() == NULL)
    {
        wxBell();
        return;
    }
    
    const bool recordSuccess = PlatformMidiManager::get()->startRecording(input, seq->getCurrentTrack());
    if (not recordSuccess)
    {
        m_main_pane->exitPlayLoop();
        return;
    }
    
    int startTick = -1;
    const bool success = PlatformMidiManager::get()->playSequence( seq, /*out*/ &startTick );
    if (not success) std::cerr << "Couldn't play" << std::endl;
    
    seq->setPlaybackStartTick( startTick );
    
    if (startTick == -1 or not success) m_main_pane->exitPlayLoop();
    else                                m_main_pane->enterPlayLoop();
        
    toolsEnterPlaybackMode();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onEnterPlaybackMode()
{
    toolsEnterPlaybackMode();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onLeavePlaybackMode()
{
    toolsExitPlaybackMode();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::toolsEnterPlaybackMode()
{
    if (m_playback_mode) return;

    m_playback_mode = true;

    m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_pause_bitmap);
    m_toolbar->EnableTool(STOP_CLICKED, true);

    if (PlatformMidiManager::get()->isRecording())
    {
        m_toolbar->SetToolNormalBitmap(RECORD_CLICKED, m_record_down_bitmap);
    }
    
    disableMenus(true);

    m_time_sig->Enable(false);
    m_first_measure->Enable(false);
    m_song_length->Enable(false);
    m_tempo_ctrl->Enable(false);
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::toolsExitPlaybackMode()
{
    m_playback_mode = false;

    if (m_paused)
    {
        m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_pause_down_bitmap);
        m_toolbar->EnableTool(STOP_CLICKED, true);
    }
    else
    {
        m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_play_bitmap);
        m_toolbar->EnableTool(STOP_CLICKED, false);
    }
    m_toolbar->SetToolNormalBitmap(RECORD_CLICKED, m_record_bitmap);

    disableMenus(false);

    m_time_sig->Enable(true);
    m_first_measure->Enable(true);
    m_song_length->Enable(true);
    m_tempo_ctrl->Enable(true);
}

// ----------------------------------------------------------------------------------------------------------

TuningPicker* MainFrame::getTuningPicker()
{
    if (m_tuning_picker == NULL)
    {
        m_tuning_picker = new TuningPicker();
    }
    
    return m_tuning_picker;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Top Bar
#endif


void MainFrame::updateTopBarAndScrollbarsForSequence(const GraphicalSequence* seq)
{
    changingValues = true; // ignore events thrown while changing values in the top bar

    const MeasureData* measData = seq->getModel()->getMeasureData();
    
    // first measure
    m_first_measure->SetValue( to_wxString(measData->getFirstMeasure()+1) );

    // time signature
    m_time_sig->SetLabel(wxString::Format(wxT("%i/%i"),
                                          measData->getTimeSigNumerator(),
                                          measData->getTimeSigDenominator()
                                         )
                         );

    // tempo
    m_tempo_ctrl->SetValue( to_wxString(seq->getModel()->getTempo()) );

    // song length
    m_song_length->SetValue( measData->getMeasureAmount() );

    // zoom
    m_display_zoom->SetValue( seq->getZoomInPercent() );


    m_expanded_measures_menu_item->Check( measData->isExpandedMode() );

    if (m_paused)             m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_pause_down_bitmap);
    else if (m_playback_mode) m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_pause_bitmap);
    else                      m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_play_bitmap);
    
#if defined(__WXOSX_COCOA__)
    OSXSetModified(seq->getModel()->somethingToUndo());
#endif
    
    // scrollbars
    updateHorizontalScrollbar();
    updateVerticalScrollbar();

    changingValues = false;
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::songLengthTextChanged(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    static wxString previousString = wxT("");

    // only send event if the same string is sent twice (i.e. first time, because it was typed in, second time because 'enter' was pressed)
    // or if the same number of chars is kept (e.g. 100 -> 150 will be updated immediatley, but 100 -> 2 will not, since user may actually be typing 250)
    const bool enter_pressed = evt.GetString().IsSameAs(previousString);
    if (enter_pressed or (previousString.Length()>0 and evt.GetString().Length()>=previousString.Length()) )
    {

        if (!evt.GetString().IsSameAs(previousString))
        {
            if (evt.GetString().Length()==1) return; // too short to update now, user probably just typed the first character of something longer
        }

        wxSpinEvent unused;
        songLengthChanged(unused);

        // give keyboard focus back to main pane
        if (enter_pressed) m_main_pane->SetFocus();
    }

    previousString = evt.GetString();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::timeSigClicked(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    MeasureData* md = getCurrentSequence()->getMeasureData();
    
    wxPoint pt = wxGetMousePosition();
    showTimeSigPicker(getCurrentGraphicalSequence(),
                      pt.x, pt.y,
                      md->getTimeSigNumerator(),
                      md->getTimeSigDenominator() );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::firstMeasureChanged(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    if (changingValues) return; // discard events thrown because the computer changes values

    int start = atoi_u( m_first_measure->GetValue() );

    if (m_first_measure->GetValue().Length() < 1) return; // text field empty, wait until user enters something to update data

    MeasureData* md = getCurrentSequence()->getMeasureData();
    
    if (not m_first_measure->GetValue().IsNumber() or start < 0 or start > md->getMeasureAmount())
    {
        wxBell();

        m_first_measure->SetValue(to_wxString(md->getFirstMeasure() + 1));
    }
    else
    {
        md->setFirstMeasure( start-1 );
    }

}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::tempoChanged(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    if (changingValues) return; // discard events thrown because the computer changes values

    if (m_tempo_ctrl->GetValue().IsEmpty()) return; // user is probably about to enter something else

    if (not m_tempo_ctrl->GetValue().IsNumber())
    {
        wxBell();
        m_tempo_ctrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
        return;
    }

    int newTempo = atoi_u(m_tempo_ctrl->GetValue());

    if (newTempo < 0)
    {
        wxBell();
        m_tempo_ctrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
    }
    else if (newTempo > 10 and newTempo < 1000)
    {
        getCurrentSequence()->setTempo(newTempo);
    }

    // necessary because tempo controller needs to be visually updated whenever tempo changes
    // better code could maybe check if tempo controller is visible before rendering - but rendering is quick anyway so it's not really bad
    Display::render();

}

// ----------------------------------------------------------------------------------------------------------
/*
void MainFrame::changeMeasureAmount(int i, bool throwEvent)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    m_song_length->SetValue(i);
    getCurrentSequence()->getMeasureData()->setMeasureAmount(i);

    if (throwEvent)
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
*/
// ----------------------------------------------------------------------------------------------------------

void MainFrame::onTimeSigSelectionChanged(int num, int denom)
{
    m_time_sig->SetLabel( wxString::Format(wxT("%i/%i"), num, denom ));
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::zoomChanged(wxSpinEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    if (changingValues) return; // discard events thrown because the computer changes values

    const int newZoom = m_display_zoom->GetValue();

    if (newZoom < 1 or newZoom > 500) return;

    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    
    const float oldZoom = gseq->getZoom();

    gseq->setZoom( newZoom );

    const int newXScroll = (int)( m_horizontal_scrollbar->GetThumbPosition()/oldZoom );

    gseq->setXScrollInMidiTicks( newXScroll );
    updateHorizontalScrollbar( newXScroll );
        
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::zoomTextChanged(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    // FIXME - AWFUL HACK
    static wxString previousString = wxT("");

    // only send event if the same string is sent twice (i.e. first time, because it was typed in, second time because 'enter' was pressed)
    // or if the same number of chars is kept (e.g. 100 -> 150 will be updated immediatley, but 100 -> 2 will not, since user may actually be typing 250)
    const bool enter_pressed = evt.GetString().IsSameAs(previousString);
    if (enter_pressed or (previousString.Length()>0 and evt.GetString().Length()>=previousString.Length()) )
    {

        if (not evt.GetString().IsSameAs(previousString))
        {
            // too short to update now, user probably just typed the first character of something longer
            if (evt.GetString().Length() == 1) return;
            
            if (evt.GetString().Length() == 2 and atoi_u(evt.GetString()) < 30)
            {
                return; // zoom too small, user probably just typed the first characters of something longer
            }
        }

        wxSpinEvent unused;
        zoomChanged(unused); // throw fake event (easier to process all events from a single method)

        // give keyboard focus back to main pane
        if (enter_pressed) m_main_pane->SetFocus();
    }

    previousString = evt.GetString();

}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::toolButtonClicked(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;
    
    EditTool currTool = Editor::getCurrentTool();
    if (currTool == EDIT_TOOL_PENCIL)
    {
        m_toolbar->SetToolNormalBitmap(TOOL_BUTTON, m_tool2_bitmap);
        Editor::setEditTool(EDIT_TOOL_ADD);
    }
    else if (currTool == EDIT_TOOL_ADD)
    {
        m_toolbar->SetToolNormalBitmap(TOOL_BUTTON, m_tool1_bitmap);
        Editor::setEditTool(EDIT_TOOL_PENCIL);
    }
    else
    {
        ASSERT (false);
    }

}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::enterPressedInTopBar(wxCommandEvent& evt)
{
    // give keyboard focus back to main pane
    m_main_pane->SetFocus();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::songLengthChanged(wxSpinEvent& evt)
{
    if (getSequenceAmount() == 0) return;

    if (changingValues) return; // discard events thrown because the computer changes values

    const int newLength = m_song_length->GetValue();

    if (newLength > 0)
    {
        MeasureData* md = getCurrentSequence()->getMeasureData();

        {
            ScopedMeasureTransaction tr(md->startTransaction());
            tr->setMeasureAmount(newLength);
        }
        
        updateHorizontalScrollbar();
    }

}

// ----------------------------------------------------------------------------------------------------------

#if !defined(__WXOSX_CARBON_)

void MainFrame::onToolsBitmapMousedown(wxMouseEvent& evt)
{
    //wxSize size = m_tools_bitmap->GetSize();
    //wxPoint pos = m_tools_bitmap->GetPosition();
    //printf("mouse down %i,%i (m_tools_bitmap = %i,%i size = %i, %i)\n", evt.GetX(), evt.GetY(), pos.x, pos.y,
    //       size.GetWidth(), size.GetHeight());
    
    //EditTool currTool = Editor::getCurrentTool();
    if (evt.GetX()> 34)
    {
        m_tools_bitmap->SetBitmap(m_tool2_bitmap);
        Editor::setEditTool(EDIT_TOOL_ADD);
    }
    else if (evt.GetX() < 34)
    {
        m_tools_bitmap->SetBitmap(m_tool1_bitmap);
        Editor::setEditTool(EDIT_TOOL_PENCIL);
    }
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onToolsBitmapMouseup(wxMouseEvent& evt)
{
    //printf("mouse up\n");
}

#endif

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Scrollbars
#endif


void MainFrame::horizontalScrolling(wxScrollEvent& evt)
{

    // don't render many times at the same location
    //static int last_scroll_position = 0;

    const int newValue = m_horizontal_scrollbar->GetThumbPosition();
    
    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    if (newValue == gseq->getXScrollInPixels()) return;

    gseq->setXScrollInPixels(newValue);
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::horizontalScrolling_arrows(wxScrollEvent& evt)
{

    const int newValue = m_horizontal_scrollbar->GetThumbPosition();
    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    const int factor   = newValue - gseq->getXScrollInPixels();

    MeasureData* md = getCurrentSequence()->getMeasureData();
    MeasureBar* mb = getCurrentGraphicalSequence()->getMeasureBar();
    
    const int newScrollInMidiTicks =
        (int)(
              gseq->getXScrollInMidiTicks() +
              factor * md->defaultMeasureLengthInTicks()
              );

    // check new scroll position is not out of bounds
    const int editor_size = Display::getWidth() - 100;
    const int total_size  = mb->getTotalPixelAmount();

    const int positionInPixels = (int)( newScrollInMidiTicks*gseq->getZoom() );

    // scrollbar out of bounds
    if ( positionInPixels < 0 )
    {
        updateHorizontalScrollbar( 0 );
        gseq->setXScrollInPixels( 0 );
        return;
    }

    // scrollbar out of bounds
    if ( positionInPixels >= total_size-editor_size)
    {
        updateHorizontalScrollbar();
        return;
    }

    gseq->setXScrollInPixels( positionInPixels );
    updateHorizontalScrollbar( newScrollInMidiTicks );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::verticalScrolling(wxScrollEvent& evt)
{
    getCurrentGraphicalSequence()->setYScroll( m_vertical_scrollbar->GetThumbPosition() );
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::verticalScrolling_arrows(wxScrollEvent& evt)
{
    if (m_sequences.size() == 0) return;

    getCurrentGraphicalSequence()->setYScroll( m_vertical_scrollbar->GetThumbPosition() );
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::updateHorizontalScrollbar(int thumbPos)
{
    if (m_sequences.size() == 0) return;
    
    const int editor_size = Display::getWidth() - 100;

    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    const int total_size  = gseq->getMeasureBar()->getTotalPixelAmount();

    ASSERT_E( gseq->getXScrollInPixels(), >=, 0 );
    
    int position = (thumbPos == -1) ? gseq->getXScrollInPixels() : (int)(thumbPos*gseq->getZoom());
        
    // if given value is wrong and needs to be changed, we'll need to throw a 'scrolling changed' event to
    // make sure display adapts to new value
    bool changedGivenValue = false;
    if (position < 0)
    {
        position = 0;
        changedGivenValue = true;
    }
    if (position >= total_size - editor_size)
    {
        position = total_size - editor_size - 1;
        changedGivenValue = true;
    }

    m_horizontal_scrollbar->Enable();
    m_horizontal_scrollbar->SetScrollbar(position,
                                         editor_size,
                                         total_size,
                                         1);

    // scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
    // send a scrolling event that will fix that
    // (internal value will be calculated from scrollbar position)
    if ( changedGivenValue )
    {
        wxScrollEvent evt;
        horizontalScrolling(evt);
    }
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::updateVerticalScrollbar()
{
    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    int position = gseq->getYScroll();

    const int total_size = gseq->getTotalHeight()+25;
    const int editor_size = Display::getHeight();

    // if given value is wrong and needs to be changed, we'll need to throw a 'scrolling changed'
    // event to make sure display adapts to new value
    bool changedGivenValue = false;
    if ( position < 0 )
    {
        position = 0;
        changedGivenValue = true;
    }

    if ( position >= total_size-editor_size)
    {
        position = total_size-editor_size-1;
        changedGivenValue = true;
    }

    m_vertical_scrollbar->SetScrollbar(position,
                                       editor_size,
                                       total_size,
                                       5 /*scroll amount*/
                                       );

    m_vertical_scrollbar->Enable( total_size - editor_size > 0 );
    
    // scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
    // send a scrolling event that will fix that
    // (internal value will be calculated from scrollbar position)
    if ( changedGivenValue )
    {

        wxScrollEvent evt;
        verticalScrolling(evt);
    }
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Sequences
#endif


void MainFrame::addSequence()
{
    Sequence* s = new Sequence(this, this, this, this, Display::isVisible());
    GraphicalSequence* gs = new GraphicalSequence(s);
    m_sequences.push_back( gs );
    setCurrentSequence( m_sequences.size() - 1, false /* update */ );
    gs->createViewForTracks(-1 /* all */);
    
    m_paused = false;
    updateTopBarAndScrollbarsForSequence( getCurrentGraphicalSequence() );
    updateMenuBarToSequence();
    Display::render();
    getMainFrame()->updateUndoMenuLabel();
}

// ----------------------------------------------------------------------------------------------------------

bool MainFrame::closeSequence(int id_arg) // -1 means current
{
    if (m_sequences.size() == 0) return false;
    
    wxString whoToFocusAfter;

    int id = id_arg;
    if (id == -1)
    {
        id = m_current_sequence;
        if (id > 0)
        {
            whoToFocusAfter = m_sequences[id - 1].getModel()->getSequenceFilename();
        }
        else if (m_sequences.size() > 0)
        {
            whoToFocusAfter = m_sequences[m_sequences.size() - 1].getModel()->getSequenceFilename();
        }
    }
    else
    {
        whoToFocusAfter = m_sequences[m_current_sequence].getModel()->getSequenceFilename();
    }


    if (m_sequences[id].getModel()->somethingToUndo())
    {
        wxString message = _("You have unsaved changes in sequence '%s'. Do you want to save them before proceeding?") +
                           wxString(wxT("\n\n")) +
                           _("Selecting 'Yes' will save your document before closing") +
                           wxString(wxT("\n")) + _("Selecting 'No' will discard unsaved changes") +
                           wxString(wxT("\n")) + _("Selecting 'Cancel' will cancel exiting the application");

        message.Replace(wxT("%s"), m_sequences[id].getModel()->getSequenceFilename(), false);

        //I18N: title of dialog shown when exiting without saving
        int answer = wxMessageBox( message, _("Unsaved changes in sequence"),
                                   wxYES_NO | wxCANCEL, this);

        if (answer == wxCANCEL) return false;

        if (answer == wxYES and not doSave())
        {
            // user canceled, don't quit
            return false;
        }
    }

    m_sequences.erase( id );
    m_paused = false;
    m_toolbar->SetToolNormalBitmap(PLAY_CLICKED, m_play_bitmap);

    /*
    if (m_sequences.size() == 0)
    {
        wxLogVerbose(wxT("MainFrame: no sequence open, will close"));
        
        // shut down program (we close last window, so wx will shut down the app)
        Hide();
        Destroy();
        return true;
    }
*/
    
    if (m_sequences.size() > 0)
    {
        int newCurrent = 0;
        const int seqCount = m_sequences.size();
        for (int n=0; n<seqCount; n++)
        {
            if (m_sequences[n].getModel()->getSequenceFilename() == whoToFocusAfter)
            {
                newCurrent = n;
                break;
            }
        }
        setCurrentSequence(newCurrent);
    }
    
    Display::render();
    return true;

}

// ----------------------------------------------------------------------------------------------------------

Sequence* MainFrame::getCurrentSequence()
{
    ASSERT_E(m_current_sequence,>=,0);
    ASSERT_E(m_current_sequence,<,m_sequences.size());

    return m_sequences.get(m_current_sequence)->getModel();
}

// ----------------------------------------------------------------------------------------------------------

GraphicalSequence* MainFrame::getCurrentGraphicalSequence()
{
    if (m_current_sequence < 0 or m_current_sequence >= m_sequences.size()) return NULL;
    return m_sequences.get(m_current_sequence);
}

// ----------------------------------------------------------------------------------------------------------

const GraphicalSequence* MainFrame::getCurrentGraphicalSequence() const
{
    if (m_current_sequence < 0 or m_current_sequence >= m_sequences.size()) return NULL;
    return m_sequences.getConst(m_current_sequence);
}

// ----------------------------------------------------------------------------------------------------------

Sequence* MainFrame::getSequence(int n)
{
    ASSERT_E(n,>=,0);
    ASSERT_E(n,<,m_sequences.size());

    return m_sequences[n].getModel();
}

// ----------------------------------------------------------------------------------------------------------

GraphicalSequence* MainFrame::getGraphicalSequence(int n)
{
    ASSERT_E(n,>=,0);
    ASSERT_E(n,<,m_sequences.size());
    
    return m_sequences.get(n);
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::setCurrentSequence(int n, bool updateView)
{
    ASSERT_E(n,>=,0);
    ASSERT_E(n,<,m_sequences.size());

    m_current_sequence = n;
    if (updateView)
    {
        m_paused = false;
        updateTopBarAndScrollbarsForSequence( getCurrentGraphicalSequence() );
        updateMenuBarToSequence();
    }
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark I/O
#endif

void MainFrame::loadAriaFile(wxString filePath)
{
    wxLogVerbose( wxT("MainFrame::loadAriaFile") );
    
    if (filePath.IsEmpty()) return;

    const int old_currentSequence = m_current_sequence;

    addSequence();
    setCurrentSequence( getSequenceAmount()-1 );
    getCurrentSequence()->setFilepath( filePath );

    WaitWindow::show(_("Please wait while .aria file is loading.") );

    const bool success = AriaMaestosa::loadAriaFile(getCurrentGraphicalSequence(), getCurrentSequence()->getFilepath());
    if (not success)
    {
        std::cout << "Loading .aria file failed." << std::endl;
        WaitWindow::hide();
        wxMessageBox(  _("Sorry, loading .aria file failed.") );

        closeSequence();

        return;
    }

    WaitWindow::hide();
    updateVerticalScrollbar();

    // change song name
    getCurrentSequence()->setSequenceFilename( extractTitle(getCurrentSequence()->getFilepath()) );

    // if a song is currently playing, it needs to stay on top
    if (PlatformMidiManager::get()->isPlaying() or m_paused)
    {
        setCurrentSequence(old_currentSequence);
    }
    else
    {
        updateTopBarAndScrollbarsForSequence( getCurrentGraphicalSequence() );
        updateMenuBarToSequence();
    }

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::loadMidiFile(wxString midiFilePath)
{
    wxLogVerbose( wxT("MainFrame::loadMidiFile") );
    if (midiFilePath.IsEmpty()) return;

    const int old_currentSequence = m_current_sequence;

    addSequence();
    setCurrentSequence( getSequenceAmount()-1 );

    WaitWindow::show( _("Please wait while midi file is loading."));

    std::set<wxString> warnings;
    if (not AriaMaestosa::loadMidiFile( getCurrentGraphicalSequence(), midiFilePath, warnings ) )
    {
        std::cout << "Loading midi file failed." << std::endl;
        WaitWindow::hide();
        wxMessageBox(  _("Sorry, loading midi file failed.") );
        closeSequence();

        return;
    }

    WaitWindow::hide();
    updateVerticalScrollbar();

    // change song name
    getCurrentSequence()->setSequenceFilename( extractTitle(midiFilePath) );

    // if a song is currently playing, it needs to stay on top
    if (PlatformMidiManager::get()->isPlaying() or m_paused) setCurrentSequence(old_currentSequence);

    Display::render();
    
    if (not warnings.empty())
    {
        std::set<wxString>::iterator it;
        std::ostringstream full;
        
        full << (const char*)wxString(_("Loading the MIDI file completed successfully, but with the following warnings (the song may not sound as intended) :")).utf8_str();
        
        for (it=warnings.begin() ; it != warnings.end(); it++)
        {
            std::cerr << (*it).utf8_str() << std::endl;
            full << "\n";
            full << "    " << (*it).utf8_str();
        }
        
        m_notification_text->SetLabel(wxString(full.str().c_str(), wxConvUTF8));
        m_notification_panel->Layout();
        m_notification_panel->GetSizer()->SetSizeHints(m_notification_panel);
		m_notification_panel->Show();
		Layout();
        
    }
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark various events and notifications
#endif


void MainFrame::onActionStackChanged()
{
    updateUndoMenuLabel();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onSequenceDataChanged()
{
    m_main_pane->render();
}

// ----------------------------------------------------------------------------------------------------------

/** event sent by the MusicPlayer to notify that it has stopped playing because the song is over. */
void MainFrame::songHasFinishedPlaying()
{
    toolsExitPlaybackMode();
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_freeVolumeSlider( wxCommandEvent& evt )
{
    freeVolumeSlider();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_freeTimeSigPicker( wxCommandEvent& evt )
{
    freeTimeSigPicker();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_showWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::show( evt.GetString(), evt.GetInt() );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_updateWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::setProgress( evt.GetInt() );
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_hideWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::hide();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_extendTick(wxCommandEvent& evt )
{
    Sequence* seq = getCurrentSequence();
    seq->getMeasureData()->extendToTick(evt.GetInt());
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_newVersionAvailable(wxCommandEvent& evt)
{
    m_notification_text->SetLabel(wxString(_("A new version of Aria Maestosa is now available!")) +
                                  wxT("\n") + _("Visit http://ariamaestosa.sourceforge.net/ to download it"));
    m_notification_panel->Layout();
    m_notification_panel->GetSizer()->SetSizeHints(m_notification_panel);
    m_notification_panel->Show();
    Layout();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_asyncErrMessage(wxCommandEvent& evt)
{
    wxMessageBox(evt.GetString(), _("An error occurred"), wxOK | wxICON_ERROR);
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::onMeasureDataChange(int change)
{
    GraphicalSequence* gseq = getCurrentGraphicalSequence();
    
    if (change & IMeasureDataListener::CHANGED_AMOUNT)
    {
        changingValues = true;
        
        m_song_length->SetValue( gseq->getModel()->getMeasureData()->getMeasureAmount() );
        
        changingValues = false;
    }
    
    updateTopBarAndScrollbarsForSequence( gseq );
    updateMenuBarToSequence();
    m_main_pane->render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::disableScrollbars()
{
    m_horizontal_scrollbar->Disable();
    m_vertical_scrollbar->Disable();
}
