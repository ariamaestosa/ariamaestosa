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
#include "IO/MidiFileReader.h"
#include "IO/IOUtils.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "GUI/MainFrame.h"
#include "GUI/GraphicalTrack.h"
#include "Midi/MeasureData.h"
#include "Dialogs/WaitWindow.h"
#include "Editors/DrumEditor.h"
#include "Editors/KeyboardEditor.h"
#include "Editors/ScoreEditor.h"

#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"

#include <cmath>
#include <string>

// TODO: remove this include, this file should not need to know about wx controls
#include <wx/textctrl.h>


bool AriaMaestosa::loadMidiFile(GraphicalSequence* gseq, wxString filepath, std::set<wxString>& warnings)
{
    Sequence* sequence = gseq->getModel();
    
    // raise import flag
    sequence->importing = true;

    // the stream used to read the input file
    jdkmidi::MIDIFileReadStreamFile rs( filepath.mb_str() );

    // the object which will hold all the tracks
    jdkmidi::MIDIMultiTrack jdksequence;

    // the object which loads the tracks into the tracks object
    jdkmidi::MIDIFileReadMultiTrack track_loader( &jdksequence );

    // the object which parses the midifile and gives it to the multitrack loader
    jdkmidi::MIDIFileRead reader( &rs, &track_loader );

    // load the midifile into the multitrack object
    if (not reader.Parse())
    {
        std::cout << "Error: could not parse midi file" << std::endl;
        return false;
    }

    jdkmidi::MIDITrack* track;
    jdkmidi::MIDITimedBigMessage* event;

    sequence->setChannelManagementType(CHANNEL_MANUAL);
    sequence->getMeasureData()->beforeImporting();
    
    const int resolution = jdksequence.GetClksPerBeat();
    sequence->setTicksPerBeat( resolution );

    const int drum_note_duration = resolution/32+1;

    int lastEventTick = 0; // last event tick for whole song, to find its duration

    bool firstTempoEvent=true;

    const int trackAmount = jdksequence.GetNumTracks();

    // check for empty tracks
    int real_track_amount=0;
    for (int trackID=0; trackID<trackAmount; trackID++)
    {
        if (jdksequence.GetTrack( trackID )->GetNumEvents() == 0){} // empty track...
        else real_track_amount++;
    }

    gseq->prepareEmptyTracksForLoading(real_track_amount /*16*/);
        
     // ----------------------------------- for each track -------------------------------------
    int realTrackID=-1;
    for (int trackID=0; trackID<trackAmount; trackID++)
    {
        track = jdksequence.GetTrack( trackID );

        // FIXME: to get reasonable performance, I have no choice but to kill the progress indicator,
        // wxYield pauses for way too long??
        // if there is a huge amount of tracks, only call wxYield once in a while because calling
        // it everytime could be very slow
        //if ((trackAmount < 10) or (trackID % (trackAmount/10) == 0))
        //{
        //    WaitWindow::setProgress(
        //                            (int)(
        //                                  log(trackID)*100/log(trackAmount)
        //                                  )
        //                            );
        //    wxYield(); // FIXME - use a thread instead
        //}
        
        // ----------------------------------- for each event -------------------------------------

        wxString trackName =  wxT("");

        const int eventAmount = track->GetNumEvents();

        int programChanges = 0;
        
        if (eventAmount == 0) continue;
        realTrackID ++;
        Track* ariaTrack = sequence->getTrack(realTrackID);

        int lastEventTick_inTrack = 0;

        int last_channel = -1;

        bool need_reorder = false;

        for (int eventID=0; eventID<eventAmount; eventID++)
        {

            event = track->GetEvent( eventID );

            const int tick = event->GetTime();

            if (tick < lastEventTick_inTrack) need_reorder = true;
            else                              lastEventTick_inTrack = tick;

            const int channel = event->GetChannel();
            if (channel != last_channel and last_channel != -1 and not event->IsMetaEvent() and
               not event->IsTextEvent() and not event->IsTempo() and
               not event->IsSystemMessage() and not event->IsSysEx())
            {
                if (event->IsNoteOn())
                {
                    warnings.insert( _("This MIDI file has tracks that play on multiple MIDI channels. This is not supported by Aria Maestosa.") );
                    ariaTrack->setChannel(channel);
                    last_channel = channel;
                }

            }

            if (last_channel == -1 and not event->IsMetaEvent() and
               not event->IsTextEvent() and not event->IsTempo() and
               not event->IsSystemMessage() and not event->IsSysEx())
            {
                ariaTrack->setChannel(channel); // its first iteration
                last_channel = channel;
            }

            // ----------------------------------- note on -------------------------------------
            if ( event->IsNoteOn() )
            {

                const int note = ((channel == 9) ? event->GetNote() : 131 - event->GetNote());
                const int volume = event->GetVelocity();

                ariaTrack->addNote_import(note,
                                         tick,
                                         tick+drum_note_duration /*temporary end until the corresponding note off event is found*/,
                                         volume);

                continue;
            }
            // ----------------------------------- note off -------------------------------------
            else if ( event->IsNoteOff() )
            {
                if (channel == 9) continue; // drum notes have no durations so dont care about this event
                const int note = (131 - event->GetNote());

                // a note off event was found, find to which note on event it corresponds
                // (start iterating from the end, because since events are in order it will probably be found near the end)
                for (int n=ariaTrack->getNoteAmount()-1; n>-1; n--)
                {

                    if (ariaTrack->getNotePitchID(n) == note and
                       ariaTrack->getNoteEndInMidiTicks(n)==ariaTrack->getNoteStartInMidiTicks(n)+drum_note_duration )
                    {
                        ariaTrack->setNoteEnd_import( tick, n );

                        ASSERT_E(ariaTrack->getNoteEndInMidiTicks(n), ==, tick);

                        break;
                    }//end if


                    if (n == 0)
                    {
                        warnings.insert( wxString::Format(_("This MIDI file appears to be incorrect; a note at tick %i in channel %i does not appear to have an end"), tick, channel) );
                    }
                } // next

                continue;
            }
            // ----------------------------------- control change -------------------------------------
            else if ( event->IsControlChange() )
            {
                const int controllerID = event->GetController();
                const int value = 127-event->GetControllerValue();

                if (controllerID > 31 and controllerID < 64)
                {
                    // LSB... not supported by Aria ATM
                    std::cout << "WARNING: This MIDI files contains LSB controller data. Aria does not support fine control changes and will discard this info." << std::endl;
                    continue;
                }


                if (controllerID == 3 or controllerID == 6 or controllerID == 9 or (controllerID > 19 and controllerID < 32) or
                   controllerID == 79 or (controllerID > 84 and controllerID < 91)
                   or (controllerID > 95 and controllerID < 200 and controllerID!=127 /*stereo mode*/))
                {
                    if (controllerID == 6 or controllerID == 38 or controllerID == 100 or controllerID == 101)
                    {
                        warnings.insert( _("This MIDI file uses Registered Parameters, which are currently not supported by Aria Maestosa.") );
                    }
                    else
                    {
                        warnings.insert( wxString::Format(_("This MIDI file uses unsupported MIDI controller #%i. Data related to this controller will be discarded."), controllerID) );
                    }
                    continue;
                }

                ariaTrack->addControlEvent_import(tick, value, controllerID);

                continue;
            }
            // ----------------------------------- pitch bend -------------------------------------
            else if ( event->IsPitchBend() )
            {

                int pitchBendVal = event->GetBenderValue();

                int value = (int)round( (pitchBendVal+8064.0)*128.0/16128.0 );
                if (value>127) value=127;
                if (value<0) value=0;

                ariaTrack->addControlEvent_import(tick, 127-value, 200);

                continue;
            }
            // ----------------------------------- program chnage -------------------------------------
            else if ( event->IsProgramChange() )
            {
                programChanges++;
                if (programChanges > 1)
                {
                    warnings.insert( _("This MIDI file plays several instruments on the same track; this is currently not supported by Aria Maestosa.") );
                }
                
                const int instrument = event->GetPGValue();

                if (channel == 9)  ariaTrack->setDrumKit(instrument);
                else               ariaTrack->setInstrument(instrument);

                continue;
            }
            // ----------------------------------- tempo -------------------------------------
            else if ( event->IsTempo() )
            {

                const int tempo = event->GetTempo32()/32;

                if (firstTempoEvent)
                {
                    sequence->setTempo(tempo);

                    //FIXME: should *not* directly access the GUI control from here!!!
                    getMainFrame()->m_tempo_ctrl->SetValue( to_wxString(tempo) );
                    firstTempoEvent=false;
                    continue;
                }
                else
                {
                    sequence->addTempoEvent_import(
                                                   new ControllerEvent(201,
                                                                       tick,
                                                                       127-(int)((tempo-20.0)/380.0*128.0)
                                                                       )
                                                   );
                    continue;
                }
                // ----------------------------------- time key/sig and beat marker -------------------------------------
            }
            else if ( event->IsTimeSig() )
            {
                sequence->getMeasureData()->addTimeSigChange_import( tick, (int)event->GetTimeSigNumerator(), (int)event->GetTimeSigDenominator() );
                continue;
            }
            else if ( event->IsKeySig() )
            {
                /*
                 This meta event is used to specify the key (number of sharps or flats) and scale (major or minor) of a sequence.
                 A positive value for the key specifies the number of sharps and a negative value specifies the number of flats.
                 A value of 0 for the scale specifies a major key and a value of 1 specifies a minor key.
                 source: http://www.sonicspot.com/guide/midifiles.html
                 */
                int amount = (int)event->GetKeySigSharpFlats();

                for (int trackn=0; trackn<real_track_amount; trackn++)
                {
                    if (amount > 0)
                    {
                        sequence->getTrack(trackn)->setKey(amount, KEY_TYPE_SHARPS);
                        //sequence->getTrack(trackn)->graphics->scoreEditor->loadKey(SHARP, amount);
                        //sequence->getTrack(trackn)->graphics->keyboardEditor->loadKey(SHARP, amount);
                    }
                    else if (amount < 0)
                    {
                        sequence->getTrack(trackn)->setKey(-amount, KEY_TYPE_FLATS);
                        //sequence->getTrack(trackn)->graphics->scoreEditor->loadKey(FLAT, -amount);
                        //sequence->getTrack(trackn)->graphics->keyboardEditor->loadKey(FLAT, -amount);
                    }
                }
                // FIXME - does midi allow a different key for each track?

            }
            /*
            else if ( event->IsBeatMarker() )
            {
                //std::cout << "Beat marker: " << (int)event->GetByte1() << " " << (int)event->GetByte2() << " " << (int)event->GetByte3() << ", type=" << (int)event->GetType() << std::endl;
            }
             */
            else  if ( event->IsTextEvent() ) // ----------------------------------- track name / text events -------------------------------------
            {

                // sequence/track name
                if ((int)event->GetByte1() == 3)
                {

                    const int length = event->GetSysEx()->GetLength(); // get name length
                    char name[length+1];
                    name[length] = 0; // make zero-terminated

                    char* buf = (char*) event->GetSysEx()->GetBuf();
                    for (int n=0; n<length; n++) name[n] = buf[n];

                    //std::cout << "s/t name: " << name << std::endl;

                    trackName = fromCString(name);
                    //if (trackName.Length() == 0) trackName = _("Untitled");
                    continue;
                }
                else if ((int)event->GetByte1() == 2) // copyright
                {

                    const int length = event->GetSysEx()->GetLength(); // get copyright length
                    char copyright[length+1];
                    copyright[length] = 0; // make zero-terminated

                    char* buf = (char*) event->GetSysEx()->GetBuf();
                    for (int n=0; n<length; n++) copyright[n] = buf[n];

                    sequence->setCopyright( fromCString(copyright) );
                    continue;
                }
                /*
                else if ((int)event->GetByte1() == 4) // instrument name
                {
                    const char* text = (char*) event->GetSysEx()->GetBuf();
                    std::cout << "instrument name : " << text << " (ignored)"<< std::endl;
                }
                else if ((int)event->GetByte1() == 5) // lyrics
                {
                    const char* text = (char*) event->GetSysEx()->GetBuf();
                    std::cout << "lyrics : " << text << " (ignored)"<< std::endl;
                }
                else if ((int)event->GetByte1() == 1) // comments
                {
                    const char* text = (char*) event->GetSysEx()->GetBuf();
                    std::cout << "comment : " << text << " (ignored)"<< std::endl;
                }
                else
                {
                    const char* text = (char*) event->GetSysEx()->GetBuf();
                    std::cout << "Unknown text=" << text << ", channel="<< channel << ", byte1=" << (int)event->GetByte1() << std::endl;
                }*/


            }
            //else{ std::cout << "ignored event" << std::endl; }

            /*
            else if ( event->IsSystemMessage() )
            {
                std::cout << "System Message: " << (int)event->GetByte1() << " " << (int)event->GetByte2() << " " << (int)event->GetByte3() << " " << (int)event->GetType() << std::endl;
            }
            else if ( event->IsSysEx() )
            {
                std::cout << "Sysex: " << (int)event->GetByte1() << " " << (int)event->GetByte2() << " " << (int)event->GetByte3() << " " << (int)event->GetType() << std::endl;
            }
            else if ( event->IsMetaEvent() )
            {
                std::cout << "Meta Event: " << (int)event->GetByte1() << " " << (int)event->GetByte2() << " " << (int)event->GetByte3() << ", type= " << (int)event->GetType();

                if ((int)event->GetByte1() == 0) std::cout << "---> sequence number" << std::endl;
                if ((int)event->GetByte1() == 1) std::cout << "---> text" << std::endl;
                if ((int)event->GetByte1() == 2) std::cout << "---> copyright" << std::endl;
                if ((int)event->GetByte1() == 3) std::cout << "---> track name" << std::endl;
                if ((int)event->GetByte1() == 4) std::cout << "---> instrument name" << std::endl;
                if ((int)event->GetByte1() == 5) std::cout << "---> lyrics name" << std::endl;
                if ((int)event->GetByte1() == 6) std::cout << "---> marker" << std::endl;
                if ((int)event->GetByte1() == 7) std::cout << "---> cue point" << std::endl;

                if ((int)event->GetByte1() == 32) std::cout << "---> prefix" << std::endl;

                if ((int)event->GetByte1() == 47) std::cout << "---> end of track" << std::endl;

                if ((int)event->GetByte1() == 81) std::cout << "---> tempo" << std::endl;

                if ((int)event->GetByte1() == 84) std::cout << "---> SMPTE" << std::endl;
                if ((int)event->GetByte1() == 88) std::cout << "---> Time signature" << std::endl;

                if ((int)event->GetByte1() == 240) std::cout << "---> Sys Ex" << std::endl;

            }
            else
            {
                std::cout << "Unknown --> " << (char)event->GetByte1() << " " << (char)event->GetByte2() << " " << (char)event->GetByte3() << std::endl;
            }
*/
        }//next event

        //std::cout << "name is " << toCString(trackName) << "  last_channel=" << last_channel << " trackID=" << trackID << std::endl;

        if (last_channel != -1)
        {
            if (trackName.Length() == 0) trackName = _("Untitled");
            ariaTrack->setName(trackName);
        }

        if (trackID == 0)
        {
            sequence->setInternalName( trackName );
        }

        if (ariaTrack->getChannel() == 9)
        {
            ariaTrack->graphics->setEditorMode(DRUM);
        }
        else
        {
            // KEYBOARD editor is the default, but set it anyway, will force the instrument name
            // to be updated
            ariaTrack->graphics->setEditorMode(KEYBOARD);
        }

        // FIXME: when does it happen?? a MIDI file contains only deltas AFAIK, I don't quite see how you can detect an incorrect order
        if (need_reorder)
        {
            std::cerr << "* midi file is wrong, it will be necessary to reorder midi events" << std::endl;
            ariaTrack->reorderNoteVector();
            ariaTrack->reorderControlVector();
        }
        ariaTrack->reorderNoteOffVector();


        if (lastEventTick_inTrack > lastEventTick) lastEventTick = lastEventTick_inTrack;
    }//next track

    // erase empty tracks
    for (int n=0; n<sequence->getTrackAmount(); n++)
    {
        if (sequence->getTrack(n)->getNoteAmount() == 0)
        {
            sequence->deleteTrack(n);
            n=-1; // track order changed, start again
        }
    }

    // check if we're in one-track-one-channel mode. by the way, make sure all tracks with the
    // same channel use the same instrument
    const int trackAmount_inAriaSeq = sequence->getTrackAmount();
    bool one_track_one_channel = true;
    for (int n=0; n<trackAmount_inAriaSeq; n++)
    {
        for (int j=0; j<trackAmount_inAriaSeq; j++)
        {
            if (n != j and sequence->getTrack(n)->getChannel() == sequence->getTrack(j)->getChannel())
            {
                // 2 tracks share the same channel... we're not in "one-track-once-channel" mode
                one_track_one_channel = false;

                // check whether the 2 tracks have the same instrument.
                // if not, use the one that is not 1 (since 1 is default value)
                if (sequence->getTrack(n)->getChannel() != 9)
                {
                    // check that their instruments are different (ignore drum tracks)
                    if (sequence->getTrack(n)->getInstrument() != sequence->getTrack(j)->getInstrument() and sequence->getTrack(j)->getChannel() != 9)
                    {
                        // FIXME: why the "1" below???
                        if (sequence->getTrack(n)->getInstrument() == 1)
                        {
                            sequence->getTrack(n)->setInstrument( sequence->getTrack(j)->getInstrument() );
                        }
                        else if (sequence->getTrack(j)->getInstrument() == 1)
                        {
                            sequence->getTrack(j)->setInstrument( sequence->getTrack(n)->getInstrument() );
                        }
                        else
                        {
                            std::cerr << "multiple program changes are not supported by Aria, sorry." << std::endl;
                            sequence->getTrack(j)->setInstrument( sequence->getTrack(n)->getInstrument() ); // arbitrary
                        }
                    }
                }
                else
                {
                    // same but in case of drum track
                    if (sequence->getTrack(n)->getDrumKit() != sequence->getTrack(j)->getDrumKit() and
                        sequence->getTrack(j)->getChannel() == 9)
                    {
                        // FIXME: what is this "1" below???
                        if (sequence->getTrack(n)->getDrumKit() == 1)
                        {
                            sequence->getTrack(n)->setDrumKit( sequence->getTrack(j)->getDrumKit() );
                        }
                        else if (sequence->getTrack(j)->getDrumKit() == 1)
                        {
                            sequence->getTrack(j)->setDrumKit( sequence->getTrack(n)->getDrumKit() );
                        }
                        else
                        {
                            std::cerr << "multiple program changes (drums) are not supported by Aria, sorry." << std::endl;
                            sequence->getTrack(j)->setDrumKit( sequence->getTrack(n)->getDrumKit() ); // arbitrary
                        }
                    }
                }
            }
        }//next j
    }//next n

    if (one_track_one_channel) sequence->setChannelManagementType(CHANNEL_AUTO);

    MeasureData* md = sequence->getMeasureData();
    
    md->afterImporting();

    gseq->setXScrollInPixels(0);
    md->setFirstMeasure(0);

    // set song length
    int measureAmount_i = md->measureAtTick(lastEventTick) + 1;

    std::cout << "song length = " << measureAmount_i << " measures, last_event_tick="
              << lastEventTick << ", beat length = " << sequence->ticksPerBeat() << std::endl;

    if (measureAmount_i < 10) measureAmount_i=10;

    getMainFrame()->changeMeasureAmount( measureAmount_i );
    md->setMeasureAmount( measureAmount_i );

    // FIXME: this function shouldn't make GUI calls
    getMainFrame()->updateTopBarAndScrollbarsForSequence(gseq);
    gseq->setZoom(100);

    sequence->clearUndoStack();

    sequence->importing = false;

    getMainFrame()->updateMenuBarToSequence();
    
    // FIXME: ugly that you need to manually call 'updateMeasureInfo', it should update itself automatically when needed
    // (transaction system? you do 'startTransaction' to get the right to modify measure data, and at the end of the
    //  transaction the data is updated)
    if (not md->isMeasureLengthConstant())
    {
        md->updateMeasureInfo();
    }
    
    return true;
}

