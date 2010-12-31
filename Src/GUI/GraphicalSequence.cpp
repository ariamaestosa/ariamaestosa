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

#include "GUI/GraphicalSequence.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

GraphicalSequence::GraphicalSequence(Sequence* s)
{
    m_sequence              = s;
    reordering_newPosition  = -1;
    x_scroll_upon_copying   = -1;
    m_measure_bar           = new MeasureBar(s->getMeasureData(), this);
    m_zoom                  = (128.0/(s->ticksPerBeat()*4));
    m_zoom_percent          = 100;
    m_dock_height           = 0;
    m_maximize_track_mode   = false;
    m_x_scroll_in_pixels    = 0;
    y_scroll                = 0;
    reorderYScroll          = 0;
    
    // create editors for any existing track (FIXME(DESIGN): flaky design)
    const int count = s->getTrackAmount();
    for (int n=0; n<count; n++)
    {
        Track* t = s->getTrack(n);
        t->graphics = new GraphicalTrack(t, this);
        t->graphics->createEditors();
    }
}

// ----------------------------------------------------------------------------------------------------------

Track* GraphicalSequence::addTrack()
{
    Track* result = m_sequence->addTrack();
    result->graphics = new GraphicalTrack(result, this);
    result->graphics->createEditors();
    
    return result;
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::prepareEmptyTracksForLoading(int amount)
{
    m_sequence->clear();
    for (int n=0; n<amount; n++)
    {
        addTrack();
    }
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::copy()
{
    m_sequence->copy();
    x_scroll_upon_copying = m_x_scroll_in_pixels;
}

// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------ Scrolling -----------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Scrolling
#endif


int GraphicalSequence::getXScrollInMidiTicks() const
{
    return (int)round(m_x_scroll_in_pixels / m_zoom);
}


// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::setXScrollInMidiTicks(int value)
{
    m_x_scroll_in_pixels = value * m_zoom;
    if (m_x_scroll_in_pixels < 0.0f) m_x_scroll_in_pixels = 0.0f;
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::setXScrollInPixels(int value)
{
    m_x_scroll_in_pixels = value;
    
    const int editor_size = Display::getWidth()-100;
    const int total_size  = m_measure_bar->getTotalPixelAmount();
    
    if (m_x_scroll_in_pixels < 0) m_x_scroll_in_pixels = 0;
    if (m_x_scroll_in_pixels >= total_size-editor_size) m_x_scroll_in_pixels = total_size-editor_size - 1;
    
    Display::render();
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::setYScroll(int value)
{
    y_scroll = value;
}

// ----------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#endif

void GraphicalSequence::setZoom(int zoom)
{
    m_zoom = (zoom/100.0) * 128.0 / ((float)m_sequence->ticksPerBeat() * 4);
    m_zoom_percent = zoom;
}

// ----------------------------------------------------------------------------------------------------------

int GraphicalSequence::getTotalHeight() const
{
    
    int totalHeight=0;
    
    const int count = m_sequence->getTrackAmount();
    for (int n=0; n<count; n++)
    {
        totalHeight += m_sequence->getTrack(n)->graphics->getTotalHeight() + 10;
    }
    
    if (m_sequence->getMeasureData()->isExpandedMode()) totalHeight += 20;
    
    return totalHeight;
}

// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------ Render --------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Render
#endif

void GraphicalSequence::renderTracks(int currentTick, RelativeXCoord mousex, int mousey, int mousey_initial, int from_y)
{
    const int draggedTrack = getMainFrame()->getMainPane()->getDraggedTrackID();
    
    // draw tracks normally
    if (draggedTrack == -1)
    {
        reorderYScroll=0;
        
        int y = from_y - y_scroll;
        const int trackAmount  = m_sequence->getTrackAmount();
        const int currentTrack = m_sequence->getCurrentTrackID();
        
        for (int n=0; n<trackAmount; n++)
        {
            Track* track = m_sequence->getTrack(n);
            track->setId(n);
            y = track->graphics->render(y, currentTick, (n == currentTrack));
        }
        
    }
    // reordering
    else
    {
        reordering_newPosition = draggedTrack;
        
        // draw tracks before current
        int first_y = mousey_initial - reorderYScroll;
        
        for (int trackid=draggedTrack-1; trackid>=0; trackid--)
        {
            Track* track = m_sequence->getTrack(trackid);
            if (track->graphics->isDocked()) continue;
            
            first_y =  mousey_initial - (draggedTrack - trackid)*50 - reorderYScroll;
            track->graphics->renderHeader(0, first_y, true);
            if (mousey < mousey_initial - (draggedTrack-trackid)*50+25 - reorderYScroll) reordering_newPosition=trackid;
        }
        
        // draw tracks after current
        int last_y = mousey_initial - reorderYScroll;
        
        const int trackCount = m_sequence->getTrackAmount();
        for (int trackid=draggedTrack+1; trackid<trackCount; trackid++)
        {
            Track* track = m_sequence->getTrack(trackid);
            if (track->graphics->isDocked()) continue;
            
            last_y = mousey_initial + (trackid - draggedTrack)*50 - reorderYScroll;
            track->graphics->renderHeader(0, last_y, true);
            if (mousey > mousey_initial + (trackid - draggedTrack)*50 + 25 - reorderYScroll)
            {
                reordering_newPosition = trackid + 1;
            }
        }
        
        // scroll up or down if mouse goes to the edges
        if (mousey < 100)
        {
            if (mousey > first_y)
                reorderYScroll -= (100-mousey)*4/100;
        }
        
        if (mousey > Display::getHeight()-100)
        {
            if (mousey < last_y+50)
                reorderYScroll += (mousey - Display::getHeight()+100)*4/100;
        }
        
        // draw track the user is dragging
        m_sequence->getTrack(draggedTrack)->graphics->renderHeader(0, mousey, true, true);
        
        const int arrow_y = (reordering_newPosition > draggedTrack) ?
        mousey_initial + (reordering_newPosition - draggedTrack)*50 - 5 - reorderYScroll :
        mousey_initial - (draggedTrack - reordering_newPosition)*50 - 5 - reorderYScroll;
        
        AriaRender::primitives();
        AriaRender::color(1,0,0);
        AriaRender::line(26, arrow_y, 10, arrow_y);
        AriaRender::triangle( 35, arrow_y,
                             25, arrow_y - 5,
                             25, arrow_y + 5);
    }//end if
    
}


// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------ Mouse Rvents --------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Mouse Events
#endif

bool GraphicalSequence::areMouseHeldDownEventsNeeded()
{
    return true;
    
    // FIXME - clarify status of this. fix or remove.
    
    /*
     const int draggedTrack = Display::getDraggedTrackID();
     
     // we're reordering tracks, it is necessary. return true.
     if (draggedTrack!=-1) return true;
     
     // ask all editors if they need such events at this point.
     const int trackAmount = tracks.size();
     for(int n=0; n<trackAmount; n++)
     {
     // there's one editor that wants them, so return true.
     if (tracks[n].graphics->getCurrentEditor()->areMouseHeldDownEventsNeeded()) return true;
     }//next
     
     // we're not reordering and no editor requested such events, so return false
     return false;
     */
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::mouseHeldDown(RelativeXCoord mousex_current, int mousey_current,
                                      RelativeXCoord mousex_initial, int mousey_initial)
{
    const int draggedTrack = getMainFrame()->getMainPane()->getDraggedTrackID();
    
    // if reordering tracks
    if (draggedTrack != -1)
    {
        Display::render();
        return;
    }
    
    // dispatch event to all tracks
    const int trackAmount = m_sequence->getTrackAmount();
    for (int n=0; n<trackAmount; n++)
    {
        m_sequence->getTrack(n)->graphics->getCurrentEditor()->mouseHeldDown(mousex_current, mousey_current,
                                                                             mousex_initial, mousey_initial);
    }//next
    
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::reorderTracks()
{
    const int draggedTrack = getMainFrame()->getMainPane()->getDraggedTrackID();
    
    if (reordering_newPosition == draggedTrack) return;
    if (reordering_newPosition == -1)           return;
    
    Track* dragged_track = m_sequence->getTrack(draggedTrack);
    
    // if we remove an element now, the IDs in the vector will be modified and reordering_newPosition may not be valid anymore.
    // so we just 'mark' the element we don't want anymore, and it will be removed just after adding the new item.
    m_sequence->tracks.markToBeRemoved(draggedTrack);
    m_sequence->tracks.add(dragged_track, reordering_newPosition);
    m_sequence->tracks.removeMarked();
    
    m_sequence->setCurrentTrackID(reordering_newPosition - 1);
    
    const int currentTrack = m_sequence->getCurrentTrackID();
    if (not (currentTrack >= 0))                                m_sequence->setCurrentTrackID(0);
    else if (not (currentTrack < m_sequence->getTrackAmount())) m_sequence->setCurrentTrackID(0);
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::addToDock(GraphicalTrack* track)
{
    m_dock.push_back(track);
    
    m_sequence->setCurrentTrackID(0);
    
    DisplayFrame::updateVerticalScrollbar();
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::removeFromDock(GraphicalTrack* track)
{
    
    for (int n=0; n<m_dock.size(); n++)
    {
        if (&m_dock[n] == track)
        {
            m_dock.remove( n );
            return;
        }
    }
    DisplayFrame::updateVerticalScrollbar();
    
}

// ----------------------------------------------------------------------------------------------------------
// --------------------------------------------- Selection --------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Selection
#endif

void GraphicalSequence::selectAll()
{
    m_sequence->getCurrentTrack()->graphics->selectNote(ALL_NOTES, true, true);
    if (m_sequence->m_seq_data_listener != NULL) m_sequence->m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::selectNone()
{
    m_sequence->getCurrentTrack()->graphics->selectNote(ALL_NOTES, false, true);
    if (m_sequence->m_seq_data_listener != NULL) m_sequence->m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------
// ------------------------------------------------ I/O -----------------------------------------------------
// ----------------------------------------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark I/O
#endif

void GraphicalSequence::saveToFile(wxFileOutputStream& fileout)
{
    
    writeData(wxT("<seqview xscroll=\"") + to_wxString(m_x_scroll_in_pixels) +
              wxT("\" yscroll=\"")       + to_wxString(y_scroll) +
              wxT("\" zoom=\"")          + to_wxString(m_zoom_percent) +
              wxT("\">\n"), fileout);
    
    m_sequence->saveToFile(fileout);
    
    writeData(wxT("</seqview>\n"), fileout);
}

// ----------------------------------------------------------------------------------------------------------

bool GraphicalSequence::readFromFile(irr::io::IrrXMLReader* xml)
{
    bool inSeqView = false;
    
    // parse the file until end reached
    while (xml != NULL and xml->read())
    {
        
        switch (xml->getNodeType())
        {
            case irr::io::EXN_TEXT:
            {
                break;
            }
            case irr::io::EXN_ELEMENT:
            {
                if (strcmp("sequence", xml->getNodeName()) == 0)
                {
                    if (inSeqView)
                    {
                        m_sequence->readFromFile(xml, this);
                    }
                    else
                    {
                        std::cerr << "WARNING: Found a misplaced <sequence> tag\n";
                        m_sequence->readFromFile(xml, this); // for backwards compatibility, try to continue anyway
                    }
                }
                // ---------- view ------
                else if (strcmp("seqview", xml->getNodeName()) == 0)
                {
                    inSeqView = true;
                    
                    const char* xscroll_c = xml->getAttributeValue("xscroll");
                    if (xscroll_c != NULL)
                    {
                        m_x_scroll_in_pixels = atoi( xscroll_c );
                    }
                    else
                    {
                        m_x_scroll_in_pixels = 0;
                        std::cerr << "Missing info from file: x scroll" << std::endl;
                    }
                    
                    if (m_x_scroll_in_pixels < 0)
                    {
                        std::cerr << "Wrong x_scroll_in_pixels: " << m_x_scroll_in_pixels << std::endl;
                        m_x_scroll_in_pixels = 0;
                    }
                    
                    const char* yscroll_c = xml->getAttributeValue("yscroll");
                    if ( yscroll_c != NULL )
                    {
                        y_scroll = atoi( yscroll_c );
                    }
                    else
                    {
                        y_scroll = 0;
                        std::cerr << "Missing info from file: y scroll" << std::endl;
                    }
                    
                    if (y_scroll < 0)
                    {
                        std::cerr << "Wrong y_scroll: " << y_scroll << std::endl;
                        y_scroll = 0;
                    }
                    
                    const char* zoom_c = xml->getAttributeValue("zoom");
                    if (zoom_c != NULL)
                    {
                        int zoom_i = atoi(zoom_c);
                        if (zoom_i > 0 and zoom_i<501) setZoom( zoom_i );
                        else return false;
                    }
                    else
                    {
                        setZoom( 100 );
                        std::cerr << "Missing info from file: zoom" << std::endl;
                    }
                    
                    if (m_zoom <= 0)
                    {
                        std::cerr << "Fatal Error: Wrong Zoom: " << m_zoom 
                        << "(char* = " << zoom_c << ") " << std::endl;
                        setZoom( 100 );
                    }
                    
                }
                break;
            }
                
            case irr::io::EXN_ELEMENT_END:
            {
                if (strcmp("seqview", xml->getNodeName()) == 0)
                {
                    inSeqView = false;
                }
                break;
            }
                
            default:
            {
                break;
            }
        } // end switch
    } // end while
    
    DisplayFrame::updateHorizontalScrollbar( m_x_scroll_in_pixels );
    
    return true;
}

// ----------------------------------------------------------------------------------------------------------

