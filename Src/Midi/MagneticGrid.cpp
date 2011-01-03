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

#include "Midi/MagneticGrid.h"
#include "Midi/Sequence.h"
#include "IO/IOUtils.h"

#include "AriaCore.h"
#include "irrXML/irrXML.h"

using namespace AriaMaestosa;

// ----------------------------------------------------------------------------------------------------------

MagneticGrid::MagneticGrid()
{
    m_triplet = false;
    m_divider = 8;
    m_label   = wxT("1/8");
}

// ----------------------------------------------------------------------------------------------------------

MagneticGrid::~MagneticGrid()
{
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGrid::setTriplet(bool val)
{
    m_triplet = val;
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGrid::updateLabel()
{
    // set right label to combo box
    if      (m_divider == 1)   m_label = wxT("1/1");
    else if (m_divider == 2)   m_label = wxT("1/2");
    else if (m_divider == 3)   m_label = wxT("1/2T");
    else if (m_divider == 4)   m_label = wxT("1/4");
    else if (m_divider == 6)   m_label = wxT("1/4T");
    else if (m_divider == 8)   m_label = wxT("1/8");
    else if (m_divider == 12)  m_label = wxT("1/8T");
    else if (m_divider == 16)  m_label = wxT("1/16");
    else if (m_divider == 24)  m_label = wxT("1/16T");
    else if (m_divider == 32)  m_label = wxT("1/32");
    else if (m_divider == 48)  m_label = wxT("1/32T");
    else if (m_divider == 64)  m_label = wxT("1/64");
    else if (m_divider == 96)  m_label = wxT("1/64T");
    else if (m_divider == 128) m_label = wxT("1/128");
    else if (m_divider == 192) m_label = wxT("1/128T");
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGrid::setDivider(const int newVal)
{
    m_divider = newVal;
    updateLabel();
}

// ----------------------------------------------------------------------------------------------------------

void MagneticGrid::saveToFile(wxFileOutputStream& fileout)
{
    
    writeData( wxT("<magneticgrid ") +
              wxString(wxT("divider=\"")) + to_wxString(m_divider) +
              wxT("\" triplet=\"") + wxString(m_triplet ? wxT("true") : wxT("false")) +
              wxT("\"/>\n"), fileout);
    
}

// ----------------------------------------------------------------------------------------------------------

bool MagneticGrid::readFromFile(irr::io::IrrXMLReader* xml)
{
    
    const char* divider_c = xml->getAttributeValue("divider");
    const char* triplet_c = xml->getAttributeValue("triplet");
    
    if (divider_c != NULL)
    {
        m_divider = atoi( divider_c );
    }
    else
    {
        std::cerr << "[MagneticGrid::ReadFromFile] WARNING: Missing info from file: grid divider" << std::endl;
        m_divider = 8;
    }
    
    if (triplet_c != NULL)
    {
        if (strcmp(triplet_c, "true") == 0)
        {
            m_triplet = true;
        }
        else if (strcmp(triplet_c, "false") == 0)
        {
            m_triplet = false;
        }
        else
        {
            std::cout << "Unknown keyword for attribute 'triplet' in magneticgrid: " << triplet_c << std::endl;
            m_triplet = false;
        }
    }
    else
    {
        std::cout << "Missing info from file: triplet" << std::endl;
        m_triplet = false;
    }
    
    updateLabel();

    return true;
}

// ----------------------------------------------------------------------------------------------------------
