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
#include "wx/wx.h"

namespace AriaMaestosa
{
    
class AriaPrintable
{
public:
    AriaPrintable();
    virtual ~AriaPrintable();
    virtual wxString getTitle();
    virtual int getPageAmount();
    virtual void printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h);
    virtual bool portraitOrientation();
};

bool printResult(AriaPrintable* printable);
    
}

#endif
