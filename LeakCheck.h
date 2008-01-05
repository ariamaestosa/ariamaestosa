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


#ifndef _leak_check_
#define _leak_check_

namespace MemoryLeaks
{
	

class Object
{
public:
	char* file;
	int line;
	
	Object(char* f, int l);
	void print();
};

void checkForLeaks();
void setLocation(char* f, int i);

class LeakCheck
{
	Object* myObj;
public:
	void setLocation(char* file, int line);
	LeakCheck();	
	~LeakCheck();


};

class LeakSafe
{
	MemoryLeaks::LeakCheck myLeakCheck;
public:
	LeakSafe(char* file, int line)
	{
		myLeakCheck.setLocation(file,line);
	}
};

}

#ifdef _CHECK_FOR_LEAKS
#define DECLARE_LEAK_CHECK() MemoryLeaks::LeakCheck myLeakCheck
#define INIT_LEAK_CHECK() myLeakCheck.setLocation(__FILE__,__LINE__);

#define DECLARE_LEAK_SAFE public MemoryLeaks::LeakSafe
#define INIT_LEAK_SAFE MemoryLeaks::LeakSafe(__FILE__,__LINE__)
#endif

#endif
