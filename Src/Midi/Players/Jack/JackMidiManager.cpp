// by y.fujii <y-fujii at mimosa-pudica.net>, public domain

#if defined( USE_JACK )

#include <algorithm>
#include <memory>
#include <exception>
#include <cassert>
#include <stdint.h>
#include <pthread.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <wx/wx.h>
#include <jdkmidi/multitrack.h>
#include <jdkmidi/sequencer.h>
#include "Midi/CommonMidiUtils.h"
#include "Midi/Sequence.h"
#include "Midi/Players/PlatformMidiManager.h"


struct ScopedLocker
{
	~ScopedLocker()
	{
		pthread_mutex_unlock(m_mutex);
	}

	ScopedLocker(pthread_mutex_t* m): m_mutex(m)
	{
		if(pthread_mutex_lock(m_mutex) != 0)
			throw std::exception();
	}

	private:
		pthread_mutex_t* m_mutex;
};

struct JackMidiPlayer
{
	// note:
	//     0. do not use new, delete, file i/o syscalls, etc. while _mutex is
	//        being locked.
	//     1. be careful of priority inversion because handleJack() is running
	//        on the high priority thread.

	~JackMidiPlayer()
	{
		jack_client_close(m_jack);
		pthread_cond_destroy(&m_finish);
		pthread_mutex_destroy(&m_mutex);
		delete m_sequencer;
	}

	JackMidiPlayer(): m_playing(false), m_frame(0), m_sequencer(0)
	{
		pthread_mutexattr_t mattr;
		pthread_mutexattr_init(&mattr);
		pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
		if(pthread_mutex_init(&m_mutex, &mattr) != 0)
			throw std::exception();
		pthread_mutexattr_destroy(&mattr);
		try
		{
			if(pthread_cond_init(&m_finish, NULL) != 0)
				throw std::exception();
			try
			{
				m_jack = jack_client_open("aria_maestosa", JackNullOption, NULL);
				if(m_jack == 0)
					throw std::exception();
				try
				{
					jack_set_process_callback(m_jack, &handleJack, this);
					m_port = jack_port_register(
						m_jack, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0
					);
					if(m_port == 0)
						throw std::exception();
					if(jack_activate(m_jack) != 0)
						throw std::exception();
				}
				catch(...)
				{
					jack_client_close(m_jack);
					throw;
				}
			}
			catch(...)
			{
				pthread_cond_destroy(&m_finish);
				throw;
			}
		}
		catch(...)
		{
			pthread_mutex_destroy(&m_mutex);
			throw;
		}
	}

	void play(jdkmidi::MIDIMultiTrack* tracks, uint64_t frame = 0)
	{
		jdkmidi::MIDISequencer* tmp = new jdkmidi::MIDISequencer(tracks);
		{
			ScopedLocker lock(&m_mutex);
			std::swap(tmp, m_sequencer);
			m_frame = frame;
			m_playing = true;
		}
		delete tmp;
	}

	uint64_t stop()
	{
		ScopedLocker lock(&m_mutex);
		m_playing = false;
		return m_frame;
	}

	void wait()
	{
		ScopedLocker lock(&m_mutex);
		if(m_playing)
		{
			pthread_cond_wait(&m_finish, &m_mutex);
		}
	}

	bool isPlaying()
	{
		ScopedLocker lock(&m_mutex);
		return m_playing;
	}

	int getTick()
	{
		ScopedLocker lock(&m_mutex);
		assert(m_sequencer != 0);
		return m_sequencer->GetCurrentMIDIClockTime();
	}
	
	private:
		static int handleJack(jack_nframes_t nFrame, void* selfv)
		{
			JackMidiPlayer* self = reinterpret_cast<JackMidiPlayer*>(selfv);
			unsigned srate = jack_get_sample_rate(self->m_jack);
			void* buf = jack_port_get_buffer(self->m_port, nFrame);
			jack_midi_clear_buffer(buf);

			ScopedLocker lock(&self->m_mutex);
			if(self->m_playing)
			{
				// [bgn, end)
				double bgn = self->m_frame * (1000.0 / srate);
				double end = (self->m_frame + nFrame) * (1000.0 / srate);

				self->m_sequencer->GoToTimeMs(bgn);

				float t;
				while(self->m_sequencer->GetNextEventTimeMs(&t) && t < end)
				{
					int trackId;
					jdkmidi::MIDITimedBigMessage msg;
					self->m_sequencer->GetNextEvent(&trackId, &msg);

					if(!msg.IsMetaEvent())
					{
						unsigned l = msg.GetLength();
						assert(l < 4);
						uint8_t* ev = jack_midi_event_reserve(
							buf, int(t * (srate / 1000.0)) - self->m_frame, l
						);
						ev[0] = msg.GetStatus();
						if(l >= 2)
						{
							ev[1] = msg.GetByte1();
						}
						if(l >= 3)
						{
							ev[2] = msg.GetByte2();
						}
					}
				}
				self->m_frame += nFrame;

				if(!self->m_sequencer->GetNextEventTimeMs(&t))
				{
					self->m_playing = false;
					pthread_cond_signal(&self->m_finish);
				}
			}

			return 0;
		}

		jack_client_t* m_jack;
		jack_port_t* m_port;
		bool m_playing;
		uint64_t m_frame;
		jdkmidi::MIDISequencer* m_sequencer;
		pthread_mutex_t m_mutex;
		pthread_cond_t m_finish;
};


namespace AriaMaestosa { namespace PlatformMidiManager
{
	std::auto_ptr<JackMidiPlayer> player;
	std::auto_ptr<jdkmidi::MIDIMultiTrack> tracks;

	void initMidiPlayer()
	{
		player.reset(new JackMidiPlayer());
	}

	void freeMidiPlayer()
	{
		player.reset();
	}

	void resetSync()
	{
		jdkmidi::MIDIMultiTrack tracks(1);
		tracks.SetClksPerBeat(960);
		for(int ch = 0; ch < 16; ++ch)
		{
			jdkmidi::MIDITimedBigMessage msg;
			msg.SetTime(0);
			msg.SetAllNotesOff(ch);
			tracks.GetTrack(0)->PutEvent(msg);
		}

		player->play(&tracks);
		player->wait(); // finish playing before destroying tracks.
	}

	void playNote(int note, int vel, int dur, int ch, int inst)
	{
		resetSync();

		jdkmidi::MIDITimedBigMessage msg0, msg1, msg2;
		msg0.SetTime(0);
		msg0.SetProgramChange(ch, inst);
		msg1.SetTime(0); // XXX: some midi devices take time to change programs.
		msg1.SetNoteOn(ch, note, vel);
		msg2.SetTime(dur);
		msg2.SetNoteOff(ch, note, vel);

		tracks.reset(new jdkmidi::MIDIMultiTrack(1));
		tracks->SetClksPerBeat(960);
		tracks->GetTrack(0)->PutEvent(msg0);
		tracks->GetTrack(0)->PutEvent(msg1);
		tracks->GetTrack(0)->PutEvent(msg2);

		player->play(tracks.get());
	}

	void stopNote()
	{
		resetSync();
	}

	bool playSequence(Sequence* seq, int* startTick)
	{
		resetSync();

		int len = -1;
		int nTrack = -1;
		tracks.reset(new jdkmidi::MIDIMultiTrack());
		makeJDKMidiSequence(seq, *tracks, false, &len, startTick, &nTrack, true);
		player->play(tracks.get());

		return true;
	}

	bool playSelected(Sequence* seq, int* startTick)
	{
		resetSync();

		int len = -1;
		int nTrack = -1;
		tracks.reset(new jdkmidi::MIDIMultiTrack());
		makeJDKMidiSequence(seq, *tracks, true, &len, startTick, &nTrack, true);
		player->play(tracks.get());

		return true;
	}

	bool isPlaying()
	{
		return player->isPlaying();
	}

	void stop()
	{
		resetSync();
	}

	int trackPlaybackProgression()
	{
		return player->getTick();
	}

	wxString const getAudioExtension()
	{
		return wxEmptyString;
	}

	wxString const getAudioWildcard()
	{
		return wxEmptyString;
	}

	void exportAudioFile(Sequence*, wxString)
	{
		assert(false);
	}

	bool exportMidiFile(Sequence* seq, wxString file)
	{
		return AriaMaestosa::exportMidiFile(seq, file);
	}

	void seq_note_on(int, int, int) {}
	void seq_note_off(int, int) {}
	void seq_prog_change(int, int) {}
	void seq_controlchange(int, int, int) {}
	void seq_pitch_bend(int, int) {}
	void seq_notify_current_tick(int) {}
	bool seq_must_continue() { return false; }

} }


#endif // defined( USE_JACK )
