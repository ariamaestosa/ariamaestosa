#include "unit_test.h"
#include <iostream>

namespace TestCaseList
{
    std::vector<UnitTestCase*>* all_test_cases = NULL;
}

UnitTestCase::UnitTestCase(const char* name)
{
    if (TestCaseList::all_test_cases == NULL) TestCaseList::all_test_cases = new std::vector<UnitTestCase*>();
    
    m_name = name;
    TestCaseList::all_test_cases->push_back(this);
}

UnitTestCase::~UnitTestCase()
{
    for (unsigned int n=0; n<TestCaseList::all_test_cases->size(); n++)
    {
        if ((*TestCaseList::all_test_cases)[n] == this)
        {
            TestCaseList::all_test_cases->erase( TestCaseList::all_test_cases->begin() + n );
            break;
        }
    }
    
    if (TestCaseList::all_test_cases->size() == 0) delete TestCaseList::all_test_cases;
}

void UnitTestCase::runAll()
{
    std::cout << "=====================" << std::endl;
    std::cout << "Running " << TestCaseList::all_test_cases->size() << " Unit Tests" << std::endl;
    std::cout << "=====================" << std::endl;

    for (unsigned int n=0; n<TestCaseList::all_test_cases->size(); n++)
    {
        std::cout << "Running test case " << (*TestCaseList::all_test_cases)[n]->m_name << "... ";
        std::cout.flush();
        
        bool passed = true;
        
        try
        {
            (*TestCaseList::all_test_cases)[n]->run();
        }
        catch (const std::exception& ex)
        {
            passed = false;
            std::cout << "FAILED : " << ex.what() << std::endl;
        }
        
        if (passed) std::cout << "passed" << std::endl;
    }
}

