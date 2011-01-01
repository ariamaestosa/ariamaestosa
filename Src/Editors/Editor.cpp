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
#include "Actions/MoveNotes.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/ImageProvider.h"
#include "IO/IOUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Pickers/MagneticGrid.h"
#include "Pickers/VolumeSlider.h"
#include "Renderers/Drawable.h"
#include "Renderers/RenderAPI.h"
#include "UnitTest.h"
#include "Utils.h"

namespace AriaMaestosa
{
    EditTool g_current_edit_tool = EDIT_TOOL_PENCIL;
}

using namespace AriaMaestosa;

// ------------------------------------------------------------------------------------------------------------

Editor::Editor(Track* track)
{
    m_y_step = 10;

    m_track           = track;
    m_sequence        = track->getSequence();
    m_graphical_track = track->getGraphics();
    m_gsequence       = track->getGraphics()->getSequence();

    m_vertical_scrolling = false;
    m_click_on_scrollbar = false;

    m_scroll_up_arrow_pressed   = false;
    m_scroll_down_arrow_pressed = false;

    m_click_on_scrollbar = false;

    m_selecting = false;
    m_use_vertical_scrollbar = true;

    m_mouse_is_in_editor = false;
    m_clicked_on_note    = false;
    m_last_clicked_note  = -1;
    m_use_instant_notes = false;

    m_default_volume = 80;
}

// ------------------------------------------------------------------------------------------------------------

Editor::~Editor()
{
}

// ------------------------------------------------------------------------------------------------------------

int Editor::getDefaultVolume() const
{
    ASSERT( MAGIC_NUMBER_OK() );
    return m_default_volume;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setDefaultVolume(const int v)
{
    ASSERT( MAGIC_NUMBER_OK() );
    m_default_volume = v;
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

    for (int m=measureID; m<measureAmount; m+=1)
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

}

// ------------------------------------------------------------------------------------------------------------

void Editor::renderScrollbar()
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    if ( !m_use_vertical_scrollbar ) return;

    AriaRender::images();

    // ---- scrollbar background -----
    sbBackgDrawable->move(getWidth() - 24, getEditorYStart());
    sbBackgDrawable->scale(1, m_height/sbBackgDrawable->getImageHeight());
    sbBackgDrawable->render();

    // ------ top arrow ------
    sbArrowDrawable->setFlip(false, false);
    sbArrowDrawable->move(getWidth() - 24, getEditorYStart());

    const int mouseX = Display::getMouseX_initial().getRelativeTo(WINDOW);
    const int mouseX2 = Display::getMouseX_current().getRelativeTo(WINDOW);

    const int mouseY = Display:: getMouseY_initial();
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
        m_scroll_up_arrow_pressed=false;

    }

    sbArrowDrawable->render();

    // ------ bottom arrow ------
    sbArrowDrawable->setFlip(false, true);
    sbArrowDrawable->move(getWidth() - 24, m_from_y + m_header_bar_height + m_height + 12); //FIXME: fix this

    if (not m_vertical_scrolling and // ignore arrows if user is dragging thumb
       Display:: isMouseDown() and // only if mouse is down
       mouseX  > sbArrowDrawable->getX() and mouseX2 > sbArrowDrawable->getX() and // and mouse is lcoated on the arrow
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
    sbThumbDrawable->move(getWidth() - 24, (int)(getEditorYStart() + 17 + (m_height - 36)*m_sb_position));
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

            // grabbing the thumb (FIXME: correct these hardcoded sizes)
            const int thumb_pos = (int)(getEditorYStart() + 17 + (m_height - 36)*m_sb_position);
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
        if (m_use_vertical_scrollbar and x.getRelativeTo(WINDOW) < getWidth() - 24)
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
            noteClicked( m_last_clicked_note );
        }
        else if (result == FOUND_SELECTED_NOTE)
        {
            m_clicked_on_note = true;
            m_track->playNote( m_last_clicked_note, false );

            // 'noteAt' set 'm_last_clicked_note' to the ID of the note that was clicked. However at this point
            // this is not important anymore to know precisely which one was clicked, because all future
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
        if (m_vertical_scrolling and mousey_initial < getYEnd()-15 and mousey_initial>getEditorYStart())
        {
            // if we reach scrollbar extremity, wait until mouse comes back to the middle of the thumb before starting to scroll again
            if (m_sb_position == 0 and mousey_current < getEditorYStart()+25)
            {
                m_last_drag_y = mousey_current;
                return;
            }
            if (m_sb_position == 1 and mousey_current > getYEnd()-25)
            {
                m_last_drag_y = mousey_current;
                return;
            }

            m_sb_position += (mousey_current - m_last_drag_y)/(float)(m_height - 36);

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
        m_vertical_scrolling=false;
        m_click_on_scrollbar=false;
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

                int snapped_start = m_track->snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI) );
                int snapped_end   = m_track->snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) );

                // reject empty/malformed notes, add on success
                if (m_use_instant_notes) // for drums
                {
                    if (snapped_end < 0)
                    {
                        m_track->getGraphics()->selectNote(ALL_NOTES, false);
                        goto end_of_func;
                    }
                    addNote( snapped_end, mousey_initial );
                }
                else
                {
                    // click without moving
                    if (g_current_edit_tool == EDIT_TOOL_ADD and snapped_start == snapped_end) 
                    {
                        addNote(snapped_start,
                                snapped_start + m_sequence->ticksPerBeat()*4 / m_graphical_track->getGridDivider(),
                                mousey_initial );
                    }
                    else if (snapped_start == snapped_end or snapped_start>snapped_end or snapped_start<0)
                    {
                        m_track->getGraphics()->selectNote(ALL_NOTES, false);
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

            const int relativeX = m_track->snapMidiTickToGrid(x_difference);
            const int relativeY = (int)round( (float)y_difference / (float)m_y_step);

            if (relativeX == 0 and relativeY == 0)
            {
                goto end_of_func;
            }

            makeMoveNoteEvent(relativeX, relativeY, m_last_clicked_note);

        }// end if clicked on note

    }//end if m_mouse_is_in_editor


end_of_func:

    m_mouse_is_in_editor = false;
    m_clicked_on_note      = false;

}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseExited(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial)
{
    ASSERT( MAGIC_NUMBER_OK() );
    
    if (m_selecting) selectNotesInRect(mousex_current, mousey_current, mousex_initial, mousey_initial);
    m_selecting = false;
    m_mouse_is_in_editor = false;

    Display::render();
}

// ------------------------------------------------------------------------------------------------------------

/*
 * Is it necessary to send frequent mouse held down events?
 */
/*
 bool Editor::areMouseHeldDownEventsNeeded()
 {

     // if user is clicking on scrollbar but not on thumb, it is needed
     if (m_click_on_scrollbar and
        not m_scroll_up_arrow_pressed and
        not m_scroll_down_arrow_pressed and
        not m_vertical_scrolling
        )
     {
         return true;
     }

     // is user is clicking on a scroll arrow, it is necessary
     if (m_scroll_up_arrow_pressed or m_scroll_down_arrow_pressed) return true;

     // otherwise, it is not necessary
     return false;
 }
 */

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
            
            m_gsequence->setXScrollInPixels( new_scroll_value );
            DisplayFrame::updateHorizontalScrollbar();
            Display::render();
            return;
        }
    }



    // -------------- check if user is clicking on scroll bar, but not grabbing the thumb ----------------
    if (m_use_vertical_scrollbar)
    {
        m_click_on_scrollbar = false;

        // make thumb slide to mouse
        if (mousex_current.getRelativeTo(WINDOW) > sbThumbDrawable->getX() and
           mousex_current.getRelativeTo(WINDOW) < sbThumbDrawable->getX() + sbThumbDrawable->getImageWidth() and
           mousey_current < getYEnd()-15 and mousey_current>getEditorYStart())
        {

            m_click_on_scrollbar = true;
            if (wxGetKeyState(WXK_F1)) std::cout << extract_filename( fromCString(__FILE__) ).mb_str() << "@" << __LINE__ << " m_click_on_scrollbar = true" << std::endl;

            // not grabbing the thumb
            if (
               not m_scroll_up_arrow_pressed and
               not m_scroll_down_arrow_pressed and
               not m_vertical_scrolling
               )
            {

                // find where mouse is on scrollbar, in the same units at m_sb_position (0 to 1)
                //FIXME; remove those hardcoded numbers to calculate height
                const float goal = (mousey_current - m_from_y - m_header_bar_height - 52)/(float)(m_height - 36);

                // thumb has reached mouse, just continue as regular scrolling
                float dist = goal-m_sb_position; // find distance betweem mouse and thumb in 0-1 units
                if (dist<0) dist -= dist*2; // make positive
                if (dist<0.01)
                {
                    m_vertical_scrolling = true;
                }
                // move thumb towards mouse
                if (m_sb_position < goal) m_sb_position += 0.005;
                if (m_sb_position > goal) m_sb_position -= 0.005;

                // make sure thumb stays within bounds
                if (m_sb_position<0) m_sb_position=0;
                if (m_sb_position>1) m_sb_position=1;

                Display::render();
            }

        }//endif

        // -------------- check if user is clicking on scroll bar arrows ----------------
        if (m_scroll_up_arrow_pressed)
        {
            m_sb_position -= 0.001;
            if (m_sb_position<0) m_sb_position=0;

            //std::cout << "scrolling up to " << m_sb_position << std::endl;

            Display::render();
        }

        if (m_scroll_down_arrow_pressed)
        {
            m_sb_position += 0.001;
            if (m_sb_position>1) m_sb_position=1;

            //std::cout << "scrolling down to " << m_sb_position << std::endl;

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
        showVolumeSlider( screen_x, screen_y, noteID, m_track);
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

void Editor::makeMoveNoteEvent(const int relativeX, const int relativeY, const int noteID)
{
    // move a single note
    if (noteID != -1)
    {
        m_track->action( new Action::MoveNotes(relativeX, relativeY, noteID) );
    }
    else
    {
        // move many notes
        m_track->action( new Action::MoveNotes(relativeX, relativeY, SELECTED_NOTES) );
    }
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setYStep(const int ystep)
{
    m_y_step = ystep;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------ size info -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Size Info
#endif

void Editor::updatePosition(int from_y, int to_y, int width, int height, int barHeight)
{
    ASSERT( MAGIC_NUMBER_OK() );
    m_from_y = from_y;
    m_to_y = to_y;
    m_width = width;
    m_height = height;
    
    m_header_bar_height = barHeight;
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
// -------------------------------------------- Utils/actions  ------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Utils/actions
#endif

int Editor::findNotePitch(Note7 note_7, PitchSign sharpness, const int octave)
{
    int note = 0;
    
    if      (note_7 == NOTE_7_A) note += 2;
    else if (note_7 == NOTE_7_B) note += 0;
    else if (note_7 == NOTE_7_C) note += 11;
    else if (note_7 == NOTE_7_D) note += 9;
    else if (note_7 == NOTE_7_E) note += 7;
    else if (note_7 == NOTE_7_F) note += 6;
    else if (note_7 == NOTE_7_G) note += 4;
    else
    {
        std::cerr << "Invalid note: " << note_7 << std::endl;
        return 0;
    }
    
    if      (sharpness == SHARP) note -= 1;
    else if (sharpness == FLAT)  note += 1;
    
    //FIXME: add sanity checks for the value of 'octave'
    return note + (9 - octave)*12;
}

UNIT_TEST( TestFindNotePitch )
{
    require(Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 69,
            "findNotePitch works correctly" );
    require(Editor::findNotePitch(NOTE_7_B, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 71,
            "findNotePitch works correctly" );
    require(Editor::findNotePitch(NOTE_7_E, PITCH_SIGN_NONE, 4 /* octave */) == 131 - 64,
            "findNotePitch works correctly" );
    require(Editor::findNotePitch(NOTE_7_D, SHARP, 4 /* octave */) == 131 - 63,
            "findNotePitch works correctly" );
    require(Editor::findNotePitch(NOTE_7_E, FLAT, 4 /* octave */) == 131 - 63,
            "findNotePitch works correctly" );
    
    require(Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 3 /* octave */) == 131 - 57,
            "findNotePitch works correctly" );
    require(Editor::findNotePitch(NOTE_7_A, PITCH_SIGN_NONE, 6 /* octave */) == 131 - 93,
            "findNotePitch works correctly" );
}

// ------------------------------------------------------------------------------------------------------------

bool Editor::findNoteName(const int pitchID, Note12* note_12, int* octave)
{
    if (pitchID < 4)   return false;
    if (pitchID > 131) return false;
    *octave = 9 - pitchID/12;
    
    const int remainder = pitchID - int(pitchID/12) * 12;
    
    switch (remainder)
    {
        case 0:
            *note_12 = NOTE_12_B;
            ASSERT_E( findNotePitch(NOTE_7_B, NATURAL, *octave), ==, pitchID );
            return true;
        case 1:
            *note_12 = NOTE_12_A_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_A, SHARP, *octave), ==, pitchID );
            return true;
        case 2:
            *note_12 = NOTE_12_A;
            ASSERT_E( findNotePitch(NOTE_7_A, NATURAL, *octave), ==, pitchID );
            return true;
        case 3:
            *note_12 = NOTE_12_G_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_G, SHARP, *octave), ==, pitchID );
            return true;
        case 4:
            *note_12 = NOTE_12_G;
            ASSERT_E( findNotePitch(NOTE_7_G, NATURAL, *octave), ==, pitchID );
            return true;
        case 5:
            *note_12 = NOTE_12_F_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_F, SHARP, *octave), ==, pitchID );
            return true;
        case 6:
            *note_12 = NOTE_12_F;
            ASSERT_E( findNotePitch(NOTE_7_F, NATURAL, *octave), ==, pitchID );
            return true;
        case 7:
            *note_12 = NOTE_12_E;
            ASSERT_E( findNotePitch(NOTE_7_E, NATURAL, *octave), ==, pitchID );
            return true;
        case 8:
            *note_12 = NOTE_12_D_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_D, SHARP, *octave), ==, pitchID );
            return true;
        case 9:
            *note_12 = NOTE_12_D;
            ASSERT_E( findNotePitch(NOTE_7_D, NATURAL, *octave), ==, pitchID );
            return true;
        case 10:
            *note_12 = NOTE_12_C_SHARP;
            ASSERT_E( findNotePitch(NOTE_7_C, SHARP, *octave), ==, pitchID );
            return true;
        case 11:
            *note_12 = NOTE_12_C;
            ASSERT_E( findNotePitch(NOTE_7_C, NATURAL, *octave), ==, pitchID );
            return true;
        default:
            return false;
    }
}


UNIT_TEST( TestFindNoteName )
{
    Note12 note;
    int octave;
    
    {
        const bool success = Editor::findNoteName(131 - 69, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Editor::findNoteName(131 - 71, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_B, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Editor::findNoteName(131 - 64, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_E, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Editor::findNoteName(131 - 63, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_D_SHARP, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Editor::findNoteName(131 - 63, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_E_FLAT, "Conversion outputs the right note");
        require(octave == 4, "Conversion outputs the right octave");
    }

    {
        const bool success = Editor::findNoteName(131 - 57, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 3, "Conversion outputs the right octave");
    }
    
    {
        const bool success = Editor::findNoteName(131 - 93, &note, &octave);
        require(success, "Conversion didn't abort with error");
        require(note == NOTE_12_A, "Conversion outputs the right note");
        require(octave == 6, "Conversion outputs the right octave");
    }

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

