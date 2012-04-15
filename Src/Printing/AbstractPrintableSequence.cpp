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

#include "Printing/AbstractPrintableSequence.h"

#include <wx/string.h>
#include <wx/intl.h>
#include <iostream>
#include "Midi/Sequence.h"

using namespace AriaMaestosa;

AbstractPrintableSequence::AbstractPrintableSequence(Sequence* parent)
{
    m_sequence = parent;
}

wxString AbstractPrintableSequence::getTitle(const Sequence* seq)
{
    wxString song_title = seq->suggestTitle();
    wxString track_title;
    
    wxString final_title;
    
    // give song title
    if (song_title.IsSameAs(_("Untitled"))) final_title = _("Aria Maestosa song");
    else                                    final_title = song_title;
    
    // give track name, if any
    if (not track_title.IsSameAs(_("Untitled")) and track_title.Length()>0)
    {
        final_title += (wxT(", ") + track_title);
    }
    
    //std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;
}
