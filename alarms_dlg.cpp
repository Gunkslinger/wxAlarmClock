// class that shows a dialog to create, edit, enable/disable, or delete multiple alarms
// buttons on bottom row: [New] [Edit] [Toggle On/Off(?)] [Remove]
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
#include "enums.hpp"
#include "wx/any.h"
#include "wx/anybutton.h"
#include "wx/arrstr.h"
#include "wx/datetime.h"
#include "wx/dialog.h"
#include "wx/choice.h"
#include "wx/button.h"
#include "wx/dynarray.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/checkbox.h"
#include "wx/spinctrl.h"
#include "wx/sizer.h"
#include "wx/stringimpl.h"
#include "wx/textctrl.h"
#include "wx/spinbutt.h"
#include "wx/tglbtn.h"
#include <cstddef>
#include <ostream>
#include "alarms_dlg.hpp"

enum{
    ID_DLG_CB_ENABLE = 1000,
    ID_DLG_DAY_CHOICE = 1100,
    ID_DLG_SPIN_HOURS = 1200,
    ID_DLG_SPIN_MINUTES = 1300,
    ID_DLG_AMPM_CHOICE = 1400,
    ID_DLG_TEXTCTRL_NOTES = 1500
};

// represents a single entry in the list of alarms the user has created.
Entry::Entry(wxDialog *parent, int n)
{
    entrySizer = new wxBoxSizer(wxHORIZONTAL); // sizer to contain the entry elements
    entry_checkBoxEnable = new wxCheckBox(parent, ID_DLG_CB_ENABLE+n, "");
    entry_checkBoxEnable->SetValue(true);
    entrySizer->Add(entry_checkBoxEnable, 0, wxEXPAND|wxALL, 5);
    wxArrayString *days = new wxArrayString({"Mon","Tue", "Wed", "Thu", "Fri",
                            "Sat", "Sun", "ALL"});
    entry_choiceDay = new wxChoice(parent, ID_DLG_DAY_CHOICE+n, wxDefaultPosition, wxDefaultSize, *days);
    entry_choiceDay->SetSelection(7);
    entrySizer->Add(entry_choiceDay, 0, wxALIGN_LEFT);
    entrySizer->AddSpacer(10);
    entry_spinHour = new wxSpinCtrl(parent, ID_DLG_SPIN_HOURS+n);
    entry_spinHour->SetRange(1, 12);
    entrySizer->Add(entry_spinHour);
    entrySizer->AddSpacer(10);
    entry_spinMinute = new wxSpinCtrl(parent, ID_DLG_SPIN_MINUTES+n);
    entry_spinMinute->SetRange(0, 59);
    entrySizer->Add(entry_spinMinute, 0, wxBOTTOM);
    entrySizer->AddSpacer(10);
    entry_choiceAMPM = new wxChoice(parent, ID_DLG_AMPM_CHOICE+n);
    entry_choiceAMPM->Append("AM");
    entry_choiceAMPM->Append("PM");
    entry_choiceAMPM->SetSelection(0);
    entrySizer->Add(entry_choiceAMPM, 0, 0);
    entry_textCtrlNote = new wxTextCtrl(parent, ID_DLG_TEXTCTRL_NOTES+n, "", wxDefaultPosition, wxSize(400, 10));
    entrySizer->Add(entry_textCtrlNote, 1, wxEXPAND);
};

void Entry::set_colors(wxColor fg, wxColor bg){
    entry_checkBoxEnable->SetForegroundColour(fg);
    entry_checkBoxEnable->SetBackgroundColour(bg);
    entry_choiceDay->SetForegroundColour(fg);
    entry_choiceDay->SetBackgroundColour(bg);
    entry_choiceDay->SetOwnBackgroundColour(bg);
    entry_spinHour->SetForegroundColour(fg);
    entry_spinHour->SetBackgroundColour(bg);
    entry_spinMinute->SetForegroundColour(fg);
    entry_spinMinute->SetBackgroundColour(bg);
     entry_choiceAMPM->SetForegroundColour(fg);
    entry_choiceAMPM->SetBackgroundColour(bg);
   entry_textCtrlNote->SetForegroundColour(fg);
    entry_textCtrlNote->SetBackgroundColour(bg);
}

AlarmsDlg::AlarmsDlg(wxWindow *parent, wxColor fg, wxColor bg)
    : wxDialog(parent, wxID_ANY, "alarmsDialog")
{
    fgcol = fg;
    bgcol = bg;

    this->SetSize(800, 450);

    dlg_close = new wxButton(this, ID_DLG_CLOSE, "Close");
    dlg_temp = new wxButton(this, ID_DLG_TEMP, "Temp");

    
    dlg_entriesSizer = new wxBoxSizer(wxVERTICAL);
    dlg_entriesSizer->AddSpacer(20);
    for(int n = 0; n < 7; n++){
        Entry *ent = new Entry(this, n);
        ent->set_colors(fgcol, bgcol);
        entryVec.push_back(ent);
        idx = entryVec.size() - 1;
        dlg_entriesSizer->Add(ent->entrySizer);
        Layout();
    //std::cout << "entries: " << entryVec.size() << std::endl;

    }

    dlg_mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(dlg_mainSizer);
    dlg_mainSizer->Add(dlg_entriesSizer,1,  0, 1);
    dlg_mainSizer->AddStretchSpacer(1);

    dlg_sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    dlg_sizerButtons->Add(dlg_temp);
    dlg_sizerButtons->AddSpacer(10);
    dlg_sizerButtons->Add(dlg_close);

    dlg_mainSizer->Add(dlg_sizerButtons,0, wxALIGN_CENTER);

    SetForegroundColour(fgcol);
    SetBackgroundColour(bgcol);
    dlg_close->SetForegroundColour(fgcol);
    dlg_close->SetBackgroundColour(bgcol);
    
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmsDlg::OnClose, this, ID_DLG_CLOSE);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmsDlg::OnTemp, this, ID_DLG_TEMP);
}

std::vector<AlarmTime>& AlarmsDlg::getAlarms()
{
    static std::vector<AlarmTime> at;
    at.clear();
    AlarmTime a;
    for(int i = 0; i < entryVec.size(); i++){
        if(!entryVec[i]->entry_checkBoxEnable->GetValue()) continue;
        a.day = entryVec[i]->entry_choiceDay->GetString(entryVec[i]->entry_choiceDay->GetSelection());
        wxDateTime dt = wxDateTime::Now();
        dt.SetHour(entryVec[i]->entry_spinHour->GetValue() + (entryVec[i]->entry_choiceAMPM->GetSelection() ? 12:0));
        dt.SetMinute(entryVec[i]->entry_spinMinute->GetValue());
        dt.SetSecond(0);
        a.time = dt.FormatISOTime();
        at.push_back(a);
    }
    return at;
}

void AlarmsDlg::OnTemp(wxCommandEvent &e)
{   // this func is the current time day formatting and matching logic
    // this should be the first filter before time of day matching
    std::vector<AlarmTime>& at = getAlarms();
    wxDateTime dt = wxDateTime::Now();

    std::cout << dt.Format("%a") << std::endl;
    for(int n = 0; n < 7; n++){
        std::cout << n << ": "
        << at[n].day
        << " "
        << at[n].time;

        if(at[n].day == dt.Format("%a") || at[n].day == "ALL")
            std::cout << " " << n << ": matches" << std::endl;
        else
            std::cout << " " << n << ": does not match" << std::endl;

    }
}


void AlarmsDlg::OnClose(wxCommandEvent &e)
{
    Close();
}