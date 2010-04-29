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

#include "Midi/Track.h"
#include "Pickers/KeyPicker.h"
#include "Editors/ScoreEditor.h"
#include "Editors/KeyboardEditor.h"
#include "GUI/GraphicalTrack.h"
#include "AriaCore.h"

namespace AriaMaestosa
{
    
    class CustomKeyDialog : public wxDialog
    {
        wxCheckBox* m_check_boxes[132];
    public:
        
        CustomKeyDialog(wxWindow* parent, GraphicalTrack* gtrack) : 
        wxDialog(parent, wxID_ANY, _("Custom Key Editor"), wxDefaultPosition,
                 wxSize(800,600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
        {
            const bool* curr_key_notes = gtrack->track->getKeyNotes();
            
            wxPanel* pane = new wxPanel(this);
            
            wxBoxSizer* over_sizer = new wxBoxSizer(wxVERTICAL);
            
            wxStaticBoxSizer* within_box_sizer = new wxStaticBoxSizer(wxVERTICAL, pane, _("Select which notes should be part of the custom key"));
            {
                wxScrolledWindow* scrollpane = new wxScrolledWindow(pane);
                
                wxBoxSizer* within_scrollpane_sizer = new wxBoxSizer(wxVERTICAL);
                
                Note12 note;
                int    octave;
                
                for (int pitch=4; pitch<=131; pitch++)
                {
                    if (Editor::findNoteName(pitch, &note, &octave))
                    {
                        wxString label = NOTE_12_NAME[note] + wxT(" ") + wxString::Format(wxT("%i"), octave);
                        wxCheckBox* cb = new wxCheckBox(scrollpane, wxID_ANY, label);
                        cb->SetValue( curr_key_notes[pitch] );
                        
                        within_scrollpane_sizer->Add(cb, 0, wxALL, 3);
                        
                        m_check_boxes[pitch] = cb;
                    }
                }
                
                scrollpane->SetSizer(within_scrollpane_sizer);
                scrollpane->FitInside();
                scrollpane->SetScrollRate(5, 5);  
                
                within_box_sizer->Add(scrollpane, 1, wxEXPAND);
            }
            over_sizer->Add(within_box_sizer, 1, wxEXPAND | wxALL, 2);
            
            {
                wxPanel* buttonsPane = new wxPanel(pane);
                over_sizer->Add(buttonsPane, 0, wxEXPAND | wxALL, 5);
                
                wxButton* ok_btn     = new wxButton(buttonsPane, wxID_OK, _("OK"));
                wxButton* cancel_btn = new wxButton(buttonsPane, wxID_CANCEL, _("Cancel"));
                
                wxBoxSizer* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
                btn_sizer->AddStretchSpacer();
                btn_sizer->Add(cancel_btn, 0, wxALL, 5);
                btn_sizer->Add(ok_btn, 0, wxALL, 5);
                buttonsPane->SetSizer(btn_sizer);
                
                ok_btn->SetDefault();
            }
            
            pane->SetSizer(over_sizer);
            Layout();
            
            Centre();
        }
        void onOK(wxCommandEvent& evt)
        {
            bool custom_key[132];
            custom_key[0] = false;
            custom_key[1] = false;
            custom_key[2] = false;
            custom_key[3] = false;
            for (int pitch=4; pitch<=131; pitch++)
            {
                custom_key[pitch] = m_check_boxes[pitch]->GetValue();
                std::cout << "Note " << pitch << " : " << custom_key[pitch] << std::endl;
            }
            
            //TODO: custom key support
            EndModal( GetReturnCode() );
        }
        void onCancel(wxCommandEvent& evt)
        {
            EndModal( GetReturnCode() );
        }
        
        DECLARE_EVENT_TABLE()
    };
    
    BEGIN_EVENT_TABLE(CustomKeyDialog, wxDialog)
    
    EVT_COMMAND(wxID_OK,     wxEVT_COMMAND_BUTTON_CLICKED, CustomKeyDialog::onOK )
    EVT_COMMAND(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, CustomKeyDialog::onCancel )
    
    END_EVENT_TABLE()
    
#if 0
#pragma mark -
#pragma mark KeyPicker
#endif
    
    enum IDs
    {
        MUSICAL_NOTATION = 1,
        LINEAR_NOTATION,
        F_CLEF,
        G_CLEF,
        OCTAVE_ABOVE,
        OCTAVE_BELOW,
        
        KEY_C_AM,
        KEY_SHARPS_1,
        KEY_SHARPS_2,
        KEY_SHARPS_3,
        KEY_SHARPS_4,
        KEY_SHARPS_5,
        KEY_SHARPS_6,
        KEY_SHARPS_7,
        KEY_FLATS_1,
        KEY_FLATS_2,
        KEY_FLATS_3,
        KEY_FLATS_4,
        KEY_FLATS_5,
        KEY_FLATS_6,
        KEY_FLATS_7,
        KEY_GUESS,
        KEY_CUSTOM,
        
        ID_AMOUNT
    };
}
using namespace AriaMaestosa;

BEGIN_EVENT_TABLE(KeyPicker, wxMenu)
EVT_MENU_RANGE(1, ID_AMOUNT-1, KeyPicker::menuItemSelected)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------------------------------

KeyPicker::KeyPicker() : wxMenu()
{
    //I18N: - in the view settings for score editor. whether to show notes in a traditional musical way.
    musical_checkbox = AppendCheckItem(MUSICAL_NOTATION,_("Musical notation")); musical_checkbox->Check(true);
    //I18N: - in the view settings for score editor. whether to show notes in a "computer" linear way.
    linear_checkbox = AppendCheckItem(LINEAR_NOTATION,_("Linear Notation")); linear_checkbox->Check(true);
    
    //AppendSeparator();
    gclef = AppendCheckItem(G_CLEF, _("G Clef")); gclef->Check(true);
    fclef = AppendCheckItem(F_CLEF, _("F Clef")); fclef->Check(true);
    //I18N: - show score an octave higher
    octave_above = AppendCheckItem(OCTAVE_ABOVE, _("Octave +1"));
    //I18N: - show score an octave lower
    octave_below = AppendCheckItem(OCTAVE_BELOW, _("Octave -1"));
    
    score_items_added = true;
    
    AppendSeparator();
    key_c = AppendCheckItem(KEY_C_AM, wxT("C, Am"));
    key_c->Check(true);
    
    AppendSeparator();
    key_sharps_1 = AppendCheckItem( KEY_SHARPS_1, wxT("G, Em"));
    key_sharps_2 = AppendCheckItem( KEY_SHARPS_2, wxT("D, Bm"));
    key_sharps_3 = AppendCheckItem( KEY_SHARPS_3, wxT("A, F#m"));
    key_sharps_4 = AppendCheckItem( KEY_SHARPS_4, wxT("E, C#m"));
    key_sharps_5 = AppendCheckItem( KEY_SHARPS_5, wxT("B, G#m"));
    key_sharps_6 = AppendCheckItem( KEY_SHARPS_6, wxT("F#, D#m"));
    key_sharps_7 = AppendCheckItem( KEY_SHARPS_7, wxT("C#, A#m"));
    
    AppendSeparator();
    key_flats_1 = AppendCheckItem( KEY_FLATS_1, wxT("F, Dm"));
    key_flats_2 = AppendCheckItem( KEY_FLATS_2, wxT("Bb, Gm"));
    key_flats_3 = AppendCheckItem( KEY_FLATS_3, wxT("Eb, Cm"));
    key_flats_4 = AppendCheckItem( KEY_FLATS_4, wxT("Ab, Fm"));
    key_flats_5 = AppendCheckItem( KEY_FLATS_5, wxT("Db, Bbm"));
    key_flats_6 = AppendCheckItem( KEY_FLATS_6, wxT("Gb, Ebm"));
    key_flats_7 = AppendCheckItem( KEY_FLATS_7, wxT("Cb, Abm"));
    
    AppendSeparator();
    Append( KEY_GUESS,  _("Guess Key"));
    //Append( KEY_CUSTOM, _("Custom Key"));
}

// -------------------------------------------------------------------------------------------------------

void KeyPicker::setParent(Track* parent_arg)
{
    parent = parent_arg->graphics;
    if (parent->editorMode == KEYBOARD)
    {
        if (score_items_added)
        {
            Remove(musical_checkbox);
            Remove(linear_checkbox);
            Remove(fclef);
            Remove(gclef);
            Remove(octave_above);
            Remove(octave_below);
            score_items_added = false;
        }
    }
    else if (parent->editorMode == SCORE)
    {
        if (not score_items_added)
        {
            Prepend(octave_below);
            Prepend(octave_above);
            Prepend(gclef);
            Prepend(fclef);
            //PrependSeparator();
            Prepend(linear_checkbox);
            Prepend(musical_checkbox);
            score_items_added = true;
        }
    }
    
    key_c->Check(false);
    key_sharps_1->Check(false);
    key_sharps_2->Check(false);
    key_sharps_3->Check(false);
    key_sharps_4->Check(false);
    key_sharps_5->Check(false);
    key_sharps_6->Check(false);
    key_sharps_7->Check(false);
    key_flats_1->Check(false);
    key_flats_2->Check(false);
    key_flats_3->Check(false);
    key_flats_4->Check(false);
    key_flats_5->Check(false);
    key_flats_6->Check(false);
    key_flats_7->Check(false);
    
    const int sharps = parent_arg->getKeySharpsAmount();
    const int flats  = parent_arg->getKeyFlatsAmount();
    
    if (sharps==0 and flats==0)
    {
        key_c->Check(true);
    }
    else if (sharps > flats)
    {
        if (sharps == 1) key_sharps_1->Check(true);
        else if (sharps == 2) key_sharps_2->Check(true);
        else if (sharps == 3) key_sharps_3->Check(true);
        else if (sharps == 4) key_sharps_4->Check(true);
        else if (sharps == 5) key_sharps_5->Check(true);
        else if (sharps == 6) key_sharps_6->Check(true);
        else if (sharps == 7) key_sharps_7->Check(true);
    }
    else
    {
        if (flats == 1) key_flats_1->Check(true);
        else if (flats == 2) key_flats_2->Check(true);
        else if (flats == 3) key_flats_3->Check(true);
        else if (flats == 4) key_flats_4->Check(true);
        else if (flats == 5) key_flats_5->Check(true);
        else if (flats == 6) key_flats_6->Check(true);
        else if (flats == 7) key_flats_7->Check(true);
    }
}

// -------------------------------------------------------------------------------------------------------

KeyPicker::~KeyPicker()
{
}

// -------------------------------------------------------------------------------------------------------

void KeyPicker::setChecks( bool musicalNotationEnabled, bool linearNotationEnabled, bool f_clef, bool g_clef, int octave_shift )
{
    musical_checkbox -> Check(musicalNotationEnabled);
    linear_checkbox  -> Check(linearNotationEnabled);
    fclef            -> Check(f_clef);
    gclef            -> Check(g_clef);
    
    if (octave_shift == -1)
    {
        octave_above -> Check(false);
        octave_below -> Check(true);
    }
    else if (octave_shift == 1)
    {
        octave_above -> Check(true);
        octave_below -> Check(false);
    }
    else
    {
        octave_above -> Check(false);
        octave_below -> Check(false);
    }
}

// -------------------------------------------------------------------------------------------------------

void KeyPicker::menuItemSelected(wxCommandEvent& evt)
{
    const int id = evt.GetId();
    
    if ( id < 0 or id > ID_AMOUNT ) return;
    
    if ( id == MUSICAL_NOTATION )
    {
        parent -> scoreEditor -> enableMusicalNotation( musical_checkbox->IsChecked() );
        
        // don't allow disabling both
        if (not musical_checkbox->IsChecked() and not linear_checkbox->IsChecked())
            parent -> scoreEditor -> enableLinearNotation( true );
    }
    else if ( id == LINEAR_NOTATION )
    {
        parent -> scoreEditor -> enableLinearNotation( linear_checkbox->IsChecked() );
        
        // don't allow disabling both
        if (not musical_checkbox->IsChecked() and not linear_checkbox->IsChecked())
            parent -> scoreEditor -> enableMusicalNotation( true );
    }
    else if ( id == F_CLEF )
    {
        parent -> scoreEditor -> enableFClef( fclef->IsChecked() );
        
        // don't allow disabling both
        if (not gclef->IsChecked() and not fclef->IsChecked())
            parent -> scoreEditor -> enableGClef( true );
    }
    else if ( id == G_CLEF )
    {
        parent -> scoreEditor -> enableGClef( gclef->IsChecked() );
        
        // don't allow disabling both
        if (not gclef->IsChecked() and not fclef->IsChecked())
            parent -> scoreEditor -> enableFClef( true );
    }
    else if ( id == OCTAVE_ABOVE )
    {
        octave_below -> Check(false);
        parent -> scoreEditor -> getScoreMidiConverter() -> setOctaveShift( octave_above->IsChecked() ? 1 : 0 );
    }
    else if ( id == OCTAVE_BELOW )
    {
        octave_above -> Check(false);
        parent -> scoreEditor -> getScoreMidiConverter() -> setOctaveShift( octave_below->IsChecked() ? -1 : 0 );
    }
    else if ( id == KEY_C_AM )
    {
        parent->track->setKey(0, NATURAL);
    }
    else if ( id >= KEY_SHARPS_1 and id <= KEY_SHARPS_7 )
    {
        parent->track->setKey(id-KEY_SHARPS_1+1, SHARP);
    }
    else if ( id >= KEY_FLATS_1 and id <= KEY_FLATS_7 )
    {
        parent->track->setKey(id-KEY_FLATS_1+1, FLAT);
    }
    else if ( id == KEY_CUSTOM )
    {
        CustomKeyDialog dialog( (wxFrame*)getMainFrame(), parent );
        dialog.ShowModal();
    }
    else if ( id == KEY_GUESS )
    {
        int note_12_occurance[12];
        for (int n=0; n<12; n++)
        {
            note_12_occurance[n] = 0;
        }
        
        // count how many A's, how many B's, etc., we have in this track
        const int noteAmount = parent->track->getNoteAmount();
        for (int n=0; n<noteAmount; n++)
        {
            const int pitch = parent->track->getNotePitchID(n);
            int r = 11-pitch%12;
            if (r < 0)  r+=12;
            if (r > 11) r-=12;
            //std::cout << "r=" << r << std::endl;
            note_12_occurance[r] += 1;
        }
        
        // find max value, will make it easier to compare values
        //int max = 0;
        //for(int n=0; n<12; n++)
        //    if (note_12_occurance[n] > max) max = note_12_occurance[n];
        // print values for debug purposes
        //for(int n=0; n<12; n++)
        //    std::cout << (note_12_occurance[n] * 100 / max) << std::endl;
        
        // test the note repartition pattern found against keys
        // and grade them ( smaller = less alike, bigger = much alike )
        // here we only test major keys; a minor one will get the same key sig
        int key_test[12];
        for (int n=0; n<12; n++)
        {
            key_test[n] = 0; // reset before starting
            
            // some values are somehwat arbitrarly multiplied by 2 cause
            // they are more likely/unlikely to be found in a given key
            
            key_test[n] += note_12_occurance[n]*2; // tonic
            key_test[n] -= note_12_occurance[(n+1)%12]*2; // 2nd minor 
            key_test[n] += note_12_occurance[(n+2)%12]; // 2nd major 
            key_test[n] -= note_12_occurance[(n+3)%12]; // 3rd minor 
            key_test[n] += note_12_occurance[(n+4)%12]; // 3rd major 
            key_test[n] += note_12_occurance[(n+5)%12]; // 4th
            key_test[n] -= note_12_occurance[(n+6)%12]*2; // tritone
            key_test[n] += note_12_occurance[(n+7)%12]*2; // fifth
                                                          //key_test[n] -= note_12_occurance[(n+8)%12]/2; // 6th minor
                                                          //key_test[n] += note_12_occurance[(n+9)%12]/2; // 6th major
                                                          // 6ths are less accounted for, since they could be the 7th of a minor key
            key_test[n] -= note_12_occurance[(n+10)%12]; // 7th minor
            key_test[n] += note_12_occurance[(n+11)%12]; // 7th major
        }
        
        int best_candidate = -1;
        for (int n=0; n<12; n++)
        {
            if (best_candidate==-1 or key_test[n]>key_test[best_candidate]) best_candidate = n;
        }
        if (best_candidate==-1) return;
        
        PitchSign sign = NATURAL;
        int amount = 0;
        switch (best_candidate)
        {
            case 0: //C
                break;
                
            case 1: // C#
                sign = SHARP;
                amount = 7;
                break;
            case 2: // D
                sign = SHARP;
                amount = 2;
                break;
            case 4: // E
                sign = SHARP;
                amount = 4;
                break; 
            case 6: // F#
                sign = SHARP;
                amount = 6;
                break; 
            case 7: // G
                sign = SHARP;
                amount = 1;
                break; 
            case 9: // A
                sign = SHARP;
                amount = 3;
                break; 
            case 11: // B
                sign = SHARP;
                amount = 5;
                break; 
                
            case 3: // D#
                sign = FLAT;
                amount = 3;
                break;
            case 5: // F
                sign = FLAT;
                amount = 1;
                break; 
            case 8: // G#
                sign = FLAT;
                amount = 4;
                break; 
            case 10: // A#
                sign = FLAT;
                amount = 2;
                break; 
        }
        parent->track->setKey(amount, sign);
        //parent -> scoreEditor    -> loadKey(sign, amount);
        //parent -> keyboardEditor -> loadKey(sign, amount);
        /*
         wxString choices[12] = { wxT("C"), wxT("C#"), wxT("D"), wxT("D#"), wxT("E"), wxT("F"),
         wxT("F#"), wxT("G"), wxT("G#"), wxT("A"), wxT("A#"), wxT("B")};
         std::cout << "best_candidate = " << choices[best_candidate].mb_str() << std::endl;
         */
    }
    
    Display::render();
}

// -------------------------------------------------------------------------------------------------------

