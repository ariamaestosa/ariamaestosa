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

#include "IO/MidiToMemoryStream.h"
#include "IO/IOUtils.h"

#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "Midi/Track.h"
#include "Midi/Players/PlatformMidiManager.h"

#include "GUI/MeasureBar.h"

#include "jdkmidi/world.h"
#include "jdkmidi/track.h"
#include "jdkmidi/multitrack.h"
#include "jdkmidi/filereadmultitrack.h"
#include "jdkmidi/fileread.h"
#include "jdkmidi/fileshow.h"
#include "jdkmidi/filewritemultitrack.h"
#include "jdkmidi/msg.h"
#include "jdkmidi/sysex.h"

#include "jdkmidi/sequencer.h"
#include "jdkmidi/driver.h"
#include "jdkmidi/process.h"

#include <wx/timer.h>

//#include <sys/resource.h>
//#include <sys/time.h>
//#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>

#include <iostream>

namespace AriaMaestosa {

	/*
	 void	SetStatus( unsigned char s )	{ status=s;		}
	 void	SetChannel( unsigned char s )	{status=(unsigned char)((status&0xf0)|s);}
	 void	SetType( unsigned char s )		{status=(unsigned char)((status&0x0f)|s);}

	 void	SetByte1( unsigned char b )		{ byte1=b;		}
	 void	SetByte2( unsigned char b )		{ byte2=b;		}
	 void	SetByte3( unsigned char b )		{ byte3=b;		}

	 void	SetNote( unsigned char n ) 		{ byte1=n;		}
	 void	SetVelocity(unsigned char v) 	{ byte2=v;		}
	 void	SetPGValue(unsigned char v) 	{ byte1=v;		}
	 void	SetController(unsigned char c) 	{ byte1=c;		}
	 void	SetControllerValue(unsigned char v ) { byte2=v;		}

	 void	SetBenderValue( short v);
	 void	SetMetaType( unsigned char t ) ;
	 void	SetMetaValue( unsigned short v );
	 void	SetNoteOn( unsigned char chan, unsigned char note, unsigned char vel );
	 void	SetNoteOff( unsigned char chan, unsigned char note, unsigned char vel );
	 void	SetPolyPressure( unsigned char chan, unsigned char note, unsigned char pres );
	 void	SetControlChange( unsigned char chan, unsigned char ctrl, unsigned char val );
	 void	SetProgramChange( unsigned char chan, unsigned char val );
	 void	SetChannelPressure( unsigned char chan, unsigned char val );
	 void	SetPitchBend( unsigned char chan, short val );
	 void	SetPitchBend( unsigned char chan, unsigned char low, unsigned char high );
	 void	SetSysEx();
	 void	SetMTC( unsigned char field, unsigned char v );
	 void	SetSongPosition( short pos );
	 void	SetSongSelect(unsigned char sng);
	 void	SetTuneRequest();
	 void	SetMetaEvent( unsigned char type, unsigned char v1, unsigned char v2 );
	 void	SetMetaEvent( unsigned char type, unsigned short v );
	 void	SetAllNotesOff( unsigned char chan, unsigned char type=C_ALL_NOTES_OFF );
	 void	SetLocal( unsigned char chan, unsigned char v );
	 void	SetNoOp();
	 void	SetTempo32( unsigned short tempo_times_32 );
	 void	SetText( unsigned short text_num, unsigned char type=META_GENERIC_TEXT );
	 void	SetDataEnd();
	 void	SetTimeSig( unsigned char numerator, unsigned char denominator );
	 void   SetKeySig( signed char sharp_flats, unsigned char major_minor );
	 void	SetBeatMarker();


	 */

	/*
	 * Cretaes a JDK sequence and fills the basic skeleton of midi data. Actual notes and control changes are filled
	 * by Track::addMidiEvents, a method that is automatically called by this function.
	 */
bool exportMidiFile(Sequence* sequence, wxString filepath)
{
		// when we're saving, we always want song to start at first measure, so temporarly switch firstMeasure to 0, and set it back in the end
		const int firstMeasureValue=sequence->measureBar->getFirstMeasure();
		sequence->measureBar->setFirstMeasure(0);

		jdkmidi::MIDIMultiTrack tracks;
		int length = -1, start = -1, numTracks = -1;
		makeJDKMidiSequence(sequence, tracks, false, &length, &start, &numTracks, false);

        // FIXME - change conversion
		wxCSConv cs( wxFONTENCODING_UTF8 );
		wxCharBuffer output = cs.cWC2MB(filepath.wc_str());

		jdkmidi::MIDIFileWriteStreamFileName file_stream( (char*)output.data() /*toCString(filepath)*/ );

		jdkmidi::MIDIFileWriteMultiTrack writer2(
												 &tracks,
												 &file_stream
												 );

		// write the output file
		if( !writer2.Write( numTracks, sequence->ticksPerBeat() ) )
		{
			fprintf( stderr, "Error writing midi file\n");
			return false;
		}

		sequence->measureBar->setFirstMeasure(firstMeasureValue);

		return true;

}

void addTimeSigFromVector(int n, int amount, MeasureBar* measureBar, jdkmidi::MIDIMultiTrack& tracks, int substract_ticks)
{
	jdkmidi::MIDITimedBigMessage m;
	int tick_time = measureBar->getTimeSig(n).tick - substract_ticks < 0;
	if(tick_time)
	{
		if( (n+1<amount and measureBar->getTimeSig(n+1).tick > substract_ticks) or n+1==amount)
			tick_time = 0; // we need to consider event because it's the last and will affect future measures
		else
			return; // event does not affect anything not in played measures, ignore it

	}
	m.SetTime( measureBar->getTimeSig(n).tick - substract_ticks );

	float denom = (float)log(measureBar->getTimeSig(n).denom)/(float)log(2);
	m.SetTimeSig( measureBar->getTimeSig(n).num, (int)denom );

	if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl;  return; }
}

void addTempoEventFromSequenceVector(int n, int amount, Sequence* sequence, jdkmidi::MIDIMultiTrack& tracks, int substract_ticks)
{
	jdkmidi::MIDITimedBigMessage m;

	int tempo_time = sequence->tempoEvents[n].getTick() - substract_ticks;

	if(tempo_time < 0)
	{
		if( (n+1<amount and sequence->tempoEvents[n+1].getTick() > substract_ticks) or n+1==amount)
			tempo_time = 0; // we need to consider event because it's the last and will affect future measures
		else
			return; // event does not affect anything not in played measures, ignore it

	}

	m.SetTime( tempo_time );
	m.SetTempo32( convertTempoBendToBPM(sequence->tempoEvents[n].getValue() ) * 32
				  // tempo is stored as bpm * 32, giving 1/32 bpm resolution
				  );

	if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl;  return; }
}


bool makeJDKMidiSequence(Sequence* sequence, jdkmidi::MIDIMultiTrack& tracks, bool selectionOnly,
						 /*out*/int* songLengthInTicks, /*out*/int* startTick, /*out*/ int* numTracks, bool playing)
{

		int trackLength=-1;
		int channel=0;

		int substract_ticks;

        tracks.SetClksPerBeat( sequence->ticksPerBeat() );

		if(selectionOnly)
		{
			//  ---------------------------- add events to tracks --------------------
			trackLength = sequence->getCurrentTrack()->addMidiEvents( tracks.GetTrack(sequence->getCurrentTrackID()+1),
																	  channel,
																	  sequence->measureBar->getFirstMeasure(),
																	  true,
																	  *startTick );

			substract_ticks = *startTick;

			if(trackLength == -1) return -1; // nothing to play in track (empty track - play nothing)
			*songLengthInTicks=trackLength;

		}
		else
			// play from beginning
		{

			(*startTick) = -1;

			const int trackAmount = sequence->getTrackAmount();
			for(int n=0; n<trackAmount; n++)
			{

				int trackFirstNote=-1;
				trackLength = sequence->getTrack(n)->addMidiEvents( tracks.GetTrack(n+1), channel, sequence->measureBar->getFirstMeasure(), false, trackFirstNote );

				if( (trackFirstNote<(*startTick) and trackFirstNote != -1) or (*startTick)==-1) (*startTick) = trackFirstNote;


				if(trackLength == -1) continue; // nothing to play in track (empty track - skip it)
				if(trackLength > *songLengthInTicks) *songLengthInTicks=trackLength;

				channel++; if (channel==9) channel++;

			}
			substract_ticks = *startTick;

		}


		if(*songLengthInTicks < 1) return -1; // nothing to play at all (empty song - play nothing)
		*numTracks = sequence->getTrackAmount()+1;

		// --------------------------------- default tempo --------------------

		jdkmidi::MIDITimedBigMessage m;
		m.SetTime( 0 );
		m.SetTempo32( sequence->getTempo() * 32 ); // tempo stored as bpm * 32, giving 1/32 bpm resolution

		if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl;  return false; }

		if(!playing)
		{
			// --------------------------------- copyright and song name ---------------------------------
			// copyright
			if(sequence->getCopyright().size()>0)
			{

				jdkmidi::MIDITimedBigMessage m;

				m.SetText( 2 );
				m.SetByte1( 2 );

				const char* copyright = toCString(sequence->getCopyright());

				jdkmidi::MIDISystemExclusive sysex( (unsigned char*)copyright,
													sequence->getCopyright().size()+1,
													sequence->getCopyright().size()+1, false);

				m.CopySysEx( &sysex );

				m.SetTime( 0 );

				if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; return false; }


			}

			// song name
			if(sequence->getInternalName().size()>0)
			{

				jdkmidi::MIDITimedBigMessage m;
				m.SetText( 3 );
				m.SetByte1( 3 );

                const char* internalname = toCString(sequence->getInternalName());

				//std::cout << "name : " << internalname << " (" << sequence->getInternalName().size()+1 << ")" << std::endl;
				jdkmidi::MIDISystemExclusive sysex( (unsigned char*)internalname,
													sequence->getInternalName().size()+1,
													sequence->getInternalName().size()+1, false);

				m.CopySysEx( &sysex );
				m.SetTime( 0 );
				if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl; assert(0); }
			}

		}// end if not playing


		// --------------------------------- time sig and tempo --------------------
		// CoreAudio chokes on time sig changes... easy hack, just ignore them when playing back
		// it's also quicker because time sig events are not heard in any way so no reason to waste time adding them when playing
		// FIXME find real problem
		if(!playing)
		{
			if( getMeasureBar()->isMeasureLengthConstant() )
			{
				// time signature
				jdkmidi::MIDITimedBigMessage m;
				m.SetTime( 0 );

				float denom = (float)log(getMeasureBar()->getTimeSigDenominator())/(float)log(2);
				m.SetTimeSig( getMeasureBar()->getTimeSigNumerator(), (int)denom );

				if( !tracks.GetTrack(0)->PutEvent( m ) ) { std::cout << "Error adding event" << std::endl;  return false; }

				// add tempo changes straight away, there's nothing else in track 0 so time order is not a problem
				const int amount = sequence->tempoEvents.size();
				for(int n=0; n<amount; n++)
				{
					addTempoEventFromSequenceVector(n, amount, sequence, tracks, substract_ticks);
				}
			}
			else
			{
				// add both tempo changes and measures in time order
				MeasureBar* measureBar = getMeasureBar();
				const int timesig_amount = measureBar->getTimeSigAmount();
				const int tempo_amount = sequence->tempoEvents.size();
				int timesig_id = 0; // which time sig event should be the next added
				int tempo_id = 0; // which tempo event should be the next added

				// at each loop, check which event comes next, the time sig one or the tempo one.
				while(true)
				{
					const int timesig_tick = (timesig_id < timesig_amount ? measureBar->getTimeSig(timesig_id).tick : -1);
					const int tempo_tick = (tempo_id < tempo_amount ? sequence->tempoEvents[tempo_id].getTick() : -1);

					if(timesig_tick==-1 and tempo_tick==-1) break; // all events added, done
					else if(timesig_tick==-1)
					{
						addTempoEventFromSequenceVector(tempo_id, tempo_amount, sequence, tracks, substract_ticks);
						tempo_id++;
					}
					else if(tempo_tick==-1)
					{
						addTimeSigFromVector(timesig_id, timesig_amount, measureBar, tracks, substract_ticks);
						timesig_id++;
					}
					else if(tempo_tick > timesig_tick)
					{
						addTimeSigFromVector(timesig_id, timesig_amount, measureBar, tracks, substract_ticks);
						timesig_id++;
					}
					else
					{
						addTempoEventFromSequenceVector(tempo_id, tempo_amount, sequence, tracks, substract_ticks);
						tempo_id++;
					}
				}

			}
		}
		else
		{
			// add tempo changes straight away, there's nothing else in track 0 so time order is not a problem
			const int amount = sequence->tempoEvents.size();
			for(int n=0; n<amount; n++)
			{
				addTempoEventFromSequenceVector(n, amount, sequence, tracks, substract_ticks);
			}
		}



		 // ---------------------------- add dummy event after the actual end to ensure it doesn't stop playing too quickly --------------------------
		 // adds event way after actual stop point, to make sure song the midi player will reach the last actual note before stopping
		 // (e.g. i had issues with Quicktime stopping playback too soon and never actually reaching end of song event)

		{
			jdkmidi::MIDITimedBigMessage m;
			m.SetTime( *songLengthInTicks + sequence->ticksPerBeat()*3 );
			m.SetControlChange( 0,
								127,
								0 );

			for(int n=0; n<sequence->getTrackAmount(); n++)
			{
				if( !tracks.GetTrack(n+1)->PutEvent( m ) ) { std::cout << "Error adding midi event!" << std::endl; }
			}//next
		}

		return true;
}

void makeMidiBytes(Sequence* sequence, bool selectionOnly, /*out*/int* songlength, /*out*/int* startTick, /*out*/char** midiSongData, /*out*/int* datalength, bool playing)
{
	int numTracks = -1;

	jdkmidi::MIDIMultiTrack tracks;

	makeJDKMidiSequence(sequence, tracks, selectionOnly, songlength, startTick, &numTracks, playing);

	// create the output stream
	MidiToMemoryStream* out_stream=new MidiToMemoryStream();

	jdkmidi::MIDIFileWriteMultiTrack writer(
											&tracks,
											out_stream
											);

	// write the output data
	if( !writer.Write( numTracks, sequence->ticksPerBeat() ) )
	{
		fprintf( stderr, "Error writing midi file\n");
		return;
	}

	(*midiSongData) = (char*)malloc(out_stream->getDataLength());

	out_stream->storeMidiData( (*midiSongData) );

	*datalength = out_stream->getDataLength();

	delete out_stream;
}

int convertTempoBendToBPM(int val)
{
	return (int)( (127-val)*380.0/128.0 + 20);
}



// ------------------------------------------------------------
//     generic portable sequencer/timer implementation
// ------------------------------------------------------------

#pragma mark -

class FtimeTimer
{
    long initial_sec, initial_millis;
    public:

    void reset_and_start()
    {
        // FIXME - ftime is apparently obsolete (http://linux.die.net/man/3/ftime)
        // use http://linux.die.net/man/2/gettimeofday instead

        timeb tb;
        ftime(&tb);
        initial_sec= tb.time;
        initial_millis = tb.millitm;
        //std::cout << "time = " << tb.time << " seconds " << tb.millitm << " millis" << std::endl;
    }

    int get_elapsed_millis()
    {
        timeb tb;
        ftime(&tb);

        const long current_sec = tb.time;
        const long current_millis = tb.millitm;

        const long delta_sec = current_sec - initial_sec;

        const long total_millis = delta_sec*1000 - initial_millis + current_millis;
        return total_millis;

    }
};

class DummyTimer
{
    long time;
    public:
    void reset_and_start(){ time=0; }
    int get_elapsed_millis(){ time+=13; return time; }
};


typedef FtimeTimer BasicTimer;
//typedef DummyTimer BasicTimer;

AriaSequenceTimer::AriaSequenceTimer(Sequence* seq)
{
    AriaSequenceTimer::seq = seq;
}

//jdkmidi::MIDIMultiTrack* jdkmidiseq = NULL;
//jdkmidi::MIDISequencer* jdksequencer = NULL;
BasicTimer* timer = NULL;

void cleanup_sequencer()
{
    //if(jdksequencer != NULL) delete jdksequencer;
    //if(jdkmidiseq != NULL) delete jdkmidiseq;
    if(timer != NULL) delete timer;

    //jdksequencer = NULL;
    //jdkmidiseq = NULL;
    timer = NULL;
}

void AriaSequenceTimer::run(jdkmidi::MIDISequencer* jdksequencer, const int songLengthInTicks)
{

    std::cout << "  * AriaSequenceTimer::run" << std::endl;

    std::cout << "trying to play " << seq->suggestFileName().mb_str() << std::endl;

    jdksequencer->GoToTimeMs( 0 );

    int bpm = seq->getTempo();
    const int beatlen = seq->ticksPerBeat();

    double ticks_per_millis = (double)bpm * (double)beatlen / (double)60000.0;

    std::cout << "bpm = " << bpm << " beatlen=" << beatlen << " ticks_per_millis=" << ticks_per_millis << std::endl;

    float next_event_time = 0;

    jdkmidi::MIDITimedBigMessage ev;
    int ev_track;

    jdkmidi::MIDIClockTime tick;
    if(!jdksequencer->GetNextEventTime(&tick))
    {
        std::cout << "failed to get first event time, returning" << std::endl;
        cleanup_sequencer();
        return;
    }

    //jdksequencer->ResetAllTracks();

    long previous_tick = tick;

    next_event_time = tick / ticks_per_millis;

    timer = new BasicTimer();
    timer->reset_and_start();

    long total_millis = 0;
    long last_millis = 0;


    while(PlatformMidiManager::seq_must_continue())
    {
        // process all events that need to be done by the current tick
        while( next_event_time <= total_millis )
        {
            if(!jdksequencer->GetNextEvent( &ev_track, &ev ))
            {
                // error
                std::cerr << "error, failed to retrieve next event, returning" << std::endl;
                cleanup_sequencer();
                return;
            }
            const int channel = ev.GetChannel();

            if( ev.IsNoteOn() )
			{
                const int note = ev.GetNote();
                const int volume = ev.GetVelocity();
                PlatformMidiManager::seq_note_on(note, volume, channel);
            }
			else if( ev.IsNoteOff() )
			{
                const int note = ev.GetNote();
                PlatformMidiManager::seq_note_off(note, channel);
			}
			else if( ev.IsControlChange() )
			{
                const int controllerID = ev.GetController();
                const int value = ev.GetControllerValue();
                PlatformMidiManager::seq_controlchange(controllerID, value, channel);
			}
			else if( ev.IsPitchBend() )
			{
                const int pitchBendVal = ev.GetBenderValue();
                PlatformMidiManager::seq_pitch_bend(pitchBendVal, channel);
			}
			else if( ev.IsProgramChange() )
			{
                const int instrument = ev.GetPGValue();
                PlatformMidiManager::seq_prog_change(instrument, channel);
            }
            else if( ev.IsTempo() )
			{
                const int bpm = ev.GetTempo32()/32;
                ticks_per_millis = (double)bpm * (double)beatlen / (double)60000.0;
			}
			/*
            else if( ev.IsPolyPressure() )
                std::cout << "poly pressure" << std::endl;
            else if( ev.IsChannelPressure() )
                std::cout << "channel pressure" << std::endl;
            else if( ev.IsSystemMessage() )
                std::cout << "system message " << std::hex << ev.GetType() << std::endl;

            if( ev.IsSysEx() )
                std::cout << "sys ex " << ev.GetSysExNum() << std::endl;
            else if( ev.IsSongPosition() )
                std::cout << "song position" << std::endl;
            else if( ev.IsNoOp() )
                std::cout << "no op" << std::endl;
            else if( ev.IsBeatMarker() )
                std::cout << "beat marker" << std::endl;
            else
            {
                std::cout << "unknown event : " << ev.GetType() << std::endl;
            }
            */

            previous_tick = tick;

            if(!jdksequencer->GetNextEventTime(&tick))
            {
                cleanup_sequencer();
                return;
            }


            if(tick > songLengthInTicks)
            {
                std::cout << "done, thread will exit" << std::endl;
                PlatformMidiManager::seq_notify_current_tick(-1);
                cleanup_sequencer();
                return;
            }

            PlatformMidiManager::seq_notify_current_tick(previous_tick);

            next_event_time += (tick - previous_tick) / ticks_per_millis;

            /*
            static int i = 0;
            i++; if(i>10) i=0;
            if(i == 1)
            {
                std::cout << "next_event_time = " << next_event_time << "  -> tick = " << tick << std::endl;
            }
            */

        }

        wxThread::Sleep(10);

/*
        {
        //jdkmidi::MIDIClockTime tick;
        //jdksequencer->GetCurrentMIDIClockTime();
        //std::cout << "current tick got from sequencer : " << tick << std::endl;
        //PlatformMidiManager::seq_notify_current_tick(tick);

        std::cout << "current beat got from sequencer : " << jdksequencer->GetCurrentBeat() << std::endl;
        }
*/
        last_millis = total_millis;
        const int delta = (timer->get_elapsed_millis() - last_millis);

        total_millis += delta;


        // http://www.meangene.com/notes/time.html
        // http://ftp.traduc.org/doc-vf/gazette-linux/html/2004/103/lg103-G.html
    }

    cleanup_sequencer();
}



}
