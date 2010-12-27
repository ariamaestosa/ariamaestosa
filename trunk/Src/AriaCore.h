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


#ifndef __ARIA_CORE_H__
#define __ARIA_CORE_H__

#include "Editors/RelativeXCoord.h"
#include "Utils.h"

class wxMenu;
class wxDC;

enum PlayDuringEditMode
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
    class DrumPicker;
    class InstrumentPicker;
    class GraphicalSequence;
    
    extern bool okToLog;
    
    // TODO: remove
    class ICurrentSequenceProvider
    {
    public:
        virtual Sequence* getCurrentSequence() = 0;
        virtual GraphicalSequence* getCurrentGraphicalSequence() = 0;
        virtual ~ICurrentSequenceProvider() {}
    };
    
    MainFrame*         getMainFrame();
    //MeasureData*       getMeasureData();
    //Sequence*          getCurrentSequence();
    //GraphicalSequence* getCurrentGraphicalSequence();
    
    void         setCurrentSequenceProvider(ICurrentSequenceProvider* provider);
    
    bool isPlaybackMode();
    
    namespace Core
    {
        void activateRenderLoop(bool on);
        void setMainPane(MainPane* pane);
        void setImporting(bool on);
        
        PlayDuringEditMode playDuringEdit();
        void setPlayDuringEdit(PlayDuringEditMode mode);
        
        void songHasFinishedPlaying();
        TuningPicker*     getTuningPicker();
        KeyPicker*        getKeyPicker();
        DrumPicker*       getDrumPicker();
        InstrumentPicker* getInstrumentPicker();
        
        wxString getPrefsValue( const char* entryName );
        long getPrefsLongValue( const char* entryName );
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
        
        // FIXME(DESIGN) - should be removed/moved
        int getDraggedTrackID();
        void setPlaybackStartTick(const int tick);
    }
    
    namespace DisplayFrame
    {
        void changeMeasureAmount(const int i, bool throwEvent=true);
        void updateHorizontalScrollbar(const int thumbPos=-1);
        void updateVerticalScrollbar();
    }
    
    
    /**
     * @return whether two values are approximately equal. this is because there is no midi
     * standard for note length and i have seen some midis use note lengths slightly different
     * from those Aria uses. This is why i check for approximate lengths, otherwise the score
     * view may end up messed up when you import a midi made in another editor
     */
    bool aboutEqual(const float float1, const float float2);
    
    /**
     * @return whether two values are approximately equal. this is because there is no midi
     * standard for note length and i have seen some midis use note lengths slightly different
     * from those Aria uses. This is why i check for approximate lengths, otherwise the score
     * view may end up messed up when you import a midi made in another editor
     */
    bool aboutEqual_tick(const int int1, const int int2, int beatLength);
}

#endif

