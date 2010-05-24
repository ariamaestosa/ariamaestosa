#ifdef RENDERER_OPENGL
#ifndef __GL_STRING_H__
#define __GL_STRING_H__

#include "Utils.h"

#include "wx/font.h"
#include "wx/string.h"
class wxImage;
class wxBitmap;
class wxDC;

#include "ptr_vector.h"

namespace AriaMaestosa
{
    
    class wxGLString;
    class wxGLStringArray;
    class wxGLStringNumber;
    
    /**
     * @brief   OpenGL render backend : text renderer helper class (OpenGL backend)
     * @ingroup renderers
     */
    class TextTexture
    {
        friend class wxGLString;
        friend class wxGLStringArray;
        friend class wxGLStringNumber;
    private:
        
        /** here I don't use GLuint to avoid including OpenGL everywhere in the project */
        unsigned int* ID;
    protected:
        
        /** here I don't use GLuint to avoid including OpenGL everywhere in the project */
        unsigned int* getID();
        
        TextTexture();
        TextTexture(wxBitmap& bmp);
        void load(wxImage* img);
    public:
        LEAK_CHECK();
        
        ~TextTexture();
    };
    
    /** 
      * @brief OpenGL render backend : utility class for text rendering
      *
      * Base class for renderable elements. You won't create this one directly,
      * but may use its public members from wxGLString since it inherits from TextGLDrawable.
      * This class will be useful if you wish to apply effects to the text like rotation or
      * scaling.
      *
      * @ingroup renderers
      */
    class TextGLDrawable
    {
        friend class wxGLString;
        friend class wxGLStringArray;
        friend class wxGLStringNumber;
    protected:
        
        int m_x, m_y, m_angle;
        float m_x_scale, m_y_scale;
        OwnerPtr<TextTexture> m_image;
        bool m_x_flip, m_y_flip;
        
        int y_offset;
        
        float tex_coord_x1, tex_coord_y1;
        float tex_coord_x2, tex_coord_y2;
        int m_w, m_h, texw, texh;
        
        int m_max_width;
        
        TextGLDrawable(TextTexture* image=(TextTexture*)0);
        void render();
        void setImage(TextTexture* image, bool giveUpOwnership=false);
        void move(int x, int y);
    public:
        
        LEAK_CHECK();
        
        virtual ~TextGLDrawable() {}
        
        /** allows you to flip the rendering vertically and/or horizontally */
        void setFlip(bool x, bool y);
        
        /** scale the rendering , horizontally and vertically (allows stretching) */
        void scale(float x, float y);
        
        /** scale the rendering and keep the same aspect ratio */
        void scale(float k);
        
        /** rotate the rendering by 'angle' degrees */
        void rotate(int angle);
        
        void setMaxWidth(const int w);
        
        /** returns the width of this element */
        int getWidth() const { return m_w; }
        /** returns the height of this element */
        int getHeight() const { return m_h; }
        
    };
    
    class wxGLStringArray;
    
    /**
     @brief OpenGL render backend : text renderer
     
     It draws a single string on a single line.
     If you plan to render multiple strings, this class is not the fastest.
     
     Use example :
     
     \code
     wxGLString my_message(wxT("Hello World"));
     ...
     if (first_render)
     my_message.consolidate(&dc);
     
     glColor3f(0,0,0); // black text
     my_message.bind();
     my_message.render(x, y);
     \endcode

     @ingroup renderers
     */
    class wxGLString : public wxString, public TextGLDrawable
    {
    protected:
        wxFont m_font;
        
        int m_warp_after;
        
        bool m_consolidated;
        
        friend class wxGLStringArray;
        
        void calculateSize(wxDC* dc, const bool ignore_font=false /* when from array */);
        void consolidateFromArray(wxDC* dc, int x, int y);
    public:    
        /** constructs an empty GLString. Set string later with operator=. */
        wxGLString();
        /** constructs a GLstring with 'message' as contents. */
        wxGLString(wxString message);
        virtual ~wxGLString();
        
        /** call just before render() - binds the OpenGL. If you render the same string many
         times, or render from an array, bind only once, this will improve performance */
        void bind();
        
        /** set how to draw string for next consolidate() - has no immediate effect,
         you need to call consolidate() to get results  */
        void setFont(wxFont font);
        
        /** consolidates the current string info into a GL string. call this after
         setting up strings, font and color (if necessary), and before rendering.
         The wxDC argument is only used to calculate text extents and will not be rendered on. */
        virtual void consolidate(wxDC* dc);
        
        void setMaxWidth(const int w, const bool warp=false /*false: truncate. true: warp.*/);
        
        /** render this string at coordinates (x,y). Must be called after bind(). */
        void render(const int x, const int y);
        
        /** changes the string of this element */
        void set(const wxString& string);
        
        // override wxString's operators
        wxString& operator =(const wxString& str){ set(str); return *this; }
        wxString& operator =(const wxChar* psz){ set(wxString(psz)); return *this;}
    };
    
    typedef wxGLString AriaRenderString;
    
    /**
     @brief OpenGL render backend : number renderer
     
     Use example :
     
     \code
     wxGLNumberRenderer glnumbers;
     ...
     if (first_render)
     glnumbers.consolidate();
     
     glColor3f(0,0,0); // black numbers
     glnumbers.bind();
     glnumbers.renderNumber( 3.141593f, x, y );
     \endcode
     
     @ingroup renderers
     */
    class wxGLNumberRenderer : public wxGLString
    {
        int* number_location;
        int space_w;
    public:
        wxGLNumberRenderer();
        virtual ~wxGLNumberRenderer();
        
        /** inits the class to be ready to render.
         The wxDC argument is only used to calculate text extents and will not be rendered on. */
        void consolidate(wxDC* dc);
        
        /** render this number at coordinates (x,y), where wxString s contains the string
         representation of a number. Must be called after bind(). */
        void renderNumber(wxString s, int x, int y);
        /** render this number at coordinates (x,y). Must be called after bind(). */
        void renderNumber(int i, int x, int y);
        /** render this number at coordinates (x,y). Must be called after bind(). */
        void renderNumber(float f, int x, int y);
    };
    
    typedef wxGLNumberRenderer AriaRenderNumber;
    
    /**
     @brief OpenGL render backend : text array renderer
     
     This class is useful to render a serie of strings that are usually rendered at the same time.
     It behaves exactly like wxGLString but is more efficient.
     
     
     Use example :
     
     \code
     wxGLStringArray my_messages();
     my_messages.addString("wxMac");
     my_messages.addString("wxGTK");
     my_messages.addString("wxMSW");
     ...
     if (first_render)
     my_messages.consolidate(&dc);
     
     glColor3f(0,0,0); // black text
     my_messages.bind();
     my_messages.get(0).render( x, y      );
     my_messages.get(1).render( x, y + 25 );
     my_messages.get(2).render( x, y + 50 );
     \endcode
     
     @ingroup renderers
     */
    class wxGLStringArray
    {
        ptr_vector<wxGLString, HOLD> strings;
        OwnerPtr<TextTexture> img;
        wxFont font;
        bool consolidated;
    public:
        /** constructs an empty array - add elements later using addString */
        wxGLStringArray();
        /** construct an array with 'strings_arg' elemnts in it */
        wxGLStringArray(const wxString strings_arg[], int amount);
        ~wxGLStringArray();
        
        /** get a sub-element - useful mainly for rendering, e.g. my_array.get(0).render(x, y); */
        wxGLString& get(const int id);
        
        /** call just before render() - binds the OpenGL. If you render the same string many
         times, or render from an array, bind only once, this will improve performance */
        void bind();
        
        /** add a string to the list for next consolidate() - has no
         immediate effect, you need to call consolidate() to get results  */
        void addString(wxString string);
        
        int getStringAmount() const { return strings.size(); }
        
        /** set how to draw string for next consolidate() - has no immediate effect,
         you need to call consolidate() to get results  */
        void setFont(wxFont font);
        
        /** consolidates the current string info into a GL string. call this after
         setting up strings, font and color (if necessary), and before rendering.
         The wxDC argument is only used to calculate text extents and will not be rendered on.  */
        void consolidate(wxDC* dc);
        
        LEAK_CHECK();
    };
    
    typedef wxGLStringArray AriaRenderArray;
    
}
#endif
#endif
