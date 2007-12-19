
#ifndef _clipboard_
#define _clipboard_

#include <vector>
#include "Midi/Note.h"

namespace AriaMaestosa
{
	
	namespace Clipboard
{
	void clear();
	void add(Note* n);
	int getSize();
	Note* getNote( int index );
}

}

#endif
