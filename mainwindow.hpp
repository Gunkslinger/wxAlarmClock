#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/timer.h>
#include <wx/event.h>
#include "wx/sound.h"

class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
    wxSpinCtrl *spinHour;
    wxSpinCtrl *spinMinute;
    wxToggleButton *toggleButtonAMPM;
    wxButton *buttonStartStop;
    wxStaticText* labelAlarmTime;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainsizer;
    wxTimer *timer;
    wxSound *alarm;
    void OnExit(wxCommandEvent& event);
    void OnToggle(wxCommandEvent& event);
    void StartStop(wxCommandEvent& event);
    void CountDown(wxTimerEvent& event);
    void keyPressEvent(wxKeyEvent& event);
};