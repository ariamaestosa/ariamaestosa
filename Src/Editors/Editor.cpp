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

#include "Config.h"
#include "AriaCore.h"

#include "Actions/MoveNotes.h"
#include "Editors/Editor.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Renderers/RenderAPI.h"
#include "Pickers/MagneticGrid.h"
#include "Pickers/VolumeSlider.h"
#include "GUI/ImageProvider.h"
#include "Renderers/Drawable.h"
#include "IO/IOUtils.h"

namespace AriaMaestosa
{
    EditTool g_current_edit_tool = EDIT_TOOL_PENCIL;
}

using namespace AriaMaestosa;

// ------------------------------------------------------------------------------------------------------------

Editor::Editor(Track* track)
{
    ystep = 10;

    Editor::track = track;
    Editor::sequence = track->sequence;
    Editor::graphicalTrack = track->graphics;

    verticalScrolling=false;
    click_on_scrollbar=false;

    scroll_up_arrow_pressed=false;
    scroll_down_arrow_pressed=false;

    click_on_scrollbar = false;

    selecting = false;
    useVerticalScrollbar_bool = true;

    mouse_is_in_editor=false;
    clickedOnNote=false;
    lastClickedNote=-1;
    useInstantNotes_bool = false;

    default_volume = 80;
}

// ------------------------------------------------------------------------------------------------------------

Editor::~Editor()
{
}

// ------------------------------------------------------------------------------------------------------------

int Editor::getDefaultVolume() const
{
    return default_volume;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setDefaultVolume(const int v)
{
    default_volume = v;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::useInstantNotes(bool enabled)
{
    useInstantNotes_bool = enabled;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::useVerticalScrollbar(const bool useScrollbar)
{
    Editor::useVerticalScrollbar_bool = useScrollbar;
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
    render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

// ------------------------------------------------------------------------------------------------------------

void Editor::render(RelativeXCoord mousex_current, int mousey_current,
                    RelativeXCoord mousex_initial, int mousey_initial, const bool focus)
{
}

// ------------------------------------------------------------------------------------------------------------

void Editor::drawVerticalMeasureLines(const int from_y, const int to_y)
{

    AriaRender::primitives();
    AriaRender::lineWidth(1);
    const int start_x = getMeasureData()->firstPixelInMeasure(
            getMeasureData()->measureAtPixel( Editor::getEditorXStart() ) );

    MeasureData* measureBar = getMeasureData();
    const int measureAmount = measureBar->getMeasureAmount();
    const float beatLength  = measureBar->beatLengthInPixels();
    float mx                = start_x;
    const int measureID     = measureBar->measureAtPixel( Editor::getEditorXStart() );
    float new_mx;

    for (int m=measureID; m<measureAmount; m+=1)
    {
        new_mx = measureBar->firstPixelInMeasure(m);

        // draw pale lines
        AriaRender::color(0.9, 0.9, 0.9);
        for(; mx < new_mx; mx += beatLength)
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
    if ( mx < end_of_screen)
    {
        AriaRender::color(0.9, 0.9, 0.9);
        for(;mx<end_of_screen; mx +=beatLength)
        {
            AriaRender::line((int)round(mx), from_y, (int)round(mx), to_y);
        }//next

    }//end if

}

// ------------------------------------------------------------------------------------------------------------

void Editor::renderScrollbar()
{
    if ( !useVerticalScrollbar_bool ) return;

    AriaRender::images();

    // ---- scrollbar background -----
    sbBackgDrawable->move(getWidth() - 24, from_y+20+barHeight);
    sbBackgDrawable->scale(1, height/sbBackgDrawable->getImageHeight());
    sbBackgDrawable->render();

    // ------ top arrow ------
    sbArrowDrawable->setFlip(false, false);
    sbArrowDrawable->move(getWidth() - 24, from_y+20+barHeight);

    const int mouseX = Display::getMouseX_initial().getRelativeTo(WINDOW);
    const int mouseX2 = Display::getMouseX_current().getRelativeTo(WINDOW);

    const int mouseY = Display:: getMouseY_initial();
    const int mouseY2 = Display:: getMouseY_current();

    if (!verticalScrolling and // ignore arrows if user is dragging thumb
       Display:: isMouseDown() and // only if mouse is down
       mouseX > sbArrowDrawable->x and mouseX2 > sbArrowDrawable->x and // and mouse is located on the arrow
       mouseY > sbArrowDrawable->y and mouseY2 > sbArrowDrawable->y and
       mouseX < sbArrowDrawable->x+sbArrowDrawable->getImageWidth() and mouseX2 < sbArrowDrawable->x+sbArrowDrawable->getImageWidth() and
       mouseY < sbArrowDrawable->y+sbArrowDrawable->getImageHeight() and mouseY2 < sbArrowDrawable->y+sbArrowDrawable->getImageHeight())
    {

        sbArrowDrawable->setImage( sbArrowDownImg );
        scroll_up_arrow_pressed=true;
        click_on_scrollbar = true;

    }
    else
    {

        sbArrowDrawable->setImage( sbArrowImg );
        scroll_up_arrow_pressed=false;

    }

    sbArrowDrawable->render();

    // ------ bottom arrow ------
    sbArrowDrawable->setFlip(false, true);
    sbArrowDrawable->move(getWidth() - 24, from_y+barHeight+height+12);

    if (!verticalScrolling and // ignore arrows if user is dragging thumb
       Display:: isMouseDown() and // only if mouse is down
       mouseX > sbArrowDrawable->x and mouseX2 > sbArrowDrawable->x and // and mouse is lcoated on the arrow
       mouseY > sbArrowDrawable->y and mouseY2 > sbArrowDrawable->y and
       mouseX < sbArrowDrawable->x+sbArrowDrawable->getImageWidth() and mouseX2 < sbArrowDrawable->x+sbArrowDrawable->getImageWidth() and
       mouseY < sbArrowDrawable->y+sbArrowDrawable->getImageHeight() and mouseY2 < sbArrowDrawable->y+sbArrowDrawable->getImageHeight())
    {

        sbArrowDrawable->setImage( sbArrowDownImg );
        scroll_down_arrow_pressed=true;
        click_on_scrollbar = true;
        //std::cout << "scroll_down_arrow_pressed = true" << std::endl;
    }
    else
    {

        sbArrowDrawable->setImage( sbArrowImg );
        scroll_down_arrow_pressed=false;
        //std::cout << "scroll_down_arrow_pressed = false" << std::endl;
    }

    sbArrowDrawable->render();

    // -------- thumb ------
    sbThumbDrawable->move(getWidth() - 24, (int)(from_y+barHeight+37+(height-36)*sb_position));
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
    backgroundTracks.clearWithoutDeleting();
}

// ------------------------------------------------------------------------------------------------------------

void Editor::addBackgroundTrack(Track* track)
{
    backgroundTracks.push_back(track);
}

// ------------------------------------------------------------------------------------------------------------

bool Editor::hasAsBackground(Track* track)
{
    const int bgTrackAmount = backgroundTracks.size();

    for(int m=0; m<bgTrackAmount; m++)
    {
        if (backgroundTracks.get(m) == track) return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------------------

/** on track deletion, we need to check if this one is being used and remove references to it if so */
void Editor::trackDeleted(Track* track)
{
    //Sequence* seq = getCurrentSequence();
    const int bgTrackAmount = backgroundTracks.size();

    for(int m=0; m<bgTrackAmount; m++)
    {
        if ( backgroundTracks.get(m) == track )
        {
            backgroundTracks.markToBeRemoved(m);
        }
    }//next

    backgroundTracks.removeMarked();
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
    selecting = false;

    lastDragY=y;

    if (useVerticalScrollbar_bool)
    {
        // check if user is grabbing the scroll bar
        if (x.getRelativeTo(WINDOW)>sbThumbDrawable->x and
           x.getRelativeTo(WINDOW)<sbThumbDrawable->x+sbThumbDrawable->getImageWidth() and
           y<getYEnd()-15 and y>getEditorYStart())
        {
            click_on_scrollbar = true;

            // grabbing the thumb
            const int thumb_pos = (int)(from_y+barHeight+37+(height-36)*sb_position);
            if (y>thumb_pos and y<thumb_pos + sbThumbDrawable->getImageHeight())
            {
                verticalScrolling = true;
            }

        }
        else
        {
            click_on_scrollbar = false;
        }

    }


    // check if click is whithin editable areas
    mouse_is_in_editor=false;

    if (useVerticalScrollbar_bool and y<getYEnd()-5 and y>getEditorYStart() and x.getRelativeTo(WINDOW) < getWidth() - 24)
    {
        mouse_is_in_editor=true;
    }
    if (!useVerticalScrollbar_bool and y<getYEnd()-5 and y>getEditorYStart() and x.getRelativeTo(WINDOW) < getXEnd())
    {
        mouse_is_in_editor=true;
    }

    clickedOnNote=false;

    // check if user clicked on a note
    if (mouse_is_in_editor)
    {
        const NoteSearchResult result = noteAt(x, y, lastClickedNote);
        if (result == FOUND_NOTE)
        {
            clickedOnNote = true;
            noteClicked( lastClickedNote );
        }
        else if (result == FOUND_SELECTED_NOTE)
        {
            clickedOnNote = true;
            track->playNote( lastClickedNote, false );

            // 'noteAt' set 'lastClickedNote' to the ID of the note that was clicked. However at this point
            // this is not important anymore to know precisely which one was clicked, because all future
            // operations that can be done will be applied to all selected notes. therefore we set
            // lastClickedNote to -1, meaning 'selected notes'.
            lastClickedNote = -1;
        }

    }

    selecting = false;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                       RelativeXCoord mousex_initial, int mousey_initial)
{

    if (useVerticalScrollbar_bool)
    {

        // -------------------  drag scrollbar  ---------------
        if (verticalScrolling and mousey_initial<getYEnd()-15 and mousey_initial>getEditorYStart())
        {
            // if we reach scrollbar extremity, wait until mouse comes back to the middle of the thumb before starting to scroll again
            if ( sb_position == 0 and mousey_current<getEditorYStart()+25)
            {
                lastDragY = mousey_current;
                return;
            }
            if ( sb_position == 1 and mousey_current>getYEnd()-25)
            {
                lastDragY = mousey_current;
                return;
            }

            sb_position += (mousey_current - lastDragY)/(float)(height-36);

            if (sb_position<0) sb_position=0;
            if (sb_position>1) sb_position=1;

        }
    }

    lastDragY = mousey_current;

    // if user did not click on note and moved the mouse vertically, then he is selecting
    if (!clickedOnNote and abs(mousey_current-mousey_initial)>ystep*2 and mouse_is_in_editor) selecting=true;

    // if selection becomes thin again, come back in note add mode
    if (selecting and abs(mousey_current-mousey_initial)<ystep) selecting = false;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseUp(RelativeXCoord mousex_current, int mousey_current,
                     RelativeXCoord mousex_initial, int mousey_initial)
{
    if (useVerticalScrollbar_bool)
    {
        verticalScrolling=false;
        click_on_scrollbar=false;
    }

    if (mouse_is_in_editor)
    {

        if (!clickedOnNote)
        {

            // --------------------------  select notes  ---------------------
            if (selecting)
            {
                selectNotesInRect(mousex_current, mousey_current, mousex_initial, mousey_initial);
            }
            else
            {
                // --------------------------  add note  --------------------------


                if (useVerticalScrollbar_bool)
                {
                    if (mousex_current.getRelativeTo(WINDOW) > sbThumbDrawable->x or mousex_initial.getRelativeTo(WINDOW) > sbThumbDrawable->x) goto end_of_func;
                }
                else
                {
                    if (mousex_current.getRelativeTo(WINDOW) > getXEnd() or mousex_initial.getRelativeTo(WINDOW) > getXEnd()) goto end_of_func;
                }

                int snapped_start=snapMidiTickToGrid(mousex_initial.getRelativeTo(MIDI) );
                int snapped_end=snapMidiTickToGrid(mousex_current.getRelativeTo(MIDI) );

                // reject empty/malformed notes, add on success
                if (useInstantNotes_bool) // for drums
                {
                    if (snapped_end<0)
                    {
                        track->selectNote(ALL_NOTES, false);
                        goto end_of_func;
                    }
                    addNote( snapped_end, mousey_initial );
                }
                else
                {
                    if (g_current_edit_tool == EDIT_TOOL_ADD and snapped_start == snapped_end) // click without moving
                    {
                        addNote( snapped_start, snapped_start + sequence->ticksPerBeat()*4 / graphicalTrack->grid->divider,
                                 mousey_initial );
                    }
                    else if (snapped_start == snapped_end or snapped_start>snapped_end or snapped_start<0)
                    {
                        track->selectNote(ALL_NOTES, false);
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

        if (clickedOnNote)
        {

            const int  x_difference = mousex_current.getRelativeTo(MIDI)-mousex_initial.getRelativeTo(MIDI);
            const int  y_difference = mousey_current-mousey_initial;

            const int relativeX = snapMidiTickToGrid(x_difference);
            const int relativeY = (int)round( (float)y_difference / (float)ystep);

            if (relativeX == 0 and relativeY == 0)
            {
                goto end_of_func;
            }

            makeMoveNoteEvent(relativeX, relativeY, lastClickedNote);

        }// end if clicked on note

    }//end if mouse_is_in_editor


end_of_func:

    mouse_is_in_editor=false;
    clickedOnNote=false;

}

// ------------------------------------------------------------------------------------------------------------

void Editor::TrackPropertiesDialog(RelativeXCoord mousex_current, int mousey_current,
                         RelativeXCoord mousex_initial, int mousey_initial)
{
    if (selecting) selectNotesInRect(mousex_current, mousey_current, mousex_initial, mousey_initial);
    selecting = false;
    mouse_is_in_editor = false;

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
     if (click_on_scrollbar and
        not scroll_up_arrow_pressed and
        not scroll_down_arrow_pressed and
        not verticalScrolling
        )
     {
         return true;
     }

     // is user is clicking on a scroll arrow, it is necessary
     if (scroll_up_arrow_pressed or scroll_down_arrow_pressed) return true;

     // otherwise, it is not necessary
     return false;
 }
 */

// ------------------------------------------------------------------------------------------------------------

void Editor::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                           RelativeXCoord mousex_initial, int mousey_initial)
{
    // ------------------------------- horizontal scroll by pushing mouse to side ------------------------------
    if (not click_on_scrollbar and selecting and mouse_is_in_editor)
    {
        mousex_initial.convertTo(MIDI);

        // scroll forward
        if (mousex_current.getRelativeTo(WINDOW) > getXEnd()-75)
        {
            getCurrentSequence()->setXScrollInPixels(
                                                     getCurrentSequence()->getXScrollInPixels()+
                                                     (mousex_current.getRelativeTo(WINDOW)-getXEnd()+75)/5 );
            DisplayFrame::updateHorizontalScrollbar();
            Display::render();
            return;
        }
        else if (mousex_current.getRelativeTo(WINDOW) < Editor::getEditorXStart()+20)
            // scroll backwards
        {
            const int new_scroll_value = getCurrentSequence()->getXScrollInPixels() -
                    (Editor::getEditorXStart() + 20 - mousex_current.getRelativeTo(WINDOW))/4;
            
            getCurrentSequence()->setXScrollInPixels( new_scroll_value );
            DisplayFrame::updateHorizontalScrollbar();
            Display::render();
            return;
        }
    }



    // -------------- check if user is clicking on scroll bar, but not grabbing the thumb ----------------
    if (useVerticalScrollbar_bool)
    {
        click_on_scrollbar = false;

        // make thumb slide to mouse
        if (mousex_current.getRelativeTo(WINDOW) > sbThumbDrawable->x and
           mousex_current.getRelativeTo(WINDOW) < sbThumbDrawable->x+sbThumbDrawable->getImageWidth() and
           mousey_current < getYEnd()-15 and mousey_current>getEditorYStart())
        {

            click_on_scrollbar = true;
            if (wxGetKeyState(WXK_F1)) std::cout << extract_filename( fromCString(__FILE__) ).mb_str() << "@" << __LINE__ << " click_on_scrollbar = true" << std::endl;

            // not grabbing the thumb
            if (
               not scroll_up_arrow_pressed and
               not scroll_down_arrow_pressed and
               not verticalScrolling
               )
            {

                // find where mouse is on scrollbar, in the same units at sb_position (0 to 1)
                const float goal = (mousey_current-from_y-barHeight-52)/(float)(height-36);

                // thumb has reached mouse, just continue as regular scrolling
                float dist = goal-sb_position; // find distance betweem mouse and thumb in 0-1 units
                if (dist<0) dist -= dist*2; // make positive
                if (dist<0.01){
                    verticalScrolling = true;
                }
                // move thumb towards mouse
                if (sb_position < goal) sb_position += 0.005;
                if (sb_position > goal) sb_position -= 0.005;

                // make sure thumb stays within bounds
                if (sb_position<0) sb_position=0;
                if (sb_position>1) sb_position=1;

                Display::render();
            }

        }//endif

        // -------------- check if user is clicking on scroll bar arrows ----------------
        if (scroll_up_arrow_pressed)
        {
            sb_position -= 0.001;
            if (sb_position<0) sb_position=0;

            //std::cout << "scrolling up to " << sb_position << std::endl;

            Display::render();
        }

        if (scroll_down_arrow_pressed)
        {
            sb_position += 0.001;
            if (sb_position>1) sb_position=1;

            //std::cout << "scrolling down to " << sb_position << std::endl;

            Display::render();
        }
    }

}

// ------------------------------------------------------------------------------------------------------------

void Editor::rightClick(RelativeXCoord x, int y)
{
    int noteID;
    const NoteSearchResult result = noteAt(x,y, noteID);

    if ( result == FOUND_NOTE or result == FOUND_SELECTED_NOTE )
    {
        int screen_x, screen_y;
        Display::clientToScreen(x.getRelativeTo(WINDOW),y, &screen_x, &screen_y);
        showVolumeSlider( screen_x, screen_y, noteID, track);
    }

}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------- note stuff to be implemented by children -----------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark To be implemented in children
#endif

//FIXME: should these be pure virtual?
NoteSearchResult Editor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    std::cerr << "ERROR base class method called" << std::endl;
    return FOUND_NOTHING;
}
void Editor::noteClicked(const int id)
{
    std::cerr << "ERROR base class method called" << std::endl;
}
void Editor::addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY)
{ 
    std::cerr << "ERROR base class method called" << std::endl;
}
void Editor::addNote(const int snappedX, const int mouseY)
{
    std::cerr << "ERROR base class method called" << std::endl;
}
void Editor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                               RelativeXCoord& mousex_initial, int mousey_initial)
{
    std::cerr << "ERROR base class method called" << std::endl;
}
void Editor::moveNote(Note& note, const int x_steps_to_move, const int y_steps_to_move)
{ 
    std::cerr << "ERROR base class method called" << std::endl;
}

// ------------------------------------------------------------------------------------------------------------
int Editor::getLevelAtY(const int y)
{
    return (y - getEditorYStart() + getYScrollInPixels())/ystep;
}
    
// ------------------------------------------------------------------------------------------------------------

void Editor::makeMoveNoteEvent(const int relativeX, const int relativeY, const int noteID)
{
        // move a single note
        if (noteID!=-1)
            track->action( new Action::MoveNotes(relativeX, relativeY, noteID) );
        else
            // move many notes
            track->action( new Action::MoveNotes(relativeX, relativeY, SELECTED_NOTES) );
}

// ------------------------------------------------------------------------------------------------------------

void Editor::setYStep(const int ystep)
{
    Editor::ystep = ystep;
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
    Editor::from_y = from_y;
    Editor::to_y = to_y;
    Editor::width = width;
    Editor::height = height;
    Editor::barHeight = barHeight;
}

// ------------------------------------------------------------------------------------------------------------

const int Editor::getEditorXStart()       {    return 90;                               }
const int Editor::getXEnd()         const {    return width - 5;                        } // FIXME - adapt to include vertical scrollbar
const int Editor::getTrackYStart()  const {    return from_y;                           }
const int Editor::getEditorYStart() const {    return from_y+barHeight+20;              }
const int Editor::getYEnd()         const {    return to_y - 10;                        }
const int Editor::getWidth()        const {    return width;                            }

// ------------------------------------------------------------------------------------------------------------
// ---------------------------------------------- Scrolling ---------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Scrolling
#endif

int Editor::getYScrollInPixels()
{
    // since each editor takes care of its own height this base class cannot return the correct answer
    // therefore this method needs to be overriden by any editor
    std::cout << "Edit base class called. This should not happen." << std::endl;
    assert(false);
    return -1;
}

// ------------------------------------------------------------------------------------------------------------

void Editor::scroll(float amount)
{
    sb_position -= amount;
    if (sb_position<0) sb_position=0;
    else if (sb_position>1) sb_position=1;
}

// ------------------------------------------------------------------------------------------------------------
// -------------------------------------------- Utils/actions  ------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Utils/actions
#endif

int Editor::snapMidiTickToGrid(int tick)
{
    int origin_tick = 0;
    if (not getMeasureData()->isMeasureLengthConstant())
    {
        const int measure = getMeasureData()->measureAtTick(tick);
        origin_tick = getMeasureData()->firstTickInMeasure(measure);
    }

    return origin_tick + (int)( round((float)(tick-origin_tick)/
                                      (float)(sequence->ticksPerBeat()*4 / graphicalTrack->grid->divider))
                                *(sequence->ticksPerBeat()*4 / graphicalTrack->grid->divider)
                                );

}

// ------------------------------------------------------------------------------------------------------------

int Editor::snapMidiTickToGrid_ceil(int tick)
{
    int origin_tick = 0;
    if (not getMeasureData()->isMeasureLengthConstant())
    {
        const int measure = getMeasureData()->measureAtTick(tick);
        origin_tick = getMeasureData()->firstTickInMeasure(measure);
    }

    return origin_tick + (int)( ceil((float)(tick-origin_tick)/
                                     (float)(sequence->ticksPerBeat()*4 / graphicalTrack->grid->divider))
                                *(sequence->ticksPerBeat()*4 / graphicalTrack->grid->divider)
                                );

}

// ------------------------------------------------------------------------------------------------------------

int Editor::findNotePitch(Note7 note_7, PitchSign sharpness, const int octave)
{
    int note = 0;
    
    if      (note_7 == A) note += 2;
    else if (note_7 == B) note += 0;
    else if (note_7 == C) note += 11;
    else if (note_7 == D) note += 9;
    else if (note_7 == E) note += 7;
    else if (note_7 == F) note += 6;
    else if (note_7 == G) note += 4;
    else
    {
        std::cerr << "Invalid note: " << note_7 << std::endl;
        return 0;
    }
    
    if      (sharpness == SHARP) note -= 1;
    else if (sharpness == FLAT)  note += 1;
    
    //FIXME: add sanity checks for 'octave'
    return note + octave*12;
}

// ------------------------------------------------------------------------------------------------------------

bool Editor::findNoteName(const int pitchID, Note12* note_12, int* octave)
{
    //TODO: this function would be excellent to test through unit tests
    assert(pitchID >= 4);
    assert(pitchID < 131);
    *octave = pitchID/12;
    
    const int remainder = pitchID - (*octave) * 12;
    
    switch (remainder)
    {
        case 0:
            *note_12 = NOTE_12_B;
            assert( findNotePitch(B, NATURAL, *octave) == pitchID );
            return true;
        case 1:
            *note_12 = NOTE_12_A_SHARP;
            assert( findNotePitch(A, SHARP, *octave) == pitchID );
            return true;
        case 2:
            *note_12 = NOTE_12_A;
            assert( findNotePitch(A, NATURAL, *octave) == pitchID );
            return true;
        case 3:
            *note_12 = NOTE_12_G_SHARP;
            assert( findNotePitch(G, SHARP, *octave) == pitchID );
            return true;
        case 4:
            *note_12 = NOTE_12_G;
            assert( findNotePitch(G, NATURAL, *octave) == pitchID );
            return true;
        case 5:
            *note_12 = NOTE_12_F_SHARP;
            assert( findNotePitch(F, SHARP, *octave) == pitchID );
            return true;
        case 6:
            *note_12 = NOTE_12_F;
            assert( findNotePitch(F, NATURAL, *octave) == pitchID );
            return true;
        case 7:
            *note_12 = NOTE_12_E;
            assert( findNotePitch(E, NATURAL, *octave) == pitchID );
            return true;
        case 8:
            *note_12 = NOTE_12_D_SHARP;
            assert( findNotePitch(D, SHARP, *octave) == pitchID );
            return true;
        case 9:
            *note_12 = NOTE_12_D;
            assert( findNotePitch(D, NATURAL, *octave) == pitchID );
            return true;
        case 10:
            *note_12 = NOTE_12_C_SHARP;
            assert( findNotePitch(C, SHARP, *octave) == pitchID );
            return true;
        case 11:
            *note_12 = NOTE_12_C;
            assert( findNotePitch(C, NATURAL, *octave) == pitchID );
            return true;
        default:
            return false;
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

