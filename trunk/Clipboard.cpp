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


#include "Clipboard.h"
#include "ptr_vector.h"
#include "Midi/Note.h"

namespace AriaMaestosa {
	
	namespace Clipboard {
		
		// a vector to store copied notes
		ptr_vector<Note> clipboard;
		
		void clear()
		{
			clipboard.clearAndDeleteAll();
		}
		
		void add(Note* n)
		{
			clipboard.push_back(n);	
		}
		
		int getSize()
		{
			return clipboard.size();	
		}
		
		Note* getNote( int index )
		{
			assertExpr(index, >=, 0);	
			assertExpr(index, <, clipboard.size());
			
			return &clipboard[index];
		}
		
	}
	
}
