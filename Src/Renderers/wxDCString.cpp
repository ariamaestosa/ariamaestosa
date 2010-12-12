#ifdef RENDERER_WXWIDGETS

#include "wxDCString.h"
#include "Utils.h"

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include "AriaCore.h"
#include "Renderers/RenderAPI.h"

namespace AriaMaestosa
{
    
#if 0
#pragma mark -
#pragma mark wxDCString
#endif
    
    /** constructs an empty GLString. Set string later with operator=. */
    wxDCString::wxDCString()
    {
        consolidated = false;
        max_width = -1;
        warp = false;
        w = -1;
        h = -1;
    }
    
    
    /** constructs a GLstring with 'message' as contents. */
    wxDCString::wxDCString(wxString message) : wxString(message)
    {
        consolidated = false;
        max_width = -1;
        warp = false;
        w = -1;
        h = -1;
    }
    
    wxDCString::~wxDCString()
    {
    }
    
    
    void wxDCString::setMaxWidth(const int w, const bool warp)
    {
        max_width = w;
        this->warp = warp;
    }
    
    void wxDCString::setFont(wxFont font)
    {
        this->font = font;
    }
    
    void wxDCString::bind()
    {
        if (not consolidated)
        {
            if (font.IsOk()) Display::renderDC->SetFont(font);
            else Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
            
            Display::renderDC->GetTextExtent(*this, &w, &h);
            
            consolidated = true;
        }
    }
    
    int wxDCString::getWidth()
    {
        ASSERT_E(w, >=, 0);
        ASSERT_E(w, <, 90000);
        return w;
    }
    void wxDCString::scale(float f)
    {
    }
    void wxDCString::rotate(int angle)
    {
    }
    
    void wxDCString::render(const int x, const int y)
    {
        if (not consolidated) bind();
            
        ASSERT_E(h, >=, 0);
        ASSERT_E(h, <, 90000);
        
        if (font.IsOk()) Display::renderDC->SetFont(font);
        else Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));

        if (max_width != -1 and getWidth()>max_width)
        {
            if (not warp)
            {
                wxString shortened = *this;
                
                while (Display::renderDC->GetTextExtent(shortened).GetWidth() > max_width)
                {
                    shortened = shortened.Truncate(shortened.size()-1);
                }
                Display::renderDC->DrawText(shortened, x, y-h);
            }
            else // wrap
            {
                int my_y = y-h;
                wxString multiline = *this;
                multiline.Replace(wxT(" "),wxT("\n"));
                multiline.Replace(wxT("/"),wxT("/\n"));

                wxStringTokenizer tkz(multiline, wxT("\n"));
                while ( tkz.HasMoreTokens() )
                {
                    wxString token = tkz.GetNextToken();
                    Display::renderDC->DrawText(token, x, my_y);
                    my_y += h;
                }
            }
        }
        else
            Display::renderDC->DrawText(*this, x, y-h);
    }
    
    
    void wxDCString::set(const wxString& string)
    {
        (*((wxString*)this))=string;
        consolidated = false;
    }
    
    
#if 0
#pragma mark -
#pragma mark wxDCNumberRenderer
#endif
    
    DEFINE_SINGLETON( wxDCNumberRenderer );
    
    wxDCNumberRenderer::wxDCNumberRenderer()
    {
        consolidated = false;
        w = -1;
        h = -1;
    }
    wxDCNumberRenderer::~wxDCNumberRenderer(){}
    
    void wxDCNumberRenderer::bind()
    {
        if (not consolidated)
        {

#if defined(__WXMSW__)
            //FIXME: find why we need this special case
            Display::renderDC->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
            //if (font.IsOk()) Display::renderDC->SetFont(font);
            // else
            Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
#endif
            
            Display::renderDC->GetTextExtent(wxT("0"), &w, &h);
            
            consolidated = true;
        }
    }
    
    void wxDCNumberRenderer::renderNumber(wxString s, int x, int y)
    {
        ASSERT_E(h, >, -1);
        ASSERT_E(h, <, 90000);

#if defined(__WXMSW__)
        //FIXME: find why we need this special case
        Display::renderDC->SetFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) );
#else
        //if (font.IsOk()) Display::renderDC->SetFont(font);
        //else
        Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
#endif
        
        Display::renderDC->DrawText(s, x, y-h);
    }
    
    void wxDCNumberRenderer::renderNumber(int i, int x, int y)
    {
        renderNumber( to_wxString(i), x, y );
    }
    void wxDCNumberRenderer::renderNumber(float f, int x, int y)
    {
        renderNumber( to_wxString(f), x, y );
    }
    
    
#if 0
#pragma mark -
#pragma mark wxDCStringArray
#endif
    
    wxDCStringArray::wxDCStringArray()
    {
        consolidated = false;
    }
    wxDCStringArray::wxDCStringArray(const wxString strings_arg[], int amount)
    {
        for (int n=0; n<amount; n++)
        {
            strings.push_back( wxDCString(strings_arg[n]) );
        }
        consolidated = false;
    }
    
    wxDCStringArray::~wxDCStringArray(){}
    
    wxDCString& wxDCStringArray::get(const int id)
    {
        return strings[id];
    }
    
    void wxDCStringArray::addString(wxString string)
    {
        strings.push_back( wxDCString(string) );
    }
    
    void wxDCStringArray::bind()
    {
        if (not consolidated)
        {
            if (not font.IsOk()) return;
            
            const int amount = strings.size();
            for (int n=0; n<amount; n++)
            {
                strings[n].setFont(font);
            }
            
            consolidated = true;
        }
    }
    
    void wxDCStringArray::setFont(wxFont font)
    {
        this->font = font;
    }
    

}
#endif
