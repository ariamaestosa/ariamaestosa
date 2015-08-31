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

/** Version of the .aria file format. */
const int CURRENT_FILE_VERSION = 4;

#include "Midi/Sequence.h"

#include "AriaCore.h"

#include "Actions/EditAction.h"
#include "Actions/Paste.h"
#include "Actions/Record.h"
#include "Actions/ScaleTrack.h"
#include "Actions/ScaleSong.h"
#include "Actions/SnapNotesToGrid.h"

// FIXME(DESIGN) : data classes shouldn't refer to GUI classes
#include "Dialogs/WaitWindow.h"

#include "IO/IOUtils.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Players/PlatformMidiManager.h"
#include "Midi/Track.h"
#include "GUI/GraphicalTrack.h"
#include "PreferencesData.h"
#include "Utils.h"

#include <wx/intl.h>
#include <wx/utils.h>
#include <wx/msgdlg.h>
#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

Sequence::Sequence(IPlaybackModeListener* playbackListener, IActionStackListener* actionStackListener,
                   ISequenceDataListener* sequenceDataListener,
                   IMeasureDataListener* measureListener, bool addDefautTrack)
{
    m_quarterNoteResolution     = 960;
    currentTrack                = 0;
    m_tempo                     = 120;
    m_importing                 = false;
    m_loop_enabled              = false;
    m_follow_playback           = PreferencesData::getInstance()->getBoolValue("followPlayback", false);
    m_playback_listener         = playbackListener;
    m_action_stack_listener     = actionStackListener;
    m_seq_data_listener         = sequenceDataListener;
    m_play_with_metronome       = false;
    m_playback_start_tick       = 0;
    m_default_key_type          = KEY_TYPE_C;
    m_default_key_symbol_amount = 0;
    
    m_sequence_filename     = new Model<wxString>( _("Untitled") );
    channelManagement = CHANNEL_AUTO;
    m_copyright = wxT("");

    if (addDefautTrack) addTrack();
    
    m_measure_data = new MeasureData(this, DEFAULT_SONG_LENGTH);
    
    if (measureListener != NULL)
    {
        m_measure_data->addListener( measureListener );
    }
}

// ----------------------------------------------------------------------------------------------------------

Sequence::~Sequence()
{
    if (okToLog)
    {
        std::cout << "[Sequence::~Sequence] cleaning up sequence " << suggestTitle().mb_str() << "..." << std::endl;
    }
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

wxString Sequence::suggestTitle() const
{
    if (not getInternalName().IsEmpty())
    {
        return getInternalName();
    }
    else if (not m_sequence_filename->getValue().IsEmpty())
    {
        return m_sequence_filename->getValue();
    }
    else if (not m_filepath.IsEmpty())
    {
        return extract_filename(m_filepath).BeforeLast('.');
    }
    else
    {
        return  wxString( _("Untitled") );
    }
}

// ----------------------------------------------------------------------------------------------------------

wxString Sequence::suggestFileName() const
{
    if (not m_filepath.IsEmpty())
    {
        return extract_filename(m_filepath).BeforeLast('.');
    }
    else if (not getInternalName().IsEmpty())
    {
        return getInternalName();
    }
    else if (not m_sequence_filename->getValue().IsEmpty())
    {
        return m_sequence_filename->getValue();
    }
    else
    {
        return  wxString( _("Untitled") );
    }
}

// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------- Text Info ----------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Text Info
#endif

void Sequence::setCopyright( wxString copyright )
{
    m_copyright = copyright;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::setInternalName(wxString name)
{
    internal_sequenceName = name;
}

// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------- Getters/Setters --------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Getters/Setters/Actions
#endif

// ----------------------------------------------------------------------------------------------------------

void Sequence::setChannelManagementType(ChannelManagementType type)
{
    channelManagement = type;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::setTicksPerQuarterNote(int res)
{
    m_quarterNoteResolution = res;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::scale(float factor,
                     bool rel_first_note, bool rel_begin, // relative to what (only one must be true)
                     bool affect_selection, bool affect_track, bool affect_song // scale what (only one must be true)
                     )
{
    ASSERT(affect_selection xor affect_track xor affect_song);
    
    int relative_to = -1;
    
    // selection
    if (affect_selection)
    {
        
        if (rel_first_note) relative_to = tracks[currentTrack].getFirstNoteTick(true);
        else if (rel_begin) relative_to = 0;
        
        tracks[currentTrack].action( new Action::ScaleTrack(factor, relative_to, true) );
    }
    
    // track
    else if (affect_track)
    {
        
        if (rel_first_note) relative_to = tracks[currentTrack].getFirstNoteTick();
        else if (rel_begin) relative_to = 0;
        
        tracks[currentTrack].action( new Action::ScaleTrack(factor, relative_to, false) );
    }
    
    // song
    else if (affect_song)
    {
        
        if (rel_first_note)
        {
            
            // find first tick in all tracks [i.e. find the first tick of all tracks and keep the samllest]
            int song_first_tick = -1;
            for (int n=0; n<tracks.size(); n++)
            {
                
                const int track_first_tick = relative_to = tracks[n].getFirstNoteTick();
                if (track_first_tick<song_first_tick or song_first_tick==-1) song_first_tick = track_first_tick;
            }//next
            
            relative_to = song_first_tick;
        }
        else if (rel_begin)
        {
            relative_to = 0;
        }
        
        // scale all tracks
        action( new Action::ScaleSong(factor, relative_to) );
        
    }
    
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::setTempo(int tmp)
{
    m_tempo = tmp;
}

// ----------------------------------------------------------------------------------------------------------

float Sequence::getTempoAtTick(const int tick) const
{
    float outTempo = getTempo();
    
    const int amount = m_tempo_events.size();
    for (int n=0; n<amount; n++)
    {
        if (m_tempo_events[n].getTick() <= tick)
        {
            outTempo = convertTempoBendToBPM(m_tempo_events[n].getValue());
        }
        else
        {
            break;
        }
    }
    return outTempo;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::addTempoEvent(ControllerEvent* evt, wxFloat64* previousValue)
{
    // add to any track, they will redirect tempo events to the right one.
    // FIXME - not too elegant
    tracks[0].addControlEvent(evt, previousValue);
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::addTempoEvent_import( ControllerEvent* evt )
{
    m_tempo_events.push_back(evt);
}

// ----------------------------------------------------------------------------------------------------------

/** A user hit a bug causing tempo events to be out of order, unfortunately I do not know which operation
 *  caused that :( so meanwhile, just make sure to keep them sorted
 */
void Sequence::sortTempoEvents()
{
    m_tempo_events.insertionSort();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::sortTextEvents()
{
    m_text_events.insertionSort();
}

// ----------------------------------------------------------------------------------------------------------

//FIXME: dubious this goes here
void Sequence::snapNotesToGrid()
{
    ASSERT(currentTrack>=0);
    ASSERT(currentTrack<tracks.size());
    
    tracks[ currentTrack ].action( new Action::SnapNotesToGrid() );
    
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::addTextEvent_import(const int x, const wxString& value, const int controller)
{
    ASSERT(isImportMode()); // not to be used when not importing
    ASSERT(value.size() > 0);
    m_text_events.push_back(new TextEvent(controller, x, value) );
}

// ----------------------------------------------------------------------------------------------------------

TextEvent* Sequence::getTextEventAt(int tick, int idController)
{
    const int eventAmount = m_text_events.size();
    for (int n=0; n<eventAmount; n++)
    {
        if (m_text_events[n].getController() == idController and
            m_text_events[n].getTick() == tick)
        {
            return m_text_events.get(n);
        }
    }
    return NULL;
}

// ----------------------------------------------------------------------------------------------------------

ControllerEvent* Sequence::getTempoEventAt(int tick)
{
    const int eventAmount = m_tempo_events.size();
    for (int n=0; n<eventAmount; n++)
    {
        if (m_tempo_events[n].getTick() == tick)
        {
            return m_tempo_events.get(n);
        }
    }
    return NULL;
}

// ----------------------------------------------------------------------------------------------------------

wxString Sequence::addTextEvent(TextEvent* evt)
{
    wxString previousValue;

    // don't bother checking order if we're importing, we know its in time order and all
    // FIXME - what about 'addControlEvent_import' ??
    if (isImportMode())
    {
        m_text_events.push_back( evt );
        return previousValue;
    }

    const int eventAmount = m_text_events.size();
    for (int n=0; n<eventAmount; n++)
    {
        if (m_text_events[n].getTick() == evt->getTick())
        {
            // if there is already an even of same type at same time, remove it first
            if (m_text_events[n].getController() == evt->getController() )
            {
                previousValue = m_text_events[n].getTextValue();
                m_text_events.erase(n);
            }
            m_text_events.add( evt, n );
            return previousValue;
        }
        else if ( m_text_events[n].getTick() > evt->getTick() )
        {
            m_text_events.add( evt, n );
            return previousValue;
        }//endif

    }//next

    m_text_events.push_back( evt );
    return previousValue;
}

// ----------------------------------------------------------------------------------------------------------

int Sequence::getLastTickInSequence() const
{
    int tick = 0;
    for (int n=0; n<tracks.size(); n++)
    {
        if (tracks[n].getNoteAmount() > 0)
        {
            const int noteTick = tracks[n].getNoteEndInMidiTicks(tracks[n].getNoteAmount() - 1);
            if (noteTick > tick)
            {
                tick = noteTick;
            }
        }
    }
    
    return tick;
}


// ----------------------------------------------------------------------------------------------------------
void Sequence::updateTrackPlayingStatus()
{
    bool soloTrackFound;
    int count;
    int n;
    
    soloTrackFound = false;
    count = tracks.size();
    for (n=0 ; n<count && !soloTrackFound ; n++)
    {
        if ( tracks[n].isSoloed() )
        {
            soloTrackFound = true;
        }
    }
    
   
    if (soloTrackFound)
    {
        // Solo status prevails
        for (n=0 ; n<count ; n++)
        {
            tracks[n].setPlayed(tracks[n].isSoloed());
        }
    }
    else
    {
        for (n=0 ; n<count ; n++)
        {
            tracks[n].setPlayed(!tracks[n].isMuted());
        }
    }
}


// ----------------------------------------------------------------------------------------------------------
// -------------------------------------------- Actions and Undo --------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Actions and Undo
#endif


void Sequence::action( Action::MultiTrackAction* actionObj)
{
    addToUndoStack( actionObj );
    actionObj->setParentSequence(this, new SequenceVisitor(this));
    actionObj->perform();
    
    if (m_action_stack_listener != NULL) m_action_stack_listener->onActionStackChanged();
    
    ASSERT(invariant());
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::addToUndoStack( Action::EditAction* actionObj )
{
    undoStack.push_back(actionObj);

    if (PlatformMidiManager::get()->isRecording() and
        dynamic_cast<Action::Record*>(actionObj) == NULL and
        undoStack.size() >= 2)
    {
        // special case when recording : the "record" action must stay at the top of the stack until
        // recording is completed
        undoStack.swap(undoStack.size() - 1, undoStack.size() - 2);
    }
    
    // remove old actions from undo stack, to not take memory uselessly
    if (undoStack.size() > 8) undoStack.erase(0);
    
    if (m_action_stack_listener != NULL) m_action_stack_listener->onActionStackChanged();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::undo()
{
    if (undoStack.size() < 1)
    {
        // nothing to undo
        wxBell();
        return;
    }

    Action::EditAction* lastAction = undoStack.get( undoStack.size() - 1 );
    if (not lastAction->canUndoNow())
    {
        wxBell();
        return;
    }
    
    lastAction->undo();
    undoStack.erase( undoStack.size() - 1 );

    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
    
    if (m_action_stack_listener != NULL) m_action_stack_listener->onActionStackChanged();
    
    ASSERT(invariant());
}

// ----------------------------------------------------------------------------------------------------------

wxString Sequence::getTopActionName() const
{
    if (undoStack.size() == 0) return wxEmptyString;
    
    const Action::EditAction* lasAction = undoStack.getConst( undoStack.size() - 1 );
    return lasAction->getName();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::clearUndoStack()
{
    undoStack.clearAndDeleteAll();
    if (m_action_stack_listener != NULL) m_action_stack_listener->onActionStackChanged();
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- Tracks ---------------------------------------------------
// ----------------------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark Tracks
#endif

Track* Sequence::addTrack(bool belowActive)
{
    Track* result = new Track(this);
    
    if (belowActive and currentTrack >= 0 and currentTrack < tracks.size())
    {
        // add new track below active one
        tracks.add(result, currentTrack+1);
    }
    else
    {
        tracks.push_back(result);
    }
    
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
    
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        m_listeners.get(n)->onTrackAdded(result);
    }
    
    return result;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::addTrack(Track* track)
{
    tracks.push_back(track);
    
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        m_listeners.get(n)->onTrackAdded(track);
    }
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::prepareEmptyTracksForLoading(int amount)
{
    const int tc = tracks.size();
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        for (int m=0; m<tc; m++)
        {
            m_listeners.get(n)->onTrackRemoved( tracks.get(m) );
        }
    }
    
    clear();
    for (int n=0; n<amount; n++)
    {
        addTrack();
    }
}

// ----------------------------------------------------------------------------------------------------------

Track* Sequence::removeSelectedTrack()
{
    Track* removedTrack = tracks.get(currentTrack);
    if (currentTrack<0 or currentTrack>tracks.size()-1) return NULL;

    tracks.remove( currentTrack );

    while (currentTrack > tracks.size()-1) currentTrack -= 1;

    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
    
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        m_listeners.get(n)->onTrackRemoved(removedTrack);
    }
    
    return removedTrack;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::deleteTrack(int id)
{
    ASSERT_E(id,>=,0);
    ASSERT_E(id,<,tracks.size());
    
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        m_listeners.get(n)->onTrackRemoved(tracks.get(id));
    }
    
    tracks.erase( id );

    while (currentTrack > tracks.size()-1) currentTrack -= 1;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::deleteTrack(Track* track)
{    
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        m_listeners.get(n)->onTrackRemoved(track);
    }
    
    tracks.erase( track );
    
    while (currentTrack > tracks.size()-1) currentTrack -= 1;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::setCurrentTrackID(int ID)
{
    currentTrack = ID;
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::setCurrentTrack(Track* track)
{
    ASSERT(track != NULL);
    
    const int trackAmount = tracks.size();
    for (int n=0; n<trackAmount; n++)
    {
        if ( &tracks[n] == track )
        {
            currentTrack = n;
            return;
        }
    }

    std::cerr << "Error: void Sequence::setCurrentTrack(Track* track) couldn't find any matching track" << std::endl;
    ASSERT(false);
}


// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- Playback -------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Playback
#endif

// FIXME: I doubt this method goes here. Sequence is primarly a data class.
// and MainFrame handles the rest of playback start/stop, why have SOME of it here???
void Sequence::spacePressed()
{
    if (not PlatformMidiManager::get()->isPlaying() and not PlatformMidiManager::get()->isRecording())
    {
        if (isPlaybackMode())
        {
            return;
        }

        if (m_playback_listener != NULL) m_playback_listener->onEnterPlaybackMode();

        int startTick = -1;
        bool success = PlatformMidiManager::get()->playSelected(this, &startTick);
        setPlaybackStartTick( startTick );

        // FIXME: there's MainFrame::playback_mode AND MainPane::enterPlayLoop/exitPlayLoop. Fix this MESS
        if (not success or startTick == -1) // failure
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
        if (m_playback_listener != NULL) m_playback_listener->onLeavePlaybackMode();

        PlatformMidiManager* midi = PlatformMidiManager::get();
        if (midi->isRecording()) midi->stopRecording();
        midi->stop();
        
        if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
    }

}

// --------------------------------------------------------------------------------------------------

void Sequence::setPlaybackStartTick(int newValue)
{
    m_playback_start_tick = newValue;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------- Copy/Paste -----------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Copy/Paste
#endif

void Sequence::copy()
{
    tracks[currentTrack].copy();
}

// ----------------------------------------------------------------------------------------------------------
/*
void Sequence::paste()
{
    tracks[currentTrack].action( new Action::Paste(false) );
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void Sequence::pasteAtMouse()
{
    tracks[currentTrack].action( new Action::Paste(true) );
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();
}*/

// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------ I/O -----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark I/O
#endif

void Sequence::saveToFile(wxFileOutputStream& fileout)
{

    writeData(wxT("<sequence"), fileout );

    writeData(wxT(" maintempo=\"")           + to_wxString(m_tempo) +
              wxT("\" measureAmount=\"")     + to_wxString(m_measure_data->getMeasureAmount()) +
              wxT("\" currentTrack=\"")      + to_wxString(currentTrack) +
              wxT("\" beatResolution=\"")    + to_wxString(m_quarterNoteResolution) +
              wxT("\" internalName=\"")      + internal_sequenceName +
              // FIXME: file format version doesn't quite belong in <sequence> anymore since that's not the top-level element anymore...
              wxT("\" fileFormatVersion=\"") + to_wxString(CURRENT_FILE_VERSION) +
              wxT("\" channelManagement=\"") + (getChannelManagementType() == CHANNEL_AUTO ?
                                                wxT("auto") : wxT("manual")) +
              wxT("\" metronome=\"")         + (m_play_with_metronome ? wxT("true") : wxT("false")) +
              wxT("\">\n\n"), fileout );
    
    //writeData(wxT("<view xscroll=\"") + to_wxString(m_x_scroll_in_pixels) +
    //          wxT("\" yscroll=\"")    + to_wxString(y_scroll) +
    //          wxT("\" zoom=\"")       + to_wxString(m_zoom_percent) +
    //          wxT("\"/>\n"), fileout );
    
    m_measure_data->saveToFile(fileout);
    
    // ---- tempo changes
    writeData(wxT("<tempo>\n"), fileout );    
    const int tempo_count = m_tempo_events.size();
    for (int n=0; n<tempo_count; n++)
    {
        m_tempo_events[n].saveToFile(fileout);
    }
    writeData(wxT("</tempo>\n"), fileout );
    
    // ---- text events
    writeData(wxT("<text>\n"), fileout );
    const int text_count = m_text_events.size();
    for (int n=0; n<text_count; n++)
    {
        m_text_events[n].saveToFile(fileout);
    }
    writeData(wxT("</text>\n"), fileout );
    
    // ---- copyright
    writeData(wxT("<copyright>\n"), fileout );
    writeData(getCopyright(), fileout );
    writeData(wxT("</copyright>\n"), fileout );
    
    
    // ---- defaut key signature 
    writeData(wxT("<defaultkeysig "), fileout );
    writeData(wxT("keytype=\""), fileout );
    writeData(to_wxString(m_default_key_type), fileout );
    writeData(wxT("\" keysymbolamount=\""), fileout );
    writeData(to_wxString(m_default_key_symbol_amount), fileout );
    writeData(wxT("\" />\n\n"), fileout );
    
    
    // ---- tracks
    for (int n=0; n<tracks.size(); n++)
    {
        tracks[n].saveToFile(fileout);
    }
    
    writeData(wxT("</sequence>"), fileout );
    
    clearUndoStack();
}

// ----------------------------------------------------------------------------------------------------------

bool Sequence::readFromFile(irr::io::IrrXMLReader* xml, GraphicalSequence* gseq)
{
    m_importing = true;
    
    const int tc = tracks.size();
    const int count = m_listeners.size();
    for (int n=0; n<count; n++)
    {
        for (int m=0; m<tc; m++)
        {
            m_listeners.get(n)->onTrackRemoved( tracks.get(m) );
        }
    }
    
    tracks.clearAndDeleteAll();
    
    once
    {
        ScopedMeasureITransaction tr(m_measure_data->startImportTransaction());
        
        ASSERT (strcmp("sequence", xml->getNodeName()) == 0);
            
        const char* maintempo = xml->getAttributeValue("maintempo");
        const int atoi_out = atoi( maintempo );
        if (maintempo != NULL and atoi_out > 0)
        {
            m_tempo = atoi_out;
        }
        else
        {
            m_tempo = 120;
            std::cerr << "Missing info from file: main tempo" << std::endl;
        }
        
        const char* fileFormatVersion = xml->getAttributeValue("fileFormatVersion");
        int fileversion = -1;
        if (fileFormatVersion != NULL)
        {
            fileversion = atoi( (char*)fileFormatVersion );
            if (fileversion > CURRENT_FILE_VERSION )
            {
                if (WaitWindow::isShown()) WaitWindow::hide();
                wxMessageBox( _("Warning : you are opening a file saved with a version of\nAria Maestosa more recent than the version you currently have.\nIt may not open correctly.") );
            }
        }
        const char* measureAmount_c = xml->getAttributeValue("measureAmount");
        if (measureAmount_c != NULL)
        {
            std::cout << "[Sequence::readFromFile] measureAmount = " <<  atoi(measureAmount_c) << std::endl;
            
            {
                ScopedMeasureTransaction tr2(m_measure_data->startTransaction());
                tr2->setMeasureAmount( atoi(measureAmount_c) );
            }
        }
        else
        {
            std::cerr << "Missing info from file: measure amount" << std::endl;
            return false;
        }
        
        const char* channelManagement_c = xml->getAttributeValue("channelManagement");
        if (channelManagement_c != NULL)
        {
            if ( fromCString(channelManagement_c).IsSameAs( wxT("manual") ) ) setChannelManagementType(CHANNEL_MANUAL);
            else if ( fromCString(channelManagement_c).IsSameAs(  wxT("auto") ) ) setChannelManagementType(CHANNEL_AUTO);
            else std::cerr << "Unknown channel management type : " << channelManagement_c << std::endl;
        }
        else
        {
            std::cerr << "Missing info from file: channel management" << std::endl;
        }
        
        const char* internalName_c = xml->getAttributeValue("internalName");
        if (internalName_c != NULL)
        {
            internal_sequenceName = fromCString(internalName_c);
        }
        else
        {
            std::cerr << "Missing info from file: song internal name" << std::endl;
        }
        
        const char* currentTrack_c = xml->getAttributeValue("currentTrack");
        if (currentTrack_c != NULL)
        {
            currentTrack = atoi( currentTrack_c );
        }
        else
        {
            currentTrack = 0;
            std::cerr << "Missing info from file: current track" << std::endl;
        }
        
        const char* beatResolution_c = xml->getAttributeValue("beatResolution");
        if (beatResolution_c != NULL)
        {
            m_quarterNoteResolution = atoi( beatResolution_c );
        }
        else
        {
            //m_quarterNoteResolution = 960;
            std::cerr << "Missing info from file: beat resolution" << std::endl;
            return false;
        }
        
        const char* metronome_c = xml->getAttributeValue("metronome");
        if (metronome_c != NULL)
        {
            if (strcmp(metronome_c, "true") == 0)
            {
                m_play_with_metronome = true;
            }
            else if (strcmp(metronome_c, "false") == 0)
            {
                m_play_with_metronome = false;
            }
            else
            {
                std::cerr << "Invalid value for 'metronome' property : " << metronome_c << "\n";
                m_play_with_metronome = false;
            }
        }
        else
        {
            m_play_with_metronome = false;
        }

        
        bool copyright_mode = false;
        bool default_key_sig = false;
        bool tempo_mode = false;
        bool text_mode = false;
        
        bool done = false;
        
        // parse the file until end reached
        while (xml != NULL and xml->read() and not done)
        {

            switch (xml->getNodeType())
            {
                case irr::io::EXN_TEXT:
                {
                    if (copyright_mode) setCopyright( fromCString((char*)xml->getNodeData()) );
                    
                    if (default_key_sig)
                    {
                        const char* keytype = xml->getAttributeValue("keytype");
                        if (keytype != NULL)
                        {
                            m_default_key_type = KeyType(atoi(keytype));
                        }
                        
                        const char* keysymbolamount = xml->getAttributeValue("keysymbolamount");
                        if (keysymbolamount != NULL)
                        {
                            m_default_key_symbol_amount = atoi(keysymbolamount);
                        }
                    }

                    break;
                }
                case irr::io::EXN_ELEMENT:
                {                
                    
                    // ---------- measure ------
                    if (strcmp("measure", xml->getNodeName()) == 0)
                    {
                        if (not m_measure_data->readFromFile(xml) ) return false;
                    }
                    
                    // ---------- time sig ------
                    else if (strcmp("timesig", xml->getNodeName()) == 0)
                    {
                        if (not m_measure_data->readFromFile(xml)) return false;
                    }
                    
                    // ---------- track ------
                    else if (strcmp("track", xml->getNodeName()) == 0)
                    {
                        Track* newTrack = new Track(this);
                        addTrack( newTrack );
                        
                        if (not newTrack->readFromFile(xml, gseq)) return false;
                    }
                    
                    // ---------- copyright ------
                    else if (strcmp("copyright", xml->getNodeName()) == 0)
                    {
                        copyright_mode=true;
                    }
                    
                    // ---------- defaut key signature ------
                    else if (strcmp("defaultkeysig", xml->getNodeName()) == 0)
                    {
                        default_key_sig=true;
                    }

                    // ---------- tempo events ------
                    else if (strcmp("tempo", xml->getNodeName()) == 0)
                    {
                        tempo_mode = true;
                    }
                    
                    // ---------- text events ------
                    else if (strcmp("text", xml->getNodeName()) == 0)
                    {
                        text_mode = true;
                    }
                    
                    // all control events in <sequence> are tempo events
                    else if (strcmp("controlevent", xml->getNodeName()) == 0)
                    {

                        if (tempo_mode)
                        {
                            ControllerEvent* temp = new ControllerEvent(0, 0, 0);
                            if (not temp->readFromFile(xml))
                            {
                                std::cerr << "Failed to read tempo event for .aria file\n";
                                delete temp;
                            }
                            else
                            {
                                m_tempo_events.push_back( temp );
                            }
                        }
                        else if (text_mode)
                        {
                            TextEvent* temp = new TextEvent(0, 0, wxT(""));
                            if (not temp->readFromFile(xml))
                            {
                                std::cerr << "Failed to read text event for .aria file\n";
                                delete temp;
                            }
                            else
                            {
                                m_text_events.push_back( temp );
                            }
                        }
                        else
                        {
                            std::cerr << "Unexpected control event" << std::endl;
                            continue;
                        }
                    }

                    break;
                }// end case
                
                case irr::io::EXN_ELEMENT_END:
                {
                    
                    if (strcmp("sequence", xml->getNodeName()) == 0)
                    {
                        done = true;
                    }
                    else if (strcmp("copyright", xml->getNodeName()) == 0)
                    {
                        copyright_mode = false;
                    }
                    else if (strcmp("defaultkeysig", xml->getNodeName()) == 0)
                    {
                        default_key_sig = false;
                    }
                    else if (strcmp("tempo", xml->getNodeName()) == 0)
                    {
                        tempo_mode = false;
                    }
                    else if (strcmp("text", xml->getNodeName()) == 0)
                    {
                        text_mode = false;
                    }
                    break;
                }
                
                default:
                    break;
            }//end switch
        }// end while

    }
    
// over:
    clearUndoStack();
    
    
    // A user hit a bug causing tempo events to be out of order, unfortunately I do not know which operation
    // caused that :( so meanwhile, just make sure to keep them sorted
    sortTempoEvents();
    sortTextEvents();

    m_importing = false;
    if (m_seq_data_listener != NULL) m_seq_data_listener->onSequenceDataChanged();

    parseBackgroundTracks();
    
    updateTrackPlayingStatus();

    ASSERT(invariant());
    
    return true;

}

void Sequence::parseBackgroundTracks()
{
    GraphicalTrack* graphicalTrack;
 
    for (int n=0; n<tracks.size(); n++)
    {
        graphicalTrack = tracks[n].getGraphics();
        graphicalTrack->getEditorFor(SCORE)->addBackgroundTracks();
        graphicalTrack->getEditorFor(GUITAR)->addBackgroundTracks();
        graphicalTrack->getEditorFor(DRUM)->addBackgroundTracks();
        graphicalTrack->getEditorFor(KEYBOARD)->addBackgroundTracks();
        graphicalTrack->getEditorFor(CONTROLLER)->addBackgroundTracks();
    }
}


bool Sequence::invariant()
{
    if (m_tempo_events.size() > 1)
    {
        int tempo_tick = m_tempo_events[0].getTick();
        for (int n = 1; n < m_tempo_events.size(); n++)
        {
            int newTick = m_tempo_events[n].getTick();
            ASSERT_E(tempo_tick, <=, newTick);
            tempo_tick = newTick;
        }
    }
    
    if (m_text_events.size() > 1)
    {
        int text_tick = m_text_events[0].getTick();
        for (int n = 1; n < m_text_events.size(); n++)
        {
            int newTick = m_text_events[n].getTick();
            ASSERT_E(text_tick, <=, newTick);
            text_tick = newTick;
        }
    }
    
    
    for (int n = 0; n < tracks.size(); n++)
    {
        tracks[n].invariant();
    }
    return true;
}
