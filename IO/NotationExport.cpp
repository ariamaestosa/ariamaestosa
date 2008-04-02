#include "IO/NotationExport.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "GUI/MeasureBar.h"
#include "Editors/GuitarEditor.h"
#include "AriaCore.h"

#include <iostream>
#include "wx/wx.h"
#include "wx/print.h"
#include "wx/printdlg.h"
#include <cmath>


namespace AriaMaestosa
{

int repetitionMinimalLength = 2;
    
int getRepetitionMinimalLength()
{
    return repetitionMinimalLength;
}
void setRepetitionMinimalLength(const int newvalue)
{
    repetitionMinimalLength = newvalue;
}

MeasureToExport::MeasureToExport()
{
    shortestDuration = -1;
    firstSimilarMeasure = -1;
    cutApart = false;
}

void MeasureToExport::setID(int id_arg)
{
    id = id_arg;
}

// FIXME - used at all?
bool MeasureToExport::isSameAs(MeasureToExport& compareWith)
{
    return (compareWith.firstSimilarMeasure == firstSimilarMeasure) or (compareWith.firstSimilarMeasure == id);
}

// if a repetition is found, it is stored in the variables and returns true,
// otherwise returns false
bool MeasureToExport::findConsecutiveRepetition(std::vector<MeasureToExport>& measures, const int measureAmount,
                                                int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                                int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/)
                                                {
    //similarMeasuresFoundLater
    
    // check if it works with first measure occurence of similar measures
    if(id+1<measureAmount and measures[id+1].firstSimilarMeasure == measures[id].firstSimilarMeasure+1 )
    {
        int amount = 0;
        
        for(int iter=1; iter<measureAmount; iter++)
        {
            if(id+iter<measureAmount and measures[id+iter].firstSimilarMeasure == measures[id].firstSimilarMeasure+iter )
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
            std::cout << "		< lvl 2, testing measure " << checkFromMeasure << std::endl;
            //if(checkFromMeasure+1<id and measures[checkFromMeasure+1].firstSimilarMeasure ==
            //   measures[checkFromMeasure].firstSimilarMeasure+1 )
            //{
            int amount = 0;
                
                // check if there is a consecutive repetition with measures from this area
                
                for(int iter=0; iter</*id-checkFromMeasure*/measureAmount; iter++)
                {
                    if(not(checkFromMeasure+iter<id and
                           checkFromMeasure+iter<measureAmount and id+iter<measureAmount)) continue;
                    
                    // check if they are identical
                    
                    if( // they are identical because they both are repetitions of the same one
                        (measures[checkFromMeasure+iter].firstSimilarMeasure == measures[id+iter].firstSimilarMeasure and
                         measures[checkFromMeasure+iter].firstSimilarMeasure != -1)
                        or
                        // they are identical because the second is a repetition of the first
                        (checkFromMeasure+iter == measures[id+iter].firstSimilarMeasure)
                        )
                    {
                        std::cout << "			//" << (checkFromMeasure+iter+1) << " is same as " << (id+iter+1) << std::endl;
                        amount++;
                    }
                    else
                    {
                        std::cout << "			//but " << (checkFromMeasure+iter+1) << " is NOT same as " << (id+iter+1) << " (" << measures[checkFromMeasure+iter].firstSimilarMeasure+1 << " != " << measures[id+iter].firstSimilarMeasure+1 << ")" << std::endl;
                        break;
                    }
                }//next
                std::cout << "		> amount=" << amount << std::endl;
                if(amount<repetitionMinimalLength) continue;
                std::cout << "measure " << id+1  << " is a level 2 repetition" << std::endl;
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
                                                
bool MeasureToExport::calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure)
{
    
    // if these 2 measures don't even have the same number of notes, they're definitely not the same
    if(
       (checkMeasure.lastNote - checkMeasure.firstNote) != (lastNote - firstNote)
       ) return false;
    
    
    const int noteAmount = (checkMeasure.lastNote - checkMeasure.firstNote);
    
    
    if(noteAmount<1) return false; //empty measure
    
    /*
     if we get till there, the 2 measures have the same amount of notes.
     to know whether they are truly identitcal, we need to compare note by note
     we will match each notes from the first measure to the identical one in the second.
     If ever one note failes to be matched, then the 2 measures are different.
     */
    int noteMatched_this[noteAmount];
    int noteMatched_other[noteAmount];
    for(int n=0; n<noteAmount; n++)
    {
        noteMatched_this[n] = false;
        noteMatched_other[n] = false;
    }
    
    for(int checkNote_this=0; checkNote_this<noteAmount; checkNote_this++)
    {
        for(int checkNote_other=0; checkNote_other<noteAmount; checkNote_other++)
        {
            if(noteMatched_other[checkNote_other]) continue; // this note was already matched
            
            // check start tick matches
            if(track->getNoteStartInMidiTicks(checkMeasure.firstNote + checkNote_other) - checkMeasure.firstTick !=
               track->getNoteStartInMidiTicks(firstNote + checkNote_this) - firstTick)
            {
                // they dont match, check the next one
                continue;
            }
            
            // check end tick matches
            if(track->getNoteEndInMidiTicks(checkMeasure.firstNote + checkNote_other) - checkMeasure.firstTick !=
               track->getNoteEndInMidiTicks(firstNote + checkNote_this) - firstTick)
            {
                // they dont match, check the next one
                continue;
            }
            
            // check pitch matches
            if(track->getNotePitchID(checkMeasure.firstNote + checkNote_other) !=
               track->getNotePitchID(firstNote + checkNote_this))
            {
                // they dont match, check the next one
                continue;
            }
            
            noteMatched_this[checkNote_this] = true;
            noteMatched_other[checkNote_other] = true;
            
            
        }//next
        
        // we couldn't find a note in the other measure that matches this one
        if(noteMatched_this[checkNote_this] == false) return false;
        
    }//next
    
    return true;
}


// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility


LayoutElement::LayoutElement(LayoutElementType type_arg, int measure_arg)
{
    type = type_arg;
    measure = measure_arg;
}

void getLayoutElements(Track* track, const bool checkRepetitions_bool, std::vector<LayoutPage>& layoutPages, std::vector<MeasureToExport>& measures)
{
    const int measureAmount = getMeasureBar()->getMeasureAmount();
    int note=0;
    const int noteAmount = track->getNoteAmount();
    std::vector<LayoutElement> layoutElements;
    
   // MeasureToExport measures[measureAmount];
	
	// -------------------- gather measure information -------------------- 
	for(int measure=0; measure<measureAmount; measure++)
	{
        measures.push_back( MeasureToExport() );
		measures[measure].setID(measure);
		
		measures[measure].firstTick = getMeasureBar()->firstTickInMeasure( measure );
		measures[measure].lastTick = getMeasureBar()->lastTickInMeasure( measure );
		
		// first note in measure (which is also last note of previous measure, that was set in last iteration of the for loop)
		measures[measure].firstNote = note;
		
		// ------------------------------------ find what the first, last and shortest note in current measure --------------------------------------
		// find what is the shortest note in this measure. this will help determine how big it must be made.
		// to do this, iterate through notes, keeping the smallest length we find, stopping when we reach next measure
		// this by the way finds where the measure begins and end, thus filling up 'measureFirstNote' and 'measureLastNote' variables.
		for(; note<noteAmount; note++)
		{
			// stop when we're at next measure - it will be done in next measure iteration
			if( track->getNoteStartInMidiTicks(note) >= measures[measure].lastTick ) break;
			
			const int currentNoteDuration = track->getNoteEndInMidiTicks(note) - track->getNoteStartInMidiTicks(note);
			
			if(currentNoteDuration <= 0)  continue; // skip malformed notes if any
			if( currentNoteDuration < measures[measure].shortestDuration or measures[measure].shortestDuration==-1) measures[measure].shortestDuration = currentNoteDuration;
			
		}
		
		measures[measure].lastNote = note; // ID of the last note in this measure (or actually, first note in NEXT measure?? FIXME)
	}
	
    //-------------------- search for repeated measures if necessary  -------------------- 
	if(checkRepetitions_bool)
	{
		for(int measure=0; measure<measureAmount; measure++)
		{
			measures[measure].track = track;
			
			// check current measure against all previous measures to see if it is not a repetition
			for(int checkMeasure=0; checkMeasure<measure; checkMeasure++)
			{
				const bool isSameAs = measures[measure].calculateIfMeasureIsSameAs(measures[checkMeasure]);
				
				if(!isSameAs) continue;
				measures[measure].firstSimilarMeasure = checkMeasure;
				measures[checkMeasure].similarMeasuresFoundLater.push_back(measure);
				break;
			}//next
		}//next
		
	}//endif check repetitions
	
	// -------------------- calculate tablature layout ----------------------
	
	for(int measure=0; measure<measureAmount; measure++)
	{
#ifdef _verbose
		std::cout << (measure+1) << ":" << std::endl;
#endif
		int firstMeasureThatRepeats, lastMeasureThatRepeats, firstRepeatedMeasure, lastRepeatedMeasure; // used when finding repetitions
		
		// ----- empty measure -----
		if(measures[measure].shortestDuration==-1)
		{
			layoutElements.push_back( LayoutElement(EMPTY_MEASURE) );
		}
		
		// repetition
		else if(checkRepetitions_bool and measures[measure].firstSimilarMeasure!=-1)
		{
			
			if(getRepetitionMinimalLength()<2)
			{
				LayoutElement element(SINGLE_REPEATED_MEASURE, measure);
				layoutElements.push_back( element );
				continue;
			}
			
			// user requires that repetitions are longer than one measure
			// check if it is the case
			
			// -------- play same measure multiple times --------
			// check if next measure is the same as current measure
			if(measure+1<measureAmount and measures[measure+1].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
			{
				int amountOfTimes = 1;
				for(int iter=1; iter<measureAmount; iter++)
				{
					if(measure+iter<measureAmount and measures[measure+iter].firstSimilarMeasure == measures[measure].firstSimilarMeasure )
					{
						amountOfTimes++;
					}
					else
					{
						break;
					}
				}//next
				
				if(amountOfTimes < getRepetitionMinimalLength())
				{
#ifdef _verbose
					std::cout << "play many times refused, measures " << (measure+1) << " to " << (measure+amountOfTimes+1) << " are normal" << std::endl;
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
					if(measures[measure].firstSimilarMeasure != -1 and measures[measure].firstSimilarMeasure != measure-1)
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
					std::cout << "measure " << (measure+1) << " is played " << amountOfTimes << " times. all are the same as " << (measures[measure].firstSimilarMeasure+1) << std::endl;
#endif
					LayoutElement element(PLAY_MANY_TIMES);
					element.amountOfTimes = amountOfTimes;
					measures[measure].cutApart = true;
					if(measures[measure].firstSimilarMeasure == measure-1) measure = measure + amountOfTimes-2;
					else measure = measure + amountOfTimes-1;
					layoutElements.push_back( element );
				}
				
			}
			
			// ------- repeat a riff --------
			// check if next measure is a reptition, and check this repetition is the next one compared to the current repeated measure
			else if( measures[measure].findConsecutiveRepetition(measures, measureAmount, firstMeasureThatRepeats, lastMeasureThatRepeats,
																 firstRepeatedMeasure, lastRepeatedMeasure) )
			{
				
				const int amount = lastMeasureThatRepeats - firstMeasureThatRepeats;
				if(amount+1 >= getRepetitionMinimalLength())
				{
#ifdef _verbose
					std::cout << "repetition from " << (firstMeasureThatRepeats+1) << " to " <<
					(lastMeasureThatRepeats+1) << "(" << (firstRepeatedMeasure+1) << ", " <<
					(lastRepeatedMeasure+1) << ")"  << std::endl;
#endif
					
					LayoutElement element(REPEATED_RIFF);
					element.firstMeasure = firstMeasureThatRepeats;
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
					std::cout << "repetition refused because " << (amount+1) << " < " << getRepetitionMinimalLength() << " measures " << (measure+1) << " to " << (measure+getRepetitionMinimalLength()+1) << " are normal" << std::endl;
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
				std::cout << "measure " << (measure+1) << " looks like a repetition but treated as normal" << std::endl;
#endif
				// despite looking like a repetition, user settings are set to treat it as a normal measure
				layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
			}
			
		}
		else
			// ------ normal measure -----
		{
#ifdef _verbose
			std::cout << "measure " << (measure+1) << " is normal" << std::endl;
#endif
			layoutElements.push_back( LayoutElement(SINGLE_MEASURE, measure) );
		}
		
	}//next measure
    
    
    // calculate approximative width of each element
    const int ticksPerBeat = getCurrentSequence()->ticksPerBeat();
    const int layoutElementsAmount = layoutElements.size();
    for(int n=0; n<layoutElementsAmount; n++)
    {
        layoutElements[n].zoom = 1;
        layoutElements[n].charWidth = 2;
        
        if(layoutElements[n].type == SINGLE_MEASURE)
        {
            const int divider = (int)(
                                      getMeasureBar()->getTimeSigNumerator(layoutElements[n].measure) * (float)ticksPerBeat /
                                      (float)measures[layoutElements[n].measure].shortestDuration
                                      );
            
            // if notes are very long, zoom a bit because we don't want a too short measure
            if( divider <= 2 ) layoutElements[n].zoom = 4;
            if( divider <= 1 ) layoutElements[n].zoom = 8;
            
            layoutElements[n].charWidth = (int)round(
                  (float)(measures[layoutElements[n].measure].lastTick - measures[layoutElements[n].measure].firstTick) /
                (float)measures[layoutElements[n].measure].shortestDuration
                  )*layoutElements[n].zoom + 2;

        }
        else if(layoutElements[n].type == REPEATED_RIFF)
        {
            layoutElements[n].charWidth = 5;
        }
        
        std::cout << "$$ setting charwidth for element " << n << " : " << layoutElements[n].charWidth << std::endl;
    }
    
    // lay out in lines and pages
    layoutPages.push_back( LayoutPage() );
    
    int totalLength = 0;
    int currentLine = 0;
    int currentPage = 0;

    assertExpr(currentPage,<,layoutPages.size());
    layoutPages[currentPage].layoutLines.push_back( LayoutLine(track) );
    
    std::cout << "+ PAGE " << currentPage << std::endl;
    std::cout << "    + LINE " << currentLine << std::endl;
    
    for(int n=0; n<layoutElementsAmount; n++)
    {
        if(totalLength + layoutElements[n].charWidth > maxCharItemsPerLine)
        {
            // too much stuff on current line, switch to another line
            layoutPages[currentPage].layoutLines[currentLine].charWidth = totalLength;
            totalLength = 0;
            currentLine++;
            assertExpr(currentPage,<,layoutPages.size());
            
            // ccheck if we need to switch to another page
            if(layoutPages[currentPage].layoutLines.size() == maxLinesInPage)
            {
                // too many lines on page, switch to another page
                currentLine = 0;
                currentPage++;
                layoutPages.push_back( LayoutPage() );
                std::cout << "+ PAGE " << currentPage << std::endl;
            }
            assertExpr(currentPage,<,layoutPages.size());
            layoutPages[currentPage].layoutLines.push_back( LayoutLine(track) );
            std::cout << "    + LINE " << currentLine << std::endl;
        }
        assertExpr(currentLine,<,layoutPages[currentPage].layoutLines.size());
        layoutPages[currentPage].layoutLines[currentLine].layoutElements.push_back(layoutElements[n]);
        totalLength += layoutElements[n].charWidth;
    }
    // for last line processed
    layoutPages[currentPage].layoutLines[currentLine].charWidth = totalLength;

}

LayoutLine::LayoutLine(Track* parent)
{
    LayoutLine::parent = parent;
    
    editorMode = parent->graphics->editorMode;
    
    if(editorMode == GUITAR)
    {
        string_amount = parent->graphics->guitarEditor->tuning.size();
    }
}

#pragma mark -

/*
 * Shows a basic example of how to print stuff in wx.
 * Use the 'preview' feature here, actually printing this will take much of your ink ^^
 */


class QuickPrint : public wxPrintout
{
    wxPageSetupDialogData page_setup;
    int orient;
    int max_x, max_y;
    int pageAmount;
    
    static const int brush_size = 15;
    
    AriaPrintable* printable;
    
public:
    QuickPrint(AriaPrintable* printable) : wxPrintout( printable->getTitle() )
    {
        QuickPrint::printable = printable;
        pageAmount =  printable->getPageAmount();
        orient = printable->portraitOrientation() ? wxPORTRAIT : wxLANDSCAPE;
    }
    
    bool OnPrintPage(int pageNum)
    {
        std::cout << "printing page " << pageNum << std::endl;
        
        wxDC& dc = *GetDC();
        
        wxRect bounds = GetLogicalPageRect();
        
        const int x0 = bounds.x;
        const int y0 = bounds.y;
        const int width = bounds.width;
        const int height = bounds.height;
        const int x1 = x0 + width;
        const int y1 = y0 + height;
        
        std::cout << "printable area : (" << x0 << ", " << y0 << ") to (" << x1 << ", " << y1 << ")" << std::endl;
        printable->printPage(pageNum, dc, x0, y0, x1, y1, width, height);

        return true;
    }  
    
    bool preparePrint(const bool showPageSetupDialog=false)
    {
        wxPrintData printdata;
        printdata.SetPrintMode( wxPRINT_MODE_PRINTER );
        printdata.SetOrientation( orient ); // wxPORTRAIT, wxLANDSCAPE
        printdata.SetNoCopies(1);
        
        page_setup = wxPageSetupDialogData(printdata);
        //page_setup.SetMarginTopLeft(wxPoint(16, 16));
        //page_setup.SetMarginBottomRight(wxPoint(16, 16));
        
        if(showPageSetupDialog)
        {
            // let user change default values if he wishes to
            wxPageSetupDialog dialog( NULL,  &page_setup );
            if(dialog.ShowModal()==wxID_OK)
            {
                page_setup = dialog.GetPageSetupData();
                orient = page_setup.GetPrintData().GetOrientation();
            }
            else
            {
                std::cout << "user canceled at page setup dialog" << std::endl;
                return false;
            }
        }
        return true;
        
    }
    
    void OnBeginPrinting()
    {
        // set-up coordinate system however we want
        // we'll use it when drawing
        
        // here i'm using arbitrary an size, use whatever you wish
        if(orient == wxPORTRAIT)
        {
            max_x = 680;
            max_y = 880;
        }
        else 
        {
            max_x = 880;
            max_y = 680;
        }
        
        FitThisSizeToPageMargins(wxSize(max_x, max_y), page_setup);
    }
    
    bool OnBeginDocument(int startPage, int endPage)
    {
        std::cout << "beginning to print document, from page " << startPage << " to " << endPage << std::endl;
        return wxPrintout::OnBeginDocument(startPage, endPage);
    }
    
    void GetPageInfo(int *minPage, int *maxPage, int *pageSelFrom, int *pageSelTo)
    {
        *minPage = 1;
        *maxPage = pageAmount;
        
        *pageSelFrom = 1;
        *pageSelTo = pageAmount;
    }
    bool HasPage(int pageNum)
    {
        if(pageNum >= 1 and pageNum <= pageAmount)
            return true;
        else
            return false;
    }
    
    void OnEndPrinting()
    {
    }
};

bool printResult(AriaPrintable* printable)
{
    
    QuickPrint myprint( printable );
    wxPrinter printer;
    
    if(!myprint.preparePrint()) return false;
    const bool success = printer.Print(NULL, &myprint, true /* show dialog */);
    
    if(!success) return false;
    
    return true;
}

// methods to be overriden if needed
AriaPrintable::AriaPrintable()
{
}

AriaPrintable::~AriaPrintable()
{
}
wxString AriaPrintable::getTitle()
{
    return wxT("Aria Maestosa printed score");
}

int AriaPrintable::getPageAmount()
{
    return 1;
}
bool AriaPrintable::portraitOrientation()
{
    return true;
}
void AriaPrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
}

}
