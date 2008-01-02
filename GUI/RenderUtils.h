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

#ifndef _renderutils_
#define _renderutils_

namespace AriaMaestosa
{
namespace AriaRender
{
    // enter mode
    void primitives();
    void images();

    void color(const float r, const float g, const float b);
    void color(const float r, const float g, const float b, const float a);
    
    void line(const int x1, const int y1, const int x2, const int y2);
    void lineWidth(const int n);
    void lineSmooth(const bool enabled);
    
    void point(const int x, const int y);
    void pointSize(const int n);
    
    void rect(const int x1, const int y1, const int x2, const int y2);
    void hollow_rect(const int x1, const int y1, const int x2, const int y2);
    void bordered_rect(const int x1, const int y1, const int x2, const int y2);
    void bordered_rect_no_start(const int x1, const int y1, const int x2, const int y2);
    
}
}
#endif
