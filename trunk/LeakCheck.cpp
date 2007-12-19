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
