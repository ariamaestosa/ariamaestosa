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

#include "Utils.h"
#include "ptr_vector.h"

#include "Editors/Editor.h"
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
    
    /** enum to denotate a note's name (A, B, C, ...) regardless of any accidental it may have */
    enum Note7
    {
        NOTE_7_A = 0,
        NOTE_7_B = 1,
        NOTE_7_C = 2,
        NOTE_7_D = 3,
        NOTE_7_E = 4,
        NOTE_7_F = 5,
        NOTE_7_G = 6
    };
    
    /** enum to denotate a note's name (A, B, C, ...) INCLUDING of any accidental it may have.
      * What interests us here is the pitch, so we consider A# is the same as Bb. */
    enum Note12
    {
        NOTE_12_A       = 0,
        NOTE_12_A_SHARP = 1,
        NOTE_12_B       = 2,
        NOTE_12_C       = 3,
        NOTE_12_C_SHARP = 4,
        NOTE_12_D       = 5,
        NOTE_12_D_SHARP = 6,
        NOTE_12_E       = 7,
        NOTE_12_F       = 8,
        NOTE_12_F_SHARP = 9,
        NOTE_12_G       = 10,
        NOTE_12_G_SHARP = 11
    };
    const Note12 NOTE_12_A_FLAT = NOTE_12_G_SHARP;
    const Note12 NOTE_12_B_FLAT = NOTE_12_A_SHARP;
    const Note12 NOTE_12_D_FLAT = NOTE_12_C_SHARP;
    const Note12 NOTE_12_E_FLAT = NOTE_12_D_SHARP;
    const Note12 NOTE_12_G_FLAT = NOTE_12_F_SHARP;
    
    static const wxString NOTE_12_NAME[] =
    {
        wxT("A"),
        wxT("A#"),
        wxT("B"),
        wxT("C"),
        wxT("C#"),
        wxT("D"),
        wxT("D#"),
        wxT("E"),
        wxT("F"),
        wxT("F#"),
        wxT("G"),
        wxT("G#")
    };
    
    /** List of tools that can be used during edition */
    enum EditTool
    {
        EDIT_TOOL_PENCIL,
        EDIT_TOOL_ADD
    };
    
    /** The common base class for all editors. Manages several common tasks and provides
      * the interface for event callbacks
      */
    class Editor
    {
    protected:
        
        /** is user is dragging the scroll thumb? */
        bool verticalScrolling; 
        
        /** the Y position of the mouse during last drag event (to see how much mouse has
          * moved between 2 events)
          */
        int lastDragY;    
    
        /** Whether this particular editor wants a vertical scrollbar */
        bool useVerticalScrollbar_bool;
        
        /** true if notes have no duration (e.g. drums). false by default */
        bool useInstantNotes_bool; 
        
        /** scrollbar position, 0 meaning at top and 1 at bottom */
        float sb_position; 
        
        /** is the user holding down the arrows at the top and bottom of the scrollbar? */
        bool scroll_up_arrow_pressed, scroll_down_arrow_pressed; 
        
        /** true if user is clicking on the scrollbar */
        bool click_on_scrollbar;
        
        int from_y;
        int to_y;
        int width;
        int height;
        
        /** height of the header bar of a track (editor) */
        int m_header_bar_height;
        
        bool selecting;
        int ystep;
        
        bool mouse_is_in_editor;
        int lastClickedNote; // contains the ID of the latest clicked note, or -1 to mean "selected notes"
        bool clickedOnNote;
        
        ptr_vector<Track, REF> backgroundTracks;
        
        unsigned short default_volume;
        
        /** Considering the vertical step, the current scrolling, etc. returns the vertical level coord from a y coord */
        int getLevelAtY(const int y);
        
        void makeMoveNoteEvent(const int relativeX, const int relativeY, const int lastClickedNote);

    public:
        LEAK_CHECK();
        DECLARE_MAGIC_NUMBER();
        
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
        
        /** override to be notified of key change events */
        virtual void onKeyChange(const int symbol_amount, const PitchSign sharpness_symbol){}
        
        // ----------------------------------------------------------------------------------------------------
        // events
        // ----------------------------------------------------------------------------------------------------
        /** event callback for when mouse button is pressed in the editor */
        virtual void mouseDown(RelativeXCoord x, int y);
        
        /** event callback for when mouse button is dragged in the editor */
        virtual void mouseDrag(RelativeXCoord mousex_current, int mousey_current,
                               RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback for when mouse button is release in the editor */
        virtual void mouseUp(RelativeXCoord mousex_current, int mousey_current,
                             RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback for when mouse right button is pressed in the editor */
        virtual void rightClick(RelativeXCoord x, int y);
        
        /** event callback invoked repeatedly when mouse button is held down in the editor */
        virtual void mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                   RelativeXCoord mousex_initial, int mousey_initial);
        
        /** event callback for when mouse drag exited the bounds of the editor */
        virtual void mouseExited(RelativeXCoord dragX_arg, int mousey_current,
                                 RelativeXCoord XBeforeDrag_arg, int mousey_initial);
        
        /** called when it's time to render; invokes the other render method in derived editor class */
        void render();
        
        /** called in derived class when it's time to render */
        virtual void render(RelativeXCoord mousex_current, int mousey_current,
                            RelativeXCoord mousex_initial, int, bool focus=false) = 0;
        
        // ----------------------------------------------------------------------------------------------------
        // methods children must implement
        // ----------------------------------------------------------------------------------------------------
        /** if there is a note here, its ID is set in 'noteID'.
          * @return either FOUND_NOTE, FOUND_SELECTED_NOTE or FOUND_NOTHING
          */
        virtual NoteSearchResult noteAt(RelativeXCoord x, const int y, int& noteID) = 0;
        
        /** to notify the editor that note 'id' was just clicked. */
        virtual void noteClicked(const int id) = 0;
        
        /** called when user adds a note */
        virtual void addNote(const int snapped_start_tick, const int snapped_end_tick, const int mouseY) = 0;
        
        /** variant of 'addNote' for instant notes */
        virtual void addNote(const int snappedX, const int mouseY) = 0;
        
        /** called when user completes a selection rectangle */
        virtual void selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current,
                                       RelativeXCoord& mousex_initial, int mousey_initial) = 0;
        
        virtual void moveNote(Note& note, const int relativeX, const int relativeY) = 0;

        /**
          * Editor holds 'sb_position', a float value between 0 and 1 of where the scrollbar is.
          * It is up to each editor to determine how many pixels of scrolling results from the value
          * of 'sb_position'.
          */
        virtual int getYScrollInPixels() = 0;
                
        /** @return the (potentially translated) name of this editor */
        virtual wxString getName() const = 0;
        
        // ------------------------------------------------------------------------------------------------------
        // methods that provide general information common to all editors
        // ------------------------------------------------------------------------------------------------------

        static const int getEditorXStart(){ return 90;                            }
        const int getXEnd()         const { return width - 5;                     } // FIXME - adapt to include vertical scrollbar
        const int getTrackYStart()  const { return from_y;                        }
        const int getEditorYStart() const { return from_y+m_header_bar_height+20; }
        const int getYEnd()         const { return to_y - 10;                     }
        const int getWidth()        const { return width;                         }

        
        //const Note7 pitchToNoteName
        
        /**
          * Returns the pitch ID of a note from its name, sharpness sign and octave
          */
        static int findNotePitch(Note7 note_7, PitchSign sharpness, const int octave);

        /**
          * Finds a nome name (A, A#, B, etc..) and octave from its pitch ID
          * @param pitchID      pitch for which you want to find the note name/octave
          * @param[out] note_12 The name of the note at this pitch (A, A#, B, etc...)
          * @param[out] octave  The octave at which this note is
          * @return             Whether conversion was successful. If a problem occurred,
          *                     returns false and out parameters should not be read.
          */ 
        static bool findNoteName(const int pitchID, Note12* note_12, int* octave);

        /** Converts a Note7 to its Note12 counterpart */
        static Note12 note7ToNote12(Note7 note7)
        {
            static const Note12 note7_to_note12[] =
            {
                NOTE_12_A,
                NOTE_12_B,
                NOTE_12_C,
                NOTE_12_D,
                NOTE_12_E,
                NOTE_12_F,
                NOTE_12_G
            };
            return note7_to_note12[note7];
        }
    };
}

#endif
