/////////////////////////////////////////////////////////////////////////////
// Name:        bsizer.cpp
// Purpose:     provide new wxBorderSizer class for layout
// Author:      Alex Andruschak
// Modified by:
// Created:
// RCS-ID:      $Id:bsizer.cpp,v 1.14 2000/20/04
// Copyright:   (c) DekaSoft
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation "bsizer.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include <iostream>

#include "bsizer.h"


//---------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxBorderSizer, wxSizer);
IMPLEMENT_ABSTRACT_CLASS(wxBorderItemHandle, wxObject);

//---------------------------------------------------------------------------
// wxBorderItemHandle
//---------------------------------------------------------------------------

wxBorderItemHandle::wxBorderItemHandle( wxString ln )
{
    if (ln.CmpNoCase(wxT("North")) == 0)
    {
        location_string = wxT("North");
        location_id = Location::ID_North;
    }
    else if (ln.CmpNoCase(wxT("South")) == 0)
    {
        location_string = wxT("South");
        location_id = Location::ID_South;
    }
    else if (ln.CmpNoCase(wxT("West")) == 0)
    {
        location_string = wxT("West");
        location_id = Location::ID_West;
    }
    else if (ln.CmpNoCase(wxT("East")) == 0)
    {
        location_string = wxT("East");
        location_id = Location::ID_East;
    }
    else if (ln.CmpNoCase(wxT("Center")) == 0)
    {
        location_string = wxT("Center");
        location_id = Location::ID_Center;
    }
    else
    {
        std::cerr << "wxBorderSizer - Invalid location: " << (char*)( ln.c_str() ) << std::endl;
    }

}

wxBorderItemHandle::wxBorderItemHandle( int ln )
{

    if (ln == Location::ID_North)
    {
        location_string = wxT("North");
        location_id = Location::ID_North;
    }
    else if (ln == Location::ID_South)
    {
        location_string = wxT("South");
        location_id = Location::ID_South;
    }
    else if (ln == Location::ID_West)
    {
        location_string = wxT("West");
        location_id = Location::ID_West;
    }
    else if (ln == Location::ID_East)
    {
        location_string = wxT("East");
        location_id = Location::ID_East;
    }
    else if (ln == Location::ID_Center)
    {
        location_string = wxT("Center");
        location_id = Location::ID_Center;
    }
    else
    {
        std::cerr << "wxBorderSizer - Invalid location: " << ln << std::endl;
    }

}
//---------------------------------------------------------------------------
wxString wxBorderItemHandle::GetLocationName()
{
    return location_string;
}
//---------------------------------------------------------------------------
unsigned long wxBorderItemHandle::GetLocationCode()
{
    return location_id;
}

//---------------------------------------------------------------------------
// wxBorderSizer
//---------------------------------------------------------------------------

wxBorderSizer::wxBorderSizer()
{
}

wxBorderSizer::~wxBorderSizer()
{
    m_children.DeleteContents(TRUE);
}

/*
 * This method calculates the size and lcoation of objects.
 */
void wxBorderSizer::RecalcSizes()
{
    wxPoint  c_point;
    wxSize   c_size;
    long     north_offset = 0;
    long     south_offset = 0;
    long     west_offset = 0;
    long     east_offset = 0;
    wxNode*  current;
    wxSize   size = GetSize();
    wxPoint  pos = GetPosition();

    if (m_children.GetCount() == 0)
        return;

    // m_children list must be sorted before layouting
    m_children.Sort(SortFunction);

    // iterate through all children
    current = (wxNode*)m_children.GetFirst();
    while (current != NULL)
    {
        wxSizerItem *item = (wxSizerItem*) current->GetData();

        // Retrieve location information given by user
        wxBorderItemHandle* location;
        if (item != NULL && (location = (wxBorderItemHandle *)item->GetUserData()) != NULL)
        {
            // what is the minimal size for this component
            wxSize minimal_needed_size = item->CalcMin();

            // if the object we are sizing now is a component and has children, ask its sizer how much space it needs.
            // if to show all its children it needs more space than the bare minimal, we will give it.
            if (item->IsWindow() && item->GetWindow()->GetSizer() != NULL)
            {
                wxSize min_size = item->GetWindow()->GetSizer()->CalcMin();
                if (minimal_needed_size.GetWidth() < min_size.GetWidth() ||
                    minimal_needed_size.GetHeight() < min_size.GetHeight())
                {
                    minimal_needed_size = min_size;
                }
            }

            // North
            if (location->GetLocationCode() == Location::ID_North)
            {
                // minimal x and y
                c_point.x = pos.x;
                c_point.y = pos.y;

                // maximal width, minimal height
                c_size.SetWidth(size.GetWidth());
                c_size.SetHeight(minimal_needed_size.GetHeight());

                // remember some space is taken at North
                north_offset = minimal_needed_size.GetHeight();
            }

            // South
            if (location->GetLocationCode() == Location::ID_South)
            {
                // minimal x, maximal y
                c_point.x = pos.x;
                c_point.y = pos.y + size.GetHeight() - minimal_needed_size.GetHeight();

                // maximal width, minimal height
                c_size.SetWidth(size.GetWidth());
                c_size.SetHeight(minimal_needed_size.GetHeight());

                // remember some space is taken at South
                south_offset = minimal_needed_size.GetHeight();
            }

            // West
            if (location->GetLocationCode() == Location::ID_West)
            {
                // minimal x and y
                c_point.x = pos.x;
                c_point.y = pos.y + north_offset;

                // minimal width, maximal height
                c_size.SetWidth(minimal_needed_size.GetWidth());
                c_size.SetHeight(size.GetHeight() - (north_offset + south_offset));

                // remember some space is taken at West
                west_offset = minimal_needed_size.GetWidth();
            }

            // East
            if (location->GetLocationCode() == Location::ID_East)
            {
                // maximal x, minimal y
                c_point.x = pos.x + size.GetWidth() - minimal_needed_size.GetWidth();
                c_point.y = pos.y + north_offset;

                // minimal width, maximal height
                c_size.SetWidth(minimal_needed_size.GetWidth());
                c_size.SetHeight(size.GetHeight() - (north_offset + south_offset));

                // remember some space is taken at East
                east_offset = minimal_needed_size.GetWidth();
            }

            // Center
            if (location->GetLocationCode() == Location::ID_Center)
            {
                // minimal x and y
                c_point.x = pos.x + west_offset;
                c_point.y = pos.y + north_offset;

                // maximal width and height
                c_size.SetWidth(size.GetWidth() - (west_offset + east_offset));
                c_size.SetHeight(size.GetHeight() - (north_offset + south_offset));
            }

            // apply new dimensions to object
            item->SetDimension(c_point, c_size);
        }
        // continue with next item
        current = current->GetNext();
    }

}

/*
 * Calculates what is the minimal needed size to display eveything
 */
wxSize wxBorderSizer::CalcMin()
{
    unsigned long width_north = 0;
    unsigned long width_south = 0;
    unsigned long min_width = 0;

    unsigned long height_west = 0;
    unsigned long height_east = 0;
    unsigned long min_height = 0;

    wxNode *current;

    // no children, return minimal size
    if (m_children.GetCount() == 0) return wxSize(2,2);

    // m_children list must be sorted before calculating
    m_children.Sort(SortFunction);

    // iterate through all children
    current = (wxNode*)m_children.GetFirst();
    while (current != NULL)
    {
        wxSizerItem *item = (wxSizerItem*) current->GetData();
        wxBorderItemHandle* location;

        // Retrieve location information given by user
        if (item != NULL && (location = (wxBorderItemHandle *)item->GetUserData()) != NULL)
        {
            wxSize minimal_needed_size = item->CalcMin();

            // if the object we are sizing now is a component and has children, ask its sizer how much space it needs.
            // if to show all its children it needs more space than the bare minimal, we will give it.
            if (item->IsWindow() && item->GetWindow()->GetSizer() != NULL)
            {
                wxSize min_size = item->GetWindow()->GetSizer()->CalcMin();
                if (minimal_needed_size.GetWidth() < min_size.GetWidth() ||
                    minimal_needed_size.GetHeight() < min_size.GetHeight())
                {
                    minimal_needed_size = min_size;
                }
            }

            /*
             * There can only be one North component, only one South component, etc.
             * Therefore, adding the height of the northern component + the height of the southern component will give minimal needed height.
             * Same thing goes horizontally.
             */

            // North
            if (location->GetLocationCode() == Location::ID_North)
            {
                min_height += minimal_needed_size.GetHeight();
                width_north = minimal_needed_size.GetWidth();
            }

            // South
            if (location->GetLocationCode() == Location::ID_South)
            {
                min_height += minimal_needed_size.GetHeight();
                width_south = minimal_needed_size.GetWidth();
            }

            // West
            if (location->GetLocationCode() == Location::ID_West)
            {
                height_west = minimal_needed_size.GetHeight();
                min_width += minimal_needed_size.GetWidth();
            }

            // East
            if (location->GetLocationCode() == Location::ID_East)
            {
                height_east = minimal_needed_size.GetHeight();
                min_width += minimal_needed_size.GetWidth();
            }

        }
        current = current->GetNext();
    }

    // use largest width between north and south (unless current minimal size is already large enough for both)
    if (width_north > width_south )
    {
        if (width_north > min_width) min_width = width_north;
    }
    else
    {
        if (width_south > min_width) min_width = width_south;
    }

    // height currently holds the needed space for north and south.
    // Add the minimal amount to hold that largest of east/west.
    // Then it will be high enough to contain everything.
    if (height_west > height_east) min_height += height_west;
    else  min_height += height_east;

    return wxSize( min_width, min_height );
}
//---------------------------------------------------------------------------
int wxBorderSizer::SortFunction(const void* arg1, const void* arg2)
{
    wxSizerItem** item1 = (wxSizerItem**)arg1;
    wxSizerItem** item2 = (wxSizerItem**)arg2;

    wxBorderItemHandle* ih1 = (wxBorderItemHandle*)(*item1)->GetUserData();
    wxBorderItemHandle* ih2 = (wxBorderItemHandle*)(*item2)->GetUserData();

    if (ih1 == NULL || ih2 == NULL) return 0;

    if (ih1->GetLocationCode() == ih2->GetLocationCode()) return 0;

    if (ih1->GetLocationCode() > ih2->GetLocationCode()) return 1;
    else return -1;

}
//---------------------------------------------------------------------------

namespace Location
{

wxObject* North(){ return (wxObject*)(new wxBorderItemHandle(Location::ID_North)); }
wxObject* South(){ return (wxObject*)(new wxBorderItemHandle(Location::ID_South)); }
wxObject* West(){ return (wxObject*)(new wxBorderItemHandle(Location::ID_West)); }
wxObject* East(){ return (wxObject*)(new wxBorderItemHandle(Location::ID_East)); }
wxObject* Center(){ return (wxObject*)(new wxBorderItemHandle(Location::ID_Center)); }

}
