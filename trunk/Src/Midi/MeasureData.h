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

#ifndef __MEASURE_DATA_H__
#define __MEASURE_DATA_H__


#include "ptr_vector.h"
#include "Utils.h"
#include "Midi/TimeSigChange.h"

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

namespace AriaMaestosa
{
    
    class MainFrame;
    class MeasureBar;
    

   /**
     * @brief This class takes care of everything related to measure data.
     *
     * Since it manages multiple time signature changes, it should
     * always be used when measure info is necessary.
     * It is closely related to MeasureBar, its graphical counterpart.
     *
     * @ingroup midi
     */
    class MeasureData
    {
        friend class MeasureBar;
        
        class MeasureInfo
        {
        public:
            MeasureInfo() { selected = false, tick = 0, pixel = 0; endTick=-1; widthInTicks=-1; widthInPixels=-1;}
            
            bool selected;
            int tick, endTick;
            int pixel, endPixel;
            int widthInTicks, widthInPixels;
        };
        
        /** contains one item for each measure in the sequence */
        std::vector<MeasureInfo> measureInfo;
        
        int m_measure_amount;
        int firstMeasure;
        
        bool expandedMode;
        
        /** contains one item for each time signature change event */
        ptr_vector<TimeSigChange> timeSigChanges;
        
        bool somethingSelected;
        int selectedTimeSig;
        
        // Only access this in expanded mode otherwise they're empty
        int totalNeededLengthInTicks;
        int totalNeededLengthInPixels;
    public:
        
        LEAK_CHECK();
        
        //FIXME: it's bad design that the data owns the graphics...
        OwnerPtr<MeasureBar>  graphics;
        
        MeasureData(int measureAmount);
        ~MeasureData();
        
        void  setExpandedMode(bool expanded);
        
        bool  isExpandedMode() const { return expandedMode; }
        
        int   getTotalTickAmount();
        int   getTotalPixelAmount();
        bool  isMeasureLengthConstant();
        
        void  setMeasureAmount(int measureAmount);
        int   getMeasureAmount() const { return m_measure_amount; }
        
        float defaultMeasureLengthInPixels();
        int   defaultMeasureLengthInTicks();
        
        int   getFirstMeasure() const { return firstMeasure; }
        void  setFirstMeasure(int firstMeasureID);
        
        int   measureAtPixel(int pixel);
        int   measureAtTick(int tick);
        int   measureDivisionAt(int pixel);
        
        float measureLengthInPixels(int measure =-1);
        int   measureLengthInTicks(int measure = -1);
        float beatLengthInPixels();
        int   beatLengthInTicks();
        
        /** @brief get time sig num, either for a specific mesure, either the default value (no argument) */
        int   getTimeSigNumerator(int measure=-1) const;
        
        /** @brief get time sig denom, either for a specific mesure, either the default value (no argument) */
        int   getTimeSigDenominator(int measure=-1) const;
        
        /** @brief Called either when user changes the numbers on the top bar, either when importing a song */
        void  setTimeSig(int num, int denom);
        
        int   getTimeSigAmount() const { return timeSigChanges.size(); }
        
        TimeSigChange& getTimeSig(int id);
        
        void  addTimeSigChange(int measure, int num, int denom);
        void  eraseTimeSig(int id);
        
        void  selectTimeSig(const int id);
        void  unselect();
        
        int   firstTickInMeasure(int id);
        int   firstPixelInMeasure(int id);
        int   lastTickInMeasure(int id);
        int   lastPixelInMeasure(int id);
        
        /**
         * @brief Time Signatures have changed, update and recalculate information about location of measures
         * and events.
         */
        void  updateMeasureInfo();
        
        /**
         * @brief Change the number of items in the selected vector sothat it contains the same amount of elements
         * as the number of measures.
         */
        void  updateVector(int newSize);
        
        void  beforeImporting();
        void  afterImporting();
        
        /** @brief used only while loading midi files */
        void  addTimeSigChange_import(int tick, int num, int denom);
        
        /** @brief deserializatiuon */
        bool  readFromFile(irr::io::IrrXMLReader* xml);
        
        /** @brief serializatiuon */
        void  saveToFile(wxFileOutputStream& fileout);
    };
    
}

#endif
