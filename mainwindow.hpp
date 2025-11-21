#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/event.h>
#include "wx/sound.h"
#include "pulse_audio.hpp"

class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
    wxSpinCtrl *spinHour;
    wxSpinCtrl *spinMinute;
    wxToggleButton *toggleButtonAMPM;
    wxButton *buttonStartStop;
    wxColour *startbutbgcolor;
    wxColour *stopbutbgcolor;
    wxStaticText* labelAlarmTime;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainsizer;
    wxTimer *timer;
    wxSound *alarm;
    int old_volume;
    int new_volume;
    void OnToggle(wxCommandEvent& event);
    void OnStartStop(wxCommandEvent& event);
    void OnCountDown(wxTimerEvent& event);
    void MonitorIdle();
    void OnAnyUserActivity(wxKeyEvent& event);
    void OnExit(wxCommandEvent& event);
};