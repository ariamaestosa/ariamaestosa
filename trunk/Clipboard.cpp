
#include "Clipboard.h"
#include "ptr_vector.h"

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
