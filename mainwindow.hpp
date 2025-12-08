#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/event.h>
#ifdef MULTI_ALARMS
#include "wx/choice.h"
#endif
#include "wx/list.h"
#include "wx/sound.h"
#include "pulse_audio.hpp"
#include "alarms_dlg.hpp"

class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
#ifdef MULTI_ALARMS
    wxChoice *choiceDays;
    wxSpinCtrl *spinHour;
    wxSpinCtrl *spinMinute;
    wxToggleButton *toggleButtonAMPM;
#endif
    wxButton *buttonStartStop;
    wxColour *startbutbgcolor;
    wxColour *stopbutbgcolor;
    wxStaticText* labelAlarmTime;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainSizer;
    wxTimer *timer;
    wxSound *alarm;
    int old_volume;
    int new_volume;
    AlarmsDlg *alarms_dlg;
    void OnToggle(wxCommandEvent& event);
    void OnStartStop(wxCommandEvent& event);
    void OnCountDown(wxTimerEvent& event);
    void MonitorIdle();
    void OnAnyUserActivity(wxKeyEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnOpenDialog(wxCommandEvent &event);
};