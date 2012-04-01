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

#include "AriaCore.h"

#include "Analysers/ScoreAnalyser.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "Editors/GuitarEditor.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/GraphicalTrack.h"
#include "IO/IOUtils.h"
#include "PreferencesData.h"
#include "Printing/AriaPrintable.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutAbstract.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutMeasure.h"
#include "Printing/SymbolPrinter/PrintLayout/PrintLayoutLine.h"
#include "Printing/SymbolPrinter/PrintLayout/RelativePlacementManager.h"
#include "Printing/SymbolPrinter/TabPrint.h"
#include "Printing/RenderRoutines.h"

#include <wx/dc.h>

using namespace AriaMaestosa;
 
const int CHAR_MARGIN = 35;

// -----------------------------------------------------------------------------------------------------------

TablaturePrintable::TablaturePrintable(GraphicalTrack* track) : EditorPrintable(track->getTrack())
{
    m_string_amount = track->getTrack()->getGuitarTuning()->tuning.size();
    m_editor        = track->getGuitarEditor();
}

// -----------------------------------------------------------------------------------------------------------

TablaturePrintable::~TablaturePrintable()
{
}

// -----------------------------------------------------------------------------------------------------------

void TablaturePrintable::earlySetup(const int trackID, GraphicalTrack* gtrack)
{
    m_analyser = new ScoreAnalyser(m_editor, -1);
    
    Track* track = gtrack->getTrack();
    
    MeasureData* md = track->getSequence()->getMeasureData();
    
    const int trackAmount = track->getNoteAmount();
    for (int n=0; n<trackAmount; n++)
    {
        PitchSign note_sign = PITCH_SIGN_NONE;
        const int noteLevel = 0;
        
        if (noteLevel == -1) continue;
        const int noteLength = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
        const int tick       = track->getNoteStartInMidiTicks(n);
        
        NoteRenderInfo currentNote = NoteRenderInfo::factory(tick, noteLevel, noteLength, note_sign,
                                                             track->isNoteSelected(n),
                                                             track->getNotePitchID(n), md);
        
        m_analyser->addToVector(currentNote);
    }
    m_analyser->putInTimeOrder();
    
    class TabSilenceProxy : public SilenceAnalyser::INoteSource
    {
        ScoreAnalyser* m_analyser;
        Sequence* m_seq;
        
    public:
        
        TabSilenceProxy(ScoreAnalyser* analyser, Sequence* seq)
        {
            m_analyser = analyser;
            m_seq = seq;
        }
        
        /** @return the number of notes that can be obtained through this interface */
        virtual int getNoteCount() const
        {
            return m_analyser->m_note_render_info.size();
        }
        
        /**
         * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
         * @return       the ID of the measure in which 'noteID' starts
         */
        virtual int getBeginMeasure(const int noteID) const
        {
            return m_seq->getMeasureData()->measureAtTick( m_analyser->m_note_render_info[noteID].getTick() );
        }
        
        /**
         * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
         * @return       the tick at which 'noteID' starts
         */
        virtual int  getStartTick(const int noteID) const
        {
            return m_analyser->m_note_render_info[noteID].getTick();
        }
        
        /**
         * @param noteID ID of the note you want info on, must be in range [0 .. getNoteCount()-1]
         * @return       the tick at which 'noteID' ends
         */
        virtual int getEndTick(const int noteID) const
        {
            return m_analyser->m_note_render_info[noteID].getTick() +
                   m_analyser->m_note_render_info[noteID].getTickLength();
        }
    };
    
    TabSilenceProxy* adapter = new TabSilenceProxy(m_analyser, track->getSequence());
    m_silences =  SilenceAnalyser::findSilences(track->getSequence(),
                                                adapter,
                                                0 /* first measure */,
                                                md->getMeasureAmount() - 1 /* last measure */,
                                                -1 /* y not important at this point */ );
    delete adapter;
}

// -----------------------------------------------------------------------------------------------------------
    
void TablaturePrintable::addUsedTicks(const PrintLayoutMeasure& measure,  const int trackID,
                                      MeasureTrackReference& trackRef,
                                      RelativePlacementManager& ticks_relative_position)
{
    //const int first_note = trackRef.getFirstNote();
    //const int last_note  = trackRef.getLastNote();
    const int firstTickInMeasure = measure.getFirstTick();
    const int lastTickInMeasure  = measure.getLastTick();
    
    //if (not empty_measure)
    {
        const int characterWidth = AriaPrintable::getCurrentPrintable()->getCharacterWidth();
        
        // ---- notes
        const int noteAmount = m_analyser->getNoteCount();
        for (int n=0; n<noteAmount; n++)
        {
            const int tick   = m_analyser->getStartTick(n);
            
            if (tick < firstTickInMeasure or tick >= lastTickInMeasure) continue;
            
            const int tickTo = m_analyser->getEndTick(n);
            //const int fret   = m_analyser->getNoteFretConst(n);

            //ticks_relative_position.addSymbol( tick, tickTo,
            //                                  (fret > 9 ? characterWidth*2 + CHAR_MARGIN : characterWidth + CHAR_MARGIN), trackID );
            ticks_relative_position.addSymbol( tick, tickTo, characterWidth*2 + CHAR_MARGIN, trackID );
        }
        
        /*
        for (int n=first_note; n<=last_note; n++)
        {
            const int tick   = track->getNoteStartInMidiTicks(n);
            const int tickTo = track->getNoteEndInMidiTicks(n);
            const int fret   = track->getNoteFretConst(n);
            
            //int noteLen = track->getNoteEndInMidiTicks(n) - track->getNoteStartInMidiTicks(n);
            
            ticks_relative_position.addSymbol( tick, tickTo,
                                              (fret > 9 ? characterWidth*2 : characterWidth), trackID );
        }
         */
    }
    
    /*
    std::cout << "--------\n";
    for (int n=0; n<m_silences.size(); n++)
    {
        std::cout << "TAB editor has silence tick " << m_silences[n].m_tick_range.from << "\n";
    }
    std::cout << "--------\n";
     */
    // ---- silences
    ticks_relative_position.addSilenceSymbols(m_silences, trackID,
                                              firstTickInMeasure, lastTickInMeasure);
}

// -----------------------------------------------------------------------------------------------------------

int TablaturePrintable::calculateHeight(const int trackID, LineTrackRef& lineTrack, LayoutLine& line,
                                        bool* empty)
{
    const int from_note = lineTrack.getFirstNote();
    const int to_note   = lineTrack.getLastNote();
    
    // check if empty
    //FIXME: if a note starts in previous line but ends in this one, track will still be reported empty
    //       even though it's not
    if (from_note == -1 or to_note == -1) *empty = true;
    else                                  *empty = false;

    return m_string_amount;
}  

// -----------------------------------------------------------------------------------------------------------

void TablaturePrintable::drawTrack(const int trackID, const LineTrackRef& currentTrack,
                                   LayoutLine& currentLine, wxDC& dc, wxGraphicsContext* gc,
                                   const bool drawMeasureNumbers)
{
    const TrackCoords* trackCoords = currentTrack.m_track_coords.raw_ptr;
    ASSERT(trackCoords != NULL);
    
    ASSERT_E(trackCoords->y0,>,0);
    ASSERT_E(trackCoords->y1,>,0);
    ASSERT_E(trackCoords->y0,<,50000);
    ASSERT_E(trackCoords->y1,<,50000);
    
    // draw tab background (guitar strings)
    dc.SetPen(  wxPen( wxColour(125,125,125), 5 ) );
    
    const float stringHeight = (float)(trackCoords->y1 - trackCoords->y0) / (float)(m_string_amount - 1);
    
    for (int s=0; s<m_string_amount; s++)
    {
        const int y = (int)round(trackCoords->y0 + stringHeight*s);
        dc.DrawLine(trackCoords->x0, y, trackCoords->x1, y);
    }
    
    setCurrentDC(&dc);
    
    // iterate through layout elements
    LayoutElement* currentElement;
    
    // draw end of line vertical line
    drawVerticalDivider(trackCoords->x1, trackCoords->y0, trackCoords->y1);

    // draw track name
    drawTrackName(dc, currentTrack, trackCoords->x0, trackCoords->y0, trackCoords->y1);
    
    const Track* track = currentTrack.getTrack()->getTrack();
    const MeasureData* md = track->getSequence()->getMeasureData();
    
    const int elementAmount = currentLine.getLayoutElementCount();
    for (int el=0; el<elementAmount; el++)
    {
        currentElement = &currentLine.getLayoutElement(el);
        drawElementBase(*currentElement, currentLine, drawMeasureNumbers,
                        trackCoords->y0, trackCoords->y1, trackCoords->y0, trackCoords->y1);
        
        drawVerticalDivider(currentElement, trackCoords->y0, trackCoords->y1);
        
        if (currentElement->m_render_end_bar)
        {
            drawVerticalDivider(currentElement, trackCoords->y0, trackCoords->y1, true /* at end */);
        }
        if (currentElement->getType() == LINE_HEADER)
        {
            dc.SetFont( getPrintTabHeaderFont() );
            dc.SetTextForeground( wxColour(0,0,0) );
            
            const int h4 = (trackCoords->y1 - trackCoords->y0)/3 - 2;    
            const int textY = trackCoords->y0;
            
            dc.DrawText( wxT("T") , currentElement->getXFrom()+20, textY);
            dc.DrawText( wxT("A") , currentElement->getXFrom()+20, textY + h4  );
            dc.DrawText( wxT("B") , currentElement->getXFrom()+20, textY + h4*2 );
            
            //wxSize textSize2 = dc.GetTextExtent( wxT("T") );
            
            dc.SetFont( getPrintFont() );
            
            // draw tuning
            const int tuning_x = currentElement->getXFrom()+140;
            for (int n=0; n<m_string_amount; n++)
            {
                const int note   = track->getConstGuitarTuning()->tuning[n]%12;
                wxString label;
                switch (note) //TODO: there is now a method to do this IIRC
                {
                    case 0:  label = wxT("B");  break;
                    case 1:  label = wxT("A#"); break;
                    case 2:  label = wxT("A");  break;
                    case 3:  label = wxT("G#"); break;
                    case 4:  label = wxT("G");  break;
                    case 5:  label = wxT("F#"); break;
                    case 6:  label = wxT("F");  break;
                    case 7:  label = wxT("E");  break;
                    case 8:  label = wxT("D#"); break;
                    case 9:  label = wxT("D");  break;
                    case 10: label = wxT("C#"); break;
                    case 11: label = wxT("C");  break;
                } // end switch
                dc.DrawText(label, tuning_x, trackCoords->y0 + n*stringHeight -
                            AriaPrintable::getCurrentPrintable()->getCharacterHeight()/2 );
            }//next
            
            continue;
        }
        if (currentElement->getType() == TIME_SIGNATURE_EL)
        {
            //std::cout << "Tablature : it's a time sig\n";
            
            EditorPrintable::renderTimeSignatureChange(currentElement, trackCoords->y0, trackCoords->y1);
            continue;
        }
        
        if (currentElement->getType() != SINGLE_MEASURE)
        {
            /*
             std::cout << "Tablature : it's something else we won't draw : ";
             if (currentElement->getType() == SINGLE_REPEATED_MEASURE) std::cout << "SINGLE_REPEATED_MEASURE\n";
             if (currentElement->getType() == EMPTY_MEASURE) std::cout << "EMPTY_MEASURE\n";
             if (currentElement->getType() == REPEATED_RIFF) std::cout << "REPEATED_RIFF\n";
             if (currentElement->getType() == PLAY_MANY_TIMES) std::cout << "PLAY_MANY_TIMES\n";
             */
            continue;
        }
        
        // for layout elements containing notes, render them
        const int firstNote = currentLine.getFirstNoteInElement(trackID, currentElement);
        const int lastNote = currentLine.getLastNoteInElement(trackID, currentElement);
        
        if (firstNote == -1 or lastNote == -1)
        {
            //std::cout << "Tablature : it's an empty measure\n";
            continue; // empty measure
        }
        
        dc.SetFont( AriaPrintable::getCurrentPrintable()->getNormalFont() );
        wxSize textSize3 = dc.GetTextExtent( wxT("X") );
        
        //std::cout << "Tablature : drawing notes " << firstNote << " to " << lastNote << std::endl;
        
        for (int i=firstNote; i<=lastNote; i++)
        {
            const int string = track->getNoteStringConst(i);
            const int fret   = track->getNoteFretConst(i);
            
            if (fret < 0)  dc.SetTextForeground( wxColour(255,0,0) );
            
            // substract from width to leave some space on the right (coordinate is from the left of the text string so we need extra space on the right)
            // if fret number is greater than 9, the string will have two characters so we need to recenter it a bit more
            //const int drawX = getNotePrintX(trackID, line, i).from + (fret > 9 ? renderInfo.pixel_width_of_an_unit/4 : renderInfo.pixel_width_of_an_unit/2);
            const int drawX = getNoteSymbolX(trackID, currentLine, i).to - (fret > 9 ? textSize3.x*2 : textSize3.x);
            const int drawY = trackCoords->y0 + stringHeight*string - textSize3.y/2;
            wxString label = to_wxString(fret);
            
            dc.DrawText( label, drawX, drawY );
            
            if (fret < 0)  dc.SetTextForeground( wxColour(0,0,0) );
                        
            //DEBUG
            //Range<int> symbolArea = getNoteSymbolX(trackID, currentLine, i);
            //dc.SetPen( wxPen(wxColour(255,0,0), 15) );
            //dc.DrawLine( symbolArea.from, drawY, symbolArea.to, drawY );
        }
        
        dc.SetFont( getPrintFont() );
    }//next element
    
    // ---- Silences
    const int fromTick = md->firstTickInMeasure(currentLine.getFirstMeasure());
    const int toTick   = md->lastTickInMeasure(currentLine.getLastMeasure());

    const int silencesY  = trackCoords->y0 + stringHeight;   // second string
    const int silencesY2 = (m_string_amount > 5 ? trackCoords->y0 + stringHeight*2 : silencesY);

    const int silenceAmount = m_silences.size();
    for (int n=0; n<silenceAmount; n++)
    {
        const int tick = m_silences[n].m_tick_range.from;
        
        //std::cout << "TAB editor : silence @ " << tick << " (" << tick/960 << ")" << std::endl;
        
        if (tick >= fromTick and tick < toTick)
        {
            Range<int> silenceX = tickToX(trackID, currentLine, tick);
            if (silenceX.from == -1 or silenceX.to == -1) continue;
            
#if wxCHECK_VERSION(2,9,1) && wxUSE_GRAPHICS_CONTEXT
            RenderRoutines::drawSilence(*gc, silenceX, (m_silences[n].m_type <= 2 ? silencesY : silencesY2),
                                        stringHeight, m_silences[n].m_type, m_silences[n].m_triplet,
                                        m_silences[n].m_dotted);
#else
            RenderRoutines::drawSilence(&dc, silenceX, (m_silences[n].m_type <= 2 ? silencesY : silencesY2),
                                        stringHeight, m_silences[n].m_type, m_silences[n].m_triplet,
                                        m_silences[n].m_dotted);
#endif
            
            //DEBUG
            //Range<int> symbolArea = tickToX(trackID, currentLine, tick);
            //dc.SetPen( wxPen(wxColour(0,255,0), 15) );
            //dc.DrawLine( symbolArea.from, silencesY, symbolArea.to, silencesY );
        }
    }
    
    // ---- Debug guides
    if (PRINT_LAYOUT_HINTS)
    {
        dc.SetPen( wxPen(*wxBLUE, 7) );
        dc.DrawLine(trackCoords->x0, trackCoords->y0, trackCoords->x1, trackCoords->y0);
        dc.DrawLine(trackCoords->x0, trackCoords->y1, trackCoords->x1, trackCoords->y1);
        dc.DrawLine(trackCoords->x0, trackCoords->y0, trackCoords->x0, trackCoords->y1);
        dc.DrawLine(trackCoords->x1, trackCoords->y0, trackCoords->x1, trackCoords->y1);
    }
}

