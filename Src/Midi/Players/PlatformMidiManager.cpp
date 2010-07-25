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
#include "ptr_vector.h"
#include "wx/intl.h"
#include "wx/msgdlg.h"

using namespace AriaMaestosa;

ptr_vector<PlatformMidiManagerFactory, REF>* g_all_midi_managers = NULL;
PlatformMidiManager* g_manager = NULL;

std::vector<wxString> PlatformMidiManager::getChoices()
{
    std::vector<wxString> out;
    const int amount = g_all_midi_managers->size();
    for (int n=0; n<amount; n++)
    {
        out.push_back(g_all_midi_managers->get(n)->getName());
    }
    return out;
}

PlatformMidiManager* PlatformMidiManager::get()
{
    if (g_all_midi_managers == NULL)
    {
        // FIXME: instead of aborting, fire a no-op driver
        wxMessageBox(_("Bad binary, no MIDI driver was compiled in!"));
        exit(1);
    }
    
    if (g_manager == NULL)
    {
        // TODO : pick a midi manager according to user choice
        g_manager = g_all_midi_managers->get(0)->newInstance();
    }
    return g_manager;
}

void PlatformMidiManager::registerManager(PlatformMidiManagerFactory* newManager)
{
    // hack to make sure the vector is always created before using it
    static bool firstTime = true;
    if (firstTime)
    {
        g_all_midi_managers = new ptr_vector<PlatformMidiManagerFactory, REF>();
        firstTime = false;
    }
    
    g_all_midi_managers->push_back(newManager);
}
