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

#include "Config.h"
#include <ptr_vector.h>

#ifdef _MORE_DEBUG_CHECKS

#include "ptr_vector.h"

#include "LeakCheck.h"
#include <iostream>

namespace AriaMaestosa
{
namespace MemoryLeaks
{

ptr_vector<MyObject> obj;

void addObj(MyObject* myObj)
{
    //std::cout << "addObj " << myObj->file << " (" << myObj->line << ")" << std::endl;
    obj.push_back(myObj);
}

void removeObj(MyObject* myObj)
{
    //std::cout << "removeObj " << myObj->file << " (" << myObj->line << ")" << std::endl;
    obj.remove(myObj);
    delete myObj;
    //std::cout << "removeObj done" << std::endl;
}

MyObject::MyObject(const char* f, int l)
{
        file = f;
        line = l;
}

void MyObject::print()
{
    std::cout << "Undeleted object " << file.c_str() << " (" << line << ")" << std::endl;
}

void checkForLeaks()
{

    std::cout << "checking for leaks... " << std::endl;

    if(obj.size()>0)
    {
        std::cout << "leaks detected!!" << std::endl;
        std::cout << "\n\n* * * * WARNING * * * * WARNING * * * * MEMORY LEAK! * * * *\n" << std::endl;
    }
    else
    {
        std::cout << "ok (no watched class left leaking)" << std::endl;
        return;
    }
    std::cout << "LEAK CHECK: " << obj.size() << " watched objects leaking" << std::endl;


    for(int n=0; n<obj.size(); n++)
    {
        obj[n].print();
    }
}


}
}
#endif
