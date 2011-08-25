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

#include "Utils.h"

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
        
#ifndef NDEBUG
        bool m_performing_deletion;
#endif
        
        DECLARE_MAGIC_NUMBER();
        std::vector<TYPE*> contentsVector;
        
        ptr_vector()
        {
#ifndef NDEBUG
            m_performing_deletion = false;
#endif
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
        
        
#if 0
#pragma mark -
#pragma mark Add Items
#endif
        
        void push_back(TYPE* t)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            
            contentsVector.push_back(t);
        }
        
        void add(TYPE* t, int index)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            contentsVector.insert(contentsVector.begin()+index, t);
        }
        
        
#if 0
#pragma mark -
#pragma mark Information on the vector
#endif
        
        /** @return the number of items in this vector */
        int size() const
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            return contentsVector.size();
        }
        
       
#if 0
#pragma mark -
#pragma mark Access items
#endif
        
        /**
          * @param ID  Index of the item to get, in range [0 .. size()-1]
          * @return a pointer to the nth element within the vector.
          */
        TYPE* get(const int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return contentsVector[ID];
        }
        
        /**
          * @param ID  Index of the item to get, in range [0 .. size()-1]
          * @return a const pointer to the nth element within the vector. You will want to use
          *         this variant of 'get' if you received a const ptr_vector.
          */
        const TYPE* getConst(const int ID) const
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return contentsVector[ID];
        }
        
        bool contains(const TYPE* obj) const
        {
            for (unsigned int n=0; n<contentsVector.size(); n++)
            {
                if (contentsVector[n] == obj) return true;
            }
            return false;
        }
        
        /**
          * Same as getConst, except it returns a reference and not a pointer
          */
        TYPE& getRef(const int ID) const
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return (TYPE&)(*(contentsVector[ID]));
        }
        
        /**
         * @param ID  Index of the item to get, in range [0 .. size()-1]
         * @return    a reference to the nth element within the vector.
         */
        TYPE& operator[](const unsigned int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return *(contentsVector[ID]);
        }
        
        /**
         * @param ID  Index of the item to get, in range [0 .. size()-1]
         * @return    a const reference to the nth element within the vector. You will use
         *            this variant of 'operator[]' if you received a const ptr_vector.
         */
        const TYPE& operator[](const unsigned int ID) const
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return *(contentsVector[ID]);
        }
        
        
#if 0
#pragma mark -
#pragma mark Remove items
#endif
        
        /** delete and remove an object from the vector. */
        void erase(const int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            delete contentsVector[ID];
            
            contentsVector.erase(contentsVector.begin()+ID);
        }
        
        /** remove (but do not delete) an object from the vector. */
        void remove(const int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            contentsVector.erase(contentsVector.begin()+ID);
        }
        
        /** delete and remove an object from the vector. */
        bool erase(TYPE* obj)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            const unsigned int count = contentsVector.size();
            for (unsigned int n=0; n<count; n++)
            {
                
                TYPE * pointer = contentsVector[n];
                if (pointer == obj)
                {
                    delete obj;
                    contentsVector.erase(contentsVector.begin()+n);
                    return true;
                }
            }
            return false;
        }
        
        /** remove (but do not delete) an object from the vector. */
        bool remove(TYPE* obj)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            const unsigned int count = contentsVector.size();
            for (unsigned int n=0; n<count; n++)
            {
                
                TYPE * pointer = contentsVector[n];
                if (pointer == obj)
                {
                    contentsVector.erase(contentsVector.begin()+n);
                    return true;
                }
            }
            return false;
        }
        
        
        /** clears the vector and deletes the objects it contained */
        void clearAndDeleteAll()
        {        
#ifndef NDEBUG
            m_performing_deletion = true;
#endif
            const unsigned int count = contentsVector.size();
            for (unsigned int n=0; n<count; n++)
            {
                
                TYPE * pointer = contentsVector[n];
                delete pointer;
            }
            
            contentsVector.clear();
            
#ifndef NDEBUG
            m_performing_deletion = false;
#endif
        }
        
        /** clears the vector, but does not delete the objects it contained */
        void clearWithoutDeleting()
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            contentsVector.clear();
        }

#if 0
#pragma mark -
#pragma mark Utils
#endif
        
        void swap(int ID1, int ID2)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            ASSERT_E(ID1,>,-1);
            ASSERT_E((unsigned int)ID1,<,contentsVector.size());
            ASSERT_E(ID2,>,-1);
            ASSERT_E((unsigned int)ID2,<,contentsVector.size());
            
            
            TYPE* temp = contentsVector[ID2];
            
            contentsVector[ID2] = contentsVector[ID1];
            contentsVector[ID1] = temp;
        }
                
#if 0
#pragma mark -
#pragma mark Item removal with marking system
#endif
        
        /**
         * "mark" is a way to delete an object without changing the order of the elements in the vector
         * (this can be useful in a 'for' loop when the loop relies on object IDs and vector size not changing).
         * call "removeMarked" when you're done marking everything you wanted to mark.
         */
        void markToBeDeleted(const int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            delete contentsVector[ID];
            
            contentsVector[ID] = 0;
            
        }
        
        /**
         * Same as "markToBeDeleted", except that here there object is removed from the
         * vector but not deleted
         */
        void markToBeRemoved(const int ID)
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            contentsVector[ID] = 0;
            
        }
        
        /** @return whether object 'ID" within this vector is marked to be removed (see 'markToBeDeleted') */
        bool isMarked(const int ID) const
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );
            ASSERT_E(ID,>,-1);
            ASSERT_E((unsigned int)ID,<,contentsVector.size());
            
            return (contentsVector[ID] == 0);
        }
        
        /** removes all objects that have been previously marked with 'markToBeDeleted' or 'markToBeRemoved' */
        void removeMarked()
        {
            ASSERT( MAGIC_NUMBER_OK() );
            ASSERT( not m_performing_deletion );

            int vectorSize = contentsVector.size();
            for (int n=0; n<vectorSize; n++)
            {
                
                if ( contentsVector[n] == 0 )
                {
                    contentsVector.erase(contentsVector.begin()+n);
                    vectorSize = contentsVector.size();
                    n -= 2;
                    if (n < -1) n=-1;
                }
            }//next
            
        }

        
    };
    
}

#endif
