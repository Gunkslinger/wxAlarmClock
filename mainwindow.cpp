#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/event.h"
#include "wx/font.h"
#include "wx/gdicmn.h"
#include "wx/menu.h"
#include "wx/stattext.h"
#include "wx/sizer.h"
#include "wx/string.h"
#include "wx/stringimpl.h"
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
    f.close();
    
    // Get UI colors from the config file
    wxColour bgcolor((std::string) config_data["bgcolor"]);
    wxColour fgcolor((std::string) config_data["fgcolor"]);
    wxColour butbgcolor((std::string) config_data["butbgcolor"]);
    wxColour butfgcolor((std::string) config_data["butfgcolor"]);
    startbutbgcolor = new wxColour((std::string) config_data["startbutbgcolor"]);
    stopbutbgcolor = new wxColour((std::string)config_data["stopbutbgcolor"]);
    
    SetBackgroundColour(bgcolor);
    SetForegroundColour(fgcolor);
    wxFont butfnt(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    wxFont labfnt(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
#define MULTI_ALARMS
#ifdef MULTI_ALARMS
    wxMenuBar *menubar = new wxMenuBar;
    wxMenu *menuAlarms = new wxMenu;
    menuAlarms->Append(ID_ALARMS, "Open", "Tooltip");
    menubar->Append(menuAlarms, "Alarms");
    SetMenuBar(menubar);
    SetSize(420, 350);

    Bind(wxEVT_MENU, &AlarmControlFrame::OnOpenDialog, this, ID_ALARMS);
    alarms_dlg = new AlarmsDlg(this, fgcolor, bgcolor);
    alarms_dlg->Show(false);
#else
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
    spinSizer->AddSpacer(10);
    spinSizer->Add(spinHour);
    spinSizer->AddSpacer(10);
    spinSizer->Add(spinMinute);
    spinSizer->AddSpacer(10);
    spinSizer->Add(toggleButtonAMPM);
#endif
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(butfnt);
    buttonStartStop->SetForegroundColour(butfgcolor);
    buttonStartStop->SetBackgroundColour(*startbutbgcolor);
    labelAlarmTime = new wxStaticText(this, wxID_ANY, "00:00");
    labelAlarmTime->SetFont(labfnt);
    labelAlarmTime->SetBackgroundColour(bgcolor);
    labelAlarmTime->SetForegroundColour(fgcolor);
    mainSizer = new wxBoxSizer(wxVERTICAL);
#ifndef MULTI_ALARMS
    mainSizer->AddSpacer(10);
    mainSizer->Add(spinSizer, 0, wxALIGN_CENTER_HORIZONTAL);
#endif
    mainSizer->AddSpacer(20);
    mainSizer->Add(buttonStartStop, 0, wxALIGN_CENTER_HORIZONTAL, 1);
    mainSizer->AddSpacer(20);
    mainSizer->Add(labelAlarmTime, 1, wxALIGN_CENTER_HORIZONTAL, 1);
    SetSizer(mainSizer);
    timer = new wxTimer(this);

    std::string s = std::getenv("HOME");
    s.append("/");
    s.append(config_data["sound"]);
    alarm = new wxSound(s);

    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::OnStartStop, this, wxID_ANY);
    Bind(wxEVT_TIMER, &AlarmControlFrame::OnCountDown, this);
#ifndef MULTI_ALARMS
    Bind(wxEVT_TOGGLEBUTTON, &AlarmControlFrame::OnToggle, this, wxID_ANY);
#endif
}

void AlarmControlFrame::OnOpenDialog(wxCommandEvent &e)
{
    alarms_dlg->Show(true);
}

#ifndef MULTI_ALARMS
void AlarmControlFrame::OnToggle(wxCommandEvent& event)
{
    toggleButtonAMPM->SetLabel(toggleButtonAMPM->GetValue() ? "AM":"PM");
}
#endif

// Start/Stop button event
bool start = false;
#ifndef MULTI_ALARMS
wxDateTime userInputTime;
#endif

void AlarmControlFrame::OnStartStop(wxCommandEvent& event)
{
    start = !start;
#ifndef MULTI_ALARMS
    // if starting, get vals in spinners
    if(start)
    {
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
            mainSizer->Layout();
            this->SetTitle(txt);
            spinHour->Enable(false);
            spinMinute->Enable(false);
            toggleButtonAMPM->Enable(false);
            timer->Start(1000);
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
#else
    if(start){
        timer->Start(1000);
        buttonStartStop->SetLabelText("Stop");
        buttonStartStop->SetBackgroundColour(*stopbutbgcolor);
    } else {
        timer->Stop();
        buttonStartStop->SetLabel("Start");
        buttonStartStop->SetBackgroundColour(*startbutbgcolor);
    }
#endif
}

#ifdef MULTI_ALARMS
void AlarmControlFrame::OnCountDown(wxTimerEvent& event)
{
    char propellor[4] = {'\\', '|', '/', '-'};
    static int rotate = 0;
    wxDateTime currentTime = wxDateTime::Now();
    wxString txt;

    // Make Animated Title
    if(rotate > 3)
        rotate = 0;
    txt.Printf("Alarm Clock %c", propellor[rotate++]);
    this->SetTitle(txt);
    
    // keep alarm clock label current
    wxString labday = currentTime.Format("%a");
    int labhour = currentTime.GetHour();
    labhour = labhour > 12 ? labhour - 12: labhour;
    wxString labtxt;
    labtxt.Printf("%s %d:%02d:%02d %s",
                labday,
                labhour,
                currentTime.GetMinute(),
                currentTime.GetSecond(),
                currentTime.GetHour() > 12 ? "pm":"am");
    labelAlarmTime->SetLabelText(labtxt);
    mainSizer->Layout();

    // get current time formatted into an AlarmTime structure for comparison
    AlarmTime ct{
        .day = currentTime.Format("%a").ToStdString(), // extract day of week
        .time = currentTime.FormatISOTime().ToStdString(), // extract ISO time
    };
    int n = 0;
    std::vector<AlarmTime> at = alarms_dlg->getAlarms(); // get alarms
    for(std::vector<AlarmTime>::iterator atit = at.begin() ; atit != at.end(); ++atit){
        std::cout << n++ << " " << atit->day << " " << atit->time << " " << atit->note << std::endl;
        if(atit->day == ct.day || atit->day == "ALL"){ // if day and
            if(atit->time == ct.time){ // time matches then set up to play alarm
                // Bind key press and mouse events to shut alarm up
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
                MonitorIdle(); // monitor keyboard and mouse for activity globally

            }
        }
    }
    std::cout  << std::endl;
}
#else
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
    
//#ifdef DEBUG // in Makefile
    printf("alarm day of week: %ls alarm time: %ls currenttime: %ls cmp: %d\n",
        alarmTime.Format("%a").t_str(), // Abbrev. Day of Week
        alarmTime.FormatISOTime().t_str(),
        currentTime.FormatISOTime().t_str(),
        cmp);
//#endif

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
#endif

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
#ifndef MULTI_ALARMS
    spinHour->Enable(true);
    spinMinute->Enable(true);
    toggleButtonAMPM->Enable(true);
#endif
    printf("SHUT UP!!\n");
}
            
void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}
