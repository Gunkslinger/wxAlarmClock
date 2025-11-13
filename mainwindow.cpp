#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/font.h"
#include "wx/gtk/colour.h"
#include "wx/longlong.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/string.h"
#include "wx/sound.h"

#include <string>
#include <sstream>
#include <iostream>
#include "enums.hpp"
#include "mainwindow.hpp"

AlarmControlFrame::AlarmControlFrame()
    : wxFrame(nullptr, wxID_ANY, "wxAlarmClock")
{
    SetSize(600, 400);
    wxColour bgcolour(55, 55, 55, 255);
    wxColour fgcolour(220, 220, 200, 255);
    wxColour butbgcolour(75, 75, 75, 255);
    wxColour butfgcolour(220, 220, 200, 255);
    wxFont f(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    spinHour = new wxSpinCtrl(this, ID_SPIN_HOURS);
    spinHour->SetRange(0, 24);
    spinHour->SetBackgroundColour(bgcolour);
    spinHour->SetForegroundColour(fgcolour);
    spinMinute = new wxSpinCtrl(this, ID_SPIN_MINUTES);
    spinMinute->SetRange(0, 59);
    spinMinute->SetBackgroundColour(bgcolour);
    spinMinute->SetForegroundColour(fgcolour);
    checkBoxAMPM = new wxCheckBox(this, wxID_ANY, "AM");
    spinSizer = new wxBoxSizer(wxHORIZONTAL);
    spinSizer->Add(spinHour);
    spinSizer->AddSpacer(10);
    spinSizer->Add(spinMinute);
    spinSizer->AddSpacer(10);
    spinSizer->Add(checkBoxAMPM);
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(f);
    buttonStartStop->SetForegroundColour(butfgcolour);
    buttonStartStop->SetBackgroundColour(butbgcolour);
    labelAlarmTime = new wxStaticText(this, wxID_ANY, "00:00");
    labelAlarmTime->SetFont(f);
    this->SetBackgroundColour(bgcolour);
    this->SetForegroundColour(fgcolour);
    labelAlarmTime->SetBackgroundColour(bgcolour);
    labelAlarmTime->SetForegroundColour(fgcolour);
    mainsizer = new wxBoxSizer(wxVERTICAL);
    mainsizer->AddSpacer(10);
    mainsizer->Add(spinSizer, 0, 1);
    mainsizer->AddSpacer(20);
    mainsizer->Add(buttonStartStop, 0, 1, 1);
    mainsizer->AddSpacer(100);
    mainsizer->Add(labelAlarmTime, 0, 1, 1);
    SetSizer(mainsizer);
    timer = new wxTimer(this);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::StartStop, this, wxID_ANY);
    // Bind(wxEVT_MENU, &AlarmControlFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_TIMER, &AlarmControlFrame::CountDown, this);
}

void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    printf("OUCH!\n");
    Close();
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
        am = checkBoxAMPM->GetValue();

        //timerseconds = spinHour->GetValue() * 3600 + spinMinute->GetValue() * 60;// + spinSeconds->GetValue();
        //start countdown timer with that value
        if(hour|minute)
        {
            buttonStartStop->SetLabelText("Stop");
            wxString txt;
            txt.Printf("%d:%02d %s", hour, minute, am ? "am":"pm");
            labelAlarmTime->SetLabelText(txt);
            printf("start time: %ls %ls\n", txt.t_str(), userInputTime.FormatDate().t_str());
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


void AlarmControlFrame::CountDown(wxTimerEvent& event)
{
    wxDateTime currentTime = wxDateTime::Now();
    hour = spinHour->GetValue();
    minute = spinMinute->GetValue();
    am = checkBoxAMPM->GetValue();
    wxString txt;
    txt.Printf("%d:%02d %s", hour, minute, am ? "am":"pm");
    labelAlarmTime->SetLabelText(txt);
    
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
        wxSound alarm("./cuckoo.wav");
        alarm.Play(wxSOUND_ASYNC|wxSOUND_LOOP);
        printf("ALARM! ALARM! ALARM!\n");
        buttonStartStop->SetLabel("Start");
    }
}

