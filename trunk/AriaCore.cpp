#include "AriaCore.h"
#include "main.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "Midi/Sequence.h"
#include "Pickers/TuningPicker.h"
#include "Pickers/KeyPicker.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/InstrumentChoice.h"
#include "wx/wx.h"

namespace AriaMaestosa
{

    MainPane* mainPane = NULL;



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
DrumChoice* getDrumPicker()
{
    return getMainFrame()->drumKit_picker;
}
InstrumentChoice* getInstrumentPicker()
{
    return getMainFrame()->instrument_picker;
}
int playDuringEdit()
{
    return getMainFrame()->play_during_edit;
}
void songHasFinishedPlaying()
{
    getMainFrame()->songHasFinishedPlaying();
}

}

MainFrame* getMainFrame()
{
    return wxGetApp().frame;
}

MeasureData* getMeasureData()
{
    return wxGetApp().frame->getCurrentSequence()->measureData;
}

Sequence* getCurrentSequence()
{
    return getMainFrame()->getCurrentSequence();
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
    void changeMeasureAmount(const int i)
    {
        getMainFrame()->changeMeasureAmount(i);
    }
    void updateHorizontalScrollbar(const int thumbPos)
    {
        getMainFrame()->updateHorizontalScrollbar(thumbPos);
    }
    void updateVerticalScrollbar()
    {
        getMainFrame()->updateVerticalScrollbar();
    }
}


}
