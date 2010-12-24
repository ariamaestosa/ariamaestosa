#ifndef __WXDC_STRING_H__
#define __WXDC_STRING_H__

#ifdef RENDERER_WXWIDGETS

#include <wx/font.h>
#include <wx/string.h>

class wxDC;

#include <vector>
#include "Singleton.h"

namespace AriaMaestosa
{
    
    class wxDCStringArray;
    
    /**
     * @brief   wxWidgets render backend : text renderer
     * @ingroup renderers
     */
    class wxDCString : public wxString
    {
    protected:
        wxFont m_font;
        
        int m_w, m_h;
        friend class wxDCStringArray;
        
        bool m_consolidated;
        
        int m_max_width;
        
        bool m_warp;
        
    public:
        
        /** constructs an empty GLString. Set string later with operator=. */
        wxDCString();
        /** constructs a GLstring with 'message' as contents. */
        wxDCString(wxString message);
        ~wxDCString();
        
        
        void bind();
        
        void setMaxWidth(const int w, const bool warp=false);
        
        /** set how to draw string for next consolidate() - has no immediate effect,
         you need to call consolidate() to get results  */
        void setFont(wxFont font);
        
        void consolidate(wxDC* dc);
        
        void render(const int x, const int y);
        
        /** changes the string of this element - has no immediate effect,
         you need to call consolidate() to get results */
        void set(const wxString& string);
        
        
        int getWidth();
        void scale(float f);
        void rotate(int angle);
    };
    
    typedef wxDCString AriaRenderString;
    
    
    /**
     * @brief   wxWidgets render backend : number renderer
     * @ingroup renderers
     */
    class wxDCNumberRenderer : public Singleton<wxDCNumberRenderer>
    {
        bool m_consolidated;
        int m_w, m_h;
        
    public:
        
        wxDCNumberRenderer();
        ~wxDCNumberRenderer();
        
        void bind();
        void consolidate(wxDC* dc){}
        
        void renderNumber(wxString s, int x, int y);
        void renderNumber(int i, int x, int y);
        void renderNumber(float f, int x, int y);
    };
    
    /**
     * @brief   wxWidgets render backend : text array renderer
     * @ingroup renderers
     */
    class wxDCStringArray
    {
        std::vector<wxDCString> m_strings;
        wxFont m_font;
        bool m_consolidated;
        
    public:
        
        wxDCStringArray();
        wxDCStringArray(const wxString strings_arg[], int amount);
        ~wxDCStringArray();
        
        wxDCString& get(const int id);
        
        void bind();
        
        void addString(wxString string);
        int getStringAmount() const { return m_strings.size(); }
        
        void setFont(wxFont font);
        
        void consolidate(wxDC* dc){}
    };
    
    typedef wxDCStringArray AriaRenderArray;
    
}
#endif
#endif
