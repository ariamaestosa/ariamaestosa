#ifndef _leak_check_
#define _leak_check_

#include "Config.h"
#include "ptr_vector.h"

#ifdef _MORE_DEBUG_CHECKS
namespace AriaMaestosa
{
namespace MemoryLeaks
{

class MyObject
{
public:
    char* file;
    int line;

    MyObject(char* f, int l);
    void print();
};

void checkForLeaks();

void addObj(MyObject* myObj);
void removeObj(MyObject* myObj);

template<typename P>
class TemplateLeakCheck
{
    MyObject* myObj;
public:
    TemplateLeakCheck()
    {
        myObj = new MyObject(P :: memCheckGetFile(), P :: memCheckGetLine());
        addObj( myObj );
    }
    TemplateLeakCheck(const TemplateLeakCheck &t)
    {
        myObj = new MyObject(P :: memCheckGetFile(), P :: memCheckGetLine());
        addObj( myObj );
    }

    ~TemplateLeakCheck()
    {
        removeObj( myObj );
    }
};

}
}

#define LEAK_CHECK( classname ) static char* memCheckGetFile() { return __FILE__; } static int memCheckGetLine() { return __LINE__; } const MemoryLeaks::TemplateLeakCheck< classname > myLeakCheck;

#else
#define LEAK_CHECK( classname )
#endif

#endif
