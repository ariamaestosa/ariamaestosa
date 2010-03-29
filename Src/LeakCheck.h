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


#ifndef __LEAK_CHECK_H__
#define __LEAK_CHECK_H__

#ifdef _MORE_DEBUG_CHECKS
namespace AriaMaestosa
{
    namespace MemoryLeaks
    {
        
        class AbstractLeakCheck;
        
        class MyObject
        {
            char** stack;
            int stackSize;
        public:
            AbstractLeakCheck* obj;
            
            MyObject(AbstractLeakCheck* obj);
            void print();
        };
        
        void checkForLeaks();
        
        void addObj(MyObject* myObj);
        void removeObj(MyObject* myObj);
        
        class AbstractLeakCheck
        {
            MyObject* myObj;
        public:
            AbstractLeakCheck()
            {
                myObj = new MyObject( this );
                addObj( myObj );
            }
            
            AbstractLeakCheck(const AbstractLeakCheck &t)
            {
                myObj = new MyObject( this );
                addObj( myObj );
            }
            
            virtual ~AbstractLeakCheck()
            {
                removeObj( myObj );
            }
            
            virtual void print() const
            {
            }
        };
        
    }
}

#define LEAK_CHECK() \
class LeakCheck : public MemoryLeaks::AbstractLeakCheck\
{  public:\
virtual void print() const\
{ \
printf("Undeleted object at %s : %i\n",  __FILE__, __LINE__);\
} \
virtual ~LeakCheck() {} \
}; \
LeakCheck leack_check_instance;

#else
#define LEAK_CHECK()
#endif

#endif
