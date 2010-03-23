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
        ptr_vector<LayoutLine> m_layout_lines;    

    public:
        DECLARE_MAGIC_NUMBER();
        
        
        LayoutPage()
        {
            ;
        }

        const int getLineCount() const
        {
            assert( MAGIC_NUMBER_OK() );

            return m_layout_lines.size();
        }
        
        LayoutLine& getLine(const int lineID)
        {
            assert( MAGIC_NUMBER_OK() );
            assert( MAGIC_NUMBER_OK_FOR(&m_layout_lines) );

            return m_layout_lines[lineID];
        }
        
        void addLine( LayoutLine* line )
        {
            assert( MAGIC_NUMBER_OK() );
            assert( MAGIC_NUMBER_OK_FOR(&m_layout_lines) );
            
            m_layout_lines.push_back(line);
        }
        
        void moveYourLastLineTo( LayoutPage& otherPage )
        {
            assert( &otherPage != this );
            
            LayoutLine* lastLine = m_layout_lines.get(m_layout_lines.size()-1);
            otherPage.addLine( lastLine );
            m_layout_lines.remove(m_layout_lines.size()-1);
        }

    };
}
    
#endif
