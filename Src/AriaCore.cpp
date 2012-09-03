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
#include "PreferencesData.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "version.h"

#include <wx/string.h>
#include <wx/font.h>
#include <wx/dcmemory.h>
#include <wx/thread.h>
#include <wx/url.h>
#include <wx/sstream.h>
#include <wx/protocol/http.h>

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
                
        TuningPicker* getTuningPicker()
        {
            return getMainFrame()->getTuningPicker();
        }
        KeyPicker* getKeyPicker()
        {
            return getMainFrame()->getKeyPicker();
        }
        DrumPicker* getDrumPicker()
        {
            return getMainFrame()->getDrumPicker();
        }
        InstrumentPicker* getInstrumentPicker()
        {
            return getMainFrame()->getInstrumentPicker();
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

    //MeasureData* getMeasureData()
    //{
    //    return getCurrentSequence()->m_measure_data;
    //}
    
    //Sequence* getCurrentSequence()
    //{
    //    return g_provider->getCurrentSequence();
    //}
    
    //GraphicalSequence* getCurrentGraphicalSequence()
    //{
    //    return g_provider->getCurrentGraphicalSequence();
    //}
    
    bool isPlaybackMode()
    {
        return getMainFrame()->isPlaybackMode();
    }
    
    namespace Display
    {
        //#ifdef RENDERER_WXWIDGETS
        wxDC* renderDC;
        //#endif
        
        void render()
        {
            mainPane->renderNow();
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
            return mainPane->isLeftArrowVisible();
        }
        bool rightArrow()
        {
            return mainPane->isRightArrowVisible();
        }
        
        bool isVisible()
        {
            return mainPane->isVisible();
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
        
        void getTextExtents(wxString string, const wxFont& font, wxCoord* txw, wxCoord* txh, wxCoord* descent, wxCoord* externalLeading)
        {
            wxBitmap dummyBmp(5,5);
            wxMemoryDC dummy(dummyBmp);
            ASSERT(dummy.IsOk());
            dummy.SetFont( font );
            dummy.GetTextExtent(string, txw, txh, descent, externalLeading);
        }
        
    }// end Display namespace
    
    
    namespace DisplayFrame
    {
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

    bool aboutEqual_tick(const int int1, const int int2, int beatLength)
    {
        return abs(int1 - int2) < beatLength/16;
    }
    
    
#if 0
#pragma mark -
#endif

    
    class VersionCheckThread : public wxThread
    {
        wxInputStream* m_in;
        wxHTTP* m_http;
        
    public:
        
        VersionCheckThread(wxInputStream* in, wxHTTP* http)
        {
            m_in = in;
            m_http = http;
        }
        
        virtual ExitCode Entry()
        {
            /*
            wxURL url(IS_BETA ? wxT("http://ariamaestosa.sourceforge.net/beta_version.txt")
                              : wxT("http://ariamaestosa.sourceforge.net/stable_version.txt"));
            if (url.GetError() == wxURL_NOERR)
            {
                wxString version_info;
                wxInputStream *in = url.GetInputStream();
                
                if (in and in->IsOk())
                {
                    wxStringOutputStream version_stream(&version_info);
                    in->Read(version_stream);
                    printf("Version : <%s>\n", (const char*)version_info.utf8_str());
                }
                delete in;
            }
            else
            {
                fprintf(stderr, "[VersionCheckThread] WARNING: failed to connect to the server. Error %i\n",
                        (int)url.GetError());
            }
            */
            

            wxString version_info;
            wxStringOutputStream version_stream(&version_info);
            m_in->Read(version_stream);
            
            long version = -1;
            if (version_info.ToLong(&version))
            {
                printf("You have aria %i, latest version is %i\n", VERSION_INT, (int)version);
                if ((int)version > VERSION_INT)
                {
                    printf("You have aria %i, but version %i is now available\n", VERSION_INT, (int)version);
                    while (getMainFrame() == NULL)
                    {
                        wxMilliSleep(100);
                    }
                    
                    wxCommandEvent evt(wxEVT_NEW_VERSION_AVAILABLE, wxID_ANY);
                    getMainFrame()->GetEventHandler()->AddPendingEvent( evt );
                }
            }
            else
            {
                fprintf(stderr, "Invalid version, should be numeric : '%s'\n", (const char*)version_info.utf8_str());
            }
            
            delete m_http;
            return 0;
        }
    };

    
    void checkVersionOnline()
    {
        wxHTTP* http = new wxHTTP();
        http->SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
        http->SetTimeout(10);
        if (not http->Connect(wxT("ariamaestosa.sourceforge.net")))
        {
            fprintf(stderr, "[VersionCheckThread] WARNING: failed to connect to the server.\n");
            return;
        }
        wxInputStream* in = http->GetInputStream(IS_BETA ? wxT("http://ariamaestosa.sourceforge.net/beta_version.txt")
                                                         : wxT("http://ariamaestosa.sourceforge.net/stable_version.txt"));
        
        if (in and in->IsOk())
        {
            VersionCheckThread* v = new VersionCheckThread(in, http);
            v->Create();
            v->Run();
        }
        else
        {
            fprintf(stderr, "[VersionCheckThread] WARNING: failed to retrieve file from server.\n");
            return;
        }
        
        /*
        wxHTTP http;
        http.SetHeader(_T("Content-type"), _T("text/html; charset=utf-8"));
        http.SetTimeout(10);
        if (not http.Connect(wxT("ariamaestosa.sourceforge.net")))
        {
            fprintf(stderr, "[VersionCheckThread] WARNING: failed to connect to the server.\n");
            return;
        }
        wxInputStream* in = http.GetInputStream(IS_BETA ? wxT("http://ariamaestosa.sourceforge.net/beta_version.txt")
                                                        : wxT("http://ariamaestosa.sourceforge.net/stable_version.txt"));
        wxString version_info;
        if (in and in->IsOk())
        {
            wxStringOutputStream version_stream(&version_info);
            in->Read(version_stream);
            printf("Version : <%s>\n", (const char*)version_info.utf8_str());
        }
        else
        {
            fprintf(stderr, "[VersionCheckThread] WARNING: failed to retrieve file from server.\n");
            return;
        }
        */
        /*
        wxURL url(IS_BETA ? wxT("http://ariamaestosa.sourceforge.net/beta_version.txt")
                  : wxT("http://ariamaestosa.sourceforge.net/stable_version.txt"));
        if (url.GetError() == wxURL_NOERR)
        {
            wxString version_info;
            wxInputStream *in = url.GetInputStream();
            
            if (in and in->IsOk())
            {
                wxStringOutputStream version_stream(&version_info);
                in->Read(version_stream);
                printf("Version : <%s>\n", (const char*)version_info.utf8_str());
            }
            delete in;
        }
        else
        {
            fprintf(stderr, "[VersionCheckThread] WARNING: failed to connect to the server. Error %i\n",
                    (int)url.GetError());
        }
        */
    }
}
