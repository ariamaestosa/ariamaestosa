#include "Printing/PrintLayout.h"
#include "Printing/PrintingBase.h"

#include "Midi/Track.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Editors/GuitarEditor.h"
#include "AriaCore.h"

#include <iostream>
#include "wx/wx.h"
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

#pragma mark -

const MeasureToExport nullMeasure(-1);

MeasureToExport::MeasureToExport(const int measID)
{
    shortestDuration = -1;
    firstSimilarMeasure = -1;
    cutApart = false;
    id = measID;
    
    if(measID != -1)
    {
        firstTick = getMeasureData()->firstTickInMeasure( measID );
        lastTick = getMeasureData()->lastTickInMeasure( measID );
    }
}

// if a repetition is found, it is stored in the variables and returns true,
// otherwise returns false
bool MeasureToExport::findConsecutiveRepetition(ptr_vector<MeasureToExport>& measures, const int measureAmount,
                                                int& firstMeasureThatRepeats /*out*/, int& lastMeasureThatRepeats /*out*/,
                                                int& firstMeasureRepeated /*out*/, int& lastMeasureRepeated /*out*/)
                                                {
    
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
            //std::cout << "		< lvl 2, testing measure " << checkFromMeasure << std::endl;
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
                        //std::cout << "			//" << (checkFromMeasure+iter+1) << " is same as " << (id+iter+1) << std::endl;
                        amount++;
                    }
                    else
                    {
                        //std::cout << "			//but " << (checkFromMeasure+iter+1) << " is NOT same as " << (id+iter+1) << " (" << measures[checkFromMeasure+iter].firstSimilarMeasure+1 << " != " << measures[id+iter].firstSimilarMeasure+1 << ")" << std::endl;
                        break;
                    }
                }//next
                //std::cout << "		> amount=" << amount << std::endl;
                if(amount<repetitionMinimalLength) continue;
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
                                                
bool MeasureToExport::calculateIfMeasureIsSameAs(MeasureToExport& checkMeasure)
{
    
    const int trackRefAmount = trackRef.size();
    for(int tref=0; tref<trackRefAmount; tref++)
    {
        
        const int my_first_note = trackRef[tref].firstNote;
        const int my_last_note = trackRef[tref].lastNote;
        const int his_first_note = checkMeasure.trackRef[tref].firstNote;
        const int his_last_note = checkMeasure.trackRef[tref].lastNote;

        assert( trackRef.size() == checkMeasure.trackRef.size() );
        assertExpr( tref,<,(int)trackRef.size() );
        assert( trackRef[tref].track == checkMeasure.trackRef[tref].track );
        Track* track = trackRef[tref].track;
        
        // if these 2 measures don't even have the same number of notes, they're definitely not the same
        if(
           (his_last_note - his_first_note) != (my_last_note - my_first_note)
           ) return false;
        
        
        const int noteAmount = (his_last_note - his_first_note);
        
        
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
                if(track->getNoteStartInMidiTicks(his_first_note + checkNote_other) - checkMeasure.firstTick !=
                   track->getNoteStartInMidiTicks(my_first_note + checkNote_this) - firstTick)
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check end tick matches
                if(track->getNoteEndInMidiTicks(his_first_note + checkNote_other) - checkMeasure.firstTick !=
                   track->getNoteEndInMidiTicks(my_first_note + checkNote_this) - firstTick)
                {
                    // they dont match, check the next one
                    continue;
                }
                
                // check pitch matches
                if(track->getNotePitchID(his_first_note + checkNote_other) !=
                   track->getNotePitchID(my_first_note + checkNote_this))
                {
                    // they dont match, check the next one
                    continue;
                }
                
                noteMatched_this[checkNote_this] = true;
                noteMatched_other[checkNote_other] = true;
                
                
            }//next note
            
            // we couldn't find a note in the other measure that matches this one
            if(noteMatched_this[checkNote_this] == false) return false;
            
        }//next note
        
    } // next track reference
    
    return true;
}

int MeasureToExport::addTrackReference(const int firstNote, Track* track)
{
    MeasureTrackReference* newTrackRef = new MeasureTrackReference();
    newTrackRef->track = track;
    
    // first note in measure (which is also last note of previous measure, that was set in last iteration of the for loop)
    newTrackRef->firstNote = firstNote;

    // find what the first, last and shortest note in current measure
    const int noteAmount = track->getNoteAmount();
    int note=firstNote;
    for(; note<noteAmount; note++)
    {
        // stop when we're at next measure - it will be done in next measure iteration
        if( track->getNoteStartInMidiTicks(note) >= lastTick ) break;
        
        const int currentNoteDuration = track->getNoteEndInMidiTicks(note) - track->getNoteStartInMidiTicks(note);
        
        if(currentNoteDuration <= 0)  continue; // skip malformed notes if any
        if( currentNoteDuration < shortestDuration or shortestDuration==-1) shortestDuration = currentNoteDuration;
        
    }
    
    newTrackRef->lastNote = note; // ID of the last note in this measure (or actually, first note in NEXT measure?? FIXME)
    
    trackRef.push_back( newTrackRef );

    return newTrackRef->lastNote;
}

#pragma mark -

// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility
LayoutElement::LayoutElement(LayoutElementType type_arg, int measure_arg)
{
    type = type_arg;
    measure = measure_arg;
    
    x = -1;
    x2 = -1;
}

LayoutLine::LayoutLine(AriaPrintable* parent)
{
    printable = parent;
    currentTrack = 0;
}

int LayoutLine::getTrackAmount()
{
    // FIXME - make proper implementation where it can vary from line to line
    // if some lines are empty or identical
    return printable->tracks.size();
}
void LayoutLine::setCurrentTrack(const int n)
{
    currentTrack = n;
}
Track* LayoutLine::getTrack() const
{
    assertExpr(currentTrack,>=,0);
    assertExpr(currentTrack,<,printable->tracks.size());
    return printable->tracks.get(currentTrack);
}
int LayoutLine::getFirstNoteInElement(const int layoutElementID)
{
    return getMeasureForElement(layoutElementID).trackRef[currentTrack].firstNote;
}
int LayoutLine::getLastNoteInElement(const int layoutElementID)
{
    return getMeasureForElement(layoutElementID).trackRef[currentTrack].lastNote;
}
int LayoutLine::getFirstNoteInElement(LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[currentTrack].firstNote;
}
int LayoutLine::getLastNoteInElement(LayoutElement* layoutElement)
{
    return getMeasureForElement(layoutElement).trackRef[currentTrack].lastNote;
}

MeasureToExport& LayoutLine::getMeasureForElement(const int layoutElementID)
{
    const int measID = layoutElements[layoutElementID].measure;
    if(measID == -1) return (MeasureToExport&)nullMeasure;
    return printable->measures[measID];
}
MeasureToExport& LayoutLine::getMeasureForElement(LayoutElement* layoutElement)
{
    return printable->measures[layoutElement->measure];
}
int LayoutLine::getLastMeasure() const
{
    for(int n=layoutElements.size()-1; n>=0; n--)
    {
        if( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}
int LayoutLine::getFirstMeasure() const
{
    const int amount = layoutElements.size();
    for(int n=0; n<amount; n++)
    {
        if( layoutElements[n].measure != -1) return layoutElements[n].measure;
    }
    return -1;
}
int LayoutLine::getLastNote() const
{
    int answer = -1;
    const int first_measure = getFirstMeasure();
    const int last_measure = getLastMeasure();
    const int from_tick = getMeasureData()->firstTickInMeasure(first_measure);
    const int to_tick   = getMeasureData()->lastTickInMeasure(last_measure);
    const Track* t = getTrack();
    
    const int noteAmount = t->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        if(t->getNoteStartInMidiTicks(n) >= from_tick and t->getNoteStartInMidiTicks(n) < to_tick)
        {
            answer = n;
        }
        else if(answer != -1) return answer;
    }
    return answer;
}
int LayoutLine::getFirstNote() const
{
    const int measure = getFirstMeasure();
    const int from_tick = getMeasureData()->firstTickInMeasure(measure);
    const Track* t = getTrack();
    const int noteAmount = t->getNoteAmount();
    for(int n=0; n<noteAmount; n++)
    {
        if(t->getNoteStartInMidiTicks(n) >= from_tick)
        {
            return n;
        }
    }
    return -1;
}
int LayoutLine::calculateHeight()
{
    level_height = 0;
    
    std::vector<int> heights;
    
    /* calculate the total height of this line (which many include multiple tracks */
    std::cout << "---- line ----" << std::endl;
    const int trackAmount = getTrackAmount();
    for(int n=0; n<trackAmount; n++)
    {
        setCurrentTrack(n);
        const int this_height = printable->editorPrintables.get(currentTrack)->calculateHeight(*this);
        heights.push_back(this_height);
        level_height += this_height;
        std::cout << this_height << "-high" << std::endl;
    }
    
    /* distribute the vertical space between tracks (some track need more vertical space than others) */
    for(int n=0; n<trackAmount; n++)
    {
        height_percent.push_back( (int)round( (float)heights[n] * 100.0f / (float)level_height ) );
        std::cout << height_percent[n] << "%" << std::endl;
    }
    
    return level_height;
}
void LayoutLine::printYourself(wxDC& dc, const int x0, const int y0, const int x1, const int y1)
{
    const int trackAmount = getTrackAmount();
    
    const float height = (float)(y1 - y0) * ( trackAmount>1 ? 0.9f : 1.0f);
    
    float current_y = y0;
    for(int n=0; n<trackAmount; n++)
    {
        setCurrentTrack(n);
        EditorPrintable* track = printable->editorPrintables.get(currentTrack);
        
        // determine how much vertical space is allocated for this track
        const float track_height = height * height_percent[n]/100.0f;
        
        std::cout << "* allocating " <<track_height << " out of " << height << " (" << height_percent[n] << "%)" << std::endl;
        
        // draw vertical grey lines to show these measures belong toghether
        if(n>1)
        {
            dc.SetPen(  wxPen( wxColour(150,150,150), 3 ) );
            dc.DrawLine( x0+3, y0, x0+3, y1);
            dc.DrawLine( x1-3, y0, x1-3, y1);
        }
        
        track->drawLine(*this, dc, x0, current_y, x1, current_y+(track_height-220), n==0);
        current_y += track_height;
        assertExpr(current_y,<=,y1);
    }
}

#pragma mark -

void generateMeasures(ptr_vector<Track, REF>& tracks, ptr_vector<MeasureToExport>& measures)
{
    const int trackAmount = tracks.size();
    const int measureAmount = getMeasureData()->getMeasureAmount();
    
    for(int tr=0; tr<trackAmount; tr++)
    {
        
        Track* track = tracks.get(tr);
        
        // add measures
        for(int measure=0; measure<measureAmount; measure++)
        {
             measures.push_back( new MeasureToExport(measure) );
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

void findSimilarMeasures(ptr_vector<MeasureToExport>& measures)
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
            
            if(!isSameAs) continue;
            measures[measure].firstSimilarMeasure = checkMeasure;
            measures[checkMeasure].similarMeasuresFoundLater.push_back(measure);
            break;
        }//next
    }//next
}

void generateOutputOrder(std::vector<LayoutElement>& layoutElements, ptr_vector<MeasureToExport>& measures, bool checkRepetitions_bool)
{
    const int measureAmount = getMeasureData()->getMeasureAmount();
    
    for(int measure=0; measure<measureAmount; measure++)
	{
#ifdef _verbose
		std::cout << (measure+1) << ":" << std::endl;
#endif
		int firstMeasureThatRepeats, lastMeasureThatRepeats, firstRepeatedMeasure, lastRepeatedMeasure; // used when finding repetitions
		
		// ----- empty measure -----
		if(measures[measure].shortestDuration==-1)
		{
			layoutElements.push_back( LayoutElement(EMPTY_MEASURE, measure) );
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
}

void calculateRelativeLengths(std::vector<LayoutElement>& layoutElements, ptr_vector<MeasureToExport>& measures)
{
    // calculate approximative width of each element
    const int ticksPerBeat = getCurrentSequence()->ticksPerBeat();
    const int layoutElementsAmount = layoutElements.size();
    for(int n=0; n<layoutElementsAmount; n++)
    {
        layoutElements[n].zoom = 1;
        layoutElements[n].width_in_units = 2;
        
        if(layoutElements[n].type == SINGLE_MEASURE)
        {
            const int divider = (int)(
                                      getMeasureData()->getTimeSigNumerator(layoutElements[n].measure) * (float)ticksPerBeat /
                                      (float)measures[layoutElements[n].measure].shortestDuration
                                      );
            
            // if notes are very long, zoom a bit because we don't want a too short measure
            if( divider <= 2 ) layoutElements[n].zoom = 2;
            if( divider <= 1 ) layoutElements[n].zoom = 4;
            
            // for very short notes, zooma bit too otherwise they'll all be stuck toghether
            if( divider >= 16 ) layoutElements[n].zoom = 2;
            
            layoutElements[n].width_in_units = (int)round(
                                                     (float)(measures[layoutElements[n].measure].lastTick - measures[layoutElements[n].measure].firstTick) /
                                                     (float)measures[layoutElements[n].measure].shortestDuration
                                                     )*layoutElements[n].zoom + 2;
            
        }
        else if(layoutElements[n].type == REPEATED_RIFF)
        {
            layoutElements[n].width_in_units = 5;
        }
        
        //std::cout << "$$ setting charwidth for element " << n << " : " << layoutElements[n].width_in_units << std::endl;
    }        
}

void calculateLineLayout(std::vector<LayoutLine>& layoutLines,
                         std::vector<LayoutPage>& layoutPages,
                         std::vector<LayoutElement>& layoutElements)
{
    const int layoutElementsAmount = layoutElements.size();
    
    int current_width = 0;
    int current_height = 0;
    
    int current_page = 0;
    layoutPages.push_back( LayoutPage() );
    layoutPages[current_page].first_line = 0;
    
    layoutLines.push_back( LayoutLine(getCurrentPrintable()) );
    int currentLine = 0;
    
    LayoutElement el(LayoutElement(LINE_HEADER, -1));
    el.width_in_units = 2;
    layoutLines[currentLine].layoutElements.push_back( el );
    
    // add layout elements one by one, switching to the next line when there's too many
    // elements on the current one
    for(int n=0; n<layoutElementsAmount; n++)
    {
        if(current_width + layoutElements[n].width_in_units > maxCharItemsPerLine)
        {
            // too much stuff on current line, switch to another line
            layoutLines[currentLine].width_in_units = current_width;
            current_width = 0;
            const int line_height = layoutLines[currentLine].calculateHeight();
            current_height += line_height;
            currentLine++;
            
           // std::cout << "    adding a new line : " <<  currentLine << std::endl;
            
            if(current_height > 50)
            {
                layoutPages[current_page].last_line = currentLine-1;
                current_height = 0;
                layoutPages.push_back( LayoutPage() );
                current_page++;
               // std::cout << "adding a new page : " <<  current_page << std::endl;
                layoutPages[current_page].first_line = currentLine;
            }
            
            layoutLines.push_back( LayoutLine(getCurrentPrintable()) );
        }
        assertExpr(currentLine,<,(int)layoutLines.size());
        layoutLines[currentLine].layoutElements.push_back(layoutElements[n]);
        current_width += layoutElements[n].width_in_units;
    }
    // for last line processed
    layoutLines[currentLine].width_in_units = current_width;
    layoutLines[currentLine].calculateHeight();
    layoutPages[current_page].last_line = currentLine;
}

void calculateLayoutElements(ptr_vector<Track, REF>& track, const bool checkRepetitions_bool,
                             std::vector<LayoutLine>& layoutLines, std::vector<LayoutPage>& layoutPages,
                             ptr_vector<MeasureToExport>& measures)
{
    std::vector<LayoutElement> layoutElements;
    
    generateMeasures(track, measures);
    
    // search for repeated measures if necessary
	if(checkRepetitions_bool) findSimilarMeasures(measures);
	
	generateOutputOrder(layoutElements, measures, checkRepetitions_bool);
    calculateRelativeLengths(layoutElements, measures);
    calculateLineLayout(layoutLines, layoutPages, layoutElements);
}




}
