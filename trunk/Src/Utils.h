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

// -------------------- my ASSERT stuff -----------------
#include <iostream>
#include <wx/string.h> // needed pretty much everywhere

#ifdef _MORE_DEBUG_CHECKS

#include "IO/IOUtils.h"

#define ASSERT(expr) if (! (expr)){assertFailed( wxT("ASSERT failed: ") + wxString( #expr , wxConvUTF8 ) + wxT("\n@ ") + extract_filename( fromCString(__FILE__) ) + wxT(": ") + to_wxString(__LINE__));}
#define ASSERT_E(v1,sign,v2) if (!((v1) sign (v2))){ std::cout << "ASSERT failed values : " << v1 << #sign << v2 << std::endl; assertFailed( wxT("Assert failed: ") + wxString( #v1, wxConvUTF8 ) + wxString( #sign , wxConvUTF8 ) + fromCString( #v2 ) + wxT("\n@ ") + extract_filename(fromCString(__FILE__)) + wxT(": ") + to_wxString(__LINE__) ); }

/** less strict than ASSERT, doesn't abort program on failure but only prints warning */
#define PREFER(expr) if (! (expr)){ std::cerr << "\n/!\\ PREFER failed: " << #expr << "\n@ " << extract_filename( fromCString(__FILE__) ).mb_str() << ": " << __LINE__ << "\n\n";}
#define PREFER_E(v1,sign,v2) if (!((v1) sign (v2))){ std::cerr << "\n/!\\ PREFER failed: " << #v1 << #sign << #v2 << "\n@ " << extract_filename( fromCString(__FILE__) ).mb_str() << ": " << __LINE__ << "\n"; std::cerr << "PREFER failed values : " << v1 << #sign << v2 << "\n\n"; }


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
#define MAGIC_NUMBER_OK_FOR(whom) ((whom).ariadebug_magic_number.ariadebug_magic_number == 0xCAFEC001)

#else

#define ASSERT(expr)
#define ASSERT_E(v1,sign,v2)

#define PREFER(expr)
#define PREFER_E(v1,sign,v2)

#define DECLARE_MAGIC_NUMBER()
#define MAGIC_NUMBER_OK() true
#define MAGIC_NUMBER_OK_FOR(whom) true

#endif

#include "LeakCheck.h"

#define PRINT_VAR( foo ) #foo << " = " << (foo) << "; "

static const wxString FILE_SEPARATOR = wxT(",");


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
    
    OwnerPtr(OwnerPtr<T>& ptr)
    {
        if (owner) delete raw_ptr;
        raw_ptr = ptr;
        ptr.owner = false; // prevent the older instance from deleting the pointer
    }
    
    OwnerPtr(T* obj)
    {
        raw_ptr = obj;
        owner = true;
    }
    
    ~OwnerPtr()
    {
        if (owner) delete raw_ptr;
#ifdef _MORE_DEBUG_CHECKS
        raw_ptr = (T*)0xDEADBEEF;
#endif
    }
    
    OwnerPtr& operator=(T* ptr)
    {
        if (owner) delete raw_ptr;
        raw_ptr = ptr;
        return *this;
    }
    OwnerPtr& operator=(OwnerPtr& other)
    {
        if (owner) delete raw_ptr;
        raw_ptr = other.raw_ptr;
        other.owner = false;
        return *this;
    }
    
    operator T*()
    { 
        return raw_ptr; 
    }
    
    operator const T*() const
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

template<typename T>
class IModelListener
{
public:
    virtual ~IModelListener() {}
    virtual void onModelChanged(T newval) = 0;
};

template<typename T>
class Model
{
    T m_data;
    IModelListener<T>* m_listener;
    
public:
    
    Model(T data)
    {
        m_data = data;
        m_listener = NULL;
    }
    
    void setListener(IModelListener<T>* listener)
    {
        m_listener = listener;
    }
    
    T       getValue()       { return m_data; }
    const T getValue() const { return m_data; }
    
    void    setValue(T nval)
    {
        m_data = nval;
        if (m_listener != NULL) m_listener->onModelChanged(nval);
    }
};

#define once for(int __xyzwrst=0; __xyzwrst<1; __xyzwrst++)

#endif
