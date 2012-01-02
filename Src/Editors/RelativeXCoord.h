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


#ifndef __RELATIVE_X_COORD_H__
#define __RELATIVE_X_COORD_H__

namespace AriaMaestosa
{
    class GraphicalSequence;
    
    /** @brief goes along class RelativeXCood */
    enum RelativeType
    {
        WINDOW,
        EDITOR,
        MIDI
    };
    
    /**
      * @brief class to ease midi coord manipulation.
      *
      * Each location can be expressed in 3 ways: pixel within the window, pixel within the editor, midi tick
      * This class allows to seamlessly work with all these datas.
      * @ingroup editors
      */
    class RelativeXCoord
    {
        int relativeToEditor;
        int relativeToWindow, relativeToMidi;
        
        GraphicalSequence* m_seq;
        
    public:
        
        RelativeXCoord(GraphicalSequence* seq);
        RelativeXCoord(int i, RelativeType relativeTo, GraphicalSequence* seq);
        
        void setValue(int i, RelativeType relativeTo);
        
        void setSequence(GraphicalSequence* seq) { m_seq = seq; }
        
        /**
         * @brief Convert the way this data is stored.
         * For instance, if you enter data as pixels, but want to keep the same tick even
         * though scrolling occurs, you could convert it to midi ticks.
         */
        void convertTo(RelativeType relativeTo);
        
        int getRelativeTo(RelativeType returnRelativeTo);
        
        bool isValid() const { return relativeToEditor != -1 or
                                      relativeToWindow != -1 or
                                      relativeToMidi   != -1; }
        
    };
    
    RelativeXCoord& RelativeXCoord_empty();
    
    
}

#endif
