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


#include <cmath>
#include "Config.h"

#include "Actions/EditAction.h"
#include "Actions/AddNote.h"
#include "Actions/MoveNotes.h"
#include "Editors/DrumEditor.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/GraphicalTrack.h"
#include "GUI/ImageProvider.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Pickers/DrumChoice.h"
#include "Pickers/MagneticGrid.h"
#include "Renderers/RenderAPI.h"
#include "Renderers/Drawable.h"

#include "AriaCore.h"

namespace AriaMaestosa {

const int y_step = 10;

DrumInfo::DrumInfo(int midiKey, const bool a_section)
{
    DrumInfo::midiKey = midiKey;

    section=a_section;

    sectionExpanded=true;
}
// ***********************************************************************************************************************************************************
// **********************************************************    CONSTRUCTOR      ****************************************************************************
// ***********************************************************************************************************************************************************

static const wxString g_drum_names[] =
{
    /*
    wxT(" "), // 0
    wxT(" "), // 1
    wxT(" "), // 2
    wxT(" "), // 3
    wxT(" "), // 4
    wxT(" "), // 5
    wxT(" "), // 6
    wxT(" "), // 7
    wxT(" "), // 8
    wxT(" "), // 9
    wxT(" "), // 10
    wxT(" "), // 11
    wxT(" "), // 12
    wxT(" "), // 13
    wxT(" "), // 14
    wxT(" "), // 15
    wxT(" "), // 16
    wxT(" "), // 17
    wxT(" "), // 18
    wxT(" "), // 19
    wxT(" "), // 20
    wxT(" "), // 21
    wxT(" "), // 22
    wxT(" "), // 23
    wxT(" "), // 24
    wxT(" "), // 25
    wxT(" "), // 26
     */
    wxT("High Q"), // 27
    wxT("Slap"), // 28
    wxT("Push"), // 29
    wxT("Pull"), // 30
    wxT("Stick"), // 31
    wxT("Square"), // 32
    wxT("Metro"), // 33
    wxT("Metro bell"), // 34
    wxT("Bass drum 2"), // 35
    wxT("Bass drum 1"), // 36
    wxT("Stick"), // 37
    wxT("Snare"), // 38
    wxT("Clap"), // 39
    wxT("Snare 2"), // 40
    wxT("Tom 1"), // 41
    wxT("Closed hi-hat"), // 42
    wxT("Tom 2"), // 43
    wxT("Pedal hi-hat"), // 44
    wxT("Tom 3"), // 45
    wxT("Open hi-hat"), // 46
    wxT("Tom 4"), // 47
    wxT("Tom 5"), // 48
    wxT("Crash"), // 49
    wxT("Tom 6"), // 50
    wxT("Ride"), // 51
    wxT("Chinese"), // 52
    wxT("Ride bell"), // 53
    wxT("Tambourine"), // 54
    wxT("Splash"), // 55
    wxT("Cowbell"), // 56
    wxT("Crash 2"), // 57
    wxT("Vibraslap"), // 58
    wxT("Ride 2"), // 59
    wxT("Hi bongo"), // 60
    wxT("Low bongo"), // 61
    wxT("Mute hi conga"), // 62
    wxT("Open hi conga"), // 63
    wxT("Low conga"), // 64
    wxT("High timbale"), // 65
    wxT("Low timbale"), // 66
    wxT("High agogo"), // 67
    wxT("Low agogo"), // 68
    wxT("Cabasa"), // 69
    wxT("Maracas"), // 70
    wxT("Short whistle"), // 71
    wxT("Long whistle"), // 72
    wxT("Short guiro"), // 73
    wxT("Long guiro"), // 74
    wxT("Claves"), // 75
    wxT("Hi wood block"), // 76
    wxT("Lo wood block"), // 77
    wxT("Mute cuica"), // 78
    wxT("Open cuica"), // 79
    wxT("Mute triangle"), // 80
    wxT("Open triangle"), // 81
    wxT("Shaker"), // 82
    wxT("Jingle Bell"), // 83
    wxT("Bell Tree"), // 84
    wxT("Castanets"), // 85
    wxT("Mute surdo"), // 86
    wxT("Open surdo"), // 87

    // sections (using unused instrument slots for these, so they be
    // put in the same string array, improving performance)
    wxT("Drumkit"), // 88
    wxT("Hi-hat"), // 89
    wxT("Cymbal"), // 90
    wxT("Toms"), // 91
    wxT("African"), // 92
    wxT("Latin"), // 93
    wxT("Others"), // 94
    wxT("Sound Effects"), // 95
    /*
    wxT(" "), // 96
    wxT(" "), // 97
    wxT(" "), // 98
    wxT(" "), // 99
    wxT(" "), // 100
    wxT(" "), // 101
    wxT(" "), // 102
    wxT(" "), // 103
    wxT(" "), // 104
    wxT(" "), // 105
    wxT(" "), // 106
    wxT(" "), // 107
    wxT(" "), // 108
    wxT(" "), // 109
    wxT(" "), // 110
    wxT(" "), // 111
    wxT(" "), // 112
    wxT(" "), // 113
    wxT(" "), // 114
    wxT(" "), // 115
    wxT(" "), // 116
    wxT(" "), // 117
    wxT(" "), // 118
    wxT(" "), // 119*/
};

    DrumEditor::DrumEditor(Track* track) : Editor(track),
    drum_names_renderer( g_drum_names, 96-27)
{

    sb_position=0;
    mouse_is_in_editor=false;
    clickedOnNote=false;
    lastClickedNote = -1;
    showUsedDrumsOnly=false;

    useDefaultDrumSet();
    Editor::useInstantNotes();

#ifdef __WXMAC__
    drum_names_renderer.setFont( wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, /*wxFONTWEIGHT_BOLD*/ wxFONTWEIGHT_NORMAL) );
#else
    drum_names_renderer.setFont( wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, /*wxFONTWEIGHT_BOLD*/ wxFONTWEIGHT_NORMAL) );
#endif
}

DrumEditor::~DrumEditor()
{
}

void DrumEditor::useCustomDrumSet()
{

    bool inuse[128];
    for(int n=0; n<128; n++)
    {
        inuse[n]=false;
    }

    const int drum_amount = track->getNoteAmount();
    for(int drumID=0; drumID<drum_amount; drumID++)
    {
        inuse[ track->getNotePitchID(drumID) ] = true;
    }

    drums.clear();

    if (inuse[36] or inuse[35] or inuse[38] or inuse[40] or inuse[37])
    {
        drums.push_back( DrumInfo(88, true) ); // Drumkit
        if (inuse[36]) drums.push_back( DrumInfo(36)); // "Bass drum 1"
        if (inuse[35]) drums.push_back( DrumInfo(35)); // "Bass drum 2"
        if (inuse[38]) drums.push_back( DrumInfo(38)); // "Snare"
        if (inuse[40]) drums.push_back( DrumInfo(40)); // "Snare 2"
        if (inuse[37]) drums.push_back( DrumInfo(37)); // "Stick"
    }

    if (inuse[42] or inuse[46] or inuse[44])
    {
        drums.push_back( DrumInfo(89, true) ); // Hi-hat
        if (inuse[42]) drums.push_back( DrumInfo(42)); // "Closed hi-hat"
        if (inuse[46]) drums.push_back( DrumInfo(46)); // "Open hi-hat"
        if (inuse[44]) drums.push_back( DrumInfo(44)); // "Pedal hi-hat"
    }

    if (inuse[49] or inuse[57] or inuse[55] or inuse[52] or inuse[51] or inuse[59] or inuse[53])
    {
        drums.push_back( DrumInfo(89, true) ); // Cymbal
        if (inuse[49]) drums.push_back( DrumInfo(49)); // "Crash"
        if (inuse[57]) drums.push_back( DrumInfo(57)); // "Crash 2"
        if (inuse[55]) drums.push_back( DrumInfo(55)); // "Splash"
        if (inuse[52]) drums.push_back( DrumInfo(52)); // "Chinese"
        if (inuse[51]) drums.push_back( DrumInfo(51)); // "Ride"
        if (inuse[59]) drums.push_back( DrumInfo(59)); // "Ride 2"
        if (inuse[53]) drums.push_back( DrumInfo(53)); // "Ride bell"
    }

    if (inuse[41] or inuse[43] or inuse[45] or inuse[47] or inuse[48] or inuse[50])
    {
        drums.push_back( DrumInfo(90, true) ); // Toms
        if (inuse[41]) drums.push_back( DrumInfo(41)); // "Tom 1"
        if (inuse[43]) drums.push_back( DrumInfo(43)); // "Tom 2"
        if (inuse[45]) drums.push_back( DrumInfo(45)); // "Tom 3"
        if (inuse[47]) drums.push_back( DrumInfo(47)); // "Tom 4"
        if (inuse[48]) drums.push_back( DrumInfo(48)); // "Tom 5"
        if (inuse[50]) drums.push_back( DrumInfo(50)); // "Tom 6"
    }

    if (inuse[76] or inuse[77] or inuse[69] or inuse[67] or inuse[68] or inuse[58] or inuse[62] or inuse[63] or inuse[64])
    {
        drums.push_back( DrumInfo(91, true) ); // African
        if (inuse[76]) drums.push_back( DrumInfo(76)); // "Hi wood block"
        if (inuse[77]) drums.push_back( DrumInfo(77)); // "Lo wood block"
        if (inuse[69]) drums.push_back( DrumInfo(69)); // "Cabasa"
        if (inuse[67]) drums.push_back( DrumInfo(67)); // "High agogo"
        if (inuse[68]) drums.push_back( DrumInfo(68)); // "Low agogo"
        if (inuse[58]) drums.push_back( DrumInfo(58)); // "Vibraslap"
        if (inuse[62]) drums.push_back( DrumInfo(62)); // "Mute hi conga"
        if (inuse[63]) drums.push_back( DrumInfo(63)); // "Open hi conga"
        if (inuse[64]) drums.push_back( DrumInfo(64)); // "Low conga"
    }

    if (inuse[73] or inuse[74] or inuse[75] or inuse[78] or inuse[79] or inuse[70] or inuse[56] or inuse[60] or inuse[61] or inuse[85] or inuse[86] or inuse[87])
    {
        drums.push_back( DrumInfo(92, true) ); // Latin
        if (inuse[73]) drums.push_back( DrumInfo(73)); // "Short guiro"
        if (inuse[74]) drums.push_back( DrumInfo(74)); // "Long guiro"
        if (inuse[75]) drums.push_back( DrumInfo(75)); // "Claves"
        if (inuse[78]) drums.push_back( DrumInfo(78)); // "Mute cuica"
        if (inuse[79]) drums.push_back( DrumInfo(79)); // "Open cuica"
        if (inuse[70]) drums.push_back( DrumInfo(70)); // "Maracas"
        if (inuse[56]) drums.push_back( DrumInfo(56)); // "Cowbell"
        if (inuse[60]) drums.push_back( DrumInfo(60)); // "Hi bongo"
        if (inuse[61]) drums.push_back( DrumInfo(61)); // "Low bongo"
        if (inuse[85]) drums.push_back( DrumInfo(85)); // "Castanets"
        if (inuse[86]) drums.push_back( DrumInfo(86)); // "Mute surdo"
        if (inuse[87]) drums.push_back( DrumInfo(87)); // "Open surdo"
    }

    if (inuse[54] or inuse[65] or inuse[66] or inuse[71] or inuse[72] or inuse[80] or inuse[81] or inuse[82] or inuse[83] or inuse[84] or inuse[31])
    {
        drums.push_back( DrumInfo(93, true) ); // Others
        if (inuse[54]) drums.push_back( DrumInfo(54)); // "Tambourine"
        if (inuse[65]) drums.push_back( DrumInfo(65)); // "High timbale"
        if (inuse[66]) drums.push_back( DrumInfo(66)); // "Low timbale"
        if (inuse[71]) drums.push_back( DrumInfo(71)); // "Short whistle"
        if (inuse[72]) drums.push_back( DrumInfo(72)); // "Long whistle"
        if (inuse[80]) drums.push_back( DrumInfo(80)); // "Mute triangle"
        if (inuse[81]) drums.push_back( DrumInfo(81)); // "Open triangle"
        if (inuse[82]) drums.push_back( DrumInfo(82)); // "Shaker"
        if (inuse[83]) drums.push_back( DrumInfo(83)); // "Jingle Bell"
        if (inuse[84]) drums.push_back( DrumInfo(84)); // "Bell Tree"
        if (inuse[31]) drums.push_back( DrumInfo(31)); // "Stick"
    }

    if (inuse[34] or inuse[33] or inuse[32] or inuse[30] or inuse[29] or inuse[28] or inuse[27] or inuse[39])
    {
        drums.push_back( DrumInfo(94, true) ); // Sound effects
        if (inuse[34]) drums.push_back( DrumInfo(34)); // "Metro bell"
        if (inuse[33]) drums.push_back( DrumInfo(33)); // "Metro"
        if (inuse[32]) drums.push_back( DrumInfo(32)); // "Square"
        if (inuse[30]) drums.push_back( DrumInfo(30)); // "Pull"
        if (inuse[29]) drums.push_back( DrumInfo(29)); // "Push"
        if (inuse[28]) drums.push_back( DrumInfo(28)); // "Slap"
        if (inuse[27]) drums.push_back( DrumInfo(27)); // "High Q"
        if (inuse[39]) drums.push_back( DrumInfo(39)); // "Clap"
    }

    // prepare midiKeyToVectorID
    for(int n=0; n<128; n++)
        midiKeyToVectorID[n]=-1;

    for(unsigned int n=0; n<drums.size(); n++)
        midiKeyToVectorID[ drums[n].midiKey ]=n;
}

void DrumEditor::useDefaultDrumSet()
{
    drums.clear();

    drums.push_back( DrumInfo(88, true )); // Drumkit
    drums.push_back( DrumInfo(36)); // "Bass drum 1"
    drums.push_back( DrumInfo(35)); // "Bass drum 2"
    drums.push_back( DrumInfo(38)); // "Snare"
    drums.push_back( DrumInfo(40)); // "Snare 2"
    drums.push_back( DrumInfo(37)); // "Stick"

    drums.push_back( DrumInfo(89, true )); // Hi-hat
    drums.push_back( DrumInfo(42)); // "Closed hi-hat"
    drums.push_back( DrumInfo(46)); // "Open hi-hat"
    drums.push_back( DrumInfo(44)); // "Pedal hi-hat"

    drums.push_back( DrumInfo(90, true )); // Cymbal
    drums.push_back( DrumInfo(49)); // "Crash"
    drums.push_back( DrumInfo(57)); // "Crash 2"
    drums.push_back( DrumInfo(55)); // "Splash"
    drums.push_back( DrumInfo(52)); // "Chinese"
    drums.push_back( DrumInfo(51)); // "Ride"
    drums.push_back( DrumInfo(59)); // "Ride 2"
    drums.push_back( DrumInfo(53)); // "Ride bell"

    drums.push_back( DrumInfo(91, true )); // Toms
    drums.push_back( DrumInfo(41)); // "Tom 1"
    drums.push_back( DrumInfo(43)); // "Tom 2"
    drums.push_back( DrumInfo(45)); // "Tom 3"
    drums.push_back( DrumInfo(47)); // "Tom 4"
    drums.push_back( DrumInfo(48)); // "Tom 5"
    drums.push_back( DrumInfo(50)); // "Tom 6"

    drums.push_back( DrumInfo(92, true )); // African
    drums.push_back( DrumInfo(76)); // "Hi wood block"
    drums.push_back( DrumInfo(77)); // "Lo wood block"
    drums.push_back( DrumInfo(69)); // "Cabasa"
    drums.push_back( DrumInfo(67)); // "High agogo"
    drums.push_back( DrumInfo(68)); // "Low agogo"
    drums.push_back( DrumInfo(58)); // "Vibraslap"
    drums.push_back( DrumInfo(62)); // "Mute hi conga"
    drums.push_back( DrumInfo(63)); // "Open hi conga"
    drums.push_back( DrumInfo(64)); // "Low conga"

    drums.push_back( DrumInfo(93, true )); // Latin
    drums.push_back( DrumInfo(73)); // "Short guiro"
    drums.push_back( DrumInfo(74)); // "Long guiro"
    drums.push_back( DrumInfo(75)); // "Claves"
    drums.push_back( DrumInfo(78)); // "Mute cuica"
    drums.push_back( DrumInfo(79)); // "Open cuica"
    drums.push_back( DrumInfo(70)); // "Maracas"
    drums.push_back( DrumInfo(56)); // "Cowbell"
    drums.push_back( DrumInfo(60)); // "Hi bongo"
    drums.push_back( DrumInfo(61)); // "Low bongo"
    drums.push_back( DrumInfo(85)); // "Castanets"
    drums.push_back( DrumInfo(86)); // "Mute surdo"
    drums.push_back( DrumInfo(87)); // "Open surdo"

    drums.push_back( DrumInfo(94, true )); // Others
    drums.push_back( DrumInfo(54)); // "Tambourine"
    drums.push_back( DrumInfo(65)); // "High timbale"
    drums.push_back( DrumInfo(66)); // "Low timbale"
    drums.push_back( DrumInfo(71)); // "Short whistle"
    drums.push_back( DrumInfo(72)); // "Long whistle"
    drums.push_back( DrumInfo(80)); // "Mute triangle"
    drums.push_back( DrumInfo(81)); // "Open triangle"
    drums.push_back( DrumInfo(82)); // "Shaker"
    drums.push_back( DrumInfo(83)); // "Jingle Bell"
    drums.push_back( DrumInfo(84)); // "Bell Tree"
    drums.push_back( DrumInfo(31)); // "Stick"

    drums.push_back( DrumInfo(95, true)); // Sound effects
    drums.push_back( DrumInfo(34)); // "Metro bell"
    drums.push_back( DrumInfo(33)); // "Metro"
    drums.push_back( DrumInfo(32)); // "Square"
    drums.push_back( DrumInfo(30)); // "Pull"
    drums.push_back( DrumInfo(29)); // "Push"
    drums.push_back( DrumInfo(28)); // "Slap"
    drums.push_back( DrumInfo(27)); // "High Q"
    drums.push_back( DrumInfo(39)); // "Clap"


    // prepare midiKeyToVectorID
    for(int n=0; n<128; n++)
        midiKeyToVectorID[n]=-1;

    for(unsigned int n=0; n<drums.size(); n++)
        midiKeyToVectorID[ drums[n].midiKey ]=n;
}



// *******************************************************************************************************************************************************
// ***********************************************************    EDITOR      ****************************************************************************
// *******************************************************************************************************************************************************

NoteSearchResult DrumEditor::noteAt(RelativeXCoord x, const int y, int& noteID)
{
    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();

        assert(track->getNotePitchID(n)>0);
        assert(track->getNotePitchID(n)<128);

        const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
        if (drumIDInVector == -1) continue;

        const int drumy=getYForDrum(drumIDInVector);

        if (x.getRelativeTo(EDITOR)>drumx-1 and x.getRelativeTo(EDITOR)<drumx+5 and y>drumy and y<drumy+y_step)
        {

            noteID = n;

            if (track->isNoteSelected(n) and !Display:: isSelectLessPressed())
            {
                // clicked on a selected note
                return FOUND_SELECTED_NOTE;
            }
            else
            {
                return FOUND_NOTE;
            }

        }

    }//next note

    return FOUND_NOTHING;
}

void DrumEditor::noteClicked(const int id)
{
    track->selectNote(ALL_NOTES, false);
    track->selectNote(id, true);
    track->playNote(id);
}
void DrumEditor::addNote(const int snappedX, const int mouseY)
{
    const int drumID = getDrumAtY(mouseY); // FIXME - double checks? something very similar is also checked in MouseDown

    if (drumID < 0) return;
    if (drumID > (int)drums.size()-1) return;

    const int note = drums[ drumID ].midiKey;
    if (note == -1 || drums[ drumID ].section) return;

    track->action( new Action::AddNote(note, snappedX, snappedX+sequence->ticksPerBeat()/32+1, default_volume ) );
}

void DrumEditor::moveNote(Note& note, const int relativeX, const int relativeY)
{
    if (note.startTick+relativeX < 0) return; // refuse to move before song start

    note.startTick += relativeX;
    note.endTick   += relativeX;

    if (relativeY==0) return;

    /*
     *  Since the drums are not in midi order on screen, the procedure to move drums up or down is to:
     *      1. Find the screen location of this midi key
     *      2. Move the drum in screen location
     *      3. Find the new midi key at the new screen location
     */

    //assert(note.pitchID>=0);
    assert(note.pitchID<128);

    // find where on screen this particular drum is drawn (their screen position is not in the same order as the midi order)
    int newVectorLoc = midiKeyToVectorID[note.pitchID];
    if (newVectorLoc == -1) return;

    // move to the new location (in screen order)
    if (newVectorLoc + relativeY < 0 or
       newVectorLoc + relativeY > (int)drums.size()-1 ) return; // discard moves that would result in an out-of-bound note

    newVectorLoc += relativeY;

    // skip sections
    while(drums[newVectorLoc].section)
    {
        newVectorLoc+=(relativeY/abs(relativeY)); // keep the same sign, but only move 1 step at a time from now on

        // discard moves that would result in an out-of-bound note
        if (newVectorLoc < 0 or newVectorLoc > (int)drums.size()-1 )
            return;
    }

    // find the midi key at the new location
    const int new_pitchID = drums[newVectorLoc].midiKey;
    if (new_pitchID < 0 or new_pitchID > 127)
        return; // invalid location - discard

    note.pitchID = new_pitchID;
}

void DrumEditor::selectNotesInRect(RelativeXCoord& mousex_current, int mousey_current, RelativeXCoord& mousex_initial, int mousey_initial)
{
    for(int n=0; n<track->getNoteAmount(); n++)
    {
        const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels();

        assert(track->getNotePitchID(n)>0);
        assert(track->getNotePitchID(n)<128);

        const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
        if (drumIDInVector == -1) continue;

        const int drumy=getYForDrum(drumIDInVector) + 5;

        if (drumx>std::min(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
           drumx<std::max(mousex_current.getRelativeTo(EDITOR), mousex_initial.getRelativeTo(EDITOR)) and
           drumy > std::min(mousey_current, mousey_initial) and
           drumy < std::max(mousey_current, mousey_initial))
        {
            track->selectNote(n, true);
        }
        else
        {
            track->selectNote(n, false);
        }

    }//next note
}

int DrumEditor::getYScrollInPixels()
{
    // check if visible area is large enough to display everything
    if ((int)(drums.size()*y_step) < height)
    {
        useVerticalScrollbar(false);
        return 0;
    }
    else
    {
        useVerticalScrollbar(true);
    }

    return (int)(   sb_position*(drums.size()*y_step-height )   );
}

// *******************************************************************************************************************************************************
// ***********************************************************    EVENTS      ****************************************************************************
// *******************************************************************************************************************************************************
void DrumEditor::mouseDown(RelativeXCoord x, const int y)
{

    const int drumID = getDrumAtY(y);

    if (drumID >= 0 and drumID < (int)drums.size())
    {
        // click in the left area
        if (x.getRelativeTo(EDITOR) < 0 and x.getRelativeTo(WINDOW) > 0)
        {
            const int note = drums[ drumID ].midiKey;
            if (note == -1 || drums[ drumID ].section) return;

            PlatformMidiManager::playNote( note, default_volume, 500 /* duration */, 9, track->getDrumKit() );
        }    
        // click on section
        else if (drums[ drumID ].section)
        {
            // user clicked on a section. if click is on the triangle, expand/collapse it. otherwise, it just selects nothing.
            if (x.getRelativeTo(EDITOR) < 25 and x.getRelativeTo(EDITOR) > 0)
            {
                drums[ drumID ].sectionExpanded = !drums[ drumID ].sectionExpanded;
                return;
            }
            // select none
            else
            {
                track->selectNote(ALL_NOTES, false, true);
            }
        }// end if section
    }
    else
    {
        // select none
        track->selectNote(ALL_NOTES, false, true);
    }

    Editor::mouseDown(x, y);

}

void DrumEditor::mouseUp(RelativeXCoord mousex_current, const int mousey_current,
                         RelativeXCoord mousex_initial, const int mousey_initial)
{
    Editor::mouseUp(mousex_current, mousey_current, mousex_initial, mousey_initial);

    // ------------------- toggle "only show used drums" widget -----------------
    if (mousex_current.getRelativeTo(WINDOW) > getEditorsXStart()-77 and
       mousex_current.getRelativeTo(WINDOW) < getEditorsXStart()-67 and
       mousey_current - getYScrollInPixels() > getEditorYStart()+1 and
       mousey_current - getYScrollInPixels() < getEditorYStart()+10)
    {
        if (track->getNoteAmount()<1) return;

        showUsedDrumsOnly = !showUsedDrumsOnly;

        if (showUsedDrumsOnly) useCustomDrumSet();
        else useDefaultDrumSet();
        Display::render();
    }

}

// ***********************************************************************************************************************************************************
// ************************************************************    RENDER      *******************************************************************************
// ***********************************************************************************************************************************************************

void DrumEditor::render()
{
    render( RelativeXCoord_empty(), -1, RelativeXCoord_empty(), -1, true );
}

void DrumEditor::render(RelativeXCoord mousex_current, int mousey_current,
                        RelativeXCoord mousex_initial, int mousey_initial, bool focus)
{
    AriaRender::beginScissors(10, getEditorYStart(), width - 15, 20+height);

    drawVerticalMeasureLines(getEditorYStart(), getYEnd());

    // ----------------------- draw horizontal lines ---------------------
    AriaRender::primitives();
    AriaRender::color(0.5, 0.5, 0.5);
    const int drumAmount = drums.size();
    for(int drumID=0; drumID<drumAmount+1; drumID++)
    {
        const int y = getEditorYStart() + drumID*y_step - getYScrollInPixels();
        if (y<getEditorYStart() or y>getYEnd()) continue;

        AriaRender::line(getEditorsXStart(), y, getXEnd(), y);

    }


    // ---------------------- draw notes ----------------------------
    const int noteAmount = track->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();

        // don't draw notes that won't visible
        if (drumx<0) continue;
        if (drumx>width) break;

        assert(track->getNotePitchID(n)>=0);
        assert(track->getNotePitchID(n)<128);

        const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
        if (drumIDInVector == -1) continue;

        const float volume=track->getNoteVolume(n)/127.0;

        if (track->isNoteSelected(n) and focus) AriaRender::color((1-volume)*1, (1-(volume/2))*1, 0);
        else AriaRender::color((1-volume)*0.9, (1-volume)*0.9, (1-volume)*0.9);

        const int drumy=getYForDrum(drumIDInVector);

        AriaRender::triangle(drumx,     drumy,
                             drumx,     drumy+y_step,
                             drumx+5,   drumy+5);
    }


    // ------------------------- mouse drag (preview) ------------------------
    if (!clickedOnNote and mouse_is_in_editor)
    {
        // selection
        if (selecting)
        {
            AriaRender::color(0,0,0);
            AriaRender::hollow_rect(mousex_initial.getRelativeTo(WINDOW), mousey_initial,
                                    mousex_current.getRelativeTo(WINDOW), mousey_current);
        }
    } // end if !clickedOnNote

    // ------------------------- move note (preview) -----------------------
    if (clickedOnNote)
    {

        AriaRender::color(1, 0.85, 0, 0.5);

        const int x_difference = mousex_current.getRelativeTo(MIDI)-mousex_initial.getRelativeTo(MIDI);
        const int y_difference = mousey_current-mousey_initial;

        const int x_steps_to_move = (int)( snapMidiTickToGrid(x_difference)*sequence->getZoom() );
        const int y_steps_to_move = (int)round(y_difference/ (float)y_step );

        // move a single note
        if (lastClickedNote != -1)
        {
            const int drumx=track->getNoteStartInPixels(lastClickedNote) - sequence->getXScrollInPixels() + getEditorsXStart();

            const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(lastClickedNote) ];
            if (drumIDInVector != -1)
            {

                const int drumy=getYForDrum(drumIDInVector);

                AriaRender::triangle(drumx + x_steps_to_move,       drumy + y_steps_to_move*y_step,
                                     drumx + x_steps_to_move,       drumy + (y_steps_to_move+1)*y_step,
                                     drumx + 5 + x_steps_to_move,   drumy + y_step/2 + y_steps_to_move*y_step);
            }

        }
        else
        {
            // move a bunch of notes

            for(int n=0; n<noteAmount; n++)
            {
                if (!track->isNoteSelected(n)) continue;

                const int drumx=track->getNoteStartInPixels(n) - sequence->getXScrollInPixels() + getEditorsXStart();

                assert(track->getNotePitchID(n)>0);
                assert(track->getNotePitchID(n)<128);

                const int drumIDInVector= midiKeyToVectorID[ track->getNotePitchID(n) ];
                if (drumIDInVector == -1) continue;

                const int drumy=getYForDrum(drumIDInVector);

                AriaRender::triangle(drumx+x_steps_to_move,         drumy + y_steps_to_move*y_step,
                                     drumx+x_steps_to_move,         drumy + (y_steps_to_move+1)*y_step,
                                     drumx + 5 + x_steps_to_move,   drumy + y_step/2 + y_steps_to_move*y_step);
            } // next note


        }

    }


    // -----------------------------------------------------------------
    // left part with drum names
    // -----------------------------------------------------------------

    // grey background
    if (!focus) AriaRender::color(0.4, 0.4, 0.4);
    else AriaRender::color(0.8, 0.8, 0.8);

    AriaRender::rect(0, getEditorYStart(),
                     getEditorsXStart()-3, getYEnd());

    // drum names
    AriaRender::color(0,0,0);
    int drumY=-1;
    for(int drumID=0; drumID<drumAmount; drumID++)
    {
        drumY++;

        const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();
        if (y<getEditorYStart()-10 or y>getYEnd()) continue;

        // only show used drums widget
        if (drumY==0)
        {
            AriaRender::color(0,0,0);
            if (showUsedDrumsOnly)
            {
                AriaRender::triangle(getEditorsXStart()-73, y+2,
                                     getEditorsXStart()-73, y+8,
                                     getEditorsXStart()-63, y+5);
            }
            else
            {
                AriaRender::triangle(getEditorsXStart()-73, y+1,
                                     getEditorsXStart()-67, y+1,
                                     getEditorsXStart()-70, y+9);
           }

        }

        AriaRender::images();
        drum_names_renderer.bind();

        if (drums[drumID].section) // section header
        {
            AriaRender::primitives();
            AriaRender::color(0,0,0);
            AriaRender::rect(getEditorsXStart(), y,
                             getXEnd(), y+y_step);

            AriaRender::color(1,1,1);

            if (!drums[drumID].sectionExpanded) // expand/collapse widget of section header
            {
                AriaRender::triangle( getEditorsXStart()+7, y+2,
                                     getEditorsXStart()+7, y+8,
                                     getEditorsXStart()+17, y+5 );
            }
            else
            {
                AriaRender::triangle(getEditorsXStart()+7, y+1,
                                     getEditorsXStart()+13, y+1,
                                     getEditorsXStart()+10, y+9 );
            }


            AriaRender::images();
            AriaRender::color(1,1,1);
            // render twice otherwise it's too pale
            drum_names_renderer.get(drums[drumID].midiKey-27).render( getEditorsXStart()+20, y+12 );
            drum_names_renderer.get(drums[drumID].midiKey-27).render( getEditorsXStart()+20, y+12 );

        }//end if section
        else
        {
            AriaRender::color(0,0,0);
            drum_names_renderer.get(drums[drumID].midiKey-27).render( getEditorsXStart()-74, y+11 );
        }

        AriaRender::color(0,0,0);

        // if section is collapsed, skip all its elements
        assertExpr(drumID,<,(int)drums.size());
        if (!drums[drumID].sectionExpanded)
        {
            drumID++;
            while(!drums[drumID++].section){ assertExpr(drumID,<,(int)drums.size()); }
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum

    // -----------------------------------------------------------------
    // Scrollbar
    // -----------------------------------------------------------------

    if (!focus) AriaRender::setImageState(AriaRender::STATE_NO_FOCUS);
    else AriaRender::setImageState(AriaRender::STATE_NORMAL);

    renderScrollbar();

    AriaRender::color(1,1,1);
    AriaRender::endScissors();
}


int DrumEditor::getDrumAtY(const int given_y)
{

    int drumY=-1;
    for(unsigned int drumID=0; drumID<drums.size(); drumID++)
    {
        drumY++;

        const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();

        // check if given y is in the area drum drumID
        if ( given_y > y and given_y < y+y_step)  return drumID;

        // if section is collapsed, skip all its elements
        assertExpr(drumID,<,drums.size());
        if (!drums[drumID].sectionExpanded)
        {
            drumID++;
            while(!drums[drumID++].section){ assertExpr(drumID,<,drums.size()); }
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum

    return -1;

}

int DrumEditor::getYForDrum(const int given_drumID)
{

    int drumY=-1;
    for(unsigned int drumID=0; drumID<drums.size(); drumID++)
    {
        drumY++;

        const int y = getEditorYStart() + drumY*y_step - getYScrollInPixels();

        if ( (int)given_drumID == (int)drumID)
        {
            return y;
        }

        // if section is collapsed, skip all its elements
        assertExpr(drumID,<,drums.size());
        if (!drums[drumID].sectionExpanded)
        {
            drumID++;
            while(!drums[drumID++].section){ assertExpr(drumID,<,drums.size()); }
            drumID=drumID-2;
            continue;
        }//end if section collapsed
    }//next drum

    return -1;

}


}
