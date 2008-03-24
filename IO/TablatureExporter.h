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


#ifndef _tablature_out_
#define _tablature_out_

#include <wx/file.h>
#include "Config.h"
#include <vector>

namespace AriaMaestosa
{
	class Track;
	class GuitarEditor;
	
class TablatureExporter
{
	DECLARE_LEAK_CHECK();
	
	int measureAmount;
	int ticksPerBeat;
	//int noteAmount;
	GuitarEditor* editor;
	int string_amount;
	int max_length_of_a_line;
	
	bool checkRepetitions_bool;
	int measures_appended;
	std::vector<wxString> strings;
	wxString title_line;
	
	Track* track;
	wxFile* file;
	
	// ------- reset at every measure --------
	int zoom;
	int added_chars;
	// contains the number of characters each line contained, before adding new ones for this measure.
	int charAmountWhenBeginningMeasure;
	
public:
		
	TablatureExporter();
	void exportTablature(Track* t, wxFile* file, bool checkRepetitions);
	void setMaxLineWidth(int i);
	void flush();
};

}

#endif
