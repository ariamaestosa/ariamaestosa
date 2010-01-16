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


#ifndef _ariacore_
#define _ariacore_

#include "Editors/RelativeXCoord.h"
#include "Config.h"

class wxMenu;
class wxDC;

enum
{
    PLAY_ALWAYS = 0,
    PLAY_ON_CHANGE = 1,
    PLAY_NEVER = 2
};

enum ChannelManagementType
{
    CHANNEL_AUTO,
    CHANNEL_MANUAL
};


namespace AriaMaestosa
{
    class MainFrame;
    class MainPane;
    class MeasureData;
    class TuningPicker;
    class Sequence;
    class KeyPicker;
    class DrumChoice;
    class InstrumentChoice;
    
    MainFrame* getMainFrame();
    MeasureData* getMeasureData();
    Sequence* getCurrentSequence();
    
    namespace Core
    {
        void activateRenderLoop(bool on);
        void setMainPane(MainPane* pane);
        void setImporting(bool on);
        int playDuringEdit();
        void songHasFinishedPlaying();
        TuningPicker* getTuningPicker();
        KeyPicker* getKeyPicker();
        DrumChoice* getDrumPicker();
        InstrumentChoice* getInstrumentPicker();
        
        long getPrefsValue( const char* entryName );
    }
    
    namespace Display
    {
        //#ifdef RENDERER_WXWIDGETS
        extern wxDC* renderDC;
        // #endif
        
        void render();
        int getWidth();
        int getHeight();
        bool isMouseDown();
        bool isSelectLessPressed();
        bool isSelectMorePressed();
        bool isCtrlDown();
        bool isVisible();
        
        RelativeXCoord getMouseX_current();
        int getMouseY_current();
        RelativeXCoord getMouseX_initial();
        int getMouseY_initial();
        
        bool leftArrow();
        bool rightArrow();
        void enterPlayLoop();
        void exitPlayLoop();
        
        void popupMenu(wxMenu* menu, const int x, const int y);
        
        void clientToScreen(const int x_in, const int y_in, int* x_out, int* y_out);
        void screenToClient(const int x_in, const int y_in, int* x_out, int* y_out);
        
        void requestFocus();
        
        // FIXME - should be removed/moved
        int getDraggedTrackID();
        void setPlaybackStartTick(const int tick);
    }
    
    namespace DisplayFrame
    {
        void changeMeasureAmount(const int i);
        void updateHorizontalScrollbar(const int thumbPos=-1);
        void updateVerticalScrollbar();
    }
    
    class Singleton
    {
    public:
        Singleton();
        virtual ~Singleton();
        static void deleteAll();
    };
    
}

#endif

