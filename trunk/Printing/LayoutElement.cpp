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

#include "AriaCore.h"
#include "Printing/EditorPrintable.h"
#include "Printing/PrintingBase.h"

namespace AriaMaestosa
{
// used to determine the order of what appears in the file.
// the order is found first before writing anything because that allows more flexibility
LayoutElement::LayoutElement(LayoutElementType type_arg, int measure_arg)
{
    type = type_arg;
    measure = measure_arg;
        
    x = -1;
    x2 = -1;
}



}

