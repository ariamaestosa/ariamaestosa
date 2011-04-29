
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

#ifdef _MAC_QUICKTIME_COREAUDIO

#ifndef _qtkitplayer_
#define _qtkitplayer_

/**
  * @ingroup midi.players
  *
  * On OSX, used to export MIDI to AIFF.
  */
class QuickTimeExport
{
public:
    
    /** Load MIDI data into QuickTime
      * @return Whether loading the data was successful
      */
    static bool qtkit_setData(char* data, int length);
    
    /**
      * Export the data loaded with qtkit_setData to AIFF
      * @return Whether exporting was successful
      */
    static bool qtkit_exportToAiff(const char* filename);
};

#endif

#endif