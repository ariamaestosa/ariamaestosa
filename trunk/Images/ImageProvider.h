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

namespace AriaMaestosa {
	
class Image; // forward
class Drawable;

// note display
extern Image* noteTrackImg;
extern Drawable* noteTrackDrawable;

// scrollbar
extern Image* sbArrowImg;
extern Image* sbArrowDownImg;
extern Image* sbBackgImg;
extern Image* sbBarImg;

extern Drawable* sbArrowDrawable;
extern Drawable* sbBackgDrawable;
extern Drawable* sbBarDrawable;

// track UI
extern Image* cornerImg;
extern Image* borderImg;
extern Image* whiteCornerImg;
extern Image* whiteBorderImg;

extern Drawable* cornerDrawable;
extern Drawable* borderDrawable;
extern Drawable* whiteCornerDrawable;
extern Drawable* whiteBorderDrawable;

// collapseDrawable
extern Image* collapseImg;
extern Image* expandImg;
extern Image* dockTrackImg;

extern Drawable* collapseDrawable;
extern Drawable* dockTrackDrawable;

// mute
extern Image* muteOnImg;
extern Image* muteOffImg;

extern Drawable* muteDrawable;

// combo box
extern Image* comboBorderImg;
extern Image* comboBodyImg;
extern Image* comboSelectImg;

extern Drawable* comboBorderDrawable;
extern Drawable* comboBodyDrawable;
extern Drawable* comboSelectDrawable;

// Editor modes
extern Image* guitar_viewImg;
extern Image* keyboard_viewImg;
extern Image* score_viewImg;
extern Image* drum_viewImg;
extern Image* controller_viewImg;

extern Drawable* guitar_view;
extern Drawable* keyboard_view;
extern Drawable* score_view;
extern Drawable* drum_view;
extern Drawable* controller_view;

extern Image* tabBorderImg;
extern Image* tabImg;

extern Drawable* tabBorderDrawable;
extern Drawable* tabDrawable;

// score
extern Image* keyGImg;
extern Drawable* keyG;
extern Image* keyFImg;
extern Drawable* keyF;

extern Image* noteClosedImg;
extern Drawable* noteClosed;

extern Image* noteOpenImg;
extern Drawable* noteOpen;

extern Image* noteTailImg;
extern Drawable* noteTail;

extern Image* sharpImg;
extern Drawable* sharpSign;
extern Image* flatImg;
extern Drawable* flatSign;
extern Image* naturalImg;
extern Drawable* naturalSign;

extern Image* silence4Img;
extern Drawable* silence4;
extern Image* silence8Img;
extern Drawable* silence8;

namespace ImageProvider {
    
    void loadImages();
    void unloadImages();
    bool imagesLoaded();
}


}

#endif
