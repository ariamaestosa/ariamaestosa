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

#ifndef _measurebar_
#define _measurebar_

/*
 *  This class takes care of everything related to measures, including drawing and data converstion
 *  (measureAtPixel, measureAtTick, ...). Since it manages multiple time signature changes, it should
 *  always be used when measure info is necessary.
 */

#include "ptr_vector.h"
#include "LeakCheck.h"
#include "Editors/RelativeXCoord.h"
#include "Midi/TimeSigChange.h"
#include "irrXML/irrXML.h"

namespace AriaMaestosa {
	
	class GLPane;
	class MainFrame;
	
	class UnselectedMenu;
	class SelectedMenu;
	
#ifndef _measureinfo_
#define _measureinfo_
class MeasureInfo
{
public:
MeasureInfo() { selected = false, tick = 0, pixel = 0; endTick=-1; widthInTicks=-1; widthInPixels=-1;}
	
bool selected;
int tick, endTick;
int pixel, endPixel;
int widthInTicks, widthInPixels;
};
#endif

class MeasureBar
{
	MainFrame* mainFrame;
	GLPane* glPane;
	std::vector<MeasureInfo> measureInfo;
	int lastMeasureInDrag;
	bool somethingSelected;
	bool expandedMode;

	int measureBarY; // remember the latest value given by the GLPane
	
	int selectedTimeSig;
	int measureAmount;
	int firstMeasure;
	
		
	UnselectedMenu* unselectedMenu;
	SelectedMenu* selectedMenu;
	
	ptr_vector<TimeSigChange> timeSigChanges;
	
	DECLARE_LEAK_CHECK();
	
public:
	// read-only. Only access these in expanded mode otherwise they're empty
	int	totalNeededLengthInTicks;
	int totalNeededLengthInPixels;
		
	MeasureBar();
	~MeasureBar();
	
	int measureAtPixel(int pixel);
	int measureAtTick(int tick);
	int measureDivisionAt(int pixel);
	
	void render(int from_y);
	void mouseDown(int x, int y);
	void mouseDrag(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
    void mouseUp(int mousex_current, int mousey_current, int mousex_initial, int mousey_initial);
    void rightClick(int x, int y);
	
	int getTotalTickAmount();
	int getTotalPixelAmount();
	bool isMeasureLengthConstant();
	
	void setMeasureAmount(int measureAmount);
	int getMeasureAmount();
		
	float defaultMeasureLengthInPixels();
	int defaultMeasureLengthInTicks();
	
	int getFirstMeasure();
	void setFirstMeasure(int firstMeasureID);
	
	float measureLengthInPixels(int measure =-1);
	int measureLengthInTicks(int measure = -1);
	float beatLengthInPixels();
	int beatLengthInTicks();
	
    // get time sig num/denom, either for a specific mesure, either the default value (no argument)
	int getTimeSigNumerator(int measure=-1);
	int getTimeSigDenominator(int measure=-1);
    
	void setTimeSig(int num, int denom); 
	int getTimeSigAmount();
	TimeSigChange& getTimeSig(int id);
	void eraseTimeSig(int id);
	TimeSigChange* removeTimeSig(int id);
	
	void addTimeSigChange(int measure, int num, int denom);
	
	// used only while loading midi files, because midi files don't store in which measure it is. dont use for other uses.
	void addTimeSigChangeAtTick(int tick, int num, int denom);
		
	void selectTimeSig(const int id);
	
	int firstTickInMeasure(int id);
	int firstPixelInMeasure(int id);
	int lastTickInMeasure(int id);
		
	void setExpandedMode(bool expanded);
	bool isExpandedMode();	
	
	int getMeasureBarHeight();
	
	void updateMeasureInfo();
	void updateVector(int newSize);
	void unselect();
	
	void beforeImporting();
	void afterImporting();
	
	bool readFromFile(irr::io::IrrXMLReader* xml);
	void saveToFile(wxFileOutputStream& fileout);
};

}

#endif
