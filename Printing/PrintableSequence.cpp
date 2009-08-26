
#include "Printing/PrintableSequence.h"
#include "Printing/PrintingBase.h"
#include "Printing/PrintLayoutLine.h"
#include "Printing/TabPrint.h"
#include "Printing/ScorePrint.h"

#include "GUI/MainFrame.h"
#include "Midi/Track.h"
#include "Midi/Sequence.h"

namespace AriaMaestosa
{

PrintableSequence::PrintableSequence(Sequence* parent)
{
    sequence = parent;
    track_amount = 0;
    is_guitar_editor_used = false;
    is_score_editor_used = false;
    max_signs_in_keysig = 0;
}

// -----------------------------------------------------------------------------------------------------------------------
bool PrintableSequence::addTrack(Track* track, int mode /* GUITAR, SCORE, etc. */)
{
    if (mode == GUITAR)
    {
        editorPrintables.push_back(new TablaturePrintable(track));
        is_guitar_editor_used = true;
    }
    else if (mode == SCORE)
    {
        editorPrintables.push_back(new ScorePrintable());
        is_score_editor_used = true;
        
        max_signs_in_keysig = std::max( max_signs_in_keysig,
                                       std::max(track->graphics->getCurrentEditor()->getKeySharpsAmount(),
                                                track->graphics->getCurrentEditor()->getKeyFlatsAmount()) );
    }
    else
    {
        std::cerr << "PrintableSequence::addTrack : mode " << mode << " not supported for printing" << std::endl;
        return false;
    }
    tracks.push_back(track);
    track_amount = tracks.size();
    return true;
}

// -----------------------------------------------------------------------------------------------------------------------
void PrintableSequence::calculateLayout(bool checkRepetitions)
{
    layout = new PrintLayoutManager(this, layoutPages /* out */);
    layout->generateMeasures(tracks);
    layout->calculateLayoutElements(tracks, checkRepetitions);
}

// -----------------------------------------------------------------------------------------------------------------------
EditorPrintable* PrintableSequence::getEditorPrintable(const int trackID)
{
    return editorPrintables.get(trackID);
}

// -----------------------------------------------------------------------------------------------------------------------
wxString PrintableSequence::getTitle()
{
    wxString song_title = sequence->suggestTitle();
    wxString track_title;
    if (tracks.size()==1) tracks[0].getName();
    
    wxString final_title;
    
    // give song title
    if (song_title.IsSameAs(_("Untitled")))
        final_title = _("Aria Maestosa song");
    else
        final_title = song_title;
    
    // give track name, if any
    if (!track_title.IsSameAs(_("Untitled")) and track_title.Length()>0) final_title += (wxT(", ") + track_title);
    
    std::cout << "Title = " << final_title.mb_str() << std::endl;
    return final_title;
}

// -----------------------------------------------------------------------------------------------------------------------
int PrintableSequence::getPageAmount()
{
    return layoutPages.size();
}
// -----------------------------------------------------------------------------------------------------------------------
LayoutPage& PrintableSequence::getPage(const int id)
{
    assertExpr(id, >=, 0);
    assertExpr(id, <, (int)layoutPages.size());
    
    LayoutPage& out = layoutPages[id];
    
    assert( MAGIC_NUMBER_OK_FOR(&out) );
    
    return out;
}
    
}
