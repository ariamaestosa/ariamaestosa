#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__

#include <string>
#include <stdexcept>
#include <sstream>

class UnitTestCase
{
    std::string m_name;
public:
    UnitTestCase(const char* name, const char* filePath);
    virtual ~UnitTestCase();
    virtual void run() = 0;
    
    const std::string& getName() const { return m_name; }
    static void showMenu();
};

#ifdef _MORE_DEBUG_CHECKS
#define UNIT_TEST( NAME ) class NAME : public UnitTestCase { public: NAME(const char* name, const char* filename) : UnitTestCase(name, filename){} void run(); }; \
                          static NAME unit_test_##NAME = NAME( #NAME, __FILE__ ); void NAME::run()
#else
// silly trick to not bloat the executable with unit test code in release mode; since the template is
// never instantiated the code will be discarded
#define UNIT_TEST( NAME ) template<typename T> void NAME()
#endif

#define require( CONDITION, MESSAGE ) if (!( CONDITION ))                          \
{                                                                                  \
char buffer[512];                                                                  \
sprintf(buffer, "Requirement failed at %s:%i : %s", __FILE__, __LINE__, MESSAGE);  \
throw std::logic_error(buffer);                                                    \
}

#define require_e( LEFT, OP, RIGHT, MESSAGE ) if (!( (LEFT) OP (RIGHT) ))          \
{                                                                                  \
char buffer[512];                                                                  \
std::ostringstream values;                                                         \
values << LEFT << " " << #OP << " " << RIGHT;                                      \
sprintf(buffer, "Requirement (%s %s %s) failed at %s:%i : %s\nwith values : %s",   \
        #LEFT, #OP, #RIGHT, __FILE__, __LINE__, MESSAGE, values.str().c_str());    \
throw std::logic_error(buffer);                                                    \
}

// pseudo-keyword :)
#define utest class

#endif
