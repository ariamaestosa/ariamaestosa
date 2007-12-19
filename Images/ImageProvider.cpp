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

#include "Images/Image.h"
#include "Images/Drawable.h"
#include "Config.h"

#include "wx/wx.h"

namespace AriaMaestosa {
	
    // FIXME - merge Image and Drawable
    
	// note display
    Image* noteTrackImg;
    Drawable* noteTrackDrawable;
    
    // scrollbar
    Image* sbArrowImg;
    Image* sbArrowDownImg;
    Image* sbBackgImg;
    Image* sbBarImg;
    
    Drawable* sbArrowDrawable;
    Drawable* sbBackgDrawable;
    Drawable* sbBarDrawable;
    
    // track UI
    Image* cornerImg;
    Image* borderImg;
    Image* whiteCornerImg;
    Image* whiteBorderImg;
    
    Drawable* cornerDrawable;
    Drawable* borderDrawable;
    Drawable* whiteCornerDrawable;
    Drawable* whiteBorderDrawable;
    
    // collapseDrawable
    Image* collapseImg;
    Image* expandImg;
    Image* dockTrackImg;
    
    Drawable* collapseDrawable;
    Drawable* dockTrackDrawable;
    
    // mute
    Image* muteOnImg;
    Image* muteOffImg;
    
    Drawable* muteDrawable;
    
    // combo box
    Image* comboBorderImg;
    Image* comboBodyImg;
    Image* comboSelectImg;
    
    Drawable* comboBorderDrawable;
    Drawable* comboBodyDrawable;
    Drawable* comboSelectDrawable;
    
    // Editor modes
    Image* guitar_viewImg;
    Image* keyboard_viewImg;
    Image* score_viewImg;
    Image* drum_viewImg;
    Image* controller_viewImg;
    
    Drawable* guitar_view;
    Drawable* keyboard_view;
    Drawable* score_view;
    Drawable* drum_view;
    Drawable* controller_view;
    
    Image* tabBorderImg;
    Image* tabImg;
    
    Drawable* tabBorderDrawable;
    Drawable* tabDrawable;
	
	// score
	Image* keyGImg;
    Drawable* keyG;
	Image* keyFImg;
    Drawable* keyF;
	
	Image* noteClosedImg;
    Drawable* noteClosed;
	
	Image* noteOpenImg;
    Drawable* noteOpen;
	
	Image* noteTailImg;
    Drawable* noteTail;
	
	Image* sharpImg;
    Drawable* sharpSign;
	Image* flatImg;
    Drawable* flatSign;
	Image* naturalImg;
    Drawable* naturalSign;
	
	Image* silence4Img;
	Drawable* silence4;
	Image* silence8Img;
	Drawable* silence8;
	
namespace ImageProvider {
	
bool images_loaded = false;
	
void loadImages()
{
	
	noteTrackImg=new Image(wxT("notetrack.png"));
	sbArrowImg=new Image(wxT("sb_arrow.png"));
	sbArrowDownImg=new Image(wxT("sb_arrow_sel.png"));
	sbBackgImg=new Image(wxT("sb_backg.png"));
	sbBarImg=new Image(wxT("sb_thumb.tga"));
	
	noteTrackDrawable=new Drawable(noteTrackImg);
	sbArrowDrawable=new Drawable(sbArrowImg);
	sbBackgDrawable=new Drawable(sbBackgImg);
	sbBarDrawable=new Drawable(sbBarImg);
	
	cornerImg=new Image(wxT("corner.jpg"));
	borderImg=new Image(wxT("border.jpg"));
	whiteCornerImg=new Image(wxT("white_corner.png"));
	whiteBorderImg=new Image(wxT("white_border.png"));
	collapseImg=new Image(wxT("collapse.jpg"));
	expandImg=new Image(wxT("expand.jpg"));
	dockTrackImg=new Image(wxT("dock.png"));
	muteOnImg=new Image(wxT("mute_on.png"));
	muteOffImg=new Image(wxT("mute_off.png"));
	comboBorderImg=new Image(wxT("combo_side.png"));
	comboBodyImg=new Image(wxT("combo_body.png"));
	comboSelectImg=new Image(wxT("combo_select.png"));
	
	cornerDrawable=new Drawable(cornerImg);
	borderDrawable=new Drawable(borderImg);
	whiteCornerDrawable=new Drawable(whiteCornerImg);
	whiteBorderDrawable=new Drawable( whiteBorderImg);
	dockTrackDrawable=new Drawable( dockTrackImg);
	collapseDrawable=new Drawable( collapseImg);
	muteDrawable=new Drawable( muteOffImg);
	comboBorderDrawable=new Drawable( comboBorderImg);
	comboBodyDrawable=new Drawable( comboBodyImg);
	comboSelectDrawable=new Drawable( comboSelectImg);
	
	guitar_viewImg=new Image(wxT("guitar_view.jpg"));
	keyboard_viewImg=new Image(wxT("keyboard_view.jpg"));
	score_viewImg=new Image(wxT("score_view.jpg"));
	drum_viewImg=new Image(wxT("drum_view.png"));
	controller_viewImg=new Image(wxT("controller_view.jpg"));
	
	guitar_view=new Drawable( guitar_viewImg);
	keyboard_view=new Drawable( keyboard_viewImg);
	score_view=new Drawable( score_viewImg);
	drum_view=new Drawable( drum_viewImg);
	controller_view=new Drawable( controller_viewImg);
	
	tabImg=new Image(wxT("tab_body.png"));
	tabBorderImg=new Image(wxT("tab_side.png"));
	tabDrawable=new Drawable( tabImg);
	tabBorderDrawable=new Drawable( tabBorderImg);
	
	keyGImg=new Image(wxT("score/keyG.png"));
	keyG=new Drawable(keyGImg);
	keyG -> setHotspot( 15, 33 );
		
	keyFImg=new Image(wxT("score/FKey.png"));
	keyF=new Drawable(keyFImg);
	keyF -> setHotspot( 7, 13 );
	
	noteClosedImg = new Image(wxT("score/noteclosed.png"));
    noteClosed = new Drawable(noteClosedImg);
	
	noteOpenImg = new Image(wxT("score/noteopen.png"));
    noteOpen = new Drawable(noteOpenImg);
	
	noteTailImg = new Image(wxT("score/notetail.png"));
    noteTail = new Drawable(noteTailImg);
	
	sharpImg = new Image(wxT("score/sharp.png"));
    sharpSign = new Drawable(sharpImg);
	sharpSign -> setHotspot( sharpImg->width/2, sharpImg->height/2 );
	
	naturalImg = new Image(wxT("score/natural.png"));
    naturalSign = new Drawable(naturalImg);
	naturalSign -> setHotspot( naturalImg->width/2, naturalImg->height/2 );
	
	flatImg = new Image(wxT("score/flat.png"));
    flatSign = new Drawable(flatImg);
	flatSign -> setHotspot( 2, 12 );
	
	silence4Img = new Image(wxT("score/silence4.png"));
    silence4 = new Drawable(silence4Img);
	silence8Img = new Image(wxT("score/silence8.png"));
    silence8 = new Drawable(silence8Img);
	
	images_loaded = true;
}

void unloadImages()
{
	
	if(!images_loaded) return;
	images_loaded = false;

	delete noteTrackImg; noteTrackImg=NULL;
	delete sbArrowImg;
	delete sbArrowDownImg;
	delete sbBackgImg;
	delete sbBarImg;
	
	delete keyGImg;
	delete keyG;
	
	delete noteTrackDrawable;
	delete sbArrowDrawable;
	delete sbBackgDrawable;
	delete sbBarDrawable;
	
	delete cornerImg;
	delete borderImg;
	delete whiteCornerImg;
	delete whiteBorderImg;
	delete collapseImg;
	delete expandImg;
	delete muteOnImg;
	delete muteOffImg;
	delete comboBorderImg;
	delete comboBodyImg;
	delete comboSelectImg;
	delete dockTrackImg;
	
	delete tabImg;
	delete tabBorderImg;
	delete tabDrawable;
	delete tabBorderDrawable;
	
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
	
	delete guitar_viewImg;
	delete keyboard_viewImg;
	delete score_viewImg;
	delete controller_viewImg;
	delete drum_viewImg;
	
	delete guitar_view;
	delete drum_view;
	delete keyboard_view;
	delete score_view;
	delete controller_view;
	
	
	delete noteClosedImg;
    delete noteClosed;
	delete noteOpenImg;
    delete noteOpen;
	delete noteTailImg;
    delete noteTail;
	
	delete sharpImg;
    delete sharpSign;
	delete flatImg;
    delete flatSign;
	delete naturalImg;
    delete naturalSign;
	
	delete keyFImg;
    delete keyF;
	
	delete silence4Img;
    delete silence4;
	delete silence8Img;
    delete silence8;
	
#ifdef _MORE_DEBUG_CHECKS
	std::cout << "~ImageProvider END" << std::endl;
#endif
	
}

bool imagesLoaded()
{
	return images_loaded;  
}

}

}
