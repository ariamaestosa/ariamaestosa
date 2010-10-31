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
#include "main.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "Dialogs/Preferences.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa
{
    class TuningPicker;
    class KeyPicker;
    class DrumChoice;
    class InstrumentChoice;
    
    MainPane* mainPane = NULL;
    
    bool okToLog = true;
    
    namespace Core
    {
        
        void setMainPane(MainPane* pane)
        {
            mainPane = pane;
        }
        
        void activateRenderLoop(bool on)
        {
            wxGetApp().activateRenderLoop(on);
        }
        
        wxString getPrefsValue( const char* entryName )
        {
            return wxGetApp().prefs->getValue( wxString(entryName, wxConvUTF8) );
        }
        
        long getPrefsLongValue( const char* entryName )
        {
            wxString asString = getPrefsValue(entryName);
            if (asString.IsEmpty()) return -1;
            long asInt = -1;
            if (not asString.ToLong(&asInt))
            {
                //ASSERT(false);
                std::cerr << "WARNING: prefs value <" << asString.mb_str() << "> is not an integer\n";
            }
            return asInt;
        }
        
        void setImporting(bool on)
        {
            getMainFrame()->getCurrentSequence()->importing = on;
        }
        TuningPicker* getTuningPicker()
        {
            return getMainFrame()->tuningPicker;
        }
        KeyPicker* getKeyPicker()
        {
            return getMainFrame()->keyPicker;
        }
        DrumPicker* getDrumPicker()
        {
            return getMainFrame()->drumKit_picker;
        }
        InstrumentPicker* getInstrumentPicker()
        {
            return getMainFrame()->instrument_picker;
        }
        
        // TODO: move this into the midi player
        PlayDuringEditMode g_play_during_edit = PLAY_ALWAYS;
        void setPlayDuringEdit(PlayDuringEditMode mode)
        {
            g_play_during_edit = mode;
        }
        PlayDuringEditMode playDuringEdit()
        {
            return g_play_during_edit;
        }
        void songHasFinishedPlaying()
        {
            getMainFrame()->songHasFinishedPlaying();
        }
        
    } // end Core namespace
    
    MainFrame* getMainFrame()
    {
        return wxGetApp().frame;
    }
    
    ICurrentSequenceProvider* g_provider;
    void setCurrentSequenceProvider(ICurrentSequenceProvider* provider)
    {
        g_provider = provider;
    }

    MeasureData* getMeasureData()
    {
        return getCurrentSequence()->measureData;
    }
    
    Sequence* getCurrentSequence()
    {
        return g_provider->getCurrentSequence();
    }
    
    bool isPlaybackMode()
    {
        return getMainFrame()->playback_mode;
    }
    
    namespace Display
    {
        //#ifdef RENDERER_WXWIDGETS
        wxDC* renderDC;
        //#endif
        
        void render()
        {
            mainPane->render();
        }
        int getWidth()
        {
            return mainPane->getWidth();
        }
        int getHeight()
        {
            return mainPane->getHeight();
        }
        bool isMouseDown()
        {
            return mainPane->isMouseDown();
        }
        bool isSelectLessPressed()
        {
            return mainPane->isSelectLessPressed();
        }
        bool isSelectMorePressed()
        {
            return mainPane->isSelectMorePressed();
        }
        bool isCtrlDown()
        {
            return mainPane->isCtrlDown();
        }
        
        void popupMenu(wxMenu* menu, const int x, const int y)
        {
            mainPane->PopupMenu(menu, x, y);
        }
        
        
        RelativeXCoord getMouseX_current()
        {
            return mainPane->getMouseX_current();
        }
        int getMouseY_current()
        {
            return mainPane->getMouseY_current();
        }
        RelativeXCoord getMouseX_initial()
        {
            return mainPane->getMouseX_initial();
        }
        int getMouseY_initial()
        {
            return mainPane->getMouseY_initial();
        }
        
        bool leftArrow()
        {
            return mainPane->leftArrow;
        }
        bool rightArrow()
        {
            return mainPane->rightArrow;
        }
        
        bool isVisible()
        {
            return mainPane->isVisible;
        }
        
        void clientToScreen(const int x_in, const int y_in, int* x_out, int* y_out)
        {
            wxPoint winCoord = mainPane->ClientToScreen(wxPoint(x_in,y_in));
            *x_out = winCoord.x;
            *y_out = winCoord.y;
        }
        void screenToClient(const int x_in, const int y_in, int* x_out, int* y_out)
        {
            wxPoint screenCoord = mainPane->ScreenToClient(wxPoint(x_in,y_in));
            *x_out = screenCoord.x;
            *y_out = screenCoord.y;
        }
        
        void enterPlayLoop()
        {
            mainPane->enterPlayLoop();
        }
        void exitPlayLoop()
        {
            mainPane->exitPlayLoop();
        }
        
        void requestFocus()
        {
            mainPane->SetFocus();
        }
        
        int getDraggedTrackID()
        {
            return mainPane->getDraggedTrackID();
        }
        
        void setPlaybackStartTick(const int tick)
        {
            mainPane->setPlaybackStartTick(tick);
        }
    }// end Display namespace
    
    
    namespace DisplayFrame
    {
        // FIXME: that function shouldn't exist. the MainFrame should register a listener on the measures
        // object and be notified automatically on change.
        void changeMeasureAmount(const int i, bool throwEvent)
        {
            if (getMainFrame() != NULL)
            {
                getMainFrame()->changeMeasureAmount(i, throwEvent);
            }
        }
        void updateHorizontalScrollbar(const int thumbPos)
        {
            if (getMainFrame() != NULL)
            {
                getMainFrame()->updateHorizontalScrollbar(thumbPos);
            }
        }
        void updateVerticalScrollbar()
        {
            if (getMainFrame() != NULL)
            {
                getMainFrame()->updateVerticalScrollbar();
            }
        }
    } // end DisplayFrame namespace
    
    
    bool aboutEqual(const float float1, const float float2)
    {
        float diff = float1 - float2;
        if (diff < 0)        diff = -diff;
        
        if (diff < 1.0/64.0) return true;
        else                 return false;
    }

    bool aboutEqual_tick(const int int1, const int int2)
    {
        return abs(int1 - int2) < getMeasureData()->beatLengthInTicks()/16;
    }
}
