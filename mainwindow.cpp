#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/event.h"
#include "wx/font.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/tglbtn.h"

#include "enums.hpp"
#include "mainwindow.hpp"

#include <cstddef>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
json config_data;

AlarmControlFrame::AlarmControlFrame()
    : wxFrame(nullptr, wxID_ANY, "wxAlarmClock")
{
    // Set up UI
    SetSize(600, 400);
    wxColour bgcolour(55, 55, 55, 255);
    wxColour fgcolour(220, 220, 200, 255);
    wxColour butbgcolour(75, 75, 75, 255);
    wxColour butfgcolour(220, 220, 200, 255);
    wxFont fnt(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    spinHour = new wxSpinCtrl(this, ID_SPIN_HOURS);
    spinHour->SetRange(0, 24); //12
    spinHour->SetBackgroundColour(bgcolour);
    spinHour->SetForegroundColour(fgcolour);
    spinMinute = new wxSpinCtrl(this, ID_SPIN_MINUTES);
    spinMinute->SetRange(0, 59);
    spinMinute->SetBackgroundColour(bgcolour);
    spinMinute->SetForegroundColour(fgcolour);
    toggleButtonAMPM = new wxToggleButton(this, wxID_ANY, "AM");
    toggleButtonAMPM->SetValue(true);
    toggleButtonAMPM->SetBackgroundColour(butbgcolour);
    toggleButtonAMPM->SetForegroundColour(butfgcolour);
    spinSizer = new wxBoxSizer(wxHORIZONTAL);
    spinSizer->Add(spinHour);
    spinSizer->AddSpacer(10);
    spinSizer->Add(spinMinute);
    spinSizer->AddSpacer(10);
    spinSizer->Add(toggleButtonAMPM);
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(fnt);
    buttonStartStop->SetForegroundColour(butfgcolour);
    buttonStartStop->SetBackgroundColour(butbgcolour);
    labelAlarmTime = new wxStaticText(this, wxID_ANY, "00:00");
    labelAlarmTime->SetFont(fnt);
    this->SetBackgroundColour(bgcolour);
    this->SetForegroundColour(fgcolour);
    labelAlarmTime->SetBackgroundColour(bgcolour);
    labelAlarmTime->SetForegroundColour(fgcolour);
    mainsizer = new wxBoxSizer(wxVERTICAL);
    mainsizer->AddSpacer(10);
    mainsizer->Add(spinSizer, 0, 1);
    mainsizer->AddSpacer(20);
    mainsizer->Add(buttonStartStop, 0, wxALIGN_CENTER_HORIZONTAL, 1);
    mainsizer->AddSpacer(100);
    mainsizer->Add(labelAlarmTime, 1, wxALIGN_CENTER_HORIZONTAL, 1);
    SetSizer(mainsizer);
    timer = new wxTimer(this);

    // Get and parse config file (only contains alarm audio file for now)
    std::string cfg = std::getenv("HOME");
    cfg.append("/.config/wxAlarmClock/wxAlarmClock.json");
    std::ifstream f(cfg);
    config_data = json::parse(f);

    std::string s = std::getenv("HOME");
    s.append("/");
    s.append(config_data["sound"]);
    alarm = new wxSound(s);

    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::StartStop, this, wxID_ANY);
    Bind(wxEVT_TIMER, &AlarmControlFrame::CountDown, this);
    Bind(wxEVT_TOGGLEBUTTON, &AlarmControlFrame::OnToggle, this, wxID_ANY);

}


void AlarmControlFrame::OnToggle(wxCommandEvent& event)
{
    toggleButtonAMPM->SetLabel(toggleButtonAMPM->GetValue() ? "AM":"PM");
}

// Start/Stop timer
bool start = false;
wxDateTime userInputTime;

void AlarmControlFrame::StartStop(wxCommandEvent& event)
{
    start = !start;
    if(start)
    {
        // if starting, get vals in spinners
        userInputTime = wxDateTime::Now();
        userInputTime.SetHour(spinHour->GetValue());
        userInputTime.SetMinute(spinMinute->GetValue());
        userInputTime.SetSecond(0);


        // start countdown timer if user input time is not zero
        if(userInputTime.GetHour() | userInputTime.GetMinute())
        {
            buttonStartStop->SetLabelText("Stop");
            wxString txt;
            txt.Printf("%d:%02d %s",
                userInputTime.GetHour(),
                userInputTime.GetMinute(),
                toggleButtonAMPM->GetValue() ? "am":"pm");
            labelAlarmTime->SetLabelText(txt);
            mainsizer->Layout();
            timer->Start(1000);
            this->SetTitle(txt);
            spinHour->Enable(false);
            spinMinute->Enable(false);
            toggleButtonAMPM->Enable(false);
        }
    } else {
        // if stopping (aborting) shut down timer
        timer->Stop();
        spinHour->Enable(true);
        spinMinute->Enable(true);
        toggleButtonAMPM->Enable(true);
        buttonStartStop->SetLabel("Start");
        this->SetTitle("wxAlarmClock");
    }
}


// Count down timer to alarm is running
void AlarmControlFrame::CountDown(wxTimerEvent& event)
{
    char propellor[4] = {'\\', '|', '/', '-'};
    static int rotate = 0;
    wxDateTime currentTime = wxDateTime::Now();
    wxString txt;

    // Make Animated Title
    if(rotate > 3)
        rotate = 0;
    txt.Printf("Alarm: %d:%02d %s %c",
        userInputTime.GetHour(),
        userInputTime.GetMinute(),
        toggleButtonAMPM->GetValue() ? "am":"pm", propellor[rotate++]);
    this->SetTitle(txt);
    
    // Adjust user input time according to AM/PM
    wxDateTime alarmTime;

    if(toggleButtonAMPM->GetValue())
    { // AM
        alarmTime = userInputTime;
    }else{ // PM
        alarmTime = userInputTime + wxTimeSpan(12);
    }

    // Compare times as strings...
    int cmp = alarmTime.FormatISOTime().Cmp(currentTime.FormatISOTime());
    
#ifdef DEBUG
    printf("alarm time: %ls currenttime: %ls cmp: %d\n",
        alarmTime.FormatISOTime().t_str(),
        currentTime.FormatISOTime().t_str(),
        cmp);
#endif
    // ...if equal, fire off alarm
    if(cmp == 0)
    {
        start = false;
        timer->Stop();
        buttonStartStop->SetLabel("Start");
        // Bind key press events to shut alarm up
        Bind(wxEVT_CHAR_HOOK, &AlarmControlFrame::keyPressEvent, this);    
        this->Raise();
        this->SetFocus();
        // Get old volume
        old_volume = get_old_vol();
        // Build an external (pulse audio) volume adjustment command based on config file
        new_volume = config_data["volume"];
        set_vol(new_volume);

        alarm->Play(wxSOUND_ASYNC|wxSOUND_LOOP);
    }
}

// Shut up alarm
void AlarmControlFrame::keyPressEvent(wxKeyEvent &event)
{
    // Reset key press event binding.
    Unbind(wxEVT_CHAR_HOOK, &AlarmControlFrame::keyPressEvent, this);
    alarm->Stop();
    set_vol(old_volume);
    spinHour->Enable(true);
    spinMinute->Enable(true);
    toggleButtonAMPM->Enable(true);
}
            
void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}
