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


#include "Dialogs/Preferences.h"

#include <iostream>
#include <wx/config.h>
#include "AriaCore.h"

#include "GUI/MainFrame.h"
#include "languages.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa {

class QuickBoxLayout
{
    wxBoxSizer* bsizer;
public:
    wxPanel* pane;
    
    QuickBoxLayout(wxWindow* component, wxSizer* parent, int orientation=wxHORIZONTAL)
    {
        pane = new wxPanel(component);
        parent->Add(pane,1,wxEXPAND);
        bsizer = new wxBoxSizer(orientation);
    }
    void add(wxWindow* window)
    {
        bsizer->Add(window, 1, wxALL, 10);
    }
    ~QuickBoxLayout()
    {
        pane->SetSizer(bsizer);
        bsizer->Layout();
        //bsizer->SetSizeHints(pane);
    }
};

    
bool follow_playback = false;
bool followPlaybackByDefault()
{
    return follow_playback;
}
    
int play_during_edit = PLAY_ON_CHANGE;
int playDuringEditByDefault()
{
    return play_during_edit;
}
  
int showLinear = true, showMusical = true;
int showLinearViewByDefault()
{
    return showLinear;
}
int showMusicalViewByDefault()
{
    return showMusical;
}

BEGIN_EVENT_TABLE(Preferences, wxDialog)

EVT_CHOICE(1, Preferences::languageSelected)
EVT_BUTTON(2, Preferences::okClicked)
EVT_CHOICE(3, Preferences::playSelected)
EVT_CHECKBOX(4, Preferences::followPlaybackChecked )
EVT_CHOICE(5, Preferences::scoreViewSelected)
    
END_EVENT_TABLE()

Preferences::Preferences(MainFrame* parent) : wxDialog(parent, wxID_ANY,
                                                       //I18N: - title of the preferences dialog 
                                                       _("Preferences"),
                                                       wxPoint(100,100), wxSize(500,300), wxCAPTION )
{

    Preferences::parent = parent;

    vert_sizer = new wxBoxSizer(wxVERTICAL);

    // language
    {
    QuickBoxLayout language_box(this, vert_sizer);
    //I18N: - in the preferences
    language_box.add(new wxStaticText(language_box.pane , wxID_ANY,  _("Language")));
    
    lang_combo = new wxChoice(language_box.pane, 1, wxDefaultPosition, wxDefaultSize, getLanguageList() );
    language_box.add(lang_combo);
    }

    // play settings
    {
    QuickBoxLayout play_box(this, vert_sizer);
    //I18N: - in the preferences
    play_box.add( new wxStaticText(play_box.pane, wxID_ANY,  _("Play during edit (default value)")) );

    //I18N: - in the preferences, for "Play during edit (default value)" (the 3 chocies being "Always", "On note change" and "Never")
    wxString choices[3] = { _("Always"),  _("On note change"),  _("Never")};

    play_combo = new wxChoice(play_box.pane, 3, wxDefaultPosition, wxDefaultSize, 3, choices );
    play_box.add( play_combo );
    }
    
    // score view
    {
        QuickBoxLayout score_view_box(this, vert_sizer);
        //I18N: - in the preferences
        score_view_box.add(new wxStaticText(score_view_box.pane , wxID_ANY,  _("Default Score View")));
        
        //I18N: - in the preferences, where we choose default score view
        wxString choices[3] = { _("Both Musical and Linear"),  _("Musical Only"),  _("Linear Only")};
        scoreview_combo = new wxChoice(score_view_box.pane, 5, wxDefaultPosition, wxDefaultSize, 3, choices );
        score_view_box.add(scoreview_combo);
    }
    
    {
        //I18N: - in the preferences
        follow_playback_checkbox = new wxCheckBox(this, 4, _("Follow playback by default"), wxDefaultPosition, wxDefaultSize );
        vert_sizer->Add( follow_playback_checkbox, 0, wxALL, 10 );
    }
    
    // *********** fill values ********
    
    lang_combo->Select( getDefaultLanguageID() );

    wxConfig* prefs;
    prefs = (wxConfig*) wxConfig::Get();

    // --- read settings from existing prefs -----
    long play_v;
    if(prefs->Read( wxT("playDuringEdit"), &play_v) )
    {
        if(play_v == PLAY_ALWAYS)
        {
            play_combo->Select(0);
            play_during_edit = PLAY_ALWAYS;
        }
        else if(play_v == PLAY_ON_CHANGE)
        {
            play_combo->Select(1);
            play_during_edit = PLAY_ON_CHANGE;
        }
        else if(play_v == PLAY_NEVER)
        {
            play_combo->Select(2);
            play_during_edit = PLAY_NEVER;
        }
    }
    else
    {
        play_combo->Select(1);
    }
    
    long followp;
    if(prefs->Read( wxT("followPlayback"), &followp) )
    {
        follow_playback = followp;
        follow_playback_checkbox->SetValue(follow_playback);
    }

    long scorev;
    if(prefs->Read( wxT("scoreview"), &scorev) )
    {
        scoreview_combo->SetSelection(scorev);
        if(scorev == 0)
        {
            showLinear = true;
            showMusical = true;
        }
        else if(scorev == 1)
        {
            showLinear = false;
            showMusical = true;
        }
        else if(scorev == 2)
        {
            showLinear = true;
            showMusical = false;
        }
    }
    // -----------------------------------

    ok_btn = new wxButton(this, 2, wxT("OK"));
    vert_sizer->Add( ok_btn, 0, wxALL, 10 );

    //I18N: - in the preferences dialog
    wxStaticText* effect_label = new wxStaticText(this, wxID_ANY,  _("Changes will take effect next time you open the app."));
    effect_label->SetFont( wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL) );
    vert_sizer->Add( effect_label, 0, wxALL, 10 );    
    
    ok_btn -> SetDefault();

    SetSizer( vert_sizer );
    vert_sizer->Layout();
    vert_sizer->SetSizeHints( this );
}

Preferences::~Preferences()
{
}

void Preferences::show()
{
    Center();
    modalCode = ShowModal();
}

void Preferences::languageSelected(wxCommandEvent& evt)
{
    setDefaultLanguage( lang_combo->GetStringSelection() );
}

void Preferences::playSelected(wxCommandEvent& evt)
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    prefs->Write( wxT("playDuringEdit"), play_combo->GetSelection() );
    prefs->Flush();
}

void Preferences::followPlaybackChecked(wxCommandEvent& evt)
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    prefs->Write( wxT("followPlayback"), follow_playback_checkbox->GetValue() );
    prefs->Flush();
}

void Preferences::scoreViewSelected(wxCommandEvent& evt)
{
    wxConfig* prefs = (wxConfig*) wxConfig::Get();
    prefs->Write( wxT("scoreview"), scoreview_combo->GetSelection() );
    prefs->Flush();
}

void Preferences::okClicked(wxCommandEvent& evt)
{
    wxDialog::EndModal(modalCode);
}

}
