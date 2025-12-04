// class that shows a dialog to create, edit, enable/disable, or delete multiple alarms
// buttons on bottom row: [Add New] [Remove Disabled] [Close]
//
// main area shows scrollable list of alarms, one per line, with elements as follows:
// Function: enable  day-of-week time-of-day   am/pm   notes
// Type:    [toggle]  [chooser]  [spin][spin] [toggle] [text]
//
// could also add element to select alarm sound

// all enabled alarm entries, regardless of how they are entered/displayed in the list,
// must be sorted internally by day-of-week and time-of-day in order to sequence from earlier
// alarms to later ones, so an internally sorted list would look like:
//
// Mon 1:30 AM
// Mon 2:15 AM
// Mon 1:02 PM
// Tue 1:15 AM
// Tue 4:30 PM
// Tue 5:45 PM
// ....
//
// this will require parsing and sorting logic, and then do a binary search to
// compare to the current time.
//
// or, I can skip all that (since the list could be changed at any moment) and scan all alarms
// and compare them to the current time every second without sorting and searching. Much simpler
// code with less to keep track of. I like this better!
//
// all it will take is:
// for(int i = 0; i < alarms; i++){
//  if(alarm[i] == currentTime){
//      playAlarm();
//  }
// }

#include "wx/colour.h"
#include "wx/dialog.h"
#include "wx/choice.h"
#include "wx/event.h"
#include "wx/frame.h"
#include "wx/checkbox.h"
#include "wx/list.h"
#include "wx/spinctrl.h"
#include "wx/textctrl.h"
#include "wx/spinbutt.h"
#include "wx/tglbtn.h"
#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/types.h"
#include "wx/vector.h"
#include "wx/window.h"
#include "wx/frame.h"


class Entry;
struct AlarmTime {
    std::string day;
    std::string time;
};

class AlarmsDlg : public wxDialog
{
public:
    AlarmsDlg(wxWindow *parent, wxColor fg, wxColor bg);
    wxColor fgcol;
    wxColor bgcol;  // from config via AlarmControlFrame
    wxBoxSizer *dlg_entriesSizer; // display area in the dialog of list of entries
    wxBoxSizer *dlg_sizerButtons; // sizer container for dialog control buttons
    wxBoxSizer *dlg_mainSizer;    // master sizer for the dialog
    wxButton *dlg_close;
    wxButton *dlg_temp;
    std::vector<Entry *> entryVec;
    int idx;
    std::vector<AlarmTime>& getAlarms();
    void OnClose(wxCommandEvent &);
    void OnTemp(wxCommandEvent &);
};

class Entry : public wxEvtHandler
{
public:
    Entry(wxDialog *parent,  int n);

    wxCheckBox *entry_checkBoxEnable;
    wxChoice *entry_choiceDay;
    wxSpinCtrl *entry_spinHour;
    wxSpinCtrl *entry_spinMinute;
    wxChoice *entry_choiceAMPM;
    wxTextCtrl  *entry_textCtrlNote;

    wxBoxSizer *entrySizer;

    void set_colors(wxColor fg, wxColor bg);

};