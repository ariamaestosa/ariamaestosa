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

using namespace AriaMaestosa;

const float UNITS_PER_TICK = 0.2f;

// -------------------------------------------------------------------------------------------------------

KeyrollPrintableSequence::KeyrollPrintableSequence(Sequence* parent) : AbstractPrintableSequence(parent)
{
    m_page_amount = 0;
}

// -------------------------------------------------------------------------------------------------------

void KeyrollPrintableSequence::printLinesInArea(wxDC& dc, const int page, const float notation_area_y0,
                                                const float notation_area_h, const int pageHeight,
                                                const int x0, const int x1)
{
}

// -------------------------------------------------------------------------------------------------------

void KeyrollPrintableSequence::calculateLayout(bool checkRepetitions)
{
    ASSERT(not checkRepetitions); // not supported
    
    const float unitCount = getMeasureData()->getTotalTickAmount() * UNITS_PER_TICK;
    
    m_page_amount = unitCount / AriaPrintable::getCurrentPrintable()->getUnitHeight(); 
        
    AbstractPrintableSequence::calculateLayout(checkRepetitions);
}

// -------------------------------------------------------------------------------------------------------
