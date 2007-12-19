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

/*
 * I made this class to work like a regular vector, except that contentsVector are placed one the heap so third-party contentsVector can keep pointers to them.
 */

#ifndef _ptr_vector_
#define _ptr_vector_

#include <vector>
#include <iostream>

#include "Config.h"
#include "wx/wx.h"

namespace AriaMaestosa
{

template<typename TYPE>
class ptr_vector
{
    
public:
    std::vector<long> contentsVector;
	
ptr_vector()
{
}

~ptr_vector()
{
	clearAndDeleteAll();		
}

void push_back(TYPE* t)
{
        long memloc = (long) t;
        contentsVector.push_back(memloc);
}

void add(TYPE* t, int index)
{
	long memloc = (long) t;
	contentsVector.insert(contentsVector.begin()+index, memloc);
}

void swap(int ID1, int ID2)
{
	assertExpr(ID1,>,-1);
	assertExpr((unsigned int)ID1,<,contentsVector.size());
	assertExpr(ID2,>,-1);
	assertExpr((unsigned int)ID2,<,contentsVector.size());
	
	
	long temp = contentsVector[ID2];
	
	contentsVector[ID2] = contentsVector[ID1];
	contentsVector[ID1] = temp;
}

// mark is a way to delete an object without changing the order of the element sin the vector
// it can be useful in a 'for' loop when the loop relies on object IDs and vector size not changing

void markToBeDeleted(const int ID) // object is removed from vector and deleted
{
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	delete ( TYPE *) contentsVector[ID];
	
	contentsVector[ID] = 0;
	
}
void markToBeRemoved(const int ID) // object is removed from vector but not deleted
{
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	contentsVector[ID] = 0;
	
}

bool isMarked(const int ID)
{
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	return (contentsVector[ID] == 0);
}

void removeMarked()
{
	int size = contentsVector.size();
	for(int n=0; n<size; n++)
	{
		
		if( contentsVector[n] == 0 )
		{
			contentsVector.erase(contentsVector.begin()+n);
			size = contentsVector.size();
			n -= 2;
			if(n < -1) n=-1;
		}
	}//next
	
}

TYPE* get(const int ID)
{
	
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	return (TYPE*) contentsVector[ID];
}

int size()
{
	return contentsVector.size();
}

void erase(const int ID)
{
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	delete ( TYPE *) contentsVector[ID];
	
	contentsVector.erase(contentsVector.begin()+ID);
}

void remove(const int ID)
{
	assertExpr(ID,>,-1);
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	contentsVector.erase(contentsVector.begin()+ID);
}

void clearAndDeleteAll()
{
	for(unsigned int n=0; n<contentsVector.size(); n++)
	{
		
		TYPE * pointer=( TYPE *) contentsVector[n];
		delete pointer;
	}
	contentsVector.clear();
}

TYPE& operator[](const unsigned int ID)
{
	
	assertExpr((unsigned int)ID,<,contentsVector.size());
	
	return *((TYPE*) contentsVector[ID]);
}

void clearWithoutDeleting()
{
	contentsVector.clear();
}

void remove(TYPE* obj)
{
	//assertExpr(ID,>,-1);
	//assertExpr((unsigned int)ID,<,contentsVector.size());
	
	
	for(unsigned int n=0; n<contentsVector.size(); n++)
	{
		
		TYPE * pointer=( TYPE *) contentsVector[n];
		if(pointer == obj)
		{
			contentsVector.erase(contentsVector.begin()+n);
			return;
		}
	}
	
	
}

};

}

#endif
