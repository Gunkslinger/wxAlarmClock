// class that shows a dialog to edit, enable/disable, 7 alarms
// buttons on bottom row: [Save] [Close]
//
// main area shows list of 7 alarms, one per line, with elements as follows:
// Function: enable  day-of-week time-of-day   am/pm   notes
// Type:    [toggle]  [chooser]  [spin][spin] [toggle] [text]
//
// could also add element to select alarm sound

// the alarms are loaded into a std::vector of Entry classes with is retrieved
// by the mainwindow class and cycled through on each timer event to see if one
// matches the current time and day-of-week. if so the alarm is sounded.

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
#include "wx/gtk/colour.h"
#include "wx/spinctrl.h"
#include "wx/sizer.h"
#include "wx/stringimpl.h"
#include "wx/textctrl.h"
#include "wx/spinbutt.h"
#include "wx/tglbtn.h"
#include <cstddef>
#include <cstdio>
#include <ostream>
#include "alarms_dlg.hpp"
#include <cstddef>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
json alarm_data;

enum{
    ID_DLG_CB_ENABLE = 1000,
    ID_DLG_DAY_CHOICE = 1100,
    ID_DLG_SPIN_HOURS = 1200,
    ID_DLG_SPIN_MINUTES = 1300,
    ID_DLG_AMPM_CHOICE = 1400,
    ID_DLG_TEXTCTRL_NOTES = 1500
};

// represents a single entry in the list of alarms the user has created.
Entry::Entry(wxDialog *parent)
{
    entrySizer = new wxBoxSizer(wxHORIZONTAL); // sizer to contain the entry elements
    entry_checkBoxEnable = new wxCheckBox(parent, ID_DLG_CB_ENABLE, "");
    entry_checkBoxEnable->SetValue(true);
    entrySizer->Add(entry_checkBoxEnable, 0, wxEXPAND|wxALL, 5);
    wxArrayString *days = new wxArrayString({"Mon","Tue", "Wed", "Thu", "Fri",
                            "Sat", "Sun", "ALL"});
    entry_choiceDay = new wxChoice(parent, ID_DLG_DAY_CHOICE, wxDefaultPosition, wxDefaultSize, *days);
    entry_choiceDay->SetSelection(7);
    entrySizer->Add(entry_choiceDay, 0, wxALIGN_LEFT);
    entrySizer->AddSpacer(10);
    entry_spinHour = new wxSpinCtrl(parent, ID_DLG_SPIN_HOURS);
    entry_spinHour->SetRange(1, 12);
    entrySizer->Add(entry_spinHour);
    entrySizer->AddSpacer(10);
    entry_spinMinute = new wxSpinCtrl(parent, ID_DLG_SPIN_MINUTES);
    entry_spinMinute->SetRange(0, 59);
    entrySizer->Add(entry_spinMinute, 0, wxBOTTOM);
    entrySizer->AddSpacer(10);
    entry_choiceAMPM = new wxChoice(parent, ID_DLG_AMPM_CHOICE);
    entry_choiceAMPM->Append("AM");
    entry_choiceAMPM->Append("PM");
    entry_choiceAMPM->SetSelection(0);
    entrySizer->Add(entry_choiceAMPM, 0, 0);
    entry_textCtrlNote = new wxTextCtrl(parent, ID_DLG_TEXTCTRL_NOTES, "", wxDefaultPosition, wxSize(400, 10));
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
    dlg_save = new wxButton(this, ID_DLG_SAVE, "Save");
    dlg_save->Enable(false);
    dlg_save->SetBackgroundColour(disbg);
    dlg_save->SetForegroundColour(disfg);

    dlg_entriesSizer = new wxBoxSizer(wxVERTICAL);
    dlg_entriesSizer->AddSpacer(20);

    //Open alarms file for loading alarms.
    std::string alrm = std::getenv("HOME");
    alrm.append("/.config/wxAlarmClock/Alarms.json");
    std::ifstream af(alrm);

    // Create alarm entries and load them from saved alarms file and push into the Entry's vector
    // each Entry's CTOR generates the needed widgets and their values are set here.
    // the alarm_data object could be sent as a param to Entry's CTOR but what the heck.

    for(int n = 0; n < 7; n++){
        Entry *ent = new Entry(this);

        af >> alarm_data;
        ent->entry_checkBoxEnable->SetValue(alarm_data["enable"]);
        ent->entry_choiceDay->SetSelection(alarm_data["day"]);
        ent->entry_spinHour->SetValue(alarm_data["hour"]);
        ent->entry_spinMinute->SetValue(alarm_data["minute"]);
        ent->entry_choiceAMPM->SetSelection(alarm_data["ampm"]);
        ent->entry_textCtrlNote->ChangeValue((std::string) alarm_data["note"]);
        //std::cout << alarm_data << std::endl;

        ent->set_colors(fgcol, bgcol);
        entryVec.push_back(ent);
        dlg_entriesSizer->Add(ent->entrySizer);
        Layout();
    }
    af.close();

    dlg_mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(dlg_mainSizer);
    dlg_mainSizer->Add(dlg_entriesSizer,1,  0, 1);
    dlg_mainSizer->AddStretchSpacer(1);

    dlg_sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    dlg_sizerButtons->Add(dlg_save);
    dlg_sizerButtons->AddSpacer(10);
    dlg_sizerButtons->Add(dlg_close);

    dlg_mainSizer->Add(dlg_sizerButtons,0, wxALIGN_CENTER);

    SetForegroundColour(fgcol);
    SetBackgroundColour(bgcol);
    dlg_close->SetForegroundColour(fgcol);
    dlg_close->SetBackgroundColour(bgcol);
    
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmsDlg::OnClose, this, ID_DLG_CLOSE);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmsDlg::OnSave, this, ID_DLG_SAVE);
    Bind(wxEVT_CHECKBOX, &AlarmsDlg::OnDirty, this, wxID_ANY);
    Bind(wxEVT_CHOICE, &AlarmsDlg::OnDirty, this, wxID_ANY);
    Bind(wxEVT_SPINCTRL, &AlarmsDlg::OnDirty, this, wxID_ANY);
    Bind(wxEVT_TEXT, &AlarmsDlg::OnDirty, this, wxID_ANY);

}


// Function to parse time from a string in "DDD HH:MM" format and return it as minutes since the start of the week
int parseTime(const std::string& dayAndTime) {
    int hours, minutes;
    char day[4];
    sscanf(dayAndTime.c_str(), "%3s %d:%d", day, &hours, &minutes);
    wxDateTime dt = wxDateTime::Now();

    // got tired of messing with this function so I'm brute forcing it a bit.

    if(!strcmp(day, "ALL")) // if day is "ALL"
    {
        // and if current time is too late for an alarm today, add 1 day to make it tomorrow 
        if( (hours * 60 + minutes) < (dt.GetHour() * 60 + dt.GetMinute()) ){
            dt.SetDay(dt.GetDay() +1);
            printf("%s is for ALL but it is too late in the day today, so changing it to %s\n",
                dayAndTime.c_str(), dt.Format("%a").ToStdString().c_str());
        }
        // otherwise just change 'ALL' to today's  abbreviated name
        strcpy(day, dt.Format("%a").char_str());
        std::cout << "Day is 'ALL' changing " << dayAndTime << " to today: " << day << std::endl;
    }

    // Map day abbreviations to their corresponding weekday numbers
    std::map<std::string, int> dayOfWeekMap = {
        {"Mon", 1}, {"Tue", 2}, {"Wed", 3}, {"Thu", 4},
        {"Fri", 5}, {"Sat", 6}, {"Sun", 7}
    };
    // Calculate total minutes since the start of the week
    int totalMinutes = (dayOfWeekMap[day] * 24 * 60) + (hours * 60) + minutes;
    return totalMinutes;
}

// Comparison function for sorting events by time
bool compareSchedules(const AlarmTime& a, const AlarmTime& b) {
    int timeA = parseTime(a.dayAndTime);
    int timeB = parseTime(b.dayAndTime);

    // If "ALL" is specified as the day, consider it to be earlier than any specific day
    if (a.dayAndTime.find("ALL") != std::string::npos) return false;
    if (b.dayAndTime.find("ALL") != std::string::npos) return true;

    return timeA < timeB;
}

std::vector<AlarmTime>& AlarmsDlg::getAlarms()
{
    static std::vector<AlarmTime> at;
    at.clear();
    AlarmTime a;

    for (auto& entryPtr : entryVec) {
        if(!entryPtr->entry_checkBoxEnable->GetValue())
            continue; // skip if disabled
        wxDateTime dt = wxDateTime::Now();
        dt.SetHour(entryPtr->entry_spinHour->GetValue() + (entryPtr->entry_choiceAMPM->GetSelection() ? 12:0));
        dt.SetMinute(entryPtr->entry_spinMinute->GetValue());
        dt.SetSecond(0);
        a.dayAndTime = entryPtr->entry_choiceDay->GetString(entryPtr->entry_choiceDay->GetSelection());
        a.dayAndTime.append(dt.Format(" %H:%M"));
        a.note = entryPtr->entry_textCtrlNote->GetValue().ToStdString();
        at.push_back(a);
    }
    
    std::sort(at.begin(), at.end(), compareSchedules);

    return at;
}

void AlarmsDlg::OnDirty(wxCommandEvent &e)
{
    dlg_save->Enable(true);
    dlg_save->SetBackgroundColour(bgcol);
    dlg_save->SetForegroundColour(fgcol);
}

void AlarmsDlg::OnSave(wxCommandEvent &e)
{
    json jentry;
    std::string alrm = std::getenv("HOME");
    alrm.append("/.config/wxAlarmClock/Alarms.json");

    std::ofstream f(alrm);
    for (auto& entryPtr : entryVec) {
        jentry =   {{"enable", entryPtr->entry_checkBoxEnable->GetValue()},
                    {"day", entryPtr->entry_choiceDay->GetSelection()},
                    {"hour", entryPtr->entry_spinHour->GetValue()},
                    {"minute", entryPtr->entry_spinMinute->GetValue()},
                    {"ampm", entryPtr->entry_choiceAMPM->GetSelection()},
                    {"note", entryPtr->entry_textCtrlNote->GetValue()}
            };
        f << jentry.dump(-1) << std::endl;
    }
    dlg_save->Enable(false);
    dlg_save->SetBackgroundColour(disbg);
    dlg_save->SetForegroundColour(disfg);
}


void AlarmsDlg::OnClose(wxCommandEvent &e)
{
    Close();
}