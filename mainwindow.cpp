#include "wx/colour.h"
#include "wx/datetime.h"
#include "wx/font.h"
#include "wx/menu.h"
#include "wx/sizer.h"
#include "wx/string.h"
#include "wx/tglbtn.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

#include "enums.hpp"
#include "consts.hpp"
#include "mainwindow.hpp"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "profiling_wrapper.hpp"

// JSON Configuration File
using json = nlohmann::json;
json config_data;

// Main Window
AlarmControlFrame::AlarmControlFrame()
    : wxFrame(nullptr, wxID_ANY, "wxAlarmClock")
{
    // Get and parse config file (only contains UI foreground and background colors,
    // alarm audio file, and volume for now)
    std::string cfgpath = std::getenv("HOME");
    cfgpath.append("/.config/wxAlarmClock/wxAlarmClock.json");
    std::ifstream f(cfgpath);
    config_data = json::parse(f);
    f.close();

    // Set up UI
    SetSize(600, 400);

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

    // Menu
    wxMenuBar *menubar = new wxMenuBar;
    wxMenu *menuAlarms = new wxMenu;
    menuAlarms->Append(ID_ALARMS, "Open", "Tooltip");
    menubar->Append(menuAlarms, "Alarms");
    SetMenuBar(menubar);
    Bind(wxEVT_MENU, &AlarmControlFrame::OnOpenDialog, this, ID_ALARMS);

    // Alarm Configuration Dialog
    alarms_dlg = new AlarmsDlg(this, *fgcolor, *bgcolor);
    alarms_dlg->Show(false);
    // Start/Stop Button
    buttonStartStop = new wxButton(this, ID_STARTSTOP_BUTT, wxString("Start"));
    buttonStartStop->SetFont(butfnt);
    buttonStartStop->SetForegroundColour(*fgcolor);
    buttonStartStop->SetBackgroundColour(*startbutbgcolor);
    // Current Time Label
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

    // Timer (1 second interval)
    timer = new wxTimer(this);

    // Default Alarm Sound from app config.
    // TODO: This might be moved to AlarmDlg for user configurable sounds per alarm, or left as default
    std::string default_alarm = std::getenv("HOME");
    default_alarm.append("/");
    // default_alarm.append(config_data["default_sound"]);
    default_alarm.append(config_data["sound"]);
    alarmPlayer = new wxSound(default_alarm);

    Bind(wxEVT_BUTTON, &AlarmControlFrame::OnStartStop, this, wxID_ANY);
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

void AlarmControlFrame::loopPlay()
{
    labelAlarmName->SetForegroundColour(*fgcolor); // only enable/active color should be set here
    this->Layout();

    Display *dpy = XOpenDisplay(NULL);
    Window w = DefaultRootWindow(dpy);
    XScreenSaverInfo *ss_info = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(dpy, w, ss_info);
    int prev_idle = ss_info->idle;

    new_volume = config_data["volume"];
    // Get old volume to reset to pre-alarm setting after alarm stops
    old_volume = get_old_vol();
    if(old_volume < 0) old_volume = new_volume;
    set_vol(new_volume);
    while(1){
        alarmPlayer->Play(wxSOUND_SYNC);
        usleep(1000); // 1 millisecond
        XScreenSaverQueryInfo(dpy, w, ss_info);
        if(ss_info->idle > prev_idle){
            prev_idle = ss_info->idle;
        } else {
            set_vol(old_volume);
            break;
        }
    }
}



extern int parseTime(std::string&);

bool compareTimes(const AlarmTime& a, const AlarmTime& b)
{
    bool x = parseTime(a.dayAndTime) == parseTime(b.dayAndTime);
#ifdef DEBUG
    printf("%s %d compareTimes calling parseTime twice. result: a %s b\n",
        __FILE__, __LINE__, x? "equals":"does not equal");
#endif
    return x;
}

// Find the next alarm in the sorted vector 'alarmsVec'
AlarmTime findNextAlarm(const std::vector<AlarmTime>& sortedAlarmsVector, const AlarmTime& currentTimeEvent) {
    int currentMinutes = parseTime(currentTimeEvent.dayAndTime);
    int alarmMinutes;
    struct candidate {
        int diff;
        AlarmTime alarm;
    };

    std::vector<candidate> candidates;


    // fill candidates list with next days candidates
    for (const auto& alarm : sortedAlarmsVector) {
        alarmMinutes = parseTime(alarm.dayAndTime);
        if (alarmMinutes > currentMinutes) {
            candidates.push_back(candidate {alarmMinutes - currentMinutes, alarm});
#ifdef DEBUG
            std::cout   << __FILE__ " " << __LINE__ << " findNextAlarm: found alarm on next day. "
                        << "currrentMinutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes
                        << std::endl;
#endif
        }
    }
    
    // search through all collected candates looking for the one closest to the current time and return it.
    if(!candidates.empty())
    {
        int mindiff = WEEK_IN_MINUTES_PLUS_ONE;
        AlarmTime a;
        for(auto& cand: candidates)
        {
            if(cand.diff < mindiff)
            {
                mindiff = cand.diff;
                a = cand.alarm;
            }
        }
#ifdef DEBUG
        printf("%s %d findNextAlarm: returning %s from %zu candidates. time difference: %d\n", __FILE__, __LINE__,
            a.dayAndTime.c_str(), candidates.size(), mindiff);
#endif
        return a;
    }
#ifdef DEBUG    
    std::cout   << __FILE__ << " " << __LINE__ << " findNextAlarm: couldn't find alarm when handling ALL or next day! Bailing out!"
                << "currentMinutes: " << currentMinutes << " alarmMinutes: " << alarmMinutes << " candidates: " << candidates.size()
                << std::endl;
#endif
    // If all events are earlier than or at the same time as the current event,
    // return an empty alarm (description will be empty)
    AlarmTime emptyAlarm;
    emptyAlarm.note = "";
#ifdef DEBUG
    std::cout << __FILE__ << " " << __LINE__ << " findNextAlarm: no alarm found. returning EMPTY alarm" << std::endl;
#endif
    return emptyAlarm;
}


// Window title and ToD label
void AlarmControlFrame::UpdateLabels(AlarmTime& alarm)
{
    int propellor[] =  {0x2190, 0x2196, 0x2191, 0x2197,
        0x2192, 0x2198, 0x2193, 0x2199};
    static int rotate = 0;
    wxString txt;
    // Make Animated Window Title
    if(rotate > 7)
        rotate = 0;
    txt.Printf("%c %s at %s %c",
        propellor[rotate],
        alarm.note,
        wxDateTime::Now().Format("%a %H:%M:%S"), //alarm.dayAndTime.substr(4),
        propellor[rotate]);
    this->SetTitle(txt);
    rotate++;
    
    // keep alarm clock TOD label current
    labelAlarmTime->SetLabelText(wxDateTime::Now().Format("%a %I:%M:%S %p"));
    mainSizer->Layout();
}
            
// When running, this function gets called every second. It animates the propeller and
// upadates the TOD label via UpdateLabels().
// It then compares the current time to the alarms and if equal to one of them calls
// the play function with that alarm as a parameter.
extern wxDateTime& fakeNow(bool inc);

void AlarmControlFrame::OnCountDown(wxTimerEvent& event)
{
    _TIMER_START

    static AlarmTime old_nextalarm{"", ""};

    // get current time formatted into an AlarmTime structure for comparison
    AlarmTime ct{
        .dayAndTime = wxDateTime::Now().Format("%a %H:%M").ToStdString(), // extract formatted DoW and ToD
        .note = ""
    };

    std::vector<AlarmTime> at = alarms_dlg->getAlarms(); // get sorted alarms

#ifdef DEBUG
    std::cout << __FILE__ << " " << __LINE__ << " OnCountDown: Sorted alarms received from getAlarms(): " << std::endl;
    for (const auto& atit : at) {
        std::cout << atit.dayAndTime << std::endl;
    }
#endif

    AlarmTime nextAlarm = findNextAlarm(at, ct);
    if (!nextAlarm.dayAndTime.empty()) {
#ifdef DEBUG
        std::cout << "OnCountDown: The next alarm is on " << nextAlarm.dayAndTime << ": " << nextAlarm.note << std::endl;
#endif
        int hour = std::stoi(nextAlarm.dayAndTime.substr(4, 2));
        int min = std::stoi(nextAlarm.dayAndTime.substr(7, 2));
        wxString s;
        s.Printf("%s %d:%02d %s %s",
            nextAlarm.dayAndTime.substr(0, 3).c_str(),
            hour >= 12 ? hour -12: hour,
            min,
            hour >= 12 ? "pm":"am",
            nextAlarm.note.c_str()); 
        labelAlarmName->SetLabelText(s);
        this->Layout();
            
        UpdateLabels(nextAlarm);

        if(compareTimes(old_nextalarm, ct)){
            loopPlay();
        }
    } else {
        labelAlarmName->SetLabelText("No next alarm found.");
        this->Layout();
#ifdef DEBUG
        std::cout << "OnCountDown: No next alarm found." << std::endl;
#endif
    }
    old_nextalarm = nextAlarm;

_MICRO_TIMER_FINISH    
      printf("OnCountDown() took %.0f microseconds\n",
                              profile__averageTime);
}


// SHUT UP, DAMNED ALARM!
// We reveived an angry key press or something from the sleeping user,
// so reset key press event binding, silence the alarm, restore old volume,
// and reenable UI.

void AlarmControlFrame::OnAnyUserActivity(wxKeyEvent &event)
{
    alarmPlayer->Stop();
//    sleep(1); // hack: give play buffer a chance to drain before changing volume
    set_vol(old_volume); // restore old volume
#ifdef DEBUG
    printf("SHUT UP!!\n");
#endif
    //TODO for per alarm sounds, free current alarm here
}
            
void AlarmControlFrame::OnExit(wxCommandEvent& event)
{
    Close();
}
