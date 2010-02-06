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


#ifndef __LAYOUT_PAGE_H__
#define __LAYOUT_PAGE_H__

#include "ptr_vector.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"

namespace AriaMaestosa
{
    
    class LayoutPage
    {
    public:
        DECLARE_MAGIC_NUMBER();
        
        ptr_vector<LayoutLine> layoutLines;    
        
        LayoutPage()
        {
            INIT_MAGIC_NUMBER();
        }

        const int getLineCount() const
        {
            assert( MAGIC_NUMBER_OK() );
            return layoutLines.size();
        }
        
        LayoutLine& getLine(const int lineID)
        {
            assert( MAGIC_NUMBER_OK() );
            assert( MAGIC_NUMBER_OK_FOR(&layoutLines) );

            return layoutLines[lineID];
        }
    };
}
    
#endif
