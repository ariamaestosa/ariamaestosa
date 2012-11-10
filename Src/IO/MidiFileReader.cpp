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
#include "GUI/GraphicalSequence.h"
#include "IO/MidiFileReader.h"
#include "IO/IOUtils.h"
#include "Midi/CommonMidiUtils.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "PreferencesData.h"

#include "jdksmidi/world.h"
#include "jdksmidi/track.h"
#include "jdksmidi/multitrack.h"
#include "jdksmidi/filereadmultitrack.h"
#include "jdksmidi/fileread.h"
#include "jdksmidi/fileshow.h"
#include "jdksmidi/filewritemultitrack.h"

#include <cmath>
#include <string>
#include <wx/intl.h>

bool AriaMaestosa::loadMidiFile(GraphicalSequence* gseq, wxString filepath, std::set<wxString>& warnings)
{
    Sequence* sequence = gseq->getModel();
    
    OwnerPtr<Sequence::Import> import(sequence->startImport());

    // the stream used to read the input file
    jdksmidi::MIDIFileReadStreamFile rs( filepath.mb_str() );

    // the object which will hold all the tracks
    jdksmidi::MIDIMultiTrack jdksequence;

    // the object which loads the tracks into the tracks object
    jdksmidi::MIDIFileReadMultiTrack track_loader( &jdksequence );

    // the object which parses the midifile and gives it to the multitrack loader
    jdksmidi::MIDIFileRead reader( &rs, &track_loader );

    // load the midifile into the multitrack object
    if (not reader.Parse())
    {
        std::cerr << "[MidiFileReader] ERROR: could not parse midi file" << std::endl;
        return false;
    }

    jdksmidi::MIDITrack* track;
    jdksmidi::MIDITimedBigMessage* event;

    sequence->setChannelManagementType(CHANNEL_MANUAL);
    
    int lastEventTick = 0; // last event tick for whole song, to find its duration

    // Flooding the console can slow down imports a lot so avoid printing the same error message repeatedly
    std::set<int> error_message_choker_note;
    std::set<int> error_message_choker_evt;
    bool lsb_message_printed = false;
    
    {
        ScopedMeasureITransaction tr(sequence->getMeasureData()->startImportTransaction());
        
        const int resolution = jdksequence.GetClksPerBeat();
        sequence->setTicksPerBeat( resolution );

        const int drum_note_duration = resolution/32+1;

        bool firstTempoEvent = true;

        const int trackAmount = jdksequence.GetNumTracks();

        // check for empty tracks
        int real_track_amount = 0;
        for (int trackID=0; trackID<trackAmount; trackID++)
        {
            if (jdksequence.GetTrack( trackID )->GetNumEvents() == 0){} // empty track...
            else real_track_amount++;
        }

        sequence->prepareEmptyTracksForLoading(real_track_amount /*16*/);
            
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
                if (channel != last_channel and last_channel != -1)
                {
                    if (event->IsNoteOn())
                    {
                        if (error_message_choker_note.find(channel*100 + last_channel) == error_message_choker_note.end())
                        {
                            error_message_choker_note.insert(channel*100 + last_channel);
                            fprintf(stderr, "[MidiFileReader] WARNING: note from channel %i != previous channel %i\n",
                                    channel, last_channel);
                            warnings.insert( _("This MIDI file has tracks that play on multiple MIDI channels. This is not supported by Aria Maestosa.") );
                        }
                        
                        //ariaTrack->setChannel(channel);
                        //last_channel = channel;
                    }
                    else if (not event->IsMetaEvent() and not event->IsAllNotesOff() and
                             not event->IsTextEvent() and not event->IsTempo() and
                             not event->IsSystemMessage() and not event->IsSystemExclusive())
                    {
                        if (error_message_choker_evt.find(channel*100 + last_channel) == error_message_choker_evt.end())
                        {
                            error_message_choker_evt.insert(channel*100 + last_channel);
                            fprintf(stderr, "[MidiFileReader] WARNING: event from channel %i != previous channel %i\n",
                                    channel, last_channel);
                            warnings.insert( _("This MIDI file has a track that sends events on multiple MIDI channels. This is not supported by Aria Maestosa.") );
                        }
                    }
                }

                if (last_channel == -1 and not event->IsMetaEvent() and
                   not event->IsTextEvent() and not event->IsTempo() and
                   not event->IsSystemMessage() and not event->IsSystemExclusive())
                {
                    ariaTrack->setChannel(channel); // its first iteration
                    last_channel = channel;
                }

                // ----------------------------------- note on -------------------------------------
                if (event->IsNoteOn() and event->GetVelocity() > 0)
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
                else if (event->IsNoteOff() or (event->IsNoteOn() and event->GetVelocity() == 0))
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
                    const int value = 127 - event->GetControllerValue();

                    if (controllerID > 31 and controllerID < 64)
                    {
                        // LSB... not supported by Aria ATM
                        if (not lsb_message_printed)
                        {
                            std::cerr << "[MidiFileReader] WARNING: This MIDI files contains LSB controller data."
                                      << " Aria does not support fine control changes and will discard this info."
                                      << std::endl;
                            lsb_message_printed = true;
                        }
                        continue;
                    }


                    if (controllerID == 3 or controllerID == 9 or controllerID == 14 or controllerID == 15 or
                        (controllerID > 19 and controllerID < 32) or (controllerID >= 85 and controllerID <= 87) or
                        controllerID == 89 or controllerID == 90 or (controllerID >= 102 and controllerID <= 119))
                    {
                        warnings.insert( wxString::Format(_("This MIDI file uses controller #%i, which is not part of the MIDI standard. This information will be discarded."), controllerID) );
                    }
                    else if (controllerID == 6 or
                             controllerID == 79 or
                             controllerID == 88 or
                             (controllerID > 95 and controllerID < 200 and controllerID != 127 /*stereo mode*/))
                    {
                        if (controllerID == 6 or controllerID == 38 or controllerID == 100 or controllerID == 101)
                        {
                            // TODO: add support for registered parameters http://www.midi.org/techspecs/midimessages.php#3
                            warnings.insert( _("This MIDI file uses Registered Parameters, which are currently not supported by Aria Maestosa.") );
                        }
                        else if (controllerID == 98 or controllerID == 99)
                        {
                            warnings.insert( _("This MIDI files uses NRPN (Non-Registered Parameters, i.e. non-standard controllers), which are currently not supported by Aria Maestosa.") );
                        }
                        else if (controllerID >= 120 and controllerID <= 127)
                        {
                            // TODO: 120: all sound off
                            //       121: reset controllers
                            //       122: local control on/off
                            //       123: all notes off
                            //       124: omni mode off (+ all notes off)
                            //       125: omni mode on (+ all notes on)
                            //       126: Mono Mode On
                            //       127: Poly Mode On (stereo)
                            warnings.insert( _("This MIDI files uses Channel Mode Message, which are currently not supported by Aria Maestosa.") );
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
                    float value = ControllerEvent::fromPitchBendValue(pitchBendVal);
                    //int value = (int)round( (pitchBendVal+8064.0)*128.0/16128.0 );
                    
                    if (value > 127) value = 127;
                    if (value < 0)   value = 0;

                    ariaTrack->addControlEvent_import(tick, value, PSEUDO_CONTROLLER_PITCH_BEND);
                    continue;
                }
                // ----------------------------------- program chnage -------------------------------------
                else if ( event->IsProgramChange() )
                {
                    const int instrument = event->GetPGValue();

                    programChanges++;
                    if (programChanges > 1)
                    {
                        int current = (channel == 9 ? ariaTrack->getDrumKit() : ariaTrack->getInstrument());
                        
                        // only warn if there are multiple _different_ instrument changes
                        if (instrument != current)
                        {
                            ariaTrack->addControlEvent_import(tick, event->GetPGValue(), PSEUDO_CONTROLLER_INSTRUMENT_CHANGE);
                            current = instrument;                    
                        }
                    }
                    else
                    {
                        if (channel == 9) 
                        {
                            ariaTrack->setDrumKit(instrument);
                            ariaTrack->setNotationType(DRUM, true);
                            ariaTrack->setNotationType(KEYBOARD, false);
                            ariaTrack->setNotationType(GUITAR, false);
                            ariaTrack->setNotationType(SCORE, false);
                        }
                        else
                        {
                            ariaTrack->setInstrument(instrument);
                        }
                    }

                    continue;
                }
                // ----------------------------------- tempo -------------------------------------
                else if ( event->IsTempo() )
                {

                    const float tempo = event->GetTempo32()/32.0f;

                    if (firstTempoEvent)
                    {
                        sequence->setTempo( (int)round(tempo) );
                        firstTempoEvent = false;
                        continue;
                    }
                    else
                    {
                        import->addTempoEvent(
                                              new ControllerEvent(PSEUDO_CONTROLLER_TEMPO,
                                                                  tick,
                                                                  convertBPMToTempoBend(tempo)
                                                                  )
                                              );
                        continue;
                    }
                    // ----------------------------------- time key/sig and beat marker -------------------------------------
                }
                else if ( event->IsTimeSig() )
                {
                    tr->addTimeSigChange( tick, (int)event->GetTimeSigNumerator(), (int)event->GetTimeSigDenominator() );
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
                            sequence->setDefaultKeySymbolAmount(amount);
                            sequence->setDefaultKeyType(KEY_TYPE_SHARPS);
                        }
                        else if (amount < 0)
                        {
                            sequence->getTrack(trackn)->setKey(-amount, KEY_TYPE_FLATS);
                            sequence->setDefaultKeySymbolAmount(-amount);
                            sequence->setDefaultKeyType(KEY_TYPE_FLATS);
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
                // ----------------------------------- track name / text events -------------------------------------
                else  if ( event->IsTextEvent() )
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
                    */
                    else if ((int)event->GetByte1() == 5) // lyrics
                    {
                        const char* text = (char*) event->GetSysEx()->GetBuf();
                        
                        if (strlen(text) > 0)
                        {
                            wxString s(text, wxConvUTF8, event->GetSysEx()->GetLength());
                            if (s.size() == 0)
                            {
                                fprintf(stderr, "[MidiFileReader] WARNING: error converting lyrics (wrong encoding?)\n");
                            }
                            else
                            {
                                sequence->addTextEvent_import(tick, s, PSEUDO_CONTROLLER_LYRICS);
                            }
                        }
                    }
                    /*
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
                ariaTrack->setNotationType(DRUM, true);
            }
            else
            {
                // set default editor
                switch (PreferencesData::getInstance()->getIntValue(SETTING_ID_DEFAULT_EDITOR))
                {
                    case 2:
                        ariaTrack->setNotationType(GUITAR, true);
                        break;
                    case 1:
                        ariaTrack->setNotationType(SCORE, true);
                        break;
                    case 0:
                    default:
                        ariaTrack->setNotationType(KEYBOARD, true);
                        break;
                }
            
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
                        if (sequence->getTrack(n)->getInstrument() != sequence->getTrack(j)->getInstrument() and
                            sequence->getTrack(j)->getChannel() != 9)
                        {
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
                        else
                        {
                            // force updating the label
                            sequence->getTrack(n)->setDrumKit( sequence->getTrack(n)->getDrumKit() );
                        }
                    }
                }
            }//next j
        }//next n

        if (one_track_one_channel) sequence->setChannelManagementType(CHANNEL_AUTO);
    } // end measure data transaction

    // set song length
    MeasureData* md = sequence->getMeasureData();
    int measureAmount_i = md->measureAtTick(lastEventTick);
    
    std::cout << "[loadMidiFile] song length = " << measureAmount_i << " measures, last_event_tick="
              << lastEventTick << ", beat length = " << sequence->ticksPerBeat() << std::endl;

    gseq->setZoom(100);

    if (measureAmount_i < 1) measureAmount_i = 1;

    {
        ScopedMeasureTransaction tr(md->startTransaction());
        tr->setMeasureAmount( measureAmount_i );
    }
    
    gseq->setZoom(100);

    sequence->clearUndoStack();

    return true;
}

