#ifndef _leak_check_
#define _leak_check_

#include "Config.h"
#include "ptr_vector.h"

#ifdef _MORE_DEBUG_CHECKS
namespace AriaMaestosa
{
namespace MemoryLeaks
{

class AbstractLeakCheck;
    
class MyObject
{
public:
    //std::string file;
    //int line;
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
