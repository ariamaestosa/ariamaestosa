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


#ifndef _drumchoice_
#define _drumchoice_

#include "wx/wx.h"

#include "Utils.h"
#include "irrXML/irrXML.h"
#include "Renderers/RenderAPI.h"

namespace AriaMaestosa
{
    
    class Track;
    
    class DrumChoice : public wxMenu
    {
        Track* parent;
        
        AriaRenderArray drumkit_names_renderer;
        
    public:
        LEAK_CHECK();
        
        DrumChoice();
        ~DrumChoice();
        
        void setParent(Track* t);
        
        const wxString& getDrumName(int id) const;
        void renderDrumKitName(const int drumID, const int x, const int y);
        
        void menuSelected(wxCommandEvent& evt);
        
        DECLARE_EVENT_TABLE();
    };
    
}

#endif
