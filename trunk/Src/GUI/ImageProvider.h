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

#ifndef _imageprovider_
#define _imageprovider_

#ifdef _DECLARE_IMAGES_
#define IMG_DEC
#else
#define IMG_DEC extern
#endif

namespace AriaMaestosa
{

    class Drawable;
    class Image;

    /** 
      * @addtogroup gui
      * @{
      */

    // note display
    IMG_DEC Drawable* noteTrackDrawable;

    // scrollbar
    IMG_DEC Image* sbArrowImg;
    IMG_DEC Image* sbArrowDownImg;
    IMG_DEC Drawable* sbArrowDrawable;
    IMG_DEC Drawable* sbBackgDrawable;
    IMG_DEC Drawable* sbThumbDrawable;

    // track UI
    IMG_DEC Drawable* cornerDrawable;
    IMG_DEC Drawable* borderDrawable;
    IMG_DEC Drawable* whiteCornerDrawable;
    IMG_DEC Drawable* whiteBorderDrawable;

    // collapse
    IMG_DEC Image* collapseImg;
    IMG_DEC Image* expandImg;

    IMG_DEC Drawable* collapseDrawable;

    // dock
    IMG_DEC Drawable* dockTrackDrawable;
    IMG_DEC Drawable* maximizeTrackDrawable;
        
    // mute
    IMG_DEC Image* muteOnImg;
    IMG_DEC Image* muteOffImg;

    IMG_DEC Drawable* muteDrawable;

    // combo box
    IMG_DEC Drawable* comboBorderDrawable;
    IMG_DEC Drawable* comboBodyDrawable;
    IMG_DEC Drawable* comboSelectDrawable;

    // Editor modes
    IMG_DEC Drawable* guitar_view;
    IMG_DEC Drawable* keyboard_view;
    IMG_DEC Drawable* score_view;
    IMG_DEC Drawable* drum_view;
    IMG_DEC Drawable* controller_view;

    // document tabs
    IMG_DEC Drawable* tabBorderDrawable;
    IMG_DEC Drawable* tabDrawable;
    IMG_DEC Drawable* tabCloseDrawable;

    // score
    IMG_DEC Drawable* clefG_drawable;
    IMG_DEC Drawable* clefF_drawable;
    IMG_DEC Drawable* noteClosed;
    IMG_DEC Drawable* noteOpen;
    IMG_DEC Drawable* noteFlag;
    IMG_DEC Drawable* sharpSign;
    IMG_DEC Drawable* flatSign;
    IMG_DEC Drawable* naturalSign;
    IMG_DEC Drawable* silence4;
    IMG_DEC Drawable* silence8;

    IMG_DEC Drawable* mgrid_1;
    IMG_DEC Drawable* mgrid_2;
    IMG_DEC Drawable* mgrid_4;
    IMG_DEC Drawable* mgrid_8;
    IMG_DEC Drawable* mgrid_16;
    IMG_DEC Drawable* mgrid_32;
    IMG_DEC Drawable* mgrid_triplet;
    IMG_DEC Drawable* mgrid_dotted;

    IMG_DEC Drawable* menu_new;
    IMG_DEC Drawable* menu_open;
    IMG_DEC Drawable* menu_import;
    IMG_DEC Drawable* menu_configure;
    IMG_DEC Drawable* menu_help;
    IMG_DEC Drawable* menu_exit;

    /** @} */

    /**
      * @ingroup gui
      */
    namespace ImageProvider
    {
        void loadImages();
        void unloadImages();
        bool imagesLoaded();
    }

}

#endif
