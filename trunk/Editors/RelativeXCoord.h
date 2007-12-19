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


/*
 * This class is there to ease midi coord manipulation.
 * Each location can be expressed in 3 ways: pixel within the window, pixel within the editor, midi tick
 * This class allows to seamlessly work with all these datas.
 */

#ifndef _relativecoord_
#define _relativecoord_

namespace AriaMaestosa {
	
	//class Sequence;
	
enum RelativeType
{
    WINDOW,
    EDITOR,
    MIDI
};

class RelativeXCoord
{
	//int value;
    //Sequence* sequence;
	int relativeToEditor;
	int relativeToWindow, relativeToMidi;
public:
	RelativeXCoord();
    RelativeXCoord(int i, RelativeType relativeTo);
    void setValue(int i, RelativeType relativeTo);
	void convertTo(RelativeType relativeTo);
    int getRelativeTo(RelativeType returnRelativeTo);
	
};

RelativeXCoord& RelativeXCoord_empty();

/*
template <typename TYPE>
class RelativeXCoord
{
    TYPE value;
    Sequence* sequence;
    int relativeTo;
    
    int relativeToEditor, relativeToWindow, relativeToMidi;
	
public:
		
	RelativeXCoord()
	{
		sequence = getMainFrame()->getCurrentSequence();
    }
    
    RelativeXCoord(Sequence* seq)
	{
        sequence = seq;
    }
    
    void setValue(TYPE i, int relativeTo)
	{
        value = i;
        RelativeXCoord::relativeTo = relativeTo;
        
        // 1. make relative to editor
        if(relativeTo == WINDOW)
		{
            value -= getEditorXStart();
        }
        else if(relativeTo == MIDI)
		{
            value = ( TYPE )( value * sequence->getZoom() );
        }
        
        
        relativeToEditor = value;
        relativeToWindow = value + getEditorXStart();
        relativeToMidi = (TYPE)( value / sequence->getZoom() );
        
    }
    
    TYPE getRelativeTo(int returnRelativeTo)
	{
        
        switch(returnRelativeTo)
		{
			case EDITOR:   
				return relativeToEditor;  
				break;
			case WINDOW:   
				return relativeToWindow;  
				break;
			case MIDI:   
				return relativeToMidi; 
				break;
        }
        std::cout << "Conversion failed!" << std::endl;
        return -1;
        
    }
};
*/

}

#endif
