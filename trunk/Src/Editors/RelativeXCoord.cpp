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

#include "Editors/Editor.h"
#include "Editors/RelativeXCoord.h"
#include "GUI/GraphicalSequence.h"
#include "AriaCore.h"

namespace AriaMaestosa
{
    RelativeXCoord* nullOne = NULL;
    
    RelativeXCoord& RelativeXCoord_empty()
    {
        if (nullOne == NULL) nullOne = new RelativeXCoord(-1, WINDOW, NULL);
        return *nullOne;
    }
}

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

RelativeXCoord::RelativeXCoord(GraphicalSequence* seq)
{
    m_seq = seq;
    relativeToEditor = -1;
    relativeToWindow = -1;
    relativeToMidi   = -1;
}

// ----------------------------------------------------------------------------------------------------------

RelativeXCoord::RelativeXCoord(int i, RelativeType relativeTo, GraphicalSequence* seq)
{
    m_seq = seq;
    setValue(i, relativeTo);
}

// ----------------------------------------------------------------------------------------------------------

void RelativeXCoord::setValue(int i, RelativeType relativeTo)
{
    relativeToEditor = -1;
    relativeToWindow = -1;
    relativeToMidi   = -1;

    if (relativeTo == WINDOW)
    {
        relativeToWindow = i;
    }
    else if (relativeTo == MIDI)
    {
        relativeToMidi = i;
    }
    else if (relativeTo == EDITOR)
    {
        // use relative to frame or midi ticks when setting the value of a coord
        std::cout << "COORD SET RELATIVE TO EDITOR!!! That will probably fail as it is deprecated" << std::endl;
        relativeToEditor = i;
    }
    else
    {
        std::cout << "!! RelativeXCoord ERROR - needs one of 3 (A)" << std::endl;
        ASSERT(0);
    }

}

// ----------------------------------------------------------------------------------------------------------

void RelativeXCoord::convertTo(RelativeType relativeTo)
{
    switch (relativeTo)
    {

        case EDITOR:

            if (relativeToWindow != -1)
            {
                relativeToEditor = relativeToWindow - Editor::getEditorXStart();
            }
            else if (relativeToMidi != -1)
            {
                relativeToEditor = ( int )( relativeToMidi * m_seq->getZoom() ) - m_seq->getXScrollInPixels();
            }
            else
            {
                std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (B)" << std::endl;
                ASSERT(0);
            }
            relativeToWindow = -1;
            relativeToMidi = -1;
            break;

        case WINDOW:

            if (relativeToWindow==-1)
            {
                    if (relativeToMidi != -1)
                    {
                        relativeToWindow = ( int )( relativeToMidi * m_seq->getZoom() ) - m_seq->getXScrollInPixels() + Editor::getEditorXStart();
                    }
                    else
                    {
                        std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (C)" << std::endl;
                        ASSERT(0);
                    }
            }
            relativeToMidi = -1;
            relativeToEditor = -1;

            break;

        case MIDI:

            if (relativeToMidi == -1)
            {

                    if (relativeToWindow != -1)
                    {
                        relativeToMidi = (int)((relativeToWindow - Editor::getEditorXStart()) / m_seq->getZoom()) +
                                         m_seq->getXScrollInMidiTicks();
                    }
                    else
                    {
                        std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (D)" << std::endl;
                        ASSERT(0);
                    }

            }
            relativeToWindow = -1;
            relativeToEditor = -1;

            break;
    }

}

// ----------------------------------------------------------------------------------------------------------

int RelativeXCoord::getRelativeTo(RelativeType returnRelativeTo)
{
    ASSERT(m_seq != NULL);
    
    switch (returnRelativeTo)
    {
        case EDITOR:

            if (relativeToWindow != -1)
            {
                relativeToEditor = relativeToWindow - Editor::getEditorXStart();
            }
            else if (relativeToMidi != -1)
            {
                relativeToEditor = ( int )( relativeToMidi * m_seq->getZoom() ) - m_seq->getXScrollInPixels();
            }
            else
            {
                std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (E)" << std::endl;
                return -1;
                //ASSERT(0);
            }

            return relativeToEditor;
            break;

        case WINDOW:

            if (relativeToWindow!=-1)
            {
                return relativeToWindow;
            }
            else
            {
                    if (relativeToMidi != -1)
                    {
                        return int(relativeToMidi * m_seq->getZoom()) -
                                   m_seq->getXScrollInPixels() + Editor::getEditorXStart();
                    }
                    else
                    {
                        std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (F)" << std::endl;
                        return -1;
                        //ASSERT(0);
                    }
            }
            break;

        case MIDI:


            if (relativeToMidi != -1)
            {
                return relativeToMidi;
            }
            else
            {

                    if (relativeToWindow != -1)
                    {
                        return (int)((relativeToWindow - Editor::getEditorXStart()) / m_seq->getZoom()) +
                                    m_seq->getXScrollInMidiTicks();
                    }
                    else
                    {
                        std::cerr << "!! RelativeXCoord ERROR - needs one of 3 (G)" << std::endl;
                        return -1;
                        //ASSERT(0);
                    }
            }

            break;
    }
    //ASSERT(0);
    std::cerr << "!! RelativeXCoord ERROR - Conversion failed!" << std::endl;
    return -1;

}

// ----------------------------------------------------------------------------------------------------------

