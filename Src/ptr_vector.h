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

/*
 * I made this class to work like a regular vector, except that contentsVector are placed
 * one the heap so third-party contentsVector can keep pointers to them.
 */

#ifndef _ptr_vector_
#define _ptr_vector_

#include <vector>
#include <iostream>

#include "Config.h"

namespace AriaMaestosa
{

enum VECTOR_TYPE
{
    REF,
    HOLD
};

template<typename TYPE, VECTOR_TYPE type=HOLD>
class ptr_vector
{
public:
    DECLARE_MAGIC_NUMBER();
    std::vector<TYPE*> contentsVector;

    ptr_vector()
    {
        INIT_MAGIC_NUMBER();
    }

    ~ptr_vector()
    {        
        if (type == HOLD) clearAndDeleteAll();
    }

    ptr_vector<TYPE, REF> getWeakView()
    {
        ptr_vector<TYPE, REF> out;
        out.contentsVector = this->contentsVector;
        return out;
    }
    
    void push_back(TYPE* t)
    {
        assert( MAGIC_NUMBER_OK() );
        
        contentsVector.push_back(t);
    }

    void add(TYPE* t, int index)
    {
        assert( MAGIC_NUMBER_OK() );
        
        contentsVector.insert(contentsVector.begin()+index, t);
    }

    void swap(int ID1, int ID2)
    {
        assert( MAGIC_NUMBER_OK() );
        
        assertExpr(ID1,>,-1);
        assertExpr((unsigned int)ID1,<,contentsVector.size());
        assertExpr(ID2,>,-1);
        assertExpr((unsigned int)ID2,<,contentsVector.size());


        TYPE* temp = contentsVector[ID2];

        contentsVector[ID2] = contentsVector[ID1];
        contentsVector[ID1] = temp;
    }

    // mark is a way to delete an object without changing the order of the element sin the vector
    // it can be useful in a 'for' loop when the loop relies on object IDs and vector size not changing

    void markToBeDeleted(const int ID) // object is removed from vector and deleted
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        delete contentsVector[ID];

        contentsVector[ID] = 0;

    }
    void markToBeRemoved(const int ID) // object is removed from vector but not deleted
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        contentsVector[ID] = 0;

    }

    bool isMarked(const int ID) const
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        return (contentsVector[ID] == 0);
    }

    void removeMarked()
    {
        assert( MAGIC_NUMBER_OK() );
        
        int size = contentsVector.size();
        for(int n=0; n<size; n++)
        {

            if ( contentsVector[n] == 0 )
            {
                contentsVector.erase(contentsVector.begin()+n);
                size = contentsVector.size();
                n -= 2;
                if (n < -1) n=-1;
            }
        }//next

    }

    TYPE* get(const int ID)
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        return contentsVector[ID];
    }
    TYPE& getRef(const int ID) const
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());
        
        return (TYPE&)(*(contentsVector[ID]));
    }

    int size() const
    {
        assert( MAGIC_NUMBER_OK() );
        return contentsVector.size();
    }

    void erase(const int ID)
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        delete contentsVector[ID];

        contentsVector.erase(contentsVector.begin()+ID);
    }

    void remove(const int ID)
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr(ID,>,-1);
        assertExpr((unsigned int)ID,<,contentsVector.size());

        contentsVector.erase(contentsVector.begin()+ID);
    }

    void clearAndDeleteAll()
    {        
        for(unsigned int n=0; n<contentsVector.size(); n++)
        {

            TYPE * pointer = contentsVector[n];
            delete pointer;
        }
        contentsVector.clear();
    }

    TYPE& operator[](const unsigned int ID)
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr((unsigned int)ID,<,contentsVector.size());

        return *(contentsVector[ID]);
    }
    const TYPE& operator[](const unsigned int ID) const
    {
        assert( MAGIC_NUMBER_OK() );
        assertExpr((unsigned int)ID,<,contentsVector.size());

        return *(contentsVector[ID]);
    }


    void clearWithoutDeleting()
    {
        assert( MAGIC_NUMBER_OK() );
        contentsVector.clear();
    }

    void remove(TYPE* obj)
    {
        assert( MAGIC_NUMBER_OK() );

        for(unsigned int n=0; n<contentsVector.size(); n++)
        {

            TYPE * pointer = contentsVector[n];
            if (pointer == obj)
            {
                contentsVector.erase(contentsVector.begin()+n);
                return;
            }
        }


    }

};

}

#endif
