#include "Printing/PrintLayout.h"
#include "Printing/PrintingBase.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Editors/GuitarEditor.h"
#include "Printing/ScorePrint.h"
#include "Printing/TabPrint.h"
#include "Printing/PrintLayoutMeasure.h"
#include "Printing/PrintableSequence.h"
#include "Printing/PrintLayoutLine.h"
#include "AriaCore.h"

#include <iostream>
#include "wx/wx.h"
#include <cmath>
#include <map>


namespace AriaMaestosa
{
const int MAX_LEVELS_ON_PAGE = 74;
const int MIN_UNIT_WIDTH = 3;

int repetitionMinimalLength = 2;

// -------------------------------------------------------------------------------------------
    
int getRepetitionMinimalLength()
{
    return repetitionMinimalLength;
}
    
// -------------------------------------------------------------------------------------------
    
void setRepetitionMinimalLength(const int newvalue)
{
    repetitionMinimalLength = newvalue;
}

// -------------------------------------------------------------------------------------------
    
// if a repetition is found, it is stored in the variables and returns true,
// otherwise returns false
    // FIXME: Why is this 'PrintLayoutMeasure' thing here...
bool PrintLayoutMeasure::findConsecutiveRepetition(ptr_vector<PrintLayoutMeasure>& measures, const int measureAmount,
                                                int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                                int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/)
{
    
    // check if it works with first measure occurence of similar measures
    if (id+1<measureAmount and measures[id+1].firstSimilarMeasure == measures[id].firstSimilarMeasure+1 )
    {
        int amount = 0;
        
        for(int iter=1; iter<measureAmount; iter++)
        {
            if (id+iter<measureAmount and measures[id+iter].firstSimilarMeasure == measures[id].firstSimilarMeasure+iter )
            {
                amount++;
            }
            else
            {
                break;
            }
        }//next
        firstMeasureThatRepeats = id;
        lastMeasureThatRepeats = id + amount;
        firstMeasureRepeated = measures[id].firstSimilarMeasure;
        lastMeasureRepeated = measures[id].firstSimilarMeasure + amount;
        return true;
    }
    // check if it works with a later occurence of a similar measure
    else
    {
        const int first_measure =  measures[id].firstSimilarMeasure;
        const int amount = measures[ first_measure ].similarMeasuresFoundLater.size();
        for(int laterOccurence=0; laterOccurence<amount; laterOccurence++)
        {
            const int checkFromMeasure = measures[ first_measure ].similarMeasuresFoundLater[laterOccurence];
            //std::cout << "        < lvl 2, testing measure " << checkFromMeasure << std::endl;
            //if (checkFromMeasure+1<id and measures[checkFromMeasure+1].firstSimilarMeasure ==
            //   measures[checkFromMeasure].firstSimilarMeasure+1 )
            //{
            int amount = 0;
            
            // check if there is a consecutive repetition with measures from this area
            
            for(int iter=0; iter</*id-checkFromMeasure*/measureAmount; iter++)
            {
                if (not(checkFromMeasure+iter<id and
                       checkFromMeasure+iter<measureAmount and id+iter<measureAmount)) continue;
                
                // check if they are identical
                
                if ( // they are identical because they both are repetitions of the same one
                   (measures[checkFromMeasure+iter].firstSimilarMeasure == measures[id+iter].firstSimilarMeasure and
                    measures[checkFromMeasure+iter].firstSimilarMeasure != -1)
                   or
                   // they are identical because the second is a repetition of the first
                   (checkFromMeasure+iter == measures[id+iter].firstSimilarMeasure)
                   )
                {
                    //std::cout << "            //" << (checkFromMeasure+iter+1) << " is same as " << (id+iter+1) << std::endl;
                    amount++;
                }
                else
                {
                    //std::cout << "            //but " << (checkFromMeasure+iter+1) << " is NOT same as " << (id+iter+1) << " (" << measures[checkFromMeasure+iter].firstSimilarMeasure+1 << " != " << measures[id+iter].firstSimilarMeasure+1 << ")" << std::endl;
                    break;
                }
            }//next
            //std::cout << "        > amount=" << amount << std::endl;
            if (amount<repetitionMinimalLength) continue;
            //std::cout << "measure " << id+1  << " is a level 2 repetition" << std::endl;
            firstMeasureThatRepeats = id;
            lastMeasureThatRepeats = id + amount-1;
            firstMeasureRepeated = checkFromMeasure;
            lastMeasureRepeated = checkFromMeasure + amount-1;
            return true;
            //}
        }//next
        
        // if we get there, it never works
        return false;
    }
}
    
// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#endif

PrintLayoutManager::PrintLayoutManager(PrintableSequence* sequence,
                                       ptr_vector<LayoutPage>& layoutPages_a /* out */) : layoutPages(layoutPages_a)
{
    this->sequence = sequence;
}
    
// -------------------------------------------------------------------------------------------
    
void PrintLayoutManager::generateMeasures(ptr_vector<Track, REF>& tracks)
{
    std::cout << "\n====\ngenerateMeasures\n====\n";
    const int trackAmount = tracks.size();
    const int measureAmount = getMeasureData()->getMeasureAmount();

    for(int tr=0; tr<trackAmount; tr++)
    {

        Track* track = tracks.get(tr);

        // add measures
        for(int measure=0; measure<measureAmount; measure++)
        {
             measures.push_back( new PrintLayoutMeasure(measure) );
        }

        int note=0;
        // give them track references
        for(int measure=0; measure<measureAmount; measure++)
        {
            assertExpr(measure,<,measures.size());
            
            note = measures[measure].addTrackReference(note, track);
            
            //std::cout << "meas% " << (measures[measure].trackRef[0].track->getName().mb_str()) << std::endl;

        } // next measure
    } // next track
}

// -------------------------------------------------------------------------------------------
    
void PrintLayoutManager::findSimilarMeasures()
{
    const int measureAmount = getMeasureData()->getMeasureAmount();

    for(int measure=0; measure<measureAmount; measure++)
    {
        // check current measure against all previous measures to see if it is not a repetition
        for(int checkMeasure=0; checkMeasure<measure; checkMeasure++)
        {
            assertExpr(measure,<,(int)measures.size());
            assertExpr(checkMeasure,<,(int)measures.size());
            const bool isSameAs = measures[measure].calculateIfMeasureIsSameAs(measures[checkMeasure]);
            //std::cout << "measure " << (measure+1) << " is same as " << (checkMeasure+1) << " ? " << isSameAs << std::endl;
            
            if (!isSameAs) continue;
            measures[measure].firstSimilarMeasure = checkMeasure;
            measures[checkMeasure].similarMeasuresFoundLater.push_back(measure);
            break;
        }//next
    }//next
}
    
// -------------------------------------------------------------------------------------------
    
#define _verbose 1
    
void PrintLayoutManager::createLayoutElements(bool checkRepetitions_bool)
{
    std::cout << "\n====\ncreateLayoutElements\n====\n";
    
    const int measureAmount = getMeasureData()->getMeasureAmount();

    int previous_num = -1, previous_denom = -1;
    
    for(int measure=0; measure<measureAmount; measure++)
    {
#ifdef _verbose
        std::cout << "Generating layout element for measure " << (measure+1) << std::endl;
#endif

        if (getMeasureData()->getTimeSigDenominator(measure) != previous_denom ||
           getMeasureData()->getTimeSigNumerator(measure) != previous_num)
        {
            // add time signature element
            LayoutElement el2(LayoutElement(TIME_SIGNATURE, -1));
            el2.width_in_units = 1;
            el2.num = getMeasureData()->getTimeSigNumerator(measure);
            el2.denom = getMeasureData()->getTimeSigDenominator(measure);
            
            previous_num = el2.num;
            previous_denom = el2.denom;
            layoutElements.push_back( el2 );
        }

        // ----- empty measure -----
        if (measures[measure].shortestDuration==-1)
        {
#ifdef _verbose
            std::cout << "    measure " << (measure+1) << " is empty\n";
#endif
            layoutElements.push_back( LayoutElement(EMPTY_MEASURE, measure) );
            layoutElements[layoutElements.size()-1].width_in_units = 2;

            // check that measure is really empty; it's possible that it contains
            // the end of a note that started in the previous measure.
            // if this is the case, we need to make the measure broader than the default 2 units
            const int track_ref_amount = measures[measure].trackRef.size();
            for(int t=0; t<track_ref_amount; t++)
            {
                Track* track = measures[measure].trackRef[t].track;
                const int noteAmount = track->getNoteAmount();
                for(int n=0; n<noteAmount; n++)
                {
                    if (track->getNoteStartInMidiTicks(n) < measures[measure].firstTick and
                       track->getNoteEndInMidiTicks(n)  > measures[measure].firstTick)
                    {
                        layoutElements[layoutElements.size()-1].width_in_units = 5;
                        t = 99; // quick hack to totally abort both loops
                        break;
                    }
                }// next note
            }// next track ref
        }// end if empty measure
        // repetition
        else if (checkRepetitions_bool and measures[measure].firstSimilarMeasure!=-1)
        {

            if (getRepetitionMinimalLength()<2)
            {
                LayoutElement element(SINGLE_REPEATED_MEASURE, measure);
                layoutElements.push_back( element );
                continue;
            }

            int firstMeasureThatRepeats, lastMeasureThatRepeats, firstRepeatedMeasure, lastRepeatedMeasure; // used when finding repetitions

            // -------- play same measure multiple times --------
            // check if next measure is the same as current measure
            if (measure+1<measureAmount and measures[measure+1].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
            {
                int amountOfTimes = 1;
                for(int iter=1; iter<measureAmount; iter++)
                {
                    if (measure+iter<measureAmount and measures[measure+iter].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
                    {
                        amountOfTimes++;
                    }
                    else
                    {
                        break;
                    }
                }//next

                if (amountOfTimes < getRepetitionMinimalLength())
                {
#ifdef _verbose
                    std::cout << "    play many times refused, measures " << (measure+1) << " to " << (measure+amountOfTimes+1) << " are normal" << std::endl;
#endif
                    // not enough repetitions, add as regular measures
                    for(int i=0; i<amountOfTimes; i++)
                    {
                        layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
                        measure++;
                    }
                    measure--;
                }
                else
                {
                    // check if we need to display the repetition first before adding play many times
                    if (measures[measure].firstSimilarMeasure != -1 and measures[measure].firstSimilarMeasure != measure-1)
                        // we don't need to if measure was not a repeptition, in which case it is already there
                        // we need neither if it is the measure just before
                    {
                        LayoutElement element(SINGLE_REPEATED_MEASURE, measure);
                        layoutElements.push_back( element );
                    }
                    else
                    {
                        amountOfTimes++; // if measure was already displayed... there were e.g. 3 additional repetitions, but we want to show x4
                    }
#ifdef _verbose
                    std::cout << "    measure " << (measure+1) << " is played " << amountOfTimes << " times. all are the same as " << (measures[measure].firstSimilarMeasure+1) << std::endl;
#endif
                    LayoutElement element(PLAY_MANY_TIMES);
                    element.amountOfTimes = amountOfTimes;
                    measures[measure].cutApart = true;
                    if (measures[measure].firstSimilarMeasure == measure-1) measure = measure + amountOfTimes-2;
                    else measure = measure + amountOfTimes-1;
                    layoutElements.push_back( element );
                }

            }

            // ------- repeat a riff --------
            // check if next measure is a reptition, and check this repetition is the next one compared to the current repeated measure
            else if ( measures[measure].findConsecutiveRepetition(measures, measureAmount, firstMeasureThatRepeats, lastMeasureThatRepeats,
                                                                 firstRepeatedMeasure, lastRepeatedMeasure) )
            {

                const int amount = lastMeasureThatRepeats - firstMeasureThatRepeats;
                if (amount+1 >= getRepetitionMinimalLength())
                {
#ifdef _verbose
                    std::cout << "    repetition from " << (firstMeasureThatRepeats+1) << " to " <<
                    (lastMeasureThatRepeats+1) << "(" << (firstRepeatedMeasure+1) << ", " <<
                    (lastRepeatedMeasure+1) << ")"  << std::endl;
#endif

                    //measures[firstMeasureThatRepeats].cutApart = true;

                    LayoutElement element(REPEATED_RIFF);
                    element.firstMeasure = firstMeasureThatRepeats;
                    element.measure = firstMeasureThatRepeats;
                    element.lastMeasure = lastMeasureThatRepeats;
                    element.firstMeasureToRepeat = firstRepeatedMeasure;
                    element.lastMeasureToRepeat = lastRepeatedMeasure;
                    measure = lastMeasureThatRepeats;//measure + amount;
                        layoutElements.push_back( element );
                }
                else
                    // repetition is not long enough, use normal measures
                {
#ifdef _verbose
                    std::cout << "    repetition refused because " << (amount+1) << " < " << getRepetitionMinimalLength() << " measures " << (measure+1) << " to " << (measure+getRepetitionMinimalLength()+1) << " are normal" << std::endl;
#endif
                    for(int iter=0; iter<getRepetitionMinimalLength(); iter++)
                    {
                        layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure+iter) );
                    }
                    measure += getRepetitionMinimalLength()-1;
                }
            }
            else
            {
#ifdef _verbose
                std::cout << "    measure " << (measure+1) << " looks like a repetition but treated as normal" << std::endl;
#endif
                // despite looking like a repetition, user settings are set to treat it as a normal measure
                layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
            }

        }
        else
            // ------ normal measure -----
        {
#ifdef _verbose
            std::cout << "    measure " << (measure+1) << " is normal" << std::endl;
#endif
            layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
        }

    }//next measure
}

// -------------------------------------------------------------------------------------------
    
void PrintLayoutManager::calculateRelativeLengths()
{
    std::cout << "\n====\ncalculateRelativeLengths\n====\n";

    // calculate approximative width of each element
    const int layoutElementsAmount = layoutElements.size();
    for (int n=0; n<layoutElementsAmount; n++)
    {
        std::cout << "= layout element " << n << " =\n";

        layoutElements[n].width_in_units = 2;

        if (layoutElements[n].getType() == SINGLE_MEASURE || layoutElements[n].getType() == EMPTY_MEASURE)
        {
            // ---- for non-linear printing mode
            
            // determine a list of all ticks on which a note starts.
            // then we can determine where within this measure should this note be drawn
            
            std::vector<int> all_ticks_vector;
            
            // Build a list of all ticks
            PrintLayoutMeasure& meas = measures[layoutElements[n].measure];
            std::map< int /* tick */, TickPosInfo >& ticks_relative_position = meas.ticks_relative_position;
            
            const int trackAmount = meas.trackRef.size();
            for (int i=0; i<trackAmount; i++)
            {
                EditorPrintable* editorPrintable = sequence->getEditorPrintable( i );
                assert( editorPrintable != NULL );
                
                editorPrintable->addUsedTicks(meas, i, meas.trackRef[i], ticks_relative_position);
            }
            
            // building the full list from 'map' prevents duplicates
            std::map<int,TickPosInfo>::iterator it;
            for (it=ticks_relative_position.begin() ; it != ticks_relative_position.end(); it++)
            {
                all_ticks_vector.push_back( (*it).first );
            }
            
            // order the vector
            const int all_ticks_amount = all_ticks_vector.size();
            bool changed; // crappy bubble sort - FIXME : use something better
            do
            {
                changed = false;
                for (int i=0; i<all_ticks_amount-1; i++)
                {
                    if (all_ticks_vector[i] > all_ticks_vector[i+1])
                    {
                        int tmp = all_ticks_vector[i];
                        all_ticks_vector[i] = all_ticks_vector[i+1];
                        all_ticks_vector[i+1] = tmp;
                        changed = true;
                    }
                }
            } while (changed);
            
            // associate a relative position to each note
            // start by total, renormalize from 0 to 1 after
            float totalRelativePosition = 0;
            for (int i=0; i<all_ticks_amount; i++)
            {
                ticks_relative_position[ all_ticks_vector[i] ].relativePosition = totalRelativePosition;
                totalRelativePosition += ticks_relative_position[ all_ticks_vector[i] ].proportion;
                ticks_relative_position[ all_ticks_vector[i] ].relativeEndPosition = totalRelativePosition;
            }
            totalRelativePosition += 1.0f; // leave 1 space worth of empty space at the end
            for (int i=0; i<all_ticks_amount; i++)
            {
                TickPosInfo& tickPosInfo = ticks_relative_position[ all_ticks_vector[i] ];
                
                //std::cout << "note relativePosition = " << 
                //" (" << tickPosInfo.relativePosition << "/" << intRelativePosition << ") = ";
                
                tickPosInfo.relativePosition = tickPosInfo.relativePosition / totalRelativePosition;
                tickPosInfo.relativeEndPosition = tickPosInfo.relativeEndPosition / totalRelativePosition;

                std::cout << "tickPosInfo.relativePosition = " << tickPosInfo.relativePosition << " to " <<
                			tickPosInfo.relativeEndPosition << std::endl;
            }
            
            layoutElements[n].width_in_units = all_ticks_amount;
            if (layoutElements[n].width_in_units < MIN_UNIT_WIDTH) layoutElements[n].width_in_units = MIN_UNIT_WIDTH;
            std::cout << "Layout element " << n << " is " << layoutElements[n].width_in_units << " unit(s) wide" << std::endl;
        }
        else if (layoutElements[n].getType() == REPEATED_RIFF)
        {
            layoutElements[n].width_in_units = 5;
        }

        //std::cout << "$$ setting charwidth for element " << n << " : " << layoutElements[n].width_in_units << std::endl;
    } // end for elements
}

// -------------------------------------------------------------------------------------------
    
/**
 * Builds the Page/Line layout tree from the full list of layout elements
 */
void PrintLayoutManager::layInLinesAndPages()
{
    std::cout << "\n====\nlayInLinesAndPages\n====\n";
    
    const int layoutElementsAmount = layoutElements.size();

    int current_width = 0;
    int current_height = 0;

    int current_page = 0;
    
    layoutPages.push_back( new LayoutPage() );

    ptr_vector<PrintLayoutMeasure, REF> measures_ref = measures.getWeakView();
    layoutPages[current_page].layoutLines.push_back( new LayoutLine(sequence, measures_ref) );
    int currentLine = 0;

    // add line header
    LayoutElement el(LayoutElement(LINE_HEADER, -1));
    
    int header_width = 2;
    
    if (sequence->is_score_editor_used)
    {
        // FIXME : this needs to be in pixels/print units, not my note-relative units.
        header_width = (int)round(3.0 + sequence->max_signs_in_keysig*4.0/7.0); // 4/7 is an empirical ratio
    }
    
    el.width_in_units = header_width;
    current_width += header_width;
    layoutPages[current_page].layoutLines[currentLine].layoutElements.push_back( el );
    
    // add layout elements one by one, switching to the next line when there's too many
    // elements on the current one
    for (int n=0; n<layoutElementsAmount; n++)
    {
        if (current_width + layoutElements[n].width_in_units > max_line_width_in_units)
        {
            // too much stuff on current line, switch to another line
            layoutPages[current_page].layoutLines[currentLine].width_in_units = current_width;
            current_width = 0;
            const int line_height = layoutPages[current_page].layoutLines[currentLine].calculateHeight();
            current_height += line_height;

            // too much lines on current page, switch to a new page
            if (current_height > MAX_LEVELS_ON_PAGE)
            {
                current_height = line_height;
                layoutPages.push_back( new LayoutPage() );
                layoutPages[current_page].layoutLines[currentLine].last_of_page = true;
                current_page++;
            }

            //std::cout << "*&^*&^*&^%$%$#@#@!! current_height = " << current_height << std::endl;

            //currentLine++;
            ptr_vector<PrintLayoutMeasure, REF> refview = measures.getWeakView();
            layoutPages[current_page].layoutLines.push_back( new LayoutLine(sequence, refview) );
            currentLine = layoutPages[current_page].layoutLines.size()-1;
        }
        assertExpr(currentLine,<,(int)layoutPages[current_page].layoutLines.size());
        layoutPages[current_page].layoutLines[currentLine].layoutElements.push_back(layoutElements[n]);
        current_width += layoutElements[n].width_in_units;
    }
    // for last line processed
    layoutPages[current_page].layoutLines[currentLine].width_in_units = current_width;
    layoutPages[current_page].layoutLines[currentLine].calculateHeight();
}

// -------------------------------------------------------------------------------------------
    
/** main function called from other classes. measures must have been geenrated. */
void PrintLayoutManager::calculateLayoutElements
                            (ptr_vector<Track, REF>& tracks,
                             const bool checkRepetitions_bool)
{
    // search for repeated measures if necessary
    if (checkRepetitions_bool) findSimilarMeasures();
    
    const int trackAmount = tracks.size();
    for(int i=0; i<trackAmount; i++)
    {
        EditorPrintable* editorPrintable = sequence->getEditorPrintable( i );
        assert( editorPrintable != NULL );
        editorPrintable->earlySetup( i, tracks.get(i) );
    }
    
    createLayoutElements(checkRepetitions_bool);
    calculateRelativeLengths();
    layInLinesAndPages();
}

// -------------------------------------------------------------------------------------------

}
