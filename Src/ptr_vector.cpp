
#include "ptr_vector.h"
#include "UnitTest.h"

namespace AriaMaestosa
{
        
    UNIT_TEST( VectorSortTest )
    {
        SortableVector<int> s;
        for (int n=0; n<100; n++) s.push_back(n);
        
        s.insertionSort();
        
        for (int n=0; n<100; n++) require_e(s[n], ==, n, "Vector sorted correctly");
    }
        
    UNIT_TEST( VectorSortTest2 )
    {
        SortableVector<int> s;
        for (int n=0; n<100; n++) s.push_back(99-n);
        
        s.insertionSort();
                
        for (int n=0; n<100; n++) require_e(s[n], ==, n, "Vector sorted correctly");
    }
    
    UNIT_TEST( VectorSortTest3 )
    {
        SortableVector<int> s;
        s.push_back(1);
        s.push_back(99);
        s.push_back(2);
        s.push_back(98);
        s.push_back(3);
        s.push_back(97);
        s.push_back(4);
        s.push_back(96);
        s.push_back(4);
        s.push_back(4);
        s.push_back(96);

        s.insertionSort();
        
        require_e(s[0],  ==, 1,  "Vector sorted correctly");
        require_e(s[1],  ==, 2,  "Vector sorted correctly");
        require_e(s[2],  ==, 3,  "Vector sorted correctly");
        require_e(s[3],  ==, 4,  "Vector sorted correctly");
        require_e(s[4],  ==, 4,  "Vector sorted correctly");
        require_e(s[5],  ==, 4,  "Vector sorted correctly");
        require_e(s[6],  ==, 96, "Vector sorted correctly");
        require_e(s[7],  ==, 96, "Vector sorted correctly");
        require_e(s[8],  ==, 97, "Vector sorted correctly");
        require_e(s[9],  ==, 98, "Vector sorted correctly");
        require_e(s[10], ==, 99, "Vector sorted correctly");
    }
}
