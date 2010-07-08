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

#include "Utils.h"
#include "AriaCore.h"
#include "Midi/Sequence.h"
#include "Midi/MeasureData.h"
#include "Printing/AriaPrintable.h"
#include "Printing/KeyrollPrintableSequence.h"

#include "wx/dc.h"

using namespace AriaMaestosa;

//const float UNITS_PER_TICK = 0.2f;

// -------------------------------------------------------------------------------------------------------

KeyrollPrintableSequence::KeyrollPrintableSequence(Sequence* parent, float cmPerBeat, float verticalMargin,
                                                   bool compact, std::vector<wxColor> colors) :
    AbstractPrintableSequence(parent)
{
    m_compact = compact;
    m_vertical_margin = verticalMargin;
    m_colors = colors;

    m_units_per_tick = cmPerBeat * UNITS_PER_CM / getMeasureData()->beatLengthInTicks();
    ASSERT_E(m_units_per_tick, >, 0.0f);
}

// -------------------------------------------------------------------------------------------------------

void KeyrollPrintableSequence::calculateLayout(bool checkRepetitions)
{
    ASSERT(not checkRepetitions); // not supported

    const int tickCount = getMeasureData()->getTotalTickAmount();
    //const float unitCount = tickCount * UNITS_PER_TICK;
    int ticksPerPage = AriaPrintable::getCurrentPrintable()->getUnitWidth() / m_units_per_tick;
    ticksPerPage = (ticksPerPage / getMeasureData()->beatLengthInTicks())* getMeasureData()->beatLengthInTicks();

    int currTick = 0;
    m_pages.clear();

    // m_page_amount = unitCount / AriaPrintable::getCurrentPrintable()->getUnitWidth();

    while (currTick < tickCount)
    {
        Page newPage;
        newPage.m_first_tick = currTick;
        newPage.m_first_print_unit = currTick*m_units_per_tick;

        currTick += ticksPerPage;
        newPage.m_last_tick = currTick - 1;
        newPage.m_last_print_unit = (currTick - 1)*m_units_per_tick;

        m_pages.push_back(newPage);
    }

    m_min_pitch = -1;
    m_max_pitch = -1;

    const int trackAmount = m_tracks.size();
    for (int n=0; n<trackAmount; n++)
    {
        const int noteAmount = m_tracks[n].getNoteAmount();
        for (int note=0; note<noteAmount; note++)
        {
            const int pitch = m_tracks[n].getNotePitchID(note);
            if (pitch < m_min_pitch or m_min_pitch == -1) m_min_pitch = pitch;
            if (pitch > m_max_pitch or m_max_pitch == -1) m_max_pitch = pitch;
        }
    }


    if (m_compact)
    {
        const int size = m_max_pitch - m_min_pitch + 1;
        m_compact_info.resize(size);

        //for (int n = 0; n<size; n++)
        //{
        //    m_compact_info[n] = false;
        //}

        int count = 0;
        for (int t=0; t<trackAmount; t++)
        {
            const KeyInclusionType* keyNotes = m_tracks[t].getKeyNotes();

            for (int p = m_min_pitch; p <= m_max_pitch; p++)
            {
                if (keyNotes[p] == KEY_INCLUSION_NONE)
                {
                    m_compact_info[p - m_min_pitch] = -1;
                }
                else
                {
                    m_compact_info[p - m_min_pitch] = count;
                    count++;
                }
            }

            //const int noteAmount = m_tracks[n].getNoteAmount();
            //for (int note=0; note<noteAmount; note++)
            //{
            //    const int pitch = m_tracks[n].getNotePitchID(note);
            //    m_compact_info[pitch - m_min_pitch] = true;
            //}
        }

        m_compact_pitch_count = 0;
        for (int n=0; n<(int)m_compact_info.size(); n++)
        {
            if (m_compact_info[n] != -1) m_compact_pitch_count++;
        }
    }

    AbstractPrintableSequence::calculateLayout(checkRepetitions);
}

// -------------------------------------------------------------------------------------------------------

void KeyrollPrintableSequence::printLinesInArea(wxDC& dc, const int page, const float notationAreaY0,
                                                const float notationAreaHeight, const int pageHeight,
                                                const int x0, const int x1)
{
    dc.SetPen( wxPen(*wxBLACK, 5) );

    const int LEFT_SIDE_WIDTH = 200;
    const int usableX0 = x0 + LEFT_SIDE_WIDTH;

    // ---- draw background
    dc.SetBrush( wxBrush(wxColor(200,200,200)) );
    dc.DrawRectangle( x0, notationAreaY0, LEFT_SIDE_WIDTH, notationAreaHeight);

    // a couple "closures" to help us
#define PITCH_TO_Y( pitch ) (m_compact ? notationAreaY0 + notationAreaHeight*(m_compact_info[pitch])/(m_compact_pitch_count) \
: notationAreaY0 + notationAreaHeight*(pitch)/(pitchRange + 1))
#define NOTE_H (m_compact ? notationAreaHeight/(m_compact_pitch_count) : notationAreaHeight/(pitchRange + 1))
#define TICK_TO_X( tick ) (usableX0 + (((tick) - m_pages[page].m_first_tick)*(x1 - usableX0)/  \
(m_pages[page].m_last_tick - m_pages[page].m_first_tick)))

    const int pitchRange = m_max_pitch - m_min_pitch + 1;

    // draw horizontal lines
    if (m_compact)
    {
        const int len = m_compact_info.size();
        //int count = 0;
        for (int n=0; n<len; n++)
        {
            if (m_compact_info[n] == -1) continue;

            const int y = PITCH_TO_Y(n);
            dc.DrawLine(usableX0, y, x1, y);

            Note12 noteName;
            int octave;
            const bool success = Editor::findNoteName(m_min_pitch+n, &noteName, &octave);
            if (success)
            {
                dc.DrawText( NOTE_12_NAME[noteName] + wxString::Format(wxT("%i"), octave), x0 + 15, y );
            }
            //count++;
        }
        dc.DrawLine(usableX0, notationAreaY0 + notationAreaHeight, x1, notationAreaY0 + notationAreaHeight);
    }
    else
    {
        for (int note = m_min_pitch; note <= m_max_pitch+1; note++)
        {
            const int y = PITCH_TO_Y(note - m_min_pitch);
            dc.DrawLine(usableX0, y, x1, y);

            Note12 noteName;
            int octave;
            const bool success = Editor::findNoteName(note, &noteName, &octave);
            if (success)
            {
                dc.DrawText( NOTE_12_NAME[noteName] + wxString::Format(wxT("%i"), octave), x0 + 15, y );
            }
        }
        dc.DrawLine(usableX0, notationAreaY0 + notationAreaHeight, x1, notationAreaY0 + notationAreaHeight);
    }

    const int beatLen = getMeasureData()->beatLengthInTicks();

    // page must start on a beat
    ASSERT_E( (m_pages[page].m_first_tick/beatLen)*beatLen, ==, m_pages[page].m_first_tick );

    ASSERT_E(m_pages[page].m_last_tick, >, m_pages[page].m_first_tick);

    // draw vertical lines
    for (int beatTick=m_pages[page].m_first_tick;
         beatTick <= m_pages[page].m_last_tick+1;
         beatTick += beatLen)
    {
        ASSERT_E(beatTick, >=, 0);
        const int x = TICK_TO_X(beatTick);
        // TODO: measure lines must be stronger than beat lines (which can be very pale)
        dc.DrawLine(x, notationAreaY0, x, notationAreaY0 + notationAreaHeight);
    }

    // ---- draw notes
    const float unitsPerCm = AriaPrintable::getCurrentPrintable()->getUnitsPerCm();
    const int noteHeightInUnits = NOTE_H;
    const int verticalMarginInTicks = (m_vertical_margin/10.f)*unitsPerCm;

    const int trackAmount = m_tracks.size();
    for (int n=0; n<trackAmount; n++)
    {
        ASSERT_E(n, <, (int)m_colors.size());
        dc.SetBrush( wxBrush(m_colors[n]) );

        const int noteAmount = m_tracks[n].getNoteAmount();
        for (int note=0; note<noteAmount; note++)
        {
            const int pitch = m_tracks[n].getNotePitchID(note);

            // skip notes
            if (m_compact and m_compact_info[pitch] == -1) continue;

            const int tick = m_tracks[n].getNoteStartInMidiTicks(note);
            const int tickTo = m_tracks[n].getNoteEndInMidiTicks(note);

            if (tick > m_pages[page].m_last_tick) break;

            if ((tick >= m_pages[page].m_first_tick and tick <= m_pages[page].m_last_tick) or
                (tickTo > m_pages[page].m_first_tick and tickTo <= m_pages[page].m_last_tick))
            {
                int x = TICK_TO_X(tick);
                int x2 = TICK_TO_X(tickTo);
                if (x < usableX0) x = usableX0;
                else if (x > x1) x = x1;
                if (x2 < usableX0) x2 = usableX0;
                else if (x2 > x1) x2 = x1;

                const int y = PITCH_TO_Y(pitch - m_min_pitch) + verticalMarginInTicks/2;

                dc.DrawRectangle(x, y, x2 - x, noteHeightInUnits - verticalMarginInTicks);
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------
