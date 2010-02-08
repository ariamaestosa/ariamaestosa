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


#ifndef __SINGLETONG_H__
#define __SINGLETONG_H__

#include <cstring> // For NULL

namespace AriaMaestosa
{
    class SingletonBase
    {
    public:        
        static void deleteAll();
    };
    
    void addToSingletonList(SingletonBase* newone);

    template<typename T>
    class Singleton : public SingletonBase
    {
    public:
        Singleton() { addToSingletonList(this); }
        virtual ~Singleton() {}
        
        static T* getInstance()
        {
            static T* instance = NULL;
            
            if (instance == NULL)
            {
                instance = new T();
            }
            return instance;
        }
    };
    
}

#endif

