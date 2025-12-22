#include "wx/any.h"
#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/event.h"
#include "wx/font.h"
#include "wx/gdicmn.h"
#include "wx/gtk/font.h"
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
    bgcolor = new wxColor((std::string) config_data["bgcolor"]);
    fgcolor = new wxColor((std::string) config_data["fgcolor"]);
    startbutbgcolor = new wxColour((std::string) config_data["startbutbgcolor"]);
    stopbutbgcolor = new wxColour((std::string)config_data["stopbutbgcolor"]);
    
    SetBackgroundColour(*bgcolor);
    SetForegroundColour(*fgcolor);
    wxFont butfnt(30, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    wxFont labfnt(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false);
    wxMenuBar *menubar = new wxMenuBar;
    wxMenu *menuAlarms = new wxMenu;
    menuAlarms->Append(ID_ALARMS, "Open", "Tooltip");
    menubar->Append(menuAlarms, "Alarms");
    SetMenuBar(menubar);
    SetSize(420, 320);

    Bind(wxEVT_MENU, &AlarmControlFrame::OnOpenDialog, this, ID_ALARMS);
    alarms_dlg = new AlarmsDlg(this, *fgcolor, *bgcolor);
    alarms_dlg->Show(false);
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(butfnt);
    buttonStartStop->SetForegroundColour(*fgcolor);
    buttonStartStop->SetBackgroundColour(*startbutbgcolor);
    labelAlarmTime = new wxStaticText(this, wxID_ANY, "00:00");
    labelAlarmTime->SetFont(labfnt);
    labelAlarmTime->SetBackgroundColour(*bgcolor);
    labelAlarmTime->SetForegroundColour(*fgcolor);
    labelAlarmName = new wxStaticText(this, wxID_ANY, "");
    labelAlarmName->SetFont(wxFont(12,wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL, false));
    labelAlarmName->SetBackgroundColour(*bgcolor);
    labelAlarmName->SetForegroundColour(*fgcolor);
    mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->AddSpacer(20);
    mainSizer->Add(buttonStartStop, 0, wxALIGN_CENTER_HORIZONTAL, 1);
    mainSizer->AddSpacer(20);
    mainSizer->Add(labelAlarmTime, 1, wxALIGN_CENTER_HORIZONTAL, 1);
    mainSizer->AddSpacer(20);
    mainSizer->Add(labelAlarmName, 1, wxALIGN_CENTER_HORIZONTAL, 1);
    SetSizer(mainSizer);
    timer = new wxTimer(this);

    std::string s = std::getenv("HOME");
    s.append("/");
    s.append(config_data["sound"]);
    alarm = new wxSound(s);

    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AlarmControlFrame::OnStartStop, this, wxID_ANY);
    Bind(wxEVT_TIMER, &AlarmControlFrame::OnCountDown, this);
}

void AlarmControlFrame::OnOpenDialog(wxCommandEvent &e)
{
    alarms_dlg->Show(true);
}

// Start/Stop button event
bool start = false;

void AlarmControlFrame::OnStartStop(wxCommandEvent& event)
{
    start = !start;
    if(start){
        timer->Start(1000);
        buttonStartStop->SetLabelText("Stop");
        buttonStartStop->SetBackgroundColour(*stopbutbgcolor);
    } else {
        timer->Stop();
        buttonStartStop->SetLabel("Start");
        buttonStartStop->SetBackgroundColour(*startbutbgcolor);
        labelAlarmName->SetLabel("");
    }
}

void AlarmControlFrame::startPlaying()
{
    //labelAlarmName->SetLabelText(at.note); // TODO: this needs to get updated with upcoming alarm before it plays
    labelAlarmName->SetForegroundColour(*fgcolor); // only enable/active color should be set here
    this->Layout();

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
    MonitorIdle(); // monitor keyboard and mouse for activity globall
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
        usleep(1000); // 1 millisecond
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

#ifdef DAYANDTIME_VERSION
extern int parseTime(std::string&);

bool compareTimes(const AlarmTime& a, const AlarmTime& b)
{
    return parseTime(a.dayAndTime) == parseTime(b.dayAndTime);
}

// Find the next alarm in the sorted vector 'alarmsVec'
AlarmTime findNextAlarm(const std::vector<AlarmTime>& alarmsVec, const AlarmTime& currentTimeEvent) {
    int currentMinutes = parseTime(currentTimeEvent.dayAndTime);
    int alarmMinutes;

    // handle ALL
    for (const auto& alarm : alarmsVec) {
        if (alarm.dayAndTime.find("ALL") != std::string::npos || 
            alarm.dayAndTime.substr(0, 3) == currentTimeEvent.dayAndTime.substr(0, 3)) {
            alarmMinutes = parseTime(alarm.dayAndTime);
            if (alarmMinutes > currentMinutes) {
                std::cout   << "findNextAlarm: found alarm when handling 'ALL' "
                            << "cur minutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes
                            << std::endl;
                return alarm;
            }
        }
    }
    std::cout   << "findNextAlarm: couldn't find alarm when handling 'ALL'. "
                << "cur minutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes
                << std::endl;

    // If no next alarm is found on the same day or with "ALL" specified as the day,
    // return the first alarm of the next day
    for (const auto& alarm : alarmsVec) {
        alarmMinutes = parseTime(alarm.dayAndTime);
        if (alarmMinutes > currentMinutes) {
            std::cout   << "findNextAlarm: found alarm on next day. "
                        << "cur minutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes
                        << std::endl;
            return alarm;
        }
    }
    std::cout   << "findNextAlarm: couldn't find alarm when handling next day "
                << "cur minutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes
                << std::endl;

    // If all events are earlier than or at the same time as the current event,
    // return an empty event (description will be empty)

    // THIS IS REACHED WHEN IT SHOULDN'T, LIKE WHEN "ALL" IS FOUND.
    AlarmTime emptyAlarm;
    emptyAlarm.note = ""; std::cout << "findNextAlarm: no alarm found. returning EMPTY alarm" << std::endl;// return alarmsVec[1];
    return emptyAlarm;
}

#endif

int propellor[] =  {0x2190, 0x2196, 0x2191, 0x2197,
    0x2192, 0x2198, 0x2193, 0x2199};

void AlarmControlFrame::UpdateLabels(wxDateTime& currentTime)
{
    static int rotate = 0;
    wxString txt;
    // Make Animated Window Title
    if(rotate > 7)
        rotate = 0;
    txt.Printf("%c Alarm Clock %c", propellor[rotate], propellor[rotate]);
    this->SetTitle(txt);
    rotate++;
    
    // keep alarm clock TOD label current
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
                
}
            
// When running this function gets called every second. It animates the propeller and
// upadates the TOD label via UpdateLabels().
// It then compares the current time to the alarms and if equal to one of them calls
// the play function with that alarm as a parameter.

void AlarmControlFrame::OnCountDown(wxTimerEvent& event)
{
    wxDateTime currentTime = wxDateTime::Now();

    UpdateLabels(currentTime);
    
#ifdef DAYANDTIME_VERSION
static AlarmTime old_nextalarm;

    // get current time formatted into an AlarmTime structure for comparison
    AlarmTime ct{
        .dayAndTime = currentTime.Format("%a %H:%M").ToStdString(), // extract formatted DoW and ToD
        .note = ""
    };

    std::vector<AlarmTime> at = alarms_dlg->getAlarms(); // get sorted alarms
    // debugging
    std::cout << "OnCountDown: Sorted alarms received from getAlarms(): " << std::endl;
    for(std::vector<AlarmTime>::iterator atit = at.begin() ; atit != at.end(); ++atit){
        std::cout << atit->dayAndTime << std::endl;
    }

    auto nextAlarm = findNextAlarm(at, ct);
    if (!nextAlarm.dayAndTime.empty()) {
        std::cout << "OnCountDown: The next alarm is on " << nextAlarm.dayAndTime << ": " << nextAlarm.note << std::endl;
        wxString s;
        std::string adate = nextAlarm.dayAndTime.substr(0, 3);
        int hour = std::stoi(nextAlarm.dayAndTime.substr(4, 2));
        int min = std::stoi(nextAlarm.dayAndTime.substr(7, 2));
        
        s.Printf("%d:%02d %s %s",
                    hour >= 12 ? hour -12: hour,
                    min,
                    hour >= 12 ? "pm":"am",
                    nextAlarm.note.c_str()); 
        labelAlarmName->SetLabelText(s);
        this->Layout();
        if(compareTimes(old_nextalarm, ct)){ // THIS NEVER BECOMES TRUE BECAUSE NEXTALARM UPDATES TOO SOON. I think.
            printf("\n\nPLAYING ************\n\n");
            startPlaying();                     // a static old_nextalarm is needed to do the comparison. DONE.
        }
    } else {
        labelAlarmName->SetLabelText("No next alarm found.");
        this->Layout();
        std::cout << "OnCountDown: No next alarm found." << std::endl;
    }
    old_nextalarm = nextAlarm;

#else
    // get current time formatted into an AlarmTime structure for comparison
    AlarmTime ct{
        .day = currentTime.Format("%a").ToStdString(), // extract day of week
        .time = currentTime.FormatISOTime().ToStdString(), // extract ISO time
    };

    int n = 0; // for printing
    std::vector<AlarmTime> at = alarms_dlg->getAlarms(); // get sorted alarms
    for(std::vector<AlarmTime>::iterator atit = at.begin() ; atit != at.end(); ++atit){
        if(atit->day == ct.day || atit->day == "ALL"){ // if day and ...
            std::cout << n++ << " " << atit->day << " " << atit->time << " " << atit->note;
            std::cout << " " << ct.day << " " << ct.time << std::endl;
            if(atit->time == ct.time){ // ... time matches then set up to play alarm
                startPlaying(*atit);
            }
        }
    }
    std::cout  << std::endl;
#endif
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
    printf("SHUT UP!!\n");
}
            
void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}
