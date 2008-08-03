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

#ifndef _addctrls_
#define _addctrls_

#include "Actions/EditAction.h"

namespace AriaMaestosa
{
	class Track;
namespace Action
{

	class AddControllerSlide : public SingleTrackAction
	{
		friend class AriaMaestosa::Track;
		int x1, value1, x2, value2, controller;

		ControlEventRelocator relocator;
		ptr_vector<ControllerEvent> removedControlEvents;
		
		void addOneEvent(ControllerEvent* ptr, ptr_vector<ControllerEvent>* vector, int id=-1);
        void pushBackOneEvent(ControllerEvent* ptr, ptr_vector<ControllerEvent>* vector);
public:
        AddControllerSlide(const int x1, const int value1, const int x2, const int value2, const int controller);
		void perform();
		void undo();
		
		
		virtual ~AddControllerSlide();
	};
	
}
}
#endif
