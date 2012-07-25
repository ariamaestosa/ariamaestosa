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

#include "Editors/Editor.h"

#include "AriaCore.h"
#include "Actions/DeleteSelected.h"
#include "Actions/Duplicate.h"
#include "Actions/MoveNotes.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/ImageProvider.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "IO/IOUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/MagneticGridPicker.h"
#include "Pickers/VolumeSlider.h"
#include "Renderers/Drawable.h"
#include "Renderers/RenderAPI.h"
#include "UnitTest.h"
#include "Utils.h"

#include <wx/tokenzr.h>

namespace AriaMaestosa
{
    EditTool g_current_edit_tool = EDIT_TOOL_PENCIL;
}

using namespace AriaMaestosa;

const int SCROLLBAR_BOTTOM_MARGIN = 1;

// TODO: get these numbers directly from the images instead of hardcoding them?
#ifdef __WXMSW__
const int SCROLLBAR_X = -29; // 20 pixels for the scrollbar itself + 9 pixels right margin
const int SCROLLBAR_ARROW_HEIGHT = 21;
const int SCROLLBAR_THUMB_HEIGHT = 30;
#else
const int SCROLLBAR_X = -24; // 15 pixels for the scrollbar itself + 9 pixels right margin
const int SCROLLBAR_ARROW_HEIGHT = 17;
const int SCROLLBAR_THUMB_HEIGHT = 23;
#endif

// ------------------------------------------------------------------------------------------------------------

Editor::Editor(GraphicalTrack* track)
{
    m_y_step = 10;

    m_track           = track->getTrack();
    m_sequence        = m_track->getSequence();
    m_graphical_track = track;
    m_gsequence       = track->getSequence();

    m_vertical_scrolling = false;
    m_click_on_scrollbar = false;

    m_scroll_up_arrow_pressed   = false;
    m_scroll_down_arrow_pressed = false;

    m_click_on_scrollbar = false;

    m_selecting = false;
    m_use_vertical_scrollbar = true;

    m_mouse_is_in_editor  = false;
    m_clicked_on_note     = false;
    m_last_clicked_note   = -1;
    m_use_instant_notes   = false;
    m_is_duplicating_note = false;

    m_relative_height = 1.0f;
}

// ------------------------------------------------------------------------------------------------------------

Editor::~Editor()
{
}

// ------------------------------------------------------------------------------------------------------------

void Editor::useInstantNotes(bool enabled)
{
    ASSERT( MAGIC_NUMBER_OK() );
    m_use_instant_notes = enabled;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::useVerticalScrollbar(const bool useScrollbar)
{
    ASSERT( MAGIC_NUMBER_OK() );
    Editor::m_use_vertical_scrollbar = useScrollbar;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ render ----------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Render
#endif

void Editor::render()
{
    ASSERT( MAGIC_NUMBER_OK() );
    render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

// ------------------------------------------------------------------------------------------------------------

void Editor::drawVerticalMeasureLines(const int from_y, const int to_y)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    AriaRender::primitives();
    AriaRender::lineWidth(1);
    
    MeasureData* md = m_sequence->getMeasureData();
    MeasureBar* mb = m_gsequence->getMeasureBar();
    
    const int measure       = mb->measureAtPixel( Editor::getEditorXStart() );
    const int start_x       = mb->firstPixelInMeasure( measure );
    const int measureAmount = md->getMeasureAmount();
    const float beatLength  = m_sequence->ticksPerBeat() * m_gsequence->getZoom();
    float mx                = start_x;
    const int measureID     = mb->measureAtPixel( Editor::getEditorXStart() );
    float new_mx;

    for (int m=measureID; m<measureAmount; m++)
    {
        new_mx = mb->firstPixelInMeasure(m);

        // draw pale lines
        AriaRender::color(0.9, 0.9, 0.9);
        for (; mx < new_mx; mx += beatLength)
        {
            AriaRender::line( (int)round(mx), from_y, (int)round(mx), to_y);
        }
        mx = new_mx;

        // draw strong line
        AriaRender::color(0.5, 0.5, 0.5);
        AriaRender::line((int)round(mx), from_y, (int)round(mx), to_y);
        mx += beatLength;

        if (mx > Display::getWidth()) break;

    }//next

    // draw lines till end of screen if we're not there yet
    const int end_of_screen = Display::getWidth();
    if (mx < end_of_screen)
    {
        AriaRender::color(0.9, 0.9, 0.9);
        for (;mx<end_of_screen; mx +=beatLength)
        {
            AriaRender::line((int)round(mx), from_y, (int)round(mx), to_y);
        }//next

    }//end if

    // close last measure with a strong line
    new_mx = mb->lastPixelInMeasure(measureAmount-1);
    AriaRender::color(0.5, 0.5, 0.5);
    AriaRender::line((int)round(new_mx), from_y, (int)round(new_mx), to_y);
}

// ------------------------------------------------------------------------------------------------------------

void Editor::renderScrollbar()
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    if (not m_use_vertical_scrollbar) return;

    AriaRender::images();

    // ---- scrollbar background -----
    sbBackgDrawable->move(getWidth() + SCROLLBAR_X, getEditorYStart());
    sbBackgDrawable->scale(1, m_height/sbBackgDrawable->getImageHeight());
    sbBackgDrawable->render();

    // ------ top arrow ------
    sbArrowDrawable->setFlip(false, false);
    sbArrowDrawable->move(getWidth() + SCROLLBAR_X, getEditorYStart());

    const int mouseX  = Display::getMouseX_initial().getRelativeTo(WINDOW);
    const int mouseX2 = Display::getMouseX_current().getRelativeTo(WINDOW);

    const int mouseY  = Display:: getMouseY_initial();
    const int mouseY2 = Display:: getMouseY_current();

    if (not m_vertical_scrolling and // ignore arrows if user is dragging thumb
       Display:: isMouseDown() and // only if mouse is down
       mouseX  > sbArrowDrawable->getX() and mouseX2 > sbArrowDrawable->getX() and // and mouse is located on the arrow
       mouseY  > sbArrowDrawable->getY() and mouseY2 > sbArrowDrawable->getY() and
       mouseX  < sbArrowDrawable->getX() + sbArrowDrawable->getImageWidth() and
       mouseX2 < sbArrowDrawable->getX() + sbArrowDrawable->getImageWidth() and
       mouseY  < sbArrowDrawable->getY() + sbArrowDrawable->getImageHeight() and
       mouseY2 < sbArrowDrawable->getY() + sbArrowDrawable->getImageHeight())
    {
        sbArrowDrawable->setImage( sbArrowDownImg );
        m_scroll_up_arrow_pressed = true;
        m_click_on_scrollbar      = true;
    }
    else
    {
        sbArrowDrawable->setImage( sbArrowImg );
        m_scroll_up_arrow_pressed = false;
    }

    sbArrowDrawable->render();

    // ------ bottom arrow ------
    sbArrowDrawable->setFlip(false, true);
    sbArrowDrawable->move(getWidth() + SCROLLBAR_X,
                          m_from_y + m_height - sbArrowDrawable->getImageHeight() - SCROLLBAR_BOTTOM_MARGIN);

    if (not m_vertical_scrolling and // ignore arrows if user is dragging thumb
        Display:: isMouseDown() and // only if mouse is down
        mouseX  > sbArrowDrawable->getX() and mouseX2 > sbArrowDrawable->getX() and // and mouse is located on the arrow
        mouseY  > sbArrowDrawable->getY() and mouseY2 > sbArrowDrawable->getY() and
        mouseX  < sbArrowDrawable->getX() + sbArrowDrawable->getImageWidth() and
        mouseX2 < sbArrowDrawable->getX() + sbArrowDrawable->getImageWidth() and
        mouseY  < sbArrowDrawable->getY() + sbArrowDrawable->getImageHeight() and
        mouseY2 < sbArrowDrawable->getY() + sbArrowDrawable->getImageHeight())
    {

        sbArrowDrawable->setImage( sbArrowDownImg );
        m_scroll_down_arrow_pressed=true;
        m_click_on_scrollbar = true;

    }
    else
    {

        sbArrowDrawable->setImage( sbArrowImg );
        m_scroll_down_arrow_pressed=false;

    }

    sbArrowDrawable->render();

    // -------- thumb ------
    const int thumb_from_y = getEditorYStart() + SCROLLBAR_ARROW_HEIGHT;
    const int net_scroll_height = m_height - SCROLLBAR_THUMB_HEIGHT - SCROLLBAR_ARROW_HEIGHT*2 - SCROLLBAR_BOTTOM_MARGIN;
    sbThumbDrawable->move(getWidth() + SCROLLBAR_X, (int)(thumb_from_y + net_scroll_height*m_sb_position));
    sbThumbDrawable->render();

}

// ------------------------------------------------------------------------------------------------------------
// ----------------------------------------- background handling  ---------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Background view handling
#endif

// ------------------------------------------------------------------------------------------------------------

void Editor::clearBackgroundTracks()
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    m_background_tracks.clearWithoutDeleting();
}

// ------------------------------------------------------------------------------------------------------------

void Editor::addBackgroundTrack(Track* track)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    m_background_tracks.push_back(track);
}

// ------------------------------------------------------------------------------------------------------------

bool Editor::hasAsBackground(Track* track)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    const int bgTrackAmount = m_background_tracks.size();

    for (int m=0; m<bgTrackAmount; m++)
    {
        if (m_background_tracks.get(m) == track) return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::trackDeleted(Track* track)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    //Sequence* seq = getCurrentSequence();
    const int bgTrackAmount = m_background_tracks.size();

    for (int m=0; m<bgTrackAmount; m++)
    {
        if ( m_background_tracks.get(m) == track )
        {
            m_background_tracks.markToBeRemoved(m);
        }
    }//next

    m_background_tracks.removeMarked();
}

// ------------------------------------------------------------------------------------------------------------


bool Editor::isBackgroundTrack() const
{
    return m_background_tracks.size()>0;
}
 

void Editor::setBackgroundTracks(const wxString& backgroundTracks)
{
    m_background_tracks_temp_string = backgroundTracks;
}

 
// ------------------------------------------------------------------------------------------------------------


wxString Editor::getBackgroundTracks()
{
    wxString tracks;
    const int bgTrackAmount = m_background_tracks.size();

    for (int m=0; m<bgTrackAmount; m++)
    {
        tracks += wxString::Format(wxT("%i"), m_background_tracks.get(m)->getId());
        
        if (m<bgTrackAmount-1)
        {
            tracks += wxT(",");
        }
    }
    
    return tracks;
}


// ------------------------------------------------------------------------------------------------------------


void Editor::addBackgroundTracks()
{
    if (!m_background_tracks_temp_string.IsEmpty())
    {
        int trackCount;
        int trackId;
        bool found;
        wxStringTokenizer tokenizer(m_background_tracks_temp_string, wxT(","));
        trackCount = m_sequence->getTrackAmount();
        while ( tokenizer.HasMoreTokens() )
        {
            trackId = wxAtoi(tokenizer.GetNextToken());
            found = false;
            for (int i=0 ; i<trackCount && !found; i++)
            {
                Track* track = m_sequence->getTrack(i);
                if (track->getId()==trackId)
                {
                    addBackgroundTrack(track);
                    found = true;
                }
            }
        }
    }
}


// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------- mouse events ---------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Mouse Events
#endif

void Editor::mouseDown(RelativeXCoord x, int y)
{
    ASSERT( MAGIC_NUMBER_OK() );
    m_selecting = false;

    m_last_drag_y = y;

    if (m_use_vertical_scrollbar)
    {
        // check if user is grabbing the scroll bar
        if (x.getRelativeTo(WINDOW) > sbThumbDrawable->getX() and
            x.getRelativeTo(WINDOW) < (sbThumbDrawable->getX() + sbThumbDrawable->getImageWidth()) and
            y < getYEnd()-15 and y > getEditorYStart())
        {
            m_click_on_scrollbar = true;

            // grabbing the thumb
            const int thumb_from_y = getEditorYStart() + SCROLLBAR_ARROW_HEIGHT;
            const int net_scroll_height = m_height - SCROLLBAR_THUMB_HEIGHT - SCROLLBAR_ARROW_HEIGHT*2 - SCROLLBAR_BOTTOM_MARGIN;
            const int thumb_pos = (int)(thumb_from_y + net_scroll_height*m_sb_position);
            if (y > thumb_pos and y < thumb_pos + sbThumbDrawable->getImageHeight())
            {
                m_vertical_scrolling = true;
            }

        }
        else
        {
            m_click_on_scrollbar = false;
        }

    }


    // check if click is whithin editable areas
    m_mouse_is_in_editor = false;

    if (y < getYEnd() - 5 and y > getEditorYStart())
    {
        if (m_use_vertical_scrollbar and x.getRelativeTo(WINDOW) < getWidth() + SCROLLBAR_X)
        {
            m_mouse_is_in_editor = true;
        }
        else if (not m_use_vertical_scrollbar and x.getRelativeTo(WINDOW) < getXEnd())
        {
            m_mouse_is_in_editor = true;
        }
    }
    
    m_clicked_on_note = false;

    // check if user clicked on a note
    if (m_mouse_is_in_editor)
    {
        const NoteSearchResult result = noteAt(x, y, m_last_clicked_note);
        if (result == FOUND_NOTE)
        {
            m_clicked_on_note = true;
            m_graphical_track->selectNote(ALL_NOTES, false);
            m_graphical_track->selectNote(m_last_clicked_note, true);
            m_track->playNote(m_last_clicked_note);
            
            if (getMainFrame()->getMainPane()->isCommandDown())
            {
                m_is_duplicating_note = true;
                m_track->action( new Action::Duplicate(this) );
                
                // perform the following operations on selected notes (the duplicate action will
                // have changed which note is selected)
                m_last_clicked_note = -1;
            }
        }
        else if (result == FOUND_SELECTED_NOTE)
        {
            m_clicked_on_note = true;
            m_track->playNote( m_last_clicked_note, false );
            
            if (getMainFrame()->getMainPane()->isCommandDown())
            {
                m_is_duplicating_note = true;
                m_track->action( new Action::Duplicate(this) );
            }
            else if (Display::isSelectMorePressed())
            {
                m_track->selectNote(m_last_clicked_note, not m_track->isNoteSelected(m_last_clicked_note), true);
            }
            
            // 'noteAt' set 'm_last_clicked_note' to the ID of the note that was clicked. However at this point
            // it is not important anymore to know precisely which one was clicked, because all future
            // operations that can be done will be applied to all selected notes. therefore we set
            // m_last_clicked_note to -1, meaning 'selected notes'.
            m_last_clicked_note = -1;
        }

    }

    m_selecting = false;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                       RelativeXCoord mousex_initial, int mousey_initial)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    if (m_use_vertical_scrollbar)
    {

        // -------------------  drag scrollbar  ---------------
        if (m_vertical_scrolling and m_click_on_scrollbar and
            mousey_initial < getYEnd() - 15 and mousey_initial > getEditorYStart())
        {
            // if we reach scrollbar extremity, wait until mouse comes back to the middle of the thumb before starting to scroll again
            if (m_sb_position == 0 and mousey_current < getEditorYStart() + 25)
            {
                m_last_drag_y = mousey_current;
                return;
            }
            if (m_sb_position == 1 and mousey_current > getYEnd()-25)
            {
                m_last_drag_y = mousey_current;
                return;
            }

            const int net_scroll_height = m_height - SCROLLBAR_THUMB_HEIGHT - SCROLLBAR_ARROW_HEIGHT*2 - SCROLLBAR_BOTTOM_MARGIN;
            m_sb_position += (mousey_current - m_last_drag_y)/(float)(net_scroll_height);

            if (m_sb_position < 0) m_sb_position = 0;
            if (m_sb_position > 1) m_sb_position = 1;
        }
    }

    m_last_drag_y = mousey_current;

    // if user did not click on note and moved the mouse vertically, then he is selecting
    if (not m_clicked_on_note and abs(mousey_current-mousey_initial) > m_y_step*2 and m_mouse_is_in_editor)
    {
        m_selecting = true;
    }

    // if selection becomes thin again, come back in note add mode
    if (m_selecting and abs(mousey_current-mousey_initial) < m_y_step)
    {
        m_selecting = false;
    }
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial)
{
    ASSERT( MAGIC_NUMBER_OK() );
        
    if (m_use_vertical_scrollbar)
    {
        m_vertical_scrolling = false;
        m_click_on_scrollbar = false;
    }
    
    if (m_mouse_is_in_editor)
    {
        if (not m_clicked_on_note)
        {

            // --------------------------  select notes  ---------------------
            if (m_selecting)
            {
                selectNotesInRect(mousex_current, mousey_current, mousex_initial, mousey_initial);
            }
            else
            {
                // --------------------------  add note  --------------------------


                if (m_use_vertical_scrollbar)
                {
                    if (mousex_current.getRelativeTo(WINDOW) > sbThumbDrawable->getX() or
                        mousex_initial.getRelativeTo(WINDOW) > sbThumbDrawable->getX())
                    {
                        goto end_of_func;
                    }
                }
                else
                {
                    if (mousex_current.getRelativeTo(WINDOW) > getXEnd() or
                        mousex_initial.getRelativeTo(WINDOW) > getXEnd())
                    {
                        goto end_of_func;
                    }
                }
                
                int snapped_start = m_track->snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI), true );
                const int len = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
                int snapped_end   = snapped_start + m_track->snapMidiTickToGrid(len, false );
                
                // reject empty/malformed notes, add on success
                if (m_use_instant_notes) // for drums
                {
                    if (snapped_end < 0)
                    {
                        m_graphical_track->selectNote(ALL_NOTES, false);
                        goto end_of_func;
                    }
                    addNote( snapped_end, mousey_initial );
                }
                else
                {
                    // click without moving
                    if (g_current_edit_tool == EDIT_TOOL_ADD and snapped_start == snapped_end) 
                    {
                        int notelen = m_sequence->ticksPerBeat()*4 / m_track->getMagneticGrid()->getDivider();
                        if (m_track->getMagneticGrid()->isDotted()) notelen = notelen*1.5f;
                        addNote(snapped_start,
                                snapped_start + notelen,
                                mousey_initial );
                    }
                    else if (snapped_start == snapped_end or snapped_start>snapped_end or snapped_start<0)
                    {
                        m_graphical_track->selectNote(ALL_NOTES, false);
                        goto end_of_func;
                    }
                    else
                    {
                        addNote( snapped_start, snapped_end, mousey_initial );
                    }
                }

            }

        }

        // ---------------------------  move note  -----------------------

        if (m_clicked_on_note)
        {

            const int  x_difference = mousex_current.getRelativeTo(MIDI) - mousex_initial.getRelativeTo(MIDI);
            const int  y_difference = mousey_current-mousey_initial;

            const int relativeX = m_track->snapMidiTickToGrid(x_difference, false);
            const int relativeY = (int)round( (float)y_difference / (float)m_y_step);

            if (relativeX == 0 and relativeY == 0)
            {
                goto end_of_func;
            }

            if (m_is_duplicating_note)
            {
                Action::EditAction* action = m_track->getSequence()->getLatestAction();
                makeMoveNoteEvent(relativeX, relativeY, m_last_clicked_note, dynamic_cast<Action::Duplicate*>(action));
            }
            else
            {
                makeMoveNoteEvent(relativeX, relativeY, m_last_clicked_note);
            }
            
        }// end if clicked on note

    }//end if m_mouse_is_in_editor


end_of_func:

    m_selecting           = false;
    m_mouse_is_in_editor  = false;
    m_clicked_on_note     = false;
    m_is_duplicating_note = false;
    Display::render();
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    if (m_selecting) selectNotesInRect(mousex_current, mousey_current, mousex_initial, mousey_initial);
    m_selecting = false;
    
    m_mouse_is_in_editor = false;
    m_clicked_on_note = false;
    m_is_duplicating_note = false;
    
    Display::render();
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                           RelativeXCoord mousex_initial, int mousey_initial)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    // ------------------------------- horizontal scroll by pushing mouse to side ------------------------------
    if (not m_click_on_scrollbar and m_selecting and m_mouse_is_in_editor)
    {
        mousex_initial.convertTo(MIDI);

        // scroll forward
        if (mousex_current.getRelativeTo(WINDOW) > getXEnd()-75)
        {
            m_gsequence->setXScrollInPixels(m_gsequence->getXScrollInPixels()+
                                            (mousex_current.getRelativeTo(WINDOW)-getXEnd()+75)/5 );
            DisplayFrame::updateHorizontalScrollbar();
            Display::render();
            return;
        }
        else if (mousex_current.getRelativeTo(WINDOW) < Editor::getEditorXStart()+20)
            // scroll backwards
        {
            const int new_scroll_value = m_gsequence->getXScrollInPixels() -
                    (Editor::getEditorXStart() + 20 - mousex_current.getRelativeTo(WINDOW))/4;
            
            if (new_scroll_value > 0)
            {
                m_gsequence->setXScrollInPixels( new_scroll_value );
                DisplayFrame::updateHorizontalScrollbar();
                Display::render();
            }
            return;
        }
    }



    // -------------- check if user is clicking on scroll bar, but not grabbing the thumb ----------------
    if (m_use_vertical_scrollbar)
    {
        //m_click_on_scrollbar = false;

        // make thumb slide to mouse
        if (mousex_current.getRelativeTo(WINDOW) > sbThumbDrawable->getX() and
            mousex_current.getRelativeTo(WINDOW) < sbThumbDrawable->getX() + sbThumbDrawable->getImageWidth() and
            mousey_current < getYEnd() - SCROLLBAR_BOTTOM_MARGIN - SCROLLBAR_ARROW_HEIGHT and
            mousey_current > getEditorYStart() + SCROLLBAR_ARROW_HEIGHT)
        {

            //m_click_on_scrollbar = true;
            
            // not grabbing the thumb
            if (
               not m_scroll_up_arrow_pressed and
               not m_scroll_down_arrow_pressed and
               not m_vertical_scrolling and
               m_click_on_scrollbar
               )
            {

                // find where mouse is on scrollbar, in the same units at m_sb_position (0 to 1)
                const int net_scroll_height = m_height - SCROLLBAR_THUMB_HEIGHT - SCROLLBAR_ARROW_HEIGHT*2 -
                                              SCROLLBAR_BOTTOM_MARGIN;
                const float goal = (mousey_current - m_from_y - SCROLLBAR_THUMB_HEIGHT/2 - SCROLLBAR_ARROW_HEIGHT)/
                                   (float)(net_scroll_height);

                // thumb has reached mouse, just continue as regular scrolling
                float dist = goal - m_sb_position; // find distance betweem mouse and thumb in 0-1 units
                if (dist < 0) dist = fabsf(dist);
                
                if (dist < 0.01)
                {
                    m_vertical_scrolling = true;
                }
                
                // move thumb towards mouse
                if (m_sb_position < goal) m_sb_position += 0.005;
                if (m_sb_position > goal) m_sb_position -= 0.005;

                // make sure thumb stays within bounds
                if (m_sb_position < 0) m_sb_position = 0;
                if (m_sb_position > 1) m_sb_position = 1;

                Display::render();
            }

        }//endif

        // -------------- check if user is clicking on scroll bar arrows ----------------
        if (m_scroll_up_arrow_pressed)
        {
            m_sb_position -= 0.001;
            if (m_sb_position < 0) m_sb_position = 0;

            Display::render();
        }

        if (m_scroll_down_arrow_pressed)
        {
            m_sb_position += 0.001;
            if (m_sb_position > 1) m_sb_position = 1;

            Display::render();
        }
    }

}

// ------------------------------------------------------------------------------------------------------------

void Editor::rightClick(RelativeXCoord x, int y)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    int noteID;
    const NoteSearchResult result = noteAt(x,y, noteID);

    if (result == FOUND_NOTE or result == FOUND_SELECTED_NOTE)
    {
        int screen_x, screen_y;
        Display::clientToScreen(x.getRelativeTo(WINDOW),y, &screen_x, &screen_y);
        showVolumeSlider(screen_x, screen_y, noteID, m_track);
    }
    else
    {
        wxCommandEvent evt(wxEVT_SHOW_TRACK_CONTEXTUAL_MENU, wxID_ANY);
        getMainFrame()->GetEventHandler()->AddPendingEvent(evt);
    }

}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

int Editor::getLevelAtY(const int y)
{
    return (y - getEditorYStart() + getYScrollInPixels()) / m_y_step;
}
    
// ------------------------------------------------------------------------------------------------------------

void Editor::makeMoveNoteEvent(const int relativeX, const int relativeY, const int noteID,
                               Action::Duplicate* duplicateParent)
{
    // move a single note
    if (noteID != -1)
    {
        Action::MoveNotes* event = new Action::MoveNotes(this, relativeX, relativeY, noteID);
        if (duplicateParent != NULL)
        {
            duplicateParent->moveEvent(event);
        }
        else
        {
            m_track->action( event );
        }
    }
    else
    {
        // move many notes
        Action::MoveNotes* event = new Action::MoveNotes(this, relativeX, relativeY, SELECTED_NOTES);

        if (duplicateParent != NULL)
        {
            duplicateParent->moveEvent(event);
        }
        else
        {
            m_track->action( event );
        }
    }
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setYStep(const int ystep)
{
    m_y_step = ystep;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::processKeyPress(int keycode, bool commandDown, bool shiftDown)
{
    // TODO: check this, there may be too many renders
    
    // --------------- move by 1 measure ------------
    if (shiftDown and not commandDown)
    {
        if (keycode == WXK_RIGHT or keycode == WXK_LEFT)
        {
            const int noteID = m_track->getFirstSelectedNote();
            if (noteID == -1)
            {
                wxBell();
            }
            else
            {
                const int tick = m_track->getNoteStartInMidiTicks(noteID);
                const int measure = m_sequence->getMeasureData()->measureAtTick(tick);
                
                const int factor = (keycode == WXK_LEFT ? -1 : 1);
                
                m_track->action(
                                new Action::MoveNotes(this, factor*m_sequence->getMeasureData()->measureLengthInTicks(measure),
                                                      0,
                                                      SELECTED_NOTES)
                                );
                Display::render();
            }
            return;
        }
    }
    
    if (not commandDown and not shiftDown)
    {
        // ---------------- move notes -----------------
        
        if (keycode == WXK_LEFT)
        {
            m_track->
            action( new Action::MoveNotes(this,
                                          -m_sequence->ticksPerBeat() * 4 /
                                          m_track->getMagneticGrid()->getDivider(), 0, SELECTED_NOTES)
                   );
            Display::render();
        }
        
        if (keycode == WXK_RIGHT)
        {
            m_track->
            action( new Action::MoveNotes(this,
                                          m_sequence->ticksPerBeat() * 4 /
                                          m_track->getMagneticGrid()->getDivider(), 0, SELECTED_NOTES)
                   );
            Display::render();
        }
        
        if (keycode == WXK_UP)
        {
            m_track->action( new Action::MoveNotes(this, 0, -1, SELECTED_NOTES) );
            Display::render();
        }
        
        if (keycode == WXK_DOWN)
        {
            m_track->action( new Action::MoveNotes(this, 0, 1, SELECTED_NOTES) );
            Display::render();
        }
        
        // ------------------------ delete notes ---------------------
        
        if (keycode == WXK_BACK or keycode == WXK_DELETE)
        {
            m_track->action( new Action::DeleteSelected(this) );
            Display::render();
        }
        
        
    }
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ size info -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Size Info
#endif

void Editor::updatePosition(int from_y, int to_y, int width, int height)
{
    // FIXME: there is no guarantee that (to_y - from_y) == height!! rethink this API
    ASSERT( MAGIC_NUMBER_OK() );
    m_from_y = from_y;
    m_to_y = to_y;
    m_width = width;
    m_height = height;
}

// ------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- Scrolling ---------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Scrolling
#endif


void Editor::scroll(float amount)
{
    ASSERT( MAGIC_NUMBER_OK() );
    m_sb_position -= amount;
    
    if      (m_sb_position < 0) m_sb_position = 0;
    else if (m_sb_position > 1) m_sb_position = 1;
}

// ------------------------------------------------------------------------------------------------------------
// ----------------------------------------- Edit tool management ---------------------------------------------
// ------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Edit tool management
#endif

EditTool Editor::getCurrentTool()
{
    return g_current_edit_tool;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setEditTool(EditTool tool)
{
    g_current_edit_tool = tool;
}

// ------------------------------------------------------------------------------------------------------------





