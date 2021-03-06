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
#include "Dialogs/SongPropertiesDialog.h"
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
#include <wx/tglbtn.h>
#include <wx/hyperlink.h>
#include <wx/timer.h>
#include <wx/stdpaths.h>

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
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
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_SHOW_TRACK_CONTEXTUAL_MENU)
}


static const int SCROLL_NOTES_INTO_VIEW_DELAY = 200;
static const int SCROLL_NOTES_INTO_VIEW_TIMER = 10000;

class MyCustomScrollbar : public wxScrollBar
{
public:
    MyCustomScrollbar(wxWindow* parent, wxWindowID id, long style) :
        wxScrollBar(parent, id, wxDefaultPosition, wxDefaultSize, style)
    {
    
    }
    
    virtual bool AcceptsFocus() const override { return false; } 
    
    virtual bool AcceptsFocusFromKeyboard() const override { return false; }
};


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
EVT_TOGGLEBUTTON(LOOP_CLICKED,   MainFrame::loopClicked)
#else
EVT_TOOL(PLAY_CLICKED,   MainFrame::playClicked)
EVT_TOOL(STOP_CLICKED,   MainFrame::stopClicked)
EVT_TOOL(RECORD_CLICKED, MainFrame::recordClicked)
EVT_TOOL(LOOP_CLICKED,   MainFrame::loopClicked)
#endif

EVT_TEXT(TEMPO, MainFrame::tempoChanged)

EVT_BUTTON(TIME_SIGNATURE, MainFrame::timeSigClicked)
EVT_TEXT(BEGINNING, MainFrame::firstMeasureChanged)
EVT_TEXT(LOOP_END_MEASURE, MainFrame::loopEndMeasureChanged)

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

EVT_COMMAND(wxID_ANY, wxEVT_SHOW_TRACK_CONTEXTUAL_MENU, MainFrame::evt_showTrackContextualMenu)


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
    AddControl(ctrl, label);
}
void CustomToolBar::realize()
{
    Realize();
}
#else
// my generic toolbar
CustomToolBar::CustomToolBar(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
    toolbarSizer = new wxFlexGridSizer(2, 100, 1, 15);
    SetSizer(toolbarSizer);
    m_is_realized = false;
}

void CustomToolBar::ClearTools()
{
    toolbarSizer->Clear(true);
    labels.clear();
    label_ids.clear();
    label_widgets.clear();
    SetSizer(NULL); // also deletes the previous sizer
    toolbarSizer = new wxFlexGridSizer(2, 100, 1, 15);
    SetSizer(toolbarSizer);
    m_is_realized = false;
}

void CustomToolBar::SetLabelById(const int id, wxString label)
{
    for (unsigned int n = 0; n < label_ids.size(); n++)
    {
        if (label_ids[n] == id)
        {
            label_widgets[n]->SetLabel(label);
            return;
        }
    }
}
void CustomToolBar::AddTool(const int id, wxString label, wxBitmap& bmp)
{
    wxBitmapButton* btn = new wxBitmapButton(this, id, bmp, wxDefaultPosition, wxDefaultSize,
                                             wxBU_AUTODRAW | wxBORDER_NONE);
    toolbarSizer->Add(btn, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    labels.push_back(label);
    label_ids.push_back(id);
}

void CustomToolBar::AddCheckTool(const int id, wxString label, wxBitmap& bmp, bool checked)
{
    wxBitmapToggleButton* btn = new wxBitmapToggleButton(this, id, bmp);
    btn->SetValue(checked);
    toolbarSizer->Add(btn, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    labels.push_back(label);
    label_ids.push_back(id);
}

bool CustomToolBar::GetToolState(int toolId)
{
    wxWindow* window = FindWindow(toolId);
    if (window == NULL)
    {
        fprintf(stderr, "[GetToolState] WARNING: Widget %i not found\n", toolId);
        return false;
    }
    return ((wxToggleButton*)window)->GetValue();
}

void CustomToolBar::ToggleTool(int toolId, bool pressed)
{
    wxWindow* window = FindWindow(toolId);
    if (window == NULL)
    {
        fprintf(stderr, "[ToggleTool] WARNING: Widget %i not found\n", toolId);
        return;
    }
    ((wxToggleButton*)window)->SetValue(pressed);
}

void CustomToolBar::AddStretchableSpace()
{
    toolbarSizer->AddStretchSpacer();
}

void CustomToolBar::add(wxControl* ctrl, wxString label)
{
    toolbarSizer->Add(ctrl, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 5);
    labels.push_back(label);
    label_ids.push_back(ctrl->GetId());
}
void CustomToolBar::realize()
{
    if (m_is_realized)
    {
        toolbarSizer->RecalcSizes();
        toolbarSizer->Layout();
        return;
    }
    
    m_is_realized = true;
    const int label_amount = labels.size();
    toolbarSizer->SetCols( label_amount );

    for (int n=0; n<label_amount; n++)
    {
        wxStaticText* label_widget = new wxStaticText(this, wxID_ANY, labels[n]);
        label_widgets.push_back(label_widget);
        toolbarSizer->Add(label_widget, 0, wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL | wxALL, 1);
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
    m_reload_mode = false;

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

    DragAcceptFiles(true);

#ifndef NO_WX_TOOLBAR
    SetToolBar(m_toolbar);
#endif
}

#undef ARIA_WINDOW_FLAGS

// ----------------------------------------------------------------------------------------------------------

MainFrame::~MainFrame()
{
    wxLogVerbose( wxT("MainFrame::~MainFrame") );
    
    std::map<int, wxTimer*>::iterator it;
    for(it = m_timer_map.begin() ; it != m_timer_map.end(); ++it)
    {
        wxTimer* timer = it->second;
        
        if (timer->IsRunning())
        {
            timer->Stop();
        }

        wxDELETE(timer);
    }
    
    saveWindowPos();

    m_border_sizer->Detach(m_main_panel);
    m_main_panel->Destroy();
    
    ImageProvider::unloadImages();
    PlatformMidiManager::get()->freeMidiPlayer();
    SongPropertiesDialogNamespace::free();
    Clipboard::clear();
    SingletonBase::deleteAll();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::init(const wxArrayString& filesToOpen, bool fileInCommandLine)
{
    wxLogVerbose( wxT("MainFrame::init") );
    changingValues = true;
    m_files_to_open = filesToOpen;
    m_file_in_command_line = fileInCommandLine;
    m_current_dir = wxStandardPaths::Get().GetAppDocumentsDir();

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

    m_loop_bitmap.LoadFile( getResourcePrefix()  + wxT("loop.png") , wxBITMAP_TYPE_PNG);
    m_record_down_bitmap.LoadFile( getResourcePrefix()  + wxT("record_down.png") , wxBITMAP_TYPE_PNG);
    m_pause_bitmap.LoadFile( getResourcePrefix()  + wxT("pause.png") , wxBITMAP_TYPE_PNG);
    m_pause_down_bitmap.LoadFile( getResourcePrefix()  + wxT("pause_down.png") , wxBITMAP_TYPE_PNG);
    m_stop_bitmap.LoadFile( getResourcePrefix()  + wxT("stop.png") , wxBITMAP_TYPE_PNG);
    
    initToolbar();

    // -------------------------- Notification Panel ----------------------------
	{
        wxBoxSizer* notification_sizer = new wxBoxSizer(wxHORIZONTAL);
        m_notification_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
        // for some reason, wxStaticText on some systems needs to be initially multiline to properly scale later one
        m_notification_text = new wxStaticText(m_notification_panel, wxID_ANY, wxT("_______________\n_______________\n_______________\n_______________"),
                                               wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
        m_notification_icon = new wxStaticBitmap(m_notification_panel, wxID_ANY,
                                                    wxArtProvider::GetBitmap(wxART_WARNING, wxART_OTHER , wxSize(48, 48)));
        notification_sizer->Add(m_notification_icon,
                                0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
        
        
        wxBoxSizer* subsizer = new wxBoxSizer(wxVERTICAL);
        subsizer->Add(m_notification_text, 1, wxEXPAND);
        notification_sizer->Add(subsizer, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);

#if wxCHECK_VERSION(2,9,1)
        m_notification_link = new wxGenericHyperlinkCtrl(m_notification_panel, wxID_ANY,
                                                  wxT("http://ariamaestosa.sourceforge.net/download.html"),
                                                  wxT("http://ariamaestosa.sourceforge.net/download.html"));
        m_notification_link->SetForegroundColour(wxColor(0, 44, 166));
        m_notification_link->SetBackgroundColour(wxColor(170,214,250));
        subsizer->Add(m_notification_link, 0, 0);
        m_notification_link->Hide();
#endif
        
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

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_BUFFER_SIZE, 32, 0};
    m_main_pane = new MainPane(m_main_panel, args);
    m_border_sizer->Add( dynamic_cast<wxGLCanvas*>(m_main_pane), 1, wxEXPAND | wxTOP | wxLEFT, 1);

#elif defined(RENDERER_WXWIDGETS)

    m_main_pane = new MainPane(m_main_panel, NULL);
    m_border_sizer->Add( dynamic_cast<wxPanel*>(m_main_pane), 1, wxEXPAND | wxTOP | wxLEFT, 1);

#endif

    // give a pointer to our GL Pane to AriaCore
    Core::setMainPane(m_main_pane);

    // -------------------------- Vertical Scrollbar ----------------------------
    wxLogVerbose( wxT("MainFrame::init (creating scrollbars)") );

    m_vertical_scrollbar = new MyCustomScrollbar(m_main_panel, SCROLLBAR_V, wxSB_VERTICAL);

    m_vertical_scrollbar->SetScrollbar(0   /* position*/,
                                       530 /* viewable height / thumb size*/,
                                       530 /* height*/,
                                       5   /* scroll amount*/);

    m_border_sizer->Add(m_vertical_scrollbar, 1, wxEXPAND | wxALL, 0 );


    // -------------------------- Horizontal Scrollbar ----------------------------
    m_horizontal_scrollbar = new MyCustomScrollbar(m_main_panel, SCROLLBAR_H, wxSB_HORIZONTAL);
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
        wxAssertHandler_t assertHandler = wxSetAssertHandler(NULL); // on XP this tends to assert
        SetIcon(ariaIcon);
        wxSetAssertHandler(assertHandler);
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
    
    wxSize s = wxGetDisplaySize();
    printf("*** %i %i\n", s.GetWidth(), s.GetHeight());
     
    PreferencesData* pd = PreferencesData::getInstance();
    if (pd->getBoolValue(SETTING_ID_REMEMBER_WINDOW_POS, false))
    {
        wxSize screenSize = wxGetDisplaySize();
        
        int x = pd->getIntValue(SETTING_ID_WINDOW_X);
        int y = pd->getIntValue(SETTING_ID_WINDOW_Y);
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        
        int w = pd->getIntValue(SETTING_ID_WINDOW_W);
        if (w < 400) w = 400;
        
        int h = pd->getIntValue(SETTING_ID_WINDOW_H);
        if (h < 400) h = 400;
        
        if (x >= screenSize.GetWidth()) x = 0;
        if (y >= screenSize.GetHeight()) y = 0;
        
        SetPosition( wxPoint(x,y) );
        SetSize( wxSize(w, h) );
    }
    else
    {
        Maximize(true);
    }
    
#ifndef __WXMSW__
    // FIXME: work around wxMSW bug (see below)
    Connect(wxEVT_SHOW, wxShowEventHandler(MainFrame::onShow));
#endif

    Layout();
    Show();
    //Maximize(true);

#ifdef __WXMSW__
    Layout();
#endif

    changingValues = false;

    // Drag files
    Connect(wxID_ANY, wxEVT_DROP_FILES, wxDropFilesEventHandler(MainFrame::onDropFile),NULL, this);

    wxLogVerbose( wxT("MainFrame::init (creating pickers)") );

    // create pickers
    m_tuning_picker       =  NULL;
    m_key_picker          =  new KeyPicker();
    m_instrument_picker   =  new InstrumentPicker();
    m_drumKit_picker      =  new DrumPicker();
    
#ifdef __WXMSW__
    // FIXME: work around wxMSW bug
    wxShowEvent evt;
    onShow(evt);
#endif
    
}

void MainFrame::initToolbar()
{
    // a few presets
    wxSize averageTextCtrlSize(wxDefaultSize);
#if defined(__WXOSX_COCOA__)
    averageTextCtrlSize.SetWidth(65);
#else
    averageTextCtrlSize.SetWidth(55);
#endif

    wxSize smallTextCtrlSize(wxDefaultSize);
    smallTextCtrlSize.SetWidth(45);


    m_toolbar->ClearTools();
    
    m_toolbar->AddTool(RECORD_CLICKED, _("Record"), m_record_bitmap);
    m_toolbar->AddTool(PLAY_CLICKED, _("Play"), m_play_bitmap);

    m_toolbar->AddTool(STOP_CLICKED, _("Stop"), m_stop_bitmap);
    m_toolbar->EnableTool(STOP_CLICKED, false);

    bool loop_enabled = (m_sequences.size() > 0 and getCurrentSequence()->isLoopEnabled());
    m_toolbar->AddCheckTool(LOOP_CLICKED, _("Loop"), m_loop_bitmap, loop_enabled);
    printf("ENABLE: %i\n", m_sequences.size() > 0);
    m_toolbar->EnableTool(LOOP_CLICKED, m_sequences.size() > 0);
    
    m_toolbar->AddSeparator();

    m_song_length = new SPINNER_CLASS(m_toolbar, LENGTH, to_wxString(DEFAULT_SONG_LENGTH), wxDefaultPosition,
#if defined(__WXGTK__) || defined(__WXMSW__) || defined(__WXOSX_COCOA__)
                              averageTextCtrlSize
#else
                              wxDefaultSize
#endif
                              , wxTE_PROCESS_ENTER);

    m_song_length->SetRange(1, 9999);

    //I18N: song length (number of measures)
    m_toolbar->add(m_song_length, _("Length"));

#ifdef __WXMAC__
    wxSpinButton* songLengthSpinner;
    m_toolbar->add(songLengthSpinner = new wxSpinButton(m_toolbar, wxID_ANY));
    songLengthSpinner->SetRange(0, 10000);
    songLengthSpinner->SetValue(5000);
    songLengthSpinner->Bind(wxEVT_SPIN_UP, &wxWorkaroundSpinCtrl::up, m_song_length);
    songLengthSpinner->Bind(wxEVT_SPIN_DOWN, &wxWorkaroundSpinCtrl::down, m_song_length);
#endif
    
#if defined(__WXMSW__)
    m_toolbar->AddSeparator();
    m_toolbar->AddSeparator();
#endif

    m_tempo_ctrl = new wxTextCtrl(m_toolbar, TEMPO, wxT("120"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER );
    m_tempo_ctrl->SetToolTip(_("This is the initial tempo of the song, see the Controller Editor for tempo variations"));
    m_toolbar->add(m_tempo_ctrl, _("Initial tempo"));

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
    
    //m_toolbar->AddSeparator();
    //#if defined(__WXMSW__)
    //m_toolbar->AddSeparator();
    //#endif
    
    m_loop_end_measure = new wxTextCtrl(m_toolbar, LOOP_END_MEASURE, wxT("1"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER);
    //I18N: loop end
    m_toolbar->add(m_loop_end_measure, _("End"));
    
    m_display_zoom = new SPINNER_CLASS(m_toolbar, ZOOM, wxT("100"), wxDefaultPosition,
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

#ifdef __WXMAC__
    wxSpinButton* zoomSpinner;
    m_toolbar->add(zoomSpinner = new wxSpinButton(m_toolbar, wxID_ANY));
    zoomSpinner->SetRange(0, 10000);
    zoomSpinner->SetValue(5000);
    zoomSpinner->Bind(wxEVT_SPIN_UP, &wxWorkaroundSpinCtrl::up, m_display_zoom);
    zoomSpinner->Bind(wxEVT_SPIN_DOWN, &wxWorkaroundSpinCtrl::down, m_display_zoom);
#endif
    
    
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
    //I18N: tool selection tip
    m_tools_bitmap->SetToolTip(_("Tool 1 : Draw notes\nTool 2 : Click to add notes"));
    m_toolbar->add(m_tools_bitmap, _("Tool"));
#else
    m_toolbar->AddTool(TOOL_BUTTON, _("Tool"), m_tool1_bitmap);
#endif
    
    if (!loop_enabled)
    {
        m_toolbar->realize();
        m_toolbar->SetLabelById(LOOP_END_MEASURE, "");
        m_toolbar->HideById(LOOP_END_MEASURE);
    }

    m_toolbar->realize();
    
#if defined(__WXOSX_COCOA__)
    skinButton( m_time_sig->GetHandle() );
    //skinToolbar( m_toolbar->GetHandle() );
    skinFrame( MacGetTopLevelWindowRef() );
#endif
}

void MainFrame::setNotificationWarning()
{
    m_notification_icon->SetBitmap(wxArtProvider::GetBitmap(wxART_WARNING, wxART_OTHER , wxSize(48, 48)));
    m_notification_panel->SetBackgroundColour(wxColor(255,225,110));
}

void MainFrame::setNotificationInfo()
{
    m_notification_icon->SetBitmap(wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_OTHER , wxSize(48, 48)));
    m_notification_panel->SetBackgroundColour(wxColor(170,214,250));
}

void MainFrame::onShow(wxShowEvent& evt)
{
    PreferencesData* pd;
    
    pd = PreferencesData::getInstance();
    
    
    Disconnect(wxEVT_SHOW, wxShowEventHandler(MainFrame::onShow)); // This callback is for initialisation so never call it again
    
#ifdef RENDERER_OPENGL
    m_main_pane->setCurrent();
#endif

    wxLogVerbose( wxT("MainFrame::init (loading images)") );
    ImageProvider::loadImages();

    wxLogVerbose( wxT("MainFrame::init (main pane now visible)") );
    m_main_pane->isNowVisible();
    
#ifdef _show_dialog_on_startup
    if (aboutDialog.raw_ptr == NULL) aboutDialog = new AboutDialog();
    aboutDialog->show();
#endif
    wxLogVerbose( wxT("MainFrame::init (done)") );
    
    
    for (unsigned int n = 0; n < m_files_to_open.Count(); n++)
    {
        loadFile(m_files_to_open[n]);
    }
    
    
    if ( pd->getBoolValue(SETTING_ID_LOAD_LAST_SESSION, false) && !m_file_in_command_line )
    {
        int currentSequenceId;
        
        currentSequenceId = pd->getIntValue(SETTING_ID_LAST_CURRENT_SEQUENCE);
        if (currentSequenceId<m_sequences.size())
        {
            setCurrentSequence(currentSequenceId);
        }
    }
    
}


void MainFrame::onTimer(wxTimerEvent & event)
{
    int sequenceId;
    int timerId;
    
    timerId = event.GetId();
    sequenceId = timerId - SCROLL_NOTES_INTO_VIEW_TIMER;
    scrollKeyboardEditorNotesIntoView(sequenceId);
    
    /* Debug only
    std::cout << "Timer : " << timerId << " -> " << m_timer_map[timerId] 
     << " seqId : " << sequenceId << " seq : " << getSequence(sequenceId) << std::endl;
    */
    
    
    wxDELETE(m_timer_map[timerId]);
    m_timer_map.erase(timerId);
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

    if (!handleApplicationEnd())
    {
        return;
    }
    else
    {
        evt.Skip();
    }
}

// ----------------------------------------------------------------------------------------------------------


void MainFrame::onDropFile(wxDropFilesEvent& event)
{
    int i;
    wxString filePath;
    for (i=0 ; i<event.GetNumberOfFiles() ;i++)
    {
        filePath = event.m_files[i];
        loadFile(filePath);
    }
}


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


void MainFrame::setResizingCursor()
{
    m_main_pane->SetCursor(wxCURSOR_SIZEWE);
}


void MainFrame::setMovingCursor()
{
    m_main_pane->SetCursor(wxCURSOR_SIZING);
}


void MainFrame::setNormalCursor()
{
    m_main_pane->SetCursor(wxNullCursor);
}


void MainFrame::setReloadMode(bool enable)
{
    m_reload_mode = enable;
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
    
    m_toolbar->Refresh();
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
    m_toolbar->Refresh();


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

void MainFrame::loopClicked(wxCommandEvent& evt)
{
    bool pressed = m_toolbar->GetToolState(LOOP_CLICKED);
    getCurrentSequence()->setLoopEnabled(pressed);
    
    m_loop_end_measure->Enable(pressed);
    if (not pressed)
    {
        m_toolbar->SetLabelById(LOOP_END_MEASURE, "");
        m_toolbar->HideById(LOOP_END_MEASURE);
    }
    else
    {
        if (m_loop_end_measure->GetValue() == "1")
        {
            int measure_count = getCurrentSequence()->getMeasureData()->getMeasureAmount();
            m_loop_end_measure->SetValue(to_wxString(measure_count));
            
            // FIXME; measure IDs are sometimes 0-based, sometimes 1-based :S
            getCurrentSequence()->getMeasureData()->setLoopEndMeasure(measure_count - 1);
        }
        m_toolbar->ShowById(LOOP_END_MEASURE);
        //I18N: loop end
        m_toolbar->SetLabelById(LOOP_END_MEASURE, _("End"));
    }
        
    m_toolbar->realize();
    Refresh();
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
    m_loop_end_measure->Enable(false);
    
    Sequence* seq = getCurrentSequence();
    if (seq->isLoopEnabled())
    {
        MeasureData* md = seq->getMeasureData();
        if (md->getFirstMeasure() > md->getLoopEndMeasure())
        {
            // invalid loop
            md->setLoopEndMeasure(md->getMeasureAmount());
            m_loop_end_measure->SetValue(to_wxString(md->getMeasureAmount()));
        }
    }
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
    m_loop_end_measure->Enable(true);
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

    // last measure
    m_loop_end_measure->SetValue(to_wxString(measData->getLoopEndMeasure() + 1));

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

    m_toolbar->EnableTool(LOOP_CLICKED, true);
    m_toolbar->ToggleTool(LOOP_CLICKED, seq->getModel()->isLoopEnabled());

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

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::loopEndMeasureChanged(wxCommandEvent& evt)
{
    if (getSequenceAmount() == 0) return;

    if (changingValues) return; // discard events thrown because the computer changes values

    int start = atoi_u( m_loop_end_measure->GetValue() );

    if (m_loop_end_measure->GetValue().Length() < 1) return; // text field empty, wait until user enters something to update data

    MeasureData* md = getCurrentSequence()->getMeasureData();

    if (not m_loop_end_measure->GetValue().IsNumber() or start < 0 or start > md->getMeasureAmount())
    {
        wxBell();

        m_loop_end_measure->SetValue(to_wxString(md->getLoopEndMeasure() + 1));
    }
    else
    {
        md->setLoopEndMeasure( start-1 );
    }

    Display::render();
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

#if !defined(__WXOSX_CARBON__)

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
    if (gseq == NULL) return;

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


void MainFrame::addSequence(bool showSongPropertiesDialog)
{
    bool completeProcess;
    int timeSigNumerator;
    int timeSigDenominator;
    
    Sequence* s = new Sequence(this, this, this, this, Display::isVisible());
    completeProcess = true;
    
    if (showSongPropertiesDialog)
    {
        if (SongPropertiesDialogNamespace::show(s, timeSigNumerator, timeSigDenominator)==wxID_CANCEL)
        {
            completeProcess = false;
            wxDELETE(s);
        }
    }

    if (completeProcess)
    {
        GraphicalSequence* gs = new GraphicalSequence(s);
        m_sequences.push_back( gs );
        setCurrentSequence( m_sequences.size() - 1, false /* update */ );
        gs->createViewForTracks(-1 /* all */);
        
        if (showSongPropertiesDialog)
        {
            // Time signature
            MeasureData* measures = s->getMeasureData();
            if (!measures->isExpandedMode())
            {
                ScopedMeasureTransaction tr(measures->startTransaction());
                tr->setTimeSig(timeSigNumerator, timeSigDenominator);
            }
            
            // Sets key from default key
            s->getTrack(0)->setKey(s->getDefaultKeySymbolAmount(), s->getDefaultKeyType());
        }

        m_paused = false;
        updateTopBarAndScrollbarsForSequence( getCurrentGraphicalSequence() );
        updateMenuBarToSequence();
        Display::render();
        getMainFrame()->updateUndoMenuLabel();
    }
   
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

        if (answer == wxCANCEL)
        {
            m_main_pane->SetToolTip(NULL);
            return false;
        }
        
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
    m_main_pane->SetToolTip(NULL);
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


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
void MainFrame::loadFile(const wxString& filePath)
{
    wxString existingFilePath;
    int size;
    bool found;

    size = m_sequences.size();
    found = false;
    
    for (int i=0 ; i<size && !found ; i++)
    {
        existingFilePath = m_sequences[i].getModel()->getFilepath();
        
        if (areFilesIdentical(existingFilePath, filePath))
        {
            setCurrentSequence(i);
            
            if (m_reload_mode)
            {
                reloadFile();
            }
            
            found = true;
        }
    }
    
    if (found)
    {
        // @todo : Should stop playing current sequence ?
        Display::render();
    }
    else
    {
        if (wxFileExists(filePath))
        {
            if (filePath.EndsWith(wxT("aria")))
            {
                loadAriaFile(filePath);
            }
            else if (filePath.EndsWith(wxT("mid")) or filePath.EndsWith(wxT("midi")) or
                     filePath.EndsWith(wxT("MID")) or filePath.EndsWith(wxT("MIDI")))
            {
                loadMidiFile(filePath);
            }
            else
            {
                wxMessageBox(_("Unknown file type: ") + filePath);
            }
        }
    }
}

// ----------------------------------------------------------------------------------------------------------
/** Opens the .aria file in filepath, reads it and prepares the editor to display and edit it. */
void MainFrame::loadAriaFile(const wxString& filePath)
{
    wxLogVerbose( wxT("MainFrame::loadAriaFile") );

    if (filePath.IsEmpty()) return;

    const int old_currentSequence = m_current_sequence;

    addSequence(false);
    setCurrentSequence( getSequenceAmount()-1 );
    getCurrentSequence()->setFilepath( filePath );

    //WaitWindow::show(this, _("Please wait while .aria file is loading.") );

    const bool success = AriaMaestosa::loadAriaFile(getCurrentGraphicalSequence(), getCurrentSequence()->getFilepath());
    if (not success)
    {
        std::cout << "Loading .aria file failed." << std::endl;
        //WaitWindow::hide();
        wxMessageBox(  _("Sorry, loading .aria file failed.") );

        closeSequence();

        return;
    }

    //WaitWindow::hide();
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
    addRecentFile(filePath);
}


// ----------------------------------------------------------------------------------------------------------
 /** Opens the .mid file in filepath, reads it and prepares the editor to display and edit it. */
void MainFrame::loadMidiFile(const wxString& filePath)
{
    wxLogVerbose( wxT("MainFrame::loadMidiFile") );
    if (filePath.IsEmpty()) return;

    const int old_currentSequence = m_current_sequence;

    addSequence(false);
    
    setCurrentSequence( getSequenceAmount()-1 );
    getCurrentSequence()->setFilepath(filePath);

    WaitWindow::show(this, _("Please wait while midi file is loading."));

    std::set<wxString> warnings;
    if (not AriaMaestosa::loadMidiFile( getCurrentGraphicalSequence(), filePath, warnings ) )
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
    getCurrentSequence()->setSequenceFilename( extractTitle(filePath) );

    ASSERT(getCurrentSequence()->invariant());

    // if a song is currently playing, it needs to stay on top
    if (PlatformMidiManager::get()->isPlaying() or m_paused) setCurrentSequence(old_currentSequence);
   
    Display::render();
    
    requestForScrollKeyboardEditorNotesIntoView();
 
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

        setNotificationWarning();

#if wxCHECK_VERSION(2,9,1)
        m_notification_link->Hide();
#endif

        m_notification_text->SetLabel(wxString(full.str().c_str(), wxConvUTF8));
        m_notification_panel->Layout();
        m_notification_panel->GetSizer()->SetSizeHints(m_notification_panel);
        m_notification_panel->Show();
        Layout();

    }
    
    addRecentFile(filePath);
}


// ----------------------------------------------------------------------------------------------------------
void MainFrame::reloadFile()
{
    wxString filePath;
    Sequence* seq;
    bool formerPlaybackMode;
    
    wxLogVerbose( wxT("MainFrame::reloadFile") );
 
    formerPlaybackMode = m_playback_mode;
    if (m_playback_mode)
    {
        wxCommandEvent event;
        stopClicked(event);
    }

    seq = getCurrentSequence();
    filePath = seq->getFilepath();

    if (wxFileExists(filePath))
    {
        if (filePath.EndsWith(wxT("aria")))
        {
            const bool success = AriaMaestosa::loadAriaFile(getCurrentGraphicalSequence(), filePath);
            if (not success)
            {
                std::cout << "Loading .aria file failed." << std::endl;
                wxMessageBox(  _("Sorry, loading .aria file failed.") );
                closeSequence();
                return;
            }
        }
        else if (filePath.EndsWith(wxT("mid")) or filePath.EndsWith(wxT("midi")))
        {
            std::set<wxString> warnings;
            if ( not AriaMaestosa::loadMidiFile(getCurrentGraphicalSequence(), filePath, warnings) )
            {
                std::cout << "Loading midi file failed." << std::endl;
                wxMessageBox(  _("Sorry, loading midi file failed.") );
                closeSequence();
                return;
            }
            else
            {
                requestForScrollKeyboardEditorNotesIntoView();
            }
        }
        
        seq->clearUndoStack();
        updateVerticalScrollbar();
        Display::render();
        if (formerPlaybackMode)
        {
            wxCommandEvent event;
            playClicked(event);
        }
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
    m_main_pane->renderNow();
}


// ----------------------------------------------------------------------------------------------------------

void MainFrame::onMouseClicked()
{
    if (isVolumeSliderShown())
    {
        freeVolumeSlider();
        m_main_pane->renderNow();
    }
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
    m_main_pane->renderNow();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_freeTimeSigPicker( wxCommandEvent& evt )
{
    freeTimeSigPicker();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::evt_showWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::show( this, evt.GetString(), evt.GetInt() );
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
    setNotificationInfo();

#if wxCHECK_VERSION(2,9,1)
    m_notification_link->Show();
#endif

    m_notification_text->SetLabel(wxString(_("A new version of Aria Maestosa is now available! Visit the following link to download it :")));
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

void MainFrame::evt_showTrackContextualMenu(wxCommandEvent& evt)
{
    PopupMenu(m_track_menu);
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
    m_main_pane->renderNow();
}

// ----------------------------------------------------------------------------------------------------------

void MainFrame::disableScrollbars()
{
    m_horizontal_scrollbar->Disable();
    m_vertical_scrollbar->Disable();
}


// ----------------------------------------------------------------------------------------------------------
bool MainFrame::handleApplicationEnd()
{
    bool exitApp;
    
    saveOpenedFiles();
    saveRecentFileList();
    
    exitApp = true;
    
    // the quit menu is greyed out in playback mode, but there are other ways to get this code called
    // (like closing the frame)
    if (m_playback_mode)
    {
        m_main_pane->exitPlayLoop();
    }

    // close all open sequences
    while (getSequenceAmount() > 0 && exitApp)
    {
        if (not closeSequence())
        {
            // user canceled, don't quit
            exitApp = false;
        }
    }
    
    return exitApp;
}


// ----------------------------------------------------------------------------------------------------------
void MainFrame::saveWindowPos()
{
    PreferencesData* pd;
    
    pd = PreferencesData::getInstance();
    if (pd->getBoolValue(SETTING_ID_REMEMBER_WINDOW_POS, false))
    {
        pd->setValue(SETTING_ID_WINDOW_X, to_wxString(GetPosition().x));
        pd->setValue(SETTING_ID_WINDOW_Y, to_wxString(GetPosition().y));
        pd->setValue(SETTING_ID_WINDOW_W, to_wxString(GetSize().GetWidth()));
        pd->setValue(SETTING_ID_WINDOW_H, to_wxString(GetSize().GetHeight()));
        pd->save();
    }
}


// ----------------------------------------------------------------------------------------------------------
void MainFrame::saveOpenedFiles()
{
    PreferencesData* pd;
    wxString files;
    wxString path;
    int count;
    
    pd = PreferencesData::getInstance();
    count = getSequenceAmount();
    for (int i=0 ; i<count ; i++)
    {
        path = getSequence(i)->getFilepath();
        
        if (wxFileExists(path))
        {
            files += path;
            if (i<count-1)
            {
                files += FILE_SEPARATOR;
            }
        }
    }
    
    pd->setValue(SETTING_ID_LAST_SESSION_FILES, files);
    pd->setValue(SETTING_ID_LAST_CURRENT_SEQUENCE, wxString::Format(wxT("%i"), m_current_sequence));
}



// ----------------------------------------------------------------------------------------------------------
void MainFrame::saveRecentFileList()
{
    PreferencesData* pd;
    wxString recentFiles;
    wxString path;
    
    wxMenuItemList menuItemlist = m_recent_files_menu->GetMenuItems();
    wxMenuItemList::iterator iter;
    for (iter = menuItemlist.begin(); iter != menuItemlist.end() ; ++iter)
    {
        path = (*iter)->GetItemLabelText();
        if (wxFileExists(path))
        {
            recentFiles += path + FILE_SEPARATOR;
        }
    }

    if (!recentFiles.IsEmpty())
    {
        recentFiles.RemoveLast();
    }
    pd = PreferencesData::getInstance();
    pd->setValue(SETTING_ID_RECENT_FILES, recentFiles);
}



void MainFrame::requestForScrollKeyboardEditorNotesIntoView()
{
    wxTimer* timer;
    int timerId;
    
    timerId = SCROLL_NOTES_INTO_VIEW_TIMER + getCurrentSequenceID();
    timer = new wxTimer(this, timerId);
    m_timer_map[timerId] = timer;
    Connect(timerId, wxEVT_TIMER, wxTimerEventHandler(MainFrame::onTimer));
    timer->Start(SCROLL_NOTES_INTO_VIEW_DELAY, wxTIMER_ONE_SHOT);
    
    /* Debug only
    std::cout << "----------------------------" << std::endl;
    std::cout << "Creation : " << timerId << " -> " << timer 
        << " seqId : "<< getCurrentSequenceID() << " seq : "<< getCurrentSequence() <<std::endl;
    std::cout << "----------------------------" << std::endl;
    */
}


void MainFrame::scrollKeyboardEditorNotesIntoView(int sequenceId)
{
    int currentSequenceId = getCurrentSequenceID();
    Sequence* seq = getSequence(sequenceId);
    int trackCount = seq->getTrackAmount();
    
    setCurrentSequence(sequenceId);
    if (seq!=NULL)
    {
       for (int i=0 ; i<trackCount ; i++)
        {
           
            seq->getTrack(i)->getGraphics()->scrollKeyboardEditorNotesIntoView();
           
            // Debug only
            //std::cout << "SeqId : " << sequenceId << " - seq : " << seq << " - i : " << i << std::endl;
        }
    }
    setCurrentSequence(currentSequenceId);
}


/** handle case where a path is relative and the other one is absolute (for the same file) */
bool MainFrame::areFilesIdentical(const wxString& filePath1, const wxString& filePath2)
{
    bool identical;
    
    identical = false;
    if (filePath1==filePath2)
    {
        identical = true;
    }
    else
    {
        wxFileName fileName1(filePath1);
        wxFileName fileName2(filePath2);
        
        if (fileName1.IsRelative()) fileName1.MakeAbsolute();
        if (fileName2.IsRelative()) fileName2.MakeAbsolute();
        
#if defined(__WXMSW__)
        // paths with same letters but different cases are considered identical under Windows
        identical = (fileName1.GetFullPath().Lower()==fileName2.GetFullPath().Lower());
#else
        identical = (fileName1.GetFullPath()==fileName2.GetFullPath());
#endif

    }
 
    return identical;
}
   

