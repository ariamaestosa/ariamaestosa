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

class wxString;

namespace AriaMaestosa
{
namespace AriaRender
{
    
    // enter mode
    void primitives();
    void images();

    void beginScissors(const int x, const int y, const int width, const int height);
    void endScissors();
    
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
    
    void text(const char* string, const int x, const int y);
    void text(wxString* string, const int x, const int y);
    int text_return_end_x(wxString* string, const int x, const int y);
    void small_text(const char* string, const int x, const int y);
    void small_text_newline_between_words(const char* string, const int x, const int y);
    void character(const char character, const int x, const int y);
    void small_character(const char character, const int x, const int y);
    void text_with_bounds(wxString* string, const int x, const int y, const int max_x);
        
    void triangle(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3);
    
    void arc(int center_x, int center_y, int radius_x, int radius_y, bool show_above);
    
    void quad(const int x1, const int y1,
              const int x2, const int y2,
              const int x3, const int y3,
              const int x4, const int y4);
}
}
#endif
