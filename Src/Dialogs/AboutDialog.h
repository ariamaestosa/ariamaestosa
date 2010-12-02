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



#ifndef __ABOUT_DIALOG_H__
#define __ABOUT_DIALOG_H__

/**
  * @defgroup dialogs
  * @brief This module contains all dialogs/frames that can appear over the main frame.
  */

#include <wx/frame.h>
#include "Utils.h"


namespace AriaMaestosa
{
    
    /**
      * @ingroup dialogs
      */
    class AboutDialog : public wxFrame
    {
        
        int dialogID;
        
    public:
        LEAK_CHECK();
        
        AboutDialog();
        void show();
        
        void onClose(wxCloseEvent& evt);
        void onCloseMenu(wxCommandEvent& evt);
    };
    
}
#endif
