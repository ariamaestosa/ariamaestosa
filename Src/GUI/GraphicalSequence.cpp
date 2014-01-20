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

#include "Actions/Paste.h"
#include "GUI/GraphicalSequence.h"
#include "GUI/MainFrame.h"
#include "GUI/MainPane.h"
#include "Midi/MeasureData.h"
#include "Midi/Sequence.h"
#include "PreferencesData.h"

#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

GraphicalSequence::GraphicalSequence(Sequence* s) : m_name_renderer(s->getNameModel(), false)
{
    m_sequence              = s;
    reordering_newPosition  = -1;
    x_scroll_upon_copying   = -1;
    m_measure_bar           = new MeasureBar(s->getMeasureData(), this);
    m_zoom                  = (128.0/(s->ticksPerQuarterNote()*4));
    m_zoom_percent          = 100;
    m_dock_height           = 0;
    m_maximize_track_mode   = false;
    m_x_scroll_in_pixels    = 0;
    y_scroll                = 0;
    reorderYScroll          = 0;
    
    m_name_renderer.setMaxWidth(155); // FIXME - won't work if lots of sequences are open (tabs will begin to get smaller)
    m_name_renderer.setFont( getSequenceFilenameFont() );
    
    s->addTrackSetListener(this);
}

void GraphicalSequence::createViewForTrack(Track* t)
{
    ASSERT(t->getSequence() == m_sequence);
    
    ASSERT( getGraphicsFor(t) == NULL );
    GraphicalTrack* gt = new GraphicalTrack(t, this, t->getMagneticGrid());
    m_gtracks.push_back(gt);
    gt->createEditors();
}

void GraphicalSequence::createViewForTracks(int id)
{    
    if (id == -1)
    {
        const int count = m_sequence->getTrackAmount();
        for (int n=0; n<count; n++)
        {
            Track* t = m_sequence->getTrack(n);
            createViewForTrack(t);
        }
    }
    else
    {
        Track* t = m_sequence->getTrack(id);
        ASSERT( getGraphicsFor(t) == NULL );
        createViewForTrack(t);
    }
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::onTrackAdded(Track* t)
{
    createViewForTrack(t);
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::onTrackRemoved(Track* t)
{    
    GraphicalTrack* gt = getGraphicsFor(t);
    
    const int count = m_gtracks.size();
    for (int n=0; n<count; n++)
    {
        m_gtracks[n].onTrackRemoved(t);
    }
    
    gt->onTrackRemoved(t);
    
    ASSERT(gt != NULL);
    
    const bool success = m_gtracks.erase(gt);
    ASSERT(success);
}

// ----------------------------------------------------------------------------------------------------------

GraphicalTrack* GraphicalSequence::getGraphicsFor(const Track* t)
{
    const int count = m_gtracks.size();
    for (int n=0; n<count; n++)
    {
        if (m_gtracks[n].getTrack() == t) return m_gtracks.get(n);
    }
    
    return NULL;
}

// ----------------------------------------------------------------------------------------------------------

const GraphicalTrack* GraphicalSequence::getGraphicsFor(const Track* t) const
{
    const int count = m_gtracks.size();
    for (int n=0; n<count; n++)
    {
        if (m_gtracks[n].getTrack() == t) return m_gtracks.getConst(n);
    }
    
    return NULL;
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::copy()
{
    m_sequence->copy();
    x_scroll_upon_copying = m_x_scroll_in_pixels;
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::paste()
{
    GraphicalTrack* gtrack = getCurrentTrack();
    gtrack->getTrack()->action( new Action::Paste(gtrack->getFocusedEditor(), false) );
    if (m_sequence->m_seq_data_listener != NULL) m_sequence->m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::pasteAtMouse()
{
    GraphicalTrack* gtrack = getCurrentTrack();
    gtrack->getTrack()->action( new Action::Paste(gtrack->getFocusedEditor(), true) );
    if (m_sequence->m_seq_data_listener != NULL) m_sequence->m_seq_data_listener->onSequenceDataChanged();
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
    ASSERT_E(value, >=, 0);
    
    m_x_scroll_in_pixels = value * m_zoom;
    
    ASSERT_E(m_x_scroll_in_pixels, >=, 0);
    
    if (m_x_scroll_in_pixels < 0.0f) m_x_scroll_in_pixels = 0.0f;
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::setXScrollInPixels(int value)
{
    m_x_scroll_in_pixels = value;
    ASSERT_E(m_x_scroll_in_pixels, >=, 0);

    const int editor_size = Display::getWidth() - 100;
    const int total_size  = m_measure_bar->getTotalPixelAmount();
        
    if (m_x_scroll_in_pixels < 0) m_x_scroll_in_pixels = 0;
    if (m_x_scroll_in_pixels >= total_size - editor_size)
    {
        if (total_size > editor_size)
        {
            m_x_scroll_in_pixels = total_size - editor_size - 1;
        }
        else
        {
            // special case where song is short enough to be fully visible
            m_x_scroll_in_pixels = 0;
        }
    }
    ASSERT_E(m_x_scroll_in_pixels, >=, 0);

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
    m_zoom = (float)zoom * 1.28 / ((float)m_sequence->ticksPerQuarterNote() * 4.0);
    m_zoom_percent = zoom;
}

// ----------------------------------------------------------------------------------------------------------

int GraphicalSequence::getTotalHeight() const
{
    
    int totalHeight=0;
    
    const int count = m_gtracks.size();
    for (int n=0; n<count; n++)
    {
        totalHeight += m_gtracks[n].getTotalHeight() + 10;
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
        
        ASSERT(m_gtracks.size() == trackAmount);
        
        for (int n=0; n<trackAmount; n++)
        {
            Track* track = m_sequence->getTrack(n);
            track->setId(n);
            y = getGraphicsFor(track)->render(y, currentTick, (n == currentTrack));
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
            if (getGraphicsFor(track)->isDocked()) continue;
            
            first_y =  mousey_initial - (draggedTrack - trackid)*50 - reorderYScroll;
            getGraphicsFor(track)->renderHeader(0, first_y, true);
            if (mousey < mousey_initial - (draggedTrack-trackid)*50+25 - reorderYScroll) reordering_newPosition=trackid;
        }
        
        // draw tracks after current
        int last_y = mousey_initial - reorderYScroll;
        
        const int trackCount = m_sequence->getTrackAmount();
        for (int trackid=draggedTrack+1; trackid<trackCount; trackid++)
        {
            Track* track = m_sequence->getTrack(trackid);
            GraphicalTrack* gtrack = getGraphicsFor(track);
            
            if (gtrack->isDocked()) continue;
            
            last_y = mousey_initial + (trackid - draggedTrack)*50 - reorderYScroll;
            gtrack->renderHeader(0, last_y, true);
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
        getGraphicsFor(m_sequence->getTrack(draggedTrack))->renderHeader(0, mousey, true, true);
        
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
    const int trackAmount = m_gtracks.size();
    for (int n=0; n<trackAmount; n++)
    {
        m_gtracks[n].getFocusedEditor()->mouseHeldDown(mousex_current, mousey_current,
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
    if (not m_dock.contains(track))
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
    getGraphicsFor(m_sequence->getCurrentTrack())->selectNote(ALL_NOTES, true, true);
    if (m_sequence->m_seq_data_listener != NULL) m_sequence->m_seq_data_listener->onSequenceDataChanged();
}

// ----------------------------------------------------------------------------------------------------------

void GraphicalSequence::selectNone()
{
    getGraphicsFor(m_sequence->getCurrentTrack())->selectNote(ALL_NOTES, false, true);
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
	
	bool foundSequenceNode = false;
    
    if (xml == NULL)
    {
        std::cerr << "XML reader is NULL\n";
        return false;
    }
    
    int zoom = -1;
    
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
					foundSequenceNode = true;
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
                        ASSERT_E(m_x_scroll_in_pixels, >=, 0);
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
                        if (zoom_i > 0 and zoom_i < 501) zoom = zoom_i;
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
    
    // It's important to set zoom last because beat resolution has not yet been set at the time we read the zoom value
    if (zoom != -1)
    {
        setZoom( zoom );
    }
    else
    {
        setZoom(100);
    }

    DisplayFrame::updateHorizontalScrollbar( m_x_scroll_in_pixels );
    
    if (not foundSequenceNode)
    {
        std::cerr << "ERROR: File contains no sequence node\n";
        return false;
    }
	
    return true;
}

// ----------------------------------------------------------------------------------------------------------

