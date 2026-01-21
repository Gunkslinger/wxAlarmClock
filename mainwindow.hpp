#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/event.h>
#include "wx/sound.h"
#include "pulse_audio.hpp"
#include "alarms_dlg.hpp"

// Main Window
class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
    wxButton *buttonStartStop;
    wxColour *startbutbgcolor;
    wxColour *stopbutbgcolor;
    wxColour *bgcolor;
    wxColour *fgcolor;
    wxStaticText* labelAlarmTime;
    wxStaticText* labelAlarmName;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainSizer;
    wxTimer *timer;
    wxSound *alarmPlayer;
    int old_volume;
    int new_volume;
    AlarmsDlg *alarms_dlg;
    void OnToggle(wxCommandEvent& event);
    void OnStartStop(wxCommandEvent& event);
    void UpdateLabels(AlarmTime&);
    void OnCountDown(wxTimerEvent& event);
    void OnAnyUserActivity(wxKeyEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnOpenDialog(wxCommandEvent &event);
    void loopPlay();
};