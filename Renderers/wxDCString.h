#ifndef _dcstring_
#define _dcstring_

#ifdef RENDERER_WXWIDGETS


#include "wx/wx.h"
#include <vector>

namespace AriaMaestosa
{

    class wxDCStringArray;

class wxDCString : public wxString
{
protected:
    wxFont font;

    int w, h;
    friend class wxDCStringArray;

    bool consolidated;
    
    int max_width;
    
    bool warp;
    
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


class wxDCNumberRenderer
{
    bool consolidated;
    int w, h;
public:
    wxDCNumberRenderer();
    ~wxDCNumberRenderer();

    void bind();
    void consolidate(wxDC* dc){}

    void renderNumber(wxString s, int x, int y);
    void renderNumber(int i, int x, int y);
    void renderNumber(float f, int x, int y);
};

class wxDCStringArray
{
    std::vector<wxDCString> strings;
    wxFont font;
    bool consolidated;
public:
    wxDCStringArray();
    wxDCStringArray(const wxString strings_arg[], int amount);
    ~wxDCStringArray();

    wxDCString& get(const int id);

    void bind();

    void addString(wxString string);
    int getStringAmount() const { return strings.size(); }
    
    void setFont(wxFont font);

    void consolidate(wxDC* dc){}
};

typedef wxDCStringArray AriaRenderArray;

}
#endif
#endif
