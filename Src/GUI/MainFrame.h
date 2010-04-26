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

#include "wx/wx.h"
//#include "wx/scrolbar.h"
//#include "wx/image.h"
//#include "wx/spinctrl.h"

#include "Utils.h"
#include "AriaCore.h"
#include "ptr_vector.h"

class wxScrollBar;
class wxSpinCtrl;
class wxSpinEvent;

namespace AriaMaestosa
{
    class CustomNoteSelectDialog;
    class Sequence;
    class PreferencesDialog;
    class AboutDialog;
    class InstrumentChoice;
    class DrumChoice;
    class VolumeSlider;
    class TuningPicker;
    class KeyPicker;
    
    // events useful if you need to show a
    // progress bar from another thread
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_SHOW_WAIT_WINDOW,   -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_UPDATE_WAIT_WINDOW, -1)
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_HIDE_WAIT_WINDOW,   -1)
    
    const int SHOW_WAIT_WINDOW_EVENT_ID = 100001;
    const int UPDT_WAIT_WINDOW_EVENT_ID = 100002;
    const int HIDE_WAIT_WINDOW_EVENT_ID = 100003;
    
    
#define MAKE_SHOW_PROGRESSBAR_EVENT(eventname, message, time_known) wxCommandEvent eventname( wxEVT_SHOW_WAIT_WINDOW, SHOW_WAIT_WINDOW_EVENT_ID ); eventname.SetString(message); eventname.SetInt(time_known)
#define MAKE_UPDATE_PROGRESSBAR_EVENT(eventname, progress) wxCommandEvent eventname( wxEVT_UPDATE_WAIT_WINDOW, UPDT_WAIT_WINDOW_EVENT_ID ); eventname.SetInt(progress)
#define MAKE_HIDE_PROGRESSBAR_EVENT(eventname) wxCommandEvent eventname( wxEVT_HIDE_WAIT_WINDOW, HIDE_WAIT_WINDOW_EVENT_ID )
    
#ifndef __WXMAC__
#define NO_WX_TOOLBAR
#endif
    
#ifndef NO_WX_TOOLBAR
    class CustomToolBar : public wxToolBar
    {
    public:
#else
        class CustomToolBar : public wxPanel
        {
            std::vector<wxString> labels;
            wxFlexGridSizer* toolbarSizer;
        public:
            void AddSeparator() {}
            void AddTool(const int id, wxString label, wxBitmap& bmp);
            void SetToolNormalBitmap(const int id, wxBitmap& bmp);
            void EnableTool(const int id, const bool enabled);
            void AddStretchableSpace();
#endif
            CustomToolBar(wxWindow* parent);
            void add(wxControl* ctrl, wxString label=wxEmptyString);
            void realize();
        };
        
        class MainFrame : public wxFrame
        {
            WxOwnerPtr<AboutDialog>  aboutDialog;
            WxOwnerPtr<CustomNoteSelectDialog>  customNoteSelectDialog;
            
            wxFlexGridSizer* borderSizer;
            CustomToolBar* toolbar;
            
            wxScrollBar* horizontalScrollbar;
            wxScrollBar* verticalScrollbar;
            
            wxButton* timeSig;
            wxTextCtrl* firstMeasure;
            wxSpinCtrl* songLength;
            wxSpinCtrl* displayZoom;
            
            wxMenu* fileMenu;
            wxMenu* editMenu;
            wxMenu* settingsMenu;
            wxMenu* trackMenu;
            wxMenu* helpMenu;
            
            wxBitmap playBitmap;
            wxBitmap pauseBitmap;
            wxBitmap tool1Bitmap;
            wxBitmap tool2Bitmap;
            
            wxMenuItem* followPlaybackMenuItem;
            
            wxMenuItem* playDuringEdits_always;
            wxMenuItem* playDuringEdits_onchange;
            wxMenuItem* playDuringEdits_never;
            
            wxMenuItem* channelManagement_automatic;
            wxMenuItem* channelManagement_manual;
            wxMenuItem* expandedMeasuresMenuItem;
            
            int currentSequence;
            ptr_vector<Sequence> sequences; // contains all open sequences
            
        public:
            LEAK_CHECK();
            
            // READ AND WRITE
            bool changingValues; // set this to true when modifying the controls in the top bar, this allows to ignore all events thrown by their modification.
            wxTextCtrl* tempoCtrl;
            
            // ------- read-only -------
            int play_during_edit; // what is the user's preference for note preview during edits
            bool playback_mode;
            MainPane* mainPane;
            WxOwnerPtr<PreferencesDialog> preferences;
            OwnerPtr<InstrumentChoice>    instrument_picker;
            OwnerPtr<DrumChoice>          drumKit_picker;
            OwnerPtr<TuningPicker>        tuningPicker;
            OwnerPtr<KeyPicker>           keyPicker;
            // ----------------------
            
            MainFrame();
            void init();
            void initMenuBar();
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
            void changeMeasureAmount(int i, bool throwEvent=true);
            void disableMenus(const bool disable);
            
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
            
            // ---- playback
            void songHasFinishedPlaying();
            void toolsEnterPlaybackMode();
            void toolsExitPlaybackMode();
            void playClicked(wxCommandEvent& evt);
            void stopClicked(wxCommandEvent& evt);
            
            // ---- I/O
            void updateTopBarAndScrollbarsForSequence(Sequence* seq);
            
            /** Opens the .aria file in filepath, reads it and prepares the editor to display and edit it. */
            void loadAriaFile(wxString path);
            
            /** Opens the .mid file in filepath, reads it and prepares the editor to display and edit it. */
            void loadMidiFile(wxString path);
            
#ifdef __WXMSW__
            void onDropFile(wxDropFilesEvent& event);
#endif
            
            // ---- scrollbars
            void updateVerticalScrollbar();
            
            /** Called to update the horizontal scrollbar, usually because song length has changed. */
            void updateHorizontalScrollbar(int thumbPos=-1);
            
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
            void addSequence();
            
            /** Returns the amount of open sequences (files). */
            int getSequenceAmount();
            
            /**
             * Close an open sequence.
             *
             * @param id  ID of the sequence to close, from 0 to sequence amount -1 (or -1 to mean "current sequence")
             */
            bool closeSequence(int id = -1);
            
            /** Returns the sequence (file) currently being active. */
            Sequence* getCurrentSequence();
            
            Sequence* getSequence(int n);
            int getCurrentSequenceID();
            void setCurrentSequence(int n);
            
            void changeShownTimeSig(int num, int denom);
            
            void evt_freeVolumeSlider( wxCommandEvent& evt );
            void evt_freeTimeSigPicker( wxCommandEvent& evt );
            
            DECLARE_EVENT_TABLE();
        };
        
    }
    
#endif
