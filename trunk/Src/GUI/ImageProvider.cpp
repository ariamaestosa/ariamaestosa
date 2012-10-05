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

#include "Renderers/ImageBase.h"
#include "Renderers/Drawable.h"
#include "Utils.h"

#include <wx/string.h>

#define _DECLARE_IMAGES_
#include "GUI/ImageProvider.h"
#undef _DECLARE_IMAGES_

namespace AriaMaestosa
{
    
    namespace ImageProvider
    {
        
        bool images_loaded = false;
        
        void loadImages()
        {
            // scrollbar
#ifdef __WXMSW__
            sbArrowImg      = new Image(wxT("sb_arrow_win.png"));
            sbArrowDownImg  = new Image(wxT("sb_arrow_sel_win.png"));
            sbArrowDrawable = new Drawable(sbArrowImg);
            sbBackgDrawable = new Drawable(wxT("sb_backg_win.png"));
            sbThumbDrawable = new Drawable(wxT("sb_thumb_win.png"));
#else
            sbArrowImg      = new Image(wxT("sb_arrow.png"));
            sbArrowDownImg  = new Image(wxT("sb_arrow_sel.png"));
            sbArrowDrawable = new Drawable(sbArrowImg);
            sbBackgDrawable = new Drawable(wxT("sb_backg.png"));
            sbThumbDrawable = new Drawable(wxT("sb_thumb.png"));
#endif

    
            // track header buttons
            collapseImg      = new Image(wxT("collapse.jpg"));
            expandImg        = new Image(wxT("expand.jpg"));
            collapseDrawable = new Drawable(collapseImg);
            
            // volume
            volumeLowImg    = new Image(wxT("volume_low.png"));
            volumeMediumImg   = new Image(wxT("volume_medium.png"));
            volumeHighImg   = new Image(wxT("volume_high.png"));
            volumeDrawable = new Drawable(volumeHighImg); 
            
            // mute
            muteOnImg    = new Image(wxT("led_red.png"));
            muteOffImg   = new Image(wxT("led_off.png"));
            muteDrawable = new Drawable(muteOffImg);
            
            // solo
            soloOnImg    = new Image(wxT("led_green.png"));
            soloOffImg   = new Image(wxT("led_off.png"));
            soloDrawable = new Drawable(soloOffImg);
            
            
            dockTrackDrawable     = new Drawable( wxT("dock.png"));
            maximizeTrackDrawable = new Drawable( wxT("maximize.png")  );
            
            guitar_view     = new Drawable( wxT("guitar_view.jpg"));
            keyboard_view   = new Drawable( wxT("keyboard_view.jpg"));
            score_view      = new Drawable( wxT("score_view.jpg"));
            drum_view       = new Drawable( wxT("drum_view.png"));
            controller_view = new Drawable( wxT("controller_view.jpg"));
            
            // track borders
            cornerDrawable      = new Drawable(wxT("corner.jpg"));
            borderDrawable      = new Drawable(wxT("border.jpg"));
            whiteCornerDrawable = new Drawable(wxT("white_corner.png"));
            whiteBorderDrawable = new Drawable( wxT("white_border.png"));
            
            // combo
            comboBorderDrawable = new Drawable( wxT("combo_side.png"));
            comboBodyDrawable   = new Drawable( wxT("combo_body.png"));
            comboSelectDrawable = new Drawable( wxT("combo_select.png"));
            
            // document tabs
            tabDrawable       = new Drawable( wxT("tab_body.png") );
            tabBorderDrawable = new Drawable( wxT("tab_side.png") );
            tabCloseDrawable  = new Drawable( wxT("close.png") );
            
            // score
            clefG_drawable = new Drawable(wxT("score/keyG.png"));
            clefG_drawable->setHotspot( 15, 33 );
            
            clefF_drawable = new Drawable(wxT("score/FKey.png"));
            clefF_drawable->setHotspot( 7, 13 );
            
            noteClosed = new Drawable(wxT("score/noteclosed.png"));
            noteOpen   = new Drawable(wxT("score/noteopen.png"));
            noteFlag   = new Drawable(wxT("score/notetail.png"));
            sharpSign  = new Drawable(wxT("score/sharp.png"));
            sharpSign->setHotspot( sharpSign->getImageWidth()/2, sharpSign->getImageHeight()/2 );
            
            naturalSign = new Drawable(wxT("score/natural.png"));
            naturalSign->setHotspot( naturalSign->getImageWidth()/2, naturalSign->getImageHeight()/2 );
            
            flatSign = new Drawable(wxT("score/flat.png"));
            flatSign->setHotspot( 2, 12 );
            
            silence4 = new Drawable(wxT("score/silence4.png"));
            silence8 = new Drawable(wxT("score/silence8.png"));
            
            mgrid_1  = new Drawable(wxT("score/mgrid_1.png"));
            mgrid_2  = new Drawable(wxT("score/mgrid_2.png"));
            mgrid_4  = new Drawable(wxT("score/mgrid_4.png"));
            mgrid_8  = new Drawable(wxT("score/mgrid_8.png"));
            mgrid_16 = new Drawable(wxT("score/mgrid_16.png"));
            mgrid_32 = new Drawable(wxT("score/mgrid_32.png"));
            mgrid_triplet = new Drawable(wxT("score/mgrid_triplet.png"));
            mgrid_dotted  = new Drawable(wxT("score/mgrid_dotted.png"));

            menu_new       = new Drawable(wxT("new.png"));
            menu_open      = new Drawable(wxT("open.png"));
            menu_import    = new Drawable(wxT("import.png"));
            menu_configure = new Drawable(wxT("configure.png"));
            menu_help      = new Drawable(wxT("help.png"));
            menu_exit      = new Drawable(wxT("exit.png"));
            
            images_loaded = true;
        }
        
        void unloadImages()
        {
            
            if (!images_loaded) return;
            images_loaded = false;
            
            delete sbArrowImg;
            delete sbArrowDownImg;
            
    
            delete sbArrowDrawable;
            delete sbBackgDrawable;
            delete sbThumbDrawable;
            
            delete collapseImg;
            delete expandImg;
            
            delete muteOnImg;
            delete muteOffImg;
            
            delete tabDrawable;
            delete tabBorderDrawable;
            delete tabCloseDrawable;
            
            delete cornerDrawable;
            delete borderDrawable;
            delete whiteCornerDrawable;
            delete whiteBorderDrawable;
            delete collapseDrawable;
            delete muteDrawable;
            delete comboBorderDrawable;
            delete comboBodyDrawable;
            delete comboSelectDrawable;
            delete dockTrackDrawable;
            delete maximizeTrackDrawable;
            
            delete guitar_view;
            delete drum_view;
            delete keyboard_view;
            delete score_view;
            delete controller_view;
            
            delete noteClosed;
            delete noteOpen;
            delete noteFlag;
            
            delete sharpSign;
            delete flatSign;
            delete naturalSign;
            
            delete clefF_drawable;
            delete clefG_drawable;
            
            delete silence4;
            delete silence8;
            
            delete mgrid_1;
            delete mgrid_2;
            delete mgrid_4;
            delete mgrid_8;
            delete mgrid_16;
            delete mgrid_32;
            delete mgrid_triplet;
            delete mgrid_dotted;

            delete menu_new;
            delete menu_open;
            delete menu_import;
            delete menu_configure;
            delete menu_help;
            delete menu_exit;
        }
        
        bool imagesLoaded()
        {
            return images_loaded;
        }
        
    }
    
}
