#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/event.h"
#include "wx/font.h"
#include "wx/gtk/colour.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/tglbtn.h"

#include "wx/event.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

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
    SetSize(420, 300);
    // Get and parse config file (only contains alarm audio file and volume for now)
    std::string cfg = std::getenv("HOME");
    cfg.append("/.config/wxAlarmClock/wxAlarmClock.json");
    std::ifstream f(cfg);
    config_data = json::parse(f);

    // Get UI colors from the config file
    std::string rgb;
    rgb.append(config_data["bgcolor"]);
    wxColour bgcolor(rgb);
    rgb.clear();
    rgb.append(config_data["fgcolor"]);
    wxColour fgcolor(rgb);
    rgb.clear();
    rgb.append(config_data["butbgcolor"]);
    wxColour butbgcolor(rgb);
    rgb.clear();
    rgb.append(config_data["butfgcolor"]);
    wxColour butfgcolor(rgb);
    rgb.clear();
    rgb.append(config_data["startbutbgcolor"]);
    startbutbgcolor = new wxColour(rgb);
    rgb.clear();
    rgb.append(config_data["stopbutbgcolor"]);
    stopbutbgcolor = new wxColour(rgb);
    
    SetBackgroundColour(bgcolor);
    SetForegroundColour(fgcolor);
    wxFont fnt(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    spinHour = new wxSpinCtrl(this, ID_SPIN_HOURS);
    spinHour->SetRange(1, 12);
    spinHour->SetBackgroundColour(bgcolor);
    spinHour->SetForegroundColour(fgcolor);
    spinMinute = new wxSpinCtrl(this, ID_SPIN_MINUTES);
    spinMinute->SetRange(0, 59);
    spinMinute->SetBackgroundColour(bgcolor);
    spinMinute->SetForegroundColour(fgcolor);
    toggleButtonAMPM = new wxToggleButton(this, wxID_ANY, "AM");
    toggleButtonAMPM->SetValue(true);
    toggleButtonAMPM->SetBackgroundColour(butbgcolor);
    toggleButtonAMPM->SetForegroundColour(butfgcolor);
    spinSizer = new wxBoxSizer(wxHORIZONTAL);
    spinSizer->Add(spinHour);
    spinSizer->AddSpacer(10);
    spinSizer->Add(spinMinute);
    spinSizer->AddSpacer(10);
    spinSizer->Add(toggleButtonAMPM);
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(fnt);
    buttonStartStop->SetForegroundColour(butfgcolor);
    buttonStartStop->SetBackgroundColour(*startbutbgcolor);
    labelAlarmTime = new wxStaticText(this, wxID_ANY, "00:00");
    labelAlarmTime->SetFont(fnt);
    labelAlarmTime->SetBackgroundColour(bgcolor);
    labelAlarmTime->SetForegroundColour(fgcolor);
    mainsizer = new wxBoxSizer(wxVERTICAL);
    mainsizer->AddSpacer(10);
    mainsizer->Add(spinSizer, 0, 1);
    mainsizer->AddSpacer(20);
    mainsizer->Add(buttonStartStop, 0, wxALIGN_CENTER_HORIZONTAL, 1);
    mainsizer->AddSpacer(20);
    mainsizer->Add(labelAlarmTime, 1, wxALIGN_CENTER_HORIZONTAL, 1);
    SetSizer(mainsizer);
    timer = new wxTimer(this);

    std::string s = std::getenv("HOME");
    s.append("/");
    s.append(config_data["sound"]);
    alarm = new wxSound(s);

    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::OnStartStop, this, wxID_ANY);
    Bind(wxEVT_TIMER, &AlarmControlFrame::OnCountDown, this);
    Bind(wxEVT_TOGGLEBUTTON, &AlarmControlFrame::OnToggle, this, wxID_ANY);

}


void AlarmControlFrame::OnToggle(wxCommandEvent& event)
{
    toggleButtonAMPM->SetLabel(toggleButtonAMPM->GetValue() ? "AM":"PM");
}

// Start/Stop button event
bool start = false;
wxDateTime userInputTime;

void AlarmControlFrame::OnStartStop(wxCommandEvent& event)
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
            buttonStartStop->SetBackgroundColour(*stopbutbgcolor);
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
        buttonStartStop->SetBackgroundColour(*startbutbgcolor);
        this->SetTitle("wxAlarmClock");
    }
}

// Timer event handler to count down until alarm should play
void AlarmControlFrame::OnCountDown(wxTimerEvent& event)
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
    
#ifdef DEBUG // in Makefile
    printf("alarm time: %ls currenttime: %ls cmp: %d\n",
        alarmTime.FormatISOTime().t_str(),
        currentTime.FormatISOTime().t_str(),
        cmp);
#endif

    // ...if equal, fire off alarm
    if(cmp == 0)
    {
        // start = false;
        // timer->Stop();
        // buttonStartStop->SetLabel("Start");
        // buttonStartStop->SetBackgroundColour(*startbutbgcolor);

        // Bind key press events to shut alarm up
        Bind(wxEVT_CHAR_HOOK, &AlarmControlFrame::OnAnyUserActivity, this);    

        // get user-specified alarm volume from config
        new_volume = config_data["volume"];
        // Get old volume to reset to pre-alarm setting after alarm stops
        old_volume = get_old_vol();
        // if old volume couldn't be gotten (hasn't happened yet)
        // just set old volume to new volume and walk away
        if(old_volume < 0) old_volume = new_volume;
        set_vol(new_volume);
        
        alarm->Play(wxSOUND_ASYNC|wxSOUND_LOOP);
        sleep(2);   // in the case where user is active at the computer, allow alarm to
                            // play for 2 seconds before monitoring for shut-off signal.

        // BUG: in wxWidgets Focus is not being granted in some cases even thought HasFocus is
        // reporting true. Why?
        //
        // within wxWidgets, Focus is needed in order to receive keypress events, which can be used
        // to signal silencing of the alarm.
        
        // Becuase of this problem I've written my own function that queries the status
        // of X11's XScreenSaver and if its timer has been reset since the last query then an event
        // is posted which causes the alarm sound to stop playing. This way this app's window does
        // not need to be raised or focused. It can be "iconized" into the system tray or behind
        // another window and it will still receive notification of keypresses and mouse motions
        // to trigger the shut off.

        MonitorIdle(); // new func to monitor keyboard and mouse for activity globally
    }
}

// Monitor for keyboard or mouse activity and post an event if detected
// This indicates that the user wants the alarm to stop regardless of the
// window's focus, minimization, or Z value in the window stack.

void AlarmControlFrame::MonitorIdle() {
    Display *dpy = XOpenDisplay(NULL);
    Window w = DefaultRootWindow(dpy);
    XScreenSaverInfo *ss_info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(dpy, w, ss_info);
    int prev_idle = ss_info->idle;
    while(1){ // alarm is playing
        usleep(1000);
        XScreenSaverQueryInfo(dpy, w, ss_info);
        if(ss_info->idle > prev_idle){
            prev_idle = ss_info->idle;
        } else {
            // post event to trigger shutting the alarm off (with OnAnyUserActivity())
            wxCommandEvent e(wxEVT_CHAR_HOOK);
            wxPostEvent(this, e);
            printf("Posting event. idle time (ms): %zu\n", ss_info->idle);
            XFree(ss_info);
            return;
        }
    }
}

// SHUT UP, DAMNED ALARM!
// We reveived an angry key press or something from the sleeping user,
// so reset key press event binding, silence the alarm, restore old volume,
// and reenable UI.

void AlarmControlFrame::OnAnyUserActivity(wxKeyEvent &event)
{
    Unbind(wxEVT_CHAR_HOOK, &AlarmControlFrame::OnAnyUserActivity, this);
    alarm->Stop();
    sleep(1); // hack: give play buffer a chance to drain before changing volume
    set_vol(old_volume); // restore old volume
    spinHour->Enable(true);
    spinMinute->Enable(true);
    toggleButtonAMPM->Enable(true);
    printf("SHUT UP!!\n");
}
            
void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}
