#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/event.h"
#include "wx/font.h"
#include "wx/gtk/colour.h"
#include "wx/longlong.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/string.h"

#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include "enums.hpp"
#include "wx/tglbtn.h"
#include "mainwindow.hpp"
#include <fstream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;


AlarmControlFrame::AlarmControlFrame()
    : wxFrame(nullptr, wxID_ANY, "wxAlarmClock")
{
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

    std::string cfg = std::getenv("HOME");
    cfg.append("/.config/wxAlarmClock/wxAlarmClock.json");
    std::ifstream f(cfg);
    json data = json::parse(f);

    std::string s = std::getenv("HOME");
        s.append("/");
        s.append(data["snddir"]);
        s.append(data["sound"]);
    printf("JSON VALUE: %s\n", s.c_str());

    alarm = new wxSound(s);

    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::StartStop, this, wxID_ANY);
    Bind(wxEVT_TIMER, &AlarmControlFrame::CountDown, this);
    Bind(wxEVT_TOGGLEBUTTON, &AlarmControlFrame::OnToggle, this, wxID_ANY);

}

void AlarmControlFrame::keyPressEvent(wxKeyEvent &event)
{
    Unbind(wxEVT_CHAR_HOOK, &AlarmControlFrame::keyPressEvent, this);
    alarm->Stop();
}

void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}

void AlarmControlFrame::OnToggle(wxCommandEvent& event)
{
    toggleButtonAMPM->SetLabel(toggleButtonAMPM->GetValue() ? "AM":"PM");
}

bool start = false;
int timerseconds = 0;
wxDateTime userInputTime;
int hour, minute;
bool am;

void AlarmControlFrame::StartStop(wxCommandEvent& event)
{
    start = !start;
    if(start){
        //if starting, get vals in spinners and convert them to seconds
        //this needs to be converted from "time of day" and possibly date to "seconds from now"
        // for now (testing) just enter timeout directly
        userInputTime = wxDateTime::Now();
        userInputTime.SetHour(spinHour->GetValue());
        userInputTime.SetMinute(spinMinute->GetValue());
        userInputTime.SetSecond(0);

        hour = userInputTime.GetHour();
        minute = userInputTime.GetMinute();
        am = toggleButtonAMPM->GetValue();

        //timerseconds = spinHour->GetValue() * 3600 + spinMinute->GetValue() * 60;// + spinSeconds->GetValue();
        //start countdown timer with that value
        if(hour|minute)
        {
            buttonStartStop->SetLabelText("Stop");
            wxString txt;
            txt.Printf("%d:%02d %s", hour, minute, am ? "am":"pm");
            labelAlarmTime->SetLabelText(txt);
            mainsizer->Layout();
            timer->Start(1000);
            this->SetTitle(txt);
        }
    } else {
        //if stopping (aborting) shut down timer
        timer->Stop();
        buttonStartStop->SetLabel("Start");
        this->SetTitle("wxAlarmClock");
    }
}

bool metro = false;
char m[4] = {'/', '|', '\\', '-'};
int n = 0;
void AlarmControlFrame::CountDown(wxTimerEvent& event)
{
    wxDateTime currentTime = wxDateTime::Now();
    hour = spinHour->GetValue();
    minute = spinMinute->GetValue();
    am = toggleButtonAMPM->GetValue();
    wxString txt;
    metro = !metro;
    if(n > 3) n = 0;

    txt.Printf("%d:%02d %s",
        hour, minute, am ? "am":"pm");
    labelAlarmTime->SetLabelText(txt);
    txt.Printf("%d:%02d %s %c",
        hour, minute, am ? "am":"pm", m[n++]);
    this->SetTitle(txt);

    wxDateTime nextday;

    if(am){ // AM
        nextday = userInputTime;
    }else{ // PM
        nextday = userInputTime + wxTimeSpan(12);
    }

    printf("alarm time: %ls currenttime: %ls cmp: %d\n",
        nextday.FormatISOTime().t_str(),
        currentTime.FormatISOTime().t_str(),
        nextday.FormatISOTime().Cmp(currentTime.FormatISOTime()));

    if(nextday.FormatISOTime().Cmp(currentTime.FormatISOTime()) == 0){
        start = false;
        timer->Stop();
        //alarm here
        Bind(wxEVT_CHAR_HOOK, &AlarmControlFrame::keyPressEvent, this);    
        this->SetFocus();
         alarm->Play(wxSOUND_ASYNC|wxSOUND_LOOP);
        printf("ALARM! ALARM! ALARM!\n");
        buttonStartStop->SetLabel("Start");
    }
}

