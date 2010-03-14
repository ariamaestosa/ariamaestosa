#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

#include <string>
#include <vector>
#include <stdexcept>

class UnitTestCase
{
    std::string m_name;
public:
    UnitTestCase(const char* name);
    ~UnitTestCase();
    virtual void run() = 0;
    
    const std::string& getName() const { return m_name; }
    static void showMenu();
};

#define UNIT_TEST( NAME ) class NAME : public UnitTestCase { public: NAME(const char* name) : UnitTestCase(name){} void run(); }; \
                          NAME unit_test_##NAME = NAME( #NAME ); void NAME::run()


#define require( CONDITION, MESSAGE ) if (!( CONDITION )) \
{ \
static char buffer[256];\
sprintf(buffer, "Requirement failed at %s:%i : %s", __FILE__, __LINE__, MESSAGE);\
throw std::logic_error(buffer);\
}

#endif
