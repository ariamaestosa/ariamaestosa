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

#ifndef _config_
#define _config_

#include "IO/IOUtils.h"

// values
#define DEFAULT_SONG_LENGTH 12

// -------------------- my assert stuff -----------------
#include <iostream>


#ifdef _MORE_DEBUG_CHECKS

#include "IO/IOUtils.h"

#undef assert
#define assert(expr) if (! (expr)){assertFailed( wxT("Assert failed: ") + wxString( #expr , wxConvUTF8 ) + wxT("\n@ ") + extract_filename( fromCString(__FILE__) ) + wxT(": ") + to_wxString(__LINE__));}
#define assertExpr(v1,sign,v2) if (!((v1) sign (v2))){ std::cout << "assert failed values : " << v1 << #sign << v2 << std::endl; assertFailed( wxT("Assert failed: ") + wxString( #v1, wxConvUTF8 ) + wxString( #sign , wxConvUTF8 ) + fromCString( #v2 ) + wxT("\n@ ") + extract_filename(fromCString(__FILE__)) + wxT(": ") + to_wxString(__LINE__) ); }

struct AriaDebugMagicNumber
{
    unsigned int ariadebug_magic_number;
    
    AriaDebugMagicNumber()
    {
        ariadebug_magic_number = 0xCAFEC001;
    }
    ~AriaDebugMagicNumber()
    {
        ariadebug_magic_number = 0xDEADBEEF;
    }
};

#define DECLARE_MAGIC_NUMBER() AriaDebugMagicNumber ariadebug_magic_number
#define MAGIC_NUMBER_OK() (this->ariadebug_magic_number.ariadebug_magic_number == 0xCAFEC001)
#define MAGIC_NUMBER_OK_FOR(whom) ((whom)->ariadebug_magic_number.ariadebug_magic_number == 0xCAFEC001)

#else

#undef assert
#define assert(expr)
#define assertExpr(v1,sign,v2)

#define DECLARE_MAGIC_NUMBER()
#define MAGIC_NUMBER_OK() true
#define MAGIC_NUMBER_OK_FOR(whom) true

#endif

#include "LeakCheck.h"

#define PRINT_VAR( foo ) #foo << " = " << (foo) << "; "


template<typename T>
class OwnerPtr
{
public:
    bool owner;
    T* raw_ptr;
    
    OwnerPtr()
    {
        raw_ptr = NULL;
        owner = true;
    }
    OwnerPtr(OwnerPtr& copyCtor)
    {
        this->raw_ptr = copyCtor.raw_ptr;
        copyCtor.raw_ptr; // prevent the oldder instance from deleting the pointer
        owner = true;
    }
    
    OwnerPtr(T* obj)
    {
        raw_ptr = obj;
        owner = true;
    }
    
    ~OwnerPtr()
    {
        if (owner) delete raw_ptr;
    }
    
    OwnerPtr& operator=(T* ptr)
    {
        if (owner) delete raw_ptr;
        raw_ptr = ptr;
        return *this;
    }
    
    operator T*()
    { 
        return raw_ptr; 
    }
    
    T* operator->() const
    {
        return raw_ptr;
    }
    
};
template<typename T>
class WxOwnerPtr
{
public:
    T* raw_ptr;
    
    WxOwnerPtr()
    {
        raw_ptr = NULL;
    }
    WxOwnerPtr(T* obj)
    {
        raw_ptr = obj;
    }
    
    ~WxOwnerPtr()
    {
        if (raw_ptr != NULL) raw_ptr->Destroy();
    }
    
    WxOwnerPtr& operator=(T* ptr)
    {
        if (raw_ptr != NULL) raw_ptr->Destroy();
        
        raw_ptr = ptr;
        return *this;
    }
    
    operator T*()
    { 
        return raw_ptr; 
    }
    
    T* operator->() const
    {
        return raw_ptr;
    }
    
};


#endif
