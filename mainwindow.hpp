#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/timer.h>
#include <wx/event.h>

class AlarmControlFrame : public wxFrame
{
public:
    AlarmControlFrame();
    wxSpinCtrl *spinHour;
    wxSpinCtrl *spinMinute;
    wxCheckBox *checkBoxAMPM;
    wxButton *buttonStartStop;
    wxStaticText* labelAlarmTime;
    wxBoxSizer *spinSizer;
    wxBoxSizer *mainsizer;
    wxTimer *timer;
    void OnExit(wxCommandEvent& event);
    void StartStop(wxCommandEvent& event);
    void CountDown(wxTimerEvent& event);
};