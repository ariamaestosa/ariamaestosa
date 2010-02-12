#ifdef RENDERER_OPENGL

#include "GLwxString.h"
#include "Config.h"

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else
#include <GL/gl.h>
#endif

#include "wx/wx.h"
#include "wx/tokenzr.h"
#include "AriaCore.h"

namespace AriaMaestosa
{

GLuint* loadImage(wxImage* img)
{
	GLuint* ID = new GLuint[1];
	glGenTextures( 1, ID );

	glBindTexture( GL_TEXTURE_2D, *ID );


	glPixelStorei(GL_UNPACK_ALIGNMENT,   1   );

    const int w = img->GetWidth(), h = img->GetHeight();


    // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB
    // and we want the Alpha channel. Furthermore, the current rendering is black-on-white, we'll
    // convert it to an alpha channel by the way (not all platforms support transparency in wxDCs
    // so it's the easiest way to go)
    GLubyte *bitmapData=img->GetData();
    GLubyte *imageData;

    int bytesPerPixel = 4;

    int imageSize = w * h * bytesPerPixel;
    imageData=(GLubyte *)malloc(imageSize);

    int rev_val=h-1;

    for(int y=0; y<h; y++)
    {
        for(int x=0; x<w; x++)
        {
            imageData[(x+y*w)*bytesPerPixel+0] = 255;
            imageData[(x+y*w)*bytesPerPixel+1] = 255;
            imageData[(x+y*w)*bytesPerPixel+2] = 255;

            // alpha
            imageData[(x+y*w)*bytesPerPixel+3] = 255 - bitmapData[( x+(rev_val-y)*w)*3];
        }//next
    }//next

    glTexImage2D(GL_TEXTURE_2D, 0, bytesPerPixel, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    free(imageData);

	// set texture parameters as you wish
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return ID;

}

#if 0
#pragma mark -
#pragma mark TextGLDrawable implementation
#endif


TextGLDrawable::TextGLDrawable(TextTexture* image_arg)
{
    x=0;
    y=0;
    angle=0;

    xscale=1;
    yscale=1;

    y_offset = 0;
    
    max_width = -1;
    
    xflip=false;
    yflip=false;
    
    w = -1;
    h = -1;

    if (image_arg != NULL) setImage(image_arg);
    else image = NULL;

    tex_coord_x1 = 0;
    tex_coord_y1 = 1;
    tex_coord_x2 = 1;
    tex_coord_y2 = 0;
}

void TextGLDrawable::setFlip(bool x, bool y)
{
    xflip=x;
    yflip=y;
}

void TextGLDrawable::setMaxWidth(const int w)
{
    max_width = w;
}

void TextGLDrawable::move(int x, int y)
{
    TextGLDrawable::x=x;
    TextGLDrawable::y=y;
}

void TextGLDrawable::scale(float x, float y)
{
    TextGLDrawable::xscale=x;
    TextGLDrawable::yscale=y;
}

void TextGLDrawable::scale(float k)
{
    TextGLDrawable::xscale=k;
    TextGLDrawable::yscale=k;
}

void TextGLDrawable::setImage(TextTexture* image, bool giveUpOwnership)
{
    TextGLDrawable::image = image;
    
    if (giveUpOwnership)
    {
        TextGLDrawable::image.owner = false;
    }
}

void TextGLDrawable::rotate(int angle)
{
    TextGLDrawable::angle=angle;
}

void TextGLDrawable::render()
{
    assert(image != NULL);

    assertExpr(w, >, 0);
    assertExpr(h, >, 0);
    assertExpr(w, <, 90000);
    assertExpr(h, <, 90000);
    glPushMatrix();
    glTranslatef(x*10,(y-h-y_offset)*10,0);
    
    if (xscale!=1 || yscale!=1) glScalef(xscale, yscale, 1);
    if (angle!=0) glRotatef(angle, 0,0,1);

    if (max_width != -1 and getWidth()>max_width)
    {
        const float ratio = (float)max_width/(float)getWidth();
        glBegin(GL_QUADS);
        
        glTexCoord2f(xflip? tex_coord_x2*ratio : tex_coord_x1,
                     yflip? tex_coord_y2 : tex_coord_y1);
        glVertex2f( 0, 0 );
        
        glTexCoord2f(xflip? tex_coord_x1 : tex_coord_x2*ratio,
                     yflip? tex_coord_y2 : tex_coord_y1);
        glVertex2f( max_width*10, 0 );
        
        glTexCoord2f(xflip? tex_coord_x1 : tex_coord_x2*ratio,
                     yflip? tex_coord_y1 : tex_coord_y2);
        glVertex2f( max_width*10, h*10 );
        
        glTexCoord2f(xflip? tex_coord_x2*ratio : tex_coord_x1,
                     yflip? tex_coord_y1 : tex_coord_y2);
        glVertex2f( 0, h*10 );
    }
    else
    {
        glBegin(GL_QUADS);

        glTexCoord2f(xflip? tex_coord_x2 : tex_coord_x1,
                     yflip? tex_coord_y2 : tex_coord_y1);
        glVertex2f( 0, 0 );

        glTexCoord2f(xflip? tex_coord_x1 : tex_coord_x2,
                     yflip? tex_coord_y2 : tex_coord_y1);
        glVertex2f( w*10, 0 );

        glTexCoord2f(xflip? tex_coord_x1 : tex_coord_x2,
                     yflip? tex_coord_y1 : tex_coord_y2);
        glVertex2f( w*10, h*10 );

        glTexCoord2f(xflip? tex_coord_x2 : tex_coord_x1,
                     yflip? tex_coord_y1 : tex_coord_y2);
        glVertex2f( 0, h*10 );
    }
    glEnd();
    glPopMatrix();
}

#if 0
#pragma mark -
#pragma mark TextTexture implementation
#endif

TextTexture::TextTexture()
{
    ID = NULL;
}

TextTexture::TextTexture(wxBitmap& bmp)
{
    wxImage img = bmp.ConvertToImage();
    load(&img);
}
void TextTexture::load(wxImage* img)
{
    ID = loadImage(img);
}

GLuint* TextTexture::getID()
{
    assert(ID != NULL);
    return ID;
}

TextTexture::~TextTexture()
{
    glDeleteTextures (1, ID);
    delete[] ID;
}


#if 0
#pragma mark -
#pragma mark wxGLString implementation
#endif

wxGLString::wxGLString() : wxString(wxT("")), TextGLDrawable()
{
    consolidated = false;
    warp_after = -1;
}
wxGLString::wxGLString(wxString message) : wxString(message), TextGLDrawable()
{
    consolidated = false;
    warp_after = -1;
}
void wxGLString::set(const wxString& string)
{
    (*((wxString*)this))=string;
    consolidated=false;
}

void wxGLString::bind()
{
    if (not consolidated) consolidate(Display::renderDC);
    
    glBindTexture(GL_TEXTURE_2D, image->getID()[0] );
}
void wxGLString::calculateSize(wxDC* dc, const bool ignore_font /* when from array */)
{
    if (!ignore_font)
    {
        if (font.IsOk()) dc->SetFont(font);
        else dc->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));
    }

    dc->GetTextExtent(*this, &w, &h);
}

void wxGLString::consolidate(wxDC* dc)
{
    TextGLDrawable::y_offset = 0;
    calculateSize(dc);

    bool multi_line = false;
    int single_line_height = 0;
    if (warp_after != -1 and w > warp_after)
    {
        Replace(wxT(" "),wxT("\n"));
        Replace(wxT("/"),wxT("/\n"));
        single_line_height = h;
        dc->GetMultiLineTextExtent(*this, &w, &h);
        std::cout << "new size: " << w << ", " << h << std::endl;
        multi_line = true;
    }
    
    const int power_of_2_w = std::max(32, (int)pow( 2, (int)ceil((float)log(w)/log(2.0)) ));
    const int power_of_2_h = std::max(32, (int)pow( 2, (int)ceil((float)log(h)/log(2.0)) ));

    wxBitmap bmp(power_of_2_w, power_of_2_h);
    assert(bmp.IsOk());

    {
        wxMemoryDC temp_dc(bmp);

        temp_dc.SetBrush(*wxWHITE_BRUSH);
        temp_dc.Clear();

        if (font.IsOk()) temp_dc.SetFont(font);
        else temp_dc.SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));

        if (multi_line)
        {
            int y = 0;
            wxStringTokenizer tkz(*this, wxT("\n"));
            while ( tkz.HasMoreTokens() )
            {
                wxString token = tkz.GetNextToken();
                temp_dc.DrawText(token, 0, y);
                y += single_line_height;
            }
            TextGLDrawable::y_offset = -y +  single_line_height;
        }
        else
            temp_dc.DrawText(*this, 0, 0);
    }
    TextTexture* img = new TextTexture(bmp);

    bmp.SaveFile(wxT("/tmp/test.png"), wxBITMAP_TYPE_PNG);
    
    TextGLDrawable::texw = power_of_2_w;
    TextGLDrawable::texh = power_of_2_h;
    TextGLDrawable::tex_coord_x2 = (float)w / (float)power_of_2_w;
    TextGLDrawable::tex_coord_y2 = 1-(float)h / (float)power_of_2_h;
    TextGLDrawable::tex_coord_y1 = 1;

    TextGLDrawable::setImage(img);

    consolidated = true;
}
    
void wxGLString::consolidateFromArray(wxDC* dc, int x, int y)
{
    dc->DrawText(*this, x, y);
    consolidated = true;
}

void wxGLString::setFont(wxFont font)
{
    wxGLString::font = font;
}

void wxGLString::render(const int x, const int y)
{
    TextGLDrawable::move(x, y);
    TextGLDrawable::render();
}
    
void wxGLString::setMaxWidth(const int w, const bool warp /*false: truncate. true: warp.*/)
{
   // std::cout << "wxGLString::setMaxWidth " << w << ", " << warp << std::endl;
    if (not warp)
        TextGLDrawable::setMaxWidth(w);
    else
    {
        warp_after = w;
    }
}
 
wxGLString::~wxGLString()
{
}


#if 0
#pragma mark -
#pragma mark wxGLNumberRenderer implementation
#endif

wxGLNumberRenderer::wxGLNumberRenderer() : wxGLString( wxT("0 1 2 3 4 5 6 7 8 9 . - ") )
{
    number_location = new int[13];
#ifdef __WXGTK__
    setFont( wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, /*wxFONTWEIGHT_BOLD*/ wxFONTWEIGHT_NORMAL) );
#endif
    space_w = -1;
}
wxGLNumberRenderer::~wxGLNumberRenderer()
{
    delete[] number_location;
}

void wxGLNumberRenderer::consolidate(wxDC* dc)
{
    wxGLString::consolidate(dc);

    if (font.IsOk()) dc->SetFont(font);
    else dc->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));

    number_location[0] = 0;
    number_location[1]  = dc->GetTextExtent(wxT("0 ")).GetWidth();
    number_location[2]  = dc->GetTextExtent(wxT("0 1 ")).GetWidth();
    number_location[3]  = dc->GetTextExtent(wxT("0 1 2 ")).GetWidth();
    number_location[4]  = dc->GetTextExtent(wxT("0 1 2 3 ")).GetWidth();
    number_location[5]  = dc->GetTextExtent(wxT("0 1 2 3 4 ")).GetWidth();
    number_location[6]  = dc->GetTextExtent(wxT("0 1 2 3 4 5 ")).GetWidth();
    number_location[7]  = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 ")).GetWidth();
    number_location[8]  = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 7 ")).GetWidth();
    number_location[9]  = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 7 8 ")).GetWidth();
    number_location[10] = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 7 8 9 ")).GetWidth();
    number_location[11] = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 7 8 9 . ")).GetWidth();
    number_location[12] = dc->GetTextExtent(wxT("0 1 2 3 4 5 6 7 8 9 . - ")).GetWidth();

    space_w = dc->GetTextExtent(wxT(" ")).GetWidth();
    consolidated = true;
}
void wxGLNumberRenderer::renderNumber(int i, int x, int y)
{
    wxString s;
    s << i;
    renderNumber(s, x, y);
}
void wxGLNumberRenderer::renderNumber(float f, int x, int y)
{
    wxString s;
    s << f;
    renderNumber(s, x, y);
}
void wxGLNumberRenderer::renderNumber(wxString s, int x, int y)
{
    assertExpr(space_w, >=, 0);
    assertExpr(space_w, <, 90000);
    
    const int full_string_w = TextGLDrawable::texw;

    const int char_amount = s.Length();
    for(int c=0; c<char_amount; c++)
    {
        int charid = -1;

        char schar = s[c];
        switch(schar)
        {
            case '0' : charid = 0; break;
            case '1' : charid = 1; break;
            case '2' : charid = 2; break;
            case '3' : charid = 3; break;
            case '4' : charid = 4; break;
            case '5' : charid = 5; break;
            case '6' : charid = 6; break;
            case '7' : charid = 7; break;
            case '8' : charid = 8; break;
            case '9' : charid = 9; break;
            case '.' :
            case ',' : charid = 10; break;
            case '-' : charid = 11; break;
            default: printf("Warning: character %c unexpected in number!\n", schar); continue;
        }

        assert( charid != -1 );

        TextGLDrawable::tex_coord_x1 = (float)number_location[charid] / (float)full_string_w;
        TextGLDrawable::tex_coord_x2 = (float)(number_location[charid+1]-space_w) / (float)full_string_w;

        const int char_width = number_location[charid+1] - number_location[charid] - space_w;
        TextGLDrawable::w = char_width;


        TextGLDrawable::move(x, y);
        TextGLDrawable::render();

        x += char_width;
    } // next

   // TextGLDrawable::w = full_string_w;
}

#if 0
#pragma mark -
#pragma mark wxGLStringArray implementation
#endif

wxGLStringArray::wxGLStringArray()
{
    img = NULL;
    consolidated = false;
}
wxGLStringArray::wxGLStringArray(const wxString strings_arg[], int amount)
{
    img = NULL;
    consolidated = false;

    for(int n=0; n<amount; n++)
        strings.push_back( new wxGLString(strings_arg[n]) );
}
    
wxGLStringArray::~wxGLStringArray()
{
}

wxGLString& wxGLStringArray::get(const int id)
{
    return strings[id];
}
void wxGLStringArray::bind()
{
    if (not consolidated) consolidate(Display::renderDC);

    glBindTexture(GL_TEXTURE_2D, img->getID()[0] );
}
void wxGLStringArray::addString(wxString string)
{
    strings.push_back( new wxGLString(string) );
}
void wxGLStringArray::setFont(wxFont font)
{
    wxGLStringArray::font = font;
}

void wxGLStringArray::consolidate(wxDC* dc)
{
    int x=0, y=0;

    if (font.IsOk()) dc->SetFont(font);
    else dc->SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));

    // find how much space we need
    int longest_string = 0;

    const int amount = strings.size();
    for(int n=0; n<amount; n++)
    {
        if (strings[n].IsEmpty()) continue;
        
        strings[n].calculateSize(dc, true);
        y += strings[n].h;
        if (strings[n].w > longest_string) longest_string = strings[n].w;
    }//next

    const int average_string_height = y / amount;

    // split in multiple columns if necessary
    int column_amount = 1;
    while (amount/column_amount > 30 and column_amount<10)
        column_amount ++;

    const int power_of_2_w = pow( 2, (int)ceil((float)log(longest_string*column_amount)/log(2.0)) );
    const int power_of_2_h = pow( 2, (int)ceil((float)log(y/column_amount)/log(2.0)) );

    //std::cout << "bitmap size : " <<  power_of_2_w << ", " << power_of_2_h << " // " << column_amount << " columns" << std::endl;

    wxBitmap bmp(power_of_2_w, power_of_2_h);
    assert(bmp.IsOk());

    {
        wxMemoryDC temp_dc(bmp);

        temp_dc.SetBrush(*wxWHITE_BRUSH);
        temp_dc.Clear();

        y = 0;
        x = 0;
        if (font.IsOk()) temp_dc.SetFont(font);
        else temp_dc.SetFont(wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT));

        for(int n=0; n<amount; n++)
        {
            if (strings[n].IsEmpty()) continue;
            strings[n].consolidateFromArray(&temp_dc, x, y);

            strings[n].tex_coord_x1 = (float)x/(float)power_of_2_w;
            strings[n].tex_coord_y1 = 1.0 - (float)y/(float)power_of_2_h;
            strings[n].tex_coord_x2 = (float)(x+strings[n].w)/(float)power_of_2_w;
            strings[n].tex_coord_y2 = 1.0 - (float)(y+strings[n].h)/(float)power_of_2_h;

            y += strings[n].h;
            if (y > power_of_2_h - average_string_height*2) // check if we need to switch to next column
            {
                y = 0;
                x += longest_string;
            }
        }
    }

    img = new TextTexture(bmp);

    for(int n=0; n<amount; n++)
    {
        strings[n].setImage(img, true);
    }

    consolidated = true;
}


}

#endif
