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

#ifndef _measuredata_h_
#define _measuredata_h_

/*
 *  This class takes care of everything related to measure data.
 *  Since it manages multiple time signature changes, it should
 *  always be used when measure info is necessary.
 *  It is closely related to MeasureBar, its graphical counterpart
 */

#include "ptr_vector.h"
#include "Config.h"
#include "Editors/RelativeXCoord.h"
#include "Midi/TimeSigChange.h"
#include "irrXML/irrXML.h"

namespace AriaMaestosa {

    class MainFrame;
    class MeasureBar;

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

class MeasureData
{
    friend class MeasureBar;

    // contains one item for each measure in the sequence
    std::vector<MeasureInfo> measureInfo;

    int measureAmount;
    int firstMeasure;

    bool expandedMode;

    // contains one item for each time signature change event
    ptr_vector<TimeSigChange> timeSigChanges;

    bool somethingSelected;
    int selectedTimeSig;

    // Only access this in expanded mode otherwise they're empty
    int    totalNeededLengthInTicks;
    int totalNeededLengthInPixels;
public:

    LEAK_CHECK();

    OwnerPtr<MeasureBar>  graphics;

    MeasureData();
    ~MeasureData();

    void setExpandedMode(bool expanded);
    bool isExpandedMode();

    int getTotalTickAmount();
    int getTotalPixelAmount();
    bool isMeasureLengthConstant();

    void setMeasureAmount(int measureAmount);
    int getMeasureAmount();

    float defaultMeasureLengthInPixels();
    int defaultMeasureLengthInTicks();

    int getFirstMeasure();
    void setFirstMeasure(int firstMeasureID);

    int measureAtPixel(int pixel);
    int measureAtTick(int tick);
    int measureDivisionAt(int pixel);

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

    void addTimeSigChange(int measure, int num, int denom);
    void eraseTimeSig(int id);
    //TimeSigChange* removeTimeSig(int id);

    void selectTimeSig(const int id);
    void unselect();

    int firstTickInMeasure(int id);
    int firstPixelInMeasure(int id);
    int lastTickInMeasure(int id);
    int lastPixelInMeasure(int id);

    void updateMeasureInfo();
    void updateVector(int newSize);

    void beforeImporting();
    void afterImporting();
    // used only while loading midi files, because midi files don't store in which measure it is. dont use for other uses.
    void addTimeSigChange_import(int tick, int num, int denom);

    bool readFromFile(irr::io::IrrXMLReader* xml);
    void saveToFile(wxFileOutputStream& fileout);
};

}

#endif
