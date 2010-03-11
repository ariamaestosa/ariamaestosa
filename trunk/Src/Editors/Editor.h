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


#ifndef _editor_
#define _editor_

#include "Config.h"
#include "ptr_vector.h"

#include "Editors/RelativeXCoord.h"

namespace AriaMaestosa
{
    
    class Track; //forward
    class Sequence;
    class GraphicalTrack;
    class Note;
    class InstrumentChoice;
    
    enum NoteSearchResult
    {
        FOUND_NOTE,
        FOUND_SELECTED_NOTE,
        FOUND_NOTHING
    };
    
    enum EditorType
    {
        KEYBOARD   = 0,
        SCORE      = 1,
        GUITAR     = 2,
        DRUM       = 3,
        CONTROLLER = 4
    };
    
    /**
     * can have 2 uses : describing note and describing visual sign
     * e.g. F# will be described as SHARP as note, but if you put it in a score where all Fs are #,
     * its visible sign will be PITCH_SIGN_NONE. When describing a note's sign, use either NATURAL or
     * PITCH_SIGN_NONE. When describing a note's pitch, PITCH_SIGN_NONE is not to be used
     */
    enum PitchSign
    {
        SHARP = 0,
        FLAT = 1,
        NATURAL = 2,
        PITCH_SIGN_NONE = 3
    };
    
    enum NoteName
    {
        A = 0,
        B = 1,
        C = 2,
        D = 3,
        E = 4,
        F = 5,
        G = 6
    };
    
    enum EditTool
    {
        EDIT_TOOL_PENCIL,
        EDIT_TOOL_ADD
    };
    
    class Editor
    {
        bool verticalScrolling; // is user is dragging the scroll thumb
        int lastDragY; // the Y position of the mouse during last drag event (to see how much mouse has moved between 2 events)
        
    protected:
        bool useVerticalScrollbar_bool;
        bool useInstantNotes_bool; // true if notes have no duration (e.g. drums). false by default
        
        float sb_position; // scrollbar position, 0 meaning at top and 1 at bottom
        bool scroll_up_arrow_pressed, scroll_down_arrow_pressed; // is the user holding down the arrows at the top and bottom of the scrollbar
        bool click_on_scrollbar; // if user is clicking on the scrollbar
        
        int from_y;
        int to_y;
        int width;
        int height;
        int barHeight;
        bool selecting;
        int ystep;
        
        bool mouse_is_in_editor;
        int lastClickedNote; // contains the ID of the latest clicked note, or -1 to mean "selected notes"
        bool clickedOnNote;
        
        ptr_vector<Track, REF> backgroundTracks;
        
        unsigned short default_volume;
        
        /** Considering the vertical step, the current scrolling, etc. returns the vertical level coord from a y coord */
        int getLevelAtY(const int y);
        
    public:
        LEAK_CHECK();
        
        Editor(Track* track);
        virtual ~Editor();
        
        static EditTool getCurrentTool();
        static void setEditTool(EditTool tool);
        
        Track* track;
        Sequence* sequence;
        GraphicalTrack* graphicalTrack;
        
        // background tracks
        void clearBackgroundTracks();
        void addBackgroundTrack(Track* track);
        void trackDeleted(Track* track); // on track deletion, we need to check if this one is being used and remove references to it if so
        bool hasAsBackground(Track* track); // is the Track passed as argument a background of this?
        
        // Is it necessary to send frequent mouse held down events in current situation? this method tells you.
        //bool areMouseHeldDownEventsNeeded();
        
        /** method called by GraphicalTrack to let the Editor know about its position */
        void updatePosition(const int from_y, const int to_y, const int width,
                            const int height, const int barHeight);
        
        /** for default volume management.
          * @return the default volume for new notes in this editor
          */
        int  getDefaultVolume() const;
        
        /** for default volume management.
          * @param v  the new default volume for new notes in this editor
          */
        void setDefaultVolume(const int v);
        
        // ------------------------------------------------------------------------------------------------------
        // utility methods that children may call as needed
        // ------------------------------------------------------------------------------------------------------
        
        /** tells Editor whether to show a vertical scrollbar or not. on by default */
        void useVerticalScrollbar(const bool useScrollbar);
        
        void useInstantNotes(bool enabled=true);
        
        void scroll(float amount);
        
        void drawVerticalMeasureLines(int from_y, int to_y);
        
        /** Snaps a tick to the magnetic grid
          * @param absolute_x  A midi tick you want to snap
          * @return            The given tick, snapped to the magnetic grid
          */
        int snapMidiTickToGrid(int absolute_x);
        
        /** Same as 'snapMidiTickToGrid', but will only snap the tick to a bigger value */
        int snapMidiTickToGrid_ceil(int absolute_x);
        
        /** if you use a scrollbar, call this method somewhere near the end of your render method. */
        void renderScrollbar();
        
        /** in Aria, most editors (but ControlEditor) are organised as a vertical grid.
          * this method tells Editor what is the height of each "level" or "step".
          */
        void setYStep(const int height);
        
        // ------------------------------------------------------------------------------------------------------
        // methods that provide basic Editor functionnality like scrollbars, selection, horizontal mouse drag,
        // note move, etc. children may override them if they need different functionality
        // ------------------------------------------------------------------------------------------------------
        virtual void mouseDown(RelativeXCoord x, int y);
        virtual void mouseDrag(RelativeXCoord mousex_current, int mousey_current, RelativeXCoord mousex_initial, int mousey_initial);
        virtual void mouseUp(RelativeXCoord mousex_current, int mousey_current, RelativeXCoord mousex_initial, int mousey_initial);
        virtual void rightClick(RelativeXCoord x, int y);
        virtual void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                   RelativeXCoord mousex_initial, int mousey_initial);
        virtual void TrackPropertiesDialog(RelativeXCoord dragX_arg, int mousey_current,
                                           RelativeXCoord XBeforeDrag_arg, int mousey_initial);
        virtual void render();
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int, bool focus=false);
        
        // ------------------------------------------------------------------------------------------------------
        // methods children must implement (unless they do not use Editor and override every event callback)
        // ------------------------------------------------------------------------------------------------------
        /** if there is a note here, its ID is set in 'noteID'.
          * @return either FOUND_NOTE, FOUND_SELECTED_NOTE or FOUND_NOTHING
          */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID);
        
        /** to notify the editor that note 'id' was just clicked. */
        virtual void noteClicked(const int id);
        
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY);
        virtual void addNote(const int snappedX, const int mouseY); // for instant notes
        
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial);
        
        /**
          * Editor holds 'sb_position', a float value between 0 and 1 of where the scrollbar is.
          * It is up to each editor to determine how many pixels of scrolling results from the value
          * of 'sb_position'.
          */
        virtual int getYScrollInPixels();
        
        virtual void makeMoveNoteEvent(const int relativeX, const int relativeY, const int lastClickedNote);
        virtual void moveNote(Note& note, const int relativeX, const int relativeY);
        
        // ------------------------------------------------------------------------------------------------------
        // methods that provide general information common to all editors
        // ------------------------------------------------------------------------------------------------------
        const int getXEnd() const;
        const int getTrackYStart() const;
        static const int getEditorXStart();
        const int getEditorYStart() const;
        const int getYEnd() const;
        const int getWidth() const;
        
        //const NoteName pitchToNoteName
        
        /**
          * Returns the pitch ID of a note from its name, sharpness sign and octave
          */
        static int findNotePitch(NoteName note_7, PitchSign sharpness, const int octave);

    };
}

#endif
