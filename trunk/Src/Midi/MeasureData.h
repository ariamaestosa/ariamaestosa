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
#include "Midi/TimeSigChange.h"
#include "Utils.h"

class wxFileOutputStream;
// forward
namespace irr { namespace io {
    class IXMLBase;
    template<class char_type, class super_class> class IIrrXMLReader;
    typedef IIrrXMLReader<char, IXMLBase> IrrXMLReader; } }

namespace AriaMaestosa
{
    class GraphicalSequence;
    class MainFrame;

    class IMeasureDataListener
    {
    public:
        
        const static int CHANGED_NOTHING = 0;
        const static int CHANGED_AMOUNT = 1;
        const static int CHANGED_TIME_SIGNATURES = 2;
        const static int CHANGED_EXPANDED_MODE_STATE = 4;
        
        virtual ~IMeasureDataListener() {}
        virtual void onMeasureDataChange(int change) = 0;
    };
    
    
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

        class MeasureInfo
        {
        public:
            MeasureInfo()
            {
                selected = false;
                tick = 0;
                // pixel = 0; widthInPixels=-1;
                endTick=-1; widthInTicks=-1;
            }
            
            bool selected;
            int tick, endTick;
            //int pixel, endPixel;
            int widthInTicks;
            //int widthInPixels;
        };
        
        /** contains one item for each measure in the sequence */
        std::vector<MeasureInfo> m_measure_info;
        
        int m_measure_amount;
        int m_first_measure;
        
        bool m_expanded_mode;
        
        /** contains one item for each time signature change event */
        ptr_vector<TimeSigChange> m_time_sig_changes;
        
        bool m_something_selected;
        int  m_selected_time_sig;
        
        // Only access this in expanded mode otherwise they're empty
        int totalNeededLengthInTicks;
        //int totalNeededLengthInPixels;
                
        /**
         * @brief Change the number of items in the selected vector sothat it contains the same amount of elements
         * as the number of measures.
         */
        void  updateVector(int newSize);
        
        Sequence* m_sequence;
        
        /**
         * @brief Time Signatures have changed, update and recalculate information about location of measures
         * and events.
         */
        void  updateMeasureInfo();
        
        void  setExpandedMode(bool expanded);
        void  setMeasureAmount(int measureAmount);

        /** @brief Called either when user changes the numbers on the top bar, either when importing a song */
        void  setTimeSig(int num, int denom);
        
        /** @brief Erase the time signature event denoted by the given ID
          * @param id Id of the time sig to erase, in range [0..getTimeSigAmount()-1]
          */
        void  eraseTimeSig(int id);
        
        /** @brief Move a timesig event to another measure */
        void setTimesigMeasure(const int id, const int newMeasure);
        
        /** @brief  Add a time signature change event
          * @note   If a time sig changes already exists at the given measure, it is modified to bear the
          *         new values
          * @return the ID of the newly added _or modified) item
          */
        int  addTimeSigChange(int measure, int num, int denom);
        
        std::vector<IMeasureDataListener*> m_listeners;
        
        void  beforeImporting();
        void  afterImporting();
        
        /** @brief used only while loading midi files */
        void  addTimeSigChange_import(int tick, int num, int denom);
        
        
    public:
        
        class Transaction
        {
            friend class MeasureData;
            
#ifdef _MORE_DEBUG_CHECKS
            unsigned long m_magic_number;
#endif
            
        protected:
            MeasureData* m_parent;
            int m_changes;
            
            Transaction(MeasureData* parent)
            {
#ifdef _MORE_DEBUG_CHECKS
                m_magic_number = 0x54325432;
#endif
                m_parent = parent;
                m_changes = IMeasureDataListener::CHANGED_NOTHING;
            }
            
        public:
            
            ~Transaction()
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
#ifdef _MORE_DEBUG_CHECKS
                m_magic_number = 0xDEADBEEF;
#endif
                
                if (m_parent != NULL) m_parent->updateMeasureInfo();
                
                const int count = m_parent->m_listeners.size();
                for (int n=0; n<count; n++) m_parent->m_listeners[n]->onMeasureDataChange(m_changes);
            }
            
            void  setExpandedMode(bool expanded)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_EXPANDED_MODE_STATE;
                m_parent->setExpandedMode(expanded);
            }
            
            void  setMeasureAmount(int measureAmount)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_AMOUNT;
                m_parent->setMeasureAmount(measureAmount);
            }
            
            /** @brief Called either when user changes the numbers on the top bar, either when importing a song */
            void  setTimeSig(int num, int denom)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_TIME_SIGNATURES;
                m_parent->setTimeSig(num, denom);
            }
            
            /** @brief Erase the time signature event denoted by the given ID
              * @param id Id of the time sig to erase, in range [0..getTimeSigAmount()-1]
              */
            void  eraseTimeSig(int id)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_TIME_SIGNATURES;
                m_parent->eraseTimeSig(id);
            }
            
            /** @brief Move a timesig event to another measure */
            void setTimesigMeasure(const int id, const int newMeasure)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_TIME_SIGNATURES;
                m_parent->setTimesigMeasure(id, newMeasure);
            }
            
            /** @brief  Add a time signature change event
              * @note   If a time sig changes already exists at the given measure, it is modified to bear the
              *         new values
              * @return the ID of the newly added _or modified) item
              */
            int  addTimeSigChange(int measure, int num, int denom)
            {
                ASSERT_E(m_magic_number, ==, 0x54325432);
                m_changes = m_changes | IMeasureDataListener::CHANGED_TIME_SIGNATURES;
                return m_parent->addTimeSigChange(measure, num, denom);
            }
        };
        
        
        class ImportingTransaction
        {
            friend class MeasureData;
            
        protected:
            MeasureData* m_parent;
            
            ImportingTransaction(MeasureData* parent)
            {
                m_parent = parent;
                m_parent->beforeImporting();
            }
        
        public:
            
            ~ImportingTransaction()
            {
                m_parent->afterImporting();
            }
            
            void addTimeSigChange(int tick, int num, int denom)
            {
                m_parent->addTimeSigChange_import(tick, num, denom);
            }
            
        };
        
        
        LEAK_CHECK();
                
        MeasureData(Sequence* seq, int measureAmount);
        ~MeasureData();
        
        int   getTotalTickAmount()      const;
        int   getFirstMeasure()         const { return m_first_measure;  }        
        int   measureAtTick(int tick)   const;
        bool  isExpandedMode()          const { return m_expanded_mode;  }
        bool  isMeasureLengthConstant() const
        {
            return (not m_expanded_mode and m_time_sig_changes.size() == 1);
        }
        
        void  selectOnly(const int measureID);
        
        int   defaultMeasureLengthInTicks();
        int   measureLengthInTicks(int measure = -1) const;
        
        void  setFirstMeasure(int firstMeasureID);

        /** @brief get time sig num, either for a specific mesure, either the default value (no argument) */
        int   getTimeSigNumerator(int measure=-1) const;
        
        /** @brief get time sig denom, either for a specific mesure, either the default value (no argument) */
        int   getTimeSigDenominator(int measure=-1) const;
        
        /** @return the amount of time signature events */
        int   getTimeSigAmount() const { return m_time_sig_changes.size(); }
        
        const TimeSigChange& getTimeSig(int id)
        {
            ASSERT_E(id,>=,0);
            ASSERT_E(id,<,m_time_sig_changes.size());
            return m_time_sig_changes[id];
        }

        int   firstTickInMeasure(int id) const;
        int   lastTickInMeasure (int id) const;
        int   getTimeSigChangeCount() const;
        
        int   getMeasureLength(int id) const
        {
            return lastTickInMeasure(id) - firstTickInMeasure(id);
        }
        
        /**
         * @brief  Invoke this when you want to modify measure data
         * @return A Transaction object that gives you write-access to measure data
         * @note   Upon destruction, the Transaction object will initiate some post-processing on
         *         the data. It is thus recommended to scope this object so it is always destroyed.
         */
        Transaction* startTransaction() { return new Transaction(this); }
        
        ImportingTransaction* startImportTransaction() { return new ImportingTransaction(this); }
        
        void addListener(IMeasureDataListener* l)
        {
            m_listeners.push_back(l);
        }
        
        bool isSomethingSelected() const
        {
            return m_something_selected;
        }
        
        int getSelectedTimeSig() const
        {
            return m_selected_time_sig;
        }
        void setSelectedTimeSig(int i)
        {
            m_selected_time_sig = i;
        }
        
        const MeasureInfo& getMeasureInfo(const int n) const
        {
            return m_measure_info[n];
        }

        // FIXME: we have those 2 sizes, they should be part of implementation details and NOT EXPOSED
        int getMeasureInfoAmount()
        {
            return m_measure_info.size();
        }
        int getMeasureAmount() const
        {
            return m_measure_amount;
        }

        int getTotalNeededLength() const
        {
            return totalNeededLengthInTicks;
        }
        
        void checkUpToDate();
        
        int getTotalNeededLengthInTicks() const
        {
            return totalNeededLengthInTicks;
        }
        
        void selectNothing();
        
        void selectMeasure(int mid);
        
        /** @brief deserializatiuon */
        bool  readFromFile(irr::io::IrrXMLReader* xml);
        
        /** @brief serializatiuon */
        void  saveToFile(wxFileOutputStream& fileout);
    };

    typedef OwnerPtr<MeasureData::Transaction> ScopedMeasureTransaction;
    typedef OwnerPtr<MeasureData::ImportingTransaction> ScopedMeasureITransaction;
}

#endif
