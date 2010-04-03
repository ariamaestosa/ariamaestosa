#include "unit_test.h"
#include <iostream>
#include <stdio.h>
#include <vector>

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

void runTest(UnitTestCase* test)
{
    std::cout << "Running test case " << test->getName() << "... ";
    std::cout.flush();
    
    bool passed = true;
    
    try
    {
        test->run();
    }
    catch (const std::exception& ex)
    {
        passed = false;
        std::cout << "FAILED : " << ex.what() << std::endl;
    }
    
    if (passed) std::cout << "passed" << std::endl;    
}

void UnitTestCase::showMenu()
{
    std::cout << "==== UNIT TESTS ===\n";
    std::cout << "Make a choice : \n";
    std::cout << " (0) Run all\n";
    for (unsigned int n=0; n<TestCaseList::all_test_cases->size(); n++)
    {
        std::cout << " (" << (n+1) << ") " << (*TestCaseList::all_test_cases)[n]->m_name << "\n";
    }
    
    std::cout << "\n> ";
    fflush(stdout);
    
    int choice;
    std::cin >> choice;
    
    if (choice == 0)
    {
        for (unsigned int n=0; n<TestCaseList::all_test_cases->size(); n++)
        {
            runTest( (*TestCaseList::all_test_cases)[n] );
        }
    }
    else
    {
        runTest( (*TestCaseList::all_test_cases)[choice - 1] ); 
    }
}

