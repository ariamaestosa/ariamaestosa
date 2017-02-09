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

#ifndef __MAIN_FRAME_H__
#define __MAIN_FRAME_H__

/**
 * @defgroup gui
 * @brief This module contains the classes that manage the main frame and its contents.
 * This includes the management of the main pane and its various areas, including the track
 * area (even though the contents of each track is managed by module Editors)
 */

#include <wx/frame.h>
#include <wx/toolbar.h>
#include <wx/panel.h>


#include "AriaCore.h"
#include "GUI/GraphicalSequence.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "ptr_vector.h"
#include "Utils.h"
#include <map>

class wxButton;
class wxScrollBar;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxCommandEvent;
class wxFlexGridSizer;
class wxTextCtrl;
class wxBoxSizer;
class wxStaticBitmap;
class wxTimer;
class wxTimerEvent;

#ifdef __WXMAC__
#include <wx/textctrl.h>
#include <wx/valnum.h>
// workaround until wxWidgets bug #15016 is fixed
class wxWorkaroundSpinCtrl : public wxTextCtrl
{
    int min, max;
public:
    wxWorkaroundSpinCtrl(wxWindow* parent, int id, const wxString& value, const wxPoint& point, const wxSize& size, long style=0)
    : wxTextCtrl(parent, id, value, point, wxSize(45, -1), style/*, wxIntegerValidator<int>()*/)
    {
        min = 0;
        max = 300;
        //dynamic_cast<wxIntegerValidator<int>*>(GetValidator())->SetRange(min, max);
        this->Connect(this->GetId(), wxEVT_KILL_FOCUS, wxFocusEventHandler(wxWorkaroundSpinCtrl::onKillFocus), 0, this);
    }
    
    void SetRange(int a, int b)
    {
        min = a;
        max = b;
        //dynamic_cast<wxIntegerValidator<int>*>(GetValidator())->SetRange(a, b);
    }
    
    void SetValue(int val)
    {
        wxTextCtrl::SetValue(wxString::Format("%i", val));
    }
    
    int GetValue()
    {
        return atoi((const char*)wxTextCtrl::GetValue().utf8_str());
    }
    
    void up(wxSpinEvent& evt)
    {
        int v = GetValue() + 1;
        if (v <= max) 
            SetValue(v);
    }
    void down(wxSpinEvent& evt)
    {
        int v = GetValue() - 1;
        if (v >= min) 
            SetValue(v);
    }

    void onKillFocus(wxFocusEvent& evt)
    {
        long val;
        if (!wxTextCtrl::GetValue().ToLong(&val))
        {
            SetValue(100);
        }

        if (val < min)
            SetValue(min);
        else if (val > max)
            SetValue(max);
    }
};
#define SPINNER_CLASS wxWorkaroundSpinCtrl
#else
#define SPINNER_CLASS wxSpinCtrl
#endif


#if wxCHECK_VERSION(2,9,1)
    class wxGenericHyperlinkCtrl;
#endif


namespace AriaMaestosa
{
    class CustomNoteSelectDialog;
    class Sequence;
    class PreferencesDialog;
    class PreferencesData;
    class AboutDialog;
    class InstrumentChoice;
    class DrumChoice;
    class VolumeSlider;
    class TuningPicker;
    class KeyPicker;

    enum IDs
    {
        PLAY_CLICKED,
        STOP_CLICKED,
        RECORD_CLICKED,
        LOOP_CLICKED,
        TEMPO,
        ZOOM,
        LENGTH,
        BEGINNING,
        LOOP_END_MEASURE,
        TOOL_BUTTON,

        SCROLLBAR_H,
        SCROLLBAR_V,

        TIME_SIGNATURE
    };


    enum MenuIDs
    {

        MENU_FILE_NEW = wxID_HIGHEST+1,
        MENU_FILE_OPEN,
        MENU_FILE_SAVE,
        MENU_FILE_SAVE_AS,
        MENU_FILE_RELOAD,
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
        MENU_EDIT_SCROLL_NOTES_INTO_VIEW,

        MENU_SETTINGS_FOLLOW_PLAYBACK,
        MENU_SETTINGS_PLAY_ALWAYS,
        MENU_SETTINGS_PLAY_NEVER,
        MENU_SETTINGS_MEASURE_EXPANDED,
        MENU_SETTINGS_PLAY_ON_CHANGE,
        MENU_SETTINGS_CHANNELS_AUTO,
        MENU_SETTINGS_CHANNEL_MANUAL,
        MENU_SETTINGS_METRONOME,
        MENU_SETTINGS_PLAYTRHOUGH,

        MENU_TRACK_ADD,
        MENU_TRACK_DUP,
        MENU_TRACK_REMOVE,
        MENU_TRACK_BACKG,

        MENU_PLAY_PAUSE,
        MENU_STOP,
        MENU_RECORD,

        MENU_FILE_LOAD_RECENT_FILE = wxID_HIGHEST + 100,
        MENU_OUTPUT_DEVICE = wxID_HIGHEST + 200,
        MENU_INPUT_DEVICE = wxID_HIGHEST + 300
    };


    // events useful if you need to show a
    // progress bar from another thread
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW,   -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW, -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW,   -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_EXTEND_TICK,        -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_NEW_VERSION_AVAILABLE, -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_ASYNC_ERROR_MESSAGE, -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOW_TRACK_CONTEXTUAL_MENU, -1)

    const int SHOW_WAIT_WINDOW_EVENT_ID = 100001;
    const int UPDT_WAIT_WINDOW_EVENT_ID = 100002;
    const int HIDE_WAIT_WINDOW_EVENT_ID = 100003;
    const int ASYNC_ERR_MESSAGE_EVENT_ID = 100004;

#define MAKE_SHOW_PROGRESSBAR_EVENT(eventname, message, time_known) wxCommandEvent eventname( wxEVT_SHOW_WAIT_WINDOW, SHOW_WAIT_WINDOW_EVENT_ID ); eventname.SetString(message); eventname.SetInt(time_known)
#define MAKE_UPDATE_PROGRESSBAR_EVENT(eventname, progress) wxCommandEvent eventname( wxEVT_UPDATE_WAIT_WINDOW, UPDT_WAIT_WINDOW_EVENT_ID ); eventname.SetInt(progress)
#define MAKE_HIDE_PROGRESSBAR_EVENT(eventname) wxCommandEvent eventname( wxEVT_HIDE_WAIT_WINDOW, HIDE_WAIT_WINDOW_EVENT_ID )
#define MAKE_ASYNC_ERR_MESSAGE_EVENT(eventname) wxCommandEvent eventname( wxEVT_ASYNC_ERROR_MESSAGE, ASYNC_ERR_MESSAGE_EVENT_ID )

#ifdef __WXGTK__
#define NO_WX_TOOLBAR
#endif


#ifdef NO_WX_TOOLBAR
#define TOOLBAR_BASE wxPanel
#else
#define TOOLBAR_BASE wxToolBar
#endif

    /**
      * @ingroup gui
      * @brief   class used to abstract differences between wxMac and wxGTK, providing a non-native
      *          toolbar where the native one is not suitable for Aria
      */
    class CustomToolBar : public TOOLBAR_BASE
    {
        bool m_is_realized;
    public:

#ifdef NO_WX_TOOLBAR
        std::vector<wxString> labels;
        std::vector<int> label_ids;
        std::vector<wxStaticText*> label_widgets;
        wxFlexGridSizer* toolbarSizer;
#endif

    public:

#ifdef NO_WX_TOOLBAR
        void AddSeparator() {}
        void AddTool(const int id, wxString label, wxBitmap& bmp);
        void AddCheckTool(const int id, wxString label, wxBitmap& bmp, bool checked);
        void SetToolNormalBitmap(const int id, wxBitmap& bmp);
        void EnableTool(const int id, const bool enabled);
        void AddStretchableSpace();
        bool GetToolState(int id);
        void ToggleTool(int id, bool pressed);
        void ClearTools();
        void SetLabelById(const int id, wxString label);
        
        void HideById(const int id)
        {
            FindWindow(id)->Hide();
            Layout();
        }
        void ShowById(const int id)
        {
            FindWindow(id)->Show();
            Layout();
        }
#else
        void AddCheckTool(const int id, wxString label, wxBitmap& bmp, bool checked)
        {
            wxToolBar::AddCheckTool(id, label, bmp)->Toggle(checked);
        }
        void SetLabelById(const int id, wxString label)
        {
            FindById(id)->SetLabel(label);
        }
        void HideById(const int id)
        {
            wxControl* control = FindById(id)->GetControl();
            control->SetSize(wxSize(0, -1));
            control->Hide();
        }
        void ShowById(const int id)
        {
            wxControl* control = FindById(id)->GetControl();
            control->SetSize(wxSize(45, -1));
            control->Show();
        }
#endif
        CustomToolBar(wxWindow* parent);
        void add(wxControl* ctrl, wxString label=wxEmptyString);
        void realize();
    };

    /**
      * @ingroup gui
      * @brief manages the main frame of Aria Maestosa
      */
    class MainFrame : public wxFrame, public IPlaybackModeListener, public IActionStackListener,
        public ISequenceDataListener, public ICurrentSequenceProvider, public IMeasureDataListener
    {
        WxOwnerPtr<CustomNoteSelectDialog>  m_custom_note_select_dialog;

        wxPanel* m_main_panel;
        wxFlexGridSizer* m_border_sizer;
        CustomToolBar* m_toolbar;

        wxScrollBar* m_horizontal_scrollbar;
        wxScrollBar* m_vertical_scrollbar;

        wxButton*   m_time_sig;
        wxTextCtrl* m_first_measure;
        wxTextCtrl* m_loop_end_measure;
        SPINNER_CLASS* m_song_length;
        SPINNER_CLASS* m_display_zoom;
        wxTextCtrl* m_tempo_ctrl;

        wxMenuBar* m_menu_bar;

        wxMenu* m_file_menu;
        wxMenu* m_edit_menu;
        wxMenu* m_settings_menu;
        wxMenu* m_track_menu;
        wxMenu* m_help_menu;
        wxMenu* m_output_menu;
        wxMenu* m_input_menu;
        wxMenu* m_playback_menu;
        wxMenu* m_recent_files_menu;
        wxMenuItem* m_recent_files_menu_item;

        wxBitmap m_play_bitmap;
        wxBitmap m_pause_bitmap;
        wxBitmap m_stop_bitmap;
        wxBitmap m_loop_bitmap;
        wxBitmap m_pause_down_bitmap;
        wxBitmap m_record_bitmap;
        wxBitmap m_record_down_bitmap;
        wxBitmap m_tool1_bitmap;
        wxBitmap m_tool2_bitmap;

        wxMenuItem* m_follow_playback_menu_item;

        wxMenuItem* m_play_during_edits_always;
        wxMenuItem* m_play_during_edits_onchange;
        wxMenuItem* m_play_during_edits_never;

        wxMenuItem* m_channel_management_automatic;
        wxMenuItem* m_channel_management_manual;
        wxMenuItem* m_expanded_measures_menu_item;

        wxMenuItem* m_metronome;
        wxMenuItem* m_playthrough;

        wxPanel*         m_notification_panel;
        wxStaticText*    m_notification_text;
        wxStaticBitmap*  m_notification_icon;

#if wxCHECK_VERSION(2,9,1)
        wxGenericHyperlinkCtrl* m_notification_link;
#endif

        void setNotificationWarning();
        void setNotificationInfo();

        wxBoxSizer* m_root_sizer;

        int m_current_sequence;

        /** Contains all open sequences */
        ptr_vector<GraphicalSequence> m_sequences;

        ptr_vector<wxMenuItem, REF>   m_output_device_menus;
        ptr_vector<wxMenuItem, REF>   m_input_device_menus;

        int m_play_during_edit; // what is the user's preference for note preview during edits
        bool m_playback_mode;
        MainPane*                     m_main_pane;
        OwnerPtr<InstrumentPicker>    m_instrument_picker;
        OwnerPtr<DrumPicker>          m_drumKit_picker;
        OwnerPtr<TuningPicker>        m_tuning_picker;
        OwnerPtr<KeyPicker>           m_key_picker;

        bool m_disabled_for_welcome_screen;
        void doDisableMenusForWelcomeScreen(const bool disable);

        wxStaticText* m_status_text;

        bool m_paused;
        int  m_pause_location;
        wxString m_current_dir;
        
        bool m_file_in_command_line;
        
        std::map<int, wxTimer*> m_timer_map;
       
#if !defined(__WXOSX_CARBON__)
        wxStaticBitmap* m_tools_bitmap;
        void onToolsBitmapMousedown(wxMouseEvent& evt);
        void onToolsBitmapMouseup(wxMouseEvent& evt);
#endif
        void updateCurrentDir(wxString& path);

        wxArrayString m_files_to_open;
        bool m_reload_mode;
        
        
        
        void loadAriaFile(const wxString& filePath);
        void loadMidiFile(const wxString& filePath);
        bool handleApplicationEnd();
        void saveWindowPos();
        void saveRecentFileList();
        void saveOpenedFiles();
        void requestForScrollKeyboardEditorNotesIntoView();
        void scrollKeyboardEditorNotesIntoView(int sequenceId);
        bool areFilesIdentical(const wxString& filePath1, const wxString& filePath2);
        void addRecentFile(const wxString& path);
        void fillRecentFilesSubmenu();
        void stackItemUp(wxMenuItemList& menuItemlist, wxMenuItem* menuItem);
        void updateClearListItem();
        wxMenuItem* lookForRecentFileListMenuItem(int menuItemId);

    public:
        LEAK_CHECK();

        // READ AND WRITE
        bool changingValues; // set this to true when modifying the controls in the top bar, this allows to ignore all events thrown by their modification.

        MainFrame();
        void init(const wxArrayString& filesToOpen, bool fileInCommandLine);
        void initMenuBar();
        void initToolbar();
        ~MainFrame();

        // top bar controls updated
        void tempoChanged(wxCommandEvent& evt);

        void updateUndoMenuLabel();

        /** Called whenever the user edits the text field containing song length. */
        void songLengthChanged(wxSpinEvent& evt);
        void zoomChanged(wxSpinEvent& evt);
        void songLengthTextChanged(wxCommandEvent& evt);
        void zoomTextChanged(wxCommandEvent& evt);
        void timeSigClicked(wxCommandEvent& evt);
        void toolButtonClicked(wxCommandEvent& evt);
        //void measureDenomChanged(wxCommandEvent& evt);
        void firstMeasureChanged(wxCommandEvent& evt);
        void loopEndMeasureChanged(wxCommandEvent& evt);
        //void changeMeasureAmount(int i, bool throwEvent=true);
        void disableMenus(const bool disable);

        void disableMenusForWelcomeScreen(const bool disable)
        {
            if (m_disabled_for_welcome_screen == disable) return;
            doDisableMenusForWelcomeScreen(disable);
        }

        void onHideNotifBar(wxCommandEvent& evt);

        void enterPressedInTopBar(wxCommandEvent& evt);

        void setStatusText(wxString text);
        
        void setResizingCursor();
        void setMovingCursor();
        void setNormalCursor();
        void setReloadMode(bool enable);

        // wait window events
        void evt_showWaitWindow(wxCommandEvent& evt);
        void evt_updateWaitWindow(wxCommandEvent& evt);
        void evt_hideWaitWindow(wxCommandEvent& evt);

        // menus
        void on_close(wxCloseEvent& evt);
        void menuEvent_new(wxCommandEvent& evt);
        void menuEvent_loadRecentFile(wxCommandEvent& evt);
        void menuEvent_clearRecentFileList(wxCommandEvent& evt);
        void menuEvent_close(wxCommandEvent& evt);
        void menuEvent_exportNotation(wxCommandEvent& evt);
        void menuEvent_open(wxCommandEvent& evt);
        void menuEvent_copy(wxCommandEvent& evt);
        void menuEvent_undo(wxCommandEvent& evt);
        void menuEvent_paste(wxCommandEvent& evt);
        void menuEvent_pasteAtMouse(wxCommandEvent& evt);
        void menuEvent_save(wxCommandEvent& evt);
        bool doSave();
        void menuEvent_saveas(wxCommandEvent& evt);
        void menuEvent_reload(wxCommandEvent& evt);
        bool doSaveAs();
        void menuEvent_selectNone(wxCommandEvent& evt);
        void menuEvent_selectAll(wxCommandEvent& evt);
        void menuEvent_addTrack(wxCommandEvent& evt);
        void menuEvent_dupTrack(wxCommandEvent& evt);
        void menuEvent_deleteTrack(wxCommandEvent& evt);
        void menuEvent_trackBackground(wxCommandEvent& evt);
        void menuEvent_importmidi(wxCommandEvent& evt);
        void menuEvent_exportmidi(wxCommandEvent& evt);
        void menuEvent_exportSampledAudio(wxCommandEvent& evt);
        void menuEvent_customNoteSelect(wxCommandEvent& evt);
        void menuEvent_snapToGrid(wxCommandEvent& evt);
        void menuEvent_scale(wxCommandEvent& evt);
        void menuEvent_songProperties(wxCommandEvent& evt);
        void menuEvent_preferences(wxCommandEvent& evt);
        void menuEvent_followPlayback(wxCommandEvent& evt);
        void menuEvent_removeOverlapping(wxCommandEvent& evt);
        void menuEvent_scrollNotesIntoView(wxCommandEvent& evt);
        void menuEvent_playAlways(wxCommandEvent& evt);
        void menuEvent_playOnChange(wxCommandEvent& evt);
        void menuEvent_playNever(wxCommandEvent& evt);
        void menuEvent_quit(wxCommandEvent& evt);
        void menuEvent_about(wxCommandEvent& evt);
        void menuEvent_manual(wxCommandEvent& evt);
        void menuEvent_automaticChannelModeSelected(wxCommandEvent& evt);
        void menuEvent_manualChannelModeSelected(wxCommandEvent& evt);
        void menuEvent_expandedMeasuresSelected(wxCommandEvent& evt);
        void menuEvent_metronome(wxCommandEvent& evt);
        void menuEvent_playthrough(wxCommandEvent& evt);
        void menuEvent_outputDevice(wxCommandEvent& evt);
        void menuEvent_inputDevice(wxCommandEvent& evt);

        void menuEvent_playpause(wxCommandEvent& evt);
        void menuEvent_stop(wxCommandEvent& evt);
        void menuEvent_record(wxCommandEvent& evt);

        void onDropFile(wxDropFilesEvent& event);
        void onMouseWheel(wxMouseEvent& event);
        void onShow(wxShowEvent& evt);
        void onTimer(wxTimerEvent & event);

        // ---- playback
        void songHasFinishedPlaying();
        void toolsEnterPlaybackMode();
        void toolsExitPlaybackMode();
        void playClicked(wxCommandEvent& evt);
        void stopClicked(wxCommandEvent& evt);
        void recordClicked(wxCommandEvent& evt);
        void loopClicked(wxCommandEvent& evt);
        bool isPlaybackMode() const { return m_playback_mode; }

        // ---- Pickers
        InstrumentPicker* getInstrumentPicker() { return m_instrument_picker; }
        DrumPicker*       getDrumPicker      () { return m_drumKit_picker;    }
        TuningPicker*     getTuningPicker    ();
        KeyPicker*        getKeyPicker       () { return m_key_picker;        }

        void updateTopBarAndScrollbarsForSequence(const GraphicalSequence* seq);
        void updateTopBarAndScrollbars()
        {
            updateTopBarAndScrollbarsForSequence( getCurrentGraphicalSequence() );
        }
        void updateMenuBarToSequence();

        // ---- I/O
        /** Opens the file in filepath, or sets sequence current, if file already open */
        void loadFile(const wxString& filePath);
        
    
        /** Updates file from disk */
        void reloadFile();


        // ---- scrollbars
        void updateVerticalScrollbar();

        /** Called to update the horizontal scrollbar, usually because song length has changed. */
        void updateHorizontalScrollbar(int thumbPos=-1);

        void disableScrollbars();

        /**
         * User scrolled horizontally by dragging.
         * Just make sure to update the display to the new values.
         */
        void horizontalScrolling(wxScrollEvent& evt);

        /**
         * User scrolled vertically by dragging.
         * Just make sure to update the display to the new values.
         */
        void verticalScrolling(wxScrollEvent& evt);

        /**
         * User scrolled horizontally by clicking oin the arrows.
         * We need to ensure it scrolls of one whole measure (cause scrolling pixel by pixel horizontally would be
         * too slow) We by the way need to make sure it doesn't get out of bounds, in which case we need to put the
         * scrollbar back into correct position.
         */
        void horizontalScrolling_arrows(wxScrollEvent& evt);

        /**
         * User scrolled vertically by clicking on the arrows.
         * Just make sure to update the display to the new values.
         */
        void verticalScrolling_arrows(wxScrollEvent& evt);

        // ---- sequences

        /** Add a new sequence. There can be multiple sequences if user opens or creates multiple files. */
        void addSequence(bool showSongPropertiesDialog);

        /** Returns the amount of open sequences (files). */
        int getSequenceAmount() const { return m_sequences.size(); }

        /**
         * Close an open sequence.
         *
         * @param id  ID of the sequence to close, from 0 to sequence amount -1 (or -1 to mean "current sequence")
         */
        bool closeSequence(int id = -1);

        /** Returns the sequence (file) currently being active. */
        virtual Sequence*  getCurrentSequence();
        Sequence*          getSequence(int n);

        GraphicalSequence* getCurrentGraphicalSequence();
        const GraphicalSequence* getCurrentGraphicalSequence() const;

        GraphicalSequence* getGraphicalSequence(int n);

        MainPane*          getMainPane() { return m_main_pane; }

        int  getCurrentSequenceID() const { return m_current_sequence; }
        void setCurrentSequence(int n, bool update=true);

        void onTimeSigSelectionChanged(int num, int denom);

        void evt_freeVolumeSlider( wxCommandEvent& evt );
        void evt_freeTimeSigPicker( wxCommandEvent& evt );
        void evt_extendTick(wxCommandEvent& evt );
        void evt_newVersionAvailable(wxCommandEvent& evt);
        void evt_asyncErrMessage(wxCommandEvent& evt);
        void evt_showTrackContextualMenu(wxCommandEvent& evt);

        void addIconItem(wxMenu* menu, int menuID, const wxString& label, const wxString& stockIconId);

        /** @brief Implement callback from IPlaybackModeListener */
        virtual void onEnterPlaybackMode();

        /** @brief Implement callback from IPlaybackModeListener */
        virtual void onLeavePlaybackMode();

        /** @brief Implement callback from IActionStackListener */
        virtual void onActionStackChanged();

        /** @brief Implement callback from ISequenceDataListener */
        virtual void onSequenceDataChanged();

        /** @brief Implement callback from IMeasureDataListener */
        virtual void onMeasureDataChange(int change);
        
        void onMouseClicked();

        DECLARE_EVENT_TABLE();
    };

}

#endif
