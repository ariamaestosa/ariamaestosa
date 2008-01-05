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


#include "ptr_vector.h"

#include "LeakCheck.h"
#include <iostream>

namespace MemoryLeaks
{
	
	AriaMaestosa::ptr_vector<Object> obj;
	

void LeakCheck::setLocation(char* f, int i)
{
		myObj = new Object(f,i);
		obj.push_back( myObj );
}
	
Object::Object(char* f, int l)
{
	file = f;
	line = l;
}

void Object::print()
{
	std::cout << "Undeleted object " << file << " (" << line << ")" << std::endl;
}


void checkForLeaks()
{
	
	std::cout << "checking for leaks... ";
	
	if(obj.size()>0)
	{
		std::cout << "leaks detected!!" << std::endl;
		std::cout << "\n\n* * * * WARNING * * * * WARNING * * * * MEMORY LEAK! * * * *\n" << std::endl;
	}
	else
	{
		std::cout << "ok (no Aria class left leaking)" << std::endl;
		return;
	}
	std::cout << "LEAK CHECK: " << obj.size() << " watched objects leaking" << std::endl;
	
	
	for(int n=0; n<obj.size(); n++)
	{
		obj[n].print();	
	}
}



LeakCheck::LeakCheck()
{
}

LeakCheck::~LeakCheck()
{
	obj.remove( myObj );
}

}
