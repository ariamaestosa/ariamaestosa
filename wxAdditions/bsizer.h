/////////////////////////////////////////////////////////////////////////////
// Name:        bsizer.h
// Purpose:     provide wxBorderSizer class for layouting
// Author:      Alex Andruschak
// Modified by:
// Created:
// RCS-ID:      $Id: bsizer.h,v 1.10 2000/04/20
// Copyright:   (c) DekaSoft
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_BORDER_SIZER_H__
#define __WX_BORDER_SIZER_H__

#ifdef __GNUG__
#pragma interface "bsizer.h"
#endif

#include "wx/sizer.h"




//---------------------------------------------------------------------------
// wxBorderItemHandle
//---------------------------------------------------------------------------

class WXDLLEXPORT wxBorderItemHandle: public wxObject
{
    DECLARE_CLASS(wxBorderItemHandle);
protected:
    wxString          location_string;
    unsigned long     location_id;

public:
    wxBorderItemHandle( wxString location );
    wxBorderItemHandle( int location );

    wxString          GetLocationName();
    unsigned long     GetLocationCode();

};

namespace Location
{
enum
{
    ID_North = 0,
    ID_South = 1,
    ID_West = 2,
    ID_East = 3,
    ID_Center = 4
};

wxObject* North();
wxObject* South();
wxObject* West();
wxObject* East();
wxObject* Center();

}

//---------------------------------------------------------------------------
// wxBorderSizer
//---------------------------------------------------------------------------

class WXDLLEXPORT wxBorderSizer: virtual public wxSizer
{
    DECLARE_CLASS(wxBorderSizer);
public:
    wxBorderSizer();
    ~wxBorderSizer();

    virtual void RecalcSizes();
    virtual wxSize CalcMin();

private:
    static int SortFunction(const void* arg1, const void* arg2);
};

#endif
  // __WX_BORDER_SIZER_H__
