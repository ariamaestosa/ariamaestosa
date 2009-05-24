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

/*
 * This is a midi Sequence, or a "file". Each tab in the tab bar represents one Sequence instance.
 * It contains general information and a vector of tracks.
 */

const float current_file_version = 1.0;

#include "Config.h"
#include "AriaCore.h"

#include "Actions/EditAction.h"
#include "Actions/SnapNotesToGrid.h"
#include "Actions/Paste.h"
#include "Actions/ScaleTrack.h"
#include "Actions/ScaleSong.h"

#include "Renderers/RenderAPI.h"
#include "GUI/MainFrame.h"

#include "Dialogs/Preferences.h"

#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/MeasureData.h"

#include "Editors/Editor.h"

#include "IO/IOUtils.h"

#include "irrXML/irrXML.h"

namespace AriaMaestosa {


Sequence::Sequence()
{
    reordering_newPosition = -1;

    dockHeight=0;

    beatResolution = 960;

    dockSize=0;
    currentTrack=0;
    tempo=120;
    x_scroll_in_pixels = 0;
    y_scroll = 0;
    reorderYScroll = 0;

    importing=false;
    maximize_track_mode = false;

    x_scroll_upon_copying = -1;

    follow_playback = Core::getPrefsValue("followPlayback") != 0;

    //setZoom(100);
    zoom = (128.0/(beatResolution*4));
    zoom_percent = 100;

    sequenceFileName.set(wxString(_("Untitled")));
    sequenceFileName.setMaxWidth(155); // FIXME - won't work if lots of sequences are open (tabs will begin to get smaller)
#ifdef __WXMAC__
    sequenceFileName.setFont( wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
    sequenceFileName.setFont( wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#endif

    if(Display::isVisible()) addTrack();

    copyright = wxT("");

    channelManagement = CHANNEL_AUTO;
    measureData = new MeasureData();
}

Sequence::~Sequence()
{
    std::cout << "cleaning up sequence " << suggestTitle().mb_str() << "..." << std::endl;
}


#if 0
#pragma mark -
#endif
wxString Sequence::suggestTitle()
{
    if(!getInternalName().IsEmpty())
    {
        return getInternalName();
    }
    else if(!sequenceFileName.IsEmpty())
    {
        return sequenceFileName;
    }
    else if(!filepath.IsEmpty())
    {
        return extract_filename(filepath).BeforeLast('.');
    }
    else
    {
        return  wxString( _("Untitled") );
    }
}
wxString Sequence::suggestFileName()
{
    if(!filepath.IsEmpty())
    {
        return extract_filename(filepath).BeforeLast('.');
    }
    else if(!getInternalName().IsEmpty())
    {
        return getInternalName();
    }
    else if(!sequenceFileName.IsEmpty())
    {
        return sequenceFileName;
    }
    else
    {
        return  wxString( _("Untitled") );
    }
}

// ------------------------------------------- text info ----------------------------------------
#if 0
#pragma mark -
#endif
void Sequence::setCopyright( wxString copyright )
{
    Sequence::copyright = copyright;
}

wxString Sequence::getCopyright()
{
    return copyright;
}

void Sequence::setInternalName(wxString name)
{
    internal_sequenceName = name;
}
wxString Sequence::getInternalName()
{
    return internal_sequenceName;
}

// ------------------------------------------- grid ----------------------------------------
#if 0
#pragma mark -
#endif
/*
 * When user selects 'snap notes to grid' from menu. Simply disptaches the event to the appropriate track.
 */

void Sequence::snapNotesToGrid()
{
    assert(currentTrack>=0);
    assert(currentTrack<tracks.size());

    tracks[ currentTrack ].action( new Action::SnapNotesToGrid() );

    Display::render();
}

// ------------------------------------------- scrolling ----------------------------------------
#if 0
#pragma mark -
#endif

int Sequence::getYScroll()
{
    return y_scroll;
}

int Sequence::getXScrollInMidiTicks()
{
    return (int)( x_scroll_in_pixels/zoom );
}

int Sequence::getXScrollInPixels()
{
    return x_scroll_in_pixels;
}

void Sequence::setXScrollInMidiTicks(int value)
{
    x_scroll_in_pixels = (int)( value * zoom );
}
void Sequence::setXScrollInPixels(int value)
{
    x_scroll_in_pixels = value;

    const int editor_size=Display::getWidth()-100,
        total_size = getMeasureData()->getTotalPixelAmount();

    if( x_scroll_in_pixels < 0 ) x_scroll_in_pixels = 0;
    if( x_scroll_in_pixels >= total_size-editor_size) x_scroll_in_pixels = total_size-editor_size-1;


    Display::render();
}

void Sequence::setYScroll(int value)
{
    y_scroll = value;
}

// ------------------------------------------- setup ----------------------------------------
#if 0
#pragma mark -
#endif
float Sequence::getZoom()
{
    return zoom;
}

int Sequence::getZoomInPercent()
{
    return zoom_percent;
}

void Sequence::setZoom(int zoom)
{

    Sequence::zoom = (zoom/100.0) * 128.0 / ((float)getMeasureData()->beatLengthInTicks() * 4);
    Sequence::zoom_percent = zoom;

}

void Sequence::setChannelManagementType(ChannelManagementType type)
{
    channelManagement = type;
}
ChannelManagementType Sequence::getChannelManagementType()
{
    return channelManagement;
}

/*
 * Get various information about resolution.
 * Ticks per beat is the number of time units in a quarter note.
 * Ticks per measure returns how many ticks there would be in a whole measure considering the beat resolution and the measure time signature.
 */

int Sequence::ticksPerBeat(){        return beatResolution;        }
/*
 int Sequence::ticksPerMeasure()
 {
     return (int)(
                  beatResolution * measureBar->getTimeSigNumerator() * (4.0/measureBar->getTimeSigDenominator())
                  );

 }
 */
void Sequence::setTicksPerBeat(int res){        beatResolution = res;        }


// ------------------------------------------- actions and undo ----------------------------------------
#if 0
#pragma mark -
#endif
/*
 * This is the method called for performing any action that can be undone.
 * A EditAction object is used to describe the task, and it also knows how to revert it.
 * The EditAction objects are kept in a stack in Sequence in order to offer multiple undo levels.
 *
 * Sequence::action does actions that affect all tracks. Also see Track::action.
 */

void Sequence::action( Action::MultiTrackAction* action)
{
    addToUndoStack( action );
    action->setParentSequence(this);
    action->perform();
}

// you do not need to call this yourself, Track::action and Sequence::action do.
void Sequence::addToUndoStack( Action::EditAction* action )
{
    undoStack.push_back(action);

    // remove old actions from undo stack, to not take memory uselessly
    if(undoStack.size()>8) undoStack.erase(0);
}
void Sequence::undo()
{
    if(undoStack.size() < 1)
    {
        // nothing to undo
        wxBell();
        return;
    }

    Action::EditAction* lastAction = undoStack.get( undoStack.size() - 1 );
    lastAction->undo();
    undoStack.erase( undoStack.size() - 1 );

    Display::render();
}

// forbid undo, drop all undo information kept in memory.
void Sequence::clearUndoStack()
{
    undoStack.clearAndDeleteAll();
}

// is there something to undo?
bool Sequence::somethingToUndo()
{
    return undoStack.size() > 0;
}

/*
 * Called just before doing any operation that modifies notes.
 * Tells the appropriate track to do a copy of current state, to be able to restore it if user wanted to.
 */
/*
void Sequence::loadUndoMemory()
{
    if(!somethingDoneToUndo) return;
    tracks[lastModifiedTrack].loadUndoMemory();
    Display::render();
}
*/

// ------------------------------------------- render ----------------------------------------
#if 0
#pragma mark -
#endif
// FIXME - -this class shouldn't do  both graphics and data
void Sequence::renderTracks(int currentTick, RelativeXCoord mousex, int mousey, int mousey_initial, int from_y)
{

    const int draggedTrack = Display::getDraggedTrackID();

    // draw tracks normally
    if(draggedTrack==-1)
    {
        reorderYScroll=0;

        int y=from_y - y_scroll;
        const int trackAmount = tracks.size();
        for(int n=0; n<trackAmount; n++)
        {
            tracks[n].setId(n);
            y = tracks[n].graphics->render(y, currentTick, n==currentTrack);
        }

    }
    // reordering
    else
    {
        reordering_newPosition = draggedTrack;

        // draw tracks before current
        int first_y = mousey_initial - reorderYScroll;

        for(int tracknum=draggedTrack-1; tracknum>=0; tracknum--)
        {
            if(tracks[tracknum].graphics->docked) continue;

            first_y =  mousey_initial - (draggedTrack-tracknum)*50 - reorderYScroll;
            tracks[tracknum].graphics->renderHeader(0, first_y, true);
            if(mousey < mousey_initial - (draggedTrack-tracknum)*50+25 - reorderYScroll) reordering_newPosition=tracknum;
        }

        // draw tracks after current
        int last_y = mousey_initial - reorderYScroll;

        for(int tracknum=draggedTrack+1; tracknum<tracks.size(); tracknum++)
        {
            if(tracks[tracknum].graphics->docked) continue;

            last_y = mousey_initial + (tracknum-draggedTrack)*50 - reorderYScroll;
            tracks[tracknum].graphics->renderHeader(0, last_y, true);
            if(mousey > mousey_initial + (tracknum-draggedTrack)*50+25 - reorderYScroll) reordering_newPosition=tracknum+1;
        }

        // scroll up or down if mouse goes to the edges
        if(mousey < 100)
        {
            if(mousey > first_y)
                reorderYScroll -= (100-mousey)*4/100;
        }

        if(mousey > Display::getHeight()-100)
        {
            if(mousey < last_y+50)
                reorderYScroll += (mousey - Display::getHeight()+100)*4/100;
        }

        // draw track the user is dragging
        tracks[draggedTrack].graphics->renderHeader(0, mousey, true, true);

        const int arrow_y = (reordering_newPosition > draggedTrack) ?
            mousey_initial + (reordering_newPosition - draggedTrack)*50 - 5 - reorderYScroll :
            mousey_initial - (draggedTrack - reordering_newPosition)*50 - 5 - reorderYScroll;

        AriaRender::primitives();
        AriaRender::color(1,0,0);
        AriaRender::line(26, arrow_y, 10, arrow_y);
        AriaRender::triangle( 35, arrow_y,
                              25, arrow_y - 5,
                              25, arrow_y + 5);
    }//end if

}



// ------------------------------------------- mouse events ----------------------------------------

/*
 * do we need to start a timer that will frequently send mouse held down events?
 */

bool Sequence::areMouseHeldDownEventsNeeded()
{
    return true;

    // FIXME - clarify status of this. fix or remove.

    /*
    const int draggedTrack = Display::getDraggedTrackID();

    // we're reordering tracks, it is necessary. return true.
    if(draggedTrack!=-1) return true;

    // ask all editors if they need such events at this point.
    const int trackAmount = tracks.size();
    for(int n=0; n<trackAmount; n++)
    {
        // there's one editor that wants them, so return true.
        if(tracks[n].graphics->getCurrentEditor()->areMouseHeldDownEventsNeeded()) return true;
    }//next

    // we're not reordering and no editor requested such events, so return false
    return false;
     */
}

/*
 *  Called repeatedly when mouse is held down
 */

void Sequence::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current, RelativeXCoord mousex_initial, int mousey_initial)
{
    // FIXME - dragging tracks has nothing to do in the display
    const int draggedTrack = Display::getDraggedTrackID();

    // if reordering tracks
    if(draggedTrack!=-1)
    {
        Display::render(); // reordering preview is done while rendering, so calling 'render' will update reordering onscreen.
        return;
    }

    // dispatch event to all tracks
    const int trackAmount = tracks.size();
    for(int n=0; n<trackAmount; n++)
    {
        tracks[n].graphics->getCurrentEditor()->mouseHeldDown(mousex_current, mousey_current, mousex_initial, mousey_initial);
    }//next

}



// ------------------------------------------- info ----------------------------------------

/*
 * Returns the number of pixels it takes to draw all tracks, vertically.
 * This is used mostly by the code managing the vertical scrollbar.
 */

int Sequence::getTotalHeight()
{

    int totalHeight=0;

    for(int n=0; n<tracks.size(); n++)
    {
        totalHeight += tracks[n].graphics->getTotalHeight() + 10;
    }

    if(getMeasureData()->isExpandedMode()) totalHeight += 20;

    return totalHeight;
}

// ------------------------------------------- tracks ----------------------------------------
#if 0
#pragma mark -
#endif
void Sequence::addTrack()
{
    if(currentTrack>=0 and currentTrack<tracks.size())
        tracks.add(new Track(getMainFrame(), this), currentTrack+1); // add new track below active one
    else
        tracks.push_back(new Track(getMainFrame(), this));

    Display::render();
}

void Sequence::deleteTrack()
{
    if(currentTrack<0 or currentTrack>tracks.size()-1) return;

    tracks.erase( currentTrack );

    while(currentTrack>tracks.size()-1) currentTrack -= 1;

    Display::render();
}

void Sequence::deleteTrack(int ID)
{
    assertExpr(ID,>=,0);
    assertExpr(ID,<,tracks.size());

    tracks.erase( ID );

    while(currentTrack>tracks.size()-1) currentTrack -= 1;
}

/*
 * Called when a user has finished dragging the track to reorder it.
 * Where the track ends was calculated while drawing the preview - all this methods needs to do is
 * remove the track from its curren location and move it to its new location.
 */

void Sequence::reorderTracks()
{

    const int draggedTrack = Display::getDraggedTrackID();

    if( reordering_newPosition == draggedTrack ) return;
    if( reordering_newPosition == -1 ) return;

    Track* dragged_track = &tracks[draggedTrack];

    // if we remove an element now, the IDs in the vector will be modified and reordering_newPosition may not be valid anymore.
    // so we just 'mark' the element we don't want anymore, and it will be removed just after adding the new item.
    tracks.markToBeRemoved(draggedTrack);
    tracks.add(dragged_track, reordering_newPosition);
    tracks.removeMarked();

    currentTrack = reordering_newPosition-1;

    if(! (currentTrack>=0) ) currentTrack=0;
    if(! (currentTrack<tracks.size()) ) currentTrack=0;
}


Track* Sequence::getCurrentTrack()
{
    return &tracks[currentTrack];
}

int Sequence::getTrackAmount()
{
    return tracks.size();
}

int Sequence::getCurrentTrackID()
{
    return currentTrack;
}

void Sequence::setCurrentTrackID(int ID)
{
    currentTrack = ID;
}

Track* Sequence::getTrack(int ID)
{
    assertExpr(ID,>=,0);
    assertExpr(ID,<,tracks.size());

    return &tracks[ID];
}

void Sequence::setCurrentTrack(Track* track)
{
    const int trackAmount = tracks.size();
    for(int n=0; n<trackAmount; n++)
    {
        if( &tracks[n] == track )
        {
            currentTrack = n;
            return;
        }
    }

    std::cout << "Error: void Sequence::setCurrentTrack(Track* track) couldn't find any matching track" << std::endl;

}

/*
 * Hide a track by sending it to the 'dock'
 */

void Sequence::addToDock(GraphicalTrack* track)
{

    dock.push_back(track);
    dockSize = dock.size();

    currentTrack = 0;

    DisplayFrame::updateVerticalScrollbar();
}

void Sequence::removeFromDock(GraphicalTrack* track)
{

    for(int n=0; n<dock.size(); n++)
    {
        if(&dock[n] == track)
        {
            dock.remove( n );
            dockSize = dock.size();
            return;
        }
    }
    DisplayFrame::updateVerticalScrollbar();

}

// ------------------------------------------- playback ----------------------------------------
#if 0
#pragma mark -
#endif
void Sequence::spacePressed()
{

    if(!PlatformMidiManager::isPlaying())
    {

        if( getMainFrame()->playback_mode )
        {
            return;
        }

        getMainFrame()->toolsEnterPlaybackMode();

        int startTick = -1;
        bool success = PlatformMidiManager::playSelected(this, &startTick);
        Display::setPlaybackStartTick( startTick ); // FIXME - start tick should NOT go in GlPane

        if(!success or startTick == -1 ) // failure
        {
            Display::exitPlayLoop();
        }
        else
        {
            Display::enterPlayLoop();
        }

    }
    else
    {

        getMainFrame()->toolsExitPlaybackMode();
        PlatformMidiManager::stop();
        Display::render();

    }

}

// ------------------------------------------- copy/paste ----------------------------------------

void Sequence::copy()
{
    tracks[currentTrack].copy();
    x_scroll_upon_copying = x_scroll_in_pixels;
}

void Sequence::paste()
{
    tracks[currentTrack].action( new Action::Paste(false) );
    Display::render();
}

void Sequence::pasteAtMouse()
{
    tracks[currentTrack].action( new Action::Paste(true) );
    Display::render();
}

// ------------------------------------------- selection ----------------------------------------

void Sequence::selectAll()
{
    tracks[currentTrack].selectNote(ALL_NOTES, true, true);
    Display::render();
}

void Sequence::selectNone()
{
    tracks[currentTrack].selectNote(ALL_NOTES, false, true);
    Display::render();
}

// ------------------------------------------- scale ----------------------------------------

void Sequence::scale(
                     float factor,
                     bool rel_first_note, bool rel_begin, // relative to what (only one must be true)
                     bool affect_selection, bool affect_track, bool affect_song // scale what (only one must be true)
                     )
{


    int relative_to = -1;

    // selection
    if(affect_selection)
    {

        if(rel_first_note) relative_to = tracks[currentTrack].getFirstNoteTick(true);
        else if(rel_begin) relative_to=0;

        tracks[currentTrack].action( new Action::ScaleTrack(factor, relative_to, true) );
    }

    // track
    else if(affect_track)
    {

        if(rel_first_note) relative_to = tracks[currentTrack].getFirstNoteTick();
        else if(rel_begin) relative_to=0;

        tracks[currentTrack].action( new Action::ScaleTrack(factor, relative_to, false) );
    }

    // song
    else if(affect_song)
    {

        if(rel_first_note)
        {

            // find first tick in all tracks [i.e. find the first tick of all tracks and keep the samllest]
            int song_first_tick = -1;
            for(int n=0; n<tracks.size(); n++)
            {

                const int track_first_tick = relative_to = tracks[n].getFirstNoteTick();
                if(track_first_tick<song_first_tick or song_first_tick==-1) song_first_tick = track_first_tick;
            }//next

            relative_to = song_first_tick;
        }
        else if(rel_begin) relative_to=0;

        // scale all tracks
        action( new Action::ScaleSong(factor, relative_to) );

    }

    Display::render();

}

// ------------------------------------------- tempo ----------------------------------------

void Sequence::setTempo(int tmp)
{
    tempo = tmp;
}

int Sequence::getTempo()
{
    return tempo;
}


void Sequence::addTempoEvent( ControllerEvent* evt )
{
    // add to any track, they will redirect tempo events to the right one.
    // FIXME - not too elegant
    tracks[0].addControlEvent( evt );
}
// adds a tempo event. used during importing - then we know events are in time order so no time is wasted verifying that
void Sequence::addTempoEvent_import( ControllerEvent* evt )
{
    tempoEvents.push_back(evt);
}

// ------------------------------------------- i/o ----------------------------------------
#if 0
#pragma mark -
#endif
/*
 * Called before loading, prepares empty tracks
 */

void Sequence::prepareEmptyTracksForLoading(int amount)
{
    tracks.clearAndDeleteAll();
    for(int n=0; n<amount; n++) tracks.push_back( new Track(getMainFrame(), this) );
}

/*
 * Called when saving <Sequence> ... </Sequence> in .aria file
 */

void Sequence::saveToFile(wxFileOutputStream& fileout)
{

    writeData(wxT("<sequence"), fileout );

    writeData(wxT(" maintempo=\"") + to_wxString(tempo) +
              wxT("\" measureAmount=\"") + to_wxString(measureData->getMeasureAmount()) +
              wxT("\" currentTrack=\"") + to_wxString(currentTrack) +
              wxT("\" beatResolution=\"") + to_wxString(beatResolution) +
              wxT("\" internalName=\"") + internal_sequenceName +
              wxT("\" fileFormatVersion=\"") + to_wxString(current_file_version) +
              wxT("\" channelManagement=\"") + (getChannelManagementType() == CHANNEL_AUTO ? wxT("auto") : wxT("manual")) +
              wxT("\">\n\n"), fileout );

    writeData(wxT("<view xscroll=\"") + to_wxString(x_scroll_in_pixels) +
              wxT("\" yscroll=\"") + to_wxString(y_scroll) +
              wxT("\" zoom=\"") + to_wxString(zoom_percent) +
              wxT("\"/>\n"), fileout );

    measureData->saveToFile(fileout);

    writeData(wxT("<tempo>\n"), fileout );
    // tempo changes
    for(int n=0; n<tempoEvents.size(); n++)
    {
        tempoEvents[n].saveToFile(fileout);
    }
    writeData(wxT("</tempo>\n"), fileout );

    writeData(wxT("<copyright>"), fileout );
    writeData(getCopyright(), fileout );
    writeData(wxT("</copyright>\n"), fileout );

    // tracks
    for(int n=0; n<tracks.size(); n++)
    {
        tracks[n].saveToFile(fileout);
    }


    writeData(wxT("</sequence>"), fileout );

    clearUndoStack();
}

/*
 * Called when reading <sequence> ... </sequence> in .aria file
 */

bool Sequence::readFromFile(irr::io::IrrXMLReader* xml)
{

    importing = true;
    tracks.clearAndDeleteAll();
    measureData->beforeImporting();

    bool copyright_mode = false;
    bool tempo_mode = false;

    // parse the file until end reached
    while(xml && xml->read())
    {


        switch(xml->getNodeType())
        {
            case irr::io::EXN_TEXT:

                if(copyright_mode) setCopyright( fromCString((char*)xml->getNodeData()) );

                break;
            case irr::io::EXN_ELEMENT:
            {

                // ---------- sequence ------
                if (!strcmp("sequence", xml->getNodeName()))
                {

                    const char* maintempo = xml->getAttributeValue("maintempo");
                    if(maintempo != NULL) tempo = atoi( maintempo );
                    else{
                        tempo = 120;
                        std::cerr << "Missing info from file: main tempo" << std::endl;
                    }

                    const char* fileFormatVersion = xml->getAttributeValue("fileFormatVersion");
                    double fileversion = -1;
                    if(fileFormatVersion != NULL)
                    {
                        fileversion = atoi( (char*)fileFormatVersion );
                        if(fileversion > current_file_version )
                        {
                            wxMessageBox( _("Warning : you are opening a file saved with a version of\nAria Maestosa more recent than the version you currently have.\nIt may not open correctly.") );
                        }
                    }
                    const char* measureAmount_c = xml->getAttributeValue("measureAmount");
                    if(measureAmount_c != NULL)
                    {
                        std::cout << "measureAmount = " <<  atoi(measureAmount_c) << std::endl;
                        measureData->setMeasureAmount( atoi(measureAmount_c) );
                        getMainFrame()->changeMeasureAmount( measureData->getMeasureAmount(), false);
                    }
                    else
                    {
                        std::cerr << "Missing info from file: measure amount" << std::endl;
                        return false;
                    }

                    const char* channelManagement_c = xml->getAttributeValue("channelManagement");
                    if( channelManagement_c != NULL )
                    {
                        if( fromCString(channelManagement_c).IsSameAs( wxT("manual") ) ) setChannelManagementType(CHANNEL_MANUAL);
                        else if( fromCString(channelManagement_c).IsSameAs(  wxT("auto") ) ) setChannelManagementType(CHANNEL_AUTO);
                        else std::cerr << "Unknown channel management type : " << channelManagement_c << std::endl;
                    }
                    else
                    {
                        currentTrack = 0;
                        std::cerr << "Missing info from file: channel management" << std::endl;
                    }

                    const char* internalName_c = xml->getAttributeValue("internalName");
                    if( internalName_c != NULL )
                    {
                        internal_sequenceName = fromCString(internalName_c);
                    }
                    else
                    {
                        std::cerr << "Missing info from file: song internal name" << std::endl;
                    }

                    const char* currentTrack_c = xml->getAttributeValue("currentTrack");
                    if( currentTrack_c != NULL ) currentTrack = atoi( currentTrack_c );
                    else
                    {
                        currentTrack = 0;
                        std::cerr << "Missing info from file: current track" << std::endl;
                    }

                    const char* beatResolution_c = xml->getAttributeValue("beatResolution");
                    if( beatResolution_c != NULL ) beatResolution = atoi( beatResolution_c );
                    else
                    {
                        //beatResolution = 960;
                        std::cerr << "Missing info from file: beat resolution" << std::endl;
                        return false;
                    }

                }

                // ---------- view ------
                else if (!strcmp("view", xml->getNodeName()))
                {

                    const char* xscroll_c = xml->getAttributeValue("xscroll");
                    if( xscroll_c != NULL ) x_scroll_in_pixels = atoi( xscroll_c );
                    else
                    {
                        x_scroll_in_pixels = 0;
                        std::cerr << "Missing info from file: x scroll" << std::endl;
                    }

                    if(x_scroll_in_pixels < 0)
                    {
                        std::cerr << "Wrong x_scroll_in_pixels: " << x_scroll_in_pixels << std::endl;
                        x_scroll_in_pixels = 0;
                    }

                    const char* yscroll_c = xml->getAttributeValue("yscroll");
                    if( yscroll_c != NULL ) y_scroll = atoi( yscroll_c );
                    else
                    {
                        y_scroll = 0;
                        std::cerr << "Missing info from file: y scroll" << std::endl;
                    }

                    if(y_scroll < 0)
                    {
                        std::cerr << "Wrong y_scroll: " << y_scroll << std::endl;
                        y_scroll = 0;
                    }

                    const char* zoom_c = xml->getAttributeValue("zoom");
                    if( zoom_c != NULL )
                    {
                        int zoom_i = atoi(zoom_c);
                        if(zoom_i > 0 and zoom_i<501) setZoom( zoom_i );
                        else return false;
                    }
                    else
                    {
                        setZoom( 100 );
                        std::cerr << "Missing info from file: zoom" << std::endl;
                    }

                    if(zoom <= 0)
                    {
                        std::cerr << "Fatal Error: Wrong Zoom: " << zoom  << "(char* = " << zoom_c << ") " << std::endl;
                        setZoom( 100 );
                    }

                }

                // ---------- measure ------
                else if (!strcmp("measure", xml->getNodeName()))
                {
                    if(! measureData->readFromFile(xml) )
                        return false;
                }
                else if (!strcmp("timesig", xml->getNodeName()))
                {
                    if(! measureData->readFromFile(xml) )
                        return false;
                }
                // ---------- track ------
                else if (!strcmp("track", xml->getNodeName()))
                {
                    tracks.push_back( new Track(getMainFrame(), this) );
                    if(! tracks[ tracks.size()-1 ].readFromFile(xml) )
                        return false;
                }

                // ---------- copyright ------
                else if (!strcmp("copyright", xml->getNodeName()))
                {
                    copyright_mode=true;
                }

                // ---------- tempo events ------
                else if (!strcmp("tempo", xml->getNodeName()))
                {
                    tempo_mode=true;
                }
                // all control events in <sequence> are tempo events
                else if (!strcmp("controlevent", xml->getNodeName()))
                {

                    if(!tempo_mode)
                    {
                        std::cerr << "Unexpected control event" << std::endl;
                        continue;
                    }

                    int tempo_tick = -1;
                    const char* tick = xml->getAttributeValue("tick");
                    if( tick != NULL ) tempo_tick = atoi( tick );
                    if( tempo_tick<0 )
                    {
                        std::cerr << "Failed to read tempo event" << std::endl;
                        continue;
                    }

                    int tempo_value = -1;
                    const char* value = xml->getAttributeValue("value");
                    if( value != NULL ) tempo_value = atoi( value );
                    if( tempo_value<0 )
                    {
                        std::cerr << "Failed to read tempo event" << std::endl;
                        continue;
                    }

                    tempoEvents.push_back( new ControllerEvent(this, 201, tempo_tick, tempo_value) );

                }

            }// end case

                break;
            case irr::io::EXN_ELEMENT_END:
            {

                if (!strcmp("sequence", xml->getNodeName()))
                {
                    goto over;
                }
                else if (!strcmp("copyright", xml->getNodeName()))
                {
                    copyright_mode=false;
                }
                else if (!strcmp("tempo", xml->getNodeName()))
                {
                    tempo_mode=false;
                }

            }
                break;

            default:break;
        }//end switch
    }// end while

over:

    measureData->afterImporting();
    clearUndoStack();

    importing = false;
    DisplayFrame::updateHorizontalScrollbar( x_scroll_in_pixels );
    Display::render();

    return true;

}

}
