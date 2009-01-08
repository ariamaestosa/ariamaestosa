#ifdef RENDERER_WXWIDGETS

#include "wxDCString.h"
#include "Config.h"

#include "wx/wx.h"
#include "AriaCore.h"
#include "Renderers/RenderAPI.h"

namespace AriaMaestosa
{
    
    /** constructs an empty GLString. Set string later with operator=. */
    wxDCString::wxDCString()
    {
        consolidated = false;
    }
    /** constructs a GLstring with 'message' as contents. */
    
    wxDCString::wxDCString(wxString message) : wxString(message)
    {
        consolidated = false;
    }
    
    wxDCString::~wxDCString()
    {
    }
    
    
    void wxDCString::setFont(wxFont font)
    {
        this->font = font;
    }
    
    void wxDCString::bind()
    {
        if(not consolidated)
        {
            if(font.IsOk()) Display::renderDC->SetFont(font);
            else Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
            
            Display::renderDC->GetTextExtent(*this, &w, &h);
            
            consolidated = true;
        }
    }
    
    int wxDCString::getWidth()
    {
        return w;
    }
    float wxDCString::scale(float f)
    {
    }
    void wxDCString::rotate(int angle)
    {
    }
    
    void wxDCString::render(const int x, const int y)
    {
        if(not consolidated) bind();
            
        if(font.IsOk()) Display::renderDC->SetFont(font);
        else Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
        
        Display::renderDC->DrawText(*this, x, y-h);
    }
    
    
    void wxDCString::operator=(wxString& string)
    {
        (*this) = string;
    }
    
    
    
    wxDCNumberRenderer::wxDCNumberRenderer()
    {
        consolidated = false;
    }
    wxDCNumberRenderer::~wxDCNumberRenderer(){}
    
    void wxDCNumberRenderer::bind()
    {
        if(not consolidated)
        {
            //if(font.IsOk()) Display::renderDC->SetFont(font);
           // else
            Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
            
            Display::renderDC->GetTextExtent(wxT("0"), &w, &h);
            
            consolidated = true;
        }
    }
    
    void wxDCNumberRenderer::renderNumber(wxString s, int x, int y)
    {
        //if(font.IsOk()) Display::renderDC->SetFont(font);
        //else
        Display::renderDC->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
        
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
    
    
    
    wxDCStringArray::wxDCStringArray()
    {
        consolidated = false;
    }
    wxDCStringArray::wxDCStringArray(const wxString strings_arg[], int amount)
    {
        for(int n=0; n<amount; n++)
            strings.push_back( wxDCString(strings_arg[n]) );
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
        if(not consolidated)
        {
            if(not font.IsOk()) return;
            
            const int amount = strings.size();
            for(int n=0; n<amount; n++)
                strings[n].setFont(font);
                
            consolidated = true;
        }
    }
    
    void wxDCStringArray::setFont(wxFont font)
    {
        this->font = font;
    }
    

}
#endif