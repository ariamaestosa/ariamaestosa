#include "Printing/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/AriaPrintable.h"

#include "Dialogs/WaitWindow.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Editors/GuitarEditor.h"
#include "Printing/ScorePrint.h"
#include "Printing/TabPrint.h"
#include "Printing/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/PrintableSequence.h"
#include "Printing/PrintLayout/PrintLayoutLine.h"
#include "AriaCore.h"

#include <iostream>
#include "wx/wx.h"
#include <cmath>
#include <map>

using namespace AriaMaestosa;

namespace AriaMaestosa
{    
    /** Determined empirically. Used to determine when it's time to switch to another page */
    const int MIN_LEVEL_HEIGHT = 60; //FIXME: PrintLayoutAbstract is not abstract at all, deals with absolute coords...

    /** Minimal width of any element, in print units, to avoid elements that are too small and look funny */
    const int LAYOUT_ELEMENT_MIN_WIDTH = 300;
    
    /** Width, in print units, of a time signature (e.g. 4/4) element */
    const int TIME_SIG_LAYOUT_ELEMENT_WIDTH = 75;
    
    /** width of a G or F clef symbol on the staff (FIXME: set more precisely) */
    const int CLEF_WIDTH = 250;
        
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
}

    
// -------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------
#if 0
#pragma mark -
#pragma mark private
#endif

// -------------------------------------------------------------------------------------------
    
void PrintLayoutAbstract::findSimilarMeasures()
{
    const int measureAmount = getMeasureData()->getMeasureAmount();

    for (int measure=0; measure<measureAmount; measure++)
    {
        // check current measure against all previous measures to see if it is not a repetition
        for (int checkMeasure=0; checkMeasure<measure; checkMeasure++)
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
    
void PrintLayoutAbstract::createLayoutElements(std::vector<LayoutElement>& layoutElements, bool checkRepetitions_bool)
{
    std::cout << "\n====\ncreateLayoutElements\n====\n";
    
    const int measureAmount = getMeasureData()->getMeasureAmount();

    int previous_num = -1, previous_denom = -1;
    
    for (int measure=0; measure<measureAmount; measure++)
    {
        WaitWindow::setProgress( 25 + measure*25/measureAmount );

#ifdef _verbose
        std::cout << "Generating layout element for measure " << (measure+1) << std::endl;
#endif

        if (getMeasureData()->getTimeSigDenominator(measure) != previous_denom ||
            getMeasureData()->getTimeSigNumerator(measure)   != previous_num)
        {
            // add time signature element
            LayoutElement el2(LayoutElement(TIME_SIGNATURE_EL, -1));
            el2.width_in_print_units = TIME_SIG_LAYOUT_ELEMENT_WIDTH;
            el2.num = getMeasureData()->getTimeSigNumerator(measure);
            el2.denom = getMeasureData()->getTimeSigDenominator(measure);
            
            previous_num = el2.num;
            previous_denom = el2.denom;
            layoutElements.push_back( el2 );
        }

        // ----- empty measure -----
        if (measures[measure].isEmpty())
        {
#ifdef _verbose
            std::cout << "    measure " << (measure+1) << " is empty\n";
#endif
            layoutElements.push_back( LayoutElement(EMPTY_MEASURE, measure) );
            layoutElements[layoutElements.size()-1].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH;

            // check that measure is really empty; it's possible that it contains
            // the end of a note that started in the previous measure.
            // if this is the case, we need to make the measure broader than the default size
            const int track_ref_amount = measures[measure].getTrackRefAmount();
            for (int t=0; t<track_ref_amount; t++)
            {
                const Track* track = measures[measure].getTrackRef(t).getConstTrack();
                const int noteAmount = track->getNoteAmount();
                for (int n=0; n<noteAmount; n++)
                {
                    if (track->getNoteStartInMidiTicks(n) < measures[measure].getFirstTick() and
                        track->getNoteEndInMidiTicks(n)   > measures[measure].getFirstTick())
                    {
                        layoutElements[layoutElements.size()-1].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH*2;
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
                    for (int iter=0; iter<getRepetitionMinimalLength(); iter++)
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
    
void PrintLayoutAbstract::calculateRelativeLengths(std::vector<LayoutElement>& layoutElements)
{
    std::cout << "\n====\ncalculateRelativeLengths\n====\n";

    // calculate approximative width of each element
    const int layoutElementsAmount = layoutElements.size();
    for (int n=0; n<layoutElementsAmount; n++)
    {
        WaitWindow::setProgress( 50 + n*25/layoutElementsAmount );

        std::cout << "= layout element " << n << " =\n";

        //layoutElements[n].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH;

        if (layoutElements[n].getType() == SINGLE_MEASURE or layoutElements[n].getType() == EMPTY_MEASURE)
        {     
            layoutElements[n].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH;
            
            // determine a list of all ticks on which a note starts.
            // then we can determine where within this measure should this note be drawn
            
            std::vector<int> all_ticks_vector;
            
            // Ask all editors to add their symbols to the list
            PrintLayoutMeasure& meas = measures[layoutElements[n].measure];
            RelativePlacementManager& ticks_relative_position = meas.ticks_placement_manager;
            
            const int trackAmount = meas.getTrackRefAmount();
            for (int i=0; i<trackAmount; i++)
            {
                EditorPrintable* editorPrintable = sequence->getEditorPrintable( i );
                assert( editorPrintable != NULL );
                
                editorPrintable->addUsedTicks(meas, i, meas.getTrackRef(i), ticks_relative_position);
            }
            
            ticks_relative_position.calculateRelativePlacement();
            
            /*
            layoutElements[n].width_in_units = ticks_relative_position.getUnitCount();
            if (layoutElements[n].width_in_units < MIN_UNIT_WIDTH)
            {
                layoutElements[n].width_in_units = MIN_UNIT_WIDTH;
            }*/
            
            layoutElements[n].width_in_print_units = ticks_relative_position.getWidth();

            if (layoutElements[n].width_in_print_units < LAYOUT_ELEMENT_MIN_WIDTH)
            {
                layoutElements[n].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH;
            }
            
            std::cout << "++++ Layout element " << n << " is " << layoutElements[n].width_in_print_units
                      << " unit(s) wide" << std::endl;
        }
        else if (layoutElements[n].getType() == REPEATED_RIFF)
        {
            layoutElements[n].width_in_print_units = LAYOUT_ELEMENT_MIN_WIDTH;
        }

        //std::cout << "$$ setting charwidth for element " << n << " : " << layoutElements[n].width_in_units << std::endl;
    } // end for elements
}

// -------------------------------------------------------------------------------------------
    
void PrintLayoutAbstract::layInLinesAndPages(std::vector<LayoutElement>& layoutElements)
{
    std::cout << "\n====\nlayInLinesAndPages\n====\n";
    
    const int usableAreaHeightPage1 = AriaPrintable::getCurrentPrintable()->getUsableAreaHeight(1);
    const int usableAreaHeightOtherPages = AriaPrintable::getCurrentPrintable()->getUsableAreaHeight(2);

    assert(usableAreaHeightPage1 > 0);
    assert(usableAreaHeightOtherPages > 0);
    
    //FIXME: margins between lines are not considered, so min size is bound not to be respected...
    //FIXME: I think margins between the various lines of one track are not accounted for either
    const int maxLevelsOnPage1      = usableAreaHeightPage1 / MIN_LEVEL_HEIGHT;
    const int maxLevelsOnOtherPages = usableAreaHeightOtherPages / MIN_LEVEL_HEIGHT;

    assert(maxLevelsOnPage1 > 0);
    assert(maxLevelsOnOtherPages > 0);
    std::cout << "maxLevelsOnPage0=" << maxLevelsOnPage1 << " maxLevelsOnOtherPages=" << maxLevelsOnOtherPages << std::endl;
    
    //FIXME: remove magic constant '600' (which is the margin)
    const int maxLineWidthInPrintUnits = AriaPrintable::getCurrentPrintable()->getUnitWidth() - 600;     
    const int layoutElementsAmount = layoutElements.size();

    int current_width = 0;
    int current_height = 0;

    int current_page = 0;
    
    layoutPages.push_back( new LayoutPage() );

    ptr_vector<PrintLayoutMeasure, REF> measures_ref = measures.getWeakView();
    
    LayoutLine* currentLine = new LayoutLine(sequence, measures_ref);
    layoutPages[current_page].addLine( currentLine );

    // add line header
    LayoutElement el(LayoutElement(LINE_HEADER, -1));
    
    int header_width = CLEF_WIDTH;
    
    if (sequence->isScoreEditorUsed())
    {
        // 50 being the max size of an accidental (FIXME: don't hardcode)
        // FIXME: some numeric widths are used here, in the abstract layout manager
        header_width += sequence->getMaxKeySignatureSignCount()*50;
    }
    
    el.width_in_print_units = header_width;
    
    current_width += header_width;
    currentLine->addLayoutElement( el );
    currentLine->setLevelFrom(current_height);
    
    // add layout elements one by one, switching to the next line when there's too many
    // elements on the current one
    for (int n=0; n<layoutElementsAmount; n++)
    {
        WaitWindow::setProgress( 75 + n*25/layoutElementsAmount );

        if (current_width + layoutElements[n].width_in_print_units + MARGIN_AT_MEASURE_BEGINNING >
            maxLineWidthInPrintUnits)
        {
            // too much stuff on current line, switch to another line
            current_width = 0;
            const int line_height = currentLine->calculateHeight();
            current_height += line_height;
            
            currentLine->setLevelTo(current_height);
            current_height += INTER_LINE_MARGIN_LEVELS;
            
            // too many lines on current page, switch to a new page
            const int maxLevelHeight = (current_page == 1 ? maxLevelsOnPage1 : maxLevelsOnOtherPages);
            if (current_height > maxLevelHeight)
            {
                current_height = line_height + INTER_LINE_MARGIN_LEVELS;
                layoutPages.push_back( new LayoutPage() );
                
                // set line abstract y coordinates
                const int previousFrom = currentLine->getLevelFrom();
                const int previousTo   = currentLine->getLevelTo();
                currentLine->setLevelTo(previousTo - previousFrom);
                currentLine->setLevelFrom(0);
                                
                // move
                current_page++;
                layoutPages[current_page-1].moveYourLastLineTo(layoutPages[current_page]);
            }
            
            assert(current_height <= (current_page == 1 ? maxLevelsOnPage1 : maxLevelsOnOtherPages));
            
            ptr_vector<PrintLayoutMeasure, REF> refview = measures.getWeakView();
            currentLine = new LayoutLine(sequence, refview);
            layoutPages[current_page].addLine( currentLine );
            
            currentLine->setLevelFrom(current_height);
        }        
        currentLine->addLayoutElement(layoutElements[n]);
        
        current_width += layoutElements[n].width_in_print_units + MARGIN_AT_MEASURE_BEGINNING;
    }
    
    // ---- for last line processed (FIXME: copy-and-paste is ugly)
    const int line_height = currentLine->calculateHeight();
    current_height += line_height;
    currentLine->setLevelTo(current_height);
    current_height += INTER_LINE_MARGIN_LEVELS;
        
    // too many lines on current page, switch to a new page
    const int maxLevelHeight = (current_page == 1 ? maxLevelsOnPage1 : maxLevelsOnOtherPages);
    if (current_height > maxLevelHeight)
    {
        current_height = line_height + INTER_LINE_MARGIN_LEVELS;
        layoutPages.push_back( new LayoutPage() );

        const int previousFrom = currentLine->getLevelFrom();
        const int previousTo   = currentLine->getLevelTo();
        currentLine->setLevelTo(previousTo - previousFrom);
        currentLine->setLevelFrom(0);
        
        current_page++;
        layoutPages[current_page-1].moveYourLastLineTo(layoutPages[current_page]);
    }
    // -------------------
    
    assert(current_height <= (current_page == 1 ? maxLevelsOnPage1 : maxLevelsOnOtherPages));
    
    
#ifndef NDEBUG
    for (int p=0; p<layoutPages.size(); p++)
    {
        std::cout << "(( PAGE " << (p+1) << " ))\n";
        const int maxh = (p == 0 ? maxLevelsOnPage1 : maxLevelsOnOtherPages);
        std::cout << "MAXIMUM height for this page : " << maxh << std::endl;

        LayoutPage& page = layoutPages[p];
        
        int total = 0;
        const int lineCount = page.getLineCount();
        for (int t=0; t<lineCount; t++)
        {
            total += page.getLine(t).m_level_height + INTER_LINE_MARGIN_LEVELS;
            std::cout << "        " << page.getLine(t).m_level_height
                      << " : " << page.getLine(t).getLevelFrom() << " to " << page.getLine(t).getLevelTo() << "\n";
            std::cout << "        " << INTER_LINE_MARGIN_LEVELS << " (margin)\n";
            
            assertExpr( page.getLine(t).getLevelFrom(),  >, -1 );
            
            assert( page.getLine(t).getLevelTo()   != -1 );
            assertExpr( page.getLine(t).getLevelTo(),   <=, maxh );
            
            assertExpr( page.getLine(t).getLevelTo(), >=, page.getLine(t).getLevelFrom() );
        }
        std::cout << "    ------------\n        " << total << "\n\n";
        assert(total <= maxh);
    }
#endif
}

// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark public
#endif

PrintLayoutAbstract::PrintLayoutAbstract(PrintableSequence* sequence,
                                         ptr_vector<LayoutPage>& layoutPages_a /* out */) : layoutPages(layoutPages_a)
{
    this->sequence = sequence;
}

// -------------------------------------------------------------------------------------------------------------

void PrintLayoutAbstract::generateMeasures(ptr_vector<Track, REF>& tracks)
{
    std::cout << "\n====\ngenerateMeasures\n====\n";
    const int trackAmount = tracks.size();
    const int measureAmount = getMeasureData()->getMeasureAmount();
    
    for (int tr=0; tr<trackAmount; tr++)
    {
        Track* track = tracks.get(tr);
        
        // add measures
        for (int measure=0; measure<measureAmount; measure++)
        {
            measures.push_back( new PrintLayoutMeasure(measure) );
        }
        
        int note=0;
        // give them track references
        for (int measure=0; measure<measureAmount; measure++)
        {
            assertExpr(measure,<,measures.size());
            
            note = measures[measure].addTrackReference(note, track);
            
            //std::cout << "meas% " << (measures[measure].trackRef[0].track->getName().mb_str()) << std::endl;
            
        } // next measure
        
        WaitWindow::setProgress( tr*25/trackAmount );

    } // next track
}

// -------------------------------------------------------------------------------------------------------------
    
void PrintLayoutAbstract::calculateLayoutElements (ptr_vector<Track, REF>& tracks, const bool checkRepetitions_bool)
{
    assert(measures.size() > 0); // generating measures must have been done first
    std::vector<LayoutElement> layoutElements;
    
    // search for repeated measures if necessary
    if (checkRepetitions_bool) findSimilarMeasures();
    
    const int trackAmount = tracks.size();
    for (int i=0; i<trackAmount; i++)
    {
        EditorPrintable* editorPrintable = sequence->getEditorPrintable( i );
        assert( editorPrintable != NULL );
        editorPrintable->earlySetup( i, tracks.get(i) );
    }
    
    createLayoutElements(layoutElements, checkRepetitions_bool);
    calculateRelativeLengths(layoutElements);
    
    // this will also move the layoutElements to their corresponding LayoutLine object
    layInLinesAndPages(layoutElements);
}

// -------------------------------------------------------------------------------------------


