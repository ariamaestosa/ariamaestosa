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

#ifndef __CUSTOM_KEY_H__
#define __CUSTOM_KEY_H__

#include "wx/dialog.h"
#include "Midi/Track.h"

class wxCheckBox;
class wxNotebook;
class wxCommandEvent;
class wxWindow;

namespace AriaMaestosa
{
    class GraphicalTrack;
    
    /**
     * @ingroup dialogs
     * @brief the dialog that lets the user setup a custom key
     */
    class CustomKeyDialog : public wxDialog
    {
        wxCheckBox* m_check_boxes[132];
        wxCheckBox* m_check_boxes_one_octave[12];
        
        GraphicalTrack* m_parent;
        
        int m_page1_id;
        int m_page2_id;
        
        wxNotebook* m_notebook;
        
    public:
        
        CustomKeyDialog(wxWindow* parent, GraphicalTrack* gtrack);
        
        
        void buildKey(KeyInclusionType customKey[131]);
        
        void onOK(wxCommandEvent& evt);
        void onCancel(wxCommandEvent& evt);
        void saveAsPreset(wxCommandEvent& evt);
        
        
        DECLARE_EVENT_TABLE()
    };
    
}


#endif

