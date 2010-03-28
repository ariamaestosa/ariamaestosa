#include "Singleton.h"
#include "ptr_vector.h"

namespace AriaMaestosa
{

    ptr_vector<SingletonBase>* g_singletons = NULL;
    void addToSingletonList(SingletonBase* newone)
    {
        if (g_singletons == NULL) g_singletons = new ptr_vector<SingletonBase>();
        
        ASSERT (g_singletons != NULL);
        g_singletons->push_back(newone);
    }
    void cleanSingletons()
    {
        ASSERT (g_singletons != NULL);
        g_singletons->clearAndDeleteAll();
        delete g_singletons;
        g_singletons = NULL;
    }
    
    void SingletonBase::deleteAll()
    {
        cleanSingletons();
    }
    
}
