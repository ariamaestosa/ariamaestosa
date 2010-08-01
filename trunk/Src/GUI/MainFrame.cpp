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

#include "AriaCore.h"
#include "Utils.h"
#include "languages.h"
#include "main.h"

#include "Actions/EditAction.h"
#include "Actions/RemoveOverlapping.h"

#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/MeasureBar.h"

#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/CommonMidiUtils.h"

#include "Editors/KeyboardEditor.h"

#include "GUI/ImageProvider.h"

#include "Dialogs/CustomNoteSelectDialog.h"
#include "Dialogs/WaitWindow.h"
#include "Dialogs/ScaleDialog.h"
#include "Dialogs/CopyrightWindow.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/CustomNoteSelectDialog.h"

#include "Pickers/InstrumentPicker.h"
#include "Pickers/DrumPicker.h"
#include "Pickers/VolumeSlider.h"
#include "Pickers/TuningPicker.h"
#include "Pickers/KeyPicker.h"
#include "Pickers/TimeSigPicker.h"

#include "IO/IOUtils.h"
#include "IO/AriaFileWriter.h"
#include "IO/MidiFileReader.h"

#include "Clipboard.h"
#include "Singleton.h"
#include <iostream>

#include "wx/spinctrl.h"

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#endif


#ifdef __WXMSW__
#include "win32/Aria.xpm"
#endif


using namespace AriaMaestosa;

namespace AriaMaestosa
{
    enum IDs
    {
        PLAY_CLICKED,
        STOP_CLICKED,
        TEMPO,
        ZOOM,
        LENGTH,
        BEGINNING,
        TOOL_BUTTON,

        SCROLLBAR_H,
        SCROLLBAR_V,

        TIME_SIGNATURE
    };


    // events useful if you need to show a progress bar from another thread
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW)
    DEFINE_LOCAL_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW)

}


// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

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
EVT_BUTTON(PLAY_CLICKED, MainFrame::playClicked)
EVT_BUTTON(STOP_CLICKED, MainFrame::stopClicked)
#else
EVT_TOOL(PLAY_CLICKED, MainFrame::playClicked)
EVT_TOOL(STOP_CLICKED, MainFrame::stopClicked)
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

END_EVENT_TABLE()

// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------


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

    for(int n=0; n<label_amount; n++)
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



// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark MainFrame
#endif


#define ARIA_WINDOW_FLAGS wxCLOSE_BOX | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCAPTION

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxT("Aria Maestosa"), wxPoint(100,100), wxSize(900,600),
                                 ARIA_WINDOW_FLAGS )
{
    m_main_panel = new wxPanel(this);

    // FIXME: normally the main panel expands to the size of the parent frame automatically.
    // except on wxMSW when starting maximized *sigh*
    wxBoxSizer* box = new wxBoxSizer(wxHORIZONTAL);
    box->Add(m_main_panel, 1, wxEXPAND | wxALL, 0);
    SetSizer(box);

#ifdef NO_WX_TOOLBAR
    toolbar = new CustomToolBar(m_main_panel);
#else
    toolbar = new CustomToolBar(this);
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
    SetToolBar(toolbar);
#endif
}

#undef ARIA_WINDOW_FLAGS

// --------------------------------------------------------------------------------------------------------

MainFrame::~MainFrame()
{
    ImageProvider::unloadImages();
    PlatformMidiManager::get()->freeMidiPlayer();
    CopyrightWindow::free();
    Clipboard::clear();
    SingletonBase::deleteAll();
#if USE_WX_LOGGING
    dynamic_cast<wxWidgetApp*>(wxTheApp)->closeLogWindow();
#endif
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::init()
{
    changingValues = true;

    Centre();

    currentSequence=0;
    playback_mode=false;

    SetMinSize(wxSize(750, 330));

    initMenuBar();

    wxInitAllImageHandlers();
#ifdef NO_WX_TOOLBAR
    borderSizer = new wxFlexGridSizer(3, 2, 0, 0);
    borderSizer->AddGrowableCol(0);
    borderSizer->AddGrowableRow(1);
#else
    borderSizer = new wxFlexGridSizer(2, 2, 0, 0);
    borderSizer->AddGrowableCol(0);
    borderSizer->AddGrowableRow(0);
#endif

    // a few presets
    wxSize averageTextCtrlSize(wxDefaultSize);
    averageTextCtrlSize.SetWidth(55);

    wxSize smallTextCtrlSize(wxDefaultSize);
    smallTextCtrlSize.SetWidth(35);

    wxSize tinyTextCtrlSize(wxDefaultSize);
    tinyTextCtrlSize.SetWidth(25);

    // -------------------------- Toolbar ----------------------------
#ifdef NO_WX_TOOLBAR
    borderSizer->Add(toolbar, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 2);
    borderSizer->AddSpacer(10);
#endif

    playBitmap.LoadFile( getResourcePrefix()  + wxT("play.png") , wxBITMAP_TYPE_PNG);
    pauseBitmap.LoadFile( getResourcePrefix()  + wxT("pause.png") , wxBITMAP_TYPE_PNG);
    toolbar->AddTool(PLAY_CLICKED, _("Play"), playBitmap);

    wxBitmap stopBitmap;
    stopBitmap.LoadFile( getResourcePrefix()  + wxT("stop.png") , wxBITMAP_TYPE_PNG);
    toolbar->AddTool(STOP_CLICKED, _("Stop"), stopBitmap);

    toolbar->AddSeparator();


    songLength=new wxSpinCtrl(toolbar, LENGTH, to_wxString(DEFAULT_SONG_LENGTH), wxDefaultPosition,
    #if defined(__WXGTK__) || defined(__WXMSW__)
                              averageTextCtrlSize
#else
                              wxDefaultSize
#endif
                              , wxTE_PROCESS_ENTER);
    songLength->SetRange(5, 9999);
    toolbar->add(songLength, _("Duration"));

    tempoCtrl=new wxTextCtrl(toolbar, TEMPO, wxT("120"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER );
    toolbar->add(tempoCtrl, _("Tempo"));

    timeSig = new wxButton(toolbar, TIME_SIGNATURE, wxT("4/4"));
    toolbar->add( timeSig, _("Time Sig") );

    toolbar->AddSeparator();

    firstMeasure=new wxTextCtrl(toolbar, BEGINNING, wxT("1"), wxDefaultPosition, smallTextCtrlSize, wxTE_PROCESS_ENTER);
    toolbar->add(firstMeasure, _("Start"));

    displayZoom=new wxSpinCtrl(toolbar, ZOOM, wxT("100"), wxDefaultPosition,
    #if defined(__WXGTK__) || defined(__WXMSW__)
                           averageTextCtrlSize
    #else
                           wxDefaultSize
    #endif
                           );

    displayZoom->SetRange(25,500);
    toolbar->add(displayZoom, _("Zoom"));

    // seems broken for now
//#if defined(NO_WX_TOOLBAR) || wxMAJOR_VERSION > 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION == 9)
//    toolbar->AddStretchableSpace();
//#else
    toolbar->AddSeparator();
//#endif

    tool1Bitmap.LoadFile( getResourcePrefix()  + wxT("tool1.png") , wxBITMAP_TYPE_PNG);
    tool2Bitmap.LoadFile( getResourcePrefix()  + wxT("tool2.png") , wxBITMAP_TYPE_PNG);
    toolbar->AddTool(TOOL_BUTTON, _("Tool"), tool1Bitmap);

    toolbar->realize();

    // -------------------------- Main Pane ----------------------------
#ifdef RENDERER_OPENGL
    
    int args[3];
    args[0]=WX_GL_RGBA;
    args[1]=WX_GL_DOUBLEBUFFER;
    args[2]=0;
    m_main_pane = new MainPane(m_main_panel, args);
    borderSizer->Add( static_cast<wxGLCanvas*>(m_main_pane), 1, wxEXPAND | wxALL, 2);
    
#elif defined(RENDERER_WXWIDGETS)
    
    m_main_pane = new MainPane(m_main_panel, NULL);
    borderSizer->Add( static_cast<wxPanel*>(m_main_pane), 1, wxEXPAND | wxALL, 2 );
    
#endif

    // give a pointer to our GL Pane to AriaCore
    Core::setMainPane(m_main_pane);

    // -------------------------- Vertical Scrollbar ----------------------------
    verticalScrollbar = new wxScrollBar(m_main_panel, SCROLLBAR_V,
                                        wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);

    verticalScrollbar->SetScrollbar(
                                    0 /*position*/,
                                    530 /*viewable height / thumb size*/,
                                    530 /*height*/,
                                    5 /*scroll amount*/
                                    );

    borderSizer->Add(verticalScrollbar, 1, wxEXPAND | wxALL, 0 );


    // -------------------------- Horizontal Scrollbar ----------------------------
    horizontalScrollbar = new wxScrollBar(m_main_panel, SCROLLBAR_H);
    borderSizer->Add(horizontalScrollbar, 1, wxEXPAND | wxALL, 0);

    // For the first time, set scrollbar manually and not using updateHorizontalScrollbar(), because this method assumes the frame is visible.
    const int editor_size=695, total_size=12*128;

    horizontalScrollbar->SetScrollbar(
                                      horizontalScrollbar->GetThumbPosition(),
                                      editor_size,
                                      total_size,
                                      1
                                      );

    borderSizer->AddSpacer(10);


    // -------------------------- finish ----------------------------

    m_main_panel->SetSizer(borderSizer);
    Centre();

#ifdef __WXMSW__
    // Main frame icon
    wxIcon FrameIcon(Aria_xpm);
       SetIcon(FrameIcon);
#elif defined(__WXGTK__)
    wxIcon ariaIcon(getResourcePrefix()+wxT("/aria64.png"));
    if (not ariaIcon.IsOk())
    {
        fprintf(stderr, "Aria icon not found! (application will have a generic icon)\n");
    }
    else
    {
        SetIcon(ariaIcon);
    }
#endif

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

    // create pickers
    tuningPicker        =  new TuningPicker();
    keyPicker           =  new KeyPicker();
    instrument_picker   =  new InstrumentPicker();
    drumKit_picker      =  new DrumPicker();

    ImageProvider::loadImages();
    m_main_pane->isNowVisible();

    //ImageProvider::loadImages();

#ifdef _show_dialog_on_startup
    if (aboutDialog.raw_ptr == NULL) aboutDialog = new AboutDialog();
    aboutDialog->show();
#endif
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::on_close(wxCloseEvent& evt)
{
    wxCommandEvent dummy;
    menuEvent_quit(dummy);
    //closeSequence();
}

// --------------------------------------------------------------------------------------------------------


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

// ----------------------------------------------------------------------------------------------------------------
// ------------------------------------------------- PLAY/STOP ----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Play/Stop
#endif

void MainFrame::playClicked(wxCommandEvent& evt)
{

    if (playback_mode)
    {
        // already playing, this button does "pause" instead
        getMeasureData()->setFirstMeasure( getMeasureData()->measureAtTick(m_main_pane->getCurrentTick()) );
        m_main_pane->exitPlayLoop();
        updateTopBarAndScrollbarsForSequence( getCurrentSequence() );
        return;
    }

    toolsEnterPlaybackMode();

    int startTick = -1;

    const bool success = PlatformMidiManager::get()->playSequence( getCurrentSequence(), /*out*/ &startTick );
    if (not success) std::cerr << "Couldn't play" << std::endl;

    m_main_pane->setPlaybackStartTick( startTick );

    if (startTick == -1 or not success) m_main_pane->exitPlayLoop();
    else                                m_main_pane->enterPlayLoop();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::stopClicked(wxCommandEvent& evt)
{
    if (not playback_mode) return;
    m_main_pane->exitPlayLoop();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::onEnterPlaybackMode()
{
    toolsEnterPlaybackMode();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::onLeavePlaybackMode()
{
    toolsExitPlaybackMode();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::toolsEnterPlaybackMode()
{
    if (playback_mode) return;

    playback_mode = true;

    toolbar->SetToolNormalBitmap(PLAY_CLICKED, pauseBitmap);
    toolbar->EnableTool(STOP_CLICKED, true);

    disableMenus(true);

    timeSig->Enable(false);
    firstMeasure->Enable(false);
    songLength->Enable(false);
    tempoCtrl->Enable(false);
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::toolsExitPlaybackMode()
{
    playback_mode = false;

    toolbar->SetToolNormalBitmap(PLAY_CLICKED, playBitmap);
    toolbar->EnableTool(STOP_CLICKED, false);

    disableMenus(false);

    timeSig->Enable(true);
    firstMeasure->Enable(true);
    songLength->Enable(true);
    tempoCtrl->Enable(true);
}


// ----------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ TOP BAR -------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Top Bar
#endif

void MainFrame::updateTopBarAndScrollbarsForSequence(Sequence* seq)
{

    changingValues=true; // ignore events thrown while changing values in the top bar

    // first measure
    firstMeasure->SetValue( to_wxString(getMeasureData()->getFirstMeasure()+1) );

    // time signature
    timeSig->SetLabel( wxString::Format(wxT("%i/%i"), getMeasureData()->getTimeSigNumerator(), getMeasureData()->getTimeSigDenominator() ));


    // tempo
    tempoCtrl->SetValue( to_wxString(seq->getTempo()) );

    // song length
    {
        songLength->SetValue( getMeasureData()->getMeasureAmount() );
    }

    // zoom
    displayZoom->SetValue( seq->getZoomInPercent() );

    // set zoom (reason to set it again is because the first time you open it, it may not already have a zoom)
    getCurrentSequence()->setZoom( seq->getZoomInPercent() );

    expandedMeasuresMenuItem->Check( getMeasureData()->isExpandedMode() );

    // scrollbars
    updateHorizontalScrollbar();
    updateVerticalScrollbar();

    changingValues=false;
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::songLengthTextChanged(wxCommandEvent& evt)
{
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

// --------------------------------------------------------------------------------------------------------

void MainFrame::timeSigClicked(wxCommandEvent& evt)
{
    wxPoint pt = wxGetMousePosition();
    showTimeSigPicker( pt.x, pt.y, getMeasureData()->getTimeSigNumerator(), getMeasureData()->getTimeSigDenominator() );
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::firstMeasureChanged(wxCommandEvent& evt)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    int start = atoi_u( firstMeasure->GetValue() );

    if (firstMeasure->GetValue().Length()<1) return; // text field empty, wait until user enters something to update data

    if ( !firstMeasure->GetValue().IsNumber() or start < 0 or start > getMeasureData()->getMeasureAmount() )
    {
        wxBell();

        firstMeasure->SetValue( to_wxString(getMeasureData()->getFirstMeasure()+1) );

    }
    else
    {
        getMeasureData()->setFirstMeasure( start-1 );
    }

}

// --------------------------------------------------------------------------------------------------------

void MainFrame::tempoChanged(wxCommandEvent& evt)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    if (tempoCtrl->GetValue().IsEmpty()) return; // user is probably about to enter something else

    if (!tempoCtrl->GetValue().IsNumber())
    {
        wxBell();
        tempoCtrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
        return;
    }

    int newTempo = atoi_u(tempoCtrl->GetValue());

    if (newTempo<0)
    {
        wxBell();
        tempoCtrl->SetValue( to_wxString(getCurrentSequence()->getTempo()) );
    }
    else if (newTempo>10 && newTempo<1000)
    {
        getCurrentSequence()->setTempo(newTempo);
    }

    // necessary because tempo controller needs to be visually updated whenever tempo changes
    // better code could maybe check if tempo controller is visible before rendering - but rendering is quick anyway so it's not really bad
    Display::render();

}

// --------------------------------------------------------------------------------------------------------

void MainFrame::changeMeasureAmount(int i, bool throwEvent)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    songLength->SetValue(i);
    getMeasureData()->setMeasureAmount(i);

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

// --------------------------------------------------------------------------------------------------------

void MainFrame::changeShownTimeSig(int num, int denom)
{
    changingValues = true; // FIXME - still necessary?
    //measureTypeTop->SetValue( to_wxString(num) );
    //measureTypeBottom->SetValue( to_wxString(denom) );
    timeSig->SetLabel( wxString::Format(wxT("%i/%i"), num, denom ));

    changingValues = false;
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::zoomChanged(wxSpinEvent& evt)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    const int newZoom=displayZoom->GetValue();

    if (newZoom<1 or newZoom>500) return;

    const float oldZoom = getCurrentSequence()->getZoom();

    getCurrentSequence()->setZoom( newZoom );

    const int newXScroll = (int)( horizontalScrollbar->GetThumbPosition()/oldZoom );

    getCurrentSequence()->setXScrollInMidiTicks( newXScroll );
    updateHorizontalScrollbar( newXScroll );
    if (!getMeasureData()->isMeasureLengthConstant()) getMeasureData()->updateMeasureInfo();

    Display::render();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::zoomTextChanged(wxCommandEvent& evt)
{
    // FIXME - AWFUL HACK
    static wxString previousString = wxT("");

    // only send event if the same string is sent twice (i.e. first time, because it was typed in, second time because 'enter' was pressed)
    // or if the same number of chars is kept (e.g. 100 -> 150 will be updated immediatley, but 100 -> 2 will not, since user may actually be typing 250)
    const bool enter_pressed = evt.GetString().IsSameAs(previousString);
    if (enter_pressed or (previousString.Length()>0 and evt.GetString().Length()>=previousString.Length()) )
    {

        if (!evt.GetString().IsSameAs(previousString))
        {
            if (evt.GetString().Length()==1) return; // too short to update now, user probably just typed the first character of something longer
            if (evt.GetString().Length()==2 and atoi_u(evt.GetString())<30 )
                return; // zoom too small, user probably just typed the first characters of something longer
        }

        wxSpinEvent unused;
        zoomChanged(unused); // throw fake event (easier to process all events from a single method)

        // give keyboard focus back to main pane
        if (enter_pressed) m_main_pane->SetFocus();
    }

    previousString = evt.GetString();

}

// --------------------------------------------------------------------------------------------------------

void MainFrame::toolButtonClicked(wxCommandEvent& evt)
{
    std::cout << "toolButtonClicked\n";
    /*
    wxToolBarToolBase* ctrl = toolbar->FindById(TOOL_BUTTON);
    if (ctrl == NULL)
    {
        std::cerr << "Tool is null :(\n";
    }
    else
    {
        wxPoint pos = ctrl->GetPosition();
        std::cout << "Tool pos : " << pos.x << ", " << pos.y << std::endl;
    }*/

    EditTool currTool = Editor::getCurrentTool();
    if (currTool == EDIT_TOOL_PENCIL)
    {
        toolbar->SetToolNormalBitmap(TOOL_BUTTON, tool2Bitmap);
        Editor::setEditTool(EDIT_TOOL_ADD);
    }
    else if (currTool == EDIT_TOOL_ADD)
    {
        toolbar->SetToolNormalBitmap(TOOL_BUTTON, tool1Bitmap);
        Editor::setEditTool(EDIT_TOOL_PENCIL);
    }
    else
    {
        ASSERT (false);
    }

}

// --------------------------------------------------------------------------------------------------------

void MainFrame::enterPressedInTopBar(wxCommandEvent& evt)
{
    // give keyboard focus back to main pane
    m_main_pane->SetFocus();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::songLengthChanged(wxSpinEvent& evt)
{

    if (changingValues) return; // discard events thrown because the computer changes values

    const int newLength=songLength->GetValue();

    if (newLength>0)
    {
        getMeasureData()->setMeasureAmount(newLength);

        updateHorizontalScrollbar();

    }

}

// ----------------------------------------------------------------------------------------------------------------
// -------------------------------------------- SCROLLBARS --------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Scrollbars
#endif

void MainFrame::horizontalScrolling(wxScrollEvent& evt)
{

    // don't render many times at the same location
    //static int last_scroll_position = 0;

    const int newValue = horizontalScrollbar->GetThumbPosition();
    if (newValue == getCurrentSequence()->getXScrollInPixels())return;

    getCurrentSequence()->setXScrollInPixels(newValue);

}

// --------------------------------------------------------------------------------------------------------

void MainFrame::horizontalScrolling_arrows(wxScrollEvent& evt)
{

    const int newValue = horizontalScrollbar->GetThumbPosition();
    const int factor = newValue - getCurrentSequence()->getXScrollInPixels();

    const int newScrollInMidiTicks =
        (int)(
              getCurrentSequence()->getXScrollInMidiTicks() +
              factor * getMeasureData()->defaultMeasureLengthInTicks()
              );

    // check new scroll position is not out of bounds
    const int editor_size=Display::getWidth()-100,
        total_size = getMeasureData()->getTotalPixelAmount();

    const int positionInPixels = (int)( newScrollInMidiTicks*getCurrentSequence()->getZoom() );

    // scrollbar out of bounds
    if ( positionInPixels < 0 )
    {
        updateHorizontalScrollbar( 0 );
        getCurrentSequence()->setXScrollInPixels( 0 );
        return;
    }

    // scrollbar out of bounds
    if ( positionInPixels >= total_size-editor_size)
    {
        updateHorizontalScrollbar();
        return;
    }

    getCurrentSequence()->setXScrollInPixels( positionInPixels );
    updateHorizontalScrollbar( newScrollInMidiTicks );
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::verticalScrolling(wxScrollEvent& evt)
{
    getCurrentSequence()->setYScroll( verticalScrollbar->GetThumbPosition() );
    Display::render();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::verticalScrolling_arrows(wxScrollEvent& evt)
{
    getCurrentSequence()->setYScroll( verticalScrollbar->GetThumbPosition() );
    Display::render();
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::updateHorizontalScrollbar(int thumbPos)
{

    const int editor_size=Display::getWidth()-100,
    total_size = getMeasureData()->getTotalPixelAmount();

    int position =
        thumbPos == -1 ?
        getCurrentSequence()->getXScrollInPixels()
                       :
        (int)(
              thumbPos*getCurrentSequence()->getZoom()
              );

    // if given value is wrong and needs to be changed, we'll need to throw a 'scrolling changed' event to make sure display adapts to new value
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

    horizontalScrollbar->SetScrollbar(
                                      position,
                                      editor_size,
                                      total_size,
                                      1
                                      );

    // scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
    // send a scrolling event that will fix that
    // (internal value will be calculated from scrollbar position)
    if ( changedGivenValue )
    {
        wxScrollEvent evt;
        horizontalScrolling(evt);
    }
}

// --------------------------------------------------------------------------------------------------------

void MainFrame::updateVerticalScrollbar()
{
    int position = getCurrentSequence()->getYScroll();

    const int total_size = getCurrentSequence()->getTotalHeight()+25;
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

    verticalScrollbar->SetScrollbar(
                                    position,
                                    editor_size,
                                    total_size,
                                    5 /*scroll amount*/
                                    );

    // scrollbar needed to be reajusted to fit in bounds, meaning that internal scroll value might be wrong.
    // send a scrolling event that will fix that
    // (internal value will be calculated from scrollbar position)
    if ( changedGivenValue )
    {

        wxScrollEvent evt;
        verticalScrolling(evt);
    }
}

// ----------------------------------------------------------------------------------------------------------------
// ------------------------------------------------- SEQUENCES ----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Sequences
#endif


void MainFrame::addSequence()
{
    sequences.push_back( new Sequence(this, this, this, Display::isVisible()) );
    setCurrentSequence( sequences.size() - 1 );
    Display::render();
    getMainFrame()->updateUndoMenuLabel();
}

// ----------------------------------------------------------------------------------------------------------------

int MainFrame::getSequenceAmount()
{
    return sequences.size();
}

// ----------------------------------------------------------------------------------------------------------------

bool MainFrame::closeSequence(int id_arg) // -1 means current
{

    std::cout << "close sequence called" << std::endl;

    wxString whoToFocusAfter;

    int id = id_arg;
    if (id == -1)
    {
        id = currentSequence;
        if     (id > 0)               whoToFocusAfter = sequences[id - 1].sequenceFileName;
        else if(sequences.size() > 0) whoToFocusAfter = sequences[sequences.size()-1].sequenceFileName;
    }
    else
    {
        whoToFocusAfter = sequences[currentSequence].sequenceFileName;
    }


    if (sequences[id].somethingToUndo())
    {
        wxString message = _("You have unsaved changes in sequence '%s'. Do you want to save them before proceeding?");
        message.Replace(wxT("%s"), sequences[id].sequenceFileName, false);

        int answer = wxMessageBox(  _("Selecting 'Yes' will save your document before closing") +
                                    wxString(wxT("\n")) + _("Selecting 'No' will discard unsaved changes") +
                                    wxString(wxT("\n")) + _("Selecting 'Cancel' will cancel exiting the application"),
                                    message,  wxYES_NO | wxCANCEL, this);

        if (answer == wxCANCEL) return false;

        if (answer == wxYES)
        {
            wxCommandEvent dummy;
            menuEvent_save(dummy);
        }
    }

    sequences.erase( id );

    if (sequences.size() == 0)
    {
        // shut down program (we close last window, so wx will shut down the app)
        Hide();
        Destroy();
        return true;
    }

    int newCurrent = 0;
    const int seqCount = sequences.size();
    for (int n=0; n<seqCount; n++)
    {
        if (sequences[n].sequenceFileName == whoToFocusAfter)
        {
            newCurrent = n;
            break;
        }
    }
    setCurrentSequence(newCurrent);

    //if (sequences.size()>0) Display::render();
    Display::render();
    return true;

}

// ----------------------------------------------------------------------------------------------------------------

Sequence* MainFrame::getCurrentSequence()
{
    ASSERT_E(currentSequence,>=,0);
    ASSERT_E(currentSequence,<,sequences.size());

    return &sequences[currentSequence];
}

// ----------------------------------------------------------------------------------------------------------------

Sequence* MainFrame::getSequence(int n)
{
    ASSERT_E(n,>=,0);
    ASSERT_E(n,<,sequences.size());

    return &sequences[n];
}

// ----------------------------------------------------------------------------------------------------------------

int MainFrame::getCurrentSequenceID()
{
    return currentSequence;
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::setCurrentSequence(int n)
{
    ASSERT_E(n,>=,0);
    ASSERT_E(n,<,sequences.size());

    currentSequence = n;
    updateTopBarAndScrollbarsForSequence( getCurrentSequence() );
    updateMenuBarToSequence();
}


// ----------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------ I/O -----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark I/O
#endif


// FIXME - it sounds very dubious that this task goes in MainFrame

void MainFrame::loadAriaFile(wxString filePath)
{
    if (filePath.IsEmpty()) return;

    const int old_currentSequence = currentSequence;

    addSequence();
    setCurrentSequence( getSequenceAmount()-1 );
    getCurrentSequence()->filepath=filePath;

    WaitWindow::show(_("Please wait while .aria file is loading.") );

    const bool success = AriaMaestosa::loadAriaFile(getCurrentSequence(), getCurrentSequence()->filepath);
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
    getCurrentSequence()->sequenceFileName.set(getCurrentSequence()->filepath.AfterLast('/').BeforeLast('.'));

    // if a song is currently playing, it needs to stay on top
    if (PlatformMidiManager::get()->isPlaying())
    {
        setCurrentSequence(old_currentSequence);
    }
    else
    {
        updateTopBarAndScrollbarsForSequence( getCurrentSequence() );
        updateMenuBarToSequence();
    }

    Display::render();
}

// ----------------------------------------------------------------------------------------------------------------

// FIXME - it sounds very dubious that this task goes in MainFrame
void MainFrame::loadMidiFile(wxString midiFilePath)
{
    if (midiFilePath.IsEmpty()) return;

    const int old_currentSequence = currentSequence;

    addSequence();
    setCurrentSequence( getSequenceAmount()-1 );

    WaitWindow::show( _("Please wait while midi file is loading."));

    if (not AriaMaestosa::loadMidiFile( getCurrentSequence(), midiFilePath ) )
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
    getCurrentSequence()->sequenceFileName.set(midiFilePath.AfterLast('/').BeforeLast('.'));

    // if a song is currently playing, it needs to stay on top
    if (PlatformMidiManager::get()->isPlaying()) setCurrentSequence(old_currentSequence);

    Display::render();
}


// ----------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark various events and notifications
#endif

void MainFrame::onActionStackChanged()
{
    updateUndoMenuLabel();
}

void MainFrame::onSequenceDataChanged()
{
    m_main_pane->render();
}

// ----------------------------------------------------------------------------------------------------------------

/** event sent by the MusicPlayer to notify that it has stopped playing because the song is over. */
void MainFrame::songHasFinishedPlaying()
{
    toolsExitPlaybackMode();
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::evt_freeVolumeSlider( wxCommandEvent& evt )
{
    freeVolumeSlider();
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::evt_freeTimeSigPicker( wxCommandEvent& evt )
{
    freeTimeSigPicker();
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::evt_showWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::show( evt.GetString(), evt.GetInt() );
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::evt_updateWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::setProgress( evt.GetInt() );
}

// ----------------------------------------------------------------------------------------------------------------

void MainFrame::evt_hideWaitWindow(wxCommandEvent& evt)
{
    WaitWindow::hide();
}

