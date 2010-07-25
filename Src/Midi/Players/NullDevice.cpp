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
#include "Midi/Players/PlatformMidiManager.h"

namespace AriaMaestosa
{
    
    class NullMidiManager : public PlatformMidiManager
    {
    public:
        virtual ~NullMidiManager() { }
        
        virtual bool playSequence(Sequence* sequence, /*out*/int* startTick) { return false; }
        
        virtual bool playSelected(Sequence* sequence, /*out*/int* startTick) { return false; }
        
        virtual bool isPlaying() { return false; }
        
        virtual void stop() { }
        
        virtual void exportAudioFile(Sequence* sequence, wxString filepath) { }
        
        virtual bool exportMidiFile(Sequence* sequence, wxString filepath) { return false; }
        

        virtual int trackPlaybackProgression() { return 0; }
        
        virtual void initMidiPlayer() { }
        
        virtual void freeMidiPlayer() { }
        

        virtual void playNote(int noteNum, int volume, int duration, int channel, int instrument){}

        virtual void stopNote() {}
        

        virtual const wxString getAudioExtension()
        {
            return wxEmptyString;
        }
        
        virtual const wxString getAudioWildcard()
        {
            return wxEmptyString;
        }
        
    };
    
    class NullMidiManagerFactory : public PlatformMidiManagerFactory
    {
    public:
        NullMidiManagerFactory() : PlatformMidiManagerFactory(wxT("No Sound"))
        {
        }
        virtual ~NullMidiManagerFactory()
        {
        }
        
        virtual PlatformMidiManager* newInstance()
        {
            return new NullMidiManager();
        }
    };
    NullMidiManagerFactory g_null_midi_manager_factory;
    
};
