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
