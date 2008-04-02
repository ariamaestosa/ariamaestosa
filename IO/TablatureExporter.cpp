
#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/file.h>

#include "Midi/Sequence.h"
#include "AriaCore.h"

#include "GUI/MeasureBar.h"
#include "Editors/GuitarEditor.h"
#include "IO/IOUtils.h"
#include "IO/TablatureExporter.h"
#include "IO/NotationExport.h"

namespace AriaMaestosa
{
    
    
TablaturePrintable::TablaturePrintable(Track* track, bool checkRepetitions_bool)
{

    getLayoutElements(track, checkRepetitions_bool, layoutPages, measures);
    
    std::cout << "\nContains " << layoutPages.size() << " pages\n" << std::endl;
}

TablaturePrintable::~TablaturePrintable()
{
}

wxString TablaturePrintable::getTitle()
{
    return wxT("AriaMaestosa tablature");
}
int TablaturePrintable::getPageAmount()
{
    return layoutPages.size();
}
void TablaturePrintable::printPage(const int pageNum, wxDC& dc, const int x0, const int y0, const int x1, const int y1, const int w, const int h)
{
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    
    dc.SetFont( wxFont(11,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL) );
    
    wxString label( getTitle() + wxT(", page ") );
    label << pageNum;
    dc.SetTextForeground( wxColour(0,0,0) );
    dc.DrawText( label, x0, y0 );
    
    wxCoord txw, txh, descent, externalLeading;
    dc.GetTextExtent(label, &txw, &txh, &descent, &externalLeading);
    text_height = txh;
    text_height_half = (int)round((float)text_height / 2.0);
    
    // -------------------- generate the tablature  -------------------- 
    assertExpr(pageNum-1,<,layoutPages.size());
    const int lineAmount = layoutPages[pageNum-1].layoutLines.size();
    
    /*
     the equivalent of 4 times "text_height" will not be printed with notation.
     first : space for title at the top
     second and third : space below title at top
     fourth : space below the last line
     */
     
    const float line_height = ((float)h - (float)text_height*4) / (float)maxLinesInPage;

    const int notation_area_origin_y = y0 + text_height*3;
    for(int l=0; l<lineAmount; l++)
    { 
        const float line_y_from = notation_area_origin_y + line_height*l;
        const float line_y_to = notation_area_origin_y + line_height*(l+0.6f);
        
        drawLine(layoutPages[pageNum-1].layoutLines[l], dc, x0, (int)round(line_y_from), x1, (int)round(line_y_to));
    }
}

void TablaturePrintable::drawLine(LayoutLine& line, wxDC& dc, const int x0, const int y0, const int x1, const int y1)
{
    static const int line_width = 1;
    
    Track* track = line.parent;
    
    // draw tab background
    dc.SetPen(  wxPen( wxColour(125,125,125), line_width ) );
    
    std::cout << "X range :" << x0 << " " << x1 << std::endl;
    
    const int stringAmount = line.string_amount;
    const float stringHeight = (float)(y1 - y0) / (float)(stringAmount-1);
    
    for(int s=0; s<stringAmount; s++)
    {
        const int y = (int)round(y0 + stringHeight*s);
        dc.DrawLine(x0, y, x1, y);
    }
    
    std::vector<LayoutElement>& layoutElements = line.layoutElements;
    
    const int layoutElementsAmount = layoutElements.size();
    
    int xloc = 2;
    
    // 2 spaces allocated for left area of the tab
    float widthOfAChar = (float)(x1 - x0) / (float)(line.charWidth+2);
    
        
    for(int n=0; n<layoutElementsAmount; n++)
    {
        std::cout << "element " << n << " width=" << layoutElements[n].charWidth << std::endl;
        
        const int meas_x_start = x0 + (int)round(xloc*widthOfAChar) - widthOfAChar;
        const int meas_x_end = x0 + (int)round((xloc+layoutElements[n].charWidth)*widthOfAChar);
        const int mesa_w = meas_x_end - meas_x_start;
        
        // draw vertical line that starts measure
        dc.SetPen(  wxPen( wxColour(0,0,0), line_width*2 ) );
        dc.DrawLine( meas_x_start, y0, meas_x_start, y1);
        
        dc.SetTextForeground( wxColour(0,0,255) );

        // ****** empty measure
        if(layoutElements[n].type == EMPTY_MEASURE)
        {
            
        } 
        // ****** repetitions
        else if(layoutElements[n].type == SINGLE_REPEATED_MEASURE or layoutElements[n].type == REPEATED_RIFF)
        {
            // FIXME - why do we I apart the measure and not the layout element
            if(measures[layoutElements[n].measure].cutApart)
			{
                // TODO...
			}
			
			wxString message;
			if(layoutElements[n].type == SINGLE_REPEATED_MEASURE)
			{
				message = to_wxString(measures[layoutElements[n].measure].firstSimilarMeasure+1);
			}
			else if(layoutElements[n].type == REPEATED_RIFF)
			{
				message =	to_wxString(layoutElements[n].firstMeasureToRepeat+1) +
				wxT(" - ") + 
				to_wxString(layoutElements[n].lastMeasureToRepeat+1);
			}
            
            dc.DrawText( message, meas_x_start + widthOfAChar/2, (y0+y1)/2-text_height_half );
        }
        // ****** play again
        else if(layoutElements[n].type == PLAY_MANY_TIMES)
        {
            wxString label(wxT("X"));
            label << layoutElements[n].amountOfTimes;
            dc.DrawText( label, meas_x_start + widthOfAChar/2, (y0+y1)/2-text_height_half );
        }
        // ****** normal measure
        else if(layoutElements[n].type == SINGLE_MEASURE)
        {  
            // draw measure ID
            wxString measureLabel;
            measureLabel << (measures[layoutElements[n].measure].id+1);
            dc.DrawText( measureLabel, meas_x_start - widthOfAChar/2, y0 - text_height*1.2 );
            
            dc.SetTextForeground( wxColour(0,0,0) );
            

            const int firstNote = measures[layoutElements[n].measure].firstNote;
            //const int lastNote = measures[layoutElements[n].measure].lastNote;
            const int firstTick = measures[layoutElements[n].measure].firstTick;
            const int lastTick = measures[layoutElements[n].measure].lastTick;
            
            int lastNote = measures[layoutElements[n].measure].lastNote;
            
            // TODO : imcomplete lines don't render correctly
            
            for(int i=firstNote; i<lastNote; i++)
            {
                const int tick = track->getNoteStartInMidiTicks(i);
                const int string = track->getNoteString(i);
                const int fret = track->getNoteFret(i);
                
                const float nratio = ((float)(tick - firstTick) / (float)(lastTick - firstTick));
               //if(nratio < 0 or nratio > 1) std::cout << "note ratio : " << nratio <<
                //    " firstTick=" << firstTick << " lastTick=" << lastTick << " tick=" << tick << std::endl;
                
               // std::cout << "note ratio : " << nratio << " lastTick=" << lastTick << " tick=" << tick << std::endl;
                
                if(fret < 0)  dc.SetTextForeground( wxColour(255,0,0) );
                
                // substract from width to leave some space on the right (coordinate is from the left of the text string so we need extra space on the right)
                // if fret number is greater than 9, the string will have two characters so we need to recenter it a bit more
                const int drawX = nratio * (mesa_w-widthOfAChar*1.5) + meas_x_start + (fret > 9 ? widthOfAChar/4 : widthOfAChar/2);
                const int drawY = y0 + stringHeight*string - text_height_half*0.8;
                wxString label = to_wxString(fret);
                
                dc.DrawText( label, drawX, drawY );
                
                if(fret < 0)  dc.SetTextForeground( wxColour(0,0,0) );
            }
        }
        
        xloc += layoutElements[n].charWidth;
        
    }//next element
}



#ifdef _DISABLED_
TablatureExporter::TablatureExporter()
{
	INIT_LEAK_CHECK();	
	max_length_of_a_line = 110;
}

void TablatureExporter::setMaxLineWidth(int i)
{
	max_length_of_a_line = i;
}

void TablatureExporter::flush()
{
	file->Write( title_line + wxT("\n") );
	for(int s=0; s<string_amount; s++)
	{
		file->Write( strings[s] + wxT("\n"));
		strings[s] = wxT("|");	;
	}
	// for last string, skip one more line
	file->Write( wxT("\n") );
	measures_appended = 0;
	charAmountWhenBeginningMeasure = 2;
	title_line = wxT("");
}

void TablatureExporter::exportTablature(Track* track_arg, wxFile* file_arg, bool checkRepetitions_bool_arg)
{
    TablaturePrintable tabPrint;
    if(!printResult(&tabPrint))
    {
        std::cout << "failure!" << std::endl;
    }
    return;

	checkRepetitions_bool = checkRepetitions_bool_arg;
	file = file_arg;
	track = track_arg;
	
	measureAmount = getMeasureBar()->getMeasureAmount();
	ticksPerBeat = getCurrentSequence()->ticksPerBeat();
	//noteAmount = track->getNoteAmount();
	editor = track->graphics->guitarEditor;
	string_amount = editor->tuning.size();
	
	//int note=0;
	
	measures_appended = 0;
	for(int i=0; i<string_amount; i++) strings.push_back( wxT("") );
	
	for(int s=0; s<string_amount; s++)
	{
		strings[s] = wxT("|");	
	}
	title_line = wxT("");
/*	
	MeasureToExport measures[measureAmount];
	
	// -------------------- gather measure information -------------------- 
	for(int measure=0; measure<measureAmount; measure++)
	{
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
			
			if(currentNoteDuration <= 0)  continue; // skpi malformed notes if any
			if( currentNoteDuration < measures[measure].shortestDuration or measures[measure].shortestDuration==-1) measures[measure].shortestDuration = currentNoteDuration;
			
		}
		
		measures[measure].lastNote = note; // ID of the last note in this measure
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
	
	std::vector<LayoutElement> layoutElements;
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
	*/
    
    std::vector<LayoutLine> layoutLines;
    MeasureToExport measures[measureAmount];
    getLayoutElements(track, checkRepetitions_bool, layoutLines, measures);
	// -------------------- generate the tablature  -------------------- 
	const int layoutElementsAmount = layoutElements.size();

	for(int n=0; n<layoutElementsAmount; n++)
	{
	
		zoom = 1;
		// contains the number of characters each line contained, before adding new ones for this measure.
		charAmountWhenBeginningMeasure = -1; // will be set later, after adding the "|-" of the measure beginning
		added_chars = 0;
		
		// ****** empty measure
		if(layoutElements[n].type == EMPTY_MEASURE)
		{
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("--|"));
			}
			title_line.Append(wxT("   "));
			measures_appended++;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + 3 > max_length_of_a_line)
			{
				flush();
			}
			
			continue;
		} 
		
		// ****** repetitions
		else if(layoutElements[n].type == SINGLE_REPEATED_MEASURE or layoutElements[n].type == REPEATED_RIFF)
		{
			// if measure must be set apart from others
			// if it is the first of its line, ignore that, it's already apart
			if(measures[layoutElements[n].measure].cutApart and measures_appended>0)
			{
				for(int s=0; s<string_amount; s++)
				{
					strings[s].Append(wxT("|"));
				}
			}
			
			wxString message;
			if(layoutElements[n].type == SINGLE_REPEATED_MEASURE)
			{
				message = to_wxString(measures[layoutElements[n].measure].firstSimilarMeasure+1);
				measures_appended++;
			}
			else if(layoutElements[n].type == REPEATED_RIFF)
			{
				message =	to_wxString(layoutElements[n].firstMeasureToRepeat+1) +
				wxT(" - ") + 
				to_wxString(layoutElements[n].lastMeasureToRepeat+1);
				measures_appended += (layoutElements[n].lastMeasure - layoutElements[n].firstMeasure);
			}
			const int chars_in_message = message.Length();
			
			// check if we reached maximal line width
			const int needed_length = chars_in_message + 4;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			for(int n=0; n<string_amount; n++)
			{
				if(n == string_amount/2) strings[n].Append( wxT(" *") + message + wxT(" |") );
				else strings[n].Append('-', chars_in_message+3 ).Append(wxT("|"));
			}
			title_line.Append(' ', chars_in_message+4 );
		}
		// ****** play again
		else if(layoutElements[n].type == PLAY_MANY_TIMES)
		{
			wxString message;
			message = to_wxString(layoutElements[n].amountOfTimes);
			measures_appended += layoutElements[n].amountOfTimes;
			const int chars_in_message = message.Length();
			
			// check if we reached maximal line width
			const int needed_length = chars_in_message + 4;
			
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			for(int n=0; n<string_amount; n++)
			{
				if(n == string_amount/2) strings[n].Append( wxT(" x") + message + wxT(" |") );
				else strings[n].Append(' ', chars_in_message+3 ).Append(wxT("|"));
			}
			title_line.Append(' ', chars_in_message+4 );
			
		}
		// ****** normal measure
		else if(layoutElements[n].type == SINGLE_MEASURE)
		{
			assertExpr(measures[layoutElements[n].measure].shortestDuration,!=,0);
			
			const int divider = (int)(
									  getMeasureBar()->getTimeSigNumerator(layoutElements[n].measure) * (float)ticksPerBeat /
									  (float)measures[layoutElements[n].measure].shortestDuration
									  );
			
			// if notes are very long, zoom a bit because we don't want a too short measure
			if( divider <= 2 ) zoom = 4;
			if( divider <= 1 ) zoom = 8;
			
			// ------------------------------------ check for end of line --------------------------------------
			// if current line is full (i.e. no more horizontal space), output current line to file and prepare a new line
			
			// check how much place the next measure would take
			const int needed_length = (int)(
											divider * getMeasureBar()->getTimeSigNumerator(layoutElements[n].measure)  / 2.0
											);
			
			// if it takes too much space, end the current line and begin a new one
			if( measures_appended > 0 and (int)strings[0].Length() + needed_length > max_length_of_a_line)
			{
				flush();
			}
			
			// ------------------------------------ begin the new measure --------------------------------------
			// if measure must be set apart from others
			// if it is the first of its line, ignore that, it's already apart
			if(measures[layoutElements[n].measure].cutApart and measures_appended>0)
			{
				for(int s=0; s<string_amount; s++)
				{
					strings[s].Append(wxT("|"));
				}
			}
			
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("-"));
			}
			charAmountWhenBeginningMeasure = strings[0].Length();
			
			// fret number higher than 9 take more place, so we one is met, aditionnal room is added
			// this bool remembers whether we already met one of these fret numbers
			bool found_fret_higher_than_9=false;

			// ------------------------------------ write notes on the strings --------------------------------------
			int currentStep = 0;
			// use the IDs of the first and the last note in the measure we previously found, and iterate through all the notes in the measure
			for(int note=measures[layoutElements[n].measure].firstNote; note<measures[layoutElements[n].measure].lastNote; note++)
			{
				// gather information about current note
				const int note_fret = track->getNoteFret(note);
				const int note_string = track->getNoteString(note);
				
				// here, what i call "step" is a "-" followed by a number or another "-". e.g. tab |-2-3-4---5-| contains 5 steps.
				// think of it as X Position, but counted in characters
				const int note_step = (int)(
											(float)divider * zoom *
											(float)(track->getNoteStartInMidiTicks(note) - (measures[layoutElements[n].measure].firstTick) ) /
											(float)getMeasureBar()->measureLengthInTicks( layoutElements[n].measure )
											);
				
				// if current note is farther than current step, add "-"s until we reach the position of the current's note step
				if(note_step > currentStep)
				{
					// we're moving to a new lcoation, so reset this variable to be able to find more 2-digit fret numbers
					found_fret_higher_than_9 = false;
					
					// calculate how many characters we need on each string to go to next note
					const int wantedStep = currentStep + (note_step - currentStep);
					
					const int wantedChars = charAmountWhenBeginningMeasure + wantedStep*2 + added_chars;
					for(int s=0; s<string_amount; s++)
					{
						while( (int)strings[s].Length() < wantedChars )
						{
							strings[s].Append(wxT("-"));
						}
					}
					currentStep = wantedStep;
				}
				
				// if fret number is greater than 9, it takes 2 digits so we need to make more place for it
				if(note_fret > 9 and !found_fret_higher_than_9)
				{
					added_chars++;
					found_fret_higher_than_9 = true;
				}
				strings[note_string].Append( to_wxString(note_fret)  );
			}//next

			// ------------------------------------ end of measure --------------------------------------
			// fill all lines that are not already full with "-"s
			const int wantedChars = charAmountWhenBeginningMeasure + divider * 2 * zoom + added_chars;
			for(int s=0; s<string_amount; s++)
			{
				while( (int)strings[s].Length() < wantedChars) strings[s].Append(wxT("-"));
			}
			
			// ------------------------------------ title line --------------------------------------
			// create title line for this measure
			wxString measureTitle = to_wxString(layoutElements[n].measure+1);
			title_line.Append( measureTitle );
			const int title_length = measureTitle.Length()/* + 1 */;
			const int chars_in_measure = (divider * 2 * zoom + added_chars);
			
			for(int i=title_length; i<chars_in_measure; i++)
			{
				title_line.Append(wxT(" "));	
			}
			while( (int)title_line.Length() < wantedChars ) title_line.Append(wxT(" "));
			
			currentStep = 0;
			
			// end measure with a vertical bar
			for(int s=0; s<string_amount; s++)
			{
				strings[s].Append(wxT("|"));
			}
			
			measures_appended++;
		}
		
	}

	// output to file anything that remains
	flush();
	
	
}

#endif

}
