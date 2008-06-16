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


#ifndef _print_base_h_
#define _print_base_h_

#include <vector>
#include "Printing/PrintLayout.h"
#include "wx/wx.h"

namespace AriaMaestosa
{
    class Track;
    
class AriaPrintable
{
protected:
    Track* parent;
    std::vector<LayoutPage> layoutPages;
    ptr_vector<MeasureToExport> measures;
    
    int text_height;
    int text_height_half;
public:
    AriaPrintable(Track* parent, bool checkRepetitions_bool);
    virtual ~AriaPrintable();
    wxString getTitle();
    int getPageAmount();
    void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h);

    bool portraitOrientation();
    
    virtual void drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1);
};

bool printResult(AriaPrintable* printable);
    
}

#endif
